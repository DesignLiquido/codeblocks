#include "plugin.h"
#include "manager.h"
#include "runner.h"

#include <sdk.h>
#include <cbeditor.h>
#include <configurationpanel.h>
#include <configmanager.h>
#include <editormanager.h>
#include <logmanager.h>
#include <loggers.h>
#include <manager.h>
#include <sdk_events.h>

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <vector>

// Registra o plugin no Code::Blocks
namespace
{
    PluginRegistrant<LinguagensDLPlugin> reg("LinguagensDL");

    const int idMenuExecutarArquivoAtual = wxNewId();
    const wxString rotuloMenuPlugin = "Design &Liquido";
    const wxString rotuloExecutarArquivo = "Executar arquivo atual";

    struct CampoRuntime
    {
        wxString chave;
        wxString rotulo;
        wxTextCtrl* controle;
    };

    class PainelConfiguracaoLinguagensDL : public cbConfigurationPanel
    {
        public:
            explicit PainelConfiguracaoLinguagensDL(wxWindow* parent)
                : cbConfigurationPanel()
            {
                Create(parent, wxID_ANY);
                ConstruirLayout();
                CarregarValores();
            }

            wxString GetTitle() const override
            {
                return "Design Liquido";
            }

            wxString GetBitmapBaseName() const override
            {
                return "LinguagensDL";
            }

            void OnApply() override
            {
                ConfigManager* configuracoes = Manager::Get()->GetConfigManager("linguagens_dl");
                if (!configuracoes)
                    return;

                for (const CampoRuntime& campo : campos_)
                {
                    wxString valor = campo.controle->GetValue();
                    valor.Trim(true);
                    valor.Trim(false);
                    configuracoes->Write(campo.chave, valor);
                }
            }

            void OnCancel() override
            {
                CarregarValores();
            }

            void OnPageChanging() override
            {
                CarregarValores();
            }

        private:
            std::vector<CampoRuntime> campos_;

            void ConstruirLayout()
            {
                wxBoxSizer* raiz = new wxBoxSizer(wxVERTICAL);
                raiz->Add(
                    new wxStaticText(
                        this,
                        wxID_ANY,
                        "Configure os executaveis de runtime (caminho absoluto ou comando no PATH)."),
                    0,
                    wxALL,
                    8);

                AdicionarCampo(raiz, "delegua.runtime", "Delegua / Pitugues / BIRL / Egua:");
                AdicionarCampo(raiz, "potigol.runtime", "Potigol:");
                AdicionarCampo(raiz, "mapler.runtime", "Portugol Mapler:");
                AdicionarCampo(raiz, "portugol.runtime", "Portugol Studio:");
                AdicionarCampo(raiz, "visualg.runtime", "VisuAlg:");

                SetSizerAndFit(raiz);
            }

            void AdicionarCampo(wxBoxSizer* raiz, const wxString& chave, const wxString& rotulo)
            {
                wxBoxSizer* linha = new wxBoxSizer(wxHORIZONTAL);
                linha->Add(new wxStaticText(this, wxID_ANY, rotulo), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

                wxTextCtrl* controle = new wxTextCtrl(this, wxID_ANY);
                linha->Add(controle, 1, wxEXPAND);

                raiz->Add(linha, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
                campos_.push_back({ chave, rotulo, controle });
            }

            void CarregarValores()
            {
                ConfigManager* configuracoes = Manager::Get()->GetConfigManager("linguagens_dl");
                if (!configuracoes)
                    return;

                for (const CampoRuntime& campo : campos_)
                    campo.controle->SetValue(configuracoes->Read(campo.chave, wxEmptyString));
            }
    };
}

BEGIN_EVENT_TABLE(LinguagensDLPlugin, cbPlugin)
    EVT_MENU(idMenuExecutarArquivoAtual, LinguagensDLPlugin::AoExecutarArquivoMenu)
END_EVENT_TABLE()

LinguagensDLPlugin::LinguagensDLPlugin()
    : gerenciador_linguagens_(nullptr)
    , executor_(nullptr)
    , logger_saida_(nullptr)
    , indice_logger_saida_(LogManager::invalid_log)
{
    // Recursos XRC ainda são opcionais enquanto o plugin não empacota um ZIP próprio.
    wxString arquivoRecursos = ConfigManager::LocateDataFile("LinguagensDL.zip", sdDataGlobal | sdDataUser);
    if (!arquivoRecursos.IsEmpty())
    {
        if (!Manager::LoadResource("LinguagensDL.zip"))
            NotifyMissingFile("LinguagensDL.zip");
    }
}

LinguagensDLPlugin::~LinguagensDLPlugin()
{
}

void LinguagensDLPlugin::OnAttach()
{
    GarantirLoggerSaida();

    gerenciador_linguagens_ = new GerenciadorLinguagens();
    executor_               = new Executor(indice_logger_saida_);

    // Registra extenções de arquivo para todas as linguagens suportadas
    gerenciador_linguagens_->RegistrarTodasExtensoesArquivo();

    // Conecta ao evento de abertura de editor para aplicar o lexer correto
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_OPEN, new cbEventFunctor<LinguagensDLPlugin, CodeBlocksEvent>(this, &LinguagensDLPlugin::AoAbrirEditor));

    // Aplica realce imediatamente no arquivo ativo (se houver)
    cbEditor* editorAtivo = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (editorAtivo)
    {
        gerenciador_linguagens_->AplicarRealce(editorAtivo, editorAtivo->GetFilename());
    }
}

void LinguagensDLPlugin::OnRelease(bool appShutDown)
{
    if (!appShutDown)
        LiberarLoggerSaida();
    else
    {
        logger_saida_ = nullptr;
        indice_logger_saida_ = LogManager::invalid_log;
    }

    delete executor_;
    executor_ = nullptr;

    delete gerenciador_linguagens_;
    gerenciador_linguagens_ = nullptr;
}

void LinguagensDLPlugin::GarantirLoggerSaida()
{
    if (logger_saida_)
        return;

    logger_saida_ = new TextCtrlLogger(true);
    CodeBlocksLogEvent eventoAdicionar(cbEVT_ADD_LOG_WINDOW, logger_saida_, "Linguagens DL");
    Manager::Get()->ProcessEvent(eventoAdicionar);
    indice_logger_saida_ = eventoAdicionar.logIndex;
}

void LinguagensDLPlugin::LiberarLoggerSaida()
{
    if (!logger_saida_)
        return;

    CodeBlocksLogEvent eventoRemover(cbEVT_REMOVE_LOG_WINDOW, logger_saida_);
    Manager::Get()->ProcessEvent(eventoRemover);

    logger_saida_ = nullptr;
    indice_logger_saida_ = LogManager::invalid_log;
}

void LinguagensDLPlugin::BuildMenu(wxMenuBar* menuBar)
{
    if (!menuBar)
        return;

    const int posicaoMenu = menuBar->FindMenu(rotuloMenuPlugin);
    wxMenu* menuPlugin = nullptr;

    if (posicaoMenu == wxNOT_FOUND)
    {
        menuPlugin = new wxMenu();
        menuBar->Append(menuPlugin, rotuloMenuPlugin);
    }
    else
    {
        menuPlugin = menuBar->GetMenu(posicaoMenu);
    }

    if (!menuPlugin)
        return;

    if (!menuPlugin->FindItem(idMenuExecutarArquivoAtual))
        menuPlugin->Append(idMenuExecutarArquivoAtual, rotuloExecutarArquivo);
}

void LinguagensDLPlugin::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* /*data*/)
{
    if (type != mtEditorManager || !menu)
        return;

    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
        return;

    if (!menu->FindItem(idMenuExecutarArquivoAtual))
    {
        menu->AppendSeparator();
        menu->Append(idMenuExecutarArquivoAtual, rotuloExecutarArquivo);
    }
}

cbConfigurationPanel* LinguagensDLPlugin::GetConfigurationPanel(wxWindow* parent)
{
    return new PainelConfiguracaoLinguagensDL(parent);
}

void LinguagensDLPlugin::AoAbrirEditor(CodeBlocksEvent& evento)
{
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinEditor(evento.GetEditor());
    if (!editor) return;

    wxString nomeArquivo = editor->GetFilename();
    gerenciador_linguagens_->AplicarRealce(editor, nomeArquivo);

    evento.Skip();
}

void LinguagensDLPlugin::AoExecutarArquivoMenu(wxCommandEvent& evento)
{
    (void)evento;

    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
    {
        Manager::Get()->GetLogManager()->LogWarning("LinguagensDL: nao ha editor ativo para executar.");
        return;
    }

    if (editor->GetModified())
    {
        Manager::Get()->GetLogManager()->LogWarning(
            "LinguagensDL: salve o arquivo antes de executar para evitar divergencia entre editor e runtime.");
        return;
    }

    executor_->ExecutarArquivo(editor->GetFilename());
}
