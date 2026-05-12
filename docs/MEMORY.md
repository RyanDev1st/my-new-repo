# Project Memory

## Summary

- Goal: finish the automata homework in `resources/GOAL.md` with both deliverables:
  the local DFA visualization and the seven C++ console programs.
- Current verified state on 2026-04-20: all seven console programs compile cleanly with `-Wall -Wextra -pedantic` and run from 15 bundled cases each (`tc01`-`tc15`), and the visualization remains the local website deliverable.
- Latest format review: all bundled `task2` sample files now use labeled, self-explanatory inputs; automata files use bare integer transition rows such as `0 a 1`, regex files use `regex ...`, grammar files use `variables ...`, and NFA files accept `""`, `lambda`, or `lamda` for lambda transitions.

## Task List

- [x] Task 1 visualization website exists and is documented.
- [x] Task 2 console implementations for all seven algorithms exist.
- [x] Sample `input.txt` files are present for all task 2 programs.
- [x] Empty-input handling reviewed and fixed in the console programs.
- [x] Submission-ready documentation added under `docs/`.
- [x] Verification rerun on 2026-04-19 for normal inputs and empty-input regressions.
- [x] Full task 2 audit rerun on 2026-04-20 with warning-clean builds and refreshed output snapshots.
- [x] Added and verified 5 extra arbitrary cases per exercise (`tc11`-`tc15`) on 2026-04-20.
- [x] Migrated automata-style inputs to strict bare-integer transition rows on 2026-04-20.

## Latest Update

- Reviewed the recent repo state and limited changes to actual defects plus required documentation.
- Added fast-fail input validation to `01` through `07` so empty inputs no longer crash or print uninitialized data.
- Updated `01_dfa_acceptance` to accept the empty string as a test case via a blank line or `""`.
- Updated `02_nfa_extended` to accept empty-string queries via `q` alone or `q ""`.
- Added missing transition validation to `03_nfa_to_dfa` and corrected the broken `E` counts in six bundled sample files.
- Refreshed the checked-in `output/` snapshots for `02_nfa_extended` and `03_nfa_to_dfa` so repo artifacts match actual program behavior.
- Cleaned the remaining `-Wall -Wextra -pedantic` warnings in `01`, `04`, `05`, and `07`.
- Added 35 new testcase inputs across `src/task2` (`tc11`-`tc15` in each exercise) to cover explicit edge/corner behavior such as empty string acceptance, invalid symbols, empty NFA result sets, lambda-closure acceptance, no-final-state minimization, regex precedence, multi-terminal grammar chains, and NFA-to-grammar lambda-transition notes.
- Generated matching checked-in outputs for all new `tc11`-`tc15` files after manual review of expected behavior.
- Switched `01`, `02`, `03`, `04`, and `07` to strict sparse transition rows with bare integer states only.
- Switched NFA lambda input aliases to `""`, `lambda`, and `lamda`, and removed old `q0 ... q1` sample syntax from affected inputs.
- Shortened code comments while keeping simple top-of-file comments that explain algorithm, input pipeline, and output purpose.
- Added `src/task2/guideline.md` so a teacher can read one file and understand task 2 folder hierarchy, input formats, and output expectations.
- Finished the labeled sample-file cleanup so every `src/task2/*/input/*.txt` file now starts with a clear header instead of raw counts.
- Corrected leftover bad transition counts while normalizing samples in `04_dfa_minimize/tc05` and `07_nfa_to_grammar/tc05`, `tc08`.
- Fixed the follow-up compile break in `02`, `03`, and `04` by letting their shared readers accept generic `istream&` after the labeled-input pre-parse step.
- Re-verified all seven programs on 2026-04-20 after the cleanup; only `07_nfa_to_grammar` emits the expected lambda-transition warnings for `tc08` and `tc15`.
- Refreshed `docs/ARCHITECTURE.md` and `docs/TEST_REPORT.md` so task 2 documentation matches the real `input/` + `output/` folder workflow.
- Added a submission-ready report to `docs/`.
- Added a DFA editor syntax tooltip in `src/task1/index.html`.
- Updated the task 1 visualization so accepted runs finish with a bright green accepting node instead of the in-progress blue state highlight.
- 2026-05-12: refactored the task 1 graph renderer in `src/task1/index.html`. Replaced the monolithic `drawArrow` with `buildEdges` plus dedicated `drawStraightEdge` (line), `drawCurvedEdge` (quadratic Bezier with perpendicular routing), `drawSelfLoop` (cubic Bezier loop) and a halo-stroked `drawLabel`. Reverse-pair edges now auto-flip onto opposite sides because the perpendicular `n=(-uy, ux)` follows the forward direction, fixing the previous overlap when both directions of a pair were curved in the same direction. Arrow heads use exact Bezier tangents (quadratic `t=1` and cubic `t=1`) instead of the prior `t=0.95` approximation.
- 2026-05-12: added an in-page test-case selector (`#test-select`) in `src/task1/index.html` with eight presets covering bidirectional adjacent, bidirectional far across intermediates, multi-symbol parallel edges, all self-loops, 5-state chain with wrap-around, 6-state circle layout with diametric pairs, single-state self-loop, and dead-state rejection, so the renderer can be visually exercised against every layout case.
- 2026-05-12: rebuilt `src/task1/index.html` end-to-end with a premium "Ethereal Glass" visual system (OLED black `#050507` background, fixed-pointer radial mesh-gradient orbs in sky/violet/emerald, paper-grain overlay, Plus Jakarta Sans + JetBrains Mono typography, double-bezel shell-plus-core cards on a 12-column bento grid, sticky glass island nav, pill buttons with the button-in-button trailing icon pattern, cubic-bezier `(0.32,0.72,0,1)` transitions, staggered reveal-on-load animation, in-canvas legend chip, state-summary chips, and a live canvas-meta pill that mirrors the current step). Renderer hardened: canvas min height bumped to 360px, row-layout padding raised to `SR + 60`, edge stroke uses `lineCap: round`, arrowhead is now a concave four-point chevron, state circles render with a radial gradient for spatial depth, and `drawCurvedEdge` clamps `curveAmt` against the canvas bounds with margin 18 so quadratic peaks never fly off-screen even on extreme stepDiffs.
- 2026-05-12: redesigned `src/task1/index.html` from Ethereal Glass to Tactical Telemetry industrial brutalism and refactored the graph renderer around deterministic layout, edge grouping, route computation, and routed drawing. User-authored transition tables now drive graph layout directly, including named states, single-state, bidirectional, dense, and self-loop-heavy DFAs.
- 2026-05-12: removed the service-style nav/hero from `src/task1/index.html` so the task 1 page opens directly on the graph/import console, and fixed manual Step so it advances one transition without scheduling auto-run continuation.

## Next Step

- Submission is ready from a code-and-docs standpoint after the 2026-04-20 expanded coverage audit, the 2026-04-20 input-format migration, and the 2026-05-12 task-1 renderer refactor.
- Only touch the implementation again if a new reviewer finding or grading requirement appears.
