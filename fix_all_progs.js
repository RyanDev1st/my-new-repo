const fs = require('fs');
const path = require('path');
const base = 'C:/Users/admin/automata/src/task2';
const BACKSLASH = String.fromCharCode(92);

const dirs = [
    '01_dfa_acceptance', '02_nfa_extended', '03_nfa_to_dfa',
    '04_dfa_minimize', '05_re_to_nfa', '06_grammar_to_nfa', '07_nfa_to_grammar'
];

for (const dir of dirs) {
    const fname = path.join(base, dir, 'main.cpp');
    const content = fs.readFileSync(fname, 'utf8');
    
    const lines = content.split('\n');
    const fixed = [];
    let i = 0;
    let changeCount = 0;
    
    while (i < lines.length) {
        const line = lines[i];
        const next = (i+1 < lines.length) ? lines[i+1].trimStart() : null;
        
        let qCount = 0;
        let esc = false;
        for (let k = 0; k < line.length; k++) {
            if (esc) { esc = false; continue; }
            if (line[k] === BACKSLASH) { esc = true; continue; }
            if (line[k] === '"') qCount++;
        }
        
        if (qCount % 2 === 1 && next !== null) {
            fixed.push(line + BACKSLASH + 'n' + next);
            i += 2;
            changeCount++;
        } else {
            fixed.push(line);
            i++;
        }
    }
    
    const result = fixed.join('\n');
    fs.writeFileSync(fname, result);
    console.log(dir + ': ' + changeCount + ' fixes, length=' + result.length);
}
