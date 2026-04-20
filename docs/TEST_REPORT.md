# Test Report

## Environment

- Compiler: g++ 13.2.0 (MSYS2 UCRT64) at `C:\msys64\ucrt64\bin\g++.exe`
- Verification rerun on 2026-04-19.
- All programs compile and run correctly with their sample inputs.
- All console programs now reject empty `input.txt` files with an `Input error:` message instead of crashing or printing garbage output.

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

## To Change Input

Edit `input.txt` in the corresponding directory and re-run `./prog.exe`.
Input format is documented at the top of each `main.cpp`.

Edge-case notes:
- `01_dfa_acceptance`: use a blank line or `""` for the empty string.
- `02_nfa_extended`: use `q` alone or `q ""` for the empty string query.
- `05_re_to_nfa`: use `~` for lambda; an empty regex line is treated as invalid input.
- All console programs now stop with `Input error:` if `input.txt` is empty.
