var katescript = {
    "name": "Julia",
    "author": "Ilia Kats <ilia-kats@gmx.net>, Paul Giannaros <paul@giannaros.org>, Gerald Senarclens de Grancy <oss@senarclens.eu>",
    "license": "LGPL",
    "revision": 1,
    "kate-version": "5.1",
    "indent-languages": ["Julia"]
}; // kate-script-header, must be at the start of the file without comments, pure json

// required katepart js libraries
require("range.js");
require("string.js");

openings = ['(', '[', '{'];
closings = [')', ']', '}'];  // requires same order as in openings
imports = ["using", "import"]
linestart_block_start = ["function", "if", "try", "for", "while", "let", "quote", "mutable struct", "struct"];
linemid_block_start = ["do", "begin"];
block_continue = ["else", "elseif", "catch", "finally"];
block_continue_lineend = ["else", "finally", "catch"];
block_continue_linemid = ["elseif", "catch"];
unindenter = "end";

triggerCharacters = "efhyd ";

immediate_unindenters = new Set(block_continue.concat([unindenter]));
all_block_expr = new RegExp(
        "\\b(" +
        [
            linestart_block_start.join("|"),
            linemid_block_start.join("|"),
            block_continue.join("|") + "|" +
            unindenter
        ].join("|") +
        ")\\b"
, "g");
full_block_expr = new RegExp(
        "\\b(" +
        linestart_block_start.join("|") + "|" +
        linemid_block_start.join("|") + "|" +
        unindenter +
        ")\\b"
, "g");
linestart_block_start_expr = new RegExp("\\b(" + linestart_block_start.join("|") + ")\\b");
linemid_block_start_expr = new RegExp("\\b(" + linemid_block_start.join("|") + ")\\b");
unindenters_newline_expr = new RegExp(
        "\\b((" +
        block_continue_lineend.join("$|") + "$|" +
        unindenter + "$)|((" +
        block_continue_linemid.join("|") +
        ")(?!.*" + all_block_expr.source + ")))\\b"
);

block_starts = new Set(linestart_block_start.concat(linemid_block_start));

var imports_expr = new RegExp("((\\s*\\w+\\s*:)(\\s*\\w+\\s*,)*|(\\s*\\w+\\s*,)+)$");
var imports_end_expr = new RegExp("(\\s*(\\w+\\s*:?)\\s*,\\s*)*\\s*(\\w+)\\s*$");

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
    for (;virtcol < line.length; ++virtcol) {
        if (!document.isComment(lineNr, virtcol)) {
            code += line[virtcol];
        }
    }
    return code.trim();
}

function getLastCodePosition(lineNr, lastVirtCol=-1) {
    var line = document.line(lineNr);
    virtcol = lastVirtCol >= 0 ? lastVirtCol : document.lastVirtualColumn(lineNr);
    if (virtcol < 0)
        return -1
    for (; virtcol >= 0; --virtcol) {
        if (!document.isComment(lineNr, virtcol) && !document.isSpace(lineNr, virtcol))
            break;
    }
    return virtcol;
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
    var indent = _calcOpeningIndent(lineNr);
    if (indent > -1)
        return indent
    indent = _calcClosingIndent(lineNr);
    if (indent > -1)
        return indent
    return -1;
}

function findMatchingParenthesis(lineNr, charNr) {
    var nests = 1;
    charNr = charNr + document.firstVirtualColumn(lineNr);
    while (lineNr >= 0) {
        var line = document.line(lineNr);
        for (i = Math.min(charNr - 1, line.length - 1); i >= 0; --i) {
            if (document.isComment(lineNr, i) || document.isString(lineNr, i))
                continue;
            if (line[i] == '(')
                nests -= 1;
            else if (line[i] == ')')
                nests += 1;
            if (nests == 0)
                return [lineNr, i - document.firstVirtualColumn(lineNr)];
        }
        --lineNr;
        charNr = Infinity;
    }
    return [-1, -1]
}

// Return true if a single unindent should occur.
function shouldUnindent(LineNr, bracketOnly=true) {
    // unindent if the last line was indented b/c of an operator
    if (LineNr >= 1) {
        var lastcodepos = getLastCodePosition(LineNr - 1);
        if (lastcodepos > 0 && document.defStyleNum(LineNr - 1, lastcodepos) == dsOperator)
            return 1;
        if (!bracketOnly && document.line(LineNr)[lastcodepos] != ',') {
            var cline = getCode(LineNr);
            if (!imports.some(imp => cline.startsWith(imp)) && imports_end_expr.test(cline)) {
                --LineNr;
                for (; LineNr >= 0; --LineNr) {
                    cline = getCode(LineNr);
                    if (!cline.length)
                        continue;
                    if (imports_expr.test(cline)) {
                        if (imports.some(imp => cline.startsWith(imp)))
                            return 1;
                    } else
                        break;
                }
            }
        }
    }
    return 0;
}

// finds the indentation level of a matching block start for a block end
function findMatchingBlock(lineNr, trigger) {
    var nested_threshold = trigger == unindenter ? 1 : 0;
    var nestedBlocks = 0;
    var found = false;
    for (var linenr = lineNr; linenr >= 0; --linenr) {
        var lline = getCode(linenr);
        var matches = [];
        while (match = full_block_expr.exec(lline)) {
            matches.push(match);
        }
        if (matches.length > 0) {
            for (match of matches.reverse()) {
                if (match[0] == unindenter) {
                    var lastclosingbracket = lline.lastIndexOf("]", match.index);
                    var lastopeningbracket = lline.lastIndexOf("[", match.index);
                    // end is also used in subsetting to get the last element, ignore this
                    // for block calculation
                    if (lastopeningbracket == -1 || lastclosingbracket > lastopeningbracket)
                        ++nestedBlocks;
                } else if (nestedBlocks == nested_threshold) {
                    found = block_starts.has(match[0]);
                    if (found)
                        break;
                } else {
                    --nestedBlocks;
                }
            }
        }
        if (found)
            break;
    }
    return found && linenr < lineNr ? document.firstVirtualColumn(linenr) : -1;
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

    var alignOnly = character == "";

    if (alignOnly || character != '\n') {
        var lline = getCode(line);
        if (immediate_unindenters.has(lline))
            return findMatchingBlock(lline == unindenter ? line : line - 1, lline);
        else if (!alignOnly)
            return -2
    }

    while (line > 0 && !document.line(line - 1).length) // ignore empty lines
        --line;
    --line;

    var virtcol = document.firstVirtualColumn(line);
    var lastLine = getCode(line, virtcol);
    var lastChar = lastLine.substr(-1);

    // indent when opening bracket or an operator is at the end the previous line
    if (openings.indexOf(lastChar) >= 0 || document.defStyleNum(line, getLastCodePosition(line)) == dsOperator) {
        return virtcol + indentWidth;
    }
    var indent = calcBracketIndent(line, indentWidth);
    if (lastChar == ')') {
        // could be a function definition with multi-line argument list
        var matching = findMatchingParenthesis(line, lastLine.length - 1);
        if (matching[0] > -1) {
            line = matching[0];
            virtcol = document.firstVirtualColumn(line);
            lastLine = getCode(line, virtcol);
        }
        else
            return -1;
    } else if (lastChar == ',' || lastChar == ':') {
        var is_import = false;
        for (const imp of imports) {
            if (lastLine.startsWith(imp))
                return virtcol + indentWidth;
        }
    }

    // this also covers single-line blocks
    var match = lastLine.match(unindenters_newline_expr);
    if (match) {
        var mblock = findMatchingBlock(line, match[0]);
        if (mblock > -1) {
            return match[0] == unindenter ? mblock : mblock + indentWidth;
        } else
            return -1;
    }

    startblock = lastLine.search(linestart_block_start_expr);
    if (startblock == 0) {
        if (indent > virtcol)
            return indent; // unmatched open bracket in proper block start
        else
            return virtcol + indentWidth;
    } else if (startblock > 0 || lastLine.search(linemid_block_start_expr) > -1) { // single-line blocks
        var endpos = lastLine.lastIndexOf(unindenter);
        var lastkeyword = unindenter;
        block_continue.forEach(function(keyw) {
            var pos = lastLine.lastIndexOf(keyw);
            if (pos >= 0 && pos > endpos) {
                lastkeyword = keyw;
                endpos = pos;
            }
        });
        if (endpos >= 0)
            return findMatchingBlock(line, lastkeyword);
        else {
            if (indent > -1)
                indent += indentWidth; // unmatched open bracket in possibly nested blocks
            else
                indent = virtcol + indentWidth;
        }
    }
    if (shouldUnindent(line, false) && (indent == -1)) {
        indent = Math.max(0, virtcol - indentWidth);
    }
    return indent;
}

// kate: space-indent on; indent-width 4; replace-tabs on;
