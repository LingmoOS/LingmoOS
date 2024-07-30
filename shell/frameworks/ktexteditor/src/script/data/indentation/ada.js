var katescript = {
    "name": "ada",
    "author": "Trevor Blight <trevor-b@ovi.com>",
    "license": "LGPL",
    "revision": 2,
    "kate-version": "5.1"
}; // kate-script-header, must be at the start of the file without comments, pure json

/*
    This file is part of the Kate Project.

    SPDX-License-Identifier: LGPL-2.0-only
*/
// loosely based on the vim ada indent file
//-------- extract from original vim header follows --------------
//
//  Description: Vim Ada indent file
//     Language: Ada (2005)
//    Copyright: Copyright (C) 2006 Martin Krischik
//   Maintainer: Martin Krischik <krischik@users.sourceforge.net>
//		Neil Bird <neil@fnxweb.com>
//		Ned Okie <nokie@radford.edu>
//
//--------------- end of extract -----------------

// apart from porting and translating from vim script to kate javascript,
// changes compared to orininal vim script:
// - added handling for labels and 'generic' keyword
// - recognise keyword 'private' as part of package specification
// - handle multi line for/while ... loop
// - cat open/close paren lines to handle scanning multi-line proc/func dec's
// - recognise various multi line statements
// - indent once only after case statement
// - add checkParens() for aligning parameters in multiline arg list
// - refactoring & optimisations

// TODO: 'protected', 'task', 'select' &
//       possibly other keywords not recognised

// required katepart js libraries
require ("range.js");
require ("cursor.js");
require ("string.js");

//BEGIN USER CONFIGURATION

// send debug output to terminal
// the ada indenter recognises this as a document variable
var debugMode = false;

//END USER CONFIGURATION

var AdaComment    = /\s*--.*$/;
var unfStmt       = /([.=\(]|:=[^;]*)\s*$/;
var unfLoop       = /^\s*(for|while)\b(?!.*\bloop\b)/i;
var AdaBlockStart = /^\s*(if\b|while\b|else\b|elsif\b|loop\b|for\b.*\b(loop|use)\b|declare\b|begin\b|type\b.*\bis\b[^;]*$|(type\b.*)?\brecord\b|procedure\b|function\b|with\s+function\b|accept\b|do\b|task\b|generic\b|package\b|private\b|then\b|when\b|is\b)/i;
var StatementStart = /^\s*(if|when|while|else|elsif|loop|for\b.*\b(loop|use)|begin)\b/i;

function dbg() {
    if (debugMode) {
        debug(Array.prototype.join.call(arguments, ' '));
    }
}


//BEGIN global variables and functions

// none implemented

//END global variables and functions


// regexp of keywords & patterns that cause reindenting of the current line.
var AdaReIndent = /^\s*((then|end|elsif|when|exception|begin|is|record|private)\s+|<<\w+>>|end;|[#\)])(.*)$/;

// characters which trigger indent, beside the default '\n'
var triggerCharacters = " \t)#>;";

// check if the trigger characters are in the right context,
// otherwise running the indenter might be annoying to the user
function reindentTrigger(line)
{
    var res = AdaReIndent.exec(document.line(line));
    dbg("reindentTrigger: checking line, found", ((res && res[3] == "" )? "'"+ (res[2]?res[2]:res[1])+"'": "nothing") );
    return ( res && (res[3] == "" ));
} // reindentTrigger


/**
 * Find last non-empty line that is code
 */
function lastCodeLine(line)
{
    do {
        line = document.prevNonEmptyLine(--line);
        if ( line == -1 ) {
            return -1;
        }

        // keep looking until we reach code
        // skip labels & preprocessor lines
        var p = document.firstColumn(line);
    } while( document.firstChar(line) == '#'
             || !( document.isCode(line, document.firstColumn(line) )
                || document.isString(line, document.firstColumn(line) )
                || document.isChar(line, document.firstColumn(line) ) )
             || /^\s*<<\w+>>/.test(document.line(line)) );

    return line;
} // lastCodeLine()


/* test if line has an unclosed opening paren '('
 * line is document line
 * n is nr chars to test, starting at pos 0
 * return index of unmatched '(', or
 *        -1 if no unmatched '('
 */
function checkOpenParen( line, n )
{
    var nest = 0;
    var i = n;
    while( nest <= 0 && --i >= 0 ) {
        if( document.isCode(line, i) ) {
            var c = document.charAt( line, i);
            if( c == ')' )      nest--;
            else if( c == '(' ) nest++;
        }
    }
    //dbg("string " + document.line(line) + (i==-1? " has matched open parens": " has unclosed paren at pos " + i ));
    return i;
}

/* test if line has an unmatched close paren
 * line is document line
 * n is nr chars to test, starting at pos 0
 * return index of rightmost unclosed paren
 *        -1 if no unmatched paren
 */
function checkCloseParen( line, n )
{
    var nest = 0;
    var i = -1;
    var bpos = -1;
    while( ++i < n ) {
        if( document.isCode(line, i) ) {
            var c = document.charAt( line, i);
            if( c == ')' )      nest++;
            else if( c == '(' ) nest--;
        }
        if( nest > 0 ) {
            nest = 0;
            bpos = i;
        }
    }
    //dbg("string " + document.line(line) + (bpos==-1? " has matched close parens": " has unbalanced paren at pos " + bpos) );
    return bpos;
}


/*********************************************
 * line up args inside parens
 * if the opening line has no args, line up with first arg on next line
 * ignore trailing comments.
 * opening line ==> figure out indent based on pos of first arg
 * subsequent args ==> maintain indent
 *       ');' ==> align closing paren
 *
 *  paren indenting has these cases:
 *   pline: (    no args     -- new line indented
 *   pline: (    args        -- new line aligns with first arg
 *   pline  )                -- check if still inside parens
 *   cline  );   no args     -- open line, align with ( or args
 *   cline  );   args        -- align with line above, same as just args
 *               just args   -- align with line above
 */
function checkParens(line, indentWidth, newLine )
{

    var openParen = document.anchor(line, 0, ')');
    if( !openParen.isValid() ) {
        // nothing found, go back
        //dbg("checkParens: not in parens");
        return -1;
    }

    // note: pline is code, and
    // we get here only when we know it exists
    var pline = lastCodeLine(line);
    var plineStr = document.line(pline).replace(AdaComment, "");

    /**
     * look for the case where the new line starts with ')'
     */
    if( document.firstChar(line) == ')' ) {
        // we pressed enter in e.g. ()
        var argIndent = document.firstVirtualColumn(pline);

        var parenIndent = document.toVirtualColumn(openParen);

        // look for something like
        //        argn     <--- pline
        //        );       <--- line, trailing close paren
        // the trailing close paren lines up with the args or the open paren,
        // whichever is the leftmost
        if( pline > openParen.line && argIndent > 0 && argIndent < parenIndent) {
            parenIndent = argIndent;
        }

        // just align parens, not args
        if( !newLine || !/[,\(]$/.test(plineStr) ) {
            dbg("checkParens: align trailing ')'");
            return parenIndent;
        }

        /* now we have one of 2 scenarios:
         *    arg,  <--- pline, , ==> new arg to be inserted
         *    )     <--- line
         * or
         *    xxx(  <--- pline
         *    )     <--- line
         *
         * in both cases, we need to open a line for the new arg and
         * place closing paren in same column as the opening paren
         * we leave cursor on the blank line, ie
         *
         * ()
         * becomes:
         * (
         *   |
         * )
         */

        document.insertText(line, document.firstColumn(line), "\n");
        var anchorLine = line+1;
        // indent closing anchor
        view.setCursorPosition(line, parenIndent);
        document.indent(new Range(anchorLine, 0, anchorLine, 1), parenIndent / indentWidth);
        // make sure we add spaces to align perfectly on left anchor
        var padding =  parenIndent % indentWidth;
        if ( padding > 0 ) {
            document.insertText(anchorLine,
                                document.fromVirtualColumn(anchorLine, parenIndent - padding),
                                String().fill(' ', padding));
        }
    } // leading close paren


    var indent = document.firstVirtualColumn(line);

    // look for line ending with unclosed paren like "func("
    // and no args following open paren
    var bpos = plineStr.search(/\($/ );
    if( bpos != -1 && document.isCode(pline, bpos) ) {
        // we have something like
        //        func(     <--- pline
        //                  <--- line, first arg goes here
        // line contains the first arg in a multi line list
        // following args will be lined up against this
        // if line is not empty, assume user has set it - leave it as it is
        dbg( "checkParens: args indented from opening paren" );
        indent = document.toVirtualColumn(pline, bpos) + indentWidth;
        return indent;
    } // trailing open paren

    //  check the line above
    //  if it has unclosed paren ==> indent to first arg
    //  if it has unbalanced closing paren ==> indent to anchor line
    //  other lines, keep indent

    // now look for something like
    //        func( arg1,     <--- pline
    //                        <--- line
    // note: already handled case when there are no args
    bpos = checkOpenParen(pline, document.lineLength(pline));
    if( bpos != -1 ) {
        //dbg( "checkParens: found open paren in ", plineStr.trim() );
        bpos = document.nextNonSpaceColumn(pline, bpos+1);
        dbg("checkParens: aligning arg to", document.wordAt(pline, bpos), "in", plineStr);
        indent = document.toVirtualColumn(pline, bpos );
        return indent;
    } // trailing opening arg


    // check nested parens
    bpos = checkCloseParen(pline, plineStr.length);
    if( bpos != -1 ) {
        // we have something like this:
        //     )  <-- pline
        //        <-- line

        // assert( indent == -1 );
        //dbg( "arg list terminated with '" + plineStr[bpos] + "'", " at pos", bpos );
        openParen = document.anchor( pline, bpos, plineStr[bpos] );
        if (openParen.isValid() ) {
            // it's more complicated than it appears:
            // we might still be inside another arglist!
            // check back from openParen,
            // if another open paren is found then
            //      indent to next non white space after that
            var testLine = openParen.line;
            //dbg("checkParens: testing " + document.line(testLine).substr(0,openParen.column) );
            bpos = checkOpenParen(testLine, openParen.column);
            if( bpos != -1 ) {
                // line above closes parens, still inside another paren level
                //dbg("checkParens: found yet another open paren @ pos", bpos);
                bpos = document.nextNonSpaceColumn(testLine, bpos+1);
                dbg("checkParens: aligning with open paren in line", openParen.line);
                indent = document.toVirtualColumn(testLine, bpos );
                return indent;
            } else if( document.anchor(line,0,'(').isValid() ) {
                dbg("checkParens: aligning to args at next outer paren level");
                return document.firstVirtualColumn(testLine );
            }
        } else {
            // dbg("checkOpenParens: line above closes arg list, can't find open paren!");
        }
    } else {
        // line above doesn't close parens, we may or may not be inside parens
    }

    // we are inside parens, so align with previous line
    dbg("checkParens: inside '(...)', aligning with previous line" );
    return document.firstVirtualColumn(pline );

} // checkParens()


// Try to find indent of the parent of the block we're in
// Hunt for a valid line with a lesser or equal indent than prev_lnum has.
// Assume prev_lnum & all other lines before that are correctly indented
// prev_indent = ignore every thing indented >= this, it's assumed to be an inner block level
// prev_lnum   = line to start looking on
// blockstart  = regexp that indicates a possible start of this block
// stop_at     = if non-null, if a matching line is found, gives up!
function MainBlockIndent (prev_indent, prev_lnum, blockstart, stop_at)
{
   // can't outdent from col 0
   if( prev_indent == 0 ) return 0;

   var pline = prev_lnum;
   while( pline >= 0 ) {
       var plineStr = document.line(pline).replace(AdaComment, "");
       //dbg("MainBlockIndent: plineStr is", plineStr, "stop_at is", stop_at? stop_at.source: "null");
       var ind = document.firstVirtualColumn(pline);
       if( ind < prev_indent ) {
           // we are at an outer level
           if( new RegExp(/^\s*/.source + blockstart.source, "i").test(plineStr) ) {
               //dbg("MainBlockIndent: found start of block at", plineStr.trim() );
	       return ind
	   }
           else if( stop_at
               &&  new RegExp(/^\s*/.source + stop_at.source, "i").test(plineStr) ) {
               dbg("MainBlockIndent: stopped hunting at", plineStr.trim() );
	       return prev_indent
           } // if
       } // if

       pline = lastCodeLine( pline );
   } // while

    // Fallback - just use prev_indent
    dbg("MainBlockIndent: no more code");
    return prev_indent;
} //  MainBlockIndent


// Try to find indent of the block we're in (and about to complete),
// including handling of nested blocks. Works on the 'end' of a block.
// prev_indent = the previous line's indent
// prev_lnum   = previous line (to start looking on)
// blockstart  = expr. that indicates a possible start of this block
// blockend    = expr. that indicates a possible end of this block
function EndBlockIndent( prev_indent, prev_lnum, blockstart, blockend )
{
   // can't outdent from col 0
   if( prev_indent == 0 ) return 0;

    var pline = prev_lnum;
    var ends = 0;
    while( pline >= 0 ) {
        var plineStr = document.line(pline).replace(AdaComment, "");
        //dbg("EndBlockIndent: ind is", ind, ", plineStr is", plineStr);
        if( new RegExp(/^\s*/.source + blockstart.source, "i").test(plineStr) ) {
	    ind = document.firstVirtualColumn(pline);
	    if( ends <= 0 ) {
	        if( ind < prev_indent ) {
                    //dbg("EndBlockIndent: ind is", ind, ", found start of block at '" + plineStr.trim() + "'");
	            return ind
	        } // if
            }
	    else {
                //dbg("EndBlockIndent: out one level,", plineStr.trim());
	        ends = ends - 1;
	    }
        }
        else if( new RegExp(/^\s*/.source + blockend.source, "i").test(plineStr) ) {
            //dbg("EndBlockIndent: in one level,", plineStr.trim());
	    ends = ends + 1;
        } // if

        pline = lastCodeLine(pline);       // Get previous non-blank/non-comment-only line
    } // while

    // Fallback - just use prev_indent
    dbg("EndBlockIndent: no more code");
    return prev_indent;

} // EndBlockIndent


// Return indent of previous statement-start
// Find start of a (possibly multiline) statement
// (after we've indented due to multi-line statements).
// eg,
//   a := very_long_expression             < -- pline, this determines indent
//            + another_long_expression;   < -- prev_lnum, skip this line
//   s1;                                   < -- indent this current line
// This time, we start searching on the line *before* the one given
// (which is the end of a statement - we want the previous beginning).
// find earlier sibling or parent to determine indent of current statement
function StatementIndent( current_indent, prev_lnum )
{
    // can't outdent from col 0
    if( current_indent == 0 ) return 0;

    var pline = prev_lnum;
    var ind;

   while( pline >= 0 ) {
      var ref_lnum = pline;
       pline = lastCodeLine( pline );
       if( pline < 0 ) {
	   return current_indent
       } // if

       var plineStr = document.line(pline).replace(AdaComment, "");
       //if(pline == lastCodeLine( prev_lnum ))
       //   dbg("StatementIndent: current_indent is", current_indent, ", plineStr is", plineStr.trim());
       // Leave indent alone if our ';' line is part of a ';'-delineated
       // aggregate (e.g., procedure args.) or first line after a block start.
       if( AdaBlockStart.test(plineStr)
           ||  /^\s*end\b/i.test(plineStr)  // this was not in vim script
           || /\(\s*$/.test(plineStr) ) {
	   ind = document.firstVirtualColumn(ref_lnum);
           if( ind < current_indent ) {  // this was not in vim script
               //dbg("StatementIndent: ind is", ind, ", taking indent of parent", document.line(ref_lnum).trim());
	       return ind;
           } else {
               //dbg("StatementIndent: stopped looking at", plineStr.trim());
	       return current_indent;
	   } // if
       } // if
       if( ! unfStmt.test(plineStr) ) {
	  ind = document.firstVirtualColumn(ref_lnum);
           //dbg("StatementIndent: ind is", ind, ", sibling found", plineStr.trim());
           if( ind < current_indent ) {
               //dbg("StatementIndent: ind is", ind, ", taking indent of sibling", document.line(ref_lnum).trim());
	       return ind;
	   } // if
       } // if
   } // while

   // Fallback - just use current one
   return current_indent;
} // StatementIndent()


//----------------------
// check what's on the previous line
// return expected indent for next line
function checkPreviousLine( pline, indentWidth )
{
    var kpos;      // temp var to hold char positions in search strings
    var plineStr = document.line(pline).replace(AdaComment, "");

    // test for multi-line statements by concatenating them
    // NOTE: original vim script did not join lines together
    if( /^\s*(renames|access|new)\b/i.test(plineStr) ) {
        var l = lastCodeLine(pline);
        if( l >= 0 ) {
            pline = l;
            plineStr = document.line(pline).replace(AdaComment, "") + plineStr;
            //dbg("checkPreviousLine: combined split statement, pline is", plineStr.trim() );
        }
    }

    // now do lines that include open & close parens, so
    //
    //   aaaaaa            <----  new pline, use this to derive indent
    //     ( ....
    //       ....
    //     ) zzzz          <----  original pline
    //
    // becomes one line:
    //
    //   aaaaaa     ( ....     ) zzzz
    //
    kpos = checkCloseParen(pline, plineStr.length);
    if( kpos != -1 ) {
        dbg("checkPreviousLine: unmatched close paren found");
        var r = document.anchor(pline, kpos, ')' );
        if(r.isValid()) {
            pline = r.line;
            plineStr = document.line(pline).replace(AdaComment, "") + plineStr;
            //dbg("checkPreviousLine: step 1, pline is", plineStr.trim() );
        }
    }

    if( /^\s*\(/.test(plineStr) ) {
	// open '(' on its own line - use previous indent
        pline = lastCodeLine( pline );
        plineStr = document.line(pline).replace(AdaComment, "") + plineStr;
        //dbg("checkPreviousLine: step 2, pline is", plineStr.trim() );
    }

    var ind = document.firstVirtualColumn(pline);

    //dbg("checkPreviousLine: ind is", ind + ", pline is", plineStr.trim() );

    if( AdaBlockStart.test( plineStr ) ||  (/\(\s*$/.test(plineStr)) ) {
        //dbg("checkPreviousLine: pline follows block start '", RegExp.$1, "'");
        // Check for false matches to AdaBlockStart
        var false_match = false;
        if( /^\s*(procedure|function|package)\b.*\bis\s+new\b/i.test(plineStr) ) {
            //dbg("checkPreviousLine: generic instantiation ignored for now");
	    false_match = true;
        }
        else if( (/\)\s*;\s*$/.test(plineStr)) || (/^([^(]*\([^)]*\))*[^(]*;\s*$/.test(plineStr)) ) {
            // matches trailing ');' or 'xxx(...) yyy;'
            //dbg("checkPreviousLine: forward declaration ignored for now");
	    false_match = true;
        }
        // Move indent in
        if ( false_match ) {
            //dbg("checkPreviousLine: block start ignored");
        } else {
            dbg("checkPreviousLine: indent after block start");
	    ind += indentWidth;
        } // if
    }
    else if( /^\s*return\b/i.test(plineStr) ) {
        // is it a return statement, or part of a function declaration?
        // NOTE: original vim script did not recognise
        //       'return' on a new line for function declarations

        var l = lastCodeLine( pline );
        while( l >= 0 ) {
           lStr = document.line(l);
            if( StatementStart.test(lStr) ) {
                dbg("checkPreviousLine: previous line is return statement");
                break;
            }
            if(/^\s*(with\s*)?function\b/i.test(lStr)) {
                // move indent back to parent 'function'
                dbg("checkPreviousLine: 'return' is function return type");
                ind  = document.firstVirtualColumn(l);
                if( ! /\s*;\s*$/.test(plineStr) ) {
                    ind +=  indentWidth;
                    //dbg("checkPreviousLine: continue function body");
                }
                break;
            }
            l = lastCodeLine(l);
        } // while
    }
    else if( /^\s*(case|exception)\b/i.test(plineStr) ) {
        dbg("checkPreviousLine: pline follows", RegExp.$1, "indenting");
        // NOTE: original vim script indented 2x
        ind = ind + indentWidth;
    }
    else if( /^\s*end\s+record\b/i.test(plineStr) ) {
        dbg("checkPreviousLine: current line follows end of record");
        // Move indent back to tallying 'type' preceeding the 'record'.
        // Allow indent to be equal to 'end record's.
        ind = MainBlockIndent( ind+indentWidth, pline, /type\b/, '' );
    }
    else if( unfStmt.test(plineStr) ) {
        dbg("checkPreviousLine: previous line is an unfinished statement");
        kpos = plineStr.indexOf(":=");
        if( kpos != -1 ) {
            kpos = document.nextNonSpaceColumn(pline, kpos+2);
        }
        if( kpos != -1 ) {
            // a := xxx ... unfinished assignment like this
            //      ^-- line up here
            ind = document.toVirtualColumn(pline, kpos);
        } else {
            // A statement continuation - move in one
            ind += indentWidth
        }
    }
    else if( unfLoop.test(plineStr) ) {
        // Multi line for/while ... loop
        ind += indentWidth;
        dbg("checkPreviousLine: ind is", ind, ", previous line is an unfinished while/for loop");
    }
    else if( /^(?!\s*end\b).*;\s*$/i.test(plineStr) ) {
        // end of statement (but not 'end' )
        // start of statement might be on a previous line
        //  - try to find current statement-start indent
        ind = StatementIndent( ind, pline )
        dbg("checkPreviousLine: ind is", ind + ", pline follows end of statement");
    } else {
        //dbg("checkPreviousLine: previous line does not influence indent");
    } // if

    return ind;

} // checkPreviousLine()


// Check current line; search for simplistic matching start-of-block
function indentLine( cline, indentWidth, newLine)
{

    // Find a non-blank code line above the current line.
    var pline = lastCodeLine( cline );
    if( pline < 0 ) {
        if( document.startsWith( cline, "--", true ) ) {
            dbg("indentLine: align comment");
            pline = document.prevNonEmptyLine( cline - 1 );
            if( pline < 0 ) return -2; // first comment line
            return -1;                 // other comments follow
        }
        dbg("indent: first code line");
        return 0;
    }

    var plineStr = document.line(pline).replace(AdaComment, "");
    //dbg("indentLine: plineStr is", plineStr.trim());

    var clineStr = document.line(cline).replace(AdaComment, "");
    //dbg("indentLine: cline is", clineStr.trim() );

    var n = checkParens( cline, indentWidth, newLine );
    if( n != -1 ) {
        //dbg("indentLine: current line aligned inside parens");
        return n;
    }
    if( /^\s*#/.test(clineStr) ) {
        dbg( "indentLine: Start of line for ada-pp" );
        return 0;
    }
    if( ((kpos = plineStr.search(/[A-Za-z0-9_.]+(\s+is)?$/)) != -1 )
             && (/^\s*\(/.test(clineStr)) ) {
        dbg("indentLine: found potential argument list");
        return document.toVirtualColumn(pline,kpos) + indentWidth;
    }

    // Get default indent from prev. line
    var pind = checkPreviousLine(pline, indentWidth);
    //dbg( "indentLine: default indent is", pind );

    if( /^\s*(begin|is)\b/i.test(clineStr) ) {
        dbg("indentLine: pind is", pind, ", found ", RegExp.$1);
        return MainBlockIndent( pind, pline, /(procedure|function|declare|package|task)\b/i, /begin\b/i )
    }
    if( /^\s*record\b/i.test(clineStr) ) {
        dbg("indentLine: line is 'record'");
        return MainBlockIndent( pind, pline, /type\b|for\b.*\buse\b/i, '' ) + indentWidth
    }
    if( /^\s*(else|elsif)\b/i.test(clineStr) ) {
        dbg("indentLine: aligning", RegExp.$1, "with corresponding 'if'");
        return MainBlockIndent( pind, pline, /if\b/, /^\s*begin\b/ )
    }
    if( /^\s*when\b/i.test(clineStr) ) {
        // Align 'when' one /in/ from matching block start
        dbg("indentLine: 'when' clause indented from parent");
        return MainBlockIndent( pind, pline, /(case|exception)\b/, /\s*begin\b/ ) + indentWidth;
    }
    if( /^\s*end\b\s*\bif\b/i.test(clineStr) ) {
        // End of if statements
        dbg("indentLine: 'end if' aligned to corresponding 'if' statement");
        return EndBlockIndent( pind, pline, /if\b/, /end\b\s*\bif\b/ )
    }
    if( /^\s*end\b\s*\bloop\b/i.test(clineStr) ) {
        dbg("indentLine: line is end of loop");
        // End of loops
        return EndBlockIndent( pind, pline, /((while|for|loop)\b)/, /end\b\s*\bloop\b/ );
    }
    if( /^\s*end\b\s*\brecord\b/i.test(clineStr) ) {
        // End of records
        dbg("indentLine: 'end record' aligned to corresponding parent");
        return EndBlockIndent( pind, pline, /(type\b.*)?\brecord\b/, /end\b\s*\brecord\b/ );
    }
    if( /^\s*end\s+procedure\b/i.test(clineStr) ) {
        dbg("indentLine: 'end procedure' aligned to corresponding parent");
        // End of procedures
        // TODO: when does 'end procedure' occur?
        // TODO: multiline procedure heading
        return EndBlockIndent( pind, pline, /procedure\b.*\bis\b/, /end\b\s*\bprocedure\b/ );
    }
    if( /^\s*end\b\s*\bcase\b/i.test(clineStr) ) {
        // NOTE: original vim script required 'is' on same line
        dbg("indentLine: 'end case' aligned to 'case'");
        return EndBlockIndent( pind, pline, /case\b[^;]*(\bis\b|$)/i, /end\b\s*\bcase\b/i );
    }
    if( /^\s*end\b/i.test( clineStr) ) {
        // General case for end
        dbg("indentLine: 'end' aligned to its parent");
        return MainBlockIndent( pind, pline, /(if|while|for|loop|accept|begin|record|case|exception|package)\b/, '' );
    }
    if( /^\s*exception\b/i.test( clineStr) ) {
        dbg("indentLine: 'exception' aligned to corresponding 'begin'");
        return MainBlockIndent( pind, pline, /begin\b/, '' );
    }
    if( /^\s*private\b/i.test( clineStr) ) {
        dbg("indentLine: 'private' aligned to corresponding 'package'");
        return MainBlockIndent( pind, pline, /package\b/, '' );
    }
    if( /^\s*<<\w+>>/.test( clineStr) ) {
        // NOTE: original vim script did not consider labels
        dbg("indentLine: 'label' aligned to corresponding 'begin'");
        return MainBlockIndent( pind, pline, /begin\b/, '' );
    }
    if( /^\s*then\b/i.test( clineStr ) ) {
        dbg("indentLine: 'then' aligned to corresponding 'if'");
        return MainBlockIndent( pind, pline, /if\b/, /begin\b/ );
    }
    if( /^\s*loop\b/i.test( clineStr ) ) {
        // is it a multiline while/for, or loop on its own?
        // search back for while/for without a loop, stop at any statement
        dbg("indentLine: 'loop' aligned either to corresponding 'for/while', or on its own");
        return MainBlockIndent( pind, pline, unfLoop, StatementStart );
    }
    if( /^\s*(function|procedure|package)\b/i.test( clineStr ) ) {
        dbg("indentLine:", RegExp.$1, " - checking for corresponding 'generic'");
        return MainBlockIndent( pind, pline, /generic\b/, /^\s*(function|procedure|package)\b/ );
    }
    if( /^\s*:=/.test( clineStr ) ) {
        dbg("indentLine: continuing assignment");
        return pind + indentWidth;
    }

    dbg("indentLine: current line has default indent");
    return pind
} // indentLine()


// Find correct indent of a new line based upon what went before
//
// we assume all previous lines are correctly indented
// indent of current line is to be determined
// arguments:
// 		cline		-- current line
//		indentWidth 	-- in spaces
//		ch		--  typed character indent
//
// returns the amount of characters (in spaces) to be indented.
// Special return values:
//   -2 = no indent
//   -1 = keep previous indent

function indent(cline, indentWidth, ch)
{
    var t = document.variable("debugMode");
    if(t) debugMode = /^(true|on|enabled|1)$/i.test( t );

    dbg("\n------------------------------------ (" + cline + ")");

    // all functions assume valid line nr on entry
    if( cline < 0 ) return -2;


    var newLine = (ch == "\n");

    if( newLine ) {
        // cr entered - align the just completed line
        view.align(new Range(cline-1, 0, cline-1, 1));
    } else if( ch == "" ) {
        if( document.firstVirtualColumn(cline) == -1 ) {
            dbg("empty line,  zero indent");
            return 0;
        }
    } else if( !reindentTrigger(cline) ) {
        return -2;
    }

    return indentLine( cline, indentWidth, newLine);

} // indent()

////////////////// end of ada.js //////////////
