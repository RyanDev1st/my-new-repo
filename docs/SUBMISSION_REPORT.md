# Automata Homework Submission Report

Date: 2026-04-19

## 1. Submission Overview

This repository contains the two required deliverables derived from the course materials:

1. A local visualization application for one algorithm from Chapters 2 and 3.
2. Console programs that implement all algorithms assigned from Chapters 2 and 3.

The selected visualization topic is **DFA string acceptance**, because it is the simplest algorithm to present clearly in an interactive browser-based form while still matching the course content directly.

## 2. Deliverables

### Task 1: Local Visualization Website

Location: `src/task1/index.html`

Purpose:
- Demonstrates DFA string acceptance step by step.
- Lets the user define a DFA locally in the browser.
- Shows the current state, transitions taken, and final accept/reject result.

Key features:
- Editable alphabet, transitions, initial state, and final states.
- Step mode and auto-run mode.
- Canvas-based state diagram rendering.
- Transition trace log.
- Empty-string behavior already supported by the visualization logic.

How to use:
- Open `src/task1/index.html` directly in a browser.
- No server or external dependency is required.

### Task 2: Console Programs

All programs are stored under `src/task2/` and read from a local `input.txt` file in their own directory.

| Folder | Algorithm |
|---|---|
| `01_dfa_acceptance` | DFA string acceptance |
| `02_nfa_extended` | NFA extended transition function `delta*` |
| `03_nfa_to_dfa` | NFA to DFA conversion |
| `04_dfa_minimize` | DFA minimization |
| `05_re_to_nfa` | Regular expression to NFA |
| `06_grammar_to_nfa` | Right-linear grammar to NFA |
| `07_nfa_to_grammar` | NFA to right-linear grammar |

## 3. Build and Run Instructions

Compiler used during verification:
- `C:\msys64\ucrt64\bin\g++.exe`

PowerShell command to compile and run every console program:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;$env:PATH"
$base = "C:\Users\admin\automata\src\task2"
$dirs = "01_dfa_acceptance","02_nfa_extended","03_nfa_to_dfa","04_dfa_minimize","05_re_to_nfa","06_grammar_to_nfa","07_nfa_to_grammar"
foreach ($d in $dirs) {
  Push-Location (Join-Path $base $d)
  & "C:\msys64\ucrt64\bin\g++.exe" -std=c++17 main.cpp -o prog.exe
  .\prog.exe
  Pop-Location
}
```

Example for one program:

```powershell
Set-Location C:\Users\admin\automata\src\task2\01_dfa_acceptance
& "C:\msys64\ucrt64\bin\g++.exe" -std=c++17 main.cpp -o prog.exe
.\prog.exe
```

## 4. Input Conventions and Edge Cases

### Standard Input Files

Each console program reads `input.txt` from its own directory. The exact file format is documented at the top of each `main.cpp`.

### Empty Input Handling

This submission now explicitly handles empty-input cases:

- If `input.txt` is empty or missing its required header data, the program prints an `Input error:` message and exits cleanly.
- This prevents crashes and prevents output from being generated from uninitialized data.

### Empty String Handling

The programs that take strings as algorithm input now support empty-string cases directly:

- `01_dfa_acceptance`
  - Use a blank line or `""` as a test string.
- `02_nfa_extended`
  - Use `q` alone or `q ""` to evaluate `delta*(q, "")`.
- `05_re_to_nfa`
  - An empty regex line is rejected.
  - Use `~` when the intended regular expression is lambda.

## 5. Verification Summary

Verification was rerun on 2026-04-19.

Validated successfully:
- All seven console programs compile with `g++ -std=c++17`.
- All seven sample inputs execute successfully.
- Empty `input.txt` regression checks now fail safely with `Input error:`.
- `01_dfa_acceptance` correctly evaluates the empty string.
- `02_nfa_extended` correctly evaluates `delta*(q, "")`.

Representative checked results:

- `01_dfa_acceptance`
  - `""` → rejected because the initial state is not final.
  - `"ab"` → accepted.
- `02_nfa_extended`
  - `delta*(0, "") = {0}` → rejected.
  - `delta*(0, "ab") = {0,2}` → accepted.
- `03_nfa_to_dfa`
  - Produces 4 DFA states for the sample NFA.
- `04_dfa_minimize`
  - Reduces the sample DFA from 6 states to 3 equivalence classes.
- `05_re_to_nfa`
  - Builds the Thompson NFA for `(a+b)*ab`.
- `06_grammar_to_nfa`
  - Produces the expected NFA structure from the sample right-linear grammar.
- `07_nfa_to_grammar`
  - Produces the expected right-linear grammar from the sample NFA.

## 6. Notes for Grading

- The project is fully local and does not require any network access.
- The visualization deliverable is intentionally simple and self-contained so it can be opened directly for inspection.
- The console programs include sample inputs and now include explicit protection for empty-input edge cases.
- The repo documentation under `docs/` matches the verified implementation state as of 2026-04-19.
