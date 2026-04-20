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
//                           use a blank line or "" for the empty string

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
using namespace std;

string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;

    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;

    return s.substr(start, end - start);
}

string parseInputWord(const string& raw) {
    string word = trim(raw);
    if (word == "\"\"") return "";
    return word;
}

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }

    int N, M;
    if (!(fin >> N >> M)) {
        cerr << "Input error: input.txt is empty or missing the DFA header.\n";
        return 1;
    }

    vector<char> alpha(M);
    for (int i = 0; i < M; i++) {
        if (!(fin >> alpha[i])) {
            cerr << "Input error: missing alphabet symbols.\n";
            return 1;
        }
    }

    // delta[state][sym_idx] = next state, -1 = dead/trap
    vector<vector<int>> delta(N, vector<int>(M, -1));

    int E;
    if (!(fin >> E)) {
        cerr << "Input error: missing number of DFA transitions.\n";
        return 1;
    }
    for (int i = 0; i < E; i++) {
        int from, sym, to;
        if (!(fin >> from >> sym >> to)) {
            cerr << "Input error: incomplete DFA transition list.\n";
            return 1;
        }
        delta[from][sym] = to;
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
    if (!(fin >> K)) {
        cerr << "Input error: missing number of test strings.\n";
        return 1;
    }

    string line;
    getline(fin, line); // consume the newline after K

    for (int caseNo = 0; caseNo < K; caseNo++) {
        if (!getline(fin, line)) {
            cerr << "Input error: missing test string on line " << (caseNo + 1) << ".\n";
            return 1;
        }
        string w = parseInputWord(line);
        string shown = w.empty() ? "\"\"" : w;

        cout << "Input: " << shown << "\n";
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
