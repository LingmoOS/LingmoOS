/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KREPLACE_H
#define KREPLACE_H

#include "kfind.h"

#include "ktextwidgets_export.h"

class KReplacePrivate;

/**
 * @class KReplace kreplace.h <KReplace>
 *
 * @brief A generic implementation of the "replace" function.
 *
 * @author S.R.Haque <srhaque@iee.org>, David Faure <faure@kde.org>
 *
 * \b Detail:
 *
 * This class includes prompt handling etc. Also provides some
 * static functions which can be used to create custom behavior
 * instead of using the class directly.
 *
 * \b Example:
 *
 * To use the class to implement a complete replace feature:
 *
 * In the slot connect to the replace action, after using KReplaceDialog:
 * \code
 *
 *  // This creates a replace-on-prompt dialog if needed.
 *  m_replace = new KReplace(pattern, replacement, options, this);
 *
 *  // Connect signals to code which handles highlighting of found text, and
 *  // on-the-fly replacement.
 *  connect(m_replace, &KFind::textFound, this, [this](const QString &text, int matchingIndex, int matchedLength) {
 *      slotHighlight(text, matchingIndex, matchedLength);
 *  });
 *  // Connect findNext signal - called when pressing the button in the dialog
 *  connect( m_replace, SIGNAL( findNext() ),
 *          this, SLOT( slotReplaceNext() ) );
 *  // Connect to the textReplaced() signal - emitted when a replacement is done
 *  connect( m_replace, &KReplace::textReplaced, this, [this](const QString &text, int replacementIndex, int replacedLength, int matchedLength) {
 *      slotReplace(text, replacementIndex, replacedLength, matchedLength);
 *  });
 * \endcode
 *  Then initialize the variables determining the "current position"
 *  (to the cursor, if the option FromCursor is set,
 *   to the beginning of the selection if the option SelectedText is set,
 *   and to the beginning of the document otherwise).
 *  Initialize the "end of search" variables as well (end of doc or end of selection).
 *  Swap begin and end if FindBackwards.
 *  Finally, call slotReplaceNext();
 *
 * \code
 *  void slotReplaceNext()
 *  {
 *      KFind::Result res = KFind::NoMatch;
 *      while ( res == KFind::NoMatch && <position not at end> ) {
 *          if ( m_replace->needData() )
 *              m_replace->setData( <current text fragment> );
 *
 *          // Let KReplace inspect the text fragment, and display a dialog if a match is found
 *          res = m_replace->replace();
 *
 *          if ( res == KFind::NoMatch ) {
 *              <Move to the next text fragment, honoring the FindBackwards setting for the direction>
 *          }
 *      }
 *
 *      if ( res == KFind::NoMatch ) // i.e. at end
 *          <Call either  m_replace->displayFinalDialog(); delete m_replace; m_replace = nullptr;
 *           or           if ( m_replace->shouldRestart() ) { reinit (w/o FromCursor) and call slotReplaceNext(); }
 *                        else { m_replace->closeReplaceNextDialog(); }>
 *  }
 * \endcode
 *
 *  Don't forget delete m_find in the destructor of your class,
 *  unless you gave it a parent widget on construction.
 *
 */
class KTEXTWIDGETS_EXPORT KReplace : public KFind
{
    Q_OBJECT

public:
    /**
     * Only use this constructor if you don't use KFindDialog, or if
     * you use it as a modal dialog.
     */
    KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent = nullptr);
    /**
     * This is the recommended constructor if you also use KReplaceDialog (non-modal).
     * You should pass the pointer to it here, so that when a message box
     * appears it has the right parent. Don't worry about deletion, KReplace
     * will notice if the find dialog is closed.
     */
    KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent, QWidget *replaceDialog);

    ~KReplace() override;

    /**
     * Return the number of replacements made (i.e. the number of times
     * the textReplaced() signal was emitted).
     *
     * Can be used in a dialog box to tell the user how many replacements were made.
     * The final dialog does so already, unless you used setDisplayFinalDialog(false).
     */
    int numReplacements() const;

    /**
     * Call this to reset the numMatches & numReplacements counts.
     * Can be useful if reusing the same KReplace for different operations,
     * or when restarting from the beginning of the document.
     */
    void resetCounts() override;

    /**
     * Walk the text fragment (e.g. kwrite line, kspread cell) looking for matches.
     * For each match, if prompt-on-replace is specified, emits the textFound() signal
     * and displays the prompt-for-replace dialog before doing the replace.
     */
    Result replace();

    /**
     * Return (or create) the dialog that shows the "find next?" prompt.
     * Usually you don't need to call this.
     * One case where it can be useful, is when the user selects the "Find"
     * menu item while a find operation is under way. In that case, the
     * program may want to call setActiveWindow() on that dialog.
     */
    QDialog *replaceNextDialog(bool create = false);

    /**
     * Close the "replace next?" dialog. The application should do this when
     * the last match was hit. If the application deletes the KReplace, then
     * "find previous" won't be possible anymore.
     */
    void closeReplaceNextDialog();

    /**
     * Searches the given @p text for @p pattern; if a match is found it is replaced
     * with @p replacement and the index of the replacement string is returned.
     *
     * @param text The string to search
     * @param pattern The pattern to search for
     * @param replacement The replacement string to insert into the text
     * @param index The starting index into the string
     * @param options The options to use
     * @param replacedLength Output parameter, contains the length of the replaced string
     *        Not always the same as replacement.length(), when backreferences are used
     * @return The index at which a match was found, or -1 otherwise
     */
    static int replace(QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength);

    /**
     * Returns true if we should restart the search from scratch.
     * Can ask the user, or return false (if we already searched/replaced the
     * whole document without the PromptOnReplace option).
     *
     * @param forceAsking set to true if the user modified the document during the
     * search. In that case it makes sense to restart the search again.
     *
     * @param showNumMatches set to true if the dialog should show the number of
     * matches. Set to false if the application provides a "find previous" action,
     * in which case the match count will be erroneous when hitting the end,
     * and we could even be hitting the beginning of the document (so not all
     * matches have even been seen).
     */
    bool shouldRestart(bool forceAsking = false, bool showNumMatches = true) const override;

    /**
     * Displays the final dialog telling the user how many replacements were made.
     * Call either this or shouldRestart().
     */
    void displayFinalDialog() const override;

Q_SIGNALS:
    /**
     * Connect to this signal to implement updating of replaced text during the replace
     * operation.
     *
     * Extra care must be taken to properly implement the "no prompt-on-replace" case.
     * For instance, the textFound() signal isn't emitted in that case (some code
     * might rely on it), and for performance reasons one should repaint after
     * replace() ONLY if prompt-on-replace was selected.
     *
     * @param text The text, in which the replacement has already been done
     * @param replacementIndex Starting index of the matched substring
     * @param replacedLength Length of the replacement string
     * @param matchedLength Length of the matched string
     *
     * @since 5.83
     */
    void textReplaced(const QString &text, int replacementIndex, int replacedLength, int matchedLength);

private:
    Q_DECLARE_PRIVATE(KReplace)
};
#endif
