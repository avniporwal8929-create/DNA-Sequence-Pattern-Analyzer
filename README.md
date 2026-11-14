# GENOME·AUTOMATA — DNA Sequence Pattern Analyzer & Mutation Detector
### B.Tech Theory of Computation (TOC) Academic Application Project

A high-performance computational biology application implementing formal language and Theory of Computation (TOC) algorithms in **C++17**, paired with a professional bioinformatics web dashboard.

---

## 1. Project Identity & Purpose

**Project Name:** GENOME·AUTOMATA  
**Subtitle:** DNA Sequence Pattern Analyzer & Mutation Detector  
**Core Language:** C++17 (Strictly mandatory for all theoretical algorithms)  
**Build System:** CMake (v3.14+) / MinGW-w64 GCC 14.2+  
**Architecture:** REST API with embedded native C++ HTTP server & Vanilla JavaScript Web Dashboard  

This is an application-based engineering project demonstrating the direct, practical implementation of Theory of Computation concepts in genomics, rather than just displaying theoretical notes.

---

## 2. Core Application Modules

### Module 01 — Overview Dashboard
- Total sequences analyzed counter.
- Valid vs. invalid sequence metrics.
- Total patterns detected across sequences.
- Mutations identified (substitutions, insertions, deletions).
- Real-time timestamped session activity stream.
- Quick-access launcher cards directly linking into each analysis tool.

### Module 02 — DNA Sequence Validator
- Enter or paste candidate DNA sequences.
- Validates sequence membership in the formal language $\Sigma^*$ over $\Sigma = \{A, T, G, C\}$.
- Automatically converts lowercase input to standard uppercase.
- Rejects non-DNA characters with exact 1-based index and symbol error reporting.
- Reports sequence length and provides quick nucleotide insertion buttons.

### Module 03 — DNA Pattern Detection Engine
- Single pattern detection and comma-separated multi-pattern detection (e.g. `ATG, TATA, GAATTC`).
- Automata-based search: synthesizes prefix NFA $\Sigma^* P$ and converts to DFA via Subset Construction.
- Computes exact 1-based starting and ending coordinates.
- Fully supports and identifies **overlapping pattern occurrences** (e.g. `ATAT` in `ATATAT` yields positions 1 and 3).
- Visual sequence display with highlighted match regions and occurrences breakdown table.
- Dynamically rendered SVG pattern matcher DFA diagram.

### Module 04 — Regex & NFA/DFA Lab
- Regular expression parsing with Shunting-Yard infix to postfix conversion.
- Supports Concatenation, Union (`|`), Kleene Star (`*`), and Grouping Parentheses (`()`).
- Inductive NFA generation via **Thompson's Construction Algorithm** with $\epsilon$-transitions.
- Deterministic DFA generation via **Subset Construction (Rabin-Scott Powerset)** with step-by-step inspector.
- **Candidate String Acceptance Testing:** Test any candidate DNA string directly against the compiled automaton to verify whether $w \in L(M)$ (`ACCEPTED` or `REJECTED`).
- Full transition tables: $\delta_{\text{NFA}}$ and $\delta_{\text{DFA}}$.

### Module 05 — DFA Stepping Simulator
- Step-by-step simulation of state transitions on input DNA strings.
- Interactive controls: **Start**, **Next Step**, **Previous Step**, **Auto Run** (with speed timer), and **Reset**.
- Highlights active state and active edge in real-time on the SVG diagram.
- Interactive nucleotide tape display tracking the read head position.
- Complete step-by-step transition trace table.
- Explicit **ACCEPTED** or **REJECTED** verdict banner.

### Module 06 — Mutation Detector
- Accepts Reference Sequence and Mutated Candidate Sequence.
- Global pairwise sequence alignment via dynamic programming (**Needleman-Wunsch algorithm**).
- Identifies and classifies:
  - Single Nucleotide Polymorphisms (Substitutions / SNPs)
  - Insertions
  - Deletions
- Displays affected coordinates (1-based), reference nucleotide, and mutated nucleotide.
- Visual alignment banner (`Seq1`, match bar `|`, substitution `*`, indel `+`/`-`, `Seq2`).
- Mutation summary metrics and genomic description table.

### Secondary Reference — TOC & Viva Guide
- Concise secondary reference at the bottom covering:
  - Formal definitions ($\Sigma, \Sigma^*$, String, Language, NFA, DFA, $\epsilon$-transitions, $\epsilon$-closure).
  - Project-specific TOC implementation rationale (Why DFAs guarantee linear $O(N)$ matching).
  - Time and space complexity table.
  - Likely viva voce questions and answers about the actual project implementation.

---

## 3. Technology Stack

- **Backend / Core Engine:** C++17
  - Standard Template Library (STL): `std::vector`, `std::set`, `std::map`, `std::queue`, `std::stack`, `std::mutex`
  - Zero external heavy dependencies; uses embedded single-header `httplib.h` and `nlohmann/json.hpp`
- **Build System:** CMake / MinGW-w64 GCC
- **Frontend:** Semantic HTML5, Modular CSS3, Vanilla JavaScript (ES6+)
- **Visualization:** Dynamic SVG graph renderer with zoom, pan, and vector export
- **Testing:** 45 C++ unit test assertions covering all modules

---

## 4. Build and Run Instructions

### 1. Compile the Standalone C++ Server
```powershell
g++ -std=c++17 -O2 -Ibackend/include `
    backend/src/DNAValidator.cpp `
    backend/src/Automaton.cpp `
    backend/src/NFA.cpp `
    backend/src/DFA.cpp `
    backend/src/RegexParser.cpp `
    backend/src/ThompsonConstruction.cpp `
    backend/src/SubsetConstruction.cpp `
    backend/src/PatternDetector.cpp `
    backend/src/SequenceComparator.cpp `
    backend/src/main.cpp `
    -o dna_server.exe -lws2_32 -lcrypt32 -static -static-libgcc -static-libstdc++
```

### 2. Run the Server
```powershell
.\dna_server.exe 8080
```

### 3. Open in Browser
Open [http://localhost:8080/](http://localhost:8080/) in your web browser.

---

## 5. Running C++ Unit Tests

```powershell
g++ -std=c++17 -O2 -Ibackend/include `
    backend/src/DNAValidator.cpp `
    backend/src/Automaton.cpp `
    backend/src/NFA.cpp `
    backend/src/DFA.cpp `
    backend/src/RegexParser.cpp `
    backend/src/ThompsonConstruction.cpp `
    backend/src/SubsetConstruction.cpp `
    backend/src/PatternDetector.cpp `
    backend/src/SequenceComparator.cpp `
    backend/tests/run_tests.cpp `
    -o run_tests.exe -static -static-libgcc -static-libstdc++

.\run_tests.exe
```

**Unit Test Output:**
```
Test Summary: 45 / 45 passed.
>>> ALL TESTS PASSED SUCCESSFULLY! <<<
```

---

## 6. REST API Endpoints

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/health` | Health and engine status |
| `GET` | `/api/stats` | Real session activity metrics |
| `POST` | `/api/dna/validate` | Validates DNA string over $\Sigma = \{A,T,G,C\}$ |
| `POST` | `/api/pattern/detect` | Single pattern detection using prefix DFA |
| `POST` | `/api/pattern/detect-multi` | Multi-pattern detection across DNA sequence |
| `POST` | `/api/regex/compile` | Compiles regex to Thompson NFA and Subset DFA |
| `POST` | `/api/automata/test-string` | Tests candidate string acceptance on automaton |
| `POST` | `/api/automata/simulate` | Generates step-by-step DFA transition trace |
| `POST` | `/api/mutation/compare` | Dynamic programming pairwise sequence alignment |
