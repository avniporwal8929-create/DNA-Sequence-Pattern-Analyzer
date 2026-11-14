# Automata-Based DNA Sequence Pattern Analyzer and Mutation Detection System

A complete, high-performance academic web application implementing formal language and Theory of Computation (TOC) algorithms in **C++17**, paired with a professional, scientific bioinformatics web dashboard.

---

## 1. Project Description

The **Automata-Based DNA Sequence Pattern Analyzer and Mutation Detection System** demonstrates core theoretical principles of Computer Science by applying formal languages, automata theory, and sequence alignment algorithms to computational genomics.

The application allows users to:
1. Validate biological DNA sequences against the formal alphabet $\Sigma = \{A, T, G, C\}$.
2. Synthesize Non-Deterministic Finite Automata (NFAs) from literal patterns and regular expressions via **Thompson's Construction Algorithm**.
3. Convert NFAs into equivalent Deterministic Finite Automata (DFAs) via the **Subset Construction Algorithm (Rabin-Scott Powersett Construction)**, inspecting every $\epsilon$-closure and Move calculation step-by-step.
4. Execute high-throughput, overlapping pattern search across DNA sequences using DFA prefix state machines ($\Sigma^* P$).
5. Interactively simulate DFA execution symbol-by-symbol with dynamic SVG state and edge highlighting.
6. Perform global pairwise sequence alignment and mutation detection using dynamic programming (**Needleman-Wunsch algorithm**).

---

## 2. Problem Statement

DNA sequence analysis in bioinformatics often relies on black-box heuristics or string-matching libraries that obscure the underlying theoretical foundations of computation.

In academic Theory of Computation curricula, concepts such as state determinization, $\epsilon$-transitions, powerset subset construction, and regular expression parsing are often presented only on paper.

This project solves this gap by providing an end-to-end, mathematically rigorous system where all Theory of Computation algorithms are implemented strictly in **C++17**, exposed via a high-performance REST API, and visualized dynamically in an interactive bioinformatics dashboard.

---

## 3. Technology Stack

- **Core Programming Language:** C++17
- **Standard Library:** Modern C++ STL (`std::vector`, `std::set`, `std::map`, `std::queue`, `std::stack`, `std::mutex`)
- **Backend Architecture:** REST API with lightweight embedded C++ HTTP server (`httplib.h`) and `nlohmann/json.hpp`
- **Build System:** CMake (v3.14+) / GCC (MinGW-w64 14.2+)
- **Frontend:** Semantic HTML5, Modular CSS3 (Scientific Bioinformatics Deep Midnight Green / Lime palette), Vanilla JavaScript (ES6+)
- **Automata Visualization:** Custom SVG dynamic layout graph visualizer with zoom, pan, and vector export
- **Testing:** C++ unit test suite covering 43 test assertions

---

## 4. Theory of Computation (TOC) Concepts & Algorithms

### Formal Alphabet and Language
- **Alphabet ($\Sigma$):** $\Sigma = \{A, T, G, C\}$ representing the four nitrogenous nucleotides.
- **Language ($L$):** Subsets of $\Sigma^*$ representing functional genomic sequences, motifs, and promoter sites.

### Regular Expression Parsing & Thompson's Construction
- Supported operators: Concatenation (implicit), Union (`|`), Kleene Star (`*`), and Parentheses (`()`).
- **Infix to Postfix:** Preprocessed with explicit concatenation (`.`) and parsed via the **Shunting-Yard Algorithm**.
- **Thompson's Construction:** Inductively builds an NFA fragment with $\epsilon$-transitions for each operator.

### Subset Construction Algorithm (NFA $\to$ DFA)
- **$\epsilon$-Closure:** Computes all states reachable via zero or more $\epsilon$-transitions using Breadth-First Search (BFS).
- **Move Function:** $\text{Move}(T, a) = \bigcup_{s \in T} \delta_{\text{NFA}}(s, a)$ for symbol $a \in \{A, T, G, C\}$.
- **Powerset State Mapping:** Maps each unique subset of NFA states to an equivalent DFA state $S_0, S_1, \dots$.
- **Acceptance Invariant:** A DFA state is accepting if and only if its state subset contains at least one NFA final state.

### Substring Pattern Detection Automaton
- Builds an NFA with a self-loop on all $\Sigma$ at the initial state: recognizes $\Sigma^* P$.
- Converted via Subset Construction to an equivalent DFA that tracks pattern prefixes in linear $O(N)$ time, detecting exact and overlapping matches.

### Pairwise Sequence Alignment (Mutation Detection)
- Dynamic programming global alignment (**Needleman-Wunsch algorithm**) with backtracking to classify:
  - Matches
  - Single Nucleotide Polymorphisms (Substitutions / SNPs)
  - Insertions
  - Deletions

---

## 5. Project Directory Structure

```
TOC PROJECT/
├── CMakeLists.txt                 # Root CMake configuration
├── README.md                      # Comprehensive project documentation
├── backend/
│   ├── CMakeLists.txt             # Backend CMake configuration
│   ├── include/
│   │   ├── Automaton.h            # Base automaton class & SVG graph structures
│   │   ├── DFA.h                  # Deterministic Finite Automaton class & simulation
│   │   ├── DNAValidator.h         # DNA nucleotide sequence validator
│   │   ├── NFA.h                  # Non-Deterministic Finite Automaton with epsilon transitions
│   │   ├── PatternDetector.h      # Automata-based pattern detection engine
│   │   ├── RegexParser.h          # Shunting-yard infix to postfix regex parser
│   │   ├── SequenceComparator.h   # Needleman-Wunsch sequence alignment & mutation detector
│   │   ├── SubsetConstruction.h   # Rabin-Scott powerset conversion engine
│   │   ├── ThompsonConstruction.h # Thompson's regex to NFA construction
│   │   ├── httplib.h              # Embedded lightweight HTTP server
│   │   └── json.hpp               # nlohmann JSON serialization
│   ├── src/
│   │   ├── Automaton.cpp
│   │   ├── DFA.cpp
│   │   ├── DNAValidator.cpp
│   │   ├── NFA.cpp
│   │   ├── PatternDetector.cpp
│   │   ├── RegexParser.cpp
│   │   ├── SequenceComparator.cpp
│   │   ├── SubsetConstruction.cpp
│   │   ├── ThompsonConstruction.cpp
│   │   └── main.cpp               # C++ REST API server & static file host
│   └── tests/
│       ├── run_tests.cpp          # Complete test runner (43 test assertions)
│       ├── test_dna_validator.cpp
│       ├── test_nfa.cpp
│       ├── test_pattern_detector.cpp
│       ├── test_regex.cpp
│       ├── test_sequence_comparator.cpp
│       └── test_subset_construction.cpp
└── frontend/
    ├── index.html                 # Main web dashboard interface
    ├── css/
    │   └── style.css              # Scientific deep midnight green & lime UI theme
    └── js/
        ├── api.js                 # REST API communication client
        ├── app.js                 # Application navigation, presets, and heartbeat
        ├── automata.js            # Regex compiler & subset construction inspector
        ├── comparator.js          # Mutation detector & pairwise alignment
        ├── dashboard.js           # Real session metrics & activity history
        ├── pattern.js             # Automata pattern detection controller
        ├── simulator.js           # Interactive DFA stepping simulator
        ├── validator.js           # DNA sequence validation controller
        └── visualization.js       # Dynamic SVG automata renderer (zoom, pan, export)
```

---

## 6. Build and Run Instructions

### Prerequisites
- **Compiler:** GCC 8.0+ or Clang 10+ supporting C++17. (On Windows: MinGW-w64).
- **Libraries:** Winsock (`-lws2_32 -lcrypt32` on Windows) or pthread on POSIX.

### Option A: Direct Compilation with GCC / MinGW (Recommended)

To compile the standalone backend server:
```powershell
# From project root directory
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

To run the backend server:
```powershell
.\dna_server.exe 8080
```
Then open your web browser to:
```
http://localhost:8080/
```

### Option B: Build with CMake

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## 7. Running Unit Tests

To compile and run the comprehensive C++ test suite:
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

### Verified Test Output
```
========================================================
  THEORY OF COMPUTATION - DNA ANALYZER C++ UNIT TESTS  
========================================================

=== 1. Testing DNA Sequence Validator ===
  [PASS] Valid DNA sequence accepted
  [PASS] Lowercase input converted to uppercase
  [PASS] Invalid nucleotide rejected with correct position
  [PASS] Empty sequence handled
  [PASS] Whitespace-only input rejected

=== 2. Testing Regular Expression Parser ===
  [PASS] Simple concatenation: ATG
  [PASS] Union operator: A|T
  [PASS] Kleene star operator: AT*
  [PASS] Parentheses: (A|T)G
  [PASS] Complex regex: (A|T)*(G|C)
  [PASS] Syntax error detected: unclosed '('
  [PASS] Syntax error detected: empty '()'
  [PASS] Syntax error detected: leading '*'

=== 3. Testing Thompson Construction & Subset Construction ===
  [PASS] NFA recognizes AG
  [PASS] NFA recognizes TG
  [PASS] NFA rejects AT
  [PASS] NFA rejects G
  [PASS] Subset construction produced valid DFA
  [PASS] DFA recognizes AG
  [PASS] DFA recognizes TG
  [PASS] DFA rejects AT
  [PASS] DFA rejects G
  [PASS] DFA rejects AGG
  [PASS] A* accepts empty string
  [PASS] A* accepts A
  [PASS] A* accepts AAAA
  [PASS] A* rejects T

=== 4. Testing Automata-Based Pattern Detector ===
  [PASS] Overlapping pattern detection success
  [PASS] Overlapping occurrences count == 2
  [PASS] Overlapping starting positions 1 and 3
  [PASS] Multiple occurrences found (3 times)
  [PASS] Starting positions 1, 6, 9
  [PASS] Pattern at beginning
  [PASS] Pattern at end
  [PASS] Pattern not found handled
  [PASS] Pattern longer than sequence handled

=== 5. Testing Sequence Comparator (Mutation Detection) ===
  [PASS] Identical sequences have 0 differences
  [PASS] Identical sequences similarity is 100%
  [PASS] Substitution detected
  [PASS] Substitution position 4 (C -> A)
  [PASS] Insertion detected
  [PASS] Deletion detected
  [PASS] Different sequence lengths handled

--------------------------------------------------------
Test Summary: 43 / 43 passed.
--------------------------------------------------------
>>> ALL TESTS PASSED SUCCESSFULLY! <<<
```

---

## 8. REST API Documentation

| Method | Endpoint | Description | Request Body Example |
|---|---|---|---|
| `GET` | `/api/health` | Service health status | None |
| `GET` | `/api/stats` | Real session activity metrics | None |
| `POST` | `/api/dna/validate` | Validates DNA string over $\Sigma$ | `{"sequence": "ATGCGATG"}` |
| `POST` | `/api/pattern/detect` | Automata-based pattern search | `{"sequence": "ATGCGATGATG", "pattern": "ATG"}` |
| `POST` | `/api/regex/compile` | Compiles regex to NFA and DFA | `{"regex": "(A\|T)G"}` |
| `POST` | `/api/automata/nfa` | Generates Thompson NFA | `{"input": "ATG", "type": "literal"}` |
| `POST` | `/api/automata/convert` | Runs Subset Construction | `{"input": "(A\|T)G", "type": "regex"}` |
| `POST` | `/api/automata/simulate`| Simulates DFA on input sequence | `{"input": "(A\|T)G", "sequence": "AG"}` |
| `POST` | `/api/mutation/compare` | Compares two sequences | `{"seq1": "ATGCGT", "seq2": "ATGAGT"}` |

---

## 9. Limitations & Future Scope

### Current Limitations
1. **Alphabet:** Strictly restricted to the standard 4-character DNA alphabet $\Sigma = \{A, T, G, C\}$. Extended IUPAC degenerate codes (`R`, `Y`, `N`) are not currently treated as matching wildcards.
2. **Regex Operators:** Focused on foundational Theory of Computation regular expression operators (Concatenation, Union `|`, Kleene Star `*`, Parentheses `()`). Extended Perl-compatible regular expressions (PCRE) lookaheads or backreferences are intentionally omitted to preserve formal regular language equivalence.
3. **Medical Disclaimer:** Sequence comparison is provided strictly for academic and educational demonstration; it does not replace clinical bioinformatics software.

### Future Scope
- Implementation of Hopcroft's or Moore's DFA State Minimization Algorithm.
- Extension to Protein / Amino Acid alphabets ($\Sigma = 20$ amino acids).
- Export to Graphviz DOT and TikZ LaTeX formats for academic thesis publication.
