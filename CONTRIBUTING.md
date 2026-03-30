# Como contribuir

Obrigado por seu interesse em contribuir com o plugin Code::Blocks da Design Líquido!

## Pré-requisitos

Antes de começar, instale:

1. **Code::Blocks 20.03+**
2. **Headers do SDK de plugins do Code::Blocks** extraídos do código-fonte
3. **wxWidgets 3.x** compilado para o mesmo compilador que você usará
4. **GCC / MinGW** (Windows) ou **GCC / Clang** (Linux/macOS)
5. **CMake 3.16+** (opcional, para builds fora do Code::Blocks)

Veja [docs/CONSTRUCAO.md](docs/CONSTRUCAO.md) para instruções detalhadas de configuração do ambiente.

## Notas práticas de configuração

### Windows com instalador oficial do Code::Blocks
O instalador binário do Code::Blocks normalmente **não inclui** os headers do SDK de plugins, como `sdk.h`, `cbeditor.h` e `cbstyledtextctrl.h`. Para desenvolvimento do plugin, extraia a pasta `src/include/` do código-fonte oficial do Code::Blocks para dentro do repositório. Neste projeto usamos a pasta `cbsdk-include/`.

### wxWidgets no Windows
Baixe o código-fonte oficial em https://wxwidgets.org/downloads/ e extraia em um caminho simples, por exemplo `C:/wxWidgets-3.3.2`.

Os headers principais do wxWidgets ficam em `C:/wxWidgets-3.3.2/include`, mas o IntelliSense e a compilação também precisam do arquivo `wx/setup.h`, gerado após a build do wxWidgets. Com MinGW, ele fica em `C:/wxWidgets-3.3.2/lib/gcc_dll/mswu`.

Se a pasta `lib/gcc_dll/mswu/` ainda não existir, o wxWidgets ainda não foi compilado para esse toolchain.

Para compilar com o MinGW do Code::Blocks, entre em `build/msw` dentro da árvore do wxWidgets e execute o `mingw32-make` com opções compatíveis com DLL e Unicode. Exemplo:

```powershell
set PATH=C:\CBMinGW\bin;%PATH%
cd C:\wxWidgets-3.3.2\build\msw
mingw32-make -f makefile.gcc MONOLITHIC=0 SHARED=1 UNICODE=1 BUILD=release -j4
```

Se o MinGW estiver instalado em `C:/Program Files/...`, prefira expor esse diretório por um caminho sem espaços, como `C:/CBMinGW`, para evitar falhas do `mingw32-make`.

### MinGW do Code::Blocks em `Program Files`
O `mingw32-make` pode falhar quando o toolchain está em um caminho com espaço, como `C:/Program Files/CodeBlocks/MinGW`. Uma solução prática é criar um atalho de diretório sem espaços, por exemplo `C:/CBMinGW`, e usar esse caminho no build e no `compilerPath` do VS Code.

### IntelliSense no VS Code
Para este repositório, o IntelliSense precisa destes includes:

- `${workspaceFolder}/fontes`
- `${workspaceFolder}/cbsdk-include`
- `C:/wxWidgets-3.3.2/include`
- `C:/wxWidgets-3.3.2/lib/gcc_dll/mswu`

Também use:

- `compilerPath`: `C:/CBMinGW/bin/g++.exe`
- `defines`: `__WXMSW__`, `WXUSINGDLL`, `_UNICODE`, `UNICODE`

## Fluxo de trabalho

1. Faça um *fork* deste repositório
2. Clone seu fork localmente
3. Crie um branch descritivo: `git checkout -b feature/realce-delegua`
4. Faça suas alterações e escreva testes quando aplicável
5. Abra um Pull Request descrevendo o que foi feito

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
