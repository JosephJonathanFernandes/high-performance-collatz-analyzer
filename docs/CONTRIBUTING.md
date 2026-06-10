# Contributing to Collatz Analyzer

First off, thank you for considering contributing to this project!

## 1. Where to Start
Please check the open issues before beginning any major work to avoid duplicating effort. If you are planning a major architectural change, open an issue first to discuss it with the maintainers.

## 2. Development Standards
- **C++ Standard:** We target C++14 to maintain maximum compatibility with older enterprise compilers (like MinGW 6.3.0).
- **No Heavy Dependencies:** We strictly avoid heavy third-party libraries (Boost, GTest) to ensure the repository remains ultra-portable and instantly compilable.
- **Header-Only Modules:** Keep logic cleanly separated into header files under `src/collatz/` unless source compilation separation is mathematically justified.
- **SOLID Principles:** Ensure new logic is fully modular and separated from the UI/CLI.

## 3. Pull Request Process
1. Fork the repo and create your branch from `main`.
2. Ensure you have added unit tests to `tests/test_collatz.cpp` covering your new functionality.
3. Run the local tests (`scripts/test.bat`) to verify zero regressions.
4. Issue your pull request. The CI pipeline will automatically run build and tests.
5. A maintainer will review your code for efficiency and clean architecture.
