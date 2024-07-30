var katescript = {
    "name": "Python",
    "author": "Paul Giannaros <paul@giannaros.org>, Gerald Senarclens de Grancy <oss@senarclens.eu>",
    "license": "LGPL",
    "revision": 4,
    "kate-version": "5.1",
    "indent-languages": ["Python"]
}; // kate-script-header, must be at the start of the file without comments, pure json

// required katepart js libraries
require ("range.js");
require ("string.js");

openings = ['(', '[', '{'];
closings = [')', ']', '}'];  // requires same order as in openings
unindenters = /\b(continue|pass|raise|return|break)\b/;
immediate_unindenters = new Set(["else", "elif", "finally", "except"])

triggerCharacters = ": ";

var debugMode = false;

function dbg() {
    if (debugMode) {
        debug.apply(this, arguments);
    }
}

// Return the given line without comments and leading or trailing whitespace.
// Eg.
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "  for i in range(3):"
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "for i in range(3):  "
// getCode(x) -> "for i in range(3):"
//     if document.line(x) == "for i in range(3):  # grand"
function getCode(lineNr, virtcol=-1) {
    var line = document.line(lineNr);
    var code = '';
    virtcol = virtcol >= 0 ? virtcol : document.firstVirtualColumn(lineNr);
    if (virtcol < 0)
        return code;
    for (; virtcol < line.length; ++virtcol) {
        if (document.isCode(lineNr, virtcol)) {
            code += line[virtcol];
        }
    }
    return code.trim();
}


// Return the indent if a opening bracket is not closed (incomplete sequence).
// The calculated intent is the innermost opening bracket's position plus 1.
// `lineNr`: the number of the line on which the brackets should be counted
function _calcOpeningIndent(lineNr) {
    var line = document.line(lineNr);
    var countClosing = new Array();
    closings.forEach(function(elem) {
        countClosing[elem] = 0;
    });
    for (i = line.length - 1; i >= 0; --i) {
        if (document.isComment(lineNr, i) || document.isString(lineNr, i))
            continue;
        if (closings.indexOf(line[i]) > -1)
            countClosing[line[i]]++;
        var index = openings.indexOf(line[i]);
        if (index > -1) {
            if (countClosing[closings[index]] == 0) {
                return i + 1;
            }
            countClosing[closings[index]]--;
        }
    }
    return -1;
}


// Return the indent if a closing bracket not opened (incomplete sequence).
// The intent is the same as on the line with the unmatched opening bracket.
// `lineNr`: the number of the line on which the brackets should be counted
function _calcClosingIndent(lineNr, indentWidth) {
    var line = document.line(lineNr);
    var countClosing = new Array();
    closings.forEach(function(elem) {
        countClosing[elem] = 0;
    });
    for (i = line.length - 1; i >= 0; --i) {
        if (document.isComment(lineNr, i) || document.isString(lineNr, i))
            continue;
        if (closings.indexOf(line[i]) > -1)
            countClosing[line[i]]++;
        var index = openings.indexOf(line[i]);
        if (index > -1)
            countClosing[closings[index]]--;
    }
    for (var key in countClosing) {
        if (countClosing[key] > 0) {  // unmatched closing bracket
            for (--lineNr; lineNr >= 0; --lineNr) {
                if (_calcOpeningIndent(lineNr) > -1) {
                    var indent = document.firstVirtualColumn(lineNr);
                    if (shouldUnindent(lineNr + 1))
                        return Math.max(0, indent - indentWidth);
                    return indent;
                }
            }
        }
    }
    return -1;
}


// Returns the indent for mismatched (opening or closing) brackets.
// If there are no mismatched brackets, -1 is returned.
// `lineNr`: number of the line for which the indent is calculated
function calcBracketIndent(lineNr, indentWidth) {
    var indent = _calcOpeningIndent(lineNr - 1);
    if (indent > -1)
        return indent
    indent = _calcClosingIndent(lineNr - 1, indentWidth);
    if (indent > -1)
        return indent
    return -1;
}


// Return true if a single unindent should occur.
function shouldUnindent(LineNr) {
    lastLine = getCode(LineNr - 1);
    if (lastLine.match(unindenters))
        return 1;

    // unindent if the last line was indented b/c of a backslash
    if (LineNr >= 2) {
        secondLastLine = getCode(LineNr - 2);
        if (secondLastLine.length && secondLastLine.substr(-1) == "\\")
            return 1;
    }
    return 0;
}

function findLastIndent(lineNr) {
    for (; getCode(lineNr).length == 0; --lineNr);
    return document.firstVirtualColumn(lineNr);
}

function getDocStringStart(line) {
    var currentLine = line;
    var currentString;

    if (!document.isComment(line, document.firstVirtualColumn(line)) || document.line(currentLine).charAt(document.firstVirtualColumn(line)) === '#') {
        return -1;
    }

    while (currentLine >= 0) {
        currentString = document.line(currentLine - 1);
        if (currentString.charAt(document.firstVirtualColumn(currentLine - 1)) !== '#') {
            if (currentString.includes("'''") || currentString.includes('"""')) {
                break;
            }
        }
        --currentLine;
    }
    return currentLine - 1;
}

function getPrevDocStringEnd(line) {
    var currentLine = line;
    var currentString;

    while (currentLine >= 0) {
        currentString = document.line(currentLine - 1);
        if (currentString.includes("'''") || currentString.includes('"""')) {
            break;
        } else if (getCode(currentLine - 1, 0).length) {
            return -1;
        }
        --currentLine;
    }
    return currentLine - 1;
}

function getMultiLineStringStart(line) {
    var currentLine = line;
    var currentString;

    if (!document.isString(line, document.firstVirtualColumn(line))) {
        return -1;
    }

    while (currentLine >= 0) {
        currentString = document.line(currentLine - 1);
        if (!document.isComment(currentLine - 1, document.firstVirtualColumn(currentLine - 1))) {
            if (currentString.includes("'''") || currentString.includes('"""')) {
                break;
            }
        }
        --currentLine;
    }
    return currentLine - 1;
}

function getPrevMultiLineStringEnd(line) {
    var currentLine = line;
    var currentString;

    while (currentLine >= 0) {
        currentString = document.line(currentLine - 1);
        currentCode = getCode(document.line(currentLine - 1), 0);
        if (!document.isComment(currentLine - 1, document.firstVirtualColumn(currentLine - 1))) {
            if (currentString.includes("'''") || currentString.includes('"""')) {
                break;
            } else if (getCode(currentLine - 1, 0).length) {
                return -1;
            }
        }
        --currentLine;
    }
    return currentLine - 1;
}

// Return the amount of characters (in spaces) to be indented.
// Special indent() return values:
//   -2 = no indent
//   -1 = keep last indent
// Follow PEP8 for unfinished sequences and argument lists.
// Nested sequences are not implemented. (neither by Emacs' python-mode)
function indent(line, indentWidth, character) {
    if (line == 0)  // don't ever act on document's first line
        return -2;
    if (!document.line(line - 1).length)  // empty line
        return -2;

    if (triggerCharacters.indexOf(character) > -1 && character.length) {
        var virtcol = document.firstVirtualColumn(line);
        var lline = getCode(line, virtcol);
        if (character != " ")
            lline = lline.substring(0, lline.length - 1);
        if (immediate_unindenters.has(lline) && virtcol == findLastIndent(line - 1) && lline.length == document.line(line).length - virtcol - 1)
            return Math.max(0, virtcol - indentWidth);
        else
            return -2
    }

    var virtcol = document.firstVirtualColumn(line - 1);
    var lastLine = getCode(line - 1, virtcol);
    var lastChar = lastLine.substr(-1);

    // indent when opening bracket or backslash is at the end the previous line
    if (openings.indexOf(lastChar) >= 0 || lastChar == "\\") {
        return virtcol + indentWidth;
    }
    var indent = calcBracketIndent(line, indentWidth);
    if (lastLine.endsWith(':')) {
        if (indent > -1)
            indent += indentWidth;
        else
            indent = virtcol + indentWidth;
    }
    docStringStart = getDocStringStart(line);
    if (docStringStart > -1) {
        if (docStringStart === line) {
            return -1;
        }
        dbg("line = {" + document.line(line) + "} docStringStart = {" + document.line(docStringStart) + "}" + " dsno = " + docStringStart);
        return document.firstVirtualColumn(line) + document.firstVirtualColumn(docStringStart) - indentWidth;
    }
    multiLineStringStart = getMultiLineStringStart(line);
    dbg("docStringStart = " + docStringStart);
    dbg("multiLineStringStart = " + multiLineStringStart);
    if (multiLineStringStart > -1) {
        if (multiLineStringStart === line) {
            return -1;
        }
        return -2;
    }
    if (indent === -1) {
        prevMultiLineStringEnd = getPrevMultiLineStringEnd(line);
        if (prevMultiLineStringEnd > -1) {
            prevMultiLineStringStart = getMultiLineStringStart(prevMultiLineStringEnd);
            return document.firstVirtualColumn(prevMultiLineStringStart);
        }
    }
    if (indent === -1) {
        prevDocStringEnd = getPrevDocStringEnd(line);
        if (prevDocStringEnd > -1) {
            prevDocStringStart = getDocStringStart(prevDocStringEnd);
            return document.firstVirtualColumn(prevDocStringStart);
        }
    }
    // continue, pass, raise, return etc. should unindent
    if (shouldUnindent(line) && (indent == -1)) {
        indent = Math.max(0, virtcol - indentWidth);
    }
    return indent;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
