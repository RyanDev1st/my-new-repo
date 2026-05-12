// NFA to right-linear grammar.
// Algorithm: nfa2G_R from Chapter 3.
// Input pipeline: read each .txt file in input/ with labeled sections
// `states`, `alphabet`, `transitions`, `initial`, and `finals`.
// Transition rows use `from symbol to`, for example `0 a 1`.
// Lambda can be written as `""`, `lambda`, or `lamda`.
// Output: grammar productions.

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cctype>
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
    for (int i = 0; i < (int)states.size(); i++) if (states[i] != i) return false;
    N = (int)states.size();
    return true;
}

int parseState(const string& t) {
    if (!isBareStateToken(t)) return -1;
    return stoi(t);
}
int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t=="\"\"" || t=="lambda" || t=="lamda") return M;
    if (t.size()==1) for (int i=0;i<M;i++) if(alpha[i]==t[0]) return i;
    return -1;
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
    bool seenAlphabet = false, seenTransitions = false, seenInitial = false, seenFinals = false;

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
        } else {
            i++;
        }
    }

    if (!seenAlphabet) { error = "missing alphabet line"; return ""; }
    if (!seenTransitions) { error = "missing transitions line"; return ""; }
    if (!seenInitial) { error = "missing initial line"; return ""; }
    if (!seenFinals) { error = "missing finals line"; return ""; }

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
    return out.str();
}

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) { cerr<<"Cannot open "<<fp.filename().string()<<"\n"; return false; }
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
        // Check if any known section label exists
        istringstream check(text);
        string l;
        while (getline(check, l)) {
            vector<string> t = splitTokens(l);
            if (!t.empty() && (t[0] == "alphabet" || t[0] == "transitions" || t[0] == "initial" || t[0] == "finals")) {
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

    int N, M;
    if (!(input>>N>>M)) { cerr<<"Input error: "<<fp.filename().string()<<" missing NFA header.\n"; return false; }

    vector<char> alpha(M);
    for (int i=0;i<M;i++) { if (!(input>>alpha[i])) { cerr<<"Input error: missing alphabet.\n"; return false; } }

    vector<vector<set<int>>> delta(N, vector<set<int>>(M+1));
    int E;
    if (!(input>>E)) { cerr<<"Input error: missing E.\n"; return false; }
    for (int i=0;i<E;i++) {
        string sf,ss,st;
        if (!(input>>sf>>ss>>st)) { cerr<<"Input error: bad transition.\n"; return false; }
        int from=parseState(sf), sym=parseSym(ss,alpha,M), to=parseState(st);
        if (from>=0&&from<N&&sym>=0&&sym<=M&&to>=0&&to<N) delta[from][sym].insert(to);
        else { cerr<<"Input error: invalid transition "<<sf<<" "<<ss<<" "<<st<<"\n"; return false; }
    }

    string sq0; if (!(input>>sq0)) { cerr<<"Input error: missing init.\n"; return false; }
    int q0=parseState(sq0);
    if (q0<0||q0>=N) { cerr<<"Input error: invalid init "<<sq0<<"\n"; return false; }
    int Fc; if (!(input>>Fc)) { cerr<<"Input error: missing Fc.\n"; return false; }
    vector<bool> isFinal(N,false);
    for (int i=0;i<Fc;i++) {
        string sf; if (!(input>>sf)) { cerr<<"Input error: bad final.\n"; return false; }
        int f=parseState(sf); if (f<0||f>=N) { cerr<<"Input error: invalid final "<<sf<<"\n"; return false; }
        isFinal[f]=true;
    }

    cout<<"NFA: "<<N<<" states, alphabet={";
    for (int i=0;i<M;i++) { if (i) cout<<","; cout<<alpha[i]; }
    cout<<"}, initial=q"<<q0<<", finals={";
    bool f=true; for (int i=0;i<N;i++) if (isFinal[i]) { if (!f) cout<<","; cout<<"q"<<i; f=false; }
    cout<<"}\n";

    bool hasLambda=false;
    for (int i=0;i<N;i++) if (!delta[i][M].empty()) { hasLambda=true; break; }
    if (hasLambda) {
        cerr<<"WARNING: NFA has lambda transitions. nfa2G_R requires lambda-free NFA.\n";
        cerr<<"Continuing with symbol transitions only; lambda transitions ignored.\n";
    }

    cout<<"=== Right-Linear Grammar G_R ===\n";
    cout<<"Variables: {"; for (int i=0;i<N;i++) { if (i) cout<<","; cout<<"q"<<i; } cout<<"}\n";
    cout<<"Terminals: {"; for (int i=0;i<M;i++) { if (i) cout<<","; cout<<alpha[i]; } cout<<"}\n";
    cout<<"Start symbol: q"<<q0<<"\n";
    cout<<"Productions:\n";

    for (int qi=0;qi<N;qi++) {
        for (int aj=0;aj<M;aj++) {
            for (int qk : delta[qi][aj]) {
                cout<<"  q"<<qi<<" -> "<<alpha[aj]<<" q"<<qk<<"\n";
            }
        }
        for (int qk : delta[qi][M]) {
            cout<<"  q"<<qi<<" -> [lambda] q"<<qk<<"  (NOTE: lambda-transition; eliminate before using grammar)\n";
        }
    }
    for (int qf=0;qf<N;qf++) if (isFinal[qf]) cout<<"  q"<<qf<<" -> lambda\n";

    cout<<"\nSummary of grammar productions:\n"<<string(40,'-')<<"\n";
    for (int qi=0;qi<N;qi++) {
        bool hasProd=false;
        for (int aj=0;aj<M;aj++) if (!delta[qi][aj].empty()) { hasProd=true; break; }
        if (!hasProd&&!isFinal[qi]) continue;
        cout<<"q"<<qi<<" -> ";
        bool first2=true;
        for (int aj=0;aj<M;aj++) {
            for (int qk : delta[qi][aj]) {
                if (!first2) cout<<" | ";
                cout<<alpha[aj]<<"q"<<qk;
                first2=false;
            }
        }
        if (isFinal[qi]) { if (!first2) cout<<" | "; cout<<"lambda"; }
        cout<<"\n";
    }
    return true;
}

int main() {
    fs::path inputDir="input"; if (!fs::exists(inputDir)||!fs::is_directory(inputDir)) { cerr<<"Error: input/ not found.\n"; return 1; }
    vector<fs::path> files; for (auto& e:fs::directory_iterator(inputDir)) { if (e.is_regular_file()&&e.path().extension()==".txt") files.push_back(e.path()); }
    if (files.empty()) { cerr<<"Error: no .txt files.\n"; return 1; }
    sort(files.begin(),files.end());

    fs::create_directories("output");

    for (auto& fp:files) {
        string outPath = "output/" + fp.filename().string();
        ofstream fout(outPath);
        TeeBuf tee(cout.rdbuf(), fout.rdbuf());
        streambuf* oldBuf = cout.rdbuf(&tee);

        cout<<"=== "<<fp.filename().string()<<" ===\n";
        processFile(fp);
        cout<<"\n";

        cout.rdbuf(oldBuf);
    }
    return 0;
}
