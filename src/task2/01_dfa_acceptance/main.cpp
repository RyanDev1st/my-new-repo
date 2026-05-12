// DFA acceptance.
// Algorithm: DFA simulation from Chapter 2.
// Input pipeline: read each .txt file in input/ with labeled sections like
// `states`, `alphabet`, `transitions`, `initial`, `finals`, and `strings`.
// Transition rows use `from symbol to`, for example `0 a 1`.
// Output: step trace and ACCEPTED / REJECTED result.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <map>
namespace fs = std::filesystem;
using namespace std;

// Write to console and output file.
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

int parseState(const string& t) {
    if (!isBareStateToken(t)) return -1;
    return stoi(t);
}

int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t.size()==1) for (int i=0;i<M;i++) if(alpha[i]==t[0]) return i;
    return -1;
}

bool parseStateList(const vector<string>& tokens, size_t start, vector<int>& states, bool allowDash = false) {
    states.clear();
    if (start >= tokens.size()) return false;
    if (allowDash && tokens.size() == start + 1 && tokens[start] == "-") return true;
    for (size_t i = start; i < tokens.size(); i++) {
        int state = parseState(tokens[i]);
        if (state < 0) return false;
        states.push_back(state);
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
    vector<string> stringLines;
    int K = -1;
    bool hasStates = false;
    bool seenAlphabet = false, seenTransitions = false, seenInitial = false, seenFinals = false, seenStrings = false;

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
        } else if (key == "strings" || key == "queries") {
            if (seenStrings) { error = "duplicate strings section"; return ""; }
            seenStrings = true;
            if (tokens.size() != 2 || !isBareStateToken(tokens[1])) { error = "expected `strings <count>` or `queries <count>`"; return ""; }
            K = stoi(tokens[1]);
            i++;
            if (i + K > allLines.size()) { error = "incomplete string list"; return ""; }
            for (int j = 0; j < K; j++) {
                stringLines.push_back(allLines[i + j]);
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
    if (!seenStrings) { error = "missing strings line"; return ""; }

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
    for (const string& row : stringLines) out << row << "\n";
    return out.str();
}

struct DFA {
    int N, M;
    vector<char> alpha;
    vector<vector<int>> delta;
    int q0;
    vector<bool> isFinal;
    vector<pair<string, string>> queries; // (original, parsed)
};

DFA readDFA(istream& in, const string& fname) {
    DFA dfa;
    if (!(in >> dfa.N >> dfa.M)) {
        cerr << "Input error: " << fname << " missing header.\n";
        return {};
    }
    dfa.alpha.resize(dfa.M);
    for (int i = 0; i < dfa.M; i++) {
        if (!(in >> dfa.alpha[i])) {
            cerr << "Input error: missing alphabet.\n";
            return {};
        }
    }
    dfa.delta.assign(dfa.N, vector<int>(dfa.M, -1));
    int E;
    if (!(in >> E)) {
        cerr << "Input error: missing E.\n";
        return {};
    }
    for (int i = 0; i < E; i++) {
        string sf, ss, st;
        if (!(in >> sf >> ss >> st)) {
            cerr << "Input error: bad transition.\n";
            return {};
        }
        int from = parseState(sf), sym = parseSym(ss, dfa.alpha, dfa.M), to = parseState(st);
        if (from < 0 || from >= dfa.N || sym < 0 || sym >= dfa.M || to < 0 || to >= dfa.N) {
            cerr << "Input error: invalid transition " << sf << " " << ss << " " << st << "\n";
            return {};
        }
        dfa.delta[from][sym] = to;
    }
    string sq0;
    if (!(in >> sq0)) {
        cerr << "Input error: missing init.\n";
        return {};
    }
    dfa.q0 = parseState(sq0);
    if (dfa.q0 < 0 || dfa.q0 >= dfa.N) {
        cerr << "Input error: invalid init " << sq0 << "\n";
        return {};
    }
    int Fc;
    if (!(in >> Fc)) {
        cerr << "Input error: missing Fc.\n";
        return {};
    }
    dfa.isFinal.assign(dfa.N, false);
    for (int i = 0; i < Fc; i++) {
        string sf;
        if (!(in >> sf)) {
            cerr << "Input error: bad final.\n";
            return {};
        }
        int f = parseState(sf);
        if (f < 0 || f >= dfa.N) {
            cerr << "Input error: invalid final " << sf << "\n";
            return {};
        }
        dfa.isFinal[f] = true;
    }
    int K;
    if (!(in >> K)) {
        cerr << "Input error: missing K.\n";
        return {};
    }
    string line;
    getline(in, line); // consume end of line
    for (int i = 0; i < K; i++) {
        if (!getline(in, line)) {
            cerr << "Input error: missing query.\n";
            return {};
        }
        string orig = trim(line);
        string parsed = parseInputWord(line);
        dfa.queries.emplace_back(orig, parsed);
    }
    return dfa;
}

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) {
        cerr << "Cannot open " << fp.filename().string() << "\n";
        return false;
    }

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
                               t[0] == "finals" || t[0] == "strings" || t[0] == "queries")) {
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
    DFA dfa = readDFA(input, fp.filename().string());
    if (dfa.N == 0) return false;

    cout << "DFA: " << dfa.N << " states, alphabet = {";
    for (int i = 0; i < dfa.M; i++) {
        if (i) cout << ",";
        cout << dfa.alpha[i];
    }
    cout << "}\n";
    cout << "Initial: q" << dfa.q0 << "  Final: {";
    bool first = true;
    for (int i = 0; i < dfa.N; i++)
        if (dfa.isFinal[i]) {
            if (!first) cout << ",";
            cout << "q" << i;
            first = false;
        }
    cout << "}\n\n";

    for (const auto& q : dfa.queries) {
        const string& orig = q.first;
        const string& w = q.second;
        cout << "Input: " << (orig.empty() ? "\"\"" : orig) << "\n";
        cout << "  Steps: q" << dfa.q0;
        int cur = dfa.q0;
        bool dead = false;
        for (char c : w) {
            int idx = -1;
            for (int i = 0; i < dfa.M; i++)
                if (dfa.alpha[i] == c) {
                    idx = i;
                    break;
                }
            if (idx == -1) {
                cout << " --" << c << "--> [INVALID SYMBOL]";
                dead = true;
                break;
            }
            int nxt = dfa.delta[cur][idx];
            cout << " --" << c << "--> ";
            if (nxt == -1) {
                cout << "DEAD";
                dead = true;
                break;
            }
            cout << "q" << nxt;
            cur = nxt;
        }
        cout << "\n  Result: ";
        if (dead)
            cout << "REJECTED (dead state)\n";
        else if (dfa.isFinal[cur])
            cout << "ACCEPTED\n";
        else
            cout << "REJECTED (non-final state q" << cur << ")\n";
        cout << "\n";
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
    if (files.empty()) {
        cerr << "Error: no .txt files found in input/.\n";
        return 1;
    }
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
