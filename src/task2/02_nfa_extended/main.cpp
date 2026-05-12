// NFA Extended Transition Function: delta*(q, w) and delta*(T, a)
// Algorithm: find_lambda_closure, move, delta* -- slides 38-42, Chapter 2
//
// Input format (input/ directory, each .txt file):
//   Flexible named format OR legacy format:
//   N M                  -- N states, M alphabet symbols (legacy)
//   s0 s1 ... sM-1       -- alphabet (index M = lambda)
//   E                    -- number of transitions
//   qX sym qY            -- sym=~ means lambda transition
//   qinit                -- initial state name
//   F_count
//   qf1 qf2 ...
//   K                    -- number of queries
//   qstate [word]        -- compute delta*(qstate, word)
//
// Named format supports sections in any order:
//   alphabet <sym1> <sym2> ...
//   transitions <count>
//   initial <state>
//   finals <state> ...
//   queries <count>
//   states <state> ... (optional - if absent, auto-discovered)

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <filesystem>
#include <map>
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

int parseState(const string& t);

vector<string> splitTokens(const string& line) {
    istringstream iss(line);
    vector<string> tokens;
    string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

bool isBareStateToken(const string& t) {
    return !t.empty() && all_of(t.begin(), t.end(), [](unsigned char c) { return isdigit(c); });
}

bool parseStateList(const vector<string>& tokens, size_t start, vector<int>& states, bool allowDash = false) {
    states.clear();
    if (start >= tokens.size()) return false;
    if (allowDash && tokens.size() == start + 1 && tokens[start] == "-") return true;
    for (size_t i = start; i < tokens.size(); i++) {
        if (!isBareStateToken(tokens[i])) return false;
        states.push_back(stoi(tokens[i]));
    }
    sort(states.begin(), states.end());
    states.erase(unique(states.begin(), states.end()), states.end());
    return true;
}

bool validateStateUniverse(const vector<int>& states, int& N) {
    if (states.empty()) return false;
    for (int i = 0; i < (int)states.size(); i++) {
        if (states[i] != i) return false;
    }
    N = (int)states.size();
    return true;
}

int parseState(const string& t) {
    if (!isBareStateToken(t)) return -1;
    return stoi(t);
}

int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t=="~"||t=="lambda"||t=="lamda"||t=="\"\"") return M;
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

string buildLegacyInputFromNamedFormat(const string& text, const string& fname, string& error) {
    istringstream in(text);
    string line;
    vector<string> allLines;
    if (!getline(in, line)) { error = fname + " is empty"; return ""; }
    allLines.push_back(line);
    while (getline(in, line)) allLines.push_back(line);

    vector<string> alphabetTokens;
    vector<string> transitionLines;
    int E = -1;
    string initialState;
    vector<int> finals;
    vector<int> explicitStates;
    int explicitN = -1;
    bool hasStates = false;
    bool seenAlphabet = false, seenTransitions = false, seenInitial = false, seenFinals = false, seenQueries = false;
    vector<string> queryLines;
    int K = -1;

    for (size_t i = 0; i < allLines.size(); ) {
        vector<string> tokens = splitTokens(allLines[i]);
        if (tokens.empty()) { i++; continue; }

        string key = tokens[0];
        if (key == "states") {
            if (hasStates) { error = "duplicate states section"; return ""; }
            hasStates = true;
            if (tokens.size() < 2) { error = "invalid states line"; return ""; }
            if (!parseStateList(tokens, 1, explicitStates) || !validateStateUniverse(explicitStates, explicitN)) {
                error = "invalid states line"; return "";
            }
            i++;
        } else if (key == "alphabet") {
            if (seenAlphabet) { error = "duplicate alphabet section"; return ""; }
            seenAlphabet = true;
            if (tokens.size() < 2) { error = "expected `alphabet ...`"; return ""; }
            alphabetTokens = vector<string>(tokens.begin() + 1, tokens.end());
            for (const string& sym : alphabetTokens) if (sym.size() != 1) { error = "alphabet symbols must be one character"; return ""; }
            i++;
        } else if (key == "transitions") {
            if (seenTransitions) { error = "duplicate transitions section"; return ""; }
            seenTransitions = true;
            if (tokens.size() != 2 || !isBareStateToken(tokens[1])) { error = "expected `transitions <count>`"; return ""; }
            E = stoi(tokens[1]);
            i++;
            if (i + E > allLines.size()) { error = "incomplete transition list"; return ""; }
            for (int j = 0; j < E; j++) {
                if (splitTokens(allLines[i + j]).size() != 3) { error = "transition rows must be `from symbol to`"; return ""; }
                transitionLines.push_back(allLines[i + j]);
            }
            i += E;
        } else if (key == "initial") {
            if (seenInitial) { error = "duplicate initial section"; return ""; }
            seenInitial = true;
            if (tokens.size() != 2 || !isBareStateToken(tokens[1])) { error = "expected `initial <state>`"; return ""; }
            initialState = tokens[1];
            i++;
        } else if (key == "finals") {
            if (seenFinals) { error = "duplicate finals section"; return ""; }
            seenFinals = true;
            if (tokens.size() < 1) { error = "expected `finals ...`"; return ""; }
            if (!parseStateList(tokens, 1, finals, true)) { error = "invalid finals line"; return ""; }
            i++;
        } else if (key == "queries") {
            if (seenQueries) { error = "duplicate queries section"; return ""; }
            seenQueries = true;
            if (tokens.size() != 2 || !isBareStateToken(tokens[1])) { error = "expected `queries <count>`"; return ""; }
            K = stoi(tokens[1]);
            i++;
            if (i + K > allLines.size()) { error = "incomplete query list"; return ""; }
            for (int j = 0; j < K; j++) {
                queryLines.push_back(allLines[i + j]);
            }
            i += K;
        } else {
            // Unknown line, skip
            i++;
        }
    }

    if (!seenAlphabet) { error = "missing alphabet line"; return ""; }
    if (!seenTransitions) { error = "missing transitions line"; return ""; }
    if (!seenInitial) { error = "missing initial line"; return ""; }
    if (!seenFinals) { error = "missing finals line"; return ""; }
    if (!seenQueries) { error = "missing queries line"; return ""; }

    int M = (int)alphabetTokens.size();
    int N;
    if (hasStates) {
        N = explicitN;
    } else {
        int maxState = -1;
        auto considerState = [&](int s) { if (s > maxState) maxState = s; };
        int init = parseState(initialState); if (init >= 0) considerState(init);
        for (int f : finals) if (f >= 0) considerState(f);
        for (const string& row : transitionLines) {
            vector<string> t = splitTokens(row);
            if (t.size() == 3) {
                int from = parseState(t[0]), to = parseState(t[2]);
                if (from >= 0) considerState(from);
                if (to >= 0) considerState(to);
            }
        }
        if (maxState < 0) { error = "no states found"; return ""; }
        N = maxState + 1;
    }

    ostringstream out;
    out << N << " " << M << "\n";
    for (size_t i = 0; i < alphabetTokens.size(); i++) {
        if (i) out << " ";
        out << alphabetTokens[i];
    }
    out << "\n" << E << "\n";
    for (const string& row : transitionLines) out << row << "\n";
    out << initialState << "\n";
    out << finals.size() << "\n";
    if (!finals.empty()) {
        for (size_t i = 0; i < finals.size(); i++) {
            if (i) out << " ";
            out << finals[i];
        }
        out << "\n";
    }
    out << K << "\n";
    for (const string& row : queryLines) out << row << "\n";
    return out.str();
}

NFA readNFA(istream& fin, const string& fname) {
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
        if (from < 0 || from >= nfa.N || sym < 0 || sym > nfa.M || to < 0 || to >= nfa.N) {
            cerr << "Input error: invalid transition " << sf << " " << ss << " " << st << "\n";
            return {};
        }
        nfa.delta[from][sym].insert(to);
    }
    string sq0;
    if (!(fin >> sq0)) { cerr << "Input error: missing initial state.\n"; return {}; }
    nfa.q0 = parseState(sq0);
    if (nfa.q0 < 0 || nfa.q0 >= nfa.N) {
        cerr << "Input error: invalid initial state " << sq0 << "\n";
        return {};
    }
    int Fc;
    if (!(fin >> Fc)) { cerr << "Input error: missing number of final states.\n"; return {}; }
    for (int i = 0; i < Fc; i++) {
        string sf;
        if (!(fin >> sf)) { cerr << "Input error: incomplete final-state list.\n"; return {}; }
        int f = parseState(sf);
        if (f < 0 || f >= nfa.N) {
            cerr << "Input error: invalid final state " << sf << "\n";
            return {};
        }
        nfa.F.insert(f);
    }
    return nfa;
}

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) { cerr << "Cannot open " << fp.filename().string() << "\n"; return false; }
    string text((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    istringstream probe(text);
    string firstLine;
    if (!getline(probe, firstLine)) {
        cerr << "Input error: " << fp.filename().string() << " is empty.\n";
        return false;
    }

    string canonical = text;
    vector<string> firstTokens = splitTokens(firstLine);
    bool isNamedFormat = (!firstTokens.empty() && firstTokens[0] == "states");
    if (!isNamedFormat) {
        istringstream check(text);
        string l;
        while (getline(check, l)) {
            vector<string> t = splitTokens(l);
            if (!t.empty() && (t[0] == "alphabet" || t[0] == "transitions" || t[0] == "initial" ||
                               t[0] == "finals" || t[0] == "queries" || t[0] == "strings")) {
                isNamedFormat = true;
                break;
            }
        }
    }
    if (isNamedFormat) {
        string error;
        canonical = buildLegacyInputFromNamedFormat(text, fp.filename().string(), error);
        if (canonical.empty()) {
            cerr << "Input error: " << error << ".\n";
            return false;
        }
    }

    istringstream input(canonical);
    NFA nfa = readNFA(input, fp.filename().string());
    if (nfa.N == 0) return false;

    cout << "NFA: " << nfa.N << " states, alphabet = {";
    for (int i = 0; i < nfa.M; i++) { if (i) cout << ","; cout << nfa.alpha[i]; }
    cout << "}\n";
    cout << "Initial: q" << nfa.q0 << "  Final: {";
    bool f = true;
    for (int s : nfa.F) { if (!f) cout << ","; cout << "q" << s; f = false; }
    cout << "}\n\n";

    int K;
    if (!(input >> K)) { cerr << "Input error: missing number of delta* queries.\n"; return false; }
    string line;
    getline(input, line);

    for (int queryNo = 0; queryNo < K; queryNo++) {
        if (!getline(input, line)) {
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
