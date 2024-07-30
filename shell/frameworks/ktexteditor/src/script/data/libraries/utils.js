/*
    Helper String Functions (copied from cstyle.js to be reused by other indenters)
    SPDX-FileCopyrightText: Alex Turbov <i.zaufi@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

//BEGIN Utils
/**
 * \brief Print or suppress debug output depending on \c debugMode variable.
 *
 * The mentioned \c debugMode supposed to be defined by a particular indenter
 * (external code).
 */
function dbg()
{
    if (debugMode)
    {
        debug.apply(this, arguments);
    }
}

/**
 * \return \c true if attribute at given position is a \e Comment
 *
 * \note C++ highlighter use \em RegionMarker for special comments,
 * so it must be counted as well...
 *
 * \todo Provide an "overload" which accepts a \c Cursor
 */
function isComment(line, column)
{
    // Check if we are not withing a comment
    var c = new Cursor(line, column);
    var mode = document.attributeName(c);
    dbg("isComment: Check mode @ " + c + ": " + mode);
    return mode.startsWith("Doxygen")
      || mode.startsWith("Alerts")
      || document.isComment(c)
      || document.isRegionMarker(c)
      ;
}

/**
 * \return \c true if attribute at given position is a \e String
 *
 * \todo Provide an "overload" which accepts a \c Cursor
 */
function isString(line, column)
{
    // Check if we are not withing a string
    var c = new Cursor(line, column);
    var mode = document.attributeName(c);
    dbg("isString: Check mode @ " + c + ": " + mode);
    return document.isString(c) || document.isChar(c);
}

/**
 * \return \c true if attribute at given position is a \e String or \e Comment
 *
 * \note C++ highlighter use \e RegionMarker for special comments,
 * soit must be counted as well...
 *
 * \todo Provide an "overload" which accepts a \c Cursor
 */
function isStringOrComment(line, column)
{
    // Check if we are not withing a string or a comment
    var c = new Cursor(line, column);
    var mode = document.attributeName(c);
    dbg("isStringOrComment: Check mode @ " + c + ": " + mode);
    return mode.startsWith("Doxygen")
      || mode.startsWith("Alerts")
      || document.isString(c)
      || document.isChar(c)
      || document.isComment(c)
      || document.isRegionMarker(c)
      ;
}

/**
 * Split a given text line by comment into parts \e before and \e after the comment
 * \return an object w/ the following fields:
 *   \li \c hasComment -- boolean: \c true if comment present on the line, \c false otherwise
 *   \li \c before -- text before the comment
 *   \li \c after -- text of the comment
 *
 * \todo Possible it would be quite reasonable to analyze a type of the comment:
 * Is it C++ or Doxygen? Is it single or w/ some text before?
 */
function splitByComment(line)
{
    var before = "";
    var after = "";
    var text = document.line(line);
    dbg("splitByComment: text='"+text+"'");

    /// \note Always ask a comment marker for Normal Text attribute
    /// of the current syntax, but not for a given line, cuz latter
    /// can be a Doxygen comment, which do not have a comment marker
    /// defined at all...
    var comment_marker = document.commentMarker(0);
    var text = document.line(line);
    var found = false;
    for (
        var pos = text.indexOf(comment_marker)
      ; pos != -1
      ; pos = text.indexOf(comment_marker, pos + 1)
      )
    {
        // Check attribute to be sure...
        if (isComment(line, pos))
        {
            // Got it!
            before = text.substring(0, pos);
            after = text.substring(pos + comment_marker.length, text.length);
            found = true;
            break;
        }
    }
    // If no comment actually found, then set text before to the original
    if (!found)
        before = text;
    dbg("splitByComment result: hasComment="+found+", before='"+before+"', after='"+after+"'");
    return {hasComment: found, before: before, after: after};
}

/**
 * \brief Remove possible comment from text
 */
function stripComment(line)
{
    var result = splitByComment(line);
    if (result.hasComment)
        return result.before.rtrim();
    return result.before.rtrim();
}

/**
 * Add a character \c c to the given position if absent.
 * Set new cursor position to the next one after the current.
 *
 * \return \c true if a desired char was added
 *
 * \todo Provide an "overload" which accepts a \c Cursor
 */
function addCharOrJumpOverIt(line, column, char)
{
    // Make sure there is a space at given position
    dbg("addCharOrJumpOverIt: checking @Cursor("+line+","+column+"), c='"+document.charAt(line, column)+"'");
    var result = false;
    if (document.lineLength(line) <= column || document.charAt(line, column) != char)
    {
        document.insertText(line, column, char);
        result = true;
    }
    /// \todo Does it really needed?
    view.setCursorPosition(line, column + 1);
    return result;
}

/**
 * Check if a character right before cursor is the very first on the line
 * and the same as a given one.
 *
 * \todo Provide an "overload" which accepts a \c Cursor
 */
function justEnteredCharIsFirstOnLine(line, column, char)
{
    return document.firstChar(line) == char && document.firstColumn(line) == (column - 1);
}
//END Utils

/**
 * \todo Unit tests! How?
 */

// kate: space-indent on; indent-width 4; replace-tabs on;
