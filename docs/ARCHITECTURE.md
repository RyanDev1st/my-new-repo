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
│   │   ├── index.html          # Task 1: original DFA visualization (open in browser)
│   │   └── index.v2.html       # Task 1: isolated minimal DFA graph rewrite
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

## Task 1: DFA Visualization (src/task1/index.html and src/task1/index.v2.html)

**Algorithm**: DFA String Acceptance (slides 20-23, Chapter 2)

**Technology**: Self-contained HTML/CSS/JS — no dependencies, opens directly in any browser.

**Current isolated rewrite** (`src/task1/index.v2.html`, updated 2026-05-13):
- Minimal DFA page focused on graph clarity and direct homework demonstration of DFA String Acceptance.
- Full graph stays visible and is generated from a complete transition-table textarea. Input format uses `states`, `alphabet`, `start`, `finals`, then transition rows like `q0 a q1`; `finals -` means no accepting states.
- Run panel validates each input symbol against the current alphabet, animates one transition per symbol, updates trace tokens, and marks accepted/rejected terminal state based on the current final-state set.
- Graph lives inside a pannable SVG canvas with a subtle grey reference grid: drag empty canvas to pan larger automata, wheel or buttons zoom, and `Fit graph` recenters the generated layout.
- Node click interaction is preserved for homework readability: clicking a state highlights only arrows pointing to that state and fades the rest; clicking the same state or empty graph space restores the full graph.
- Renderer groups identical `(from, to)` transitions into comma-separated labels, lays states around a generated ellipse, routes self-loops outward, and separates reverse pairs with opposite quadratic curves.
- The in-page `Test case preset` selector loads seven audit cases from easy to extreme: substring `abb`, reverse pair, self-loop mix, 6-state ring, 8-state crossing mesh, 10-state dense paired mesh, and 12-state large sample.
- Browser verification used Chrome headless screenshots `docs/chrome-task1-v2-dynamic.png` and `docs/chrome-task1-v2-presets-grid.png`, static JS checks, and Chrome DevTools Protocol checks. Verified: all seven presets load with no table errors, generated graphs range from 4 states/7 grouped edges to 12 states/23 grouped edges, grey grid renders, and zoom changes the viewport transform. Earlier dynamic-graph verification confirmed q7 click checks highlight only incoming q7 edges.

**Original full editor features**:
- User-editable DFA definition (alphabet, transitions, initial/final states)
- Inline `?` tooltip explaining transition syntax (`from symbol to`)
- Test string input with step-by-step animation
- Canvas rendering: states as circles, transitions as labeled arrows, self-loops
- Color-coded current state (green highlight), accepted (green), rejected (red)
- Accepted runs end with a bright green final-state highlight on the terminal accepting node
- Step mode (manual) and Run mode (auto, configurable speed)
- Trace log showing every state transition
- Test-case selector dropdown that loads twelve curated preset DFAs covering renderer edge-cases (bidirectional adjacent, bidirectional far across intermediates, multi-symbol parallel edges, all self-loops, 5-state chain with wrap-around, 6-state circle layout with diametric pairs, single state self-loop, dead-state rejection, 12-state stress, 14-state crossing mesh, dense hub spokes, and parallel reverse labels)
- Page opens directly on the graph/import console; there is no hero or service-style landing section

**Renderer architecture** (readability refactor 2026-05-12):
- `buildGraphModel(model)` groups `dfa.delta` by ordered `(from, to)` pair, merges symbols into a comma-separated label, indexes self-loops, and builds an unordered-pair lookup for reverse-pair lane assignment.
- `analyzeGraph(graph)` derives degree, self-loop counts, reverse pairs, highest-degree hub, edge density, and a `dense` flag (`>=9` states, `>16` grouped edges, or density `>1.65`). Dense mode changes the representation instead of trying to show every label at once.
- `layoutGraph(graph)` selects one of `empty`, `single`, `pair`, `circle`, or `hub` modes. The `hub` mode triggers when one state has degree ≥ max(5, 0.6·(count−1)); that state goes to canvas center and the rest form an outer ring. All other multi-state graphs (count ≥ 3) use `circle` — every state evenly spaced on a single outer ring.
- `barycenterOrder(states, edges)` runs 18 passes of a crossing-minimization heuristic: each pass replaces every state's circle position with the circular mean of its neighbors' angles; the next pass re-sorts by the new angles. Ties break with `sortStates`.
- `routeGraphEdges(graph, layout)` produces deterministic route objects per edge. Straight chords are used only for short adjacent rim edges and hub spokes. Quadratic curves separate long chords and reverse pairs. Self-loops extend outward from the rim along the radial direction so they do not overlap interior chords.
- Dense mode suppresses static label clutter: all transitions remain as faint structural strokes, but labels are shown only for the active transition, the selected/current state's incident transitions, and merged multi-symbol summary edges. Clicking a node toggles that node as the selected state.
- `#edge-ledger` lists the currently readable dense-mode transitions below the canvas, binding labels to `from --symbols--> to` text without forcing every label into the graph body.
- `drawDFA()` paints in layers: grid → visible static edges → faint ghost edges → state circles → selected/visible labels → active highlighted edge and label. Active animation stays readable above dense graph structure.
- `window.statePos`, `window.layoutMode`, and `window.routedEdges` are exposed each draw to support Playwright-driven structural verification.

**Renderer verification** (2026-05-12):
- Verification harness runs against a local static server (`npx http-server src/task1`) using `playwright-cli`, iterating all 12 presets through `#test-select`.
- Per-preset checks read `window.statePos` and `window.routedEdges` and assert: zero out-of-bounds nodes, zero off-canvas edge endpoints, zero off-canvas label points, zero labels overlapping unrelated state circles, zero straight edges crossing unrelated nodes, and no `undefined`/`NaN`/`[object Object]` strings in the rendered text.
- Refreshed canvas screenshots for every preset are stored in [`docs/playwright-task1-renderer-refactor/`](c:/Users/admin/automata/docs/playwright-task1-renderer-refactor/) for visual review of the hard 12/14-state crossing meshes, dense hub-and-spoke graph, and parallel reverse-label graph.

**Visual system** (audited 2026-05-12):
- **Page chrome — pure black/white**: off-black substrate (`#0a0a0a`), phosphor-white text, all borders and decorative elements in neutral grey only. No red or color outside the canvas.
- **Canvas signal palette only**: three semantic colors confined to the graph canvas: yellow `#f5c518` = current active state/transition during walk, green `#4af626` = accepted final state, red `#e61919` = rejected/dead state. All other canvas elements are neutral grey/white.
- **State semantics**: initial state is marked with an entry arrow; final states use a double ring in neutral white; active states glow yellow; accepted glow green; rejected glow red.
- **Typography**: JetBrains Mono / IBM Plex Mono for all UI — no display font. Educational tool aesthetic, not commercial product.
- **Layout**: square-corner panels, 1px grey compartment lines, no crosshair markers, no decorative accent color.
- **Accessibility**: controls keep visible focus rings, high-contrast button text, semantic sections, skip-to-content link, and keyboard-friendly form controls.

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
