#pragma once

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
        void EnviarMensagemDAP(const wxString& payloadJSON);
        int ProximoIdRequisicao();
        wxString EscapeJSON(const wxString& texto) const;
        wxString ConstruirComandoAdaptador(const wxString& adaptadorDAP) const;

        ProcessoDAP* processo_dap_;
        long pid_processo_;
        int proximo_id_requisicao_;
        int indice_log_saida_;
};
