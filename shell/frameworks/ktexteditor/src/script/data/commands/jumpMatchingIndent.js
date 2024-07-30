var katescript = {
    "author": "Alan Prescott",
    "license": "LGPL-2.0-or-later",
    "revision": 2,
    "kate-version": "5.1",
    "functions": ["jumpIndentUp", "jumpIndentDown"],
    "actions": [
        {   "function": "jumpIndentUp",
            "name": "Move Cursor to Previous Matching Indent",
            "shortcut": "Alt+Shift+Up",
            "category": "Navigation"
        },
        {   "function": "jumpIndentDown",
            "name": "Move Cursor to Next Matching Indent",
            "shortcut": "Alt+Shift+Down",
            "category": "Navigation"
        }
    ]
}; // kate-script-header, must be at the start of the file without comments, pure json

/**
 * Move cursor to next/previous line with an equal indent - especially useful
 * for moving around blocks of Python code.
 *
 * A first attempt at a kate script created by modifying the jump.js script
 * written by Dzikri Aziz.
 * I've been looking for this feature in an editor ever since I came across it
 * in an old MS-DOS editor called Edwin.
 */

// required katepart js libraries
require ("range.js");

function jumpIndentDown() {
  return _jumpIndent();
}

function jumpIndentUp() {
  return _jumpIndent( true );
}

function help( cmd ) {
  if (cmd == 'jumpIndentUp') {
    return i18n("Move cursor to previous matching indent");
  }
  else if (cmd == 'jumpIndentDown') {
    return i18n("Move cursor to next matching indent");
  }
}

function _jumpIndent( up ) {
  var iniLine = view.cursorPosition().line,
      iniCol = view.cursorPosition().column,
      lines  = document.lines(),
      indent, target;

  indent = document.firstColumn(iniLine);
  target = iniLine;

  if ( up === true ) {
    if ( target > 0 ) {
      target--;
      while ( target > 0 && document.firstColumn(target) != indent ) {
        target--;
      }
    }
  }

  else {
    if ( target < lines ) {
      target++;
      while ( target < lines && document.firstColumn(target) != indent ) {
        target++;
      }
    }
  }

  view.setCursorPosition(target, iniCol);
}
