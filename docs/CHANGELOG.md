# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Enterprise-grade directory structure (`src/`, `tests/`, `docs/`, `scripts/`).
- `test_collatz.cpp` custom unit testing framework.
- GitHub Actions CI pipeline for automated testing.
- `SECURITY.md`, `CONTRIBUTING.md`, and `ARCHITECTURE.md`.
- Win32 API multi-threading support for legacy MinGW environments.

### Changed
- Monolithic `main.cpp` refactored into distinct, modular headers under `src/collatz/`.
- Updated `CMakeLists.txt` to support building both the main application and test binaries.
- Re-styled README.md to adhere to enterprise recruiter-ready standards, removing AI-generated tone.

### Fixed
- Added robust `std::invalid_argument` error handling for CLI inputs.
- Wrapped single-threaded `unordered_map` tests in `std::bad_alloc` handlers to safely bypass the 2GB 32-bit process memory ceiling.
