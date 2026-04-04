# Como contribuir

Obrigado por seu interesse em contribuir com o plugin Code::Blocks da Design Líquido!

## Pré-requisitos

Antes de começar, instale:

1. **Code::Blocks 20.03+**
2. **Headers do SDK de plugins do Code::Blocks** extraídos do código-fonte
3. **wxWidgets 3.2.x** compilado para o mesmo compilador que você usará
4. **GCC / MinGW** (Windows) ou **GCC / Clang** (Linux/macOS)
5. **CMake 3.16+** (opcional, para builds fora do Code::Blocks)

Veja [docs/CONSTRUCAO.md](docs/CONSTRUCAO.md) para instruções detalhadas de configuração do ambiente.

## Notas práticas de configuração

### Windows com MSYS2 UCRT64
No Windows, prefira usar o ambiente **MSYS2 UCRT64** para o compilador, as bibliotecas do Code::Blocks e o wxWidgets. Neste projeto, os headers do SDK do Code::Blocks já estão vendorizados em `cbsdk-include/`, então o ponto crítico é manter o linker no mesmo ABI do `ucrt64`.

### wxWidgets no Windows
Se você instalou o pacote `mingw-w64-ucrt-x86_64-wxwidgets3.2-msw`, use `C:/msys64/ucrt64` como prefixo.

Os headers principais do wxWidgets ficam em `C:/msys64/ucrt64/include`, mas o IntelliSense e a compilação também precisam do arquivo `wx/setup.h`, que fica em `C:/msys64/ucrt64/lib/gcc_x64_dll/mswu`.

No MSYS2 UCRT64, instale as dependências com:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-codeblocks mingw-w64-ucrt-x86_64-wxwidgets3.2-msw
```

Evite misturar o Code::Blocks de `C:/Program Files` com as bibliotecas do MSYS2. Isso costuma gerar falhas de link por incompatibilidade de ABI.

### IntelliSense no VS Code
Para este repositório, o IntelliSense precisa destes includes:

- `${workspaceFolder}/fontes`
- `${workspaceFolder}/cbsdk-include`
- `${workspaceFolder}/sdk/wxscintilla/include`
- `C:/msys64/ucrt64/include`
- `C:/msys64/ucrt64/lib/gcc_x64_dll/mswu`

Também use:

- `compilerPath`: `C:/msys64/ucrt64/bin/g++.exe`
- `defines`: `__WXMSW__`, `WXUSINGDLL`, `_UNICODE`, `UNICODE`

## Fluxo de trabalho

1. Faça um *fork* deste repositório
2. Clone seu fork localmente
3. Crie um branch descritivo: `git checkout -b feature/realce-delegua`
4. Faça suas alterações e escreva testes quando aplicável
5. Abra um Pull Request descrevendo o que foi feito

## Empacotando para teste no Windows

Depois de compilar o target `Release` no Code::Blocks, gere o pacote instalável:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\empacotar-plugin.ps1 `
  -BinaryPath .\bin\Release\LinguagensDL.dll `
  -OutputDir dist `
  -Version 0.1.0
```

O arquivo `.cbplugin` resultante em `dist/` é o pacote que deve ser usado em **Plugins → Manage Plugins → Install new**.

## Convenções de código

- **Idioma do código**: português (nomes de variáveis, funções, classes)
- **Idioma da documentação e comentários**: português (Brasil)
- **Padrão C++**: C++17
- **Formatação**: use o estilo Code::Blocks (similar ao K&R / Allman para chaves)
- **Headers**: `#pragma once` em vez de include guards

## Áreas de contribuição

### Realce de sintaxe
Cada linguagem precisa de uma definição de lexer Scintilla e de uma lista de palavras-chave. Veja `fontes/highlighter/` e `recursos/languages/`.

### Integração com runtimes
Os runtimes das linguagens são baseados em Node.js. A integração envolve:
- Detectar o caminho do executável (`delegua`, `potigol`, etc.)
- Capturar saída padrão e erros para exibir no console do CB
- Permitir configuração via painel de preferências do plugin

### Completude de código
Cada linguagem tem palavras-chave e funções internas próprias. Veja `fontes/completion/`.

### Integração com depurador
Os runtimes de Delégua e dialetos usam o protocolo DAP (Debug Adapter Protocol). Veja `fontes/debugger/` e [docs/ARQUITETURA.md](docs/ARQUITETURA.md).

## Reportando problemas

Abra uma *issue* descrevendo:
- Versão do Code::Blocks e sistema operacional
- Passos para reproduzir o problema
- Comportamento esperado vs. comportamento observado
