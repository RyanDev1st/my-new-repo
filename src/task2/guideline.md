# Task 2 Guideline

This folder contains the 7 console exercises for the homework.

Each exercise folder has:
- `main.cpp` : the source code
- `input/` : sample input files like `tc01.txt`, `tc02.txt`, ...
- `output/` : saved program output for the matching input files

Typical folder shape:

```text
src/task2/
├── guideline.md
├── 01_dfa_acceptance/
│   ├── main.cpp
│   ├── input/
│   └── output/
├── 02_nfa_extended/
├── 03_nfa_to_dfa/
├── 04_dfa_minimize/
├── 05_re_to_nfa/
├── 06_grammar_to_nfa/
└── 07_nfa_to_grammar/
```

## What Teacher Should Expect

- Every exercise reads all `.txt` files inside its own `input/` folder.
- For each `input/tcXX.txt`, the program writes one matching `output/tcXX.txt`.
- The sample files use labeled sections; sections can appear in any order.

Example:
- `input/tc01.txt` -> `output/tc01.txt`
- `input/tc02.txt` -> `output/tc02.txt`

## Main Idea

There are 3 input styles in `task2`:
- `01`, `02`, `03`, `04`, `07` use labeled automata files
- `05` uses one labeled regex line
- `06` uses labeled grammar sections

So the files are now coherent, but `05` is still a regex exercise and `06` is still a grammar exercise, as required by `resources/GOAL.md`.

## 1. Labeled Automata Format

Used by:
- `01_dfa_acceptance`
- `02_nfa_extended`
- `03_nfa_to_dfa`
- `04_dfa_minimize`
- `07_nfa_to_grammar`

General shape (sections may appear in any order):

```text
alphabet a b
transitions 4
0 a 1
0 b 0
1 a 1
1 b 2
initial 0
finals 2
```

The `states` line is optional. If omitted, the number of states `N` is auto-discovered as `max(state id seen) + 1`.

Important meaning:
- `0 a 1` means `q0 --a--> q1`
- `0 "" 1`, `0 lambda 1`, or `0 lamda 1` mean a lambda move in NFA files
- `finals 2` means state `2` is final
- `finals 1 3` means states `1` and `3` are final
- `finals -` means there is no final state

Input uses plain integers, but the programs print states as `q0`, `q1`, `q2`, ...

### 01. DFA Acceptance

Adds:

```text
strings 4
""
a
bb
aaab
```

Notes:
- DFA files do not allow lambda transitions
- Use `""` or a blank line for the empty string

### 02. NFA Extended Transition Function

Adds:

```text
queries 3
0 ""
0 ab
1 b
```

Notes:
- Lambda can be written as `""`, `lambda`, or `lamda`
- Query format is `state [word]`
- `0` alone also means query from state `0` with the empty string

### 03. NFA to DFA

Uses only the general automata shape.

It prints:
- subset states
- DFA transition table
- DFA final states

### 04. DFA Minimize

Uses only the general automata shape.

It prints:
- accessible states
- indistinguishable pairs
- minimized DFA

### 07. NFA to Grammar

Uses only the general automata shape.

If the input NFA has lambda transitions, the program prints a note because textbook `nfa2G_R` expects lambda-free NFA.

## 2. Regex Format

Used by:
- `05_re_to_nfa`

Format:

```text
regex (ab+~)*
```

Notes:
- `+` means union
- `*` means Kleene star
- `()` means grouping
- `~` means lambda

## 3. Grammar Format

Used by:
- `06_grammar_to_nfa`

Format:

```text
variables S A
terminals a b
start S
productions 3
S a A
S b -
A a S
```

Meaning:
- `variables ...` lists grammar variables
- `terminals ...` lists terminal symbols
- `start S` gives the start variable
- `productions 3` says there are 3 production rows below it
- `-` means there is no next variable after the terminal chain

## Meaning of `input/` and `output/`

`input/`
- sample test cases
- the program reads these files

`output/`
- saved program results for the matching input files
- useful for checking correctness quickly

Example:

```text
input/tc12.txt
output/tc12.txt
```

These two files belong together.

## Quick Reading Tip

If the teacher wants the short rule:

- `01/02/03/04/07` : labeled automata files with rows like `0 a 1`
- `05` : one line `regex ...`
- `06` : labeled grammar file with `variables`, `terminals`, `start`, `productions`
