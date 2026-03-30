#include "plugin.h"
#include "manager.h"
#include "runner.h"

#include <sdk.h>
#include <cbeditor.h>
#include <editormanager.h>
#include <logmanager.h>
#include <manager.h>

// Registra o plugin no Code::Blocks
namespace
{
    PluginRegistrant<LinguagensDLPlugin> reg("LinguagensDL");
}

BEGIN_EVENT_TABLE(LinguagensDLPlugin, cbPlugin)
END_EVENT_TABLE()

LinguagensDLPlugin::LinguagensDLPlugin()
    : gerenciador_linguagens_(nullptr)
    , executor_(nullptr)
{
    // Carrega as configurações padrão do plugin
    if (!Manager::LoadResource("LinguagensDL.zip"))
    {
        NotifyMissingFile("LinguagensDL.zip");
    }
}

LinguagensDLPlugin::~LinguagensDLPlugin()
{
}

void LinguagensDLPlugin::OnAttach()
{
    gerenciador_linguagens_ = new GerenciadorLinguagens();
    executor_               = new Executor();

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

void LinguagensDLPlugin::OnRelease(bool /*appShutDown*/)
{
    delete executor_;
    executor_ = nullptr;

    delete gerenciador_linguagens_;
    gerenciador_linguagens_ = nullptr;
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
    if (!editor) return;

    executor_->ExecutarArquivo(editor->GetFilename());
}
