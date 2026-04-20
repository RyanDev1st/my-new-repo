# Task 2 Transition Row Format Design

Date: 2026-04-20
Status: Approved in chat, pending written-spec review

## Goal

Align task 2 automata-style inputs with the user-requested sparse transition-row syntax while preserving assignment correctness from [resources/GOAL.md](c:/Users/admin/automata/resources/GOAL.md).

Per `GOAL.md`:
- `01_dfa_acceptance`: input string + DFA, output acceptance result.
- `02_nfa_extended`: input NFA transition structure, compute `delta*(q, w)`.
- `03_nfa_to_dfa`: input NFA, output equivalent DFA.
- `04_dfa_minimize`: input DFA, output equivalence classes and reduced DFA.
- `05_re_to_nfa`: input regular expression, output NFA.
- `06_grammar_to_nfa`: input right-linear grammar, output NFA.
- `07_nfa_to_grammar`: input NFA, output right-linear grammar.

The format change applies only to automata-style inputs:
- Change: `01`, `02`, `03`, `04`, `07`
- No change: `05`, `06`

## Approved Direction

Use strict sparse transition rows, not dense tables and not dual-format parsing.

Examples:
- `0 a 1` means `q0 --a--> q1`
- `3 b 1` means `q3 --b--> q1`
- `0 "" 1` means `q0 --lambda--> q1`
- `0 lambda 1` means `q0 --lambda--> q1`
- `0 lamda 1` means `q0 --lambda--> q1`

State identifiers in files are bare integers only.
Program output remains unchanged and continues to print states as `q0`, `q1`, etc.

## Input Format Changes

### 01 DFA Acceptance

Keep:
- `N M`
- alphabet line
- `E`
- initial state
- final-state list
- string count + query strings

Change transition rows to:
- `from symbol to`
- `from` and `to` are bare integers
- `symbol` must be one of alphabet symbols
- lambda rows are invalid for DFA input

### 02 NFA Extended Transition Function

Keep:
- `N M`
- alphabet line
- `E`
- initial state
- final-state list
- query count + query lines

Change transition rows to:
- `from symbol to`
- `from` and `to` are bare integers
- lambda accepted as `""`, `lambda`, or `lamda`

Query format remains:
- `qstate [word]`
- `qstate` becomes bare integer state id
- empty string query remains `q ""` or `q` alone

### 03 NFA to DFA

Same transition-row change as `02`:
- bare integer states
- lambda aliases: `""`, `lambda`, `lamda`

### 04 DFA Minimization

Same transition-row change as `01`:
- bare integer states
- no lambda transitions

### 07 NFA to Right-Linear Grammar

Same transition-row change as `02`/`03`:
- bare integer states
- lambda aliases: `""`, `lambda`, `lamda`

## Task 1 Change

Update DFA visualization help text and example guidance in `src/task1/index.html` so syntax documentation matches task 2 conventions where relevant:
- explicit example: `0 a 1`
- mention lambda identifiers accepted by project conventions: `""`, `lambda`, `lamda`

Task 1 remains a DFA visualizer; lambda is only documented as project syntax reference, not enabled as DFA behavior.

## Non-Goals

- Do not change `05_re_to_nfa` input format from regex to automaton rows.
- Do not change `06_grammar_to_nfa` input format from grammar productions.
- Do not change console program output format unless required by correctness.
- Do not add backward compatibility for old `q0 a q1` input rows.

## Implementation Plan

1. Update parsers in `01`, `02`, `03`, `04`, `07` to parse bare integer state ids.
2. Extend NFA symbol parsing in `02`, `03`, `07` to accept `""`, `lambda`, and `lamda` as lambda aliases.
3. Reject lambda rows in DFA programs `01` and `04`.
4. Rewrite all bundled testcase inputs in affected exercises to the new syntax.
5. Regenerate checked-in outputs from fresh runs.
6. Update `task1` tooltip/example text.
7. Update docs to describe the new format accurately.

## Verification Plan

Fresh verification must prove both parsing and algorithm correctness.

### Build Verification

Compile all seven programs with:
- `g++ -std=c++17 -Wall -Wextra -pedantic`

Required result:
- no compile failures
- no warnings

### Execution Verification

Run all bundled inputs for all seven exercises.

Required result:
- every `.txt` testcase executes
- checked-in `output/` matches fresh generated output

### Factuality Review Against Assignment

Spot-check outputs against `GOAL.md` requirements:
- `01`: acceptance/rejection traces match DFA semantics
- `02`: `delta*(q, w)` reflects move + lambda-closure behavior
- `03`: subset construction yields correct DFA states/finals/transitions
- `04`: equivalence classes and reduced DFA are correct
- `05`: Thompson output still matches regex semantics
- `06`: grammar-to-NFA construction still matches right-linear grammar rules
- `07`: grammar output matches NFA symbol transitions and final-state lambda productions

## Risks

- Existing sample files are numerous; parser and sample rewrites must stay synchronized.
- `task1` wording must mention lambda identifiers without implying DFA lambda execution.
- Final verification must catch stale outputs after format rewrite.

## Success Criteria

- Affected programs accept only the new bare-integer sparse row syntax.
- `05` and `06` remain aligned with `GOAL.md` input requirements.
- Task 1 tooltip/example text reflects the new syntax conventions.
- All bundled tests compile, run, and match checked-in outputs.
