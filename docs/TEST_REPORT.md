# Test Report

## Environment

- Compiler: g++ 13.2.0 (MSYS2 UCRT64) at `C:\msys64\ucrt64\bin\g++.exe`
- Verification rerun on 2026-04-20.
- All programs compile cleanly with `-Wall -Wextra -pedantic` and run correctly with their bundled sample inputs.
- Coverage now includes 15 bundled cases per exercise (`tc01`-`tc15`), including 35 additional arbitrary edge/corner cases added in this pass.
- Automata-style inputs in `01`, `02`, `03`, `04`, and `07` now use labeled sample files with bare-integer transition rows such as `0 a 1`.
- NFA lambda input aliases are now `""`, `lambda`, and `lamda`.
- `05_re_to_nfa` and `06_grammar_to_nfa` still take regex and grammar input per `GOAL.md`, but bundled files now use labeled first lines (`regex ...`, `variables ...`) so the samples explain themselves.
- All console programs now reject empty `input.txt` files with an `Input error:` message instead of crashing or printing garbage output.
- `03_nfa_to_dfa` sample metadata was corrected where several testcases had the wrong transition count.
- `04_dfa_minimize/tc05` and `07_nfa_to_grammar/tc05`, `tc08` also had wrong transition counts in older raw files and were normalized during the labeled-format cleanup.
- Fresh 2026-04-20 rerun after the labeled-format cleanup: all 7 programs compile with `-Wall -Wextra -pedantic`, all bundled inputs run, and only `07_nfa_to_grammar` prints 2 expected lambda-transition warnings (`tc08`, `tc15`).
- Checked-in output snapshots for `02_nfa_extended` and `03_nfa_to_dfa` were refreshed from the verified run.
- Checked-in output snapshots for every exercise now include the new `tc11`-`tc15` cases.

## Quick Start

Open PowerShell and run:

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

Or compile + run individually:
```powershell
Set-Location C:\Users\admin\automata\src\task2\01_dfa_acceptance
& "C:\msys64\ucrt64\bin\g++.exe" -std=c++17 main.cpp -o prog.exe
.\prog.exe
```

## Task 1 — Visualization

Open `src/task1/index.html` directly in any browser (no server needed).

Features to test:
1. Click "Build DFA" — automaton renders on canvas
2. Hover or focus the `?` beside Transitions — syntax tooltip should appear
3. Confirm the tooltip example `0 a 1` matches the actual input format
4. Enter a string ("abab") and click "▶ Run" — watch step-by-step animation
5. Accepted run should end with the final accepting state highlighted bright green
6. Click "⏭ Step" for manual stepping
7. Try a string NOT accepted ("ba") — should end in red REJECTED
8. Edit transitions in the textarea — rebuild and re-test

## Task 2 — All 7 Programs (Status: PASSED)

### 01 — DFA Acceptance
Input: DFA over {a,b} accepting strings ending in "ab"
```
""     → REJECTED  ✓
"ab"   → ACCEPTED  ✓
"aab"  → ACCEPTED  ✓
"ba"   → REJECTED  ✓
"aba"  → REJECTED  ✓
"abab" → ACCEPTED  ✓
```

### 02 — NFA Extended Transition Function
Input: NFA accepting strings containing "ab" as substring
```
delta*(0, "")    → {0}    [REJECTED]  ✓
delta*(0, "ab")  → {0,2}  [ACCEPTED]  ✓
delta*(0, "b")   → {0}    [REJECTED]  ✓
delta*(0, "aab") → {0,2}  [ACCEPTED]  ✓
```

### 03 — NFA to DFA (Subset Construction)
Same NFA as 02. Result: 4 DFA states (D0..D3), finals = D2, D3. ✓

### 04 — DFA Minimization
Input: 6-state DFA with equivalent state pairs.
Indistinguishable pairs: (q0,q1), (q2,q3), (q4,q5)
Result: Minimized to 3 states C0{q0,q1}, C1{q2,q3}(final), C2{q4,q5} ✓

### 05 — RE to NFA (Thompson's Construction)
Input: `(a+b)*ab`
Result: 12-state NFA, start=q6, accept=q11 ✓

### 06 — Right-Linear Grammar to NFA
Input: Grammar V0→aV1|ba, V1→aV1|abV0|b (from slide 42)
Result: 5-state NFA matching the slide diagram ✓

### 07 — NFA to Right-Linear Grammar
Input: 4-state NFA over {a,b}
Result:
```
q0 → aq2 | bq1
q1 → aq1 | bq1
q2 → aq3 | bq2
q3 → aq3 | lambda
```
Correct per nfa2G_R algorithm (S1/S2/S3 from slides 44-45) ✓

## Expanded Arbitrary Coverage (`tc11`-`tc15`)

- `01_dfa_acceptance`: `tc11` even-`a` DFA with empty-string acceptance; `tc12` accept-if-starts-with-`b` plus invalid-symbol rejection; `tc13` missing transition -> `DEAD`; `tc14` multi-final 3-cycle; `tc15` single-state universal acceptor.
- `02_nfa_extended`: `tc11` lambda-chain accepts empty via closure; `tc12` branching on same symbol then accept on `ab`; `tc13` invalid symbol + empty-set results; `tc14` lambda after symbol and non-start queries; `tc15` no reachable final except separate component.
- `03_nfa_to_dfa`: `tc11` deterministic 2-state collapse; `tc12` lambda-start subset `{0,1,2}`; `tc13` branching `a` creates 3 DFA states; `tc14` final-by-lambda-closure makes both DFA states final; `tc15` reachable DFA with no final states.
- `04_dfa_minimize`: `tc11` already minimal DFA stays at 3 states; `tc12` unreachable state removed and `(q0,q1)` merged; `tc13` all-final DFA collapses to 1 state; `tc14` no-final DFA collapses to 1 state; `tc15` equivalent final pair `(q1,q2)` merges.
- `05_re_to_nfa`: `tc11` lambda-only regex; `tc12` plain concatenation `ab`; `tc13` union `a+b`; `tc14` nested precedence with star `(ab+~)*`; `tc15` mixed star/concat/union `a*(ba+ab)`.
- `06_grammar_to_nfa`: `tc11` grammar with lambda production; `tc12` multi-terminal production `abB`; `tc13` start-symbol lambda grammar; `tc14` branching grammar over three variables; `tc15` mixed multi-terminal chain plus lambda exit.
- `07_nfa_to_grammar`: `tc11` simple accepting chain; `tc12` same-symbol branching to two finals; `tc13` no-final NFA gives no lambda productions; `tc14` multiple finals with loops; `tc15` lambda-transition note path exercised and symbol-transition grammar still emitted for reachable non-lambda edges.

## To Change Input

Edit an existing `.txt` file under the corresponding `input/` directory, or add a new `.txt` testcase there, then re-run `./prog.exe`.
Input format is documented at the top of each `main.cpp` and in `src/task2/guideline.md`.

Edge-case notes:
- `01_dfa_acceptance`: use labeled headers and `strings K`; use a blank line or `""` for the empty string.
- `02_nfa_extended`: use labeled headers and `queries K`; use a bare integer state id, for example `0 ab`; use `0` alone or `0 ""` for the empty string query.
- `05_re_to_nfa`: use `regex <expr>`; use `~` for lambda; an empty regex line is treated as invalid input.
- `06_grammar_to_nfa`: use `variables`, `terminals`, `start`, and `productions` headers.
