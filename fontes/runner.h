#pragma once

#include <wx/string.h>
#include <logger.h>

/**
 * Responsável por executar arquivos com o runtime adequado para cada linguagem.
 *
 * O caminho de cada runtime é lido da configuração do plugin
 * (Settings > Environment > Design Líquido).
 */
class Executor
{
    public:
        explicit Executor(int indiceLogSaida = -1);
        ~Executor() = default;

        void DefinirIndiceLogSaida(int indiceLogSaida);

        /**
         * Executa o arquivo indicado com o runtime correto.
         * A saída (stdout/stderr) é redirecionada para o log do Code::Blocks.
         */
        void ExecutarArquivo(const wxString& caminhoArquivo);

    private:
        void RegistrarMensagem(const wxString& mensagem, Logger::level nivel = Logger::info) const;

        int indice_log_saida_;

        bool ArquivoPossuiExecucaoDireta(const wxString& extensao) const;
        wxString ObterRuntimeParaArquivo(const wxString& caminhoArquivo) const;
        wxString ObterRuntimeConfigurado(const wxString& chave) const;
        wxString ObterRuntimePadrao(const wxString& chave) const;
        wxString ObterChaveRuntimePorExtensao(const wxString& extensao) const;
        wxString ObterArgumentosRuntime(const wxString& chaveRuntime) const;
        wxString ObterArgumentosPrograma(const wxString& extensao) const;
        wxString ResolverExecutavel(const wxString& runtime) const;
};
