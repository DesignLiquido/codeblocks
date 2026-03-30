# Arquitetura e Roadmap

## Visão geral

O plugin é implementado em **C++17** usando o **Code::Blocks SDK** e **wxWidgets 3.x**. Ele herda de `cbPlugin` (plugin genérico) e registra handlers para eventos do IDE (abertura de arquivos, compilação, etc.).

```
┌──────────────────────────────────────────────────────┐
│                   Code::Blocks IDE                   │
│                                                      │
│  ┌────────────┐  eventos   ┌──────────────────────┐  │
│  │  Editor /  │ ─────────► │  LinguagensDLPlugin  │  │
│  │  Projeto   │            │  (cbPlugin)          │  │
│  └────────────┘            └──────┬───────────────┘  │
│                                   │                  │
│              ┌────────────────────┼────────────────┐ │
│              │                    │                │ │
│       ┌──────────────▼───────┐   ┌─────────▼──────┐  ┌─────▼──────┐
│       │ GerenciadorLinguagens│   │  SyntaxHighlighter│  │ Executor  │
│       │                      │   │  (Scintilla)      │  │ (delegua, │
│       │ - registro           │   │  - lexer por lang │  │ potigol…) │
│       │   de exts            │   │  - keywords       │  └─────┬──────┘
│       └──────────────────────┘   └───────────────────┘        │
│                                                       │
│       ┌──────────────────┐   ┌────────────────────┐   │
│       │  CompletionProvider│  │  DebuggerBridge    │◄──┘
│       │  (palavras-chave,│   │  (DAP)             │
│       │   funções built-in)│  │                    │
│       └──────────────────┘   └────────────────────┘
└──────────────────────────────────────────────────────┘
```

---

## Componentes

### `LinguagensDLPlugin` (`fontes/plugin.h/.cpp`)
Classe principal. Herda de `cbPlugin`. Responsabilidades:
- Inicialização e finalização do plugin
- Registro de handlers de eventos do CB
- Instanciação dos sub-componentes abaixo

### `GerenciadorLinguagens` (`fontes/manager.h/.cpp`)
Cataloga todas as linguagens suportadas. Para cada linguagem mantém:
- Extensões de arquivo
- Nome do lexer Scintilla a usar
- Lista de palavras-chave (em português)
- Caminho configurável para o runtime

### `SyntaxHighlighter` (`fontes/highlighter/`)
Responsável pelo realce de sintaxe. Estratégia por linguagem:

| Linguagem | Lexer base no Scintilla | Justificativa |
|-----------|------------------------|---------------|
| Delégua, Pituguês, BIRL, Égua | `SCLEX_CPP` com keywords PT-BR | Sintaxe com blocos, operadores e strings semelhante ao C/JS |
| Potigol | `SCLEX_CPP` | Sintaxe funcional/imperativa semelhante |
| Portugol Mapler, Portugol Studio, VisuAlg | `SCLEX_CPP` | Mesma família Portugol |
| LMHT | `SCLEX_XML` | Linguagem de marcação baseada em XML |
| FolEs | `SCLEX_CSS` | Linguagem de estilização semelhante ao CSS |
| LinConEs | `SCLEX_SQL` | Linguagem de consulta inspirada no SQL |

Cada linguagem tem um arquivo `.xml` em `recursos/languages/` que declara as listas de palavras-chave.

### `Executor` (`fontes/runner.h/.cpp`)
Executa o arquivo aberto no editor via runtime externo. Fluxo:
1. Lê a extensão do arquivo ativo e determina a linguagem
2. Localiza o executável do runtime (configurável nas preferências do plugin)
3. Invoca o processo e redireciona stdout/stderr para o painel de saída do CB

Runtimes esperados:
- **Delégua / maioria dos dialetos**: `delegua` (via `delegua-node`)
- **Potigol**: `potigol`
- **Portugol Studio**: `portugol-studio` / servidor local
- **VisuAlg**: `visualg3` (Windows-only) ou runtime compatível

### `CompletionProvider` (`fontes/completion/`)
Provê completude de código via `cbCodeCompletionPlugin`. Fases:
1. **Fase 1** — completude de palavras-chave estáticas por linguagem
2. **Fase 2** — completude de funções da biblioteca padrão de Delégua
3. **Fase 3** — completude baseada em símbolos do arquivo aberto (análise semântica leve)

### `DebuggerBridge` (`fontes/debugger/`)
Integração com o mecanismo de depuração do Code::Blocks via **DAP (Debug Adapter Protocol)**, que é o protocolo já implementado pelos runtimes de Delégua e dialetos compatíveis.

> **Atenção:** esta é a parte mais complexa. O CB tem sua própria abstração de depurador (`cbDebuggerPlugin`). A ponte entre DAP e a API do CB será desenvolvida na Fase 5.

---

## Roadmap de fases

### Fase 1 — Fundação (MVP)
**Meta:** plugin que carrega no CB sem erros, com extensões de arquivo registradas.

- [ ] Esqueleto de `LinguagensDLPlugin` (herda `cbPlugin`)
- [ ] `manifest.xml` com metadados do plugin
- [ ] Arquivo de projeto `.cbp`
- [ ] `GerenciadorLinguagens` registrando as extensões de todos os arquivos suportados
- [ ] Ícones do plugin (80×80 PNG)
- [ ] CI básico (build Linux e Windows via GitHub Actions)

### Fase 2 — Realce de sintaxe
**Meta:** arquivos das linguagens suportadas são abertos com cores corretas no editor do CB.

- [ ] Arquivos `.xml` de palavras-chave para cada linguagem (espelhar listas da extensão VSCode em `gramaticas/`)
- [ ] `SyntaxHighlighter` aplicando lexer Scintilla correto ao abrir arquivo
- [ ] Cores padrão para: palavras-chave, strings, comentários, números, operadores
- [ ] Detectar corretamente arquivos `.delegua`, `.lmht`, `.foles`, `.lincones`, `.pitugues`, `.alg`, `.por`, `.mapler`, `.egua`, `.potigol`, `.birl`

### Fase 3 — Execução de código
**Meta:** botão/menu para executar o arquivo atual com o runtime adequado.

- [ ] Painel de configurações do plugin (caminhos dos runtimes)
- [ ] `Executor` detectando linguagem e invocando runtime correto
- [ ] Saída exibida no console de saída do Code::Blocks
- [ ] Suporte a argumentos de linha de comando configuráveis
- [ ] Detecção automática de runtimes instalados via PATH

### Fase 4 — Completude de código
**Meta:** auto-complete básico para todas as linguagens, completo para Delégua.

- [ ] `CompletionProvider` com listas de palavras-chave para todos os dialetos
- [ ] Funções da biblioteca padrão de Delégua com assinaturas
- [ ] Completude de símbolos definidos no arquivo atual (variáveis, funções)

### Fase 5 — Depurador
**Meta:** depuração de Delégua (e dialetos compatíveis) com breakpoints e inspeção de variáveis.

- [ ] `DebuggerBridge` com comunicação DAP (JSON-RPC sobre stdio)
- [ ] Suporte a: iniciar sessão, breakpoints, step over/into/out, continuar, parar
- [ ] Painel de variáveis (watch)
- [ ] Integração com a UI de depuração padrão do Code::Blocks

### Fase 6 — Funcionalidades avançadas
**Meta:** experiência completa de desenvolvimento.

- [ ] Assistente de projetos (*wizard*) para criar novos projetos Delégua
- [ ] Formatação de código para Delégua (invocar estilizador)
- [ ] *Code folding* para blocos de Delégua
- [ ] Tradução entre linguagens (VisuAlg → Delégua, Delégua → JS, etc.) via menu

---

## Referência: extensão VSCode

A [extensão VSCode da Design Líquido](https://github.com/DesignLiquido/vscode) é a principal referência de funcionalidades. Os seguintes artefatos dela são diretamente aproveitáveis:

| Artefato VSCode | Uso no plugin CB |
|-----------------|-----------------|
| `gramaticas/*.json` | Listas de palavras-chave e padrões de tokens |
| `snippets/*.json` | Base para trechos de código |
| `recursos/` | Ícones e imagens de linguagens |

As gramáticas Textmate (`.json`) precisam ser convertidas para o formato Scintilla XML durante a Fase 2.
