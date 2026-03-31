#include "plugin.h"
#include "completion.h"
#include "debugger_bridge.h"
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

#include <cbstyledtextctrl.h>
#include <wx/arrstr.h>
#include <wx/dirdlg.h>
#include <wx/filefn.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choicdlg.h>
#include <wx/utils.h>

#include <vector>

// Registra o plugin no Code::Blocks
namespace
{
    PluginRegistrant<LinguagensDLPlugin> reg("LinguagensDL");

    const int idMenuExecutarArquivoAtual = wxNewId();
    const int idMenuNovoProjeto = wxNewId();
    const int idMenuFormatarArquivoAtual = wxNewId();
    const int idMenuDepurarArquivoAtual = wxNewId();
    const int idMenuContinuarDepuracao = wxNewId();
    const int idMenuPassoSobre = wxNewId();
    const int idMenuPassoDentro = wxNewId();
    const int idMenuPassoFora = wxNewId();
    const int idMenuPararDepuracao = wxNewId();
    const int idMenuAtualizarWatch = wxNewId();
    const wxString rotuloMenuPlugin = "Design &Liquido";
    const wxString rotuloNovoProjeto = "Novo projeto Delégua...";
    const wxString rotuloFormatarArquivo = "Formatar arquivo atual";
    const wxString rotuloExecutarArquivo = "Executar arquivo atual";
    const wxString rotuloDepurarArquivo = "Depurar arquivo atual (experimental)";
    const wxString rotuloContinuarDepuracao = "Depurador: continuar";
    const wxString rotuloPassoSobre = "Depurador: passo sobre";
    const wxString rotuloPassoDentro = "Depurador: passo dentro";
    const wxString rotuloPassoFora = "Depurador: passo fora";
    const wxString rotuloPararDepuracao = "Depurador: parar";
    const wxString rotuloAtualizarWatch = "Atualizar watch (experimental)";

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
                AdicionarCampo(raiz, "formatter.command", "Formatador ({file}):");

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
    EVT_MENU(idMenuNovoProjeto, LinguagensDLPlugin::AoNovoProjetoMenu)
    EVT_MENU(idMenuFormatarArquivoAtual, LinguagensDLPlugin::AoFormatarArquivoMenu)
    EVT_MENU(idMenuExecutarArquivoAtual, LinguagensDLPlugin::AoExecutarArquivoMenu)
    EVT_MENU(idMenuDepurarArquivoAtual, LinguagensDLPlugin::AoDepurarArquivoMenu)
    EVT_MENU(idMenuContinuarDepuracao, LinguagensDLPlugin::AoContinuarDepuracaoMenu)
    EVT_MENU(idMenuPassoSobre, LinguagensDLPlugin::AoPassoSobreMenu)
    EVT_MENU(idMenuPassoDentro, LinguagensDLPlugin::AoPassoDentroMenu)
    EVT_MENU(idMenuPassoFora, LinguagensDLPlugin::AoPassoForaMenu)
    EVT_MENU(idMenuPararDepuracao, LinguagensDLPlugin::AoPararDepuracaoMenu)
    EVT_MENU(idMenuAtualizarWatch, LinguagensDLPlugin::AoAtualizarWatchMenu)
END_EVENT_TABLE()

LinguagensDLPlugin::LinguagensDLPlugin()
    : gerenciador_linguagens_(nullptr)
    , provedor_completude_(nullptr)
    , ponte_depurador_(nullptr)
    , executor_(nullptr)
    , logger_saida_(nullptr)
    , indice_logger_saida_(LogManager::invalid_log)
    , logger_watch_(nullptr)
    , indice_logger_watch_(LogManager::invalid_log)
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
    GarantirLoggerWatch();

    gerenciador_linguagens_ = new GerenciadorLinguagens();
    provedor_completude_    = new ProvedorCompletude(gerenciador_linguagens_);
    ponte_depurador_        = new PonteDepurador(indice_logger_saida_);
    executor_               = new Executor(indice_logger_saida_);

    // Registra extenções de arquivo para todas as linguagens suportadas
    gerenciador_linguagens_->RegistrarTodasExtensoesArquivo();

    // Conecta ao evento de abertura de editor para aplicar o lexer correto
    Manager::Get()->RegisterEventSink(cbEVT_EDITOR_OPEN, new cbEventFunctor<LinguagensDLPlugin, CodeBlocksEvent>(this, &LinguagensDLPlugin::AoAbrirEditor));

    // Aplica realce imediatamente no arquivo ativo (se houver)
    cbEditor* editorAtivo = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (editorAtivo)
    {
        ConfigurarEditorParaCompletude(editorAtivo);
        gerenciador_linguagens_->AplicarRealce(editorAtivo, editorAtivo->GetFilename());
    }
}

void LinguagensDLPlugin::OnRelease(bool appShutDown)
{
    LimparMarcadoresDepuracao();

    if (!appShutDown)
    {
        LiberarLoggerWatch();
        LiberarLoggerSaida();
    }
    else
    {
        logger_watch_ = nullptr;
        indice_logger_watch_ = LogManager::invalid_log;
        logger_saida_ = nullptr;
        indice_logger_saida_ = LogManager::invalid_log;
    }

    delete executor_;
    executor_ = nullptr;

    delete ponte_depurador_;
    ponte_depurador_ = nullptr;

    RemoverGanchosCompletude();

    delete provedor_completude_;
    provedor_completude_ = nullptr;

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

void LinguagensDLPlugin::GarantirLoggerWatch()
{
    if (logger_watch_)
        return;

    logger_watch_ = new TextCtrlLogger(true);
    CodeBlocksLogEvent eventoAdicionar(cbEVT_ADD_LOG_WINDOW, logger_watch_, "Linguagens DL - Watch");
    Manager::Get()->ProcessEvent(eventoAdicionar);
    indice_logger_watch_ = eventoAdicionar.logIndex;
}

void LinguagensDLPlugin::LiberarLoggerWatch()
{
    if (!logger_watch_)
        return;

    CodeBlocksLogEvent eventoRemover(cbEVT_REMOVE_LOG_WINDOW, logger_watch_);
    Manager::Get()->ProcessEvent(eventoRemover);

    logger_watch_ = nullptr;
    indice_logger_watch_ = LogManager::invalid_log;
}

void LinguagensDLPlugin::AtualizarPainelWatch()
{
    if (!ponte_depurador_ || !ponte_depurador_->SessaoAtiva())
    {
        Manager::Get()->GetLogManager()->LogWarning("LinguagensDL: inicie uma sessao de depuracao para atualizar o watch.");
        return;
    }

    wxArrayString variaveis;
    const bool ok = ponte_depurador_->ColetarVariaveisAtuais(variaveis);

    LogManager* logs = Manager::Get()->GetLogManager();
    if (!logs)
        return;

    if (indice_logger_watch_ == LogManager::invalid_log)
        GarantirLoggerWatch();

    if (!ok)
    {
        logs->Log("LinguagensDL: nenhuma variavel disponivel no frame atual.", indice_logger_watch_, Logger::warning);
        return;
    }

    logs->Log("LinguagensDL: variaveis atuais", indice_logger_watch_);
    for (const wxString& linha : variaveis)
        logs->Log("  " + linha, indice_logger_watch_);
}

void LinguagensDLPlugin::MarcarLinhaAtualDepuracao(cbEditor* editor)
{
    if (!editor || !editor->GetControl())
        return;

    const int linhaAtual = editor->GetControl()->GetCurrentLine();
    editor->AddBreakpoint(linhaAtual, false);
    editor->SetDebugLine(linhaAtual);
    editor->RefreshBreakpointMarkers();
}

void LinguagensDLPlugin::LimparMarcadoresDepuracao()
{
    EditorManager* gerenciadorEditores = Manager::Get()->GetEditorManager();
    if (!gerenciadorEditores)
        return;

    for (int i = 0; i < gerenciadorEditores->GetEditorsCount(); ++i)
    {
        cbEditor* editor = gerenciadorEditores->GetBuiltinEditor(i);
        if (!editor)
            continue;

        editor->SetDebugLine(-1);
        editor->RefreshBreakpointMarkers();
    }
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
    {
        menuPlugin->Append(idMenuNovoProjeto, rotuloNovoProjeto);
        menuPlugin->AppendSeparator();
        menuPlugin->Append(idMenuFormatarArquivoAtual, rotuloFormatarArquivo);
        menuPlugin->Append(idMenuExecutarArquivoAtual, rotuloExecutarArquivo);
    }

    if (!menuPlugin->FindItem(idMenuDepurarArquivoAtual))
        menuPlugin->Append(idMenuDepurarArquivoAtual, rotuloDepurarArquivo);

    if (!menuPlugin->FindItem(idMenuContinuarDepuracao))
        menuPlugin->Append(idMenuContinuarDepuracao, rotuloContinuarDepuracao);

    if (!menuPlugin->FindItem(idMenuPassoSobre))
        menuPlugin->Append(idMenuPassoSobre, rotuloPassoSobre);

    if (!menuPlugin->FindItem(idMenuPassoDentro))
        menuPlugin->Append(idMenuPassoDentro, rotuloPassoDentro);

    if (!menuPlugin->FindItem(idMenuPassoFora))
        menuPlugin->Append(idMenuPassoFora, rotuloPassoFora);

    if (!menuPlugin->FindItem(idMenuPararDepuracao))
        menuPlugin->Append(idMenuPararDepuracao, rotuloPararDepuracao);

    if (!menuPlugin->FindItem(idMenuAtualizarWatch))
        menuPlugin->Append(idMenuAtualizarWatch, rotuloAtualizarWatch);
}

void LinguagensDLPlugin::ConfigurarEditorParaCompletude(cbEditor* editor)
{
    if (!editor || !editor->GetControl())
        return;

    editor->GetControl()->Connect(
        wxEVT_SCI_CHARADDED,
        wxScintillaEventHandler(LinguagensDLPlugin::AoCaractereAdicionado),
        nullptr,
        this);
}

void LinguagensDLPlugin::RemoverGanchosCompletude()
{
    EditorManager* gerenciadorEditores = Manager::Get()->GetEditorManager();
    if (!gerenciadorEditores)
        return;

    for (int i = 0; i < gerenciadorEditores->GetEditorsCount(); ++i)
    {
        cbEditor* editor = gerenciadorEditores->GetBuiltinEditor(i);
        if (!editor || !editor->GetControl())
            continue;

        editor->GetControl()->Disconnect(
            wxEVT_SCI_CHARADDED,
            wxScintillaEventHandler(LinguagensDLPlugin::AoCaractereAdicionado),
            nullptr,
            this);
    }
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
        menu->Append(idMenuNovoProjeto, rotuloNovoProjeto);
        menu->Append(idMenuFormatarArquivoAtual, rotuloFormatarArquivo);
        menu->Append(idMenuExecutarArquivoAtual, rotuloExecutarArquivo);
        menu->Append(idMenuDepurarArquivoAtual, rotuloDepurarArquivo);
        menu->Append(idMenuContinuarDepuracao, rotuloContinuarDepuracao);
        menu->Append(idMenuPassoSobre, rotuloPassoSobre);
        menu->Append(idMenuPassoDentro, rotuloPassoDentro);
        menu->Append(idMenuPassoFora, rotuloPassoFora);
        menu->Append(idMenuPararDepuracao, rotuloPararDepuracao);
        menu->Append(idMenuAtualizarWatch, rotuloAtualizarWatch);
    }
}

cbConfigurationPanel* LinguagensDLPlugin::GetConfigurationPanel(wxWindow* parent)
{
    return new PainelConfiguracaoLinguagensDL(parent);
}

void LinguagensDLPlugin::AoNovoProjetoMenu(wxCommandEvent& evento)
{
    (void)evento;

    wxWindow* pai = Manager::Get()->GetAppWindow();

    wxDirDialog seletorDiretorio(
        pai,
        "Escolha a pasta onde o novo projeto sera criado",
        wxGetCwd(),
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (seletorDiretorio.ShowModal() != wxID_OK)
        return;

    wxTextEntryDialog entradaNome(
        pai,
        "Nome do projeto:",
        "Novo projeto Delégua",
        "meu-projeto-delegua");

    if (entradaNome.ShowModal() != wxID_OK)
        return;

    wxString nomeProjeto = entradaNome.GetValue();
    nomeProjeto.Trim(true);
    nomeProjeto.Trim(false);
    if (nomeProjeto.IsEmpty())
    {
        wxMessageBox("Nome do projeto invalido.", "LinguagensDL", wxOK | wxICON_WARNING, pai);
        return;
    }

    wxArrayString linguagens;
    linguagens.Add("Delegua");
    linguagens.Add("Pitugues");
    linguagens.Add("Potigol");

    wxSingleChoiceDialog escolhaLinguagem(
        pai,
        "Escolha a linguagem inicial do projeto:",
        "Template do projeto",
        linguagens);
    escolhaLinguagem.SetSelection(0);

    if (escolhaLinguagem.ShowModal() != wxID_OK)
        return;

    const wxString linguagem = escolhaLinguagem.GetStringSelection();
    wxString extensao = "delegua";
    wxString conteudo = "funcao inicio() {\n    escreva('Ola, Design Liquido!');\n}\n\ninicio();\n";

    if (linguagem == "Pitugues")
    {
        extensao = "pitugues";
        conteudo = "funcao inicio() {\n    escreva('Ola, Pitugues!');\n}\n\ninicio();\n";
    }
    else if (linguagem == "Potigol")
    {
        extensao = "potigol";
        conteudo = "inicio\n  escreva(\"Ola, Potigol!\")\nfim\n";
    }

    wxFileName pastaProjeto(seletorDiretorio.GetPath(), nomeProjeto);
    if (!wxFileName::Mkdir(pastaProjeto.GetFullPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
    {
        wxMessageBox("Nao foi possivel criar a pasta do projeto.", "LinguagensDL", wxOK | wxICON_ERROR, pai);
        return;
    }

    wxFileName arquivoPrincipal(pastaProjeto.GetFullPath(), wxString::Format("principal.%s", extensao));
    wxFFile saidaPrincipal(arquivoPrincipal.GetFullPath(), "w");
    if (!saidaPrincipal.IsOpened())
    {
        wxMessageBox("Nao foi possivel criar o arquivo principal.", "LinguagensDL", wxOK | wxICON_ERROR, pai);
        return;
    }
    saidaPrincipal.Write(conteudo);
    saidaPrincipal.Close();

    wxFileName arquivoReadme(pastaProjeto.GetFullPath(), "README.md");
    wxFFile saidaReadme(arquivoReadme.GetFullPath(), "w");
    if (saidaReadme.IsOpened())
    {
        saidaReadme.Write(wxString::Format(
            "# %s\n\nProjeto criado pelo plugin LinguagensDL para Code::Blocks.\n\nLinguagem inicial: %s\n",
            nomeProjeto,
            linguagem));
        saidaReadme.Close();
    }

    wxFileName arquivoGitignore(pastaProjeto.GetFullPath(), ".gitignore");
    wxFFile saidaGitignore(arquivoGitignore.GetFullPath(), "w");
    if (saidaGitignore.IsOpened())
    {
        saidaGitignore.Write("build/\n*.log\n");
        saidaGitignore.Close();
    }

    Manager::Get()->GetEditorManager()->Open(arquivoPrincipal.GetFullPath());
    Manager::Get()->GetLogManager()->Log(wxString::Format(
        "LinguagensDL: projeto criado em %s",
        pastaProjeto.GetFullPath()));
}

void LinguagensDLPlugin::AoAbrirEditor(CodeBlocksEvent& evento)
{
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinEditor(evento.GetEditor());
    if (!editor) return;

    ConfigurarEditorParaCompletude(editor);

    wxString nomeArquivo = editor->GetFilename();
    gerenciador_linguagens_->AplicarRealce(editor, nomeArquivo);

    evento.Skip();
}

void LinguagensDLPlugin::AoCaractereAdicionado(wxScintillaEvent& evento)
{
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor || editor->GetControl() != evento.GetEventObject())
    {
        evento.Skip();
        return;
    }

    if (provedor_completude_)
        provedor_completude_->TalvezExibirCompletude(editor, evento.GetKey());

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

void LinguagensDLPlugin::AoFormatarArquivoMenu(wxCommandEvent& evento)
{
    (void)evento;

    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
    {
        Manager::Get()->GetLogManager()->LogWarning("LinguagensDL: nao ha editor ativo para formatar.");
        return;
    }

    if (editor->GetModified() && !editor->Save())
    {
        Manager::Get()->GetLogManager()->LogWarning("LinguagensDL: nao foi possivel salvar o arquivo antes da formatacao.");
        return;
    }

    const wxString caminhoArquivo = editor->GetFilename();
    ConfigManager* cfg = Manager::Get()->GetConfigManager("linguagens_dl");
    wxString comando = cfg
        ? cfg->Read("formatter.command", wxString("delegua format \"{file}\""))
        : wxString("delegua format \"{file}\"");

    comando.Trim(true);
    comando.Trim(false);
    if (comando.IsEmpty())
    {
        Manager::Get()->GetLogManager()->LogWarning(
            "LinguagensDL: configure 'formatter.command' nas configuracoes do plugin.");
        return;
    }

    if (comando.Find("{file}") != wxNOT_FOUND)
        comando.Replace("{file}", wxString::Format("\"%s\"", caminhoArquivo));
    else
        comando.Append(wxString::Format(" \"%s\"", caminhoArquivo));

    wxArrayString saida;
    wxArrayString erros;
    const long codigo = wxExecute(comando, saida, erros);

    LogManager* logs = Manager::Get()->GetLogManager();
    if (!logs)
        return;

    for (const wxString& linha : saida)
        logs->Log(linha, indice_logger_saida_);
    for (const wxString& linha : erros)
        logs->Log(linha, indice_logger_saida_, Logger::warning);

    if (codigo != 0)
    {
        logs->Log(
            wxString::Format("LinguagensDL: formatador retornou codigo %ld.", codigo),
            indice_logger_saida_,
            Logger::error);
        return;
    }

    editor->Reload();
    logs->Log("LinguagensDL: formatacao concluida.", indice_logger_saida_);
}

void LinguagensDLPlugin::AoDepurarArquivoMenu(wxCommandEvent& evento)
{
    (void)evento;

    if (!ponte_depurador_)
        return;

    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    if (!editor)
    {
        Manager::Get()->GetLogManager()->LogWarning("LinguagensDL: nao ha editor ativo para depurar.");
        return;
    }

    wxString arquivo = editor->GetFilename();
    ConfigManager* cfg = Manager::Get()->GetConfigManager("linguagens_dl");
    wxString adaptador = cfg ? cfg->Read("debugger.adapter", wxString("delegua-dap")) : wxString("delegua-dap");
    wxString args = cfg ? cfg->Read("debugger.program_args", wxString(wxEmptyString)) : wxString(wxEmptyString);

    if (!ponte_depurador_->IniciarSessao(adaptador, arquivo, args))
        return;

    MarcarLinhaAtualDepuracao(editor);
    ponte_depurador_->DefinirBreakpoint(arquivo, editor->GetControl()->GetCurrentLine() + 1);
    ponte_depurador_->ContinuarExecucao();
    AtualizarPainelWatch();
}

void LinguagensDLPlugin::AoAtualizarWatchMenu(wxCommandEvent& evento)
{
    (void)evento;
    AtualizarPainelWatch();
}

void LinguagensDLPlugin::AoContinuarDepuracaoMenu(wxCommandEvent& evento)
{
    (void)evento;
    if (!ponte_depurador_ || !ponte_depurador_->SessaoAtiva())
        return;

    ponte_depurador_->ContinuarExecucao();
}

void LinguagensDLPlugin::AoPassoSobreMenu(wxCommandEvent& evento)
{
    (void)evento;
    if (!ponte_depurador_ || !ponte_depurador_->SessaoAtiva())
        return;

    ponte_depurador_->PassoSobre();
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    MarcarLinhaAtualDepuracao(editor);
    AtualizarPainelWatch();
}

void LinguagensDLPlugin::AoPassoDentroMenu(wxCommandEvent& evento)
{
    (void)evento;
    if (!ponte_depurador_ || !ponte_depurador_->SessaoAtiva())
        return;

    ponte_depurador_->PassoDentro();
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    MarcarLinhaAtualDepuracao(editor);
    AtualizarPainelWatch();
}

void LinguagensDLPlugin::AoPassoForaMenu(wxCommandEvent& evento)
{
    (void)evento;
    if (!ponte_depurador_ || !ponte_depurador_->SessaoAtiva())
        return;

    ponte_depurador_->PassoFora();
    cbEditor* editor = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();
    MarcarLinhaAtualDepuracao(editor);
    AtualizarPainelWatch();
}

void LinguagensDLPlugin::AoPararDepuracaoMenu(wxCommandEvent& evento)
{
    (void)evento;
    if (!ponte_depurador_ || !ponte_depurador_->SessaoAtiva())
        return;

    ponte_depurador_->EncerrarSessao();
    LimparMarcadoresDepuracao();
}
