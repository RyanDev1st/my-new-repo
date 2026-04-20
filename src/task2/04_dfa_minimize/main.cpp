// DFA State Minimization
// Algorithm: mark() + reduce() -- slides 59-60, 64, 72, Chapter 2
//
// Input format (input/ directory, each .txt file):
//   N M / s0..sM-1 / E / qX sym qY transitions / qinit / F_count / finals

#include <iostream>
#include <fstream>
#include <vector>
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

struct DFA { int N,M; vector<char> alpha; vector<vector<int>> delta; int q0; vector<bool> isFinal; };

int parseState(const string& t) {
    if (!t.empty() && (t[0]=='q'||t[0]=='Q')) return stoi(t.substr(1));
    return stoi(t);
}
int parseSym(const string& t, const vector<char>& alpha, int M) {
    if (t=="~"||t=="lambda") return M;
    if (t.size()==1) for (int i=0;i<M;i++) if(alpha[i]==t[0]) return i;
    return -1;
}
DFA readDFA(ifstream& fin, const string& fname) {
    DFA dfa;
    if (!(fin>>dfa.N>>dfa.M)) { cerr<<"Input error: "<<fname<<" missing header.\n"; return {}; }
    dfa.alpha.resize(dfa.M);
    for (int i=0;i<dfa.M;i++) { if (!(fin>>dfa.alpha[i])) { cerr<<"Input error: missing alpha.\n"; return {}; } }
    dfa.delta.assign(dfa.N, vector<int>(dfa.M,-1));
    int E; if (!(fin>>E)) { cerr<<"Input error: missing E.\n"; return {}; }
    for (int i=0;i<E;i++) {
        string sf,ss,st; if (!(fin>>sf>>ss>>st)) { cerr<<"Input error: bad trans.\n"; return {}; }
        int from=parseState(sf),sym=parseSym(ss,dfa.alpha,dfa.M),to=parseState(st);
        if (from>=0&&from<dfa.N&&sym>=0&&sym<dfa.M&&to>=0&&to<dfa.N) dfa.delta[from][sym]=to;
    }
    string sq0; if (!(fin>>sq0)) { cerr<<"Input error: missing init.\n"; return {}; }
    dfa.q0=parseState(sq0);
    int Fc; if (!(fin>>Fc)) { cerr<<"Input error: missing Fc.\n"; return {}; }
    dfa.isFinal.assign(dfa.N,false);
    for (int i=0;i<Fc;i++) {
        string sf; if (!(fin>>sf)) { cerr<<"Input error: bad final.\n"; return {}; }
        int f=parseState(sf); if (f>=0&&f<dfa.N) dfa.isFinal[f]=true;
    }
    return dfa;
}
bool processFile(const fs::path& fp) {
    ifstream fin(fp); if (!fin) { cerr<<"Cannot open "<<fp.filename().string()<<"\n"; return false; }
    DFA dfa=readDFA(fin,fp.filename().string()); if (dfa.N==0) return false;
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
                    if (di==-1) di=N; if (dj==-1) dj=N; if (di==dj) continue;
                    int lo=min(di,dj),hi=max(di,dj); if (mark[lo][hi]) { mark[i][j]=true; changed=true;
                        string li=(i<N)?"q"+to_string(i):"dead"; string lj=(j<N)?"q"+to_string(j):"dead";
                        cout<<"    mark("<<li<<", "<<lj<<") via "<<dfa.alpha[a]<<"\n"; break; } } } }
        if (!changed) cout<<"    (stable)\n"; }
    cout<<"\nIndistinguishable: "; f=true;
    for (int i=0;i<N;i++) { if (!accessible[i]) continue; for (int j=i+1;j<N;j++) { if (!accessible[j]) continue;
        if (!mark[i][j]) { if (!f) cout<<", "; cout<<"(q"<<i<<",q"<<j<<")"; f=false; } } }
    if (f) cout<<"(none)"; cout<<"\n";
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
