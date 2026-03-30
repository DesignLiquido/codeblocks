#pragma once

#include <cbplugin.h>

class GerenciadorLinguagens;
class Executor;
class TextCtrlLogger;
class wxMenu;
class wxMenuBar;

/**
 * Plugin principal para suporte às linguagens da Design Líquido no Code::Blocks.
 *
 * Registra extensões de arquivo, aplica realce de sintaxe e integra runtimes
 * para execução e depuração das linguagens Delégua, LMHT, FolEs, LinConEs e
 * dialetos de Portugol.
 */
class LinguagensDLPlugin : public cbPlugin
{
    public:
        LinguagensDLPlugin();
        virtual ~LinguagensDLPlugin();

        /** Retorna o painel de configurações do plugin (para Settings > Environment). */
        virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent) override;
        virtual void BuildMenu(wxMenuBar* menuBar) override;
        virtual void BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data = nullptr) override;

    protected:
        virtual void OnAttach() override;
        virtual void OnRelease(bool appShutDown) override;

    private:
        GerenciadorLinguagens* gerenciador_linguagens_;
        Executor*              executor_;
        TextCtrlLogger*        logger_saida_;
        int                    indice_logger_saida_;

        void GarantirLoggerSaida();
        void LiberarLoggerSaida();

        void AoAbrirEditor(CodeBlocksEvent& evento);
        void AoExecutarArquivoMenu(wxCommandEvent& evento);

        DECLARE_EVENT_TABLE()
};
