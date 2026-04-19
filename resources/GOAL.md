# GOAL

Our goal is to achieve and finish this following homework. 
From "2-Finite Automata.pdf"These are explicitly labeled as "Coding Exercise (Advanced)" in the slides:DFA String Acceptance CheckLocation: Slide 20Task: Input a string and a DFA, and output whether the string is accepted.Reference Code: Slides 21, 22, and 23 provide the C++ logic (e.g., bool validS()).NFA Extended Transition FunctionLocation: Slide 38Task: Input an NFA transition table, an initial state $q_0$, and an edge label $a$. Compute and output $\delta^{*}(q,a)$ and $\delta^{*}(T,a)$.Reference Code: Slides 39, 40, 41, and 42 provide the C++ pseudo-code for find_lamda_closure(), move(), and find_T_closure().NFA to DFA ConversionLocation: Slide 53Task: Input an NFA and output an equivalent DFA.Algorithm Definition: You are instructed to use the nfa-to-dfa procedure detailed on Slides 45 and 46.DFA State Reduction (Minimization)Location: Slide 72Task: Input a DFA, output its equivalence classes, and output the new reduced DFA.Algorithm Definition: You are instructed to implement the mark() procedure defined on Slides 59 and 60, followed by the reduce() procedure defined on Slide 64.From "3-Regular Language and Regular Grammar.pdf"This file does not use the exact phrase "Coding Exercise," but it explicitly defines three step-by-step computational "Procedures" which represent the algorithms for this chapter:Regular Expression to NFALocation: Slide 17Task: Implement Procedure: re2nfa().Input/Output: Input a regular expression $r$; output an NFA $M = (Q, \Sigma, \delta, q_0, F)$.Right-Linear Grammar to NFALocation: Slide 40Task: Implement Procedure $G_R$ to nfa().Input/Output: Input a Right-linear grammar $G_R = (V, T, S, P)$; output an NFA $M = (Q, \Sigma, \delta, q_0, F)$.NFA to Right-Linear GrammarLocation: Slide 44Task: Implement Procedure: nfa2$G_R$.Input/Output: Input an NFA $M = (Q, \Sigma, \delta, q_0, F)$; output a Right-Linear Grammar $G_R = (V, \Sigma, S, P)$.

These are the tasks derived from C:\Users\admin\automata\resources\2-Finite Automata.pdf and C:\Users\admin\automata\resources\3-Regular Language and Regular Grammar.pdf

# TASK
Student submits registration source code for practice test exemption if applicable. Student selects two options for the practice program:

1. A visualization application that demonstrates one of the algorithms included in chapter 2&3.
(We will make a website for this. Keep everything local and simple. Aim for visualization only.)
2. Console programs (input from a file) that implement all algorithms included in chapter 2&3.

Note: Your programs must have sufficient documentation for compilation and execution. For console programs, sample input file(s) need to be included as well.

# CHOICE
We will do both the visualization console program tasks. Choose the easiest algorithm for task 1. 