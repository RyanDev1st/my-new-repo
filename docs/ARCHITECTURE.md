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

**Current isolated rewrite** (`src/task1/index.v2.html`, updated 2026-05-17):
- Minimal DFA page focused on graph clarity and direct homework demonstration of DFA String Acceptance.
- Full graph stays visible and is generated from a complete transition-table textarea. Input format uses `states`, `alphabet`, `start`, `finals`, then transition rows like `q0 a q1`; `finals -` means no accepting states.
- Run panel validates each input symbol against the current alphabet, animates one transition per symbol, updates trace tokens, and marks accepted/rejected terminal state based on the current final-state set.
- Graph lives inside a pannable SVG canvas with a subtle grey reference grid rendered inside the same transformed SVG viewport: drag empty canvas to pan larger automata, wheel or buttons zoom, and `Fit graph` recenters the generated layout (fit now includes the full curve extents so long backward arcs are never clipped).
- Node click interaction is preserved for homework readability: clicking a state highlights only arrows pointing to that state and fades the rest; clicking the same state or empty graph space restores the full graph.
- Renderer uses a directed ranked layout inspired by Graphviz `dot`: breadth-first ranks flow outward from the start state, states are spaced in rank columns, and multi-symbol labels use a compact `a · b` style instead of comma blobs.
- The in-page `Test case preset` selector loads seven audit cases from easy to extreme: substring `abb`, reverse pair, self-loop mix, 6-state ring, 8-state crossing mesh, 10-state dense paired mesh, and 12-state large sample.

**Edge routing algorithm** (rewritten 2026-05-17 to fix overlap on dense graphs):
- Edges between distinct states use a quadratic Bezier whose control point sits perpendicular to the canonical line from `min(from,to)` to `max(from,to)`. Each candidate "side" (positive vs negative perpendicular offset) is evaluated independently.
- For every (from,to) pair, `buildSidedCurve` sweeps a range of amplitudes from `baseAmp(dist, kind) * 0.32` up to `max(baseAmp*3+80, dist*0.85)` in even steps. At each amplitude the curve is sampled at `t = 0.15, 0.30, 0.50, 0.70, 0.85` and the minimum distance to any non-endpoint state is computed (`sampleCurveClearance`).
- The chosen amplitude is the one closest to `baseAmp` *among* amplitudes that achieve clearance ≥ `R + 22`; if none clear, the highest-clearance amplitude is returned. This finds the smallest visually pleasing bend that still keeps the curve outside every other node.
- For reverse pairs (`A→B` and `B→A` both exist) the two curves are forced onto opposite sides of the canonical line by deterministic lex assignment (`sortedIds[0] === edge.from ? +1 : -1`). Because both curves use the *same* canonical normal, this guarantees they always render on opposite screen sides — the prior overlap bug (both directions curving the same way because each edge mirrored both `normal` and `curve` sign) is fixed.
- For single-direction edges, both sides are evaluated and the side with greater clearance wins; ties break by smaller amplitude.
- Endpoints (`a`, `b`) are clipped to each node ring along the line to the control point via `pointToward`, so the curve enters and exits tangent to the arrow rather than along the straight chord — arrowheads now align with the actual curve direction at the node ring.
- Self-loops use a cubic Bezier whose anchor angle is chosen by `pickSelfLoopAngle`: it scans 24 candidate angles around the node and picks the one whose minimum angular distance to any neighbor edge direction is maximal, with a small penalty for angles that point at other nodes within `2R` and a faint preference for vertical placement.
- Labels are positioned at the visual apex of each curve (`0.25·a + 0.5·C + 0.25·b` for quadratics; for self-loops the radial direction plus a small offset). The existing white halo (`stroke-width: 9px`) keeps labels readable when they sit over grid lines.
- `fitView` now derives its bounds from every path control/endpoint point, not just node centers, so the auto-fit zoom always shows the full curve geometry (important for long backward edges whose apex can sit far above or below the rank layout).

**Edge routing verification** (algorithmic, 2026-05-17):
- Synthetic harness builds each preset model, runs `layoutStates` + `buildEdges`, samples each path at 200 points, and asserts no path passes within `R + 5` of a non-endpoint state.
- All seven presets now pass: minimum curve clearance is 49.2px (loops-mix) and the typical clearance sits between 60-250px. Reverse-pair separation verified: all 17 reverse pairs across the presets have their control points on opposite screen-sides of the canonical line.
- Earlier-known dense failures `q5→q0` (ring-six, was 37.7px) and `q4→q0` (hub-ten, was 22.2px) are now resolved.

**Interaction & UX layer** (added 2026-05-17, TDD authored before implementation):
- **Self-loop label on curve**: label position moved from `outerR + 14` (≈49px from the loop curve) to the cubic Bezier apex at t=0.5 (`0.125·a + 0.375·c1 + 0.375·c2 + 0.125·b`). Labels now sit on the loop like every other edge label, halo provides background contrast.
- **Run speed slider**: `#run-speed` range input (100ms–2000ms, default 720ms, step 20ms) gates the symbol-by-symbol replay. `runInput` reads `stepInterval()` between every transition (initial pause = `step·0.6`, per-symbol pause = `step`, post-symbol settle = `step·0.4`). Label `#run-speed-value` updates live as the slider drags.
- **Node dragging**: pointer-event handlers on each `.node` group capture the pointer, project screen coords into viewport-local space via `svgPoint(event)` (using `viewport.getScreenCTM().inverse()`), and rewrite the node's `(x, y)` in `states`. After each drag tick the routine calls `buildEdges` + `renderEdges` + `renderStartArrow` so curves track the dragged node live; the dragged node's DOM circles/labels are moved in place to keep pointer capture intact. Drag-vs-click is disambiguated by a 3px movement threshold (`nodeDrag.moved`); below the threshold pointerup still toggles selection like before, preserving the keyboard-accessible Enter/Space toggle.
- **Cursor contrast**: `.graph-wrap` now uses a custom inline-SVG cursor (cream fill, dark stroke, with a dark/cream-stroke variant during `.dragging`) so the cursor stays visible against the off-white canvas. SVG children inherit the canvas cursor (`.graph-wrap svg * { cursor: inherit; }`) to prevent the default text I-beam over edge labels; nodes opt into `cursor: move` to advertise drag affordance.

**Feature tests** (TDD, `src/task1/__tests__/feature_tests.js`):
- Playwright-driven, runs against `file://` URL.
- Covers: self-loop `labelToApex < 20px`; `#run-speed` exists as range input with sensible min/max/default; mouse drag on `.node[data-state="q1"]` moves the state more than 30-40px and keeps edge endpoints adjacent; computed cursor on `#graph-wrap` matches `grab|move|crosshair|pointer`.
- Initial run (red): 6 / 12 failed (loop-label distance 49.1, missing slider, drag inert). Post-implementation (green): 12 / 12 pass.

**Run-animation layer** (added 2026-05-17, isolated to `src/task1/index.v2.html`):
- Particle traveler `#travel-layer > .travel-dot` rides the active edge using `path.getPointAtLength(t * totalLength)` over a `requestAnimationFrame` loop with `easeInOutCubic`. A fading `.travel-trail` circle trails ~12% behind the dot. A `.travel-arrow` chevron leads the dot by 11 px along the path with its rotation set from the local tangent (sampled at `tipDist - 4` ↔ `tipDist` on the path), so the arrow head turns through curves and self-loops as it travels. The static `marker-end` of the active edge is suppressed via `marker-end: none` on `.edge.active .edge-path` so the arrow head reads as moving with the dot rather than fixed at the destination ring. Travel duration is `max(160, step * 0.85)` ms; per-step settle pause is `step * 0.35`.
- Source state pulses with `.node.reading` (orange ring, `ring-pulse` keyframes with stroke-width/drop-shadow swell). Destination previews with `.node.target` (dashed orange ring, `target-pulse` keyframes). Both classes only apply during the read phase; `current` highlight is suppressed while `readingFromState` or `readingToState` is set, so the source-vs-destination distinction is unambiguous.
- Active edge gets `.edge.active` with a wider orange stroke, `stroke-dasharray: 14 8`, and `dash-flow` keyframe animation (700 ms linear loop) producing a directional "flowing dashes" cue. The active edge label upgrades to 17px / orange.
- Step meter `#step-meter-text` + `#step-meter-fill` shows `Step n / N` with a progress bar that animates with `cubic-bezier(0.32, 0.72, 0, 1)` width transitions. Status title formats transitions as `q0 → q1` (state pills + orange arrow glyph); status text inserts the current symbol as a `.symbol-pill` orange capsule.
- Trace tokens now carry three states: empty, `.token.now` (currently-being-read symbol — orange-fill pill with a `token-pulse` scale + shadow-ring keyframe), `.token.done` (already-read). Empty-string inputs render a single `ε` placeholder token.
- Terminal verdicts trigger one-shot animations on the whole `.node` group: `accept-burst` scales 1.0 → 1.16 → 1.0 with a green drop-shadow on the ring; `reject-shake` translates the group ±6 / ±4 / ±3 px with a red drop-shadow on the ring. `transform-box: fill-box; transform-origin: center` keeps the animations centered on each node.
- New state vars: `readingFromState`, `readingToState`, `activeSymbolIndex`. `applyViewState` toggles the new classes; `setTrace(chars, doneCount, nowIndex)` carries the now-index. `clearTravelLayer()` runs on reset/cancel and on token mismatch inside the rAF loop so a fresh Run never inherits a stale traveler.

**Run-animation tests** (`src/task1/__tests__/anim_tests.js`, 22 cases):
- Mid-run assertions while a step is in flight (slider 1400 ms): `.edge.active` count, `.node.reading=q0`, `.node.target=q1`, `#travel-layer .travel-dot` rendered, `.token.now` text = `a`, step meter labelled `Step 0 / 4`.
- End-of-run assertions wait on `.node.accepted, .node.rejected` via `page.waitForFunction`: `aabb` accepts at q3 with status `Accepted` and step meter 100%; `aa` rejects at q1; empty input `ε` rejects at q0 with meter `Step 0 / 0`.
- Difficulty-ladder coverage: the largest preset `large-twelve` is run at default-speed (720 ms) to confirm the traveler, reading/target pulses, and active-edge dash flow stay legible against 12-state graph density; assertions cover mid-run `.edge.active`, travel dot, reading + target classes, and end-of-run verdict + 100% fill.
- All 22 cases pass. Existing 12 layout tests still pass. Screenshots `docs/anim-midrun.png`, `docs/anim-accepted.png`, `docs/anim-rejected.png`, `docs/anim-large-twelve-midrun.png` show the four representative states.

**Security hardening**: status title / status text use `innerHTML` to render orange state-pill / symbol-pill markup. User-supplied state ids, alphabet symbols, and input symbols are routed through an `escapeHtml` helper before interpolation so a transition table with a state id like `<script>` cannot inject HTML into the status pane.

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
