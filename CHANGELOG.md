# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Planned
- **Bytecode Support:** Option to package and run precompiled Lua bytecode for faster startup and obfuscation.
- **C Library Integration:** Ability to bundle and link external C libraries directly into the package/executable.
- **Encryption Keys:** Support for encrypt/decrypt keys to protect packaged code and assets, preventing unauthorized binary inspection.
- **Advanced Security:** Encrypted package headers and payloads to avoid direct access to raw Lua source.
- **Developer Experience Improvements:**
  - Allow inclusion patterns like `/assets`, `scripts/*.lua`, making it easier to package entire directories or globbed sets of files.
  - Automatic dependency resolution: Lupck will parse the entry point and map all required modules (`require("app/lib/utils.lua")`, etc.) without needing to manually list every file.
  - Smarter packaging rules to reduce boilerplate and simplify project setup.
---

## [0.1.0] - 2026-05-04
### Added
- **Initial Release:** First functional version of Lupck.
- **Packer:** Ability to bundle Lua scripts and assets into `.lpk` packages.
- **Runtime Engine:** Base binary capable of loading Lua scripts from a virtual file system (VFS).
- **Custom Searcher:** Injection of Lupck searcher into `package.searchers` at position 2, enabling `require()` to load modules from the package.
- **CLI Commands:** Implemented `pack`, `unpack` `run`, and `compile` commands for basic workflow.
