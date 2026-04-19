// DFA String Acceptance Check
// Algorithm: delta(q, w) -- slide 20-23, Chapter 2
//
// Input format (input.txt):
//   N M                  -- N states (0..N-1), M alphabet symbols
//   s0 s1 ... sM-1       -- alphabet characters
//   E                    -- number of transitions
//   from sym_idx to      -- each transition (sym_idx: 0..M-1)
//   q0                   -- initial state
//   F_count
//   f0 f1 ...            -- final states
//   K                    -- number of test strings
//   w0 w1 ...            -- test strings (one per line)

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }

    int N, M;
    fin >> N >> M;

    vector<char> alpha(M);
    for (int i = 0; i < M; i++) fin >> alpha[i];

    // delta[state][sym_idx] = next state, -1 = dead/trap
    vector<vector<int>> delta(N, vector<int>(M, -1));

    int E;
    fin >> E;
    for (int i = 0; i < E; i++) {
        int from, sym, to;
        fin >> from >> sym >> to;
        delta[from][sym] = to;
    }

    int q0;
    fin >> q0;

    int Fc;
    fin >> Fc;
    vector<bool> isFinal(N, false);
    for (int i = 0; i < Fc; i++) {
        int f; fin >> f;
        isFinal[f] = true;
    }

    cout << "DFA: " << N << " states, alphabet = {";
    for (int i = 0; i < M; i++) { if (i) cout << ","; cout << alpha[i]; }
    cout << "}\n";
    cout << "Initial: q" << q0 << "  Final: {";
    bool first = true;
    for (int i = 0; i < N; i++) if (isFinal[i]) {
        if (!first) cout << ","; cout << "q" << i; first = false;
    }
    cout << "}\n\n";

    int K;
    fin >> K;
    while (K--) {
        string w;
        fin >> w;

        cout << "Input: \"" << w << "\"\n";
        cout << "  Steps: q" << q0;

        int cur = q0;
        bool dead = false;

        for (char c : w) {
            int idx = -1;
            for (int i = 0; i < M; i++)
                if (alpha[i] == c) { idx = i; break; }

            if (idx == -1) {
                cout << " --" << c << "--> [INVALID SYMBOL]";
                dead = true; break;
            }

            int nxt = delta[cur][idx];
            cout << " --" << c << "--> ";
            if (nxt == -1) {
                cout << "DEAD";
                dead = true; break;
            }
            cout << "q" << nxt;
            cur = nxt;
        }

        cout << "\n  Result: ";
        if (dead)         cout << "REJECTED (dead state)\n";
        else if (isFinal[cur]) cout << "ACCEPTED\n";
        else               cout << "REJECTED (non-final state q" << cur << ")\n";
        cout << "\n";
    }

    return 0;
}
