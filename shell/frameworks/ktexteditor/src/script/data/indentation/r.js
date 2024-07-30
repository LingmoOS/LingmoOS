var katescript = {
    "name": "R",
    "author": "Pierre de Villemerereuil <pierre.de.villemereuil@mailoo.org>",
    "license": "MIT",
    "revision": 1,
    "kate-version": "5.0",
    "indent-languages": ["R", "R Script", "Script R"]
}; // kate-script-header, must be at the start of the file without comments

// Some functions modified from Python indentation file 
// (credit to Paul Giannaros <paul@giannaros.org>, Gerald Senarclens de Grancy <oss@senarclens.eu>)

// required katepart js libraries
require ("range.js");
require ("string.js");
require ("utils.js")

triggerCharacters = '})]';

openings  = ['(', '[','{'];
closings  = [')', ']','}'];  // requires same order as in openings
operators = ['+', '-', '*', '/', '^',
             '&', '|', '==', '>', '<', '<=', '>=', '!=',
             '%%', '%*%', '%/%', '%in%', 
             '%>%', '%T>%', '%$%', '%<>%'];
equalOperatorSigns = ['=', '>', '<', '!'];

var debugMode = false;

function dbg() {
    if (debugMode) {
        debug.apply(this, arguments);
    }
}

// Extension of endswith for an array of tests
function endsWithAny(suffixes, string) {
    return suffixes.some(function (suffix) {
        return string.endsWith(suffix);
    });
}

// Return the given line without comments and leading or trailing whitespace.
// but keep strings (compared to getCode)
// Eg.
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "  for i in range(3):"
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "for i in range(3):  "
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "for i in range(3):  # grand"
function getCodeWithString(lineNr) {
    var line = document.line(lineNr);
    var code = '';
    for (var position = 0; position < line.length; position++) {
        if (document.isCode(lineNr, position) || document.isString(lineNr, position) || document.isOthers(lineNr, position)) {
            code += line[position];
        }
    }
    return code.trim();
}

// Return the given line without comments and leading or trailing whitespace.
// Eg.
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "  for i in range(3):"
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "for i in range(3):  "
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "for i in range(3):  # grand"
function getCode(lineNr) {
    var line = document.line(lineNr);
    var code = '';
    for (var position = 0; position < line.length; position++) {
        if (document.isCode(lineNr, position)) {
            code += line[position];
        }
    }
    return code.trim();
}

// Returns the number of spaces after "pos""
// on the line number "lineNr"
function countSpaces(lineNr, pos) {
    var line = document.line(lineNr);
    var add = 0;
    var pos = pos + 1;
    while (line[pos] == " ") {
        add++;
        pos++;
    }
    return add;
}

// Returns the indent if finding a mismatch using brackets, commas and equal signs
// If there are no mismatch, -1 is returned.
// `lineNr`: number of the line for which the indent is calculated
function calcMismatchIndent(lineNr, add) {
    // initialising some counters
    var countClosing = new Array();
    closings.forEach(function(elem) {
        countClosing[elem] = 0;
    });
    if (closings.indexOf(add) > -1 && add.length) {
        countClosing[add]++;
    }
    var countComma = 0;
    
    // starting looking for mismatches
    for (var i = lineNr; i >= 0; --i) {
        var lastOpen = document.line(i).length;
        var lineString = document.line(i);
        for (var j = lineString.length; j >= 0; --j) {
            // Ignore comments and strings
            if (document.isComment(i, j) || document.isString(i, j))
                continue;
            // Testing for brackets
            // If a closing bracket, add 1 to counter
            if (closings.indexOf(lineString[j]) > -1) {
                countClosing[lineString[j]]++;
            }
            // If an opening bracket, add 1 to the corresponding closing counter
            var index = openings.indexOf(lineString[j]);
            if (index > -1) {
                countClosing[closings[index]]--;
                // If an open-but-not-closed bracket is found
                // Return indent corresponding to its position
                if (countClosing[closings[index]] == -1)
                    return {indent : j + 1, line : i, pos: lastOpen, type : "unclosed"};
                // Otherwise just update "lastOpen" to j
                lastOpen = j;
            }
            // If the start of the line is reached and
            // no comma was "opened"
            if (j == document.firstVirtualColumn(i) && countComma == 0) {
                // Test if all brackets are closed
                var allclosed = true;
                for (var key in countClosing) {
                    if (countClosing[key] != 0) {
                        allclosed = false;
                    }
                }                // If they are all closed, return the indent of this line
                dbg("allclosed: " + allclosed)
                if (allclosed) {
                    // if we didn't move, return -1 (keep indent), else return the indent of line i
                    if (i == lineNr) {
                        return {indent : -1, line : i, pos: lastOpen, type : "allclosed"};
                    } else {
                        return {indent : j, line : i, pos: lastOpen, type : "allclosed"};
                    }
                }
            }
            // Counting the commas if needed
            if (lineString[j] == ',') {
                // If not between comma-inducing brackets
                if (countClosing[')'] == 0 && countClosing[']'] == 0)
                    countComma++;
            }

//             // Handling equal signs
//             if (lineString[j] == "=" && equalOperatorSigns.indexOf(lineString[j - 1]) == -1 && lineString[j + 1] != "=") {
//                // If not between equal-inducing brackets
//                if (countClosing[')'] == 0 && countClosing[']'] == 0) {
//                    // If no comma is "closing" the equal
//                    if (countComma == 0) {
//                        // Return the position of equal to create a new indent
//                        return {indent : j + 1 + countSpaces(i,j), line : i, type : "equal"};
//                    }
//                }
//             }
        }
    }
    return {indent : -1, line : i, type : "unknown"};
}

// Find the position of an align operator with in the focus line
// accounting for opening and closing brackets
// `lineNr`: number of the line to search the align operator in
// Returns -1 if nothing was found
function findAlignOperator(lineNr, pos) {
    var lineString = document.line(lineNr);
    // initialising some counters
    var countClosing = new Array();
    closings.forEach(function(elem) {
        countClosing[elem] = 0;
    });
    
    for (var j = pos - 1; j >= 0; --j) {
        // Ignore comments and strings
        if (document.isComment(lineNr, j) || document.isString(lineNr, j))
            continue; 
        
        // Testing for brackets
        // If a closing bracket, add 1 to counter
        if (closings.indexOf(lineString[j]) > -1) {
            countClosing[lineString[j]]++;
        }
        
        // If an opening bracket, add 1 to the corresponding closing counter
        var index = openings.indexOf(lineString[j]);
        if (index > -1) {
            countClosing[closings[index]]--;
            // If an open-but-not-closed bracket is found
            // Return -1 because nothing was found
            if (countClosing[closings[index]] == -1)
                return -1
        }
              
        
        if (lineString.substr(j - 1, 2) == "<-" ||
            (lineString[j] == "=" && equalOperatorSigns.indexOf(lineString[j - 1]) == -1 && lineString[j + 1] != "=") ||
            lineString[j] == "~") 
        {
            // Test if all brackets are closed
            var allclosed = true;
            for (var key in countClosing) {
                if (countClosing[key] != 0) {
                    allclosed = false;
                }
            }                // If they are all closed, return the indent of this line
            if (allclosed) {
                return j;
            }
        }
    }

    return -1
}

// Returns the indent based on operators
// `lineNr`: number of the line for which the indent is calculated 
// `indentWidth` : indent width
// `lineLastOp` : does the line on which returns was hit end with an operator
//  (note that the line for lineLastOp is not necessarily lineNr)
function calcOperatorIndent(lineNr, indentWidth, pos, lineLastOp) {
    var currentIndent = document.firstVirtualColumn(lineNr);
    var refLine = lineNr;
    // If we haven't indented yet and line ends up with an operator
    // then indent
//     if (currentIndent == 0 && lineLastOp) {
//         return indentWidth;
//     }
    // If the current line ends up with an operator
    if (lineLastOp) {
        var previousLine = getCodeWithString(refLine - 1);
        while (previousLine == '' && refLine >= 0) {
            refLine = refLine - 1;
            previousLine = getCodeWithString(refLine - 1);
        }
        // If the line before the indent line doesn't ends up with an operator
        if (!endsWithAny(operators, previousLine)) {
            // then look for align operator
            locAlign = findAlignOperator(lineNr, pos);
            if (locAlign != -1) {
                return locAlign + 1 + countSpaces(lineNr, locAlign);
            } else {
                // if no align operator, simply indent
                return currentIndent + indentWidth;
            }
        } else {
            // else don't
            return currentIndent;
        }
    } else {
        var previousLine = getCodeWithString(refLine - 1);
        while (previousLine == '' && refLine >= 0) {
            refLine = refLine - 1;
            previousLine = getCodeWithString(refLine - 1);
        }
        
        // If the previous line ends with an operator
        if (endsWithAny(operators, previousLine)) {
            // Looking for the start of the operator indenting
            for (i = refLine - 1; i>=0; --i) {
                // If commented line, skip
                if (getCodeWithString(i) != '') {
                    // If we indented in the past
                    if (document.firstVirtualColumn(i) < currentIndent) {
                        currentIndent = document.firstVirtualColumn(i);
                        var previousLine = getCodeWithString(i - 1);
                        // and doesn't end up with an operator
                        if (!endsWithAny(operators, previousLine)) {
                            //return this line indent otherwise
                            return currentIndent;
                        }
                    }
                }
            }
        } else {
                return currentIndent; 
        }
        
        // If the current line doesn't ends up with an operator, we might need to unindent
        // Let's look above
        for (i = refLine; i>=0; --i) {
            // If a line has a lower indent
            if (document.firstVirtualColumn(i) <= currentIndent) {
                currentIndent = document.firstVirtualColumn(i);
                var previousLine = getCodeWithString(i - 1);
                // and doesn't end up with an operator
                if (!endsWithAny(operators, previousLine)) {
                    //return this line indent
                    return currentIndent;
                }
            }
        }
    }
    return -1;
}

// Align when a closing bracket was entered
// `lineNr`: number of the line for which the indent is calculated 
// `c` : the bracket that was entered
function alignBrackets(lineNr, lastChar, newChar, indentWidth) {
    var charsMatch = ( lastChar == '(' && newChar == ')' ) ||
                     ( lastChar == '{' && newChar == '}' ) ||
                     ( lastChar == '[' && newChar == ']' );
    if (charsMatch) {
        indentation = document.firstVirtualColumn(lineNr - 1);
        document.insertText(lineNr, document.firstColumn(lineNr), "\n");
        view.setCursorPosition(lineNr, document.line(lineNr).length);
        document.indent(new Range(lineNr + 1, 0, lineNr + 1, 1), indentation / indentWidth);
    }
    return document.firstVirtualColumn(lineNr - 1) + indentWidth;
}


// Return the amount of characters (in spaces) to be indented.
// Special indent() return values:
//   -2 = no indent
//   -1 = keep last indent
function indent(line, indentWidth, ch) {
    if (line == 0)  // don't ever act on document's first line
        return -2;
    var lastLine = getCodeWithString(line - 1);
    var lastChar = lastLine.substr(-1);
    var newChar  = document.line(line).substr(document.firstVirtualColumn(line), 1);
    dbg("newChar = '" + newChar + "'");
    dbg("lastChar = '" + lastChar + "'");
    dbg("ch = '" + ch + "'");
        
    // Align if a closing bracket was entered
    // (note that the code above might also be triggered if a bracket is closed right after an opened one)
    if (closings.indexOf(ch) > -1 && ch.length) {
        dbg("closings");
        //if the entered bracket doesn't start the line, do nothing
        if (newChar != ch) {
            return document.firstVirtualColumn(line);
        }
        var matchOpen = openings[closings.indexOf(ch)];
        if (lastChar == matchOpen) {
            return alignBrackets(line, lastChar, newChar, indentWidth)
        } else {
            var mismatch = calcMismatchIndent(line - 1, ch);
            return document.firstVirtualColumn(mismatch.line);
        }
    }
             
    // opening brackets and returns: indent (and unindent following bracket if needed)
    if (openings.indexOf(lastChar) > -1 && lastChar.length) {
        return alignBrackets(line, lastChar, newChar, indentWidth);
    }
       
    // calculate indents based on mismatch of brackets, commas and equal signs 
    var mismatch = calcMismatchIndent(line - 1, '');
    var indent = mismatch.indent;
    
    dbg("mismatch.line = " + mismatch.line)
    dbg("mismatch.pos = " + mismatch.pos)
    dbg("mismatch.indent = " + mismatch.indent)
    dbg("mismatch.type = " + mismatch.type)
      
    // if indent is based on non-opened brackets, try indent because of operators
    // Don't do it if the end is "<-" though (necessary because "-" is an operator...)
    if (mismatch.type == "allclosed" && !lastLine.endsWith('<-')) {
        // compute indent due to an operator
        indent = calcOperatorIndent(mismatch.line, indentWidth, mismatch.pos, endsWithAny(operators, lastLine));
    }
    
    if (mismatch.type == "unclosed" && endsWithAny(operators, lastLine)) {
        dbg("unclosed");
        // look whether we should account for an align operator
        locAlign = findAlignOperator(mismatch.line, mismatch.pos);
        if (locAlign != -1) {
            indent = locAlign + 1 + countSpaces(mismatch.line, locAlign);
        }
    }
    
    // if unclosed bracket is found, check that the line didn't end with this bracket
    // if it did, do not change anything to the indent
    if (mismatch.type == "unclosed" && mismatch.indent == document.line(mismatch.line).length) {
        indent = document.firstVirtualColumn(mismatch.line + 1);
    }
       
    // At that point, we might have computed an indent equal to the current one,
    // let's keep it simple and set indent to -1 in that case
    if (document.firstVirtualColumn(line - 1) == indent) {
        indent = -1;
    }
    
    // If the next character is a closing bracket, 
    // let's see if we should indent back to the corresponding indent
    if (closings.indexOf(newChar) > -1 && newChar.length) {
        dbg("next char is closing bracket");
        mismatch = calcMismatchIndent(line - 1, newChar);
        // change indent only if the closing bracket matches a "final" one
        if (endsWithAny(openings, getCodeWithString(mismatch.line))) {
            return document.firstVirtualColumn(mismatch.line);
        }
    }
    
    // Assignment is important and particular, so always indent when we do it
    if (getCodeWithString(line - 1).endsWith('<-') || getCodeWithString(line - 1).endsWith('=')) {
        if (indent > -1)
            indent += indentWidth;
        else
            indent = document.firstVirtualColumn(line - 1) + indentWidth;
    }
    return indent;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
