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
- Inline `?` tooltip explaining transition syntax (`from symbol to`)
- Test string input with step-by-step animation
- Canvas rendering: states as circles, transitions as labeled arrows, self-loops
- Color-coded current state (green highlight), accepted (green), rejected (red)
- Accepted runs end with a bright green final-state highlight on the terminal accepting node
- Step mode (manual) and Run mode (auto, configurable speed)
- Trace log showing every state transition
- Test-case selector dropdown that loads nine curated preset DFAs covering renderer edge-cases (bidirectional adjacent, bidirectional far across intermediates, multi-symbol parallel edges, all self-loops, 5-state chain with wrap-around, 6-state circle layout with diametric pairs, single state self-loop, dead-state rejection, and a 12-state stress case)
- Page opens directly on the graph/import console; there is no hero or service-style landing section

**Renderer architecture** (refactored 2026-05-12):
- `layoutStates(states)` adapts node placement by graph size: centered single state, two-state row, three/four-state diamond, and circular layouts for larger user-authored transition tables.
- `buildEdges()` groups `dfa.delta` by ordered `(from, to)` pair and merges symbols into one comma-separated label.
- `routeEdges(edges, positions)` computes deterministic geometry before drawing: straight short edges, mirrored reverse-pair curves, lifted long/intermediate-crossing curves, and free-side self-loops.
- `drawRoutedEdge(edge, color)` consumes precomputed route objects so arrowheads use tangent direction and labels use clamped route points rather than ad hoc draw-time math.
- `drawState(s, p)` renders start, final, current, accepted, and rejected state styling in the tactical color system.
- Canvas dimensions use device-pixel-ratio scaling and `height = max(380, width * 0.56)` so dense graphs have room for labels and loops.

**Visual system** (redesigned 2026-05-12):
- **Vibe — Tactical Telemetry**: off-black CRT substrate, phosphor-white text, aviation-red accent, and one terminal-green status readout for active/accepted automata state.
- **Typography**: Archivo Black for heavy structural headings and JetBrains Mono / IBM Plex Mono style stacks for controls, labels, chips, and data readouts.
- **Layout — Industrial Grid**: square-corner panels, 1px compartment lines, crosshair markers, dense metadata, and no glassmorphism, rounded pills, or blue/purple gradients.
- **Canvas treatment**: graph panel includes a low-contrast measurement grid and halo-stroked labels for readable dense transition diagrams.
- **Accessibility**: controls keep visible focus rings, high-contrast button text, semantic sections, skip-to-content navigation, and keyboard-friendly form controls.

---

## Task 2: C++ Console Programs

All programs scan the local `input/` directory in their own folder and process every `.txt` file found there, writing matching reports to `output/`.
Compile: `g++ -std=c++17 -o program main.cpp`
Run: `./program` (or `program.exe` on Windows)
Bundled testcase coverage now includes `tc01`-`tc15` for every task 2 exercise; `tc11`-`tc15` are additional arbitrary edge/corner cases added during the 2026-04-20 correctness audit.
There is also a teacher-facing guide at [`src/task2/guideline.md`](c:/Users/admin/automata/src/task2/guideline.md) that explains folder hierarchy, input formats, and what the `output/` folders contain.

### Input Format Conventions

**Automata programs** (`01`, `02`, `03`, `04`, `07`) use labeled sections. Sections may appear in any order. The `states` line is optional; if omitted, the number of states is auto-discovered from the highest state ID referenced in transitions, initial, and finals.

```text
states 0 1 2        # optional - if omitted, N = max_state_id + 1
alphabet a b
transitions 4
0 a 1
0 b 0
1 a 1
1 b 2
initial 0
finals 2
```

Meaning:
- `0 a 1` means `q0 --a--> q1`
- `finals 2` means state `2` is final
- `finals 1 3` means states `1` and `3` are final
- `finals -` means there is no final state

State identifiers are bare integers. All referenced states must be ≤ N-1. When `states` is omitted, N is computed as `max(state ids seen) + 1`.

For `01_dfa_acceptance`, add:
```text
strings 3
""
ab
aba
```
Use a blank line or `""` for the empty string. Lambda rows are invalid in DFA inputs.

For `02_nfa_extended`, add:
```text
queries 3
0 ""
0 ab
1 b
```
Lambda can be written as `""`, `lambda`, or `lamda`.

For `03_nfa_to_dfa`, malformed transitions, invalid initial states, and invalid final states fail fast with `Input error:`.

**RE** (program `05`): labeled regex line.
```text
regex (ab+~)*
```
Operators: `+` union, `*` star, `()` grouping, `~` lambda. Concat is implicit.

**Grammar** (program `06`): labeled grammar sections.
```text
variables S A
terminals a b
start S
productions 3
S a A
S b -
A a S
```

All console programs fail fast with `Input error:` when a required labeled section or field is missing or malformed.

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
