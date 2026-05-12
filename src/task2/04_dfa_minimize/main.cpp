// DFA minimization.
// Algorithm: mark() + reduce() from Chapter 2.
// Input pipeline: read each .txt file in input/ with labeled sections
// `states`, `alphabet`, `transitions`, `initial`, and `finals`.
// Transition rows use `from symbol to`, for example `0 a 1`.
// DFA files do not allow lambda transitions.
// Output: equivalence classes and minimized DFA.

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
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

struct DFA { int N,M; vector<char> alpha; vector<vector<int>> delta; int q0; vector<bool> isFinal; };

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

DFA readDFA(istream& fin, const string& fname) {
    DFA dfa;
    if (!(fin>>dfa.N>>dfa.M)) { cerr<<"Input error: "<<fname<<" missing header.\n"; return {}; }
    dfa.alpha.resize(dfa.M);
    for (int i=0;i<dfa.M;i++) { if (!(fin>>dfa.alpha[i])) { cerr<<"Input error: missing alpha.\n"; return {}; } }
    dfa.delta.assign(dfa.N, vector<int>(dfa.M,-1));
    int E; if (!(fin>>E)) { cerr<<"Input error: missing E.\n"; return {}; }
    for (int i=0;i<E;i++) {
        string sf,ss,st; if (!(fin>>sf>>ss>>st)) { cerr<<"Input error: bad trans.\n"; return {}; }
        int from=parseState(sf),sym=parseSym(ss,dfa.alpha,dfa.M),to=parseState(st);
        if (from<0||from>=dfa.N||sym<0||sym>=dfa.M||to<0||to>=dfa.N) {
            cerr<<"Input error: invalid transition "<<sf<<" "<<ss<<" "<<st<<"\n";
            return {};
        }
        dfa.delta[from][sym]=to;
    }
    string sq0; if (!(fin>>sq0)) { cerr<<"Input error: missing init.\n"; return {}; }
    dfa.q0=parseState(sq0);
    if (dfa.q0<0||dfa.q0>=dfa.N) { cerr<<"Input error: invalid init "<<sq0<<"\n"; return {}; }
    int Fc; if (!(fin>>Fc)) { cerr<<"Input error: missing Fc.\n"; return {}; }
    dfa.isFinal.assign(dfa.N,false);
    for (int i=0;i<Fc;i++) {
        string sf; if (!(fin>>sf)) { cerr<<"Input error: bad final.\n"; return {}; }
        int f=parseState(sf); if (f<0||f>=dfa.N) { cerr<<"Input error: invalid final "<<sf<<"\n"; return {}; }
        dfa.isFinal[f]=true;
    }
    return dfa;
}
bool processFile(const fs::path& fp) {
    ifstream fin(fp); if (!fin) { cerr<<"Cannot open "<<fp.filename().string()<<"\n"; return false; }
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
    DFA dfa=readDFA(input,fp.filename().string()); if (dfa.N==0) return false;
    int N=dfa.N;
    cout<<"Input DFA: "<<N<<" states, alphabet = {";
    for (int i=0;i<dfa.M;i++) { if (i) cout<<","; cout<<dfa.alpha[i]; }
    cout<<"}, initial=q"<<dfa.q0<<"\n";
    vector<bool> accessible(N,false);
    { queue<int> bfs; bfs.push(dfa.q0); accessible[dfa.q0]=true;
      while (!bfs.empty()) { int cur=bfs.front(); bfs.pop();
        for (int a=0;a<dfa.M;a++) { int nxt=dfa.delta[cur][a]; if (nxt!=-1&&!accessible[nxt]) { accessible[nxt]=true; bfs.push(nxt); } } } }
    cout<<"Accessible: {"; bool f=true; for (int i=0;i<N;i++) if (accessible[i]) { if (!f) cout<<","; cout<<"q"<<i; f=false; } cout<<"}\n";
    cout<<"Inaccessible: {"; f=true; for (int i=0;i<N;i++) if (!accessible[i]) { if (!f) cout<<","; cout<<"q"<<i; f=false; } cout<<"}\n";
    int total=N+1; vector<vector<bool>> mark(total,vector<bool>(total,false));
    auto isFinalOf=[&](int s)->bool{ return (s<N)?dfa.isFinal[s]:false; };
    cout<<"mark() procedure:\nS2: Initial marking:\n";
    for (int i=0;i<total;i++) { if (i<N&&!accessible[i]) continue;
        for (int j=i+1;j<total;j++) { if (j<N&&!accessible[j]) continue;
            if (isFinalOf(i)!=isFinalOf(j)) { mark[i][j]=true;
                string li=(i<N)?"q"+to_string(i):"dead"; string lj=(j<N)?"q"+to_string(j):"dead";
                cout<<"    mark("<<li<<", "<<lj<<")\n"; } } }
    int pass=1; bool changed=true;
    while (changed) { changed=false; cout<<"  Pass "<<pass++<<":\n";
        for (int i=0;i<total;i++) { if (i<N&&!accessible[i]) continue;
            for (int j=i+1;j<total;j++) { if (j<N&&!accessible[j]) continue; if (mark[i][j]) continue;
                for (int a=0;a<dfa.M;a++) { int di=(i<N)?dfa.delta[i][a]:-1; int dj=(j<N)?dfa.delta[j][a]:-1;
                    if (di==-1) di=N;
                    if (dj==-1) dj=N;
                    if (di==dj) continue;
                    int lo=min(di,dj),hi=max(di,dj); if (mark[lo][hi]) { mark[i][j]=true; changed=true;
                        string li=(i<N)?"q"+to_string(i):"dead"; string lj=(j<N)?"q"+to_string(j):"dead";
                        cout<<"    mark("<<li<<", "<<lj<<") via "<<dfa.alpha[a]<<"\n"; break; } } } }
        if (!changed) cout<<"    (stable)\n"; }
    cout<<"\nIndistinguishable: "; f=true;
    for (int i=0;i<N;i++) { if (!accessible[i]) continue; for (int j=i+1;j<N;j++) { if (!accessible[j]) continue;
        if (!mark[i][j]) { if (!f) cout<<", "; cout<<"(q"<<i<<",q"<<j<<")"; f=false; } } }
    if (f) cout<<"(none)";
    cout<<"\n";
    vector<int> classOf(N,-1); int numClasses=0; vector<vector<int>> classes;
    for (int i=0;i<N;i++) { if (!accessible[i]||classOf[i]!=-1) continue;
        classes.push_back({i}); classOf[i]=numClasses;
        for (int j=i+1;j<N;j++) { if (!accessible[j]||classOf[j]!=-1) continue;
            if (!mark[i][j]) { classOf[j]=numClasses; classes.back().push_back(j); } }
        numClasses++; }
    cout<<"=== Minimized DFA ===\nStates: "<<numClasses<<"\n";
    int initClass=classOf[dfa.q0];
    cout<<"Transition table:\nClass\t\tStates\t\t"; for (int a=0;a<dfa.M;a++) cout<<"on "<<dfa.alpha[a]<<"\t\t"; cout<<"\n"<<string(70,'-')<<"\n";
    for (int c=0;c<numClasses;c++) {
        cout<<"C"<<c; if (c==initClass) cout<<"(init)";
        bool anyFinal=false; for (int s:classes[c]) if (dfa.isFinal[s]) { anyFinal=true; break; }
        if (anyFinal) cout<<"(final)";
        cout<<"\t\t{"; for (int k=0;k<(int)classes[c].size();k++) { if (k) cout<<","; cout<<"q"<<classes[c][k]; } cout<<"}\t\t";
        int rep=classes[c][0]; for (int a=0;a<dfa.M;a++) { int nxt=dfa.delta[rep][a]; if (nxt==-1||!accessible[nxt]) cout<<"dead\t\t"; else cout<<"C"<<classOf[nxt]<<"\t\t"; } cout<<"\n"; }
    cout<<"\nInitial state: C"<<initClass<<"\n";
    cout<<"Final states: "; for (int c=0;c<numClasses;c++) { bool af=false; for (int s:classes[c]) if (dfa.isFinal[s]) { af=true; break; } if (af) cout<<"C"<<c<<" "; } cout<<"\n";
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
