// NFA to Right-Linear Grammar
// Algorithm: nfa2G_R -- slides 44-45, Chapter 3
//
// Input format (input/ directory, each .txt file):
//   N M
//   s0 ... sM-1
//   E
//   qX sym qY   (sym=~ means lambda)
//   qinit
//   F_count
//   qf1 qf2 ...

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
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

int parseState(const string& t) {
    if (!t.empty() && (t[0]=='q'||t[0]=='Q')) return stoi(t.substr(1));
    return stoi(t);
}
int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t=="~"||t=="lambda") return M;
    if (t.size()==1) for (int i=0;i<M;i++) if(alpha[i]==t[0]) return i;
    return -1;
}

bool processFile(const fs::path& fp) {
    ifstream fin(fp);
    if (!fin) { cerr<<"Cannot open "<<fp.filename().string()<<"\n"; return false; }

    int N, M;
    if (!(fin>>N>>M)) { cerr<<"Input error: "<<fp.filename().string()<<" missing NFA header.\n"; return false; }

    vector<char> alpha(M);
    for (int i=0;i<M;i++) { if (!(fin>>alpha[i])) { cerr<<"Input error: missing alphabet.\n"; return false; } }

    vector<vector<set<int>>> delta(N, vector<set<int>>(M+1));
    int E;
    if (!(fin>>E)) { cerr<<"Input error: missing E.\n"; return false; }
    for (int i=0;i<E;i++) {
        string sf,ss,st;
        if (!(fin>>sf>>ss>>st)) { cerr<<"Input error: bad transition.\n"; return false; }
        int from=parseState(sf), sym=parseSym(ss,alpha,M), to=parseState(st);
        if (from>=0&&from<N&&sym>=0&&sym<=M&&to>=0&&to<N) delta[from][sym].insert(to);
        else { cerr<<"Input error: invalid transition "<<sf<<" "<<ss<<" "<<st<<"\n"; return false; }
    }

    string sq0; if (!(fin>>sq0)) { cerr<<"Input error: missing init.\n"; return false; }
    int q0=parseState(sq0);
    int Fc; if (!(fin>>Fc)) { cerr<<"Input error: missing Fc.\n"; return false; }
    vector<bool> isFinal(N,false);
    for (int i=0;i<Fc;i++) {
        string sf; if (!(fin>>sf)) { cerr<<"Input error: bad final.\n"; return false; }
        int f=parseState(sf); if (f>=0&&f<N) isFinal[f]=true;
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
                if (!first2) cout<<" | "; cout<<alpha[aj]<<"q"<<qk; first2=false;
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
