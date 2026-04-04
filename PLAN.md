# Plano do Plugin Code::Blocks — Design Líquido

## Linguagens suportadas (11 no total)

| Categoria | Linguagens |
|-----------|------------|
| Nativas Design Líquido | Delégua (`.delegua`), LMHT (`.lmht`), FolEs (`.foles`), LinConEs (`.lincones`) |
| Dialetos de Delégua | Pituguês (`.pitugues`), BIRL (`.birl`), Potigol (`.potigol`), Égua (`.egua`) |
| Dialetos de Portugol | Portugol Mapler (`.mapler`), Portugol Studio (`.por`), VisuAlg (`.alg`) |

---

## Fases de desenvolvimento

### Fase 1 — Fundação (MVP)
**Meta:** plugin que carrega no Code::Blocks sem erros, com extensões de arquivo registradas.

- [x] Esqueleto de `LinguagensDLPlugin` (herda `cbPlugin`)
- [x] `manifest.xml` com metadados do plugin
- [x] Arquivo de projeto `linguagens-dl.cbp`
- [x] `LanguagesManager` registrando as extensões de todos os arquivos suportados
- [x] Ícones do plugin (PNG 80×80, estado ativo e inativo)
- [x] CI básico (build Linux e Windows via GitHub Actions)

### Fase 2 — Realce de sintaxe
**Meta:** arquivos das linguagens suportadas são abertos com cores corretas no editor do Code::Blocks.

Cada linguagem usa um lexer Scintilla diferente:

| Linguagem | Lexer base no Scintilla | Justificativa |
|-----------|------------------------|---------------|
| Delégua, Pituguês, BIRL, Égua, Potigol | `SCLEX_CPP` com keywords PT-BR | Sintaxe com blocos, operadores e strings semelhante ao C/JS |
| Portugol Mapler, Portugol Studio, VisuAlg | `SCLEX_CPP` | Mesma família Portugol |
| LMHT | `SCLEX_XML` | Linguagem de marcação baseada em XML |
| FolEs | `SCLEX_CSS` | Linguagem de estilização semelhante ao CSS |
| LinConEs | `SCLEX_SQL` | Linguagem de consulta inspirada no SQL |

Tarefas:
- [x] Arquivos `.xml` de palavras-chave para cada linguagem (conversão a partir das gramáticas da extensão VSCode em `gramaticas/`)
- [x] `SyntaxHighlighter` aplicando lexer Scintilla correto ao abrir arquivo
- [x] Cores padrão para: palavras-chave, strings, comentários, números, operadores
- [x] Detectar corretamente todos os tipos de arquivo suportados

### Fase 3 — Execução de código
**Meta:** botão/menu para executar o arquivo atual com o runtime adequado.

- [x] Painel de configurações do plugin (caminhos dos runtimes)
- [x] `Runner` detectando linguagem e invocando runtime correto
- [x] Saída exibida no console de saída do Code::Blocks
- [x] Suporte a argumentos de linha de comando configuráveis
- [x] Detecção automática de runtimes instalados via `PATH`

Runtimes esperados:

| Linguagem | Runtime |
|-----------|---------|
| Delégua, Pituguês, BIRL, Égua | `delegua` (via `delegua-node`) |
| Potigol | `potigol` |
| Portugol Mapler | `mapler` |
| Portugol Studio | `portugol-studio` |
| VisuAlg | `visualg3` (Windows) ou runtime compatível |

### Fase 4 — Completude de código
**Meta:** auto-complete básico para todas as linguagens, completo para Delégua.

 - [x] `CompletionProvider` com listas de palavras-chave para todos os dialetos
 - [x] Funções da biblioteca padrão de Delégua com assinaturas
 - [x] Completude de símbolos definidos no arquivo atual (variáveis, funções)

### Fase 5 — Depurador
**Meta:** depuração de Delégua (e dialetos compatíveis) com breakpoints e inspeção de variáveis.

- [x] `DebuggerBridge` com comunicação DAP (JSON-RPC sobre stdio)
- [x] Suporte a: iniciar sessão, breakpoints, step over/into/out, continuar, parar
- [x] Painel de variáveis (watch)
- [x] Integração com a UI de depuração padrão do Code::Blocks

### Fase 6 — Funcionalidades avançadas
**Meta:** experiência completa de desenvolvimento.

- [x] Assistente de projetos (*wizard*) para criar novos projetos Delégua
- [x] Formatação de código para Delégua (invocar estilizador)
- [x] *Code folding* para blocos de Delégua
- [x] Tradução entre linguagens (VisuAlg → Delégua, Delégua → JS, etc.) via menu

### Fase 7 — Compatibilidade com wxWidgets 3.3.x
**Meta:** compilar o plugin contra wxWidgets 3.3.x, eliminando a dependência da série 3.2.

Contexto: o SDK do Code::Blocks 25.03 foi compilado contra wxWidgets 3.2. A série 3.3 introduziu quebras de API (`wxColourImpl`, `WXWidget`, entre outros) que impedem a compilação direta do plugin. Resolver isso requer uma das abordagens abaixo:

- [ ] Recompilar o Code::Blocks 25.03 a partir do código-fonte linkando contra wxWidgets 3.3.x, e usar o SDK resultante
- [ ] Auditar os headers do CB SDK (`cbplugin.h`, `globals.h`, `editorbase.h`, etc.) e adicionar guardas de compatibilidade para os tipos que mudaram entre 3.2 e 3.3
- [ ] Atualizar o CI (`build.yml`, `release.yml`) para usar `mingw-w64-x86_64-wxwidgets3.3-msw` quando a compatibilidade for atingida

---

## Arquivos do repositório

| Arquivo/Pasta | Descrição |
|---------------|-----------|
| [README.md](README.md) | Visão geral, tabela de linguagens, checklist de funcionalidades |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Guia de contribuição e setup do ambiente |
| [docs/CONSTRUCAO.md](docs/CONSTRUCAO.md) | Instruções de build para Windows, Linux e macOS |
| [docs/ARQUITETURA.md](docs/ARQUITETURA.md) | Diagrama de componentes, decisões técnicas, roadmap detalhado |
| [recursos/manifest.xml](recursos/manifest.xml) | Metadados do plugin para o Code::Blocks |
| [fontes/plugin.h](fontes/plugin.h) / [fontes/plugin.cpp](fontes/plugin.cpp) | Classe principal `LinguagensDLPlugin` (herda `cbPlugin`) |
| [fontes/manager.h](fontes/manager.h) / [fontes/manager.cpp](fontes/manager.cpp) | Catálogo de linguagens com keywords e mapeamento de extensões |
| [fontes/runner.h](fontes/runner.h) / [fontes/runner.cpp](fontes/runner.cpp) | Execução de código via runtime externo |

---

## Referências

- [Extensão VSCode da Design Líquido](https://github.com/DesignLiquido/vscode) — referência principal de funcionalidades; as gramáticas em `gramaticas/` são aproveitadas para as listas de keywords
- [Creating a simple "Hello World" plugin](https://wiki.codeblocks.org/index.php/Creating_a_simple_%22Hello_World%22_plugin)
- [Managing Plug-in Resources](https://wiki.codeblocks.org/index.php/Managing_Plug-in_Resources)
- [Code::Blocks Plugins](https://wiki.codeblocks.org/index.php/Code::Blocks_Plugins)
