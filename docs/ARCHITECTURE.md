# Project Architecture

## Overview

This project implements all algorithms from Chapters 2 (Finite Automata) and 3 (Regular Grammar) of the Theoretical Computer Science course.

Deliverables:
- **Task 1**: A local visualization website for DFA String Acceptance (Chapter 2, slides 20-23).
- **Task 2**: Seven standalone C++ console programs, each implementing one algorithm.

---

## Directory Structure

```
automata/
├── src/
│   ├── task1/
│   │   └── index.html          # Task 1: DFA visualization (open in browser)
│   └── task2/
│       ├── 01_dfa_acceptance/  # DFA string acceptance check
│       ├── 02_nfa_extended/    # NFA extended transition function delta*
│       ├── 03_nfa_to_dfa/      # NFA to DFA via subset construction
│       ├── 04_dfa_minimize/    # DFA minimization (mark + reduce)
│       ├── 05_re_to_nfa/       # Regex to NFA (Thompson's construction)
│       ├── 06_grammar_to_nfa/  # Right-linear grammar to NFA
│       └── 07_nfa_to_grammar/  # NFA to right-linear grammar
├── docs/
│   ├── ARCHITECTURE.md
│   ├── MEMORY.md
│   └── TEST_REPORT.md
└── resources/
    ├── 2-Finite Automata.pdf
    └── 3-Regular Language and Regular Grammar.pdf
```

---

## Task 1: DFA Visualization (src/task1/index.html)

**Algorithm**: DFA String Acceptance (slides 20-23, Chapter 2)

**Technology**: Self-contained HTML/CSS/JS — no dependencies, opens directly in any browser.

**Features**:
- User-editable DFA definition (alphabet, transitions, initial/final states)
- Test string input with step-by-step animation
- Canvas rendering: states as circles, transitions as labeled arrows, self-loops
- Color-coded current state (blue highlight), accepted (green), rejected (red)
- Step mode (manual) and Run mode (auto, configurable speed)
- Trace log showing every state transition

---

## Task 2: C++ Console Programs

All programs read from `input.txt` in the same directory.
Compile: `g++ -std=c++17 -o program main.cpp`
Run: `./program` (or `program.exe` on Windows)

### Input Format Conventions

**DFA** (programs 01, 04):
```
N M              # N states (0..N-1), M alphabet symbols
s0 s1 ... sM-1   # alphabet characters
E                # number of edges
from sym_idx to  # sym_idx in [0..M-1]
q0               # initial state
F_count
f0 f1 ...
[K + strings]    # 01 only: test strings
```

**NFA** (programs 02, 03, 07):
Same as DFA but `sym_idx = M` means lambda transition.

**RE** (program 05): single regex line.
Operators: `+` union, `*` star, `()` grouping, `~` lambda. Concat is implicit.

**Grammar** (program 06):
```
V_count
varname0 ...       # index 0 = start symbol
T_count
t0 t1 ...
P_count
LHS_idx t_idx... [rhs_var_idx | -1]   # -1 = terminal-only production
```

---

## Algorithm Reference

| # | Program | Algorithm | Slides |
|---|---------|-----------|--------|
| 1 | 01_dfa_acceptance | DFA simulation | Ch2 20-23 |
| 2 | 02_nfa_extended   | NFA delta*(q,w): lambda-closure + move | Ch2 38-42 |
| 3 | 03_nfa_to_dfa     | Subset construction | Ch2 45-46, 53 |
| 4 | 04_dfa_minimize   | mark() + reduce() | Ch2 59-60, 64, 72 |
| 5 | 05_re_to_nfa      | Thompson's construction re2nfa() | Ch3 17-20 |
| 6 | 06_grammar_to_nfa | G_R to nfa() | Ch3 39-42 |
| 7 | 07_nfa_to_grammar | nfa2G_R | Ch3 44-45 |

---

## Key Data Structures

**NFA** (programs 02, 03, 07):
```cpp
struct NFA {
    int N, M;                          // states, alphabet size
    vector<char> alpha;                // alpha[i]; index M = lambda
    vector<vector<set<int>>> delta;    // delta[state][sym] = set of next states
    int q0;
    set<int> F;
};
```

**DFA** (programs 01, 04):
```cpp
struct DFA {
    int N, M;
    vector<char> alpha;
    vector<vector<int>> delta;         // delta[state][sym] = next, -1 = dead
    int q0;
    vector<bool> isFinal;
};
```

**NFA Fragment** (Thompson's construction, program 05):
```cpp
struct Frag { int start, accept; };
vector<Edge> edges;   // global edge list built during construction
```