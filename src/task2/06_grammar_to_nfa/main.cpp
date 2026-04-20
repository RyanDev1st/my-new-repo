// Right-Linear Grammar to NFA
// Algorithm: G_R to nfa() -- slides 39-42, Chapter 3
//
// Input format (input.txt):
//   V_count                -- number of variables
//   V0 V1 V2 ...           -- variable names (first = start symbol)
//   T_count                -- number of terminals
//   a b c ...              -- terminal symbols
//   P_count                -- number of productions
//   Each production line:  LHS_idx t1 t2 ... [RHS_var_idx | -1]
//     where t1..tm are terminal indices (0..T-1),
//     last value: variable index (0..V-1) OR -1 (terminal-only production)
//
// Example: V0->aV1 encoded as: 0 0 1  (var0, terminal_a, var1)
//          V0->ba  encoded as: 0 1 0 -1 (var0, terminal_b, terminal_a, -1=no var)

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

struct Edge {
    int from, to;
    char sym;
};

int main() {
    ifstream fin("input.txt");
    if (!fin) { cerr << "Cannot open input.txt\n"; return 1; }

    int V;
    if (!(fin >> V)) {
        cerr << "Input error: input.txt is empty or missing the grammar header.\n";
        return 1;
    }
    vector<string> varNames(V);
    for (int i = 0; i < V; i++) {
        if (!(fin >> varNames[i])) {
            cerr << "Input error: missing variable names.\n";
            return 1;
        }
    }

    int T;
    if (!(fin >> T)) {
        cerr << "Input error: missing number of terminals.\n";
        return 1;
    }
    vector<char> term(T);
    for (int i = 0; i < T; i++) {
        if (!(fin >> term[i])) {
            cerr << "Input error: missing terminal symbols.\n";
            return 1;
        }
    }

    int P;
    if (!(fin >> P)) {
        cerr << "Input error: missing number of productions.\n";
        return 1;
    }

    // S1: Each variable Vi -> state i; add one shared final state Vf (index V)
    // S2: Initial state = S = state 0 (first variable)
    int Vf = V; // final state index
    int totalStates = V + 1; // variables + final state
    int nextTemp = V + 1;    // next available state for intermediate nodes

    // We'll collect all edges and count states dynamically
    vector<Edge> edges;

    cout << "Right-Linear Grammar to NFA\n";
    cout << "Variables: {";
    for (int i = 0; i < V; i++) { if (i) cout << ","; cout << varNames[i]; }
    cout << "}\n";
    cout << "Terminals: {";
    for (int i = 0; i < T; i++) { if (i) cout << ","; cout << term[i]; }
    cout << "}\n";
    cout << "Start: " << varNames[0] << " = state 0\n";
    cout << "Final state: Vf = state " << Vf << "\n\n";

    cout << "Productions and NFA transitions:\n";

    for (int p = 0; p < P; p++) {
        int lhs;
        if (!(fin >> lhs)) {
            cerr << "Input error: missing left-hand side for production " << (p + 1) << ".\n";
            return 1;
        }

        // Read terminals of the RHS
        vector<int> terminals;
        int lastToken;
        // Read until end of line
        // We read values one by one; last value is either -1 (no var) or var_idx
        // We peek to see if next token is -1 or a var_idx
        // Strategy: read all ints on the line
        string line;
        getline(fin, line); // read the rest of the line after lhs
        // parse line for remaining ints
        vector<int> tokens;
        {
            size_t i = 0;
            while (i < line.size()) {
                while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) i++;
                if (i >= line.size()) break;
                bool neg = false;
                if (line[i] == '-') { neg = true; i++; }
                int num = 0; bool hasNum = false;
                while (i < line.size() && line[i] >= '0' && line[i] <= '9') {
                    num = num * 10 + (line[i] - '0'); i++; hasNum = true;
                }
                if (hasNum) tokens.push_back(neg ? -num : num);
            }
        }

        if (tokens.empty()) {
            cerr << "Input error: production " << (p + 1)
                 << " is missing its right-hand side.\n";
            return 1;
        }

        // Last token: -1 = terminal-only, else = variable index
        int rhs_var = tokens.back();
        // All tokens except last are terminal indices
        vector<int> rhs_terms(tokens.begin(), tokens.end() - 1);

        cout << "  " << varNames[lhs] << " -> ";
        for (int t : rhs_terms) cout << term[t];
        if (rhs_var >= 0) cout << varNames[rhs_var];
        else if (rhs_terms.empty()) cout << "lambda";
        cout << "  =>  transitions: ";

        if (rhs_terms.empty()) {
            // Unit production Vi -> Vj: lambda edge lhs -> rhs_var
            // Lambda production Vi -> lambda: lambda edge lhs -> Vf
            int dest = (rhs_var >= 0) ? rhs_var : Vf;
            Edge e; e.from = lhs; e.to = dest; e.sym = '\0';
            edges.push_back(e);
            cout << "q" << lhs << "--lambda-->q" << dest;
        } else {
            // Build chain of states for terminal sequence
            int cur = lhs;
            for (int k = 0; k < (int)rhs_terms.size(); k++) {
                int dest;
                bool isLastTerm = (k == (int)rhs_terms.size() - 1);

                if (isLastTerm) {
                    dest = (rhs_var >= 0) ? rhs_var : Vf;
                } else {
                    dest = nextTemp++;
                    if (nextTemp > totalStates) totalStates = nextTemp;
                }

                char sym = term[rhs_terms[k]];
                edges.push_back({cur, dest, sym});
                cout << "q" << cur << "--" << sym << "-->q" << dest;
                if (k < (int)rhs_terms.size() - 1) cout << "  ";
                cur = dest;
            }
        }
        cout << "\n";
    }

    totalStates = max(totalStates, nextTemp);

    // Print NFA
    cout << "\n=== Resulting NFA ===\n";
    cout << "Total states: " << totalStates << " (0.." << totalStates-1 << ")\n";
    cout << "Initial state: q0 (" << varNames[0] << ")\n";
    cout << "Final state: q" << Vf << " (Vf)\n\n";

    cout << "All transitions:\n";
    for (auto& e : edges) {
        cout << "  q" << e.from << " --";
        if (e.sym == '\0') cout << "lambda"; else cout << e.sym;
        cout << "--> q" << e.to << "\n";
    }

    // Print state labels
    cout << "\nState labels:\n";
    for (int i = 0; i < V; i++)
        cout << "  q" << i << " = " << varNames[i] << "\n";
    cout << "  q" << Vf << " = Vf (final)\n";
    for (int i = V + 1; i < totalStates; i++)
        cout << "  q" << i << " = intermediate\n";

    return 0;
}
