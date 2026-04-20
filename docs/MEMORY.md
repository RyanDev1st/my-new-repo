# Project Memory

## Summary

- Goal: finish the automata homework in `resources/GOAL.md` with both deliverables:
  the local DFA visualization and the seven C++ console programs.
- Current verified state on 2026-04-19: all seven console programs compile and run from sample inputs, and the visualization remains the local website deliverable.
- Latest defect review: the console programs did not handle empty `input.txt` safely, `01_dfa_acceptance` and `02_nfa_extended` did not expose empty-string cases clearly, and `02_nfa_extended/input.txt` was missing.

## Task List

- [x] Task 1 visualization website exists and is documented.
- [x] Task 2 console implementations for all seven algorithms exist.
- [x] Sample `input.txt` files are present for all task 2 programs.
- [x] Empty-input handling reviewed and fixed in the console programs.
- [x] Submission-ready documentation added under `docs/`.
- [x] Verification rerun on 2026-04-19 for normal inputs and empty-input regressions.

## Latest Update

- Reviewed the recent repo state and limited changes to actual defects plus required documentation.
- Added fast-fail input validation to `01` through `07` so empty `input.txt` no longer crashes or prints uninitialized data.
- Updated `01_dfa_acceptance` to accept the empty string as a test case via a blank line or `""`.
- Updated `02_nfa_extended` to accept empty-string queries via `q` alone or `q ""`.
- Restored `src/task2/02_nfa_extended/input.txt` with a working sample NFA and queries, including the empty-string case.
- Refreshed `docs/ARCHITECTURE.md` and `docs/TEST_REPORT.md`.
- Added a submission-ready report to `docs/`.
- Added a DFA editor syntax tooltip in `src/task1/index.html`.
- Updated the task 1 visualization so accepted runs finish with a bright green accepting node instead of the in-progress blue state highlight.

## Next Step

- Submission is ready from a code-and-docs standpoint.
- Only touch the implementation again if a new reviewer finding or grading requirement appears.
