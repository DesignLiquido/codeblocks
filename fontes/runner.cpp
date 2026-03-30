#include "runner.h"

#include <sdk.h>
#include <configmanager.h>
#include <logmanager.h>
#include <manager.h>

#include <wx/filename.h>
#include <wx/process.h>

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

void Executor::ExecutarArquivo(const wxString& caminhoArquivo)
{
    wxFileName arquivo(caminhoArquivo);
    wxString extensao = arquivo.GetExt().Lower();

    if (extensao == "lmht" || extensao == "foles" || extensao == "lincones")
    {
        Manager::Get()->GetLogManager()->LogWarning(
            wxString::Format("LinguagensDL: a extensao '.%s' nao possui runtime de execucao direta.", extensao));
        return;
    }

    wxString runtime = ObterRuntimeParaArquivo(caminhoArquivo);
    if (runtime.IsEmpty())
    {
        Manager::Get()->GetLogManager()->LogWarning(
            wxString::Format("LinguagensDL: nenhum runtime configurado para '%s'", caminhoArquivo));
        return;
    }

    wxString comando = wxString::Format(wxT("\"%s\" \"%s\""), runtime, caminhoArquivo);
    Manager::Get()->GetLogManager()->Log(
        wxString::Format("LinguagensDL: executando: %s", comando));

    // TODO (Fase 3): substituir por execução assíncrona com captura de saída
    // integrada ao painel de saída do Code::Blocks (usando cbProcess ou similar).
    wxExecute(comando);
}

wxString Executor::ObterRuntimeParaArquivo(const wxString& caminhoArquivo) const
{
    wxFileName arquivo(caminhoArquivo);
    wxString extensao = arquivo.GetExt().Lower();

    for (const auto& entrada : s_runtimeMap)
    {
        if (extensao == entrada.ext)
        {
            wxString runtimeConfigurado = ObterRuntimeConfigurado(entrada.key);
            if (!runtimeConfigurado.IsEmpty())
                return runtimeConfigurado;

            return ObterRuntimePadrao(entrada.key);
        }
    }
    return wxEmptyString;
}

wxString Executor::ObterRuntimeConfigurado(const wxString& chave) const
{
    ConfigManager* configuracoes = Manager::Get()->GetConfigManager("linguagens_dl");
    return configuracoes ? configuracoes->Read(chave, wxEmptyString) : wxEmptyString;
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
