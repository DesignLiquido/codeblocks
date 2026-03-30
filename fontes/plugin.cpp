#include "plugin.h"
#include "manager.h"
#include "runner.h"

#include <sdk.h>
#include <cbeditor.h>
#include <configmanager.h>
#include <editormanager.h>
#include <logmanager.h>
#include <loggers.h>
#include <manager.h>
#include <sdk_events.h>

#include <wx/menu.h>

// Registra o plugin no Code::Blocks
namespace
{
    PluginRegistrant<LinguagensDLPlugin> reg("LinguagensDL");

    const int idMenuExecutarArquivoAtual = wxNewId();
    const wxString rotuloMenuPlugin = "Design &Liquido";
    const wxString rotuloExecutarArquivo = "Executar arquivo atual";
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
    (void)parent;
    // TODO: implementar painel de configurações (Fase 3)
    return nullptr;
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
