#pragma once

#include <wx/arrstr.h>
#include <wx/string.h>

class cbEditor;
class cbStyledTextCtrl;
class GerenciadorLinguagens;
struct InformacoesLinguagem;

class ProvedorCompletude
{
    public:
        explicit ProvedorCompletude(const GerenciadorLinguagens* gerenciadorLinguagens);

        void TalvezExibirCompletude(cbEditor* editor, int caractereDigitado) const;

    private:
        bool LinguagemSuportaAssinaturasDelegua(const InformacoesLinguagem* linguagem) const;
        void TalvezExibirAssinaturaFuncao(const InformacoesLinguagem* linguagem,
                                         cbStyledTextCtrl* controleTexto,
                                         int caractereDigitado) const;
        wxString ObterTokenAnteriorAoCursor(cbStyledTextCtrl* controleTexto) const;
        wxString ExtrairPrefixo(cbStyledTextCtrl* controleTexto) const;
        wxString ConstruirListaSugestoes(const InformacoesLinguagem* linguagem,
                                         cbStyledTextCtrl* controleTexto,
                                         const wxString& prefixo) const;
        void AdicionarPalavrasChave(const InformacoesLinguagem* linguagem,
                                    const wxString& prefixo,
                                    wxArrayString& sugestoes) const;
        void AdicionarSimbolosArquivo(cbStyledTextCtrl* controleTexto,
                                      const wxString& prefixo,
                                      wxArrayString& sugestoes) const;
        void AdicionarAssinaturasBibliotecaDelegua(const InformacoesLinguagem* linguagem,
                               const wxString& prefixo,
                               wxArrayString& sugestoes) const;
        bool EhCaractereDisparador(int caractereDigitado) const;

        const GerenciadorLinguagens* gerenciador_linguagens_;
};