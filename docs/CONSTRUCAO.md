# Como compilar o plugin

## Windows

### 1. Instalar dependências

**Code::Blocks com SDK**
- No Windows, use o **MSYS2 UCRT64** para obter bibliotecas compatíveis com o plugin.
- Abra o terminal **MSYS2 UCRT64** e instale:
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-codeblocks mingw-w64-ucrt-x86_64-wxwidgets3.2-msw
  ```
- O repositório já inclui cópias vendorizadas dos headers do Code::Blocks em `cbsdk-include/` e dos headers do `wxScintilla` em `sdk/wxscintilla/include/`, então o build do projeto não depende dos headers em `C:\Program Files\CodeBlocks`.

**wxWidgets**
- **Versão obrigatória: 3.2.x.** O SDK do Code::Blocks 25.03 foi compilado contra wxWidgets 3.2 e é incompatível com wxWidgets 3.3.x (mudanças de API como `wxColourImpl` causam erros de compilação). Use exatamente a série 3.2.
- Se você instalou `mingw-w64-ucrt-x86_64-wxwidgets3.2-msw` pelo MSYS2, use diretamente o prefixo `C:\msys64\ucrt64` como `WXWIN`.
- Os arquivos importantes ficam em:
  - headers: `C:\msys64\ucrt64\include`
  - `setup.h`: `C:\msys64\ucrt64\lib\gcc_x64_dll\mswu\wx\setup.h`
  - bibliotecas: `C:\msys64\ucrt64\lib`

**MSYS2 UCRT64 (recomendado)**

> **Atenção:** evitar misturar `C:\Program Files\CodeBlocks`, wxWidgets compilado no MSYS2 e compilador de outro toolchain. O plugin precisa ser compilado e ligado com bibliotecas do mesmo ambiente ABI. Para Windows, o caminho mais seguro é usar **UCRT64** do início ao fim.

1. Baixe e instale o MSYS2 em https://www.msys2.org/
2. Abra o terminal **MSYS2 UCRT64**
3. Instale as dependências:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-codeblocks mingw-w64-ucrt-x86_64-wxwidgets3.2-msw
   ```
4. No Code::Blocks: **Settings → Compiler → GNU GCC Compiler → Toolchain executables**
   - Defina o diretório de instalação como `C:\msys64\ucrt64`
   - Confirme que `gcc.exe` e `g++.exe` existem em `C:\msys64\ucrt64\bin\`
   - Clique em **Set as default**
5. Defina as variáveis de ambiente:
   ```powershell
   setx CBSDK "C:\msys64\ucrt64"
   setx WXWIN "C:\msys64\ucrt64"
   ```

### 2. Abrir o projeto

Abra `linguagens-dl.cbp` no Code::Blocks. Verifique as variáveis globais do projeto:
- `$(CBSDK)` → prefixo do MSYS2 UCRT64 com as bibliotecas do Code::Blocks (ex.: `C:\msys64\ucrt64`)
- `$(WXWIN)` → prefixo do MSYS2 UCRT64 com as bibliotecas do wxWidgets (ex.: `C:\msys64\ucrt64`)

Para configurar essas variáveis, acesse **Settings → Global Variables...** no menu do Code::Blocks. Na tela que abrir:

1. Clique em **New** e digite o nome da variável (ex.: `CBSDK`)
2. No campo **base path**, informe o caminho correspondente (ex.: `C:\msys64\ucrt64`)
3. Repita para `WXWIN` (ex.: `C:\msys64\ucrt64`)
4. Clique em **Close**

> **Atenção:** na prática, o Code::Blocks resolve `$(CBSDK)` e `$(WXWIN)` a partir das **variáveis de ambiente do Windows**, não das Global Variables. Se o projeto não encontrar os caminhos corretos, defina as variáveis via terminal (como administrador) e reinicie o Code::Blocks:
> ```powershell
> setx CBSDK "C:\msys64\ucrt64"
> setx WXWIN "C:\msys64\ucrt64"
> ```

### 3. Compilar

Selecione o target "Release" e pressione `Ctrl+F9` (Build).

O binário gerado será `bin/Release/LinguagensDL.dll`.

### 4. Empacotar

Depois do build, gere o pacote instalável:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\empacotar-plugin.ps1 `
  -BinaryPath .\bin\Release\LinguagensDL.dll `
  -OutputDir dist `
  -Version 0.1.0
```

O artefato final será `dist/LinguagensDL-0.1.0.cbplugin`.

### 5. Instalar

No Code::Blocks: **Plugins → Manage Plugins → Install new** e selecione o arquivo `.cbplugin`.

---

## Linux

### 1. Instalar dependências

**Ubuntu / Debian:**
```bash
sudo apt-get install codeblocks codeblocks-dev libwxgtk3.2-dev build-essential
```

**Fedora / RHEL:**
```bash
sudo dnf install codeblocks codeblocks-devel wxGTK-devel gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S codeblocks wxwidgets-gtk3 base-devel
```

### 2. Compilar

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Ou abra `linguagens-dl.cbp` no Code::Blocks e compile normalmente.

### 3. Instalar

```bash
sudo make install
```

Ou via Code::Blocks: **Plugins → Manage Plugins → Install new**.

---

## macOS

### 1. Instalar dependências

```bash
brew install codeblocks wxwidgets
```

> **Nota:** O suporte ao macOS é experimental. O Code::Blocks no macOS tem limitações conhecidas.

### 2. Compilar

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DwxWidgets_ROOT_DIR=$(brew --prefix wxwidgets)
make -j$(sysctl -n hw.ncpu)
```

---

## Variáveis de CMake relevantes

| Variável | Padrão | Descrição |
|----------|--------|-----------|
| `CBSDK_PATH` | `/usr/include/codeblocks` | Caminho para os headers do CB SDK |
| `WXWIDGETS_VERSION` | `3.2` | Versão do wxWidgets |
| `CMAKE_BUILD_TYPE` | `Release` | `Debug` ou `Release` |

---

## Estrutura do artefato gerado

O plugin é empacotado como um arquivo `.cbplugin` (zip renomeado):

```
LinguagensDL-0.1.0.cbplugin
├── LinguagensDL.dll
├── LinguagensDL.png
├── LinguagensDL-off.png
└── LinguagensDL.zip
    ├── manifest.xml
    └── palavras-chave/
        ├── delegua.xml
        ├── pitugues.xml
        └── ...
```

> **Importante:** o nome-base do plugin precisa ser `LinguagensDL`. O instalador do Code::Blocks deriva os nomes internos do `.dll` e do `.zip` a partir do nome do arquivo `.cbplugin`; por isso, nomes com hífen no identificador do plugin podem quebrar a instalação.
