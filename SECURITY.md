# Security Policy

## Supported Versions

Currently, only the `main` branch of this repository is supported with security updates.

## Reporting a Vulnerability

We take security seriously. Since this project is an open-source mathematical toolkit, the primary risk vectors are Denial of Service (DoS) via integer overflow, memory leaks during massive limits, or improper command-line argument parsing.

Please **do not** report security vulnerabilities via public GitHub issues. 

Instead, report them privately via email to the repository maintainers or use GitHub's private vulnerability reporting feature.

### Evaluation Criteria
A valid vulnerability for this repository must demonstrate:
- Arbitrary code execution via CLI inputs.
- Heap or Stack corruption bypassing the existing `std::bad_alloc` handlers.
- Unsafe memory bounds access resulting in data leaks outside process scope.

Expect a response acknowledging the report within 48 hours.
