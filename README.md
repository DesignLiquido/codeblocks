# Plugin Code::Blocks — Design Líquido

Plugin para o [Code::Blocks](https://www.codeblocks.org/) que adiciona suporte a todas as linguagens da [Design Líquido](https://github.com/DesignLiquido) e dialetos de Portugol.

## Linguagens suportadas

### Linguagens nativas (Design Líquido)

| Linguagem | Extensão | Descrição |
|-----------|----------|-----------|
| [Delégua](https://github.com/DesignLiquido/delegua) | `.delegua` | Linguagem de programação 100% em português |
| [LMHT](https://github.com/DesignLiquido/LMHT) | `.lmht` | Linguagem de marcação em português (inspirada em HTML) |
| [FolEs](https://github.com/DesignLiquido/FolEs) | `.foles` | Linguagem de estilização em português (inspirada em CSS/SASS) |
| [LinConEs](https://github.com/DesignLiquido/LinConEs) | `.lincones` | Linguagem de consulta a bancos de dados relacionais em português (inspirada em SQL) |

### Dialetos de Delégua

| Linguagem | Extensão | Observação |
|-----------|----------|------------|
| [Pituguês](https://github.com/DesignLiquido/delegua/wiki/Dialetos#pitugues) | `.pitugues` | Dialeto de Delégua |
| [BIRL](https://github.com/DesignLiquido/birl) | `.birl` | Dialeto de Delégua |
| [Potigol](https://github.com/DesignLiquido/potigol) | `.potigol` | Dialeto de Delégua |
| [Égua](https://egua.tech/) | `.egua` | Dialeto de Delégua |

### Dialetos de Portugol

| Linguagem | Extensão | Observação |
|-----------|----------|------------|
| [Portugol Mapler](https://github.com/DesignLiquido/mapler) | `.mapler` | Dialeto Portugol Mapler |
| [Portugol Studio](http://lite.acad.univali.br/portugol/) | `.por` | Portugol Studio / Webstudio |
| [Portugol VisuAlg](https://visualg3.com.br/) | `.alg` | VisuAlg 3 |

## Funcionalidades planejadas

- [x] Registro de extensões de arquivo no Code::Blocks
- [x] Realce de sintaxe para todas as linguagens
- [x] Integração com runtimes (executar arquivos diretamente)
- [x] Completude de código (palavras-chave e funções internas de Delégua)
- [x] Integração com depurador
- [x] Assistente de projetos (wizards)
- [x] Dobragem de código (*code folding*)
- [x] Formatação de código para Delégua
- [ ] Trechos de código (*snippets*)

> Estado de versão: os itens marcados com [x] compõem a primeira versão (v1). Snippets ficam para uma próxima iteração.

## Requisitos de build

- Code::Blocks 25.03 e wxWidgets 3.2 via **MSYS2 UCRT64**
- Headers do SDK do Code::Blocks já vendorizados no repositório
- wxWidgets 3.2.x
- C++17
- GCC / MinGW (Windows) ou GCC / Clang (Linux e macOS)

Veja [docs/CONSTRUCAO.md](docs/CONSTRUCAO.md) para instruções detalhadas.

## Empacotamento no Windows

No Windows, o build gera `LinguagensDL.dll`. O pacote instalável do Code::Blocks precisa ser montado separadamente como um `.cbplugin`, contendo:

- `LinguagensDL.dll`
- `LinguagensDL.zip`
- `LinguagensDL.png`
- `LinguagensDL-off.png`

Para isso, use:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\empacotar-plugin.ps1 `
  -BinaryPath .\bin\Release\LinguagensDL.dll `
  -OutputDir dist `
  -Version 0.1.0
```

## Estrutura do repositório

```
codeblocks/
├── fontes/
│   ├── plugin.h / plugin.cpp       # Classe principal do plugin
│   ├── manager.h / manager.cpp     # Gerenciador de linguagens
│   ├── runner.h / runner.cpp       # Execução de código
│   ├── highlighter/                # Definições de realce de sintaxe
│   ├── completion/                 # Provedores de completude por linguagem
│   └── debugger/                   # Integração com depurador (DAP)
├── recursos/
│   ├── manifest.xml                # Metadados do plugin
│   ├── palavras-chave/             # Palavras-chave por linguagem
│   └── LinguagensDL*.png           # Ícones do plugin
├── docs/
│   ├── CONSTRUCAO.md               # Como compilar o plugin
│   └── ARQUITETURA.md              # Decisões de arquitetura
├── scripts/
│   └── empacotar-plugin.ps1        # Gera o arquivo .cbplugin no Windows
└── linguagens-dl.cbp               # Arquivo de projeto do Code::Blocks
```

## Fases de desenvolvimento

Veja [docs/ARQUITETURA.md](docs/ARQUITETURA.md) para o roadmap completo.

## Referências

- [VSCode extension da Design Líquido](https://github.com/DesignLiquido/vscode) — extensão análoga para VS Code, usada como referência de funcionalidades
- [Creating a simple "Hello World" plugin](https://wiki.codeblocks.org/index.php/Creating_a_simple_%22Hello_World%22_plugin)
- [Managing Plug-in Resources](https://wiki.codeblocks.org/index.php/Managing_Plug-in_Resources)
- [Code::Blocks Plugins](https://wiki.codeblocks.org/index.php/Code::Blocks_Plugins)

## Licença

MIT — veja o arquivo [LICENSE](LICENSE).
