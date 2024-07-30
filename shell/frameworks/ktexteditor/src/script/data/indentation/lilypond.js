var katescript = {
    "name": "LilyPond",
    "author": "Wilbert Berendsen <info@wilbertberendsen.nl>",
    "license": "LGPL",
    "revision": 3,
    "kate-version": "5.1",
    "required-syntax-style": "lilypond",
    "indent-languages": ["lilypond"]
}; // kate-script-header, must be at the start of the file without comments, pure json

/*
    SPDX-FileCopyrightText: 2008 Wilbert Berendsen <info@wilbertberendsen.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// required katepart js libraries
require ("range.js");

var triggerCharacters = "}>%;";

function dbg(s) {
  // debug to the term in blue so that it's easier to make out amongst all
  // of Kate's other debug output.
  debug("\u001B[34m" + s + "\u001B[0m");
}

reCloser = /\}|\>\>/g;
reStartClosers = /^(\s*([%#]?\}|\>\>))+/
reSkipLine = /^\s*$|^%(?![{}])|^;/;
reFullCommentLine = /^\s*(;;;|%%%)/;

function indent(line, indentWidth, ch)
{
  // not necessary to indent the first line
  if (line == 0)
    return -2;

  var c = document.line(line); // current line

  // no indent for triple commented lines
  if (c.match(reFullCommentLine))
    return 0;

  // Search backwards for first non-space, non-comment line.
  var prev = line;
  while (prev--) {
    if (!document.line(prev).match(reSkipLine)) {
      var p = document.line(prev); // previous non-space line
      var prevIndent = document.firstVirtualColumn(prev);
      var pos = 0;
      var end = document.lineLength(prev);
      // Discard first closers: } and >> as they already influenced the indent.
      if (m = p.match(reStartClosers))
	var pos = m[0].length;

      var delta = 0;    // count normal openers/closers: { << } >>
      var level = 0;    // count opened Scheme parens
      var paren = [];   // save their positions
      // Walk over openers and closers in the remainder of the previous line.
      while (pos < end) {
	if (!document.isString(prev, pos)) {
	  var one = document.charAt(prev, pos);
	  var two = one + (pos+1 < end ? document.charAt(prev, pos+1) : "");
	  if (two == "%{" || two == "#{" || two == "<<")
	    ++delta, ++pos;
	  else if (two == "%}" || two == "#}" || two == ">>")
	    --delta, ++pos;
	  else if (one == "%" || one == ";")
	    break; // discard rest of line
	  else if (one == "{")
	    ++delta;
	  else if (one == "}")
	    --delta;
	  // match parens only if they are Scheme code (not LilyPond slurs)
	  else if (document.attribute(prev, pos) == 25 ||
                   document.attribute(prev, pos) == 26) {
	    if (one == "(") {
	      // save position of first
	      if (level >= 0 && paren[level] == null)
		paren[level] = pos;
	      ++level, ++delta;
	    }
	    else if (one == ")") {
	      --level, --delta;
	      // is this the final closing paren of a Scheme expression?
	      isLast = document.attribute(prev, pos) == 26
	      // have we seen an opening parenthesis in this line?
	      if (level >= 0 && paren[level] != null && !isLast) {
		delta = 0;
		prevIndent = document.toVirtualColumn(prev, paren[level]);
	      }
	      else {
		var cur = document.anchor(prev, pos, "(");
		if (cur.isValid()) {
		  delta = 0;
		  if (isLast)
		    prevIndent = document.firstVirtualColumn(cur.line);
		  else
		    prevIndent = document.toVirtualColumn(cur.line, cur.column);
		}
	      }
	    }
	  }
	}
	++pos;
      }
      // now count the number of closers in the beginning of the current line.
      if (m = c.match(reStartClosers))
	delta -= m[0].match(reCloser).length;
      return Math.max(0, prevIndent + delta * indentWidth);
    }
  }
  return 0;
}
