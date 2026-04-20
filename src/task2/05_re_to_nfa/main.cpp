// Regular Expression to NFA (Thompson's Construction)
// Algorithm: re2nfa() -- slides 17-20, Chapter 3
//
// Input format (input.txt):
//   regex_string
//   (use + for union, * for Kleene star, concatenation is implicit,
//    () for grouping, ~ for lambda/epsilon)
//   Empty input is invalid; use ~ for lambda.
//
// Grammar parsed:
//   expr   -> term ('+' term)*
//   term   -> factor+
//   factor -> atom ('*')*
//   atom   -> CHAR | '~' | '(' expr ')'

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <map>
using namespace std;

struct Edge {
    int from, to;
    char sym; // '\0' = lambda
};

int stateCount = 0;
vector<Edge> edges;
set<char> alphabetSet;

int newState() { return stateCount++; }

void addLambda(int from, int to) { edges.push_back({from, to, '\0'}); }
void addEdge(int from, int to, char sym) {
    alphabetSet.insert(sym);
    edges.push_back({from, to, sym});
}

struct Frag { int start, accept; };

// S1a: NFA for empty set (no transitions, no accept)
Frag makeEmpty() {
    int s = newState(), a = newState();
    return {s, a};
}

// S1b: NFA for lambda (accepts empty string)
Frag makeLambda() {
    int s = newState(), a = newState();
    addLambda(s, a);
    return {s, a};
}

// S1c: NFA for single symbol a
Frag makeSymbol(char c) {
    int s = newState(), a = newState();
    addEdge(s, a, c);
    return {s, a};
}

// S2a: Union r1 + r2
Frag makeUnion(Frag m1, Frag m2) {
    int s = newState(), a = newState();
    addLambda(s, m1.start);
    addLambda(s, m2.start);
    addLambda(m1.accept, a);
    addLambda(m2.accept, a);
    return {s, a};
}

// S2b: Concatenation r1.r2
Frag makeConcat(Frag m1, Frag m2) {
    addLambda(m1.accept, m2.start);
    return {m1.start, m2.accept};
}

// S2c: Kleene star r*
// new_start -λ-> m.start; m.accept -λ-> new_final
// new_start -λ-> new_final (accept lambda)
// m.accept -λ-> m.start   (loop for repetition)
Frag makeStar(Frag m) {
    int s = newState(), a = newState();
    addLambda(s, m.start);
    addLambda(m.accept, a);
    addLambda(s, a);
    addLambda(m.accept, m.start);
    return {s, a};
}

// Recursive descent parser
string re;
int pos;

Frag parseExpr();
Frag parseTerm();
Frag parseFactor();
Frag parseAtom();

Frag parseExpr() {
    Frag result = parseTerm();
    while (pos < (int)re.size() && re[pos] == '+') {
        pos++;
        Frag right = parseTerm();
        result = makeUnion(result, right);
    }
    return result;
}

Frag parseTerm() {
    Frag result = parseFactor();
    while (pos < (int)re.size() && re[pos] != ')' && re[pos] != '+') {
        Frag right = parseFactor();
        result = makeConcat(result, right);
    }
    return result;
}

Frag parseFactor() {
    Frag result = parseAtom();
    while (pos < (int)re.size() && re[pos] == '*') {
        pos++;
        result = makeStar(result);
    }
    return result;
}

Frag parseAtom() {
    if (pos >= (int)re.size()) { return makeLambda(); }
    if (re[pos] == '(') {
        pos++;
        Frag result = parseExpr();
        if (pos < (int)re.size() && re[pos] == ')') pos++;
        return result;
    }
    if (re[pos] == '~') { pos++; return makeLambda(); }
    char c = re[pos++];
    return makeSymbol(c);
}

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }

    if (!getline(fin, re)) {
        cerr << "Input error: input.txt is empty. Provide a regular expression.\n";
        return 1;
    }
    // strip trailing whitespace/cr
    while (!re.empty() && (re.back() == '\r' || re.back() == '\n' || re.back() == ' '))
        re.pop_back();

    if (re.empty()) {
        cerr << "Input error: regular expression is empty. Use '~' to represent lambda.\n";
        return 1;
    }

    cout << "Regular expression: " << re << "\n\n";

    pos = 0;
    Frag result = parseExpr();

    cout << "Thompson's construction result:\n";
    cout << "  Total states: " << stateCount << " (0.." << stateCount-1 << ")\n";
    cout << "  Start state:  " << result.start << "\n";
    cout << "  Accept state: " << result.accept << "\n";

    vector<char> alpha(alphabetSet.begin(), alphabetSet.end());
    sort(alpha.begin(), alpha.end());
    cout << "  Alphabet: {";
    for (int i = 0; i < (int)alpha.size(); i++) { if (i) cout << ","; cout << alpha[i]; }
    cout << "}\n\n";

    cout << "Transitions:\n";
    for (auto& e : edges) {
        cout << "  q" << e.from << " --";
        if (e.sym == '\0') cout << "lambda";
        else cout << e.sym;
        cout << "--> q" << e.to << "\n";
    }

    // Print as transition table for clarity
    cout << "\nTransition table (NFA):\n";
    // Map alphabet chars to indices
    map<char, int> symIdx;
    for (int i = 0; i < (int)alpha.size(); i++) symIdx[alpha[i]] = i;
    int M = (int)alpha.size();

    // Build adjacency per state
    vector<vector<set<int>>> delta(stateCount, vector<set<int>>(M + 1)); // M = lambda
    for (auto& e : edges) {
        if (e.sym == '\0') delta[e.from][M].insert(e.to);
        else delta[e.from][symIdx[e.sym]].insert(e.to);
    }

    cout << "State\t";
    for (char c : alpha) cout << c << "\t\t";
    cout << "lambda\n";
    cout << string(50, '-') << "\n";

    for (int i = 0; i < stateCount; i++) {
        cout << "q" << i;
        if (i == result.start) cout << "(start)";
        if (i == result.accept) cout << "(final)";
        cout << "\t";
        for (int a = 0; a <= M; a++) {
            cout << "{";
            bool f = true;
            for (int s : delta[i][a]) { if (!f) cout << ","; cout << s; f = false; }
            cout << "}\t\t";
        }
        cout << "\n";
    }

    return 0;
}
