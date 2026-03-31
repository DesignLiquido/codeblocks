#include "manager.h"

#include <sdk.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include <editorcolourset.h>
#include <editormanager.h>
#include <logmanager.h>
#include <manager.h>

#include <wx/filename.h>
#include <wx/arrstr.h>
#include <wx/xml/xml.h>

// Constantes Scintilla usadas como lexers base
// Definidas em <Scintilla/include/SciLexer.h>, incluído pelo CB SDK
#ifndef SCLEX_CPP
#  define SCLEX_CPP  3
#endif
#ifndef SCLEX_XML
#  define SCLEX_XML  5
#endif
#ifndef SCLEX_CSS
#  define SCLEX_CSS  38
#endif
#ifndef SCLEX_SQL
#  define SCLEX_SQL  7
#endif

GerenciadorLinguagens::GerenciadorLinguagens()
{
    ConstruirCatalogoLinguagens();
}

void GerenciadorLinguagens::ConstruirCatalogoLinguagens()
{
    // -------------------------------------------------------------------------
    // Delégua — linguagem principal
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "Delégua",
        { "delegua" },
        SCLEX_CPP,
        "C/C++",
        // Palavras-chave primárias
        "enquanto para cada se senão senao função funcao retorne retornar "
        "classe herda novo var constante verdadeiro falso nulo e ou não nao "
        "importar de tente capture finalmente lançar lancar assíncrono assincrono "
        "aguarde pausa continuar escolha caso padrão padrao romper interromper",
        "delegua.runtime"
    });

    // -------------------------------------------------------------------------
    // Pituguês — dialeto de Delégua
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "Pituguês",
        { "pitugues" },
        SCLEX_CPP,
        "C/C++",
        "enquanto para cada se senão função retorne retornar classe herda novo "
        "var constante verdadeiro falso nulo e ou não importar",
        "delegua.runtime"
    });

    // -------------------------------------------------------------------------
    // BIRL — dialeto de Delégua
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "BIRL",
        { "birl" },
        SCLEX_CPP,
        "C/C++",
        "HORA DO SHOW BIRL E AEBORA MONSTRAO TRAPEZIO DESCENDENTE "
        "MENOR OU E MENOR QUE MAIOR OU E MAIOR QUE IGUAL",
        "delegua.runtime"
    });

    // -------------------------------------------------------------------------
    // Potigol — dialeto de Delégua
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "Potigol",
        { "potigol" },
        SCLEX_CPP,
        "C/C++",
        "se então senão para cada em enquanto faça função retorne tipo "
        "verdadeiro falso nulo e ou não imprima leia",
        "potigol.runtime"
    });

    // -------------------------------------------------------------------------
    // Égua — dialeto de Delégua
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "Égua",
        { "egua" },
        SCLEX_CPP,
        "C/C++",
        "enquanto para cada se senão função retorne classe herda novo "
        "var verdadeiro falso nulo e ou não importar",
        "delegua.runtime"
    });

    // -------------------------------------------------------------------------
    // LMHT — linguagem de marcação (baseada em XML)
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "LMHT",
        { "lmht" },
        SCLEX_XML,
        "XML",
        "",   // LMHT usa tags XML; palavras-chave são as tags da spec
        ""    // Sem runtime de execução direto
    });

    // -------------------------------------------------------------------------
    // FolEs — linguagem de estilização (baseada em CSS)
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "FolEs",
        { "foles" },
        SCLEX_CSS,
        "CSS",
        "cor fundo margem enchimento borda fonte tamanho peso estilo "
        "decoração alinhamento exibição posição largura altura",
        ""
    });

    // -------------------------------------------------------------------------
    // LinConEs — linguagem de consulta (baseada em SQL)
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "LinConEs",
        { "lincones" },
        SCLEX_SQL,
        "SQL",
        "selecionar de onde ordenar por crescente decrescente agrupar "
        "tendo inserir em valores atualizar definir deletar criar tabela "
        "indice visao procedimento função juntar esquerda direita interno externo",
        ""
    });

    // -------------------------------------------------------------------------
    // Portugol Mapler
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "Portugol Mapler",
        { "mapler" },
        SCLEX_CPP,
        "C/C++",
        "algoritmo inicio fim se entao senao enquanto faca para de ate "
        "funcao procedimento retorne inteiro real logico caracter texto "
        "verdadeiro falso e ou nao escreva leia",
        "mapler.runtime"
    });

    // -------------------------------------------------------------------------
    // Portugol Studio / Webstudio
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "Portugol Studio",
        { "por" },
        SCLEX_CPP,
        "C/C++",
        "programa funcao inicio fim se entao senao enquanto faca para de ate "
        "inteiro real logico caracter cadeia vazio retorne e ou nao "
        "verdadeiro falso escreva leia",
        "portugol.runtime"
    });

    // -------------------------------------------------------------------------
    // Portugol VisuAlg
    // -------------------------------------------------------------------------
    linguagens_.push_back({
        "Portugol VisuAlg",
        { "alg" },
        SCLEX_CPP,
        "C/C++",
        "algoritmo inicio fimalgoritmo var inteiro real logico caracter texto "
        "se entao senao fimse enquanto faca fimenquanto para ate passo "
        "fimpara escreva escreval leia e ou nao verdadeiro falso",
        "visualg.runtime"
    });

    // Constrói o mapa extensão → linguagem
    for (const auto& linguagem : linguagens_)
    {
        for (const auto& extensao : linguagem.extensoes)
        {
            mapaExtensao_[extensao.Lower()] = &linguagem;
        }
    }

    CarregarPalavrasChaveExternas();
}

void GerenciadorLinguagens::CarregarPalavrasChaveExternas()
{
    for (auto& linguagem : linguagens_)
    {
        if (linguagem.extensoes.empty())
            continue;

        const wxString caminhoArquivo = ObterCaminhoArquivoPalavrasChave(linguagem.extensoes.front());
        if (caminhoArquivo.IsEmpty())
            continue;

        const wxString palavrasCarregadas = CarregarPalavrasChaveDoArquivo(caminhoArquivo);
        if (palavrasCarregadas.IsEmpty())
            continue;

        linguagem.palavrasChave = palavrasCarregadas;
    }
}

wxString GerenciadorLinguagens::CarregarPalavrasChaveDoArquivo(const wxString& caminhoArquivo) const
{
    wxXmlDocument documento;
    if (!documento.Load(caminhoArquivo))
        return wxEmptyString;

    wxXmlNode* raiz = documento.GetRoot();
    if (!raiz)
        return wxEmptyString;

    wxString palavras;
    for (wxXmlNode* no = raiz->GetChildren(); no; no = no->GetNext())
    {
        if (no->GetType() != wxXML_ELEMENT_NODE)
            continue;

        if (no->GetName() != "set" && no->GetName() != "grupo")
            continue;

        wxString conteudo;
        for (wxXmlNode* filho = no->GetChildren(); filho; filho = filho->GetNext())
        {
            if (filho->GetType() == wxXML_TEXT_NODE || filho->GetType() == wxXML_CDATA_SECTION_NODE)
                conteudo.Append(filho->GetContent());
        }

        conteudo.Trim(true);
        conteudo.Trim(false);
        if (conteudo.IsEmpty())
            continue;

        if (!palavras.IsEmpty())
            palavras.Append(' ');
        palavras.Append(conteudo);
    }

    return palavras;
}

wxString GerenciadorLinguagens::ObterCaminhoArquivoPalavrasChave(const wxString& extensao) const
{
    wxFileName arquivoLocal(wxString::Format("recursos/palavras-chave/%s.xml", extensao));
    if (arquivoLocal.FileExists())
        return arquivoLocal.GetFullPath();

    return wxEmptyString;
}

wxString GerenciadorLinguagens::ConstruirMascarasPorPerfil(const wxString& perfilBase) const
{
    wxArrayString mascaras;

    EditorManager* editorManager = Manager::Get()->GetEditorManager();
    EditorColourSet* tema = editorManager ? editorManager->GetColourSet() : nullptr;
    if (!tema)
        return wxEmptyString;

    HighlightLanguage linguagemBase = tema->GetHighlightLanguage(perfilBase);
    if (linguagemBase != HL_NONE)
        mascaras = tema->GetFileMasks(linguagemBase);

    for (const auto& linguagem : linguagens_)
    {
        if (linguagem.perfilBase != perfilBase)
            continue;

        for (const auto& extensao : linguagem.extensoes)
        {
            mascaras.Add(wxString::Format("*.%s", extensao));
        }
    }

    mascaras = MakeUniqueArray(mascaras, false);
    return GetStringFromArray(mascaras, ",", false);
}

void GerenciadorLinguagens::RegistrarTodasExtensoesArquivo()
{
    EditorManager* editorManager = Manager::Get()->GetEditorManager();
    EditorColourSet* tema = editorManager ? editorManager->GetColourSet() : nullptr;

    if (tema)
    {
        const wxString perfisBase[] = { "C/C++", "XML", "CSS", "SQL" };
        for (const wxString& perfilBase : perfisBase)
        {
            HighlightLanguage linguagemBase = tema->GetHighlightLanguage(perfilBase);
            if (linguagemBase == HL_NONE)
                continue;

            wxString mascaras = ConstruirMascarasPorPerfil(perfilBase);
            if (!mascaras.IsEmpty())
                tema->SetFileMasks(linguagemBase, mascaras);
        }
    }

    size_t quantidadeExtensoes = 0;
    wxString listaExtensoes;

    for (const auto& linguagem : linguagens_)
    {
        for (const auto& extensao : linguagem.extensoes)
        {
            if (!listaExtensoes.IsEmpty())
                listaExtensoes.Append(", ");
            listaExtensoes.Append('.');
            listaExtensoes.Append(extensao);
            ++quantidadeExtensoes;
        }
    }

    Manager::Get()->GetLogManager()->Log(
        wxString::Format(
            "LinguagensDL: %zu extensoes registradas no editor (%s)",
            quantidadeExtensoes,
            listaExtensoes));
}

void GerenciadorLinguagens::AplicarRealce(cbEditor* editor, const wxString& nomeArquivo)
{
    if (!editor) return;

    wxFileName arquivo(nomeArquivo);
    wxString extensao = arquivo.GetExt().Lower();

    const InformacoesLinguagem* linguagem = ObterLinguagemPorExtensao(extensao);
    if (!linguagem) return;

    cbStyledTextCtrl* controleTexto = editor->GetControl();
    if (!controleTexto) return;

    EditorManager* editorManager = Manager::Get()->GetEditorManager();
    EditorColourSet* tema = editorManager ? editorManager->GetColourSet() : nullptr;
    if (tema)
    {
        HighlightLanguage linguagemBase = tema->GetHighlightLanguage(linguagem->perfilBase);
        if (linguagemBase != HL_NONE)
            editor->SetLanguage(linguagemBase, false);
        else
            controleTexto->SetLexer(linguagem->lexerScintilla);
    }
    else
    {
        controleTexto->SetLexer(linguagem->lexerScintilla);
    }

    if (!linguagem->palavrasChave.IsEmpty())
    {
        controleTexto->SetKeyWords(0, linguagem->palavrasChave);
    }
    else
    {
        controleTexto->SetKeyWords(0, "");
    }

    controleTexto->Colourise(0, -1);

    Manager::Get()->GetLogManager()->DebugLog(
        wxString::Format(
            "LinguagensDL: realce aplicado para %s (%s)",
            nomeArquivo,
            linguagem->nome));
}

const InformacoesLinguagem* GerenciadorLinguagens::ObterLinguagemPorExtensao(const wxString& extensao) const
{
    auto it = mapaExtensao_.find(extensao.Lower());
    if (it != mapaExtensao_.end())
        return it->second;
    return nullptr;
}
