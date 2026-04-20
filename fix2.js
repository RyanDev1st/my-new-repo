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
        const next = (i+1 < lines.length) ? lines[i+1] : null;
        
        // Count unescaped double quotes in line
        let qCount = 0;
        let esc = false;
        for (let k = 0; k < line.length; k++) {
            if (esc) { esc = false; continue; }
            if (line[k] === BACKSLASH) { esc = true; continue; }
            if (line[k] === '"') qCount++;
        }
        
        // If odd quotes AND next line starts with " (continuation of broken string)
        if (qCount % 2 === 1 && next !== null) {
            const nextTrimmed = next.trimStart();
            if (nextTrimmed.startsWith('"')) {
                // The next line's " closes the current string - just concatenate, no \n needed
                // But we need to check: did the broken string end with \n escape, or just abruptly?
                // If line ends with \n escape (backslash + n = last 2 chars), then joining would
                // give: cout << "}\n" + ";\n  -- but we need: cout << "}\n"; not cout << "}\n"";\n"
                // Wait: line = 'cout << "}\n'  (ends with \n escape, missing closing ")
                //        nextTrimmed = '";\n' (starts with ")
                //        Joined: 'cout << "}\n' + '";\n' = 'cout << "}\n";\n'  <- CORRECT if we treat
                //        the " in nextTrimmed as the CLOSING quote
                // BUT this gives: cout << "}\n";\n  where ;\n is outside - that's:
                //   the \n at end of next is trailing \n outside string - that's fine C++!
                // Wait: cout << "}\n"; is the statement, then \n is from our fix_all insertion... 
                // No wait - I'm confusing. Let me just check what was actually in the files
                // BEFORE fix_all ran. The issue: fix_all added \n between line and next.
                // The \n between them created: cout << "}\n" then literal \n then ";\n
                // which in C++ is: string "}\n" concatenated with string ";\n" = "}\n;\n"
                // but followed by a stray \n.
                
                // CORRECT fix: just join line + nextTrimmed (no \n insertion)
                // This gives: cout << "}\n"  +  ";\n  = cout << "}\n";\n (well, with the \n from fix_all at end)
                // Hmm. The \n at end of nextTrimmed comes from the original template literal.
                // nextTrimmed = '";\n' meaning the content after trimStart is: "; then \n escape
                // So joining: line + nextTrimmed = cout << "}\n" + ";\n = cout << "}\n";\n
                // which is: cout << "}\n"  (string) then ; (semicolon) then \n (escape)
                // That's cout << "}\n"; with trailing \n escape outside the string!
                // The trailing \n outside is: just a \n in the source code? That's: a backslash followed by n
                // which is not valid C++ outside a string. 
                
                // Actually: let's check what ;\n means. In C++:
                // cout << "}\n";  <- valid
                // cout << "}\n";\  <- no, backslash continuation?
                // Wait the \n at end of nextTrimmed - is it the C++ escape or is it a real newline?
                // Let me check: nextTrimmed ends with BACKSLASH + 'n' = two bytes 5c 6e
                // In C++ source, outside a string, 5c6e is \n which is a line continuation!
                // In C++, backslash at end of line means the next line is a continuation.
                // But that's only when it's the LAST character before the newline.
                // Hmm, but after fix_all, we joined lines so there's no actual newline there.
                // Wait - after joining: 'cout << "}\n"' + '\n' + '";\n'
                // The \n between them is the BACKSLASH + 'n' we inserted = chars 5c 6e
                // So in the file: cout << "}\n"\n";\n  (where \n is chars 5c 6e each time)
                // = cout << "}\n" then \ then n then " then ; then \ then n
                // = two string literals "}\n" and ";\n" with \n and ";\n outside = WRONG

                // The CORRECT fix is simpler: the source file after template literal had:
                // cout << "}        (actual newline 0x0a follows)
                // ";               (starts with ")
                // But fix_all ran AFTER something already processed the file
                // Actually let me just look at what we have NOW and do a targeted fix
                
                // For the case where line ends with \n escape (not a real newline) and missing closing ":
                // line = 'cout << "}\n'  -> should become: 'cout << "}\n";'
                // next line = '";\n' -> should become empty (consumed)
                // But next line also has content after "; so we need to be careful

                // Simplest: if line ends with BACKSLASH+'n' and is missing closing quote:
                // Just append '"' to close the string, and then append the rest of next line
                // after the leading '"' (which is the duplicate closing quote we don't want)
                
                // Pattern: line ends with BACKSLASH + 'n', missing closing "
                // next starts with "; (closing quote then rest)
                // Fix: append '"' to line, then append ';' and rest of next after the '"'
                // Result: cout << "}\n"; which is perfect C++!
                
                if (line.endsWith(BACKSLASH + 'n')) {
                    // next starts with " which is the closing quote
                    // Just append " to close the string, and the rest after " from next
                    const afterQuote = nextTrimmed.substring(1); // remove the leading "
                    fixed.push(line + '"' + afterQuote);
                    changeCount++;
                } else {
                    // Line has odd quotes but doesn't end with \n - just join
                    fixed.push(line + nextTrimmed);
                    changeCount++;
                }
                i += 2;
                continue;
            }
        }
        
        // Also fix the case where fix_all produced incorrect results
        // Pattern: '...\n" + literal_\n + '";\n' -> need to clean up
        // This would show as line containing: "\n"\n";\n or similar
        // Actually, let's just handle the remaining broken patterns
        
        fixed.push(line);
        i++;
    }
    
    const result = fixed.join('\n');
    fs.writeFileSync(fname, result);
    console.log(dir + ': ' + changeCount + ' additional fixes, length=' + result.length);
}
