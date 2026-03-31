#include "completion.h"

#include "manager.h"

#include <sdk.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>

#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>

namespace
{
    bool ComecaComPrefixo(const wxString& valor, const wxString& prefixo)
    {
        return valor.Length() >= prefixo.Length() && valor.Left(prefixo.Length()).CmpNoCase(prefixo) == 0;
    }

    bool EhIdentificadorValido(const wxString& token)
    {
        if (token.IsEmpty())
            return false;

        const wxChar primeiro = token[0];
        if (!(wxIsalpha(primeiro) || primeiro == '_'))
            return false;

        for (size_t i = 1; i < token.Length(); ++i)
        {
            const wxChar caractere = token[i];
            if (!(wxIsalnum(caractere) || caractere == '_'))
                return false;
        }

        return true;
    }

    void AdicionarSugestaoUnica(const wxString& sugestao, wxArrayString& sugestoes)
    {
        if (sugestao.IsEmpty())
            return;

        for (const wxString& existente : sugestoes)
        {
            if (existente.CmpNoCase(sugestao) == 0)
                return;
        }

        sugestoes.Add(sugestao);
    }
}

ProvedorCompletude::ProvedorCompletude(const GerenciadorLinguagens* gerenciadorLinguagens)
    : gerenciador_linguagens_(gerenciadorLinguagens)
{
}

void ProvedorCompletude::TalvezExibirCompletude(cbEditor* editor, int caractereDigitado) const
{
    if (!editor || !gerenciador_linguagens_ || !EhCaractereDisparador(caractereDigitado))
        return;

    cbStyledTextCtrl* controleTexto = editor->GetControl();
    if (!controleTexto || controleTexto->AutoCompActive())
        return;

    wxFileName arquivo(editor->GetFilename());
    const InformacoesLinguagem* linguagem = gerenciador_linguagens_->ObterLinguagemPorExtensao(arquivo.GetExt().Lower());
    if (!linguagem)
        return;

    wxString prefixo = ExtrairPrefixo(controleTexto);
    if (prefixo.Length() < 2)
        return;

    wxString listaSugestoes = ConstruirListaSugestoes(linguagem, controleTexto, prefixo);
    if (listaSugestoes.IsEmpty())
        return;

    controleTexto->AutoCompSetIgnoreCase(true);
    controleTexto->AutoCompSetChooseSingle(false);
    controleTexto->AutoCompShow(prefixo.Length(), listaSugestoes);
}

wxString ProvedorCompletude::ExtrairPrefixo(cbStyledTextCtrl* controleTexto) const
{
    const int posicaoAtual = controleTexto->GetCurrentPos();
    const int inicioPalavra = controleTexto->WordStartPosition(posicaoAtual, true);
    if (inicioPalavra == wxSCI_INVALID_POSITION || inicioPalavra >= posicaoAtual)
        return wxEmptyString;

    return controleTexto->GetTextRange(inicioPalavra, posicaoAtual);
}

wxString ProvedorCompletude::ConstruirListaSugestoes(const InformacoesLinguagem* linguagem,
                                                     cbStyledTextCtrl* controleTexto,
                                                     const wxString& prefixo) const
{
    wxArrayString sugestoes;

    AdicionarPalavrasChave(linguagem, prefixo, sugestoes);
    AdicionarSimbolosArquivo(controleTexto, prefixo, sugestoes);

    if (sugestoes.IsEmpty())
        return wxEmptyString;

    sugestoes.Sort([](const wxString& esquerda, const wxString& direita)
    {
        return esquerda.CmpNoCase(direita);
    });

    wxString listaSugestoes;
    for (size_t i = 0; i < sugestoes.Count(); ++i)
    {
        if (i != 0)
            listaSugestoes << ' ';
        listaSugestoes << sugestoes[i];
    }

    return listaSugestoes;
}

void ProvedorCompletude::AdicionarPalavrasChave(const InformacoesLinguagem* linguagem,
                                                const wxString& prefixo,
                                                wxArrayString& sugestoes) const
{
    wxStringTokenizer tokens(linguagem->palavrasChave, " ");
    while (tokens.HasMoreTokens())
    {
        const wxString token = tokens.GetNextToken();
        if (EhIdentificadorValido(token) && ComecaComPrefixo(token, prefixo))
            AdicionarSugestaoUnica(token, sugestoes);
    }
}

void ProvedorCompletude::AdicionarSimbolosArquivo(cbStyledTextCtrl* controleTexto,
                                                  const wxString& prefixo,
                                                  wxArrayString& sugestoes) const
{
    const wxString texto = controleTexto->GetText();
    wxStringTokenizer linhas(texto, "\n", wxTOKEN_RET_EMPTY_ALL);

    const wxArrayString marcadoresDeclaracao = {
        "var", "constante", "classe", "funcao", "função", "procedimento",
        "tipo", "inteiro", "real", "logico", "lógico", "caracter", "carácter",
        "cadeia", "texto"
    };

    while (linhas.HasMoreTokens())
    {
        wxString linha = linhas.GetNextToken();
        linha.Replace("(", " ");
        linha.Replace(")", " ");
        linha.Replace(":", " ");
        linha.Replace(",", " ");
        linha.Replace("=", " ");

        wxStringTokenizer tokens(linha, " \t\r");
        wxString tokenAnterior;
        while (tokens.HasMoreTokens())
        {
            const wxString token = tokens.GetNextToken();
            for (const wxString& marcador : marcadoresDeclaracao)
            {
                if (tokenAnterior.CmpNoCase(marcador) == 0 && EhIdentificadorValido(token) && ComecaComPrefixo(token, prefixo))
                {
                    AdicionarSugestaoUnica(token, sugestoes);
                    break;
                }
            }
            tokenAnterior = token;
        }
    }
}

bool ProvedorCompletude::EhCaractereDisparador(int caractereDigitado) const
{
    return wxIsalnum(static_cast<wxChar>(caractereDigitado)) || caractereDigitado == '_';
}