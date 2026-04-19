// DFA State Minimization
// Algorithm: mark() + reduce() -- slides 59-60, 64, 72, Chapter 2
//
// Input format (input.txt): same as 01_dfa_acceptance (no test strings)
//   N M
//   s0 ... sM-1
//   E
//   from sym_idx to
//   q0
//   F_count
//   f0 f1 ...

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
using namespace std;

struct DFA {
    int N, M;
    vector<char> alpha;
    vector<vector<int>> delta; // delta[state][sym] = next, -1 = dead
    int q0;
    vector<bool> isFinal;
};

DFA readDFA(ifstream& fin) {
    DFA dfa;
    fin >> dfa.N >> dfa.M;
    dfa.alpha.resize(dfa.M);
    for (int i = 0; i < dfa.M; i++) fin >> dfa.alpha[i];
    dfa.delta.assign(dfa.N, vector<int>(dfa.M, -1));
    int E; fin >> E;
    for (int i = 0; i < E; i++) {
        int from, sym, to; fin >> from >> sym >> to;
        dfa.delta[from][sym] = to;
    }
    fin >> dfa.q0;
    int Fc; fin >> Fc;
    dfa.isFinal.assign(dfa.N, false);
    for (int i = 0; i < Fc; i++) { int f; fin >> f; dfa.isFinal[f] = true; }
    return dfa;
}

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }

    DFA dfa = readDFA(fin);
    int N = dfa.N;

    cout << "Input DFA: " << N << " states, alphabet = {";
    for (int i = 0; i < dfa.M; i++) { if (i) cout << ","; cout << dfa.alpha[i]; }
    cout << "}, initial=q" << dfa.q0 << "\n\n";

    // === mark() procedure ===
    // S1: Remove inaccessible states via BFS from q0
    vector<bool> accessible(N, false);
    {
        queue<int> bfs;
        bfs.push(dfa.q0);
        accessible[dfa.q0] = true;
        while (!bfs.empty()) {
            int cur = bfs.front(); bfs.pop();
            for (int a = 0; a < dfa.M; a++) {
                int nxt = dfa.delta[cur][a];
                if (nxt != -1 && !accessible[nxt]) {
                    accessible[nxt] = true;
                    bfs.push(nxt);
                }
            }
        }
    }

    cout << "Accessible states: {";
    bool f = true;
    for (int i = 0; i < N; i++) if (accessible[i]) { if (!f) cout << ","; cout << "q"<<i; f=false; }
    cout << "}\n";
    cout << "Inaccessible states: {";
    f = true;
    for (int i = 0; i < N; i++) if (!accessible[i]) { if (!f) cout << ","; cout << "q"<<i; f=false; }
    cout << "}\n\n";

    // Use virtual DEAD state at index N for missing transitions
    // States for marking: 0..N-1 (accessible only) + N (dead state)
    // isFinal[DEAD] = false

    // mark[i][j] (i < j) = true if states i,j are distinguishable
    int total = N + 1; // include dead state
    vector<vector<bool>> mark(total, vector<bool>(total, false));

    // S2: Mark (p,q) if one is final and the other is not
    auto isFinalOf = [&](int s) -> bool {
        if (s >= N) return false; // dead state is not final
        return dfa.isFinal[s];
    };
    auto isAccessOf = [&](int s) -> bool {
        if (s >= N) return true; // dead state always "accessible" in the sense we consider it
        return accessible[s];
    };

    cout << "mark() procedure:\n";
    cout << "  S2: Initial marking (final vs non-final):\n";
    for (int i = 0; i < total; i++) {
        if (i < N && !accessible[i]) continue;
        for (int j = i + 1; j < total; j++) {
            if (j < N && !accessible[j]) continue;
            if (isFinalOf(i) != isFinalOf(j)) {
                mark[i][j] = true;
                auto label = [&](int s){ return s < N ? "q"+to_string(s) : "dead"; };
                cout << "    mark(" << label(i) << ", " << label(j) << ")\n";
            }
        }
    }

    // S3: Iterate until stable -- mark (p,q) if delta(p,a) and delta(q,a) are already marked
    int pass = 1;
    bool changed = true;
    while (changed) {
        changed = false;
        cout << "  Pass " << pass++ << ":\n";
        for (int i = 0; i < total; i++) {
            if (i < N && !accessible[i]) continue;
            for (int j = i + 1; j < total; j++) {
                if (j < N && !accessible[j]) continue;
                if (mark[i][j]) continue;
                for (int a = 0; a < dfa.M; a++) {
                    int di = (i < N) ? dfa.delta[i][a] : -1;
                    int dj = (j < N) ? dfa.delta[j][a] : -1;
                    if (di == -1) di = N; // dead
                    if (dj == -1) dj = N; // dead
                    if (di == dj) continue;
                    int lo = min(di, dj), hi = max(di, dj);
                    if (mark[lo][hi]) {
                        mark[i][j] = true;
                        changed = true;
                        auto lbl = [&](int s){ return s < N ? "q"+to_string(s) : "dead"; };
                        cout << "    mark(" << lbl(i) << ", " << lbl(j) << ") via '" << dfa.alpha[a] << "'\n";
                        break;
                    }
                }
            }
        }
        if (!changed) cout << "    (no new markings - stable)\n";
    }

    cout << "\nIndistinguishable pairs: ";
    f = true;
    for (int i = 0; i < N; i++) {
        if (!accessible[i]) continue;
        for (int j = i + 1; j < N; j++) {
            if (!accessible[j]) continue;
            if (!mark[i][j]) {
                if (!f) cout << ", ";
                cout << "(q" << i << ",q" << j << ")";
                f = false;
            }
        }
    }
    if (f) cout << "(none)";
    cout << "\n\n";

    // === reduce() procedure ===
    // Build equivalence classes from unmarked pairs
    vector<int> classOf(N, -1);
    int numClasses = 0;
    vector<vector<int>> classes;

    for (int i = 0; i < N; i++) {
        if (!accessible[i]) continue;
        if (classOf[i] != -1) continue;
        classes.push_back({i});
        classOf[i] = numClasses;
        for (int j = i + 1; j < N; j++) {
            if (!accessible[j]) continue;
            if (classOf[j] != -1) continue; // already assigned
            if (!mark[i][j]) {
                classOf[j] = numClasses;
                classes.back().push_back(j);
            }
        }
        numClasses++;
    }

    // Output minimized DFA
    cout << "=== Minimized DFA ===\n";
    cout << "States: " << numClasses << "\n\n";

    int initClass = classOf[dfa.q0];

    cout << "Transition table:\n";
    cout << "Class\t\tStates\t\t";
    for (int a = 0; a < dfa.M; a++) cout << "on " << dfa.alpha[a] << "\t\t";
    cout << "\n" << string(70, '-') << "\n";

    for (int c = 0; c < numClasses; c++) {
        cout << "C" << c;
        if (c == initClass) cout << "(init)";
        bool anyFinal = false;
        for (int s : classes[c]) if (dfa.isFinal[s]) { anyFinal = true; break; }
        if (anyFinal) cout << "(final)";
        cout << "\t\t{";
        for (int k = 0; k < (int)classes[c].size(); k++) {
            if (k) cout << ","; cout << "q" << classes[c][k];
        }
        cout << "}\t\t";

        int rep = classes[c][0];
        for (int a = 0; a < dfa.M; a++) {
            int nxt = dfa.delta[rep][a];
            if (nxt == -1 || !accessible[nxt]) cout << "dead\t\t";
            else cout << "C" << classOf[nxt] << "\t\t";
        }
        cout << "\n";
    }

    cout << "\nInitial state: C" << initClass << "\n";
    cout << "Final states: ";
    for (int c = 0; c < numClasses; c++) {
        bool anyFinal = false;
        for (int s : classes[c]) if (dfa.isFinal[s]) { anyFinal = true; break; }
        if (anyFinal) cout << "C" << c << " ";
    }
    cout << "\n";

    return 0;
}
