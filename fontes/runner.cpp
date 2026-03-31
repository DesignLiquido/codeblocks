#include "runner.h"

#include <sdk.h>
#include <configmanager.h>
#include <logger.h>
#include <logmanager.h>
#include <manager.h>

#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/utils.h>

// Mapa simplificado de extensão → chave de runtime configurável
static const struct { const wxChar* ext; const wxChar* key; } s_runtimeMap[] =
{
    { wxT("delegua"),  wxT("delegua.runtime")  },
    { wxT("pitugues"), wxT("delegua.runtime")  },
    { wxT("egua"),     wxT("delegua.runtime")  },
    { wxT("birl"),     wxT("delegua.runtime")  },
    { wxT("potigol"),  wxT("potigol.runtime")  },
    { wxT("mapler"),   wxT("mapler.runtime")   },
    { wxT("por"),      wxT("portugol.runtime") },
    { wxT("alg"),      wxT("visualg.runtime")  },
};

namespace
{
    wxString RemoverAspasExternas(const wxString& valor)
    {
        if (valor.Length() >= 2 && valor.StartsWith("\"") && valor.EndsWith("\""))
            return valor.Mid(1, valor.Length() - 2);
        return valor;
    }

    wxString ConstruirComando(const wxString& executavel,
                              const wxString& argumentosRuntime,
                              const wxString& caminhoArquivo,
                              const wxString& argumentosPrograma)
    {
        wxString comando = wxString::Format("\"%s\"", executavel);

        if (!argumentosRuntime.IsEmpty())
            comando << ' ' << argumentosRuntime;

        comando << ' ' << wxString::Format("\"%s\"", caminhoArquivo);

        if (!argumentosPrograma.IsEmpty())
            comando << ' ' << argumentosPrograma;

        return comando;
    }
}

Executor::Executor(int indiceLogSaida)
    : indice_log_saida_(indiceLogSaida)
{
}

void Executor::DefinirIndiceLogSaida(int indiceLogSaida)
{
    indice_log_saida_ = indiceLogSaida;
}

void Executor::RegistrarMensagem(const wxString& mensagem, Logger::level nivel) const
{
    LogManager* gerenciadorLogs = Manager::Get()->GetLogManager();
    if (!gerenciadorLogs)
        return;

    if (indice_log_saida_ != LogManager::invalid_log)
        gerenciadorLogs->Log(mensagem, indice_log_saida_, nivel);
    else
        gerenciadorLogs->Log(mensagem, LogManager::app_log, nivel);
}

void Executor::ExecutarArquivo(const wxString& caminhoArquivo)
{
    wxFileName arquivo(caminhoArquivo);
    if (!arquivo.FileExists())
    {
        RegistrarMensagem(
            wxString::Format("LinguagensDL: arquivo nao encontrado para execucao: %s", caminhoArquivo),
            Logger::error);
        return;
    }

    wxString extensao = arquivo.GetExt().Lower();
    if (!ArquivoPossuiExecucaoDireta(extensao))
    {
        RegistrarMensagem(
            wxString::Format("LinguagensDL: a extensao '.%s' nao possui runtime de execucao direta.", extensao),
            Logger::warning);
        return;
    }

    wxString chaveRuntime = ObterChaveRuntimePorExtensao(extensao);
    wxString runtime = ObterRuntimeParaArquivo(caminhoArquivo);
    if (runtime.IsEmpty())
    {
        RegistrarMensagem(
            wxString::Format("LinguagensDL: nenhum runtime configurado para '%s'", caminhoArquivo),
            Logger::warning);
        return;
    }

    wxString executavelResolvido = ResolverExecutavel(runtime);
    if (executavelResolvido.IsEmpty())
    {
        RegistrarMensagem(
            wxString::Format("LinguagensDL: runtime '%s' nao foi encontrado no PATH nem em caminho absoluto.", runtime),
            Logger::error);
        return;
    }

    wxString comando = ConstruirComando(
        executavelResolvido,
        ObterArgumentosRuntime(chaveRuntime),
        caminhoArquivo,
        ObterArgumentosPrograma(extensao));

    RegistrarMensagem(wxString::Format("LinguagensDL: executando: %s", comando));

    wxArrayString saidaPadrao;
    wxArrayString saidaErro;
    long codigoSaida = wxExecute(comando, saidaPadrao, saidaErro, wxEXEC_SYNC | wxEXEC_NODISABLE);

    for (const wxString& linha : saidaPadrao)
    {
        if (!linha.IsEmpty())
            RegistrarMensagem(linha);
    }

    for (const wxString& linha : saidaErro)
    {
        if (!linha.IsEmpty())
            RegistrarMensagem(linha, Logger::error);
    }

    if (codigoSaida == -1)
    {
        RegistrarMensagem("LinguagensDL: falha ao iniciar o processo do runtime.", Logger::error);
        return;
    }

    RegistrarMensagem(wxString::Format("LinguagensDL: processo finalizado com codigo %ld", codigoSaida));
}

bool Executor::ArquivoPossuiExecucaoDireta(const wxString& extensao) const
{
    return !ObterChaveRuntimePorExtensao(extensao).IsEmpty();
}

wxString Executor::ObterRuntimeParaArquivo(const wxString& caminhoArquivo) const
{
    wxFileName arquivo(caminhoArquivo);
    wxString extensao = arquivo.GetExt().Lower();
    wxString chaveRuntime = ObterChaveRuntimePorExtensao(extensao);
    if (chaveRuntime.IsEmpty())
        return wxEmptyString;

    wxString runtimeConfigurado = ObterRuntimeConfigurado(chaveRuntime);
    if (!runtimeConfigurado.IsEmpty())
        return runtimeConfigurado;

    return ObterRuntimePadrao(chaveRuntime);
}

wxString Executor::ObterChaveRuntimePorExtensao(const wxString& extensao) const
{
    for (const auto& entrada : s_runtimeMap)
    {
        if (extensao == entrada.ext)
            return entrada.key;
    }

    return wxEmptyString;
}

wxString Executor::ObterRuntimeConfigurado(const wxString& chave) const
{
    ConfigManager* configuracoes = Manager::Get()->GetConfigManager("linguagens_dl");
    return configuracoes ? configuracoes->Read(chave, wxString(wxEmptyString)) : wxString(wxEmptyString);
}

wxString Executor::ObterArgumentosRuntime(const wxString& chaveRuntime) const
{
    if (chaveRuntime.IsEmpty())
        return wxEmptyString;

    ConfigManager* configuracoes = Manager::Get()->GetConfigManager("linguagens_dl");
    if (!configuracoes)
        return wxEmptyString;

    return configuracoes->Read("args." + chaveRuntime, wxEmptyString);
}

wxString Executor::ObterArgumentosPrograma(const wxString& extensao) const
{
    if (extensao.IsEmpty())
        return wxEmptyString;

    ConfigManager* configuracoes = Manager::Get()->GetConfigManager("linguagens_dl");
    if (!configuracoes)
        return wxEmptyString;

    return configuracoes->Read("program_args." + extensao, wxEmptyString);
}

wxString Executor::ObterRuntimePadrao(const wxString& chave) const
{
    if (chave == "delegua.runtime")
        return "delegua";
    if (chave == "potigol.runtime")
        return "potigol";
    if (chave == "mapler.runtime")
        return "mapler";
    if (chave == "portugol.runtime")
        return "portugol-studio";
    if (chave == "visualg.runtime")
        return "visualg3";

    return wxEmptyString;
}

wxString Executor::ResolverExecutavel(const wxString& runtime) const
{
    wxString candidato = RemoverAspasExternas(runtime);
    if (candidato.IsEmpty())
        return wxEmptyString;

    wxFileName arquivo(candidato);
    if (arquivo.IsAbsolute() || candidato.Find('\\') != wxNOT_FOUND || candidato.Find('/') != wxNOT_FOUND)
        return arquivo.FileExists() ? arquivo.GetFullPath() : wxString(wxEmptyString);

    wxArrayString extensoesExecutavel;
#ifdef __WXMSW__
    wxString pathExt;
    if (wxGetEnv("PATHEXT", &pathExt) && !pathExt.IsEmpty())
        extensoesExecutavel = GetArrayFromString(pathExt.Lower(), ";");
    if (extensoesExecutavel.IsEmpty())
    {
        extensoesExecutavel.Add(".exe");
        extensoesExecutavel.Add(".cmd");
        extensoesExecutavel.Add(".bat");
        extensoesExecutavel.Add(".com");
    }
#else
    extensoesExecutavel.Add(wxEmptyString);
#endif

    const wxString nomeBase = arquivo.GetExt().IsEmpty() ? candidato : arquivo.GetFullName();
    wxString valorPath;
    if (!wxGetEnv("PATH", &valorPath) || valorPath.IsEmpty())
        return wxEmptyString;

    wxArrayString diretorios = GetArrayFromString(valorPath, wxPATH_SEP);
    for (const wxString& diretorio : diretorios)
    {
        if (diretorio.IsEmpty())
            continue;

        wxFileName caminhoBase(diretorio, nomeBase);
        if (caminhoBase.FileExists())
            return caminhoBase.GetFullPath();

#ifdef __WXMSW__
        if (arquivo.GetExt().IsEmpty())
        {
            for (const wxString& extensao : extensoesExecutavel)
            {
                wxFileName caminhoComExt(diretorio, candidato + extensao);
                if (caminhoComExt.FileExists())
                    return caminhoComExt.GetFullPath();
            }
        }
#endif
    }

    return wxEmptyString;
}
