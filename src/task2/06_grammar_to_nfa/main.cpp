// Right-linear grammar to NFA.
// Algorithm: G_R to nfa() from Chapter 3.
// Input pipeline: read grammar data from each .txt file in input/.
// Bundled samples use labeled sections: `variables`, `terminals`, `start`,
// and `productions`.
// Output: NFA states, transitions, and state labels.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <algorithm>
#include <sstream>
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

struct Edge { int from, to; char sym; };

vector<string> splitTokens(const string& line) {
    istringstream iss(line);
    vector<string> tokens;
    string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}

string buildLegacyGrammarInput(const string& text, const string& fname, string& error) {
    istringstream in(text);
    string line;
    if (!getline(in, line)) { error = fname + " is empty"; return ""; }
    vector<string> tokens = splitTokens(line);
    if (tokens.size() < 2 || tokens[0] != "variables") { error = "first line must be `variables ...`"; return ""; }
    vector<string> variables(tokens.begin() + 1, tokens.end());

    if (!getline(in, line)) { error = "missing terminals line"; return ""; }
    tokens = splitTokens(line);
    if (tokens.size() < 2 || tokens[0] != "terminals") { error = "expected `terminals ...`"; return ""; }
    vector<string> terminals(tokens.begin() + 1, tokens.end());

    if (!getline(in, line)) { error = "missing start line"; return ""; }
    tokens = splitTokens(line);
    if (tokens.size() != 2 || tokens[0] != "start") { error = "expected `start <variable>`"; return ""; }
    if (tokens[1] != variables.front()) { error = "start variable must match the first variable"; return ""; }

    if (!getline(in, line)) { error = "missing productions line"; return ""; }
    tokens = splitTokens(line);
    if (tokens.size() != 2 || tokens[0] != "productions") { error = "expected `productions <count>`"; return ""; }
    int P = stoi(tokens[1]);

    vector<string> productions;
    for (int i = 0; i < P; i++) {
        if (!getline(in, line)) { error = "missing production line"; return ""; }
        productions.push_back(line);
    }

    ostringstream out;
    out << variables.size() << "\n";
    for (size_t i = 0; i < variables.size(); i++) {
        if (i) out << " ";
        out << variables[i];
    }
    out << "\n" << terminals.size() << "\n";
    for (size_t i = 0; i < terminals.size(); i++) {
        if (i) out << " ";
        out << terminals[i];
    }
    out << "\n" << P << "\n";
    for (const string& row : productions) out << row << "\n";
    return out.str();
}

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) { cerr<<"Cannot open "<<fp.filename().string()<<"\n"; return false; }
    string text((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    istringstream probe(text);
    string firstLine;
    if (!getline(probe, firstLine)) { cerr<<"Input error: "<<fp.filename().string()<<" is empty.\n"; return false; }
    string canonical = text;
    vector<string> firstTokens = splitTokens(firstLine);
    if (!firstTokens.empty() && firstTokens[0] == "variables") {
        string error;
        canonical = buildLegacyGrammarInput(text, fp.filename().string(), error);
        if (canonical.empty()) { cerr<<"Input error: "<<error<<".\n"; return false; }
    }
    istringstream input(canonical);

    int V;
    if (!(input>>V)) { cerr<<"Input error: "<<fp.filename().string()<<" missing V_count.\n"; return false; }
    vector<string> varNames(V);
    map<string,int> varIdx;
    for (int i=0;i<V;i++) {
        if (!(input>>varNames[i])) { cerr<<"Input error: missing variable names.\n"; return false; }
        varIdx[varNames[i]] = i;
    }

    int T;
    if (!(input>>T)) { cerr<<"Input error: missing T_count.\n"; return false; }
    vector<char> term(T);
    map<char,int> termIdx;
    for (int i=0;i<T;i++) {
        if (!(input>>term[i])) { cerr<<"Input error: missing terminal symbols.\n"; return false; }
        termIdx[term[i]] = i;
    }

    int P;
    if (!(input>>P)) { cerr<<"Input error: missing P_count.\n"; return false; }

    int Vf = V; // final state index
    int nextTemp = V + 1;
    int totalStates = V + 1;
    vector<Edge> edges;

    cout<<"Right-Linear Grammar to NFA\n";
    cout<<"Variables: {"; for (int i=0;i<V;i++) { if (i) cout<<","; cout<<varNames[i]; } cout<<"}\n";
    cout<<"Terminals: {"; for (int i=0;i<T;i++) { if (i) cout<<","; cout<<term[i]; } cout<<"}\n";
    cout<<"Start: "<<varNames[0]<<" = state 0\n";
    cout<<"Final state: Vf = state "<<Vf<<"\n";
    cout<<"Productions and NFA transitions:\n";

    string line;
    getline(input, line); // consume newline after P

    for (int p=0;p<P;p++) {
        if (!getline(input, line)) {
            cerr<<"Input error: missing production "<<(p+1)<<".\n"; return false;
        }
        istringstream iss(line);
        vector<string> tokens;
        string tok;
        while (iss >> tok) tokens.push_back(tok);
        if (tokens.size() < 2) {
            cerr<<"Input error: production "<<(p+1)<<" too short.\n"; return false;
        }

        string lhsName = tokens[0];
        string rhsToken = tokens.back();
        int lhs = -1;
        if (varIdx.count(lhsName)) lhs = varIdx[lhsName];
        else { cerr<<"Input error: unknown variable "<<lhsName<<"\n"; return false; }

        int rhs_var = -1;
        if (rhsToken != "-") {
            if (varIdx.count(rhsToken)) rhs_var = varIdx[rhsToken];
            else { cerr<<"Input error: unknown RHS token "<<rhsToken<<"\n"; return false; }
        }

        vector<char> rhs_terms;
        for (int k=1; k<(int)tokens.size()-1; k++) {
            if (tokens[k].size()==1) rhs_terms.push_back(tokens[k][0]);
            else { cerr<<"Input error: expected single char terminal, got: "<<tokens[k]<<"\n"; return false; }
        }

        cout<<"  "<<lhsName<<" -> ";
        for (char c : rhs_terms) cout<<c;
        if (rhs_var>=0) cout<<varNames[rhs_var];
        else if (rhs_terms.empty()) cout<<"lambda";
        cout<<"  =>  transitions: ";

        if (rhs_terms.empty()) {
            int dest = (rhs_var>=0) ? rhs_var : Vf;
            edges.push_back({lhs, dest, '\0'});
            cout<<"q"<<lhs<<"--lambda-->q"<<dest;
        } else {
            int cur = lhs;
            for (int k=0;k<(int)rhs_terms.size();k++) {
                bool isLast = (k==(int)rhs_terms.size()-1);
                int dest;
                if (isLast) dest = (rhs_var>=0) ? rhs_var : Vf;
                else { dest = nextTemp++; if (nextTemp>totalStates) totalStates=nextTemp; }
                char sym = rhs_terms[k];
                edges.push_back({cur, dest, sym});
                cout<<"q"<<cur<<"--"<<sym<<"-->q"<<dest;
                if (k<(int)rhs_terms.size()-1) cout<<"  ";
                cur = dest;
            }
        }
        cout<<"\n";
    }

    totalStates = max(totalStates, nextTemp);
    cout<<"\n=== Resulting NFA ===\n";
    cout<<"Total states: "<<totalStates<<" (0.."<<totalStates-1<<")\n";
    cout<<"Initial state: q0 ("<<varNames[0]<<")\n";
    cout<<"Final state: q"<<Vf<<" (Vf)\n";
    cout<<"All transitions:\n";
    for (auto& e : edges) {
        cout<<"  q"<<e.from<<" --"; if (e.sym=='\0') cout<<"lambda"; else cout<<e.sym; cout<<"--> q"<<e.to<<"\n";
    }
    cout<<"\nState labels:\n";
    for (int i=0;i<V;i++) cout<<"  q"<<i<<" = "<<varNames[i]<<"\n";
    cout<<"  q"<<Vf<<" = Vf (final)\n";
    for (int i=V+1;i<totalStates;i++) cout<<"  q"<<i<<" = intermediate\n";
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
