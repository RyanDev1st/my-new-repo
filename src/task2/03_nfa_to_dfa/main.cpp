// NFA to DFA Conversion (Subset Construction)
// Algorithm: nfa-to-dfa procedure -- slides 45-46, 53, Chapter 2
//
// Input format (input/ directory, each .txt file):
//   N M
//   s0 ... sM-1
//   E
//   qX sym qY     (sym=~ means lambda)
//   qinit
//   F_count
//   qf1 qf2 ...

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;
using namespace std;

// Tee: write to two streambufs simultaneously
class TeeBuf : public streambuf {
    streambuf *b1, *b2;
public:
    TeeBuf(streambuf* b1, streambuf* b2) : b1(b1), b2(b2) {}
    int overflow(int c) override {
        if (c == EOF) return !EOF;
        int r1 = b1->sputc(c), r2 = b2->sputc(c);
        return (r1 == EOF || r2 == EOF) ? EOF : c;
    }
    streamsize xsputn(const char* s, streamsize n) override {
        b1->sputn(s, n); return b2->sputn(s, n);
    }
};

struct NFA {
    int N, M;
    vector<char> alpha;
    vector<vector<set<int>>> delta;
    int q0;
    set<int> F;
};

int parseState(const string& t) {
    if (!t.empty() && (t[0]=='q'||t[0]=='Q')) return stoi(t.substr(1));
    return stoi(t);
}

int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t=="~"||t=="lambda") return M;
    if (t.size()==1) for (int i=0;i<M;i++) if(alpha[i]==t[0]) return i;
    return -1;
}

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
    for (int q : T) for (int nxt : nfa.delta[q][sym]) result.insert(nxt);
    return result;
}

NFA readNFA(ifstream& fin, const string& fname) {
    NFA nfa;
    if (!(fin >> nfa.N >> nfa.M)) {
        cerr << "Input error: " << fname << " is empty or missing the NFA header.\n"; return {};
    }
    nfa.alpha.resize(nfa.M);
    for (int i = 0; i < nfa.M; i++) {
        if (!(fin >> nfa.alpha[i])) { cerr << "Input error: missing alphabet symbols.\n"; return {}; }
    }
    nfa.delta.assign(nfa.N, vector<set<int>>(nfa.M + 1));
    int E;
    if (!(fin >> E)) { cerr << "Input error: missing number of NFA transitions.\n"; return {}; }
    for (int i = 0; i < E; i++) {
        string sf, ss, st;
        if (!(fin >> sf >> ss >> st)) { cerr << "Input error: incomplete NFA transition list.\n"; return {}; }
        int from = parseState(sf);
        int sym = parseSym(ss, nfa.alpha, nfa.M);
        int to = parseState(st);
        nfa.delta[from][sym].insert(to);
    }
    string sq0;
    if (!(fin >> sq0)) { cerr << "Input error: missing initial state.\n"; return {}; }
    nfa.q0 = parseState(sq0);
    int Fc;
    if (!(fin >> Fc)) { cerr << "Input error: missing number of final states.\n"; return {}; }
    for (int i = 0; i < Fc; i++) {
        string sf;
        if (!(fin >> sf)) { cerr << "Input error: incomplete final-state list.\n"; return {}; }
        nfa.F.insert(parseState(sf));
    }
    return nfa;
}

void printSet(const set<int>& s) {
    cout << "{";
    bool f = true;
    for (int x : s) { if (!f) cout << ","; cout << x; f = false; }
    cout << "}";
}

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) { cerr << "Cannot open " << fp.filename().string() << "\n"; return false; }
    NFA nfa = readNFA(fin, fp.filename().string());
    if (nfa.N == 0) return false;

    cout << "NFA: " << nfa.N << " states, alphabet = {";
    for (int i = 0; i < nfa.M; i++) { if (i) cout << ","; cout << nfa.alpha[i]; }
    cout << "}\n";
    cout << "Initial: q" << nfa.q0 << "  Final NFA states: {";
    bool f = true;
    for (int s : nfa.F) { if (!f) cout << ","; cout << "q" << s; f = false; }
    cout << "}\n";

    // Subset construction
    map<set<int>, int> dfaStateId;
    vector<set<int>> dfaStates;
    queue<set<int>> worklist;

    set<int> startSet = lambdaClosure(nfa.q0, nfa);
    dfaStateId[startSet] = 0;
    dfaStates.push_back(startSet);
    worklist.push(startSet);

    vector<vector<int>> dfaTrans;
    set<int> dfaFinal;

    cout << "Subset construction:\n";
    cout << "  Start: ";
    printSet(startSet);
    cout << " = D0\n";

    while (!worklist.empty()) {
        set<int> cur = worklist.front(); worklist.pop();
        int curId = dfaStateId[cur];
        dfaTrans.resize(dfaStates.size(), vector<int>(nfa.M, -1));

        bool isFinal = false;
        for (int s : cur) if (nfa.F.count(s)) { isFinal = true; break; }
        if (isFinal) dfaFinal.insert(curId);

        for (int a = 0; a < nfa.M; a++) {
            set<int> moved = moveSet(cur, a, nfa);
            set<int> next = lambdaClosureSet(moved, nfa);
            if (next.empty()) continue;

            if (!dfaStateId.count(next)) {
                int newId = (int)dfaStates.size();
                dfaStateId[next] = newId;
                dfaStates.push_back(next);
                worklist.push(next);
                dfaTrans.resize(dfaStates.size(), vector<int>(nfa.M, -1));
                cout << "  New DFA state D" << newId << " = ";
                printSet(next);
                cout << "\n";
            }
            dfaTrans[curId][a] = dfaStateId[next];
        }
    }

    int Nd = (int)dfaStates.size();
    cout << "\nResulting DFA: " << Nd << " states\n";

    cout << "DFA Transition Table:\n";
    cout << "State\t\t";
    for (int a = 0; a < nfa.M; a++) cout << nfa.alpha[a] << "\t\t";
    cout << "\n" << string(50, '-') << "\n";

    for (int d = 0; d < Nd; d++) {
        cout << "D" << d;
        if (d == 0) cout << "(init)";
        if (dfaFinal.count(d)) cout << "(final)";
        cout << "=";
        printSet(dfaStates[d]);
        cout << "\t\t";
        for (int a = 0; a < nfa.M; a++) {
            int nxt = (d < (int)dfaTrans.size() && a < (int)dfaTrans[d].size()) ? dfaTrans[d][a] : -1;
            if (nxt == -1) cout << "dead\t\t";
            else cout << "D" << nxt << "\t\t";
        }
        cout << "\n";
    }

    cout << "\nInitial DFA state: D0\n";
    cout << "Final DFA states: {";
    f = true;
    for (int d : dfaFinal) { if (!f) cout << ","; cout << "D" << d; f = false; }
    cout << "}\n";
    return true;
}

int main() {
    fs::path inputDir = "input";
    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        cerr << "Error: input/ directory not found.\n"; return 1;
    }
    vector<fs::path> files;
    for (auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
            files.push_back(entry.path());
    }
    if (files.empty()) { cerr << "Error: no .txt files found in input/.\n"; return 1; }
    sort(files.begin(), files.end());

    fs::create_directories("output");

    for (auto& fp : files) {
        string outPath = "output/" + fp.filename().string();
        ofstream fout(outPath);
        TeeBuf tee(cout.rdbuf(), fout.rdbuf());
        streambuf* oldBuf = cout.rdbuf(&tee);

        cout << "=== " << fp.filename().string() << " ===\n";
        processFile(fp);
        cout << "\n";

        cout.rdbuf(oldBuf);
    }
    return 0;
}
