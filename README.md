# Know Nepal Engine 🇳🇵

Know Nepal Engine is a standalone C++20 experimental engine for computational workloads related to [Know Nepal](https://know-nepal.vercel.app/).

The project explores native C++ implementations of selected computational workloads while keeping the production Know Nepal platform independent.

## Overview

The engine currently provides a C++ search pipeline that processes and searches structured Know Nepal search data.

It includes:

- Search index preprocessing
- UTF-8-aware normalization and tokenization
- Alias and keyword preprocessing
- ID generation and duplicate detection
- Levenshtein similarity
- Search scoring and ranking
- Exact, prefix, substring, fuzzy, alias, token and keyword matching
- Popularity-based scoring
- Deterministic result ordering
- Search CLI
- TypeScript ↔ C++ parity benchmarking

## Architecture

```text
Search Data
    │
    ▼
Index Builder
    │
    ▼
Processed Search Index
    │
    ▼
C++ Search Engine
    │
    ├── Matching
    ├── Similarity
    ├── Scoring
    └── Ranking
    │
    ▼
Search Results
````

## Search Processing

The index builder preprocesses search entries and generates fields used during search:

* `normalized`
* `nameTokens`
* `normalizedAliases`
* `aliasTokens`

Original entry data is preserved, including metadata, aliases, keywords, locations and popularity.

The search engine implements the computational ranking behavior of Know Nepal's existing TypeScript search.

## Benchmark

The C++ implementation has been compared against the TypeScript reference implementation using the same search dataset and queries.

### Result Parity

* Dataset: **249 entries**
* Queries: **20**
* Result parity: **20/20 (100%)**
* Score differences: **0**
* MatchKind differences: **0**
* Ordering differences: **0**

### Performance

| Metric  | TypeScript |      C++ | Speedup |
| ------- | ---------: | -------: | ------: |
| p50     |   35.44 ms |  8.51 ms |   4.16× |
| p95     |   56.14 ms | 10.32 ms |   5.44× |
| p99     |   76.07 ms | 11.89 ms |   6.40× |
| Average |   39.11 ms |  8.63 ms |   4.53× |

The benchmark measures search computation only and excludes file I/O, JSON parsing, process startup and console output.

These results represent a controlled benchmark and do not imply that the C++ engine currently replaces the production search implementation.

## Project Structure

```text
know-nepal-engine/
├── include/
│   └── engine/
├── src/
├── tests/
├── benchmark/
├── data/
├── .github/
│   └── workflows/
├── CMakeLists.txt
├── vcpkg.json
└── README.md
```

## Development

### Requirements

* C++20
* CMake
* GoogleTest
* nlohmann/json

### Build

Configure and build the project using a C++20-compatible toolchain.

Run tests with:

```bash
ctest --test-dir build --output-on-failure
```

## Testing

The project uses GoogleTest for automated testing.

Current test suite:

**94/94 tests passing**

GitHub Actions automatically builds and tests the project on changes.

## Scope

This repository is intentionally independent from the production Know Nepal platform.

It currently focuses on:

* Computational experimentation
* Search performance
* Behavioral compatibility
* Benchmarking
* Native C++ engineering

Production integration will be considered separately based on measured technical requirements and system trade-offs.

## License

MIT License

Copyright (c) 2026 KnowNepalOrg

```

This is the version I'd use on the GitHub repository.

It tells a new engineer **what it is, what it does, how it works, what the benchmark showed, how to build/test it, and where its boundary is**—without turning the README into a chronological record of everything you did.
```
