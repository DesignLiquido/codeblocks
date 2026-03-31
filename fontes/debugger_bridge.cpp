#include "debugger_bridge.h"

#include <sdk.h>
#include <configmanager.h>
#include <logmanager.h>
#include <manager.h>

#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/utils.h>

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
    , buffer_entrada_dap_()
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

bool PonteDepurador::ColetarVariaveisAtuais(wxArrayString& variaveis)
{
    variaveis.Clear();

    if (!SessaoAtiva())
        return false;

    wxString respostaThreads;
    if (!EnviarRequisicaoEAguardarResposta("threads", "{}", respostaThreads))
        return false;

    int threadId = 0;
    if (!ExtrairPrimeiroInteiro(respostaThreads, "threadId", threadId) || threadId <= 0)
        return false;

    wxString respostaStack;
    if (!EnviarRequisicaoEAguardarResposta(
            "stackTrace",
            wxString::Format("{\"threadId\":%d,\"startFrame\":0,\"levels\":1}", threadId),
            respostaStack))
        return false;

    int frameId = 0;
    if (!ExtrairPrimeiroInteiro(respostaStack, "id", frameId))
        return false;

    wxString respostaScopes;
    if (!EnviarRequisicaoEAguardarResposta(
            "scopes",
            wxString::Format("{\"frameId\":%d}", frameId),
            respostaScopes))
        return false;

    int variablesReference = 0;
    if (!ExtrairPrimeiroInteiro(respostaScopes, "variablesReference", variablesReference)
        || variablesReference <= 0)
        return false;

    wxString respostaVariaveis;
    if (!EnviarRequisicaoEAguardarResposta(
            "variables",
            wxString::Format("{\"variablesReference\":%d}", variablesReference),
            respostaVariaveis))
        return false;

    ExtrairVariaveis(respostaVariaveis, variaveis);
    return !variaveis.IsEmpty();
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
        buffer_entrada_dap_.clear();
    }
}

bool PonteDepurador::EnviarRequisicaoEAguardarResposta(const wxString& comando,
                                                       const wxString& argumentosJSON,
                                                       wxString& resposta,
                                                       int timeoutMs)
{
    if (!SessaoAtiva())
        return false;

    const int sequencia = ProximoIdRequisicao();
    EnviarMensagemDAP(wxString::Format(
        "{\"seq\":%d,\"type\":\"request\",\"command\":\"%s\",\"arguments\":%s}",
        sequencia,
        comando,
        argumentosJSON));

    return AguardarRespostaParaSequencia(sequencia, resposta, timeoutMs);
}

bool PonteDepurador::AguardarRespostaParaSequencia(int sequencia, wxString& resposta, int timeoutMs)
{
    resposta.clear();
    const long inicio = wxGetUTCTimeMillis().GetValue();

    while ((wxGetUTCTimeMillis().GetValue() - inicio) < timeoutMs)
    {
        wxString payload;
        if (!LerMensagemDAP(payload, 100))
            continue;

        wxRegEx padraoSequencia(wxString::Format("\"request_seq\"[[:space:]]*:[[:space:]]*%d", sequencia));
        if (!padraoSequencia.IsValid() || !padraoSequencia.Matches(payload))
            continue;

        if (payload.Find("\"type\":\"response\"") == wxNOT_FOUND)
            continue;

        resposta = payload;
        return true;
    }

    return false;
}

bool PonteDepurador::LerMensagemDAP(wxString& payload, int timeoutMs)
{
    payload.clear();

    if (!SessaoAtiva() || !processo_dap_)
        return false;

    wxInputStream* saidaAdaptador = processo_dap_->GetInputStream();
    if (!saidaAdaptador)
        return false;

    const long inicio = wxGetUTCTimeMillis().GetValue();

    while ((wxGetUTCTimeMillis().GetValue() - inicio) < timeoutMs)
    {
        while (saidaAdaptador->CanRead())
        {
            char c = 0;
            saidaAdaptador->Read(&c, 1);
            if (saidaAdaptador->LastRead() == 1)
                buffer_entrada_dap_.Append(c);
            else
                break;
        }

        const int idxSeparador = buffer_entrada_dap_.Find("\r\n\r\n");
        if (idxSeparador != wxNOT_FOUND)
        {
            const wxString cabecalho = buffer_entrada_dap_.substr(0, idxSeparador);
            wxRegEx regexTamanho("Content-Length:[[:space:]]*([0-9]+)", wxRE_ICASE);
            if (!regexTamanho.IsValid() || !regexTamanho.Matches(cabecalho))
            {
                buffer_entrada_dap_.clear();
                return false;
            }

            long tamanhoPayload = 0;
            regexTamanho.GetMatch(cabecalho, 1).ToLong(&tamanhoPayload);
            const int inicioPayload = idxSeparador + 4;

            if ((long)buffer_entrada_dap_.length() >= (inicioPayload + tamanhoPayload))
            {
                payload = buffer_entrada_dap_.substr(inicioPayload, tamanhoPayload);
                buffer_entrada_dap_ = buffer_entrada_dap_.substr(inicioPayload + tamanhoPayload);
                return true;
            }
        }

        wxMilliSleep(10);
    }

    return false;
}

bool PonteDepurador::ExtrairPrimeiroInteiro(const wxString& payload, const wxString& chave, int& valor) const
{
    valor = 0;
    wxRegEx padrao(wxString::Format("\"%s\"[[:space:]]*:[[:space:]]*([0-9]+)", chave));
    if (!padrao.IsValid() || !padrao.Matches(payload))
        return false;

    long extraido = 0;
    if (!padrao.GetMatch(payload, 1).ToLong(&extraido))
        return false;

    valor = (int)extraido;
    return true;
}

void PonteDepurador::ExtrairVariaveis(const wxString& payload, wxArrayString& variaveis) const
{
    variaveis.Clear();

    size_t cursor = 0;
    while (cursor < payload.length())
    {
        const int idxNomeToken = payload.find("\"name\":\"", cursor);
        if (idxNomeToken == wxNOT_FOUND)
            break;

        const size_t inicioNome = (size_t)idxNomeToken + 8;
        const int idxFimNome = payload.find('"', inicioNome);
        if (idxFimNome == wxNOT_FOUND)
            break;

        const wxString nome = payload.substr(inicioNome, (size_t)idxFimNome - inicioNome);

        const int idxValorToken = payload.find("\"value\":\"", (size_t)idxFimNome);
        if (idxValorToken == wxNOT_FOUND)
            break;

        const size_t inicioValor = (size_t)idxValorToken + 9;
        const int idxFimValor = payload.find('"', inicioValor);
        if (idxFimValor == wxNOT_FOUND)
            break;

        const wxString valor = payload.substr(inicioValor, (size_t)idxFimValor - inicioValor);
        variaveis.Add(wxString::Format("%s = %s", nome, valor));

        cursor = (size_t)idxFimValor + 1;
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
