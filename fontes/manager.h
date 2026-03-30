#pragma once

#include <wx/string.h>
#include <map>
#include <vector>

class cbEditor;

/** Representa uma linguagem suportada pelo plugin. */
struct InformacoesLinguagem
{
    wxString              nome;           ///< Nome legível (ex.: "Delégua")
    std::vector<wxString> extensoes;      ///< Extensões sem ponto (ex.: "delegua")
    int                   lexerScintilla; ///< Constante SCLEX_* do Scintilla
    wxString              palavrasChave;  ///< Lista de palavras-chave separadas por espaços
    wxString              chaveRuntime;   ///< Chave de configuração para o executável do runtime
};

/**
 * Gerencia o catálogo de linguagens suportadas.
 *
 * Responsabilidades:
 *  - Definir metadados (extensões, lexer, keywords) para cada linguagem
 *  - Registrar extensões no sistema de mapeamento de arquivos do Code::Blocks
 *  - Determinar qual linguagem corresponde a um nome de arquivo
 *  - Aplicar o lexer e as palavras-chave corretos a um editor aberto
 */
class GerenciadorLinguagens
{
    public:
        GerenciadorLinguagens();
        ~GerenciadorLinguagens() = default;

        /** Registra todas as extensões de arquivo suportadas no Code::Blocks. */
        void RegistrarTodasExtensoesArquivo();

        /**
         * Aplica o lexer Scintilla e os keywords da linguagem ao editor.
         * Não faz nada se a extensão do arquivo não for reconhecida.
         */
        void AplicarRealce(cbEditor* editor, const wxString& nomeArquivo);

        /** Retorna as informações de linguagem para uma extensão, ou nullptr se desconhecida. */
        const InformacoesLinguagem* ObterLinguagemPorExtensao(const wxString& extensao) const;

    private:
        void ConstruirCatalogoLinguagens();

        std::vector<InformacoesLinguagem> linguagens_;
        std::map<wxString, const InformacoesLinguagem*> mapaExtensao_;
};
