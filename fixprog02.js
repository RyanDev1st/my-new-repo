const fs = require('fs');
const path = require('path');
const base = 'C:/Users/admin/automata/src/task2';
const fname = path.join(base, '02_nfa_extended', 'main.cpp');
const content = fs.readFileSync(fname, 'utf8');

const lines = content.split('\n');
const fixed = [];
let i = 0;
const BACKSLASH = String.fromCharCode(92);

while (i < lines.length) {
    const line = lines[i];
    const next = (i+1 < lines.length) ? lines[i+1].trimStart() : null;
    
    // Count double quotes in line (rough check for unclosed string)
    let qCount = 0;
    let esc = false;
    for (let k = 0; k < line.length; k++) {
        if (esc) { esc = false; continue; }
        if (line[k] === BACKSLASH) { esc = true; continue; }
        if (line[k] === '"') qCount++;
    }
    
    // If odd number of quotes, string is broken across lines
    if (qCount % 2 === 1 && next !== null) {
        fixed.push(line + BACKSLASH + 'n' + next);
        i += 2;
    } else {
        fixed.push(line);
        i++;
    }
}

const result = fixed.join('\n');
fs.writeFileSync(fname, result);
console.log('Fixed length:', result.length);
const nl = result.split('\n');
for (let j = 84; j < 100; j++) console.log((j+1) + ': ' + nl[j]);
