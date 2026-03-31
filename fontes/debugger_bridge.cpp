#include "debugger_bridge.h"

#include <sdk.h>
#include <configmanager.h>
#include <logmanager.h>
#include <manager.h>

#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/sstream.h>

PonteDepurador::ProcessoDAP::ProcessoDAP(PonteDepurador* owner)
    : wxProcess(nullptr)
    , owner_(owner)
{
    Redirect();
}

void PonteDepurador::ProcessoDAP::OnTerminate(int /*pid*/, int status)
{
    if (owner_)
        owner_->NotificarTerminoProcesso(this, status);
}

PonteDepurador::PonteDepurador(int indiceLogSaida)
    : processo_dap_(nullptr)
    , pid_processo_(0)
    , proximo_id_requisicao_(1)
    , indice_log_saida_(indiceLogSaida)
{
}

PonteDepurador::~PonteDepurador()
{
    EncerrarSessao();
}

bool PonteDepurador::IniciarSessao(const wxString& adaptadorDAP,
                                  const wxString& caminhoPrograma,
                                  const wxString& argumentosPrograma)
{
    if (processo_dap_ && pid_processo_ == 0)
    {
        delete processo_dap_;
        processo_dap_ = nullptr;
    }

    if (SessaoAtiva())
    {
        RegistrarMensagem("LinguagensDL: uma sessao de depuracao ja esta ativa.");
        return true;
    }

    wxString comando = ConstruirComandoAdaptador(adaptadorDAP);
    if (comando.IsEmpty())
    {
        RegistrarErro("LinguagensDL: adaptador DAP nao configurado.");
        return false;
    }

    processo_dap_ = new ProcessoDAP(this);
    pid_processo_ = wxExecute(comando, wxEXEC_ASYNC, processo_dap_);
    if (pid_processo_ == 0)
    {
        RegistrarErro(wxString::Format("LinguagensDL: falha ao iniciar adaptador DAP: %s", comando));
        delete processo_dap_;
        processo_dap_ = nullptr;
        return false;
    }

    RegistrarMensagem(wxString::Format("LinguagensDL: depurador iniciado: %s", comando));

    const int idInitialize = ProximoIdRequisicao();
    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"initialize\",\"arguments\":{"
        "\"clientID\":\"codeblocks-linguagens-dl\",\"clientName\":\"Code::Blocks LinguagensDL\","
        "\"adapterID\":\"delegua\",\"pathFormat\":\"path\",\"linesStartAt1\":true,\"columnsStartAt1\":true}}",
        idInitialize));

    const int idLaunch = ProximoIdRequisicao();
    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"launch\",\"arguments\":{"
        "\"program\":\"%s\",\"args\":\"%s\"}}",
        idLaunch,
        EscapeJSON(caminhoPrograma),
        EscapeJSON(argumentosPrograma)));

    return true;
}

void PonteDepurador::EncerrarSessao()
{
    if (!processo_dap_)
    {
        pid_processo_ = 0;
        return;
    }

    if (pid_processo_ != 0)
    {
        EnviarMensagemDAP(wxString::Format(
            "{\"seq\":%d,\"type\":\"request\",\"command\":\"disconnect\",\"arguments\":{\"restart\":false}}",
            ProximoIdRequisicao()));
        wxProcess::Kill(pid_processo_, wxSIGTERM, wxKILL_CHILDREN);
    }

    delete processo_dap_;
    processo_dap_ = nullptr;
    pid_processo_ = 0;
}

void PonteDepurador::DefinirBreakpoint(const wxString& caminhoArquivo, int linha)
{
    if (!SessaoAtiva())
    {
        RegistrarErro("LinguagensDL: nao ha sessao ativa para definir breakpoint.");
        return;
    }

    const int id = ProximoIdRequisicao();
    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"setBreakpoints\",\"arguments\":{"
        "\"source\":{\"path\":\"%s\"},\"breakpoints\":[{\"line\":%d}]}}",
        id,
        EscapeJSON(caminhoArquivo),
        linha));
}

void PonteDepurador::ContinuarExecucao()
{
    if (!SessaoAtiva())
        return;

    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"continue\",\"arguments\":{\"threadId\":1}}",
        ProximoIdRequisicao()));
}

void PonteDepurador::PassoSobre()
{
    if (!SessaoAtiva())
        return;

    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"next\",\"arguments\":{\"threadId\":1}}",
        ProximoIdRequisicao()));
}

void PonteDepurador::PassoDentro()
{
    if (!SessaoAtiva())
        return;

    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"stepIn\",\"arguments\":{\"threadId\":1}}",
        ProximoIdRequisicao()));
}

void PonteDepurador::PassoFora()
{
    if (!SessaoAtiva())
        return;

    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"stepOut\",\"arguments\":{\"threadId\":1}}",
        ProximoIdRequisicao()));
}

bool PonteDepurador::SessaoAtiva() const
{
    return processo_dap_ != nullptr && pid_processo_ != 0;
}

void PonteDepurador::RegistrarMensagem(const wxString& mensagem) const
{
    LogManager* logs = Manager::Get()->GetLogManager();
    if (!logs)
        return;

    if (indice_log_saida_ != LogManager::invalid_log)
        logs->Log(mensagem, indice_log_saida_);
    else
        logs->Log(mensagem);
}

void PonteDepurador::RegistrarErro(const wxString& mensagem) const
{
    LogManager* logs = Manager::Get()->GetLogManager();
    if (!logs)
        return;

    if (indice_log_saida_ != LogManager::invalid_log)
        logs->Log(mensagem, indice_log_saida_, Logger::error);
    else
        logs->LogError(mensagem);
}

void PonteDepurador::NotificarTerminoProcesso(ProcessoDAP* processo, int status)
{
    RegistrarMensagem(wxString::Format("LinguagensDL: sessao de depuracao encerrada (codigo %d)", status));

    if (processo_dap_ == processo)
    {
        processo_dap_ = nullptr;
        pid_processo_ = 0;
    }
}

void PonteDepurador::EnviarMensagemDAP(const wxString& payloadJSON)
{
    if (!SessaoAtiva())
        return;

    wxOutputStream* entradaAdaptador = processo_dap_->GetOutputStream();
    if (!entradaAdaptador)
    {
        RegistrarErro("LinguagensDL: stream de entrada do adaptador DAP indisponivel.");
        return;
    }

    wxCharBuffer utf8 = payloadJSON.utf8_str();
    const size_t tamanhoPayload = strlen(utf8.data());
    wxString cabecalho = wxString::Format("Content-Length: %zu\r\n\r\n", tamanhoPayload);

    wxCharBuffer cabecalhoUtf8 = cabecalho.ToUTF8();
    entradaAdaptador->Write(cabecalhoUtf8, strlen(cabecalhoUtf8));
    entradaAdaptador->Write(utf8.data(), tamanhoPayload);
    entradaAdaptador->Sync();
}

int PonteDepurador::ProximoIdRequisicao()
{
    return proximo_id_requisicao_++;
}

wxString PonteDepurador::EscapeJSON(const wxString& texto) const
{
    wxString escaped = texto;
    escaped.Replace("\\", "\\\\");
    escaped.Replace("\"", "\\\"");
    escaped.Replace("\r", "\\r");
    escaped.Replace("\n", "\\n");
    escaped.Replace("\t", "\\t");
    return escaped;
}

wxString PonteDepurador::ConstruirComandoAdaptador(const wxString& adaptadorDAP) const
{
    wxString valor = adaptadorDAP;
    valor.Trim(true);
    valor.Trim(false);
    return valor;
}
