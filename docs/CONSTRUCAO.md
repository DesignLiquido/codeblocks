# Como compilar o plugin

## Windows

### 1. Instalar dependências

**Code::Blocks com SDK**
- Baixe o instalador em https://www.codeblocks.org/downloads/
- Durante a instalação, selecione "CB SDK" ou "Developer files"
- O SDK ficará em algo como `C:\Program Files\CodeBlocks\sdk\`

**wxWidgets**
- Baixe a versão 3.2.x em https://www.wxwidgets.org/downloads/
- Compile com MinGW (o mesmo usado pelo Code::Blocks):
  ```
  mingw32-make -f makefile.gcc MONOLITHIC=0 SHARED=1 UNICODE=1 BUILD=release
  ```
- Defina a variável de ambiente `WXWIN` apontando para o diretório raiz do wxWidgets

**MinGW**
- Use o MinGW incluído com o Code::Blocks ou instale via MSYS2

### 2. Abrir o projeto

Abra `linguagens-dl.cbp` no Code::Blocks. Verifique as variáveis globais do projeto:
- `$(CBSDK)` → caminho para o SDK do Code::Blocks (ex.: `C:\Program Files\CodeBlocks\sdk`)
- `$(WXWIN)` → caminho para wxWidgets (ex.: `C:\wxWidgets-3.2.0`)

### 3. Compilar

Selecione o target "Release" e pressione `Ctrl+F9` (Build).

O artefato final será `linguagens-dl.cbplugin` na pasta `bin/Release/`.

### 4. Instalar

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
linguagens-dl.cbplugin
├── linguagens-dl.dll       (Windows) / linguagens-dl.so (Linux)
├── linguagens-dl.png       (ícone 80×80, ativo)
├── linguagens-dl-off.png   (ícone 80×80, inativo)
└── linguagens-dl.zip
    ├── manifest.xml
    └── resources/
        └── (arquivos XRC e de configuração de linguagens)
```
