// DFA String Acceptance Check
// Algorithm: delta(q, w) -- slide 20-23, Chapter 2
//
// Input format (input/ directory, each .txt file):
//   N M                  -- N states (0..N-1), M alphabet symbols
//   s0 s1 ... sM-1       -- alphabet characters
//   E                    -- number of transitions
//   qX sym qY            -- each transition (qX,qY = state names; sym = actual char)
//   qinit                -- initial state name
//   F_count
//   qf1 qf2 ...          -- final state names
//   K                    -- number of test strings
//   w0 w1 ...            -- test strings (one per line, blank line = empty string)

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
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
    if (!fin) { cerr << "Cannot open " << fp.filename().string() << "\n"; return false; }

    int N, M;
    if (!(fin >> N >> M)) {
        cerr << "Input error: " << fp.filename().string() << " is empty or missing the DFA header.\n";
        return false;
    }

    vector<char> alpha(M);
    for (int i = 0; i < M; i++) {
        if (!(fin >> alpha[i])) {
            cerr << "Input error: missing alphabet symbols.\n";
            return false;
        }
    }

    vector<vector<int>> delta(N, vector<int>(M, -1));

    int E;
    if (!(fin >> E)) {
        cerr << "Input error: missing number of DFA transitions.\n";
        return false;
    }
    for (int i = 0; i < E; i++) {
        string sf, ss, st;
        if (!(fin >> sf >> ss >> st)) {
            cerr << "Input error: incomplete DFA transition list.\n";
            return false;
        }
        int from = parseState(sf);
        int sym = parseSym(ss, alpha, M);
        int to = parseState(st);
        if (from < 0 || from >= N || sym < 0 || sym >= M || to < 0 || to >= N) {
            cerr << "Input error: invalid transition " << sf << " " << ss << " " << st << "\n";
            return false;
        }
        delta[from][sym] = to;
    }

    string sq0;
    if (!(fin >> sq0)) {
        cerr << "Input error: missing initial state.\n";
        return false;
    }
    int q0 = parseState(sq0);

    int Fc;
    if (!(fin >> Fc)) {
        cerr << "Input error: missing number of final states.\n";
        return false;
    }
    vector<bool> isFinal(N, false);
    for (int i = 0; i < Fc; i++) {
        string sf;
        if (!(fin >> sf)) {
            cerr << "Input error: incomplete final-state list.\n";
            return false;
        }
        int f = parseState(sf);
        if (f < 0 || f >= N) { cerr << "Input error: invalid final state " << sf << "\n"; return false; }
        isFinal[f] = true;
    }

    cout << "DFA: " << N << " states, alphabet = {";
    for (int i = 0; i < M; i++) { if (i) cout << ","; cout << alpha[i]; }
    cout << "}\n";
    cout << "Initial: q" << q0 << "  Final: {";
    bool first = true;
    for (int i = 0; i < N; i++) if (isFinal[i]) {
        if (!first) cout << ","; cout << "q" << i; first = false;
    }
    cout << "}\n\n";

    int K;
    if (!(fin >> K)) {
        cerr << "Input error: missing number of test strings.\n";
        return false;
    }

    string line;
    getline(fin, line); // consume newline after K

    for (int caseNo = 0; caseNo < K; caseNo++) {
        if (!getline(fin, line)) {
            cerr << "Input error: missing test string on line " << (caseNo + 1) << "\n";
            return false;
        }
        string w = parseInputWord(line);
        string shown = w.empty() ? "\"\"" : w;

        cout << "Input: " << shown << "\n";
        cout << "  Steps: q" << q0;

        int cur = q0;
        bool dead = false;

        for (char c : w) {
            int idx = -1;
            for (int i = 0; i < M; i++)
                if (alpha[i] == c) { idx = i; break; }

            if (idx == -1) {
                cout << " --" << c << "--> [INVALID SYMBOL]";
                dead = true; break;
            }

            int nxt = delta[cur][idx];
            cout << " --" << c << "--> ";
            if (nxt == -1) {
                cout << "DEAD";
                dead = true; break;
            }
            cout << "q" << nxt;
            cur = nxt;
        }

        cout << "\n  Result: ";
        if (dead)              cout << "REJECTED (dead state)\n";
        else if (isFinal[cur]) cout << "ACCEPTED\n";
        else                   cout << "REJECTED (non-final state q" << cur << ")\n";
        cout << "\n";
    }
    return true;
}

int main() {
    fs::path inputDir = "input";
    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        cerr << "Error: input/ directory not found.\n";
        return 1;
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
