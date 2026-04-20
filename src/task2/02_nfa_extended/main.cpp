// NFA Extended Transition Function: delta*(q, w) and delta*(T, a)
// Algorithm: find_lambda_closure, move, delta* -- slides 38-42, Chapter 2
//
// Input format (input/ directory, each .txt file):
//   N M                  -- N states, M alphabet symbols
//   s0 s1 ... sM-1       -- alphabet (index M = lambda)
//   E                    -- number of transitions
//   qX sym qY            -- sym=~ means lambda transition
//   qinit                -- initial state name
//   F_count
//   qf1 qf2 ...
//   K                    -- number of queries
//   qstate [word]        -- compute delta*(qstate, word)

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <filesystem>
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
        for (int nxt : nfa.delta[cur][nfa.M]) {
            if (!closure.count(nxt)) { closure.insert(nxt); stk.push_back(nxt); }
        }
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

set<int> deltastar(int q, const string& w, const NFA& nfa) {
    set<int> cur = lambdaClosure(q, nfa);
    cout << "    lambda-closure({" << q << "}) = {";
    bool f = true; for (int s : cur) { if (!f) cout << ","; cout << s; f = false; }
    cout << "}\n";

    for (int i = 0; i < (int)w.size(); i++) {
        char c = w[i];
        int idx = -1;
        for (int j = 0; j < nfa.M; j++) if (nfa.alpha[j] == c) { idx = j; break; }
        if (idx == -1) { cout << "    [symbol not in alphabet]\n"; return {}; }

        set<int> moved = moveSet(cur, idx, nfa);
        cout << "    move({";
        f = true; for (int s : cur) { if (!f) cout << ","; cout << s; f = false; }
        cout << "}, " << c << ") = {";
        f = true; for (int s : moved) { if (!f) cout << ","; cout << s; f = false; }
        cout << "}\n";

        cur = lambdaClosureSet(moved, nfa);
        cout << "    lambda-closure = {";
        f = true; for (int s : cur) { if (!f) cout << ","; cout << s; f = false; }
        cout << "}\n";
    }
    return cur;
}

NFA readNFA(ifstream& fin, const string& fname) {
    NFA nfa;
    if (!(fin >> nfa.N >> nfa.M)) {
        cerr << "Input error: " << fname << " is empty or missing the NFA header.\n";
        return {};
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
        if (from < 0 || from >= nfa.N || sym < 0 || sym > nfa.M || to < 0 || to >= nfa.N) {
            cerr << "Input error: invalid transition " << sf << " " << ss << " " << st << "\n";
            return {};
        }
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

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) { cerr << "Cannot open " << fp.filename().string() << "\n"; return false; }
    NFA nfa = readNFA(fin, fp.filename().string());
    if (nfa.N == 0) return false;

    cout << "NFA: " << nfa.N << " states, alphabet = {";
    for (int i = 0; i < nfa.M; i++) { if (i) cout << ","; cout << nfa.alpha[i]; }
    cout << "}\n";
    cout << "Initial: q" << nfa.q0 << "  Final: {";
    bool f = true;
    for (int s : nfa.F) { if (!f) cout << ","; cout << "q" << s; f = false; }
    cout << "}\n\n";

    int K;
    if (!(fin >> K)) { cerr << "Input error: missing number of delta* queries.\n"; return false; }
    string line;
    getline(fin, line);

    for (int queryNo = 0; queryNo < K; queryNo++) {
        if (!getline(fin, line)) {
            cerr << "Input error: missing query line " << (queryNo + 1) << "\n";
            return false;
        }
        istringstream iss(line);
        string sqstate;
        if (!(iss >> sqstate)) {
            cerr << "Input error: query line " << (queryNo + 1) << " must start with a state name.\n";
            return false;
        }
        int q = parseState(sqstate);
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
