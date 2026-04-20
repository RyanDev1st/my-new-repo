// NFA to Right-Linear Grammar
// Algorithm: nfa2G_R -- slides 44-45, Chapter 3
//
// Input format (input.txt): same NFA format as 02/03
//   N M
//   s0 ... sM-1
//   E
//   from sym_idx to   (sym_idx = M = lambda)
//   q0
//   F_count
//   f0 f1 ...

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
using namespace std;

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }

    int N, M;
    if (!(fin >> N >> M)) {
        cerr << "Input error: input.txt is empty or missing the NFA header.\n";
        return 1;
    }

    vector<char> alpha(M);
    for (int i = 0; i < M; i++) {
        if (!(fin >> alpha[i])) {
            cerr << "Input error: missing alphabet symbols.\n";
            return 1;
        }
    }

    // delta[state][sym] = set of next states; sym=M is lambda
    vector<vector<set<int>>> delta(N, vector<set<int>>(M + 1));

    int E;
    if (!(fin >> E)) {
        cerr << "Input error: missing number of NFA transitions.\n";
        return 1;
    }
    for (int i = 0; i < E; i++) {
        int from, sym, to;
        if (!(fin >> from >> sym >> to)) {
            cerr << "Input error: incomplete NFA transition list.\n";
            return 1;
        }
        delta[from][sym].insert(to);
    }

    int q0;
    if (!(fin >> q0)) {
        cerr << "Input error: missing initial state.\n";
        return 1;
    }
    int Fc;
    if (!(fin >> Fc)) {
        cerr << "Input error: missing number of final states.\n";
        return 1;
    }
    vector<bool> isFinal(N, false);
    for (int i = 0; i < Fc; i++) {
        int f;
        if (!(fin >> f)) {
            cerr << "Input error: incomplete final-state list.\n";
            return 1;
        }
        isFinal[f] = true;
    }

    cout << "NFA: " << N << " states, alphabet={";
    for (int i = 0; i < M; i++) { if (i) cout << ","; cout << alpha[i]; }
    cout << "}, initial=q" << q0 << ", finals={";
    bool f = true;
    for (int i = 0; i < N; i++) if (isFinal[i]) { if (!f) cout << ","; cout << "q"<<i; f=false; }
    cout << "}\n\n";

    // Check for lambda transitions: nfa2G_R requires lambda-free NFA
    bool hasLambda = false;
    for (int i = 0; i < N; i++) if (!delta[i][M].empty()) { hasLambda = true; break; }
    if (hasLambda) {
        cerr << "WARNING: NFA has lambda transitions. nfa2G_R requires a lambda-free NFA.\n";
        cerr << "Apply lambda-elimination (see 02_nfa_extended / 03_nfa_to_dfa) first.\n";
        cerr << "Continuing with symbol transitions only; lambda transitions ignored.\n\n";
    }

    // nfa2G_R procedure:
    // S1: V = Q (each state qi is a variable); S = q0
    // S2: For each transition delta(qi, aj) = qk: add production qi -> aj qk
    // S3: For each final state qf in F: add production qf -> lambda

    cout << "=== Right-Linear Grammar G_R ===\n";
    cout << "Variables: {";
    for (int i = 0; i < N; i++) { if (i) cout << ","; cout << "q" << i; }
    cout << "}\n";
    cout << "Terminals: {";
    for (int i = 0; i < M; i++) { if (i) cout << ","; cout << alpha[i]; }
    cout << "}\n";
    cout << "Start symbol: q" << q0 << "\n\n";

    cout << "Productions:\n";

    // S2: For each transition delta(qi, aj) = qk
    for (int qi = 0; qi < N; qi++) {
        for (int aj = 0; aj < M; aj++) {
            for (int qk : delta[qi][aj]) {
                cout << "  q" << qi << " -> " << alpha[aj] << " q" << qk << "\n";
            }
        }
        // Lambda transitions: if delta(qi, lambda) = qk, we get qi -> qk
        // But right-linear grammar doesn't allow pure variable productions...
        // The slides only discuss NFA without lambda transitions for this procedure.
        // For NFAs with lambda, we note them but these would require lambda removal first.
        for (int qk : delta[qi][M]) {
            cout << "  q" << qi << " -> [lambda] q" << qk
                 << "  (NOTE: lambda-transition; eliminate before using grammar)\n";
        }
    }

    // S3: For each final state qf, production qf -> lambda
    for (int qf = 0; qf < N; qf++) {
        if (isFinal[qf]) {
            cout << "  q" << qf << " -> lambda\n";
        }
    }

    cout << "\nSummary of grammar productions:\n";
    cout << string(40, '-') << "\n";
    for (int qi = 0; qi < N; qi++) {
        bool hasProd = false;
        for (int aj = 0; aj < M; aj++) if (!delta[qi][aj].empty()) { hasProd = true; break; }
        if (!hasProd && !isFinal[qi]) continue;

        cout << "q" << qi << " -> ";
        bool first2 = true;

        for (int aj = 0; aj < M; aj++) {
            for (int qk : delta[qi][aj]) {
                if (!first2) cout << " | ";
                cout << alpha[aj] << "q" << qk;
                first2 = false;
            }
        }
        if (isFinal[qi]) {
            if (!first2) cout << " | ";
            cout << "lambda";
        }
        cout << "\n";
    }

    return 0;
}
