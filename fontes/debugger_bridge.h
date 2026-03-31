#pragma once

#include <wx/arrstr.h>
#include <wx/process.h>
#include <wx/string.h>

class LogManager;

class PonteDepurador
{
    public:
        explicit PonteDepurador(int indiceLogSaida = -1);
        ~PonteDepurador();

        bool IniciarSessao(const wxString& adaptadorDAP,
                           const wxString& caminhoPrograma,
                           const wxString& argumentosPrograma = wxEmptyString);
        void EncerrarSessao();

        void DefinirBreakpoint(const wxString& caminhoArquivo, int linha);
        void ContinuarExecucao();
        void PassoSobre();
        void PassoDentro();
        void PassoFora();
        bool ColetarVariaveisAtuais(wxArrayString& variaveis);

        bool SessaoAtiva() const;

    private:
        class ProcessoDAP : public wxProcess
        {
            public:
                explicit ProcessoDAP(PonteDepurador* owner);
                void OnTerminate(int pid, int status) override;

            private:
                PonteDepurador* owner_;
        };

        void RegistrarMensagem(const wxString& mensagem) const;
        void RegistrarErro(const wxString& mensagem) const;
        void NotificarTerminoProcesso(ProcessoDAP* processo, int status);
        bool EnviarRequisicaoEAguardarResposta(const wxString& comando,
                               const wxString& argumentosJSON,
                               wxString& resposta,
                               int timeoutMs = 1000);
        bool AguardarRespostaParaSequencia(int sequencia, wxString& resposta, int timeoutMs);
        bool LerMensagemDAP(wxString& payload, int timeoutMs);
        bool ExtrairPrimeiroInteiro(const wxString& payload, const wxString& chave, int& valor) const;
        void ExtrairVariaveis(const wxString& payload, wxArrayString& variaveis) const;
        void EnviarMensagemDAP(const wxString& payloadJSON);
        int ProximoIdRequisicao();
        wxString EscapeJSON(const wxString& texto) const;
        wxString ConstruirComandoAdaptador(const wxString& adaptadorDAP) const;

        ProcessoDAP* processo_dap_;
        long pid_processo_;
        int proximo_id_requisicao_;
        int indice_log_saida_;
        wxString buffer_entrada_dap_;
};
