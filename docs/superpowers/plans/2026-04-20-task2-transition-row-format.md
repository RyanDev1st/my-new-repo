# Task 2 Transition Row Format Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Change task 2 automata inputs to bare-integer transition rows for `01`, `02`, `03`, `04`, and `07`, update task 1 help text, and fully verify correctness against `resources/GOAL.md`.

**Architecture:** Keep program structure the same. Only change parsers, sample inputs, generated outputs, and docs. Regex (`05`) and grammar (`06`) stay on their assignment-defined input formats.

**Tech Stack:** C++17 console programs, HTML/CSS/JS for task 1, PowerShell verification, g++.

---

### Task 1: Update Spec and Comment Rule

**Files:**
- Modify: `docs/superpowers/specs/2026-04-20-task2-transition-row-format-design.md`

- [ ] **Step 1: Add comment-style rule to spec**

Add this section:

```md
## Comment Style

- Keep comments short and easy to read.
- Remove extra long inline comments when they are not needed.
- Keep top-of-file comments in each program.
- Top-of-file comments should explain, in simple English:
  - what algorithm the file implements
  - what input pipeline or input format the file reads
  - what the program outputs
```

- [ ] **Step 2: Review spec for scope**

Run: `rtk read docs/superpowers/specs/2026-04-20-task2-transition-row-format-design.md`
Expected: scope still limited to `01/02/03/04/07` plus task 1 help text.

### Task 2: Make Parser Tests Fail First With New Input Syntax

**Files:**
- Modify: `src/task2/01_dfa_acceptance/input/tc01.txt`
- Modify: `src/task2/02_nfa_extended/input/tc01.txt`
- Modify: `src/task2/03_nfa_to_dfa/input/tc01.txt`
- Modify: `src/task2/04_dfa_minimize/input/tc01.txt`
- Modify: `src/task2/07_nfa_to_grammar/input/tc01.txt`

- [ ] **Step 1: Rewrite first sample inputs to bare-integer rows**

Examples:

```txt
0 a 1
0 b 0
1 a 1
1 b 2
```

Lambda examples for NFA files:

```txt
0 "" 1
0 lambda 1
0 lamda 1
```

- [ ] **Step 2: Run full build + execution and confirm failures**

Run full verification command after rewriting only those test files.
Expected: affected programs fail on old parser logic because `q0` is no longer present in those files.

### Task 3: Update DFA Parsers (`01`, `04`)

**Files:**
- Modify: `src/task2/01_dfa_acceptance/main.cpp`
- Modify: `src/task2/04_dfa_minimize/main.cpp`

- [ ] **Step 1: Update top comments**

Top comments must stay, but be short and simple. They must say:
- algorithm name
- simple input pipeline
- output purpose

- [ ] **Step 2: Change state parsing to bare integers**

Implementation shape:

```cpp
int parseState(const string& t) {
    return stoi(t);
}
```

- [ ] **Step 3: Reject lambda rows in DFA files**

Implementation shape:

```cpp
int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t == "\"\"" || t == "lambda" || t == "lamda" || t == "~") return -1;
    if (t.size() == 1) {
        for (int i = 0; i < M; i++) if (alpha[i] == t[0]) return i;
    }
    return -1;
}
```

Expected behavior: DFA input with lambda row becomes `Input error`.

- [ ] **Step 4: Run targeted check**

Run: compile and execute `01` and `04`.
Expected: rewritten `tc01` files pass under new parser.

### Task 4: Update NFA Parsers (`02`, `03`, `07`)

**Files:**
- Modify: `src/task2/02_nfa_extended/main.cpp`
- Modify: `src/task2/03_nfa_to_dfa/main.cpp`
- Modify: `src/task2/07_nfa_to_grammar/main.cpp`

- [ ] **Step 1: Update top comments**

Keep short header comments only.

- [ ] **Step 2: Change state parsing to bare integers**

Implementation shape:

```cpp
int parseState(const string& t) {
    return stoi(t);
}
```

- [ ] **Step 3: Accept all lambda aliases**

Implementation shape:

```cpp
int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t == "\"\"" || t == "lambda" || t == "lamda" || t == "~") return M;
    if (t.size() == 1) {
        for (int i = 0; i < M; i++) if (alpha[i] == t[0]) return i;
    }
    return -1;
}
```

- [ ] **Step 4: Change `02` query state ids to bare integers**

Expected input examples:

```txt
0 a
0 ""
1 bb
```

- [ ] **Step 5: Run targeted check**

Run: compile and execute `02`, `03`, `07`.
Expected: rewritten `tc01` files pass under new parser.

### Task 5: Rewrite All Affected Sample Inputs

**Files:**
- Modify: all `tc01`-`tc15` in:
  - `src/task2/01_dfa_acceptance/input/`
  - `src/task2/02_nfa_extended/input/`
  - `src/task2/03_nfa_to_dfa/input/`
  - `src/task2/04_dfa_minimize/input/`
  - `src/task2/07_nfa_to_grammar/input/`

- [ ] **Step 1: Rewrite all transition rows**

Rule:
- old: `q0 a q1`
- new: `0 a 1`

Rule for NFA lambda rows:
- `0 "" 1`
- `0 lambda 1`
- `0 lamda 1`

- [ ] **Step 2: Rewrite all `02` query lines**

Rule:
- old: `q0 ab`
- new: `0 ab`

- [ ] **Step 3: Sanity-read a few files**

Run:
- `rtk read src/task2/01_dfa_acceptance/input/tc01.txt`
- `rtk read src/task2/02_nfa_extended/input/tc01.txt`
- `rtk read src/task2/07_nfa_to_grammar/input/tc15.txt`

Expected: only bare integer state ids appear in automata-style inputs.

### Task 6: Update Task 1 Syntax Help

**Files:**
- Modify: `src/task1/index.html`

- [ ] **Step 1: Change tooltip/example text**

Text should include:
- `0 a 1` means `q0 --a--> q1`
- `0 "" 1`, `0 lambda 1`, `0 lamda 1` mean lambda edge

- [ ] **Step 2: Keep DFA limitation explicit**

Text should also say task 1 is DFA-only. Lambda text is syntax reference only.

### Task 7: Regenerate Checked-In Outputs

**Files:**
- Modify: `src/task2/*/output/tc01.txt` through `tc15.txt`

- [ ] **Step 1: Run all programs**

Run all seven programs after parser/input rewrite.

- [ ] **Step 2: Copy fresh outputs back into repo**

Expected: output snapshots match new input syntax and current program behavior.

### Task 8: Update Docs

**Files:**
- Modify: `docs/ARCHITECTURE.md`
- Modify: `docs/MEMORY.md`
- Modify: `docs/TEST_REPORT.md`

- [ ] **Step 1: Update format docs**

State clearly:
- `01/02/03/04/07` use `from symbol to` with bare integer states
- NFA lambda identifiers: `""`, `lambda`, `lamda`
- `05` and `06` unchanged because `GOAL.md` requires regex / grammar input

- [ ] **Step 2: Update verification notes**

Record:
- format migration complete
- final compile/run verification date
- factual spot-check results

### Task 9: Final Verification

**Files:**
- Verify all task 2 source, input, output, and docs files touched above

- [ ] **Step 1: Compile everything**

Run:

```powershell
$compiler='C:\msys64\ucrt64\bin\g++.exe'
```

Then compile `01` through `07` with:

```powershell
-std=c++17 -Wall -Wextra -pedantic
```

Expected: all pass, no warnings.

- [ ] **Step 2: Run all bundled cases**

Expected:
- 15 inputs per exercise
- all execute
- generated outputs match checked-in `output/`

- [ ] **Step 3: Factual spot-check against assignment**

Confirm:
- `01` output matches DFA acceptance semantics
- `02` output matches `delta*`
- `03` output matches subset construction
- `04` output matches equivalence classes + reduced DFA
- `05` output still matches Thompson construction
- `06` output still matches grammar-to-NFA construction
- `07` output still matches NFA-to-grammar rules
