/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2009 Paul Gideon Dann <pdgiddie@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katelayoutcache.h"
#include "kateviinputmode.h"
#include "ktexteditor/message.h"
#include <vimode/globalstate.h>
#include <vimode/inputmodemanager.h>
#include <vimode/jumps.h>
#include <vimode/lastchangerecorder.h>
#include <vimode/marks.h>
#include <vimode/modes/modebase.h>
#include <vimode/modes/normalvimode.h>
#include <vimode/modes/replacevimode.h>
#include <vimode/modes/visualvimode.h>
#include <vimode/registers.h>
#include <vimode/searcher.h>

#include <KLocalizedString>
#include <QRegularExpression>
#include <QString>

using namespace KateVi;

// TODO: the "previous word/WORD [end]" methods should be optimized. now they're being called in a
// loop and all calculations done up to finding a match are trown away when called with a count > 1
// because they will simply be called again from the last found position.
// They should take the count as a parameter and collect the positions in a QList, then return
// element (count - 1)

////////////////////////////////////////////////////////////////////////////////
// HELPER METHODS
////////////////////////////////////////////////////////////////////////////////

void ModeBase::yankToClipBoard(QChar chosen_register, const QString &text)
{
    // only yank to the clipboard if no register was specified,
    // textlength > 1 and there is something else then whitespace
    if ((chosen_register == QLatin1Char('0') || chosen_register == QLatin1Char('-') || chosen_register == PrependNumberedRegister) && text.length() > 1
        && !text.trimmed().isEmpty()) {
        KTextEditor::EditorPrivate::self()->copyToClipboard(text, m_view->doc()->url().fileName());
    }
}

bool ModeBase::deleteRange(Range &r, OperationMode mode, bool addToRegister)
{
    r.normalize();
    bool res = false;
    const QString removedText = getRange(r, mode);

    if (mode == LineWise) {
        doc()->editStart();
        for (int i = 0; i < r.endLine - r.startLine + 1; i++) {
            res = doc()->removeLine(r.startLine);
        }
        doc()->editEnd();
    } else {
        res = doc()->removeText(r.toEditorRange(), mode == Block);
    }

    // the UnnamedRegister here is only a placeholder to signify that no register was selected
    // this is needed because the fallback register depends on whether the deleted text spans a line/lines
    QChar chosenRegister = getChosenRegister(UnnamedRegister);
    if (addToRegister) {
        fillRegister(chosenRegister, removedText, mode);
    }

    const QChar lastChar = removedText.size() > 0 ? removedText.back() : QLatin1Char('\0');
    if (chosenRegister != BlackHoleRegister && (r.startLine != r.endLine || lastChar == QLatin1Char('\n') || lastChar == QLatin1Char('\r'))) {
        // for deletes spanning a line/lines, always prepend to the numbered registers
        fillRegister(PrependNumberedRegister, removedText, mode);
        chosenRegister = PrependNumberedRegister;
    } else if (chosenRegister == UnnamedRegister) {
        // only set the SmallDeleteRegister when no register was selected
        fillRegister(SmallDeleteRegister, removedText, mode);
        chosenRegister = SmallDeleteRegister;
    }
    yankToClipBoard(chosenRegister, removedText);

    return res;
}

const QString ModeBase::getRange(Range &r, OperationMode mode) const
{
    r.normalize();
    QString s;

    if (mode == LineWise) {
        r.startColumn = 0;
        r.endColumn = getLine(r.endLine).length();
    }

    if (r.motionType == InclusiveMotion) {
        r.endColumn++;
    }

    KTextEditor::Range range = r.toEditorRange();
    if (mode == LineWise) {
        s = doc()->textLines(range).join(QLatin1Char('\n'));
        s.append(QLatin1Char('\n'));
    } else if (mode == Block) {
        s = doc()->text(range, true);
    } else {
        s = doc()->text(range);
    }

    return s;
}

const QString ModeBase::getLine(int line) const
{
    return (line < 0) ? m_view->doc()->line(m_view->cursorPosition().line()) : doc()->line(line);
}

const QChar ModeBase::getCharUnderCursor() const
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    QString line = getLine(c.line());

    if (line.length() == 0 && c.column() >= line.length()) {
        return QChar::Null;
    }

    return line.at(c.column());
}

const QString ModeBase::getWordUnderCursor() const
{
    return doc()->text(getWordRangeUnderCursor());
}

const KTextEditor::Range ModeBase::getWordRangeUnderCursor() const
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    // find first character that is a “word letter” and start the search there
    QChar ch = doc()->characterAt(c);
    int i = 0;
    while (!ch.isLetterOrNumber() && !ch.isMark() && ch != QLatin1Char('_') && m_extraWordCharacters.indexOf(ch) == -1) {
        // advance cursor one position
        c.setColumn(c.column() + 1);
        if (c.column() > doc()->lineLength(c.line())) {
            c.setColumn(0);
            c.setLine(c.line() + 1);
            if (c.line() == doc()->lines()) {
                return KTextEditor::Range::invalid();
            }
        }

        ch = doc()->characterAt(c);
        i++; // count characters that were advanced so we know where to start the search
    }

    // move cursor the word (if cursor was placed on e.g. a paren, this will move
    // it to the right
    updateCursor(c);

    KTextEditor::Cursor c1 = findPrevWordStart(c.line(), c.column() + 1 + i, true);
    KTextEditor::Cursor c2 = findWordEnd(c1.line(), c1.column() + i - 1, true);
    c2.setColumn(c2.column() + 1);

    return KTextEditor::Range(c1, c2);
}

KTextEditor::Cursor ModeBase::findNextWordStart(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine(fromLine);

    // the start of word pattern need to take m_extraWordCharacters into account if defined
    QString startOfWordPattern = QStringLiteral("\\b(\\w");
    if (m_extraWordCharacters.length() > 0) {
        startOfWordPattern.append(QLatin1String("|[") + m_extraWordCharacters + QLatin1Char(']'));
    }
    startOfWordPattern.append(QLatin1Char(')'));

    const QRegularExpression startOfWord(startOfWordPattern, QRegularExpression::UseUnicodePropertiesOption); // start of a word
    static const QRegularExpression nonSpaceAfterSpace(QStringLiteral("\\s\\S"), QRegularExpression::UseUnicodePropertiesOption); // non-space right after space
    static const QRegularExpression nonWordAfterWord(
        QStringLiteral("\\b(?!\\s)\\W"),
        QRegularExpression::UseUnicodePropertiesOption); // word-boundary followed by a non-word which is not a space

    int l = fromLine;
    int c = fromColumn;

    bool found = false;

    while (!found) {
        int c1 = line.indexOf(startOfWord, c + 1);
        int c2 = line.indexOf(nonSpaceAfterSpace, c);
        int c3 = line.indexOf(nonWordAfterWord, c + 1);

        if (c1 == -1 && c2 == -1 && c3 == -1) {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor::invalid();
            } else if (l >= doc()->lines() - 1) {
                c = qMax(line.length() - 1, 0);
                return KTextEditor::Cursor::invalid();
            } else {
                c = 0;
                l++;

                line = getLine(l);

                if (line.length() == 0 || !line.at(c).isSpace()) {
                    found = true;
                }

                continue;
            }
        }

        c2++; // the second regexp will match one character *before* the character we want to go to

        if (c1 <= 0) {
            c1 = line.length() - 1;
        }
        if (c2 <= 0) {
            c2 = line.length() - 1;
        }
        if (c3 <= 0) {
            c3 = line.length() - 1;
        }

        c = qMin(c1, qMin(c2, c3));

        found = true;
    }

    return KTextEditor::Cursor(l, c);
}

KTextEditor::Cursor ModeBase::findNextWORDStart(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine();

    int l = fromLine;
    int c = fromColumn;

    bool found = false;
    static const QRegularExpression startOfWORD(QStringLiteral("\\s\\S"), QRegularExpression::UseUnicodePropertiesOption);

    while (!found) {
        c = line.indexOf(startOfWORD, c);

        if (c == -1) {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor(l, c);
            } else if (l >= doc()->lines() - 1) {
                c = line.length() - 1;
                break;
            } else {
                c = 0;
                l++;

                line = getLine(l);

                if (line.length() == 0 || !line.at(c).isSpace()) {
                    found = true;
                }

                continue;
            }
        } else {
            c++;
            found = true;
        }
    }

    return KTextEditor::Cursor(l, c);
}

KTextEditor::Cursor ModeBase::findPrevWordEnd(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine(fromLine);

    QString endOfWordPattern = QStringLiteral("\\S\\s|\\S$|\\S\\b|\\w\\W|^$");

    if (m_extraWordCharacters.length() > 0) {
        endOfWordPattern.append(QLatin1String("|[") + m_extraWordCharacters + QLatin1String("][^") + m_extraWordCharacters + QLatin1Char(']'));
    }

    const QRegularExpression endOfWord(endOfWordPattern, QRegularExpression::UseUnicodePropertiesOption);

    int l = fromLine;
    int c = fromColumn;

    bool found = false;

    while (!found) {
        int c1 = line.lastIndexOf(endOfWord, c - 1);

        if (c1 != -1 && c - 1 != -1) {
            found = true;
            c = c1;
        } else {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor::invalid();
            } else if (l > 0) {
                line = getLine(--l);
                c = line.length();

                continue;
            } else {
                return KTextEditor::Cursor::invalid();
            }
        }
    }

    return KTextEditor::Cursor(l, c);
}

KTextEditor::Cursor ModeBase::findPrevWORDEnd(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine(fromLine);

    static const QRegularExpression endOfWORDPattern(QStringLiteral("\\S\\s|\\S$|^$"), QRegularExpression::UseUnicodePropertiesOption);

    int l = fromLine;
    int c = fromColumn;

    bool found = false;

    while (!found) {
        int c1 = line.lastIndexOf(endOfWORDPattern, c - 1);

        if (c1 != -1 && c - 1 != -1) {
            found = true;
            c = c1;
        } else {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor::invalid();
            } else if (l > 0) {
                line = getLine(--l);
                c = line.length();

                continue;
            } else {
                c = 0;
                return KTextEditor::Cursor::invalid();
            }
        }
    }

    return KTextEditor::Cursor(l, c);
}

KTextEditor::Cursor ModeBase::findPrevWordStart(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine(fromLine);

    // the start of word pattern need to take m_extraWordCharacters into account if defined
    QString startOfWordPattern = QStringLiteral("\\b(\\w");
    if (m_extraWordCharacters.length() > 0) {
        startOfWordPattern.append(QLatin1String("|[") + m_extraWordCharacters + QLatin1Char(']'));
    }
    startOfWordPattern.append(QLatin1Char(')'));

    const QRegularExpression startOfWord(startOfWordPattern, QRegularExpression::UseUnicodePropertiesOption); // start of a word
    static const QRegularExpression nonSpaceAfterSpace(QStringLiteral("\\s\\S"), QRegularExpression::UseUnicodePropertiesOption); // non-space right after space
    static const QRegularExpression nonWordAfterWord(
        QStringLiteral("\\b(?!\\s)\\W"),
        QRegularExpression::UseUnicodePropertiesOption); // word-boundary followed by a non-word which is not a space
    static const QRegularExpression startOfLine(QStringLiteral("^\\S"), QRegularExpression::UseUnicodePropertiesOption); // non-space at start of line

    int l = fromLine;
    int c = fromColumn;

    bool found = false;

    while (!found) {
        int c1 = (c > 0) ? line.lastIndexOf(startOfWord, c - 1) : -1;
        int c2 = (c > 1) ? line.lastIndexOf(nonSpaceAfterSpace, c - 2) : -1;
        int c3 = (c > 0) ? line.lastIndexOf(nonWordAfterWord, c - 1) : -1;
        int c4 = (c > 0) ? line.lastIndexOf(startOfLine, c - 1) : -1;

        if (c1 == -1 && c2 == -1 && c3 == -1 && c4 == -1) {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor::invalid();
            } else if (l <= 0) {
                return KTextEditor::Cursor::invalid();
            } else {
                line = getLine(--l);
                c = line.length();

                if (line.length() == 0) {
                    c = 0;
                    found = true;
                }

                continue;
            }
        }

        c2++; // the second regexp will match one character *before* the character we want to go to

        if (c1 <= 0) {
            c1 = 0;
        }
        if (c2 <= 0) {
            c2 = 0;
        }
        if (c3 <= 0) {
            c3 = 0;
        }
        if (c4 <= 0) {
            c4 = 0;
        }

        c = qMax(c1, qMax(c2, qMax(c3, c4)));

        found = true;
    }

    return KTextEditor::Cursor(l, c);
}

KTextEditor::Cursor ModeBase::findPrevWORDStart(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine(fromLine);

    static const QRegularExpression startOfWORD(QStringLiteral("\\s\\S"), QRegularExpression::UseUnicodePropertiesOption);
    static const QRegularExpression startOfLineWORD(QStringLiteral("^\\S"), QRegularExpression::UseUnicodePropertiesOption);

    int l = fromLine;
    int c = fromColumn;

    bool found = false;

    while (!found) {
        int c1 = (c > 1) ? line.lastIndexOf(startOfWORD, c - 2) : -1;
        int c2 = (c > 0) ? line.lastIndexOf(startOfLineWORD, c - 1) : -1;

        if (c1 == -1 && c2 == -1) {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor::invalid();
            } else if (l <= 0) {
                return KTextEditor::Cursor::invalid();
            } else {
                line = getLine(--l);
                c = line.length();

                if (line.length() == 0) {
                    c = 0;
                    found = true;
                }

                continue;
            }
        }

        c1++; // the startOfWORD pattern matches one character before the word

        c = qMax(c1, c2);

        if (c <= 0) {
            c = 0;
        }

        found = true;
    }

    return KTextEditor::Cursor(l, c);
}

KTextEditor::Cursor ModeBase::findWordEnd(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine(fromLine);

    QString endOfWordPattern = QStringLiteral("\\S\\s|\\S$|\\w\\W|\\S\\b");

    if (m_extraWordCharacters.length() > 0) {
        endOfWordPattern.append(QLatin1String("|[") + m_extraWordCharacters + QLatin1String("][^") + m_extraWordCharacters + QLatin1Char(']'));
    }

    const QRegularExpression endOfWORD(endOfWordPattern, QRegularExpression::UseUnicodePropertiesOption);

    int l = fromLine;
    int c = fromColumn;

    bool found = false;

    while (!found) {
        int c1 = line.indexOf(endOfWORD, c + 1);

        if (c1 != -1) {
            found = true;
            c = c1;
        } else {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor::invalid();
            } else if (l >= doc()->lines() - 1) {
                c = line.length() - 1;
                return KTextEditor::Cursor::invalid();
            } else {
                c = -1;
                line = getLine(++l);

                continue;
            }
        }
    }

    return KTextEditor::Cursor(l, c);
}

KTextEditor::Cursor ModeBase::findWORDEnd(int fromLine, int fromColumn, bool onlyCurrentLine) const
{
    QString line = getLine(fromLine);

    static const QRegularExpression endOfWORD(QStringLiteral("\\S\\s|\\S$"), QRegularExpression::UseUnicodePropertiesOption);

    int l = fromLine;
    int c = fromColumn;

    bool found = false;

    while (!found) {
        int c1 = line.indexOf(endOfWORD, c + 1);

        if (c1 != -1) {
            found = true;
            c = c1;
        } else {
            if (onlyCurrentLine) {
                return KTextEditor::Cursor::invalid();
            } else if (l >= doc()->lines() - 1) {
                c = line.length() - 1;
                return KTextEditor::Cursor::invalid();
            } else {
                c = -1;
                line = getLine(++l);

                continue;
            }
        }
    }

    return KTextEditor::Cursor(l, c);
}

Range innerRange(Range range, bool inner)
{
    Range r = range;

    if (inner) {
        const int columnDistance = qAbs(r.startColumn - r.endColumn);
        if ((r.startLine == r.endLine) && columnDistance == 1) {
            // Start and end are right next to each other; there is nothing inside them.
            return Range::invalid();
        }
        r.startColumn++;
        r.endColumn--;
    }

    return r;
}

Range ModeBase::findSurroundingQuotes(const QChar &c, bool inner) const
{
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    Range r;
    r.startLine = cursor.line();
    r.endLine = cursor.line();

    QString line = doc()->line(cursor.line());

    // If cursor on the quote we should choose the best direction.
    if (line.at(cursor.column()) == c) {
        int attribute = m_view->doc()->kateTextLine(cursor.line()).attribute(cursor.column());

        //  If at the beginning of the line - then we might search the end.
        if (doc()->kateTextLine(cursor.line()).attribute(cursor.column() + 1) == attribute
            && doc()->kateTextLine(cursor.line()).attribute(cursor.column() - 1) != attribute) {
            r.startColumn = cursor.column();
            r.endColumn = line.indexOf(c, cursor.column() + 1);

            return innerRange(r, inner);
        }

        //  If at the end of the line - then we might search the beginning.
        if (doc()->kateTextLine(cursor.line()).attribute(cursor.column() + 1) != attribute
            && doc()->kateTextLine(cursor.line()).attribute(cursor.column() - 1) == attribute) {
            r.startColumn = line.lastIndexOf(c, cursor.column() - 1);
            r.endColumn = cursor.column();

            return innerRange(r, inner);
        }
        // Try to search the quote to right
        int c1 = line.indexOf(c, cursor.column() + 1);
        if (c1 != -1) {
            r.startColumn = cursor.column();
            r.endColumn = c1;

            return innerRange(r, inner);
        }

        // Try to search the quote to left
        int c2 = line.lastIndexOf(c, cursor.column() - 1);
        if (c2 != -1) {
            r.startColumn = c2;
            r.endColumn = cursor.column();

            return innerRange(r, inner);
        }

        // Nothing found - give up :)
        return Range::invalid();
    }

    r.startColumn = line.lastIndexOf(c, cursor.column());
    r.endColumn = line.indexOf(c, cursor.column());

    if (r.startColumn == -1 || r.endColumn == -1 || r.startColumn > r.endColumn) {
        return Range::invalid();
    }

    return innerRange(r, inner);
}

Range ModeBase::findSurroundingBrackets(const QChar &c1, const QChar &c2, bool inner, const QChar &nested1, const QChar &nested2) const
{
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    Range r(cursor, InclusiveMotion);
    int line = cursor.line();
    int column = cursor.column();
    int catalan;

    // Chars should not differ. For equal chars use findSurroundingQuotes.
    Q_ASSERT(c1 != c2);

    const QString &l = m_view->doc()->line(line);
    if (column < l.size() && l.at(column) == c2) {
        r.endLine = line;
        r.endColumn = column;
    } else {
        if (column < l.size() && l.at(column) == c1) {
            column++;
        }

        for (catalan = 1; line < m_view->doc()->lines(); line++) {
            const QString &l = m_view->doc()->line(line);

            for (; column < l.size(); column++) {
                const QChar &c = l.at(column);

                if (c == nested1) {
                    catalan++;
                } else if (c == nested2) {
                    catalan--;
                }
                if (!catalan) {
                    break;
                }
            }
            if (!catalan) {
                break;
            }
            column = 0;
        }

        if (catalan != 0) {
            return Range::invalid();
        }
        r.endLine = line;
        r.endColumn = column;
    }

    // Same algorithm but backwards.
    line = cursor.line();
    column = cursor.column();

    if (column < l.size() && l.at(column) == c1) {
        r.startLine = line;
        r.startColumn = column;
    } else {
        if (column < l.size() && l.at(column) == c2) {
            column--;
        }

        for (catalan = 1; line >= 0; line--) {
            const QString &l = m_view->doc()->line(line);

            for (; column >= 0; column--) {
                const QChar &c = l.at(column);

                if (c == nested1) {
                    catalan--;
                } else if (c == nested2) {
                    catalan++;
                }
                if (!catalan) {
                    break;
                }
            }
            if (!catalan || !line) {
                break;
            }
            column = m_view->doc()->line(line - 1).size() - 1;
        }
        if (catalan != 0) {
            return Range::invalid();
        }
        r.startColumn = column;
        r.startLine = line;
    }

    return innerRange(r, inner);
}

Range ModeBase::findSurrounding(const QRegularExpression &c1, const QRegularExpression &c2, bool inner) const
{
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    QString line = getLine();

    int col1 = line.lastIndexOf(c1, cursor.column());
    int col2 = line.indexOf(c2, cursor.column());

    Range r(cursor.line(), col1, cursor.line(), col2, InclusiveMotion);

    if (col1 == -1 || col2 == -1 || col1 > col2) {
        return Range::invalid();
    }

    if (inner) {
        r.startColumn++;
        r.endColumn--;
    }

    return r;
}

int ModeBase::findLineStartingWitchChar(const QChar &c, int count, bool forward) const
{
    int line = m_view->cursorPosition().line();
    int lines = doc()->lines();
    int hits = 0;

    if (forward) {
        line++;
    } else {
        line--;
    }

    while (line < lines && line >= 0 && hits < count) {
        QString l = getLine(line);
        if (l.length() > 0 && l.at(0) == c) {
            hits++;
        }
        if (hits != count) {
            if (forward) {
                line++;
            } else {
                line--;
            }
        }
    }

    if (hits == getCount()) {
        return line;
    }

    return -1;
}

void ModeBase::updateCursor(const KTextEditor::Cursor c) const
{
    m_viInputModeManager->updateCursor(c);
}

/**
 * @return the register given for the command. If no register was given, defaultReg is returned.
 */
QChar ModeBase::getChosenRegister(const QChar &defaultReg) const
{
    return (m_register != QChar::Null) ? m_register : defaultReg;
}

QString ModeBase::getRegisterContent(const QChar &reg)
{
    QString r = m_viInputModeManager->globalState()->registers()->getContent(reg);

    if (r.isNull()) {
        error(i18n("Nothing in register %1", reg.toLower()));
    }

    return r;
}

OperationMode ModeBase::getRegisterFlag(const QChar &reg) const
{
    return m_viInputModeManager->globalState()->registers()->getFlag(reg);
}

void ModeBase::fillRegister(const QChar &reg, const QString &text, OperationMode flag)
{
    m_viInputModeManager->globalState()->registers()->set(reg, text, flag);
}

KTextEditor::Cursor ModeBase::getNextJump(KTextEditor::Cursor cursor) const
{
    return m_viInputModeManager->jumps()->next(cursor);
}

KTextEditor::Cursor ModeBase::getPrevJump(KTextEditor::Cursor cursor) const
{
    return m_viInputModeManager->jumps()->prev(cursor);
}

Range ModeBase::goLineDown()
{
    return goLineUpDown(getCount());
}

Range ModeBase::goLineUp()
{
    return goLineUpDown(-getCount());
}

/**
 * method for moving up or down one or more lines
 * note: the sticky column is always a virtual column
 */
Range ModeBase::goLineUpDown(int lines)
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);
    int tabstop = doc()->config()->tabWidth();

    // if in an empty document, just return
    if (lines == 0) {
        return r;
    }

    r.endLine += lines;

    // limit end line to be from line 0 through the last line
    if (r.endLine < 0) {
        r.endLine = 0;
    } else if (r.endLine > doc()->lines() - 1) {
        r.endLine = doc()->lines() - 1;
    }

    Kate::TextLine startLine = doc()->plainKateTextLine(c.line());
    Kate::TextLine endLine = doc()->plainKateTextLine(r.endLine);

    int endLineLen = doc()->lineLength(r.endLine) - 1;

    if (endLineLen < 0) {
        endLineLen = 0;
    }

    int endLineLenVirt = endLine.toVirtualColumn(endLineLen, tabstop);
    int virtColumnStart = startLine.toVirtualColumn(c.column(), tabstop);

    // if sticky column isn't set, set end column and set sticky column to its virtual column
    if (m_stickyColumn == -1) {
        r.endColumn = endLine.fromVirtualColumn(virtColumnStart, tabstop);
        m_stickyColumn = virtColumnStart;
    } else {
        // sticky is set - set end column to its value
        r.endColumn = endLine.fromVirtualColumn(m_stickyColumn, tabstop);
    }

    // make sure end column won't be after the last column of a line
    if (r.endColumn > endLineLen) {
        r.endColumn = endLineLen;
    }

    // if we move to a line shorter than the current column, go to its end
    if (virtColumnStart > endLineLenVirt) {
        r.endColumn = endLineLen;
    }

    return r;
}

Range ModeBase::goVisualLineUpDown(int lines)
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);
    int tabstop = doc()->config()->tabWidth();

    if (lines == 0) {
        // We're not moving anywhere.
        return r;
    }

    KateLayoutCache *cache = m_viInputModeManager->inputAdapter()->layoutCache();

    // Work out the real and visual line pair of the beginning of the visual line we'd end up
    // on by moving lines visual lines.  We ignore the column, for now.
    int finishVisualLine = cache->viewLine(m_view->cursorPosition());
    int finishRealLine = m_view->cursorPosition().line();
    int count = qAbs(lines);
    bool invalidPos = false;
    if (lines > 0) {
        // Find the beginning of the visual line "lines" visual lines down.
        while (count > 0) {
            finishVisualLine++;
            const KateLineLayout *lineLayout = cache->line(finishRealLine);
            if (lineLayout && finishVisualLine >= lineLayout->viewLineCount()) {
                finishRealLine++;
                finishVisualLine = 0;
            }
            if (finishRealLine >= doc()->lines()) {
                invalidPos = true;
                break;
            }
            count--;
        }
    } else {
        // Find the beginning of the visual line "lines" visual lines up.
        while (count > 0) {
            finishVisualLine--;
            if (finishVisualLine < 0) {
                finishRealLine--;
                if (finishRealLine < 0) {
                    invalidPos = true;
                    break;
                }
                const auto lineLayout = cache->line(finishRealLine);
                if (!lineLayout) {
                    finishVisualLine = 0;
                    continue;
                }
                finishVisualLine = lineLayout->viewLineCount() - 1;
            }
            count--;
        }
    }
    if (invalidPos) {
        r.endLine = -1;
        r.endColumn = -1;
        return r;
    }

    // We know the final (real) line ...
    r.endLine = finishRealLine;
    // ... now work out the final (real) column.

    if (m_stickyColumn == -1 || !m_lastMotionWasVisualLineUpOrDown) {
        // Compute new sticky column. It is a *visual* sticky column.
        int startVisualLine = cache->viewLine(m_view->cursorPosition());
        int startRealLine = m_view->cursorPosition().line();
        const Kate::TextLine startLine = doc()->plainKateTextLine(c.line());
        // Adjust for the fact that if the portion of the line before wrapping is indented,
        // the continuations are also "invisibly" (i.e. without any spaces in the text itself) indented.
        const bool isWrappedContinuation = (cache->textLayout(startRealLine, startVisualLine).lineLayout().lineNumber() != 0);
        const int numInvisibleIndentChars = [&] {
            if (isWrappedContinuation) {
                auto l = doc()->plainKateTextLine(startRealLine);
                return startLine.toVirtualColumn(l.nextNonSpaceChar(0), tabstop);
            }
            return 0;
        }();

        const int realLineStartColumn = cache->textLayout(startRealLine, startVisualLine).startCol();
        const int lineStartVirtualColumn = startLine.toVirtualColumn(realLineStartColumn, tabstop);
        const int visualColumnNoInvisibleIndent = startLine.toVirtualColumn(c.column(), tabstop) - lineStartVirtualColumn;
        m_stickyColumn = visualColumnNoInvisibleIndent + numInvisibleIndentChars;
        Q_ASSERT(m_stickyColumn >= 0);
    }

    // The "real" (non-virtual) beginning of the current "line", which might be a wrapped continuation of a
    // "real" line.
    const int realLineStartColumn = cache->textLayout(finishRealLine, finishVisualLine).startCol();
    const Kate::TextLine endLine = doc()->plainKateTextLine(r.endLine);
    // Adjust for the fact that if the portion of the line before wrapping is indented,
    // the continuations are also "invisibly" (i.e. without any spaces in the text itself) indented.
    const bool isWrappedContinuation = (cache->textLayout(finishRealLine, finishVisualLine).lineLayout().lineNumber() != 0);
    const int numInvisibleIndentChars = [&] {
        if (isWrappedContinuation) {
            auto l = doc()->plainKateTextLine(finishRealLine);
            return endLine.toVirtualColumn(l.nextNonSpaceChar(0), tabstop);
        }
        return 0;
    }();
    if (m_stickyColumn == (unsigned int)KateVi::EOL) {
        const int visualEndColumn = cache->textLayout(finishRealLine, finishVisualLine).lineLayout().textLength() - 1;
        r.endColumn = endLine.fromVirtualColumn(visualEndColumn + realLineStartColumn - numInvisibleIndentChars, tabstop);
    } else {
        // Algorithm: find the "real" column corresponding to the start of the line.  Offset from that
        // until the "visual" column is equal to the "visual" sticky column.
        int realOffsetToVisualStickyColumn = 0;
        const int lineStartVirtualColumn = endLine.toVirtualColumn(realLineStartColumn, tabstop);
        while (true) {
            const int visualColumn =
                endLine.toVirtualColumn(realLineStartColumn + realOffsetToVisualStickyColumn, tabstop) - lineStartVirtualColumn + numInvisibleIndentChars;
            if (visualColumn >= m_stickyColumn) {
                break;
            }
            realOffsetToVisualStickyColumn++;
        }
        r.endColumn = realLineStartColumn + realOffsetToVisualStickyColumn;
    }
    m_currentMotionWasVisualLineUpOrDown = true;

    return r;
}

bool ModeBase::startNormalMode()
{
    /* store the key presses for this "insert mode session" so that it can be repeated with the
     * '.' command
     * - ignore transition from Visual Modes
     */
    if (!(m_viInputModeManager->isAnyVisualMode() || m_viInputModeManager->lastChangeRecorder()->isReplaying())) {
        m_viInputModeManager->storeLastChangeCommand();
        m_viInputModeManager->clearCurrentChangeLog();
    }

    m_viInputModeManager->viEnterNormalMode();
    m_view->doc()->setUndoMergeAllEdits(false);
    Q_EMIT m_view->viewModeChanged(m_view, m_view->viewMode());

    return true;
}

bool ModeBase::startInsertMode()
{
    m_viInputModeManager->viEnterInsertMode();
    m_view->doc()->setUndoMergeAllEdits(true);
    Q_EMIT m_view->viewModeChanged(m_view, m_view->viewMode());

    return true;
}

bool ModeBase::startReplaceMode()
{
    m_view->doc()->setUndoMergeAllEdits(true);
    m_viInputModeManager->viEnterReplaceMode();
    Q_EMIT m_view->viewModeChanged(m_view, m_view->viewMode());

    return true;
}

bool ModeBase::startVisualMode()
{
    if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualLineMode) {
        m_viInputModeManager->getViVisualMode()->setVisualModeType(ViMode::VisualMode);
        m_viInputModeManager->changeViMode(ViMode::VisualMode);
    } else if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualBlockMode) {
        m_viInputModeManager->getViVisualMode()->setVisualModeType(ViMode::VisualMode);
        m_viInputModeManager->changeViMode(ViMode::VisualMode);
    } else {
        m_viInputModeManager->viEnterVisualMode();
    }

    Q_EMIT m_view->viewModeChanged(m_view, m_view->viewMode());

    return true;
}

bool ModeBase::startVisualBlockMode()
{
    if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualMode) {
        m_viInputModeManager->getViVisualMode()->setVisualModeType(ViMode::VisualBlockMode);
        m_viInputModeManager->changeViMode(ViMode::VisualBlockMode);
    } else {
        m_viInputModeManager->viEnterVisualMode(ViMode::VisualBlockMode);
    }

    Q_EMIT m_view->viewModeChanged(m_view, m_view->viewMode());

    return true;
}

bool ModeBase::startVisualLineMode()
{
    if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualMode) {
        m_viInputModeManager->getViVisualMode()->setVisualModeType(ViMode::VisualLineMode);
        m_viInputModeManager->changeViMode(ViMode::VisualLineMode);
    } else {
        m_viInputModeManager->viEnterVisualMode(ViMode::VisualLineMode);
    }

    Q_EMIT m_view->viewModeChanged(m_view, m_view->viewMode());

    return true;
}

void ModeBase::error(const QString &errorMsg)
{
    delete m_infoMessage;

    m_infoMessage = new KTextEditor::Message(errorMsg, KTextEditor::Message::Error);
    m_infoMessage->setPosition(KTextEditor::Message::BottomInView);
    m_infoMessage->setAutoHide(2000); // 2 seconds
    m_infoMessage->setView(m_view);

    m_view->doc()->postMessage(m_infoMessage);
}

void ModeBase::message(const QString &msg)
{
    delete m_infoMessage;

    m_infoMessage = new KTextEditor::Message(msg, KTextEditor::Message::Positive);
    m_infoMessage->setPosition(KTextEditor::Message::BottomInView);
    m_infoMessage->setAutoHide(2000); // 2 seconds
    m_infoMessage->setView(m_view);

    m_view->doc()->postMessage(m_infoMessage);
}

QString ModeBase::getVerbatimKeys() const
{
    return m_keysVerbatim;
}

const QChar ModeBase::getCharAtVirtualColumn(const QString &line, int virtualColumn, int tabWidth)
{
    int column = 0;
    int tempCol = 0;

    // sanity check: if the line is empty, there are no chars
    if (line.length() == 0) {
        return QChar::Null;
    }

    while (tempCol < virtualColumn) {
        if (line.at(column) == QLatin1Char('\t')) {
            tempCol += tabWidth - (tempCol % tabWidth);
        } else {
            tempCol++;
        }

        if (tempCol <= virtualColumn) {
            column++;

            if (column >= line.length()) {
                return QChar::Null;
            }
        }
    }

    if (line.length() > column) {
        return line.at(column);
    }

    return QChar::Null;
}

void ModeBase::addToNumberUnderCursor(int count)
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    QString line = getLine();

    if (line.isEmpty()) {
        return;
    }

    const int cursorColumn = c.column();
    const int cursorLine = c.line();
    const KTextEditor::Cursor prevWordStart = findPrevWordStart(cursorLine, cursorColumn);
    int wordStartPos = prevWordStart.column();
    if (prevWordStart.line() < cursorLine) {
        // The previous word starts on the previous line: ignore.
        wordStartPos = 0;
    }
    if (wordStartPos > 0 && line.at(wordStartPos - 1) == QLatin1Char('-')) {
        wordStartPos--;
    }

    int numberStartPos = -1;
    QString numberAsString;
    static const QRegularExpression numberRegex(QStringLiteral("0x[0-9a-fA-F]+|\\-?\\d+"));
    auto numberMatchIter = numberRegex.globalMatch(line, wordStartPos);
    while (numberMatchIter.hasNext()) {
        const auto numberMatch = numberMatchIter.next();
        const bool numberEndedBeforeCursor = (numberMatch.capturedStart() + numberMatch.capturedLength() <= cursorColumn);
        if (!numberEndedBeforeCursor) {
            // This is the first number-like string under or after the cursor - this'll do!
            numberStartPos = numberMatch.capturedStart();
            numberAsString = numberMatch.captured();
            break;
        }
    }

    if (numberStartPos == -1) {
        // None found.
        return;
    }

    bool parsedNumberSuccessfully = false;
    int base = numberAsString.startsWith(QLatin1String("0x")) ? 16 : 10;
    if (base != 16 && numberAsString.startsWith(QLatin1Char('0')) && numberAsString.length() > 1) {
        // If a non-hex number with a leading 0 can be parsed as octal, then assume
        // it is octal.
        numberAsString.toInt(&parsedNumberSuccessfully, 8);
        if (parsedNumberSuccessfully) {
            base = 8;
        }
    }
    const int originalNumber = numberAsString.toInt(&parsedNumberSuccessfully, base);

    if (!parsedNumberSuccessfully) {
        // conversion to int failed. give up.
        return;
    }

    QString basePrefix;
    if (base == 16) {
        basePrefix = QStringLiteral("0x");
    } else if (base == 8) {
        basePrefix = QStringLiteral("0");
    }

    const int withoutBaseLength = numberAsString.length() - basePrefix.length();

    const int newNumber = originalNumber + count;

    // Create the new text string to be inserted. Prepend with “0x” if in base 16, and "0" if base 8.
    // For non-decimal numbers, try to keep the length of the number the same (including leading 0's).
    const QString newNumberPadded =
        (base == 10) ? QStringLiteral("%1").arg(newNumber, 0, base) : QStringLiteral("%1").arg(newNumber, withoutBaseLength, base, QLatin1Char('0'));
    const QString newNumberText = basePrefix + newNumberPadded;

    // Replace the old number string with the new.
    doc()->editStart();
    doc()->removeText(KTextEditor::Range(cursorLine, numberStartPos, cursorLine, numberStartPos + numberAsString.length()));
    doc()->insertText(KTextEditor::Cursor(cursorLine, numberStartPos), newNumberText);
    doc()->editEnd();
    updateCursor(KTextEditor::Cursor(m_view->cursorPosition().line(), numberStartPos + newNumberText.length() - 1));
}

void ModeBase::switchView(Direction direction)
{
    std::vector<KTextEditor::ViewPrivate *> visible_views;
    for (KTextEditor::ViewPrivate *view : KTextEditor::EditorPrivate::self()->views()) {
        if (view->isVisible()) {
            visible_views.push_back(view);
        }
    }

    QPoint current_point = m_view->mapToGlobal(m_view->pos());
    int curr_x1 = current_point.x();
    int curr_x2 = current_point.x() + m_view->width();
    int curr_y1 = current_point.y();
    int curr_y2 = current_point.y() + m_view->height();
    const KTextEditor::Cursor cursorPos = m_view->cursorPosition();
    const QPoint globalPos = m_view->mapToGlobal(m_view->cursorToCoordinate(cursorPos));
    int curr_cursor_y = globalPos.y();
    int curr_cursor_x = globalPos.x();

    KTextEditor::ViewPrivate *bestview = nullptr;
    int best_x1 = -1;
    int best_x2 = -1;
    int best_y1 = -1;
    int best_y2 = -1;
    int best_center_y = -1;
    int best_center_x = -1;

    if (direction == Next && visible_views.size() != 1) {
        for (size_t i = 0; i < visible_views.size(); i++) {
            if (visible_views.at(i) == m_view) {
                if (i != visible_views.size() - 1) {
                    bestview = visible_views.at(i + 1);
                } else {
                    bestview = visible_views.at(0);
                }
            }
        }
    } else {
        for (KTextEditor::ViewPrivate *view : visible_views) {
            QPoint point = view->mapToGlobal(view->pos());
            int x1 = point.x();
            int x2 = point.x() + view->width();
            int y1 = point.y();
            int y2 = point.y() + m_view->height();
            int center_y = (y1 + y2) / 2;
            int center_x = (x1 + x2) / 2;

            switch (direction) {
            case Left:
                if (view != m_view && x2 <= curr_x1
                    && (x2 > best_x2 || (x2 == best_x2 && qAbs(curr_cursor_y - center_y) < qAbs(curr_cursor_y - best_center_y)) || bestview == nullptr)) {
                    bestview = view;
                    best_x2 = x2;
                    best_center_y = center_y;
                }
                break;
            case Right:
                if (view != m_view && x1 >= curr_x2
                    && (x1 < best_x1 || (x1 == best_x1 && qAbs(curr_cursor_y - center_y) < qAbs(curr_cursor_y - best_center_y)) || bestview == nullptr)) {
                    bestview = view;
                    best_x1 = x1;
                    best_center_y = center_y;
                }
                break;
            case Down:
                if (view != m_view && y1 >= curr_y2
                    && (y1 < best_y1 || (y1 == best_y1 && qAbs(curr_cursor_x - center_x) < qAbs(curr_cursor_x - best_center_x)) || bestview == nullptr)) {
                    bestview = view;
                    best_y1 = y1;
                    best_center_x = center_x;
                }
                break;
            case Up:
                if (view != m_view && y2 <= curr_y1
                    && (y2 > best_y2 || (y2 == best_y2 && qAbs(curr_cursor_x - center_x) < qAbs(curr_cursor_x - best_center_x)) || bestview == nullptr)) {
                    bestview = view;
                    best_y2 = y2;
                    best_center_x = center_x;
                }
                break;
            default:
                return;
            }
        }
    }
    if (bestview != nullptr) {
        bestview->setFocus();
        bestview->setInputMode(KTextEditor::View::ViInputMode);
    }
}

Range ModeBase::motionFindPrev()
{
    Searcher *searcher = m_viInputModeManager->searcher();
    Range match = searcher->motionFindPrev(getCount());
    if (searcher->lastSearchWrapped()) {
        m_view->showSearchWrappedHint(/*isReverseSearch*/ true);
    }

    return match;
}

Range ModeBase::motionFindNext()
{
    Searcher *searcher = m_viInputModeManager->searcher();
    Range match = searcher->motionFindNext(getCount());
    if (searcher->lastSearchWrapped()) {
        m_view->showSearchWrappedHint(/*isReverseSearch*/ false);
    }

    return match;
}

void ModeBase::goToPos(const Range &r)
{
    KTextEditor::Cursor c;
    c.setLine(r.endLine);
    c.setColumn(r.endColumn);

    if (!c.isValid()) {
        return;
    }

    if (r.jump) {
        m_viInputModeManager->jumps()->add(m_view->cursorPosition());
    }

    if (c.line() >= doc()->lines()) {
        c.setLine(doc()->lines() - 1);
    }

    updateCursor(c);
}

unsigned int ModeBase::linesDisplayed() const
{
    return m_viInputModeManager->inputAdapter()->linesDisplayed();
}

void ModeBase::scrollViewLines(int l)
{
    m_viInputModeManager->inputAdapter()->scrollViewLines(l);
}

int ModeBase::getCount() const
{
    if (m_oneTimeCountOverride != -1) {
        return m_oneTimeCountOverride;
    }
    return (m_count > 0) ? m_count : 1;
}
