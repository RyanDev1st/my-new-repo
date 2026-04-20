// Right-Linear Grammar to NFA
// Algorithm: G_R to nfa() -- slides 39-42, Chapter 3
//
// Input format (input/ directory, each .txt file):
//   V_count
//   VarName0 VarName1 ...     (first = start symbol)
//   T_count
//   t1 t2 ...                 (terminal chars)
//   P_count
//   LHSVarName term1 term2 ... [RHSVarName | -]
//     where - means no RHS variable (terminal-only or lambda production)
//     Lambda production: LHSName -  (just name and dash)

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

struct Edge { int from, to; char sym; };

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) { cerr<<"Cannot open "<<fp.filename().string()<<"\n"; return false; }

    int V;
    if (!(fin>>V)) { cerr<<"Input error: "<<fp.filename().string()<<" missing V_count.\n"; return false; }
    vector<string> varNames(V);
    map<string,int> varIdx;
    for (int i=0;i<V;i++) {
        if (!(fin>>varNames[i])) { cerr<<"Input error: missing variable names.\n"; return false; }
        varIdx[varNames[i]] = i;
    }

    int T;
    if (!(fin>>T)) { cerr<<"Input error: missing T_count.\n"; return false; }
    vector<char> term(T);
    map<char,int> termIdx;
    for (int i=0;i<T;i++) {
        if (!(fin>>term[i])) { cerr<<"Input error: missing terminal symbols.\n"; return false; }
        termIdx[term[i]] = i;
    }

    int P;
    if (!(fin>>P)) { cerr<<"Input error: missing P_count.\n"; return false; }

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
    getline(fin, line); // consume newline after P

    for (int p=0;p<P;p++) {
        if (!getline(fin, line)) {
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
