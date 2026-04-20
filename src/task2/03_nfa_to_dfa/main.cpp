// NFA to DFA Conversion (Subset Construction)
// Algorithm: nfa-to-dfa procedure -- slides 45-46, 53, Chapter 2
//
// Input format (input.txt): same as 02_nfa_extended
//   N M
//   s0 ... sM-1
//   E
//   from sym_idx to     (sym_idx = M = lambda)
//   q0
//   F_count
//   f0 f1 ...

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <string>
using namespace std;

struct NFA {
    int N, M;
    vector<char> alpha;
    vector<vector<set<int>>> delta;
    int q0;
    set<int> F;
};

set<int> lambdaClosure(int q, const NFA& nfa) {
    set<int> closure = {q};
    vector<int> stk = {q};
    while (!stk.empty()) {
        int cur = stk.back(); stk.pop_back();
        for (int nxt : nfa.delta[cur][nfa.M])
            if (!closure.count(nxt)) { closure.insert(nxt); stk.push_back(nxt); }
    }
    return closure;
}

set<int> lambdaClosureSet(const set<int>& T, const NFA& nfa) {
    set<int> result;
    for (int q : T) { auto lc = lambdaClosure(q, nfa); result.insert(lc.begin(), lc.end()); }
    return result;
}

set<int> moveSet(const set<int>& T, int sym, const NFA& nfa) {
    set<int> result;
    for (int q : T)
        for (int nxt : nfa.delta[q][sym]) result.insert(nxt);
    return result;
}

NFA readNFA(ifstream& fin) {
    NFA nfa;
    if (!(fin >> nfa.N >> nfa.M)) {
        cerr << "Input error: input.txt is empty or missing the NFA header.\n";
        return {};
    }
    nfa.alpha.resize(nfa.M);
    for (int i = 0; i < nfa.M; i++) {
        if (!(fin >> nfa.alpha[i])) {
            cerr << "Input error: missing alphabet symbols.\n";
            return {};
        }
    }
    nfa.delta.assign(nfa.N, vector<set<int>>(nfa.M + 1));
    int E;
    if (!(fin >> E)) {
        cerr << "Input error: missing number of NFA transitions.\n";
        return {};
    }
    for (int i = 0; i < E; i++) {
        int from, sym, to;
        if (!(fin >> from >> sym >> to)) {
            cerr << "Input error: incomplete NFA transition list.\n";
            return {};
        }
        nfa.delta[from][sym].insert(to);
    }
    if (!(fin >> nfa.q0)) {
        cerr << "Input error: missing initial state.\n";
        return {};
    }
    int Fc;
    if (!(fin >> Fc)) {
        cerr << "Input error: missing number of final states.\n";
        return {};
    }
    for (int i = 0; i < Fc; i++) {
        int f;
        if (!(fin >> f)) {
            cerr << "Input error: incomplete final-state list.\n";
            return {};
        }
        nfa.F.insert(f);
    }
    return nfa;
}

void printSet(const set<int>& s) {
    cout << "{";
    bool f = true;
    for (int x : s) { if (!f) cout << ","; cout << x; f = false; }
    cout << "}";
}

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }

    NFA nfa = readNFA(fin);
    if (!fin) return 1;

    cout << "NFA: " << nfa.N << " states, alphabet = {";
    for (int i = 0; i < nfa.M; i++) { if (i) cout << ","; cout << nfa.alpha[i]; }
    cout << "}, initial=q" << nfa.q0 << ", finals={";
    bool f = true; for (int s : nfa.F) { if (!f) cout << ","; cout << "q"<<s; f=false; }
    cout << "}\n\n";

    // Subset construction
    // DFA state = set of NFA states (after lambda-closure)
    // S1: Initial DFA state = lambda-closure(q0)
    set<int> initSet = lambdaClosure(nfa.q0, nfa);

    map<set<int>, int> stateId;
    vector<set<int>> stateList;
    queue<set<int>> worklist;

    auto getOrAdd = [&](const set<int>& s) -> int {
        auto it = stateId.find(s);
        if (it != stateId.end()) return it->second;
        int id = (int)stateList.size();
        stateId[s] = id;
        stateList.push_back(s);
        worklist.push(s);
        return id;
    };

    getOrAdd(initSet);

    // dfaDelta[dfa_state][sym] = next dfa_state (-1 = dead)
    map<int, vector<int>> dfaDelta;

    // S2-S6: For each DFA state, compute transitions
    cout << "Subset construction steps:\n";
    while (!worklist.empty()) {
        set<int> cur = worklist.front(); worklist.pop();
        int curId = stateId[cur];

        cout << "  D" << curId << " = "; printSet(cur); cout << ":\n";
        dfaDelta[curId].assign(nfa.M, -1);

        for (int a = 0; a < nfa.M; a++) {
            // S3: compute delta*({cur states}, a)
            set<int> moved = moveSet(cur, a, nfa);
            set<int> next = lambdaClosureSet(moved, nfa);

            cout << "    on '" << nfa.alpha[a] << "': move="; printSet(moved);
            cout << " -> lc="; printSet(next);

            if (!next.empty()) {
                int nextId = getOrAdd(next);
                dfaDelta[curId][a] = nextId;
                cout << " = D" << nextId;
            } else {
                cout << " = dead";
            }
            cout << "\n";
        }
    }

    int numDFA = (int)stateList.size();
    cout << "\n=== Resulting DFA ===\n";
    cout << "States: " << numDFA << " (D0 .. D" << numDFA-1 << ")\n";
    cout << "Alphabet: {";
    for (int i = 0; i < nfa.M; i++) { if (i) cout << ","; cout << nfa.alpha[i]; }
    cout << "}\n\n";

    // Transition table header
    cout << "State\t\t";
    for (int a = 0; a < nfa.M; a++) cout << nfa.alpha[a] << "\t\t";
    cout << "NFA-states\n";
    cout << string(60, '-') << "\n";

    for (int i = 0; i < numDFA; i++) {
        cout << "D" << i;
        if (stateId[initSet] == i) cout << "(init)";

        // Check if final (contains any NFA final state)
        bool isFinal = false;
        for (int s : stateList[i]) if (nfa.F.count(s)) { isFinal = true; break; }
        if (isFinal) cout << "(final)";
        cout << "\t";

        for (int a = 0; a < nfa.M; a++) {
            int nxt = dfaDelta[i][a];
            if (nxt == -1) cout << "dead\t\t";
            else cout << "D" << nxt << "\t\t";
        }
        printSet(stateList[i]);
        cout << "\n";
    }

    cout << "\nInitial state: D" << stateId[initSet] << "\n";
    cout << "Final states: ";
    for (int i = 0; i < numDFA; i++) {
        bool isFinal = false;
        for (int s : stateList[i]) if (nfa.F.count(s)) { isFinal = true; break; }
        if (isFinal) cout << "D" << i << " ";
    }
    cout << "\n";

    return 0;
}
