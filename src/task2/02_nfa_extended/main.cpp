// NFA Extended Transition Function: delta*(q, w) and delta*(T, a)
// Algorithm: find_lambda_closure, move, delta* -- slides 38-42, Chapter 2
//
// Input format (input.txt):
//   N M                  -- N states, M alphabet symbols
//   s0 s1 ... sM-1       -- alphabet (index M = lambda)
//   E                    -- number of transitions
//   from sym_idx to      -- sym_idx = M means lambda transition
//   q0                   -- initial state
//   F_count
//   f0 f1 ...
//   K                    -- number of queries
//   q [w]                -- compute delta*(q, w)
//                           use q alone or q "" for the empty string

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
using namespace std;

struct NFA {
    int N, M;
    vector<char> alpha;
    // delta[state][sym] = set of dest states; sym=M is lambda
    vector<vector<set<int>>> delta;
    int q0;
    set<int> F;
};

string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;

    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;

    return s.substr(start, end - start);
}

string parseQueryWord(const string& raw) {
    string word = trim(raw);
    if (word == "\"\"") return "";
    return word;
}

// S1: compute lambda-closure of single state (all states reachable via lambda)
set<int> lambdaClosure(int q, const NFA& nfa) {
    set<int> closure = {q};
    vector<int> stk = {q};
    while (!stk.empty()) {
        int cur = stk.back(); stk.pop_back();
        for (int nxt : nfa.delta[cur][nfa.M]) {   // nfa.M = lambda column
            if (!closure.count(nxt)) {
                closure.insert(nxt);
                stk.push_back(nxt);
            }
        }
    }
    return closure;
}

// S2: lambda-closure of a set T
set<int> lambdaClosureSet(const set<int>& T, const NFA& nfa) {
    set<int> result;
    for (int q : T) {
        auto lc = lambdaClosure(q, nfa);
        result.insert(lc.begin(), lc.end());
    }
    return result;
}

// S3: move(T, a) -- states reachable from T via symbol a (no lambda)
set<int> moveSet(const set<int>& T, int sym, const NFA& nfa) {
    set<int> result;
    for (int q : T)
        for (int nxt : nfa.delta[q][sym])
            result.insert(nxt);
    return result;
}

// S4: delta*(q, w) = lambda-closure(move(lambda-closure(q), first_sym)) ...
set<int> deltastar(int q, const string& w, const NFA& nfa) {
    set<int> cur = lambdaClosure(q, nfa);
    cout << "    lambda-closure({" << q << "}) = {";
    bool f = true; for (int s : cur) { if (!f) cout << ","; cout << s; f = false; }
    cout << "}\n";

    for (int i = 0; i < (int)w.size(); i++) {
        char c = w[i];
        int idx = -1;
        for (int j = 0; j < nfa.M; j++)
            if (nfa.alpha[j] == c) { idx = j; break; }
        if (idx == -1) {
            cout << "    [symbol '" << c << "' not in alphabet]\n";
            return {};
        }

        set<int> moved = moveSet(cur, idx, nfa);
        cout << "    move({";
        f = true; for (int s : cur) { if (!f) cout << ","; cout << s; f = false; }
        cout << "}, '" << c << "') = {";
        f = true; for (int s : moved) { if (!f) cout << ","; cout << s; f = false; }
        cout << "}\n";

        cur = lambdaClosureSet(moved, nfa);
        cout << "    lambda-closure = {";
        f = true; for (int s : cur) { if (!f) cout << ","; cout << s; f = false; }
        cout << "}\n";
    }
    return cur;
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

    // M+1 columns: M symbols + lambda
    nfa.delta.assign(nfa.N, vector<set<int>>(nfa.M + 1));

    int E;
    if (!(fin >> E)) {
        cerr << "Input error: missing number of NFA transitions.\n";
        return {};
    }
    for (int i = 0; i < E; i++) {
        int from, sym, to;
        if (!(fin >> from >> sym >> to)) {   // sym=nfa.M means lambda
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

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }
    
    NFA nfa = readNFA(fin);
    if (!fin) return 1;

    cout << "NFA: " << nfa.N << " states, alphabet = {";
    for (int i = 0; i < nfa.M; i++) { if (i) cout << ","; cout << nfa.alpha[i]; }
    cout << "}\n";
    cout << "Initial: q" << nfa.q0 << "  Final: {";
    bool f = true;
    for (int s : nfa.F) { if (!f) cout << ","; cout << "q" << s; f = false; }
    cout << "}\n\n";

    int K;
    if (!(fin >> K)) {
        cerr << "Input error: missing number of delta* queries.\n";
        return 1;
    }

    string line;
    getline(fin, line); // consume the newline after K

    for (int queryNo = 0; queryNo < K; queryNo++) {
        if (!getline(fin, line)) {
            cerr << "Input error: missing query line " << (queryNo + 1) << ".\n";
            return 1;
        }

        istringstream iss(line);
        int q;
        if (!(iss >> q)) {
            cerr << "Input error: query line " << (queryNo + 1)
                 << " must start with a state id.\n";
            return 1;
        }

        string rawWord;
        getline(iss, rawWord);
        string w = parseQueryWord(rawWord);

        cout << "delta*(" << q << ", \"" << w << "\"):\n";
        set<int> result = deltastar(q, w, nfa);

        cout << "  => {";
        f = true;
        for (int s : result) { if (!f) cout << ","; cout << s; f = false; }
        cout << "}";

        bool accepted = false;
        for (int s : result) if (nfa.F.count(s)) { accepted = true; break; }
        cout << "  [" << (accepted ? "ACCEPTED" : "REJECTED") << "]\n\n";
    }

    return 0;
}
