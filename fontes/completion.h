#pragma once

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
        bool EhCaractereDisparador(int caractereDigitado) const;

        const GerenciadorLinguagens* gerenciador_linguagens_;
};