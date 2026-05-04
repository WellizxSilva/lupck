<p align="center">
    <img src="./assets/lupck.png" alt="lupck" height="250">
</p>
<p align="center">
    Lightweigth utility designed to bundle Lua scripts and assets into a single standalone executable
</p>
<p align="center">
  <img src="https://img.shields.io/badge/Lua-2C2D72?style=for-the-badge&logo=lua&logoColor=white" alt="Lua" />
  <img src="https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++" />
  <img src="https://img.shields.io/badge/Windows%2011-0078D6?style=for-the-badge&logo=windows11&logoColor=white" alt="Windows 11" />
  <img src="https://img.shields.io/badge/Linux-FCC624.svg?style=for-the-badge&logo=Linux&logoColor=black" al="Linux"/>
  <img src="https://img.shields.io/badge/license-MIT-green?style=for-the-badge" alt="License" />
  <img src="https://img.shields.io/luarocks/v/WellizxSilva/lupck?style=for-the-badge&logo=lua" alt="LuaRocks Version" />  
  <!--
   <img src="https://img.shields.io/github/actions/workflow/status/WellizxSilva/lupck/ci.yml?style=for-the-badge&logo=github" alt="Build Status" />
   --->
</p>

### Lupck a simple lightweight utility designed to bundle Lua scripts and assets into a single, standalone executable or a custom `.lpk`package.

---

## Features
- **Zero-Extraction Execution:** Runs scripts directly from memory buffers.
- **Asset Bundling**: Pack multiple Lua files and resources into one binary.
- **Standalone Compilation:** Merges the **Base Engine** with your package to create a self-running .exe or ELF binary.

## 🛠 How it Works
The project is split into three core components:

1.**The Packer:** Collects files, calculates hashes, builds a file index, and generates a `.lpk` binary.
2. **Base Engine:** Pre-compiled host binary containing the Lua interpreter and Virtual File System (VFS) loader.
3. **Standalone Compilation:** Merge the **Base Engine** with your package to create a self-running `.exe` or ELF binary.


### Binary Structure
When you compile a standalone app, the final file anatomy looks like this:
[ Base Engine ] + [ Encapsulated .lpk Data ] + [ 8-byte Marker ] + [ 8-byte PkgSize ]

---

## Commands
**1. Packing a project**
Bundle your assets into a package. Lupck will automatically look for `lupck.config.lua`
```sh
# Logging is enabled by default. Use '--log false' to silence.
lupck pack my_app.lpk scripts/ assets/ --log false
```
**2. Compiling a standalone Executable**
Fuse the `.lpk` package with the pre-compiled binary **(Base)**.
```sh
# Result is a .exe on Windows or an ELF binary on Linux
# Use '--log false' to disable compilation output
lupck compile my_app.lpk my_app.exe --base bin/lupck_base.exe --log false
```
3. Running a package
Runs a `.lpk` file directly using the lupck environment.
```sh
lupck run my_app.lpk
```
**4. Unpacking**
Extract the contents of a package for inspection or debugging.
```sh
lupck unpack my_app.lpk -o extracted_files/
```

## Configuration (lupck.config.lua)
Every package must include a configuration file to define the entry point and metadata.
You can either use the `lupck` helper:
```lua
local lupck = require("lupck")

return lupck.config.define({
    entry = "main.lua",      -- Main script to execute
    version = "1.0.0",       -- App version
    author = "Developer",
})
```
Or simply return a plain Lua table (useful if Lupck is not installed in your environment):
```lua
return {
    entry = "main.lua",      -- Main script to execute
    version = "1.0.0",       -- App version
    author = "Developer",
}
```

## Installation
1. Clone the repository
```sh
git clone https://github.com/WellizxSilva/lupck.git
cd lupck
```
2. Build the project
Ensure you have a **C++20** compiler and Lua **5.4+** installed.
Configure paths in the `Makefile` to point to your Lua headers and libraries.
```sh
make clean
make
```
> Note: This will generate an ELF binary on Linux and a `.exe` on Windows (via MinGW/MSYS2).

3. **(Optional) Install lua lib via LuaRocks for config autocomplete and integration**
```sh
luarocks install lupck
```

## 🏗 Build & Architecture
**Requirements**
- **C++20** compliant compiler (GCC 11+, Clang 13+, or MSVC 2022).
- **Lua 5.4+** installed (Header files and Static/Dynamic libraries).

## Makefile Configuration
Before building **Lupck**, you must configure the dependency paths in the `Makefile` to ensure the linker finds Lua correctly on your system.

## Makefile Configuration
Before building **Lupck**, you must configure the dependency paths in the `Makefile` to ensure the linker finds Lua correctly on your system.

## ⚙️ Linux Setup Notes
When building **Lupck** on Linux or WSL, a few additional steps are required to ensure the environment is correctly configured:

**Dependencies**
- `build-essential` (compiler toolchain)
- `g++` (C++20 support)
- `make`
- `liblua5.4-dev` (Lua headers and libraries)

Install with:
```sh
sudo apt update
sudo apt install -y build-essential g++ make liblua5.4-dev
```
**Automatic Setup (Recommended)**
You can run the provided `setup.sh` script to automatically configure your shell and build the project:
```sh 
chmod +x ./setup.sh
./setup.sh
```
This script will:
- Detect your current shell (`zsh`, `bash`, or fallback to `~/.profile`).
- Append `eval "$(luarocks path)"` to the correct configuration file.
- Run `make clean && make` to build the project.

**Manual Setup (Alternative)**
If you prefer, you can manually edit your shell configuration (`~/.zshrc`, `~/.bashrc`, or `~/.profile`) and add:
```sh
eval "$(luarocks path)"
```
Then build directly with:
```sh
make clean
make
```

## 📄 Output Artifacts
- `.lpk:` Raw resource package.
- `.exe`/**ELF:** Final standalone application.
- `blueprint.yaml:` Metadata, offsets, and SHA-256 hashes for auditing.

---

## 🤝 Contribuiting
Found a bug or have a feature request? Please [open a new issue](https://github.com/WellizxSilva/lupck/issues). Contributions are welcome!

## 📜 License
Distributed under the *MIT* License. See [LICENSE](./LICENSE) for more information.

---


## Disclaimer
Although **Lupck** has the perspective to grow into a real-world utility, this project is currently intended **primarily for testing, experimentation, and study purposes**.  
Features such as bytecode execution, integration with external C libraries, and encryption/decryption mechanisms are planned, but not yet stable or production-ready.  

Use **Lupck** at your own discretion, and avoid relying on it for critical or sensitive deployments until future versions mature.
