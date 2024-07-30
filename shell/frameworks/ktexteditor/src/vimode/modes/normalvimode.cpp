/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>
    SPDX-FileCopyrightText: 2009 Paul Gideon Dann <pdgiddie@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katebuffer.h"
#include "katecmd.h"
#include "katecompletionwidget.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "kateundomanager.h"
#include "kateviewhelpers.h"
#include "kateviewinternal.h"
#include "kateviinputmode.h"
#include <ktexteditor/attribute.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>
#include <vimode/globalstate.h>
#include <vimode/history.h>
#include <vimode/inputmodemanager.h>
#include <vimode/keymapper.h>
#include <vimode/keyparser.h>
#include <vimode/lastchangerecorder.h>
#include <vimode/macrorecorder.h>
#include <vimode/marks.h>
#include <vimode/modes/insertvimode.h>
#include <vimode/modes/normalvimode.h>
#include <vimode/modes/replacevimode.h>
#include <vimode/modes/visualvimode.h>
#include <vimode/registers.h>
#include <vimode/searcher.h>

#include <KLocalizedString>
#include <QApplication>
#include <QList>

using namespace KateVi;

NormalViMode::NormalViMode(InputModeManager *viInputModeManager, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal)
    : ModeBase()
{
    m_view = view;
    m_viewInternal = viewInternal;
    m_viInputModeManager = viInputModeManager;
    m_stickyColumn = -1;
    m_lastMotionWasVisualLineUpOrDown = false;
    m_currentMotionWasVisualLineUpOrDown = false;

    // FIXME: make configurable
    m_extraWordCharacters = QString();
    m_matchingItems[QStringLiteral("/*")] = QStringLiteral("*/");
    m_matchingItems[QStringLiteral("*/")] = QStringLiteral("-/*");

    m_matchItemRegex = generateMatchingItemRegex();

    m_scroll_count_limit = 1000; // Limit of count for scroll commands.

    m_pendingResetIsDueToExit = false;
    m_isRepeatedTFcommand = false;
    m_lastMotionWasLinewiseInnerBlock = false;
    m_motionCanChangeWholeVisualModeSelection = false;
    resetParser(); // initialise with start configuration

    m_isUndo = false;
    connect(doc()->undoManager(), &KateUndoManager::undoStart, this, &NormalViMode::undoBeginning);
    connect(doc()->undoManager(), &KateUndoManager::undoEnd, this, &NormalViMode::undoEnded);

    updateYankHighlightAttrib();
    connect(view, &KTextEditor::View::configChanged, this, &NormalViMode::updateYankHighlightAttrib);
    connect(doc(), &KTextEditor::DocumentPrivate::aboutToInvalidateMovingInterfaceContent, this, &NormalViMode::clearYankHighlight);
    connect(doc(), &KTextEditor::DocumentPrivate::aboutToDeleteMovingInterfaceContent, this, &NormalViMode::aboutToDeleteMovingInterfaceContent);
}

NormalViMode::~NormalViMode()
{
    qDeleteAll(m_highlightedYanks);
}

/**
 * parses a key stroke to check if it's a valid (part of) a command
 * @return true if a command was completed and executed, false otherwise
 */
bool NormalViMode::handleKeypress(const QKeyEvent *e)
{
    const int keyCode = e->key();

    // ignore modifier keys alone
    if (keyCode == Qt::Key_Shift || keyCode == Qt::Key_Control || keyCode == Qt::Key_Alt || keyCode == Qt::Key_Meta) {
        return false;
    }

    clearYankHighlight();

    if (keyCode == Qt::Key_Escape || (keyCode == Qt::Key_C && e->modifiers() == CONTROL_MODIFIER)
        || (keyCode == Qt::Key_BracketLeft && e->modifiers() == CONTROL_MODIFIER)) {
        m_viInputModeManager->inputAdapter()->setCaretStyle(KTextEditor::caretStyles::Block);
        m_pendingResetIsDueToExit = true;
        // Vim in weird as if we e.g. i<ctrl-o><ctrl-c> it claims (in the status bar) to still be in insert mode,
        // but behaves as if it's in normal mode. I'm treating the status bar thing as a bug and just exiting
        // insert mode altogether.
        m_viInputModeManager->setTemporaryNormalMode(false);
        reset();
        return true;
    }

    const QChar key = KeyParser::self()->KeyEventToQChar(*e);

    const QChar lastChar = m_keys.isEmpty() ? QChar::Null : m_keys.at(m_keys.size() - 1);
    const bool waitingForRegisterOrCharToSearch = this->waitingForRegisterOrCharToSearch();

    // Use replace caret when reading a character for "r"
    if (key == QLatin1Char('r') && !waitingForRegisterOrCharToSearch) {
        m_viInputModeManager->inputAdapter()->setCaretStyle(KTextEditor::caretStyles::Underline);
    }

    m_keysVerbatim.append(KeyParser::self()->decodeKeySequence(key));

    if ((keyCode >= Qt::Key_0 && keyCode <= Qt::Key_9 && lastChar != QLatin1Char('"')) // key 0-9
        && (m_countTemp != 0 || keyCode != Qt::Key_0) // first digit can't be 0
        && (!waitingForRegisterOrCharToSearch) // Not in the middle of "find char" motions or replacing char.
        && e->modifiers() == Qt::NoModifier) {
        m_countTemp *= 10;
        m_countTemp += keyCode - Qt::Key_0;

        return true;
    } else if (m_countTemp != 0) {
        m_count = getCount() * m_countTemp;
        m_countTemp = 0;
        m_iscounted = true;
    }

    m_keys.append(key);

    if (m_viInputModeManager->macroRecorder()->isRecording() && key == QLatin1Char('q')) {
        // Need to special case this "finish macro" q, as the "begin macro" q
        // needs a parameter whereas the finish macro does not.
        m_viInputModeManager->macroRecorder()->stop();
        resetParser();
        return true;
    }

    if ((key == QLatin1Char('/') || key == QLatin1Char('?')) && !waitingForRegisterOrCharToSearch) {
        // Special case for "/" and "?": these should be motions, but this is complicated by
        // the fact that the user must interact with the search bar before the range of the
        // motion can be determined.
        // We hack around this by showing the search bar immediately, and, when the user has
        // finished interacting with it, have the search bar send a "synthetic" keypresses
        // that will either abort everything (if the search was aborted) or "complete" the motion
        // otherwise.
        m_positionWhenIncrementalSearchBegan = m_view->cursorPosition();
        if (key == QLatin1Char('/')) {
            commandSearchForward();
        } else {
            commandSearchBackward();
        }
        return true;
    }

    // Special case: "cw" and "cW" work the same as "ce" and "cE" if the cursor is
    // on a non-blank.  This is because Vim interprets "cw" as change-word, and a
    // word does not include the following white space. (:help cw in vim)
    if ((m_keys == QLatin1String("cw") || m_keys == QLatin1String("cW")) && !getCharUnderCursor().isSpace()) {
        // Special case of the special case: :-)
        // If the cursor is at the end of the current word rewrite to "cl"
        const bool isWORD = (m_keys.at(1) == QLatin1Char('W'));
        const KTextEditor::Cursor currentPosition(m_view->cursorPosition());
        const KTextEditor::Cursor endOfWordOrWORD = (isWORD ? findWORDEnd(currentPosition.line(), currentPosition.column() - 1, true)
                                                            : findWordEnd(currentPosition.line(), currentPosition.column() - 1, true));

        if (currentPosition == endOfWordOrWORD) {
            m_keys = QStringLiteral("cl");
        } else {
            if (isWORD) {
                m_keys = QStringLiteral("cE");
            } else {
                m_keys = QStringLiteral("ce");
            }
        }
    }

    if (m_keys[0] == QChar(Qt::Key_QuoteDbl)) {
        if (m_keys.size() < 2) {
            return true; // waiting for a register
        } else {
            QChar r = m_keys[1].toLower();

            if ((r >= QLatin1Char('0') && r <= QLatin1Char('9')) || (r >= QLatin1Char('a') && r <= QLatin1Char('z')) || r == QLatin1Char('_')
                || r == QLatin1Char('-') || r == QLatin1Char('+') || r == QLatin1Char('*') || r == QLatin1Char('#') || r == QLatin1Char('^')) {
                m_register = m_keys[1];
                m_keys.clear();
                return true;
            } else {
                resetParser();
                return true;
            }
        }
    }

    // if we have any matching commands so far, check which ones still match
    if (!m_matchingCommands.isEmpty()) {
        int n = m_matchingCommands.size() - 1;

        // remove commands not matching anymore
        for (int i = n; i >= 0; i--) {
            if (!commands().at(m_matchingCommands.at(i)).matches(m_keys)) {
                if (commands().at(m_matchingCommands.at(i)).needsMotion()) {
                    // "cache" command needing a motion for later
                    m_motionOperatorIndex = m_matchingCommands.at(i);
                }
                m_matchingCommands.remove(i);
            }
        }

        // check if any of the matching commands need a motion/text object, if so
        // push the current command length to m_awaitingMotionOrTextObject so one
        // knows where to split the command between the operator and the motion
        for (int i = 0; i < m_matchingCommands.size(); i++) {
            if (commands().at(m_matchingCommands.at(i)).needsMotion()) {
                m_awaitingMotionOrTextObject.push(m_keys.size());
                break;
            }
        }
    } else {
        // go through all registered commands and put possible matches in m_matchingCommands
        for (size_t i = 0; i < commands().size(); i++) {
            if (commands().at(i).matches(m_keys)) {
                m_matchingCommands.push_back(i);
                if (commands().at(i).needsMotion() && commands().at(i).pattern().length() == m_keys.size()) {
                    m_awaitingMotionOrTextObject.push(m_keys.size());
                }
            }
        }
    }

    // this indicates where in the command string one should start looking for a motion command
    int checkFrom = (m_awaitingMotionOrTextObject.isEmpty() ? 0 : m_awaitingMotionOrTextObject.top());

    // Use operator-pending caret when reading a motion for an operator
    // in normal mode. We need to check that we are indeed in normal mode
    // since visual mode inherits from it.
    if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode && !m_awaitingMotionOrTextObject.isEmpty()) {
        m_viInputModeManager->inputAdapter()->setCaretStyle(KTextEditor::caretStyles::Half);
    }

    // look for matching motion commands from position 'checkFrom'
    // FIXME: if checkFrom hasn't changed, only motions whose index is in
    // m_matchingMotions should be checked
    bool motionExecuted = false;
    if (checkFrom < m_keys.size()) {
        for (size_t i = 0; i < motions().size(); i++) {
            const QString motion = m_keys.mid(checkFrom);
            if (motions().at(i).matches(motion)) {
                m_lastMotionWasLinewiseInnerBlock = false;
                m_matchingMotions.push_back(i);

                // if it matches exact, we have found the motion command to execute
                if (motions().at(i).matchesExact(motion)) {
                    m_currentMotionWasVisualLineUpOrDown = false;
                    motionExecuted = true;
                    if (checkFrom == 0) {
                        // no command given before motion, just move the cursor to wherever
                        // the motion says it should go to
                        Range r = motions().at(i).execute(this);
                        m_motionCanChangeWholeVisualModeSelection = motions().at(i).canChangeWholeVisualModeSelection();

                        if (!motions().at(i).canLandInsideFoldingRange()) {
                            // jump over folding regions since we are just moving the cursor
                            // except for motions that can end up inside ranges (e.g. n/N, f/F, %, #)
                            int currLine = m_view->cursorPosition().line();
                            int delta = r.endLine - currLine;
                            int vline = m_view->textFolding().lineToVisibleLine(currLine);
                            r.endLine = m_view->textFolding().visibleLineToLine(qMax(vline + delta, 0) /* ensure we have a valid line */);
                        }

                        if (r.endLine >= doc()->lines()) {
                            r.endLine = doc()->lines() - 1;
                        }

                        // make sure the position is valid before moving the cursor there
                        // TODO: can this be simplified? :/
                        if (r.valid && r.endLine >= 0 && (r.endLine == 0 || r.endLine <= doc()->lines() - 1) && r.endColumn >= 0) {
                            if (r.endColumn >= doc()->lineLength(r.endLine) && doc()->lineLength(r.endLine) > 0) {
                                r.endColumn = doc()->lineLength(r.endLine) - 1;
                            }

                            goToPos(r);

                            // in the case of VisualMode we need to remember the motion commands as well.
                            if (!m_viInputModeManager->isAnyVisualMode()) {
                                m_viInputModeManager->clearCurrentChangeLog();
                            }
                        } else {
                            qCDebug(LOG_KTE) << "Invalid position: (" << r.endLine << "," << r.endColumn << ")";
                        }

                        resetParser();

                        // if normal mode was started by using Ctrl-O in insert mode,
                        // it's time to go back to insert mode.
                        if (m_viInputModeManager->getTemporaryNormalMode()) {
                            startInsertMode();
                            m_viewInternal->repaint();
                        }

                        m_lastMotionWasVisualLineUpOrDown = m_currentMotionWasVisualLineUpOrDown;

                        break;
                    } else {
                        // execute the specified command and supply the position returned from
                        // the motion

                        m_commandRange = motions().at(i).execute(this);
                        m_linewiseCommand = motions().at(i).isLineWise();

                        // if we didn't get an explicit start position, use the current cursor position
                        if (m_commandRange.startLine == -1) {
                            KTextEditor::Cursor c(m_view->cursorPosition());
                            m_commandRange.startLine = c.line();
                            m_commandRange.startColumn = c.column();
                        }

                        // special case: When using the "w" motion in combination with an operator and
                        // the last word moved over is at the end of a line, the end of that word
                        // becomes the end of the operated text, not the first word in the next line.
                        if (motions().at(i).pattern() == QLatin1String("w") || motions().at(i).pattern() == QLatin1String("W")) {
                            if (m_commandRange.endLine != m_commandRange.startLine && m_commandRange.endColumn == getFirstNonBlank(m_commandRange.endLine)) {
                                m_commandRange.endLine--;
                                m_commandRange.endColumn = doc()->lineLength(m_commandRange.endLine);
                            }
                        }

                        m_commandWithMotion = true;

                        if (m_commandRange.valid) {
                            executeCommand(&commands().at(m_motionOperatorIndex));
                        } else {
                            qCDebug(LOG_KTE) << "Invalid range: "
                                             << "from (" << m_commandRange.startLine << "," << m_commandRange.startColumn << ")"
                                             << "to (" << m_commandRange.endLine << "," << m_commandRange.endColumn << ")";
                        }

                        if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode) {
                            m_viInputModeManager->inputAdapter()->setCaretStyle(KTextEditor::caretStyles::Block);
                        }
                        m_commandWithMotion = false;
                        reset();
                        break;
                    }
                }
            }
        }
    }

    if (this->waitingForRegisterOrCharToSearch()) {
        // If we are waiting for a char to search or a new register,
        // don't translate next character; we need the actual character so that e.g.
        // 'ab' is translated to 'fb' if the mappings 'a' -> 'f' and 'b' -> something else
        // exist.
        m_viInputModeManager->keyMapper()->setDoNotMapNextKeypress();
    }

    if (motionExecuted) {
        return true;
    }

    // if we have only one match, check if it is a perfect match and if so, execute it
    // if it's not waiting for a motion or a text object
    if (m_matchingCommands.size() == 1) {
        if (commands().at(m_matchingCommands.at(0)).matchesExact(m_keys) && !commands().at(m_matchingCommands.at(0)).needsMotion()) {
            if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode) {
                m_viInputModeManager->inputAdapter()->setCaretStyle(KTextEditor::caretStyles::Block);
            }

            const Command &cmd = commands().at(m_matchingCommands.at(0));
            executeCommand(&cmd);

            // check if reset() should be called. some commands in visual mode should not end visual mode
            if (cmd.shouldReset()) {
                reset();
                m_view->setBlockSelection(false);
            }
            resetParser();

            return true;
        }
    } else if (m_matchingCommands.size() == 0 && m_matchingMotions.size() == 0) {
        resetParser();
        // A bit ugly:  we haven't made use of the key event,
        // but don't want "typeable" keypresses (e.g. a, b, 3, etc) to be marked
        // as unused as they will then be added to the document, but we don't
        // want to swallow all keys in case this was a shortcut.
        // So say we made use of it if and only if it was *not* a shortcut.
        return e->type() != QEvent::ShortcutOverride;
    }

    m_matchingMotions.clear();
    return true; // TODO - need to check this - it's currently required for making tests pass, but seems odd.
}

/**
 * (re)set to start configuration. This is done when a command is completed
 * executed or when a command is aborted
 */
void NormalViMode::resetParser()
{
    m_keys.clear();
    m_keysVerbatim.clear();
    m_count = 0;
    m_oneTimeCountOverride = -1;
    m_iscounted = false;
    m_countTemp = 0;
    m_register = QChar::Null;
    m_findWaitingForChar = false;
    m_matchingCommands.clear();
    m_matchingMotions.clear();
    m_awaitingMotionOrTextObject.clear();
    m_motionOperatorIndex = 0;

    m_commandWithMotion = false;
    m_linewiseCommand = true;
    m_deleteCommand = false;

    m_commandShouldKeepSelection = false;

    m_currentChangeEndMarker = KTextEditor::Cursor::invalid();

    if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode) {
        m_viInputModeManager->inputAdapter()->setCaretStyle(KTextEditor::caretStyles::Block);
    }
}

// reset the command parser
void NormalViMode::reset()
{
    resetParser();
    m_commandRange.startLine = -1;
    m_commandRange.startColumn = -1;
}

void NormalViMode::beginMonitoringDocumentChanges()
{
    connect(doc(), &KTextEditor::DocumentPrivate::textInsertedRange, this, &NormalViMode::textInserted);
    connect(doc(), &KTextEditor::DocumentPrivate::textRemoved, this, &NormalViMode::textRemoved);
}

void NormalViMode::executeCommand(const Command *cmd)
{
    const ViMode originalViMode = m_viInputModeManager->getCurrentViMode();

    cmd->execute(this);

    // if normal mode was started by using Ctrl-O in insert mode,
    // it's time to go back to insert mode.
    if (m_viInputModeManager->getTemporaryNormalMode()) {
        startInsertMode();
        m_viewInternal->repaint();
    }

    // if the command was a change, and it didn't enter insert mode, store the key presses so that
    // they can be repeated with '.'
    if (m_viInputModeManager->getCurrentViMode() != ViMode::InsertMode && m_viInputModeManager->getCurrentViMode() != ViMode::ReplaceMode) {
        if (cmd->isChange() && !m_viInputModeManager->lastChangeRecorder()->isReplaying()) {
            m_viInputModeManager->storeLastChangeCommand();
        }

        // when we transition to visual mode, remember the command in the keys history (V, v, ctrl-v, ...)
        // this will later result in buffer filled with something like this "Vjj>" which we can use later with repeat "."
        const bool commandSwitchedToVisualMode = ((originalViMode == ViMode::NormalMode) && m_viInputModeManager->isAnyVisualMode());
        if (!commandSwitchedToVisualMode) {
            m_viInputModeManager->clearCurrentChangeLog();
        }
    }

    // make sure the cursor does not end up after the end of the line
    KTextEditor::Cursor c(m_view->cursorPosition());
    if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode) {
        int lineLength = doc()->lineLength(c.line());

        if (c.column() >= lineLength) {
            if (lineLength == 0) {
                c.setColumn(0);
            } else {
                c.setColumn(lineLength - 1);
            }
        }
        updateCursor(c);
    }
}

////////////////////////////////////////////////////////////////////////////////
// COMMANDS AND OPERATORS
////////////////////////////////////////////////////////////////////////////////

/**
 * enter insert mode at the cursor position
 */

bool NormalViMode::commandEnterInsertMode()
{
    m_stickyColumn = -1;
    m_viInputModeManager->getViInsertMode()->setCount(getCount());
    return startInsertMode();
}

/**
 * enter insert mode after the current character
 */

bool NormalViMode::commandEnterInsertModeAppend()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    c.setColumn(c.column() + 1);

    // if empty line, the cursor should start at column 0
    if (doc()->lineLength(c.line()) == 0) {
        c.setColumn(0);
    }

    // cursor should never be in a column > number of columns
    if (c.column() > doc()->lineLength(c.line())) {
        c.setColumn(doc()->lineLength(c.line()));
    }

    updateCursor(c);

    m_stickyColumn = -1;
    m_viInputModeManager->getViInsertMode()->setCount(getCount());
    return startInsertMode();
}

/**
 * start insert mode after the last character of the line
 */

bool NormalViMode::commandEnterInsertModeAppendEOL()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    c.setColumn(doc()->lineLength(c.line()));
    updateCursor(c);

    m_stickyColumn = -1;
    m_viInputModeManager->getViInsertMode()->setCount(getCount());
    return startInsertMode();
}

bool NormalViMode::commandEnterInsertModeBeforeFirstNonBlankInLine()
{
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    int c = getFirstNonBlank();

    cursor.setColumn(c);
    updateCursor(cursor);

    m_stickyColumn = -1;
    m_viInputModeManager->getViInsertMode()->setCount(getCount());
    return startInsertMode();
}

/**
 * enter insert mode at the last insert position
 */

bool NormalViMode::commandEnterInsertModeLast()
{
    KTextEditor::Cursor c = m_viInputModeManager->marks()->getInsertStopped();
    if (c.isValid()) {
        updateCursor(c);
    }

    m_stickyColumn = -1;
    return startInsertMode();
}

bool NormalViMode::commandEnterVisualLineMode()
{
    if (m_viInputModeManager->getCurrentViMode() == VisualLineMode) {
        reset();
        return true;
    }

    return startVisualLineMode();
}

bool NormalViMode::commandEnterVisualBlockMode()
{
    if (m_viInputModeManager->getCurrentViMode() == VisualBlockMode) {
        reset();
        return true;
    }

    return startVisualBlockMode();
}

bool NormalViMode::commandReselectVisual()
{
    // start last visual mode and set start = `< and cursor = `>
    KTextEditor::Cursor c1 = m_viInputModeManager->marks()->getSelectionStart();
    KTextEditor::Cursor c2 = m_viInputModeManager->marks()->getSelectionFinish();

    // we should either get two valid cursors or two invalid cursors
    Q_ASSERT(c1.isValid() == c2.isValid());

    if (c1.isValid() && c2.isValid()) {
        m_viInputModeManager->getViVisualMode()->setStart(c1);
        bool returnValue = false;

        switch (m_viInputModeManager->getViVisualMode()->getLastVisualMode()) {
        case ViMode::VisualMode:
            returnValue = commandEnterVisualMode();
            break;
        case ViMode::VisualLineMode:
            returnValue = commandEnterVisualLineMode();
            break;
        case ViMode::VisualBlockMode:
            returnValue = commandEnterVisualBlockMode();
            break;
        default:
            Q_ASSERT("invalid visual mode");
        }
        m_viInputModeManager->getViVisualMode()->goToPos(c2);
        return returnValue;
    } else {
        error(QStringLiteral("No previous visual selection"));
    }

    return false;
}

bool NormalViMode::commandEnterVisualMode()
{
    if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualMode) {
        reset();
        return true;
    }

    return startVisualMode();
}

bool NormalViMode::commandToOtherEnd()
{
    if (m_viInputModeManager->isAnyVisualMode()) {
        m_viInputModeManager->getViVisualMode()->switchStartEnd();
        return true;
    }

    return false;
}

bool NormalViMode::commandEnterReplaceMode()
{
    m_stickyColumn = -1;
    m_viInputModeManager->getViReplaceMode()->setCount(getCount());
    return startReplaceMode();
}

bool NormalViMode::commandDeleteLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    Range r;

    r.startLine = c.line();
    r.endLine = c.line() + getCount() - 1;

    int column = c.column();

    bool ret = deleteRange(r, LineWise);

    c = m_view->cursorPosition();
    if (column > doc()->lineLength(c.line()) - 1) {
        column = doc()->lineLength(c.line()) - 1;
    }
    if (column < 0) {
        column = 0;
    }

    if (c.line() > doc()->lines() - 1) {
        c.setLine(doc()->lines() - 1);
    }

    c.setColumn(column);
    m_stickyColumn = -1;
    updateCursor(c);

    m_deleteCommand = true;
    return ret;
}

bool NormalViMode::commandDelete()
{
    m_deleteCommand = true;
    return deleteRange(m_commandRange, getOperationMode());
}

bool NormalViMode::commandDeleteToEOL()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    OperationMode m = CharWise;

    m_commandRange.endColumn = KateVi::EOL;
    switch (m_viInputModeManager->getCurrentViMode()) {
    case ViMode::NormalMode:
        m_commandRange.startLine = c.line();
        m_commandRange.startColumn = c.column();
        m_commandRange.endLine = c.line() + getCount() - 1;
        break;
    case ViMode::VisualMode:
    case ViMode::VisualLineMode:
        m = LineWise;
        break;
    case ViMode::VisualBlockMode:
        m_commandRange.normalize();
        m = Block;
        break;
    default:
        /* InsertMode and ReplaceMode will never call this method. */
        Q_ASSERT(false);
    }

    bool r = deleteRange(m_commandRange, m);

    switch (m) {
    case CharWise:
        c.setColumn(doc()->lineLength(c.line()) - 1);
        break;
    case LineWise:
        c.setLine(m_commandRange.startLine);
        c.setColumn(getFirstNonBlank(qMin(doc()->lastLine(), m_commandRange.startLine)));
        break;
    case Block:
        c.setLine(m_commandRange.startLine);
        c.setColumn(m_commandRange.startColumn - 1);
        break;
    }

    // make sure cursor position is valid after deletion
    if (c.line() < 0) {
        c.setLine(0);
    }
    if (c.line() > doc()->lastLine()) {
        c.setLine(doc()->lastLine());
    }
    if (c.column() > doc()->lineLength(c.line()) - 1) {
        c.setColumn(doc()->lineLength(c.line()) - 1);
    }
    if (c.column() < 0) {
        c.setColumn(0);
    }

    updateCursor(c);

    m_deleteCommand = true;
    return r;
}

bool NormalViMode::commandMakeLowercase()
{
    KTextEditor::Cursor c = m_view->cursorPosition();

    OperationMode m = getOperationMode();
    QString text = getRange(m_commandRange, m);
    if (m == LineWise) {
        text.chop(1); // don't need '\n' at the end;
    }
    QString lowerCase = text.toLower();

    m_commandRange.normalize();
    KTextEditor::Cursor start(m_commandRange.startLine, m_commandRange.startColumn);
    KTextEditor::Cursor end(m_commandRange.endLine, m_commandRange.endColumn);
    KTextEditor::Range range(start, end);

    doc()->replaceText(range, lowerCase, m == Block);

    if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode) {
        updateCursor(start);
    } else {
        updateCursor(c);
    }

    return true;
}

bool NormalViMode::commandMakeLowercaseLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    if (doc()->lineLength(c.line()) == 0) {
        // Nothing to do.
        return true;
    }

    m_commandRange.startLine = c.line();
    m_commandRange.endLine = c.line() + getCount() - 1;
    m_commandRange.startColumn = 0;
    m_commandRange.endColumn = doc()->lineLength(c.line()) - 1;

    return commandMakeLowercase();
}

bool NormalViMode::commandMakeUppercase()
{
    if (!m_commandRange.valid) {
        return false;
    }
    KTextEditor::Cursor c = m_view->cursorPosition();
    OperationMode m = getOperationMode();
    QString text = getRange(m_commandRange, m);
    if (m == LineWise) {
        text.chop(1); // don't need '\n' at the end;
    }
    QString upperCase = text.toUpper();

    m_commandRange.normalize();
    KTextEditor::Cursor start(m_commandRange.startLine, m_commandRange.startColumn);
    KTextEditor::Cursor end(m_commandRange.endLine, m_commandRange.endColumn);
    KTextEditor::Range range(start, end);

    doc()->replaceText(range, upperCase, m == Block);
    if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode) {
        updateCursor(start);
    } else {
        updateCursor(c);
    }

    return true;
}

bool NormalViMode::commandMakeUppercaseLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    if (doc()->lineLength(c.line()) == 0) {
        // Nothing to do.
        return true;
    }

    m_commandRange.startLine = c.line();
    m_commandRange.endLine = c.line() + getCount() - 1;
    m_commandRange.startColumn = 0;
    m_commandRange.endColumn = doc()->lineLength(c.line()) - 1;

    return commandMakeUppercase();
}

bool NormalViMode::commandChangeCase()
{
    QString text;
    KTextEditor::Range range;
    KTextEditor::Cursor c(m_view->cursorPosition());

    // in visual mode, the range is from start position to end position...
    if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualMode || m_viInputModeManager->getCurrentViMode() == ViMode::VisualBlockMode) {
        KTextEditor::Cursor c2 = m_viInputModeManager->getViVisualMode()->getStart();

        if (c2 > c) {
            c2.setColumn(c2.column() + 1);
        } else {
            c.setColumn(c.column() + 1);
        }

        range.setRange(c, c2);
        // ... in visual line mode, the range is from column 0 on the first line to
        // the line length of the last line...
    } else if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualLineMode) {
        KTextEditor::Cursor c2 = m_viInputModeManager->getViVisualMode()->getStart();

        if (c2 > c) {
            c2.setColumn(doc()->lineLength(c2.line()));
            c.setColumn(0);
        } else {
            c.setColumn(doc()->lineLength(c.line()));
            c2.setColumn(0);
        }

        range.setRange(c, c2);
        // ... and in normal mode the range is from the current position to the
        // current position + count
    } else {
        KTextEditor::Cursor c2 = c;
        c2.setColumn(c.column() + getCount());

        if (c2.column() > doc()->lineLength(c.line())) {
            c2.setColumn(doc()->lineLength(c.line()));
        }

        range.setRange(c, c2);
    }

    bool block = m_viInputModeManager->getCurrentViMode() == ViMode::VisualBlockMode;

    // get the text the command should operate on
    text = doc()->text(range, block);

    // for every character, switch its case
    for (int i = 0; i < text.length(); i++) {
        if (text.at(i).isUpper()) {
            text[i] = text.at(i).toLower();
        } else if (text.at(i).isLower()) {
            text[i] = text.at(i).toUpper();
        }
    }

    // replace the old text with the modified text
    doc()->replaceText(range, text, block);

    // in normal mode, move the cursor to the right, in visual mode move the
    // cursor to the start of the selection
    if (m_viInputModeManager->getCurrentViMode() == ViMode::NormalMode) {
        updateCursor(range.end());
    } else {
        updateCursor(range.start());
    }

    return true;
}

bool NormalViMode::commandChangeCaseRange()
{
    OperationMode m = getOperationMode();
    QString changedCase = getRange(m_commandRange, m);
    if (m == LineWise) {
        changedCase.chop(1); // don't need '\n' at the end;
    }
    KTextEditor::Range range = KTextEditor::Range(m_commandRange.startLine, m_commandRange.startColumn, m_commandRange.endLine, m_commandRange.endColumn);
    // get the text the command should operate on
    // for every character, switch its case
    for (int i = 0; i < changedCase.length(); i++) {
        if (changedCase.at(i).isUpper()) {
            changedCase[i] = changedCase.at(i).toLower();
        } else if (changedCase.at(i).isLower()) {
            changedCase[i] = changedCase.at(i).toUpper();
        }
    }
    doc()->replaceText(range, changedCase, m == Block);
    return true;
}

bool NormalViMode::commandChangeCaseLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    if (doc()->lineLength(c.line()) == 0) {
        // Nothing to do.
        return true;
    }

    m_commandRange.startLine = c.line();
    m_commandRange.endLine = c.line() + getCount() - 1;
    m_commandRange.startColumn = 0;
    m_commandRange.endColumn = doc()->lineLength(c.line()) - 1; // -1 is for excluding '\0'

    if (!commandChangeCaseRange()) {
        return false;
    }

    KTextEditor::Cursor start(m_commandRange.startLine, m_commandRange.startColumn);
    if (getCount() > 1) {
        updateCursor(c);
    } else {
        updateCursor(start);
    }
    return true;
}

bool NormalViMode::commandOpenNewLineUnder()
{
    doc()->setUndoMergeAllEdits(true);

    KTextEditor::Cursor c(m_view->cursorPosition());

    c.setColumn(doc()->lineLength(c.line()));
    updateCursor(c);

    doc()->newLine(m_view);

    m_stickyColumn = -1;
    startInsertMode();
    m_viInputModeManager->getViInsertMode()->setCount(getCount());
    m_viInputModeManager->getViInsertMode()->setCountedRepeatsBeginOnNewLine(true);

    return true;
}

bool NormalViMode::commandOpenNewLineOver()
{
    doc()->setUndoMergeAllEdits(true);

    KTextEditor::Cursor c(m_view->cursorPosition());

    if (c.line() == 0) {
        doc()->insertLine(0, QString());
        c.setColumn(0);
        c.setLine(0);
        updateCursor(c);
    } else {
        c.setLine(c.line() - 1);
        c.setColumn(getLine(c.line()).length());
        updateCursor(c);
        doc()->newLine(m_view);
    }

    m_stickyColumn = -1;
    startInsertMode();
    m_viInputModeManager->getViInsertMode()->setCount(getCount());
    m_viInputModeManager->getViInsertMode()->setCountedRepeatsBeginOnNewLine(true);

    return true;
}

bool NormalViMode::commandJoinLines()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    unsigned int from = c.line();
    unsigned int to = c.line() + ((getCount() == 1) ? 1 : getCount() - 1);

    // if we were given a range of lines, this information overrides the previous
    if (m_commandRange.startLine != -1 && m_commandRange.endLine != -1) {
        m_commandRange.normalize();
        c.setLine(m_commandRange.startLine);
        from = m_commandRange.startLine;
        to = m_commandRange.endLine;
    }

    if (to >= (unsigned int)doc()->lines()) {
        return false;
    }

    bool nonEmptyLineFound = false;
    for (unsigned int lineNum = from; lineNum <= to; lineNum++) {
        if (!doc()->line(lineNum).isEmpty()) {
            nonEmptyLineFound = true;
        }
    }

    const int firstNonWhitespaceOnLastLine = doc()->kateTextLine(to).firstChar();
    QString leftTrimmedLastLine;
    if (firstNonWhitespaceOnLastLine != -1) {
        leftTrimmedLastLine = doc()->line(to).mid(firstNonWhitespaceOnLastLine);
    }

    joinLines(from, to);

    if (nonEmptyLineFound && leftTrimmedLastLine.isEmpty()) {
        // joinLines won't have added a trailing " ", whereas Vim does - follow suit.
        doc()->insertText(KTextEditor::Cursor(from, doc()->lineLength(from)), QStringLiteral(" "));
    }

    // Position cursor just before first non-whitesspace character of what was the last line joined.
    c.setColumn(doc()->lineLength(from) - leftTrimmedLastLine.length() - 1);
    if (c.column() >= 0) {
        updateCursor(c);
    }

    m_deleteCommand = true;
    return true;
}

bool NormalViMode::commandChange()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    OperationMode m = getOperationMode();

    doc()->setUndoMergeAllEdits(true);

    commandDelete();

    if (m == LineWise) {
        // if we deleted several lines, insert an empty line and put the cursor there.
        doc()->insertLine(m_commandRange.startLine, QString());
        c.setLine(m_commandRange.startLine);
        c.setColumn(0);
    } else if (m == Block) {
        // block substitute can be simulated by first deleting the text
        // (done above) and then starting block prepend.
        return commandPrependToBlock();
    } else {
        if (m_commandRange.startLine < m_commandRange.endLine) {
            c.setLine(m_commandRange.startLine);
        }
        c.setColumn(m_commandRange.startColumn);
    }

    updateCursor(c);
    setCount(0); // The count was for the motion, not the insertion.
    commandEnterInsertMode();

    // correct indentation level
    if (m == LineWise) {
        m_view->align();
    }

    m_deleteCommand = true;
    return true;
}

bool NormalViMode::commandChangeToEOL()
{
    commandDeleteToEOL();

    if (getOperationMode() == Block) {
        return commandPrependToBlock();
    }

    m_deleteCommand = true;
    return commandEnterInsertModeAppend();
}

bool NormalViMode::commandChangeLine()
{
    m_deleteCommand = true;
    KTextEditor::Cursor c(m_view->cursorPosition());
    c.setColumn(0);
    updateCursor(c);

    doc()->setUndoMergeAllEdits(true);

    // if count >= 2 start by deleting the whole lines
    if (getCount() >= 2) {
        Range r(c.line(), 0, c.line() + getCount() - 2, 0, InclusiveMotion);
        deleteRange(r);
    }

    // ... then delete the _contents_ of the last line, but keep the line
    Range r(c.line(), c.column(), c.line(), doc()->lineLength(c.line()) - 1, InclusiveMotion);
    deleteRange(r, CharWise, true);

    // ... then enter insert mode
    if (getOperationMode() == Block) {
        return commandPrependToBlock();
    }
    commandEnterInsertModeAppend();

    // correct indentation level
    m_view->align();

    return true;
}

bool NormalViMode::commandSubstituteChar()
{
    if (commandDeleteChar()) {
        // The count is only used for deletion of chars; the inserted text is not repeated
        setCount(0);
        return commandEnterInsertMode();
    }

    m_deleteCommand = true;
    return false;
}

bool NormalViMode::commandSubstituteLine()
{
    m_deleteCommand = true;
    return commandChangeLine();
}

bool NormalViMode::commandYank()
{
    bool r = false;
    QString yankedText;

    OperationMode m = getOperationMode();
    yankedText = getRange(m_commandRange, m);

    highlightYank(m_commandRange, m);

    QChar chosen_register = getChosenRegister(ZeroRegister);
    fillRegister(chosen_register, yankedText, m);
    yankToClipBoard(chosen_register, yankedText);

    return r;
}

bool NormalViMode::commandYankLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    QString lines;
    int linenum = c.line();

    for (int i = 0; i < getCount(); i++) {
        lines.append(getLine(linenum + i) + QLatin1Char('\n'));
    }

    Range yankRange(linenum, 0, linenum + getCount() - 1, getLine(linenum + getCount() - 1).length(), InclusiveMotion);
    highlightYank(yankRange);

    QChar chosen_register = getChosenRegister(ZeroRegister);
    fillRegister(chosen_register, lines, LineWise);
    yankToClipBoard(chosen_register, lines);

    return true;
}

bool NormalViMode::commandYankToEOL()
{
    OperationMode m = CharWise;
    KTextEditor::Cursor c(m_view->cursorPosition());

    MotionType motion = m_commandRange.motionType;
    m_commandRange.endLine = c.line() + getCount() - 1;
    m_commandRange.endColumn = doc()->lineLength(m_commandRange.endLine) - 1;
    m_commandRange.motionType = InclusiveMotion;

    switch (m_viInputModeManager->getCurrentViMode()) {
    case ViMode::NormalMode:
        m_commandRange.startLine = c.line();
        m_commandRange.startColumn = c.column();
        break;
    case ViMode::VisualMode:
    case ViMode::VisualLineMode:
        m = LineWise;
        {
            VisualViMode *visual = static_cast<VisualViMode *>(this);
            visual->setStart(KTextEditor::Cursor(visual->getStart().line(), 0));
        }
        break;
    case ViMode::VisualBlockMode:
        m = Block;
        break;
    default:
        /* InsertMode and ReplaceMode will never call this method. */
        Q_ASSERT(false);
    }

    const QString &yankedText = getRange(m_commandRange, m);
    m_commandRange.motionType = motion;
    highlightYank(m_commandRange);

    QChar chosen_register = getChosenRegister(ZeroRegister);
    fillRegister(chosen_register, yankedText, m);
    yankToClipBoard(chosen_register, yankedText);

    return true;
}

// Insert the text in the given register after the cursor position.
// This is the non-g version of paste, so the cursor will usually
// end up on the last character of the pasted text, unless the text
// was multi-line or linewise in which case it will end up
// on the *first* character of the pasted text(!)
// If linewise, will paste after the current line.
bool NormalViMode::commandPaste()
{
    return paste(AfterCurrentPosition, false, false);
}

// As with commandPaste, except that the text is pasted *at* the cursor position
bool NormalViMode::commandPasteBefore()
{
    return paste(AtCurrentPosition, false, false);
}

// As with commandPaste, except that the cursor will generally be placed *after* the
// last pasted character (assuming the last pasted character is not at  the end of the line).
// If linewise, cursor will be at the beginning of the line *after* the last line of pasted text,
// unless that line is the last line of the document; then it will be placed at the beginning of the
// last line pasted.
bool NormalViMode::commandgPaste()
{
    return paste(AfterCurrentPosition, true, false);
}

// As with commandgPaste, except that it pastes *at* the current cursor position or, if linewise,
// at the current line.
bool NormalViMode::commandgPasteBefore()
{
    return paste(AtCurrentPosition, true, false);
}

bool NormalViMode::commandIndentedPaste()
{
    return paste(AfterCurrentPosition, false, true);
}

bool NormalViMode::commandIndentedPasteBefore()
{
    return paste(AtCurrentPosition, false, true);
}

bool NormalViMode::commandDeleteChar()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c.line(), c.column(), c.line(), c.column() + getCount(), ExclusiveMotion);

    if (m_commandRange.startLine != -1 && m_commandRange.startColumn != -1) {
        r = m_commandRange;
    } else {
        if (r.endColumn > doc()->lineLength(r.startLine)) {
            r.endColumn = doc()->lineLength(r.startLine);
        }
    }

    // should delete entire lines if in visual line mode and selection in visual block mode
    OperationMode m = CharWise;
    if (m_viInputModeManager->getCurrentViMode() == VisualLineMode) {
        m = LineWise;
    } else if (m_viInputModeManager->getCurrentViMode() == VisualBlockMode) {
        m = Block;
    }

    m_deleteCommand = true;
    return deleteRange(r, m);
}

bool NormalViMode::commandDeleteCharBackward()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    Range r(c.line(), c.column() - getCount(), c.line(), c.column(), ExclusiveMotion);

    if (m_commandRange.startLine != -1 && m_commandRange.startColumn != -1) {
        r = m_commandRange;
    } else {
        if (r.startColumn < 0) {
            r.startColumn = 0;
        }
    }

    // should delete entire lines if in visual line mode and selection in visual block mode
    OperationMode m = CharWise;
    if (m_viInputModeManager->getCurrentViMode() == VisualLineMode) {
        m = LineWise;
    } else if (m_viInputModeManager->getCurrentViMode() == VisualBlockMode) {
        m = Block;
    }

    m_deleteCommand = true;
    return deleteRange(r, m);
}

bool NormalViMode::commandReplaceCharacter()
{
    QString key = KeyParser::self()->decodeKeySequence(m_keys.right(1));

    // Filter out some special keys.
    const int keyCode = KeyParser::self()->encoded2qt(m_keys.right(1));
    switch (keyCode) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Delete:
    case Qt::Key_Insert:
    case Qt::Key_Backspace:
    case Qt::Key_CapsLock:
        return true;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        key = QStringLiteral("\n");
    }

    bool r;
    if (m_viInputModeManager->isAnyVisualMode()) {
        OperationMode m = getOperationMode();
        QString text = getRange(m_commandRange, m);

        if (m == LineWise) {
            text.chop(1); // don't need '\n' at the end;
        }

        static const QRegularExpression nonNewlineRegex(QStringLiteral("[^\n]"));
        text.replace(nonNewlineRegex, key);

        m_commandRange.normalize();
        KTextEditor::Cursor start(m_commandRange.startLine, m_commandRange.startColumn);
        KTextEditor::Cursor end(m_commandRange.endLine, m_commandRange.endColumn);
        KTextEditor::Range range(start, end);

        r = doc()->replaceText(range, text, m == Block);

    } else {
        KTextEditor::Cursor c1(m_view->cursorPosition());
        KTextEditor::Cursor c2(m_view->cursorPosition());

        c2.setColumn(c2.column() + getCount());

        if (c2.column() > doc()->lineLength(m_view->cursorPosition().line())) {
            return false;
        }

        r = doc()->replaceText(KTextEditor::Range(c1, c2), key.repeated(getCount()));
        updateCursor(c1);
    }
    return r;
}

bool NormalViMode::commandSwitchToCmdLine()
{
    QString initialText;
    if (m_viInputModeManager->isAnyVisualMode()) {
        // if in visual mode, make command range == visual selection
        m_viInputModeManager->getViVisualMode()->saveRangeMarks();
        initialText = QStringLiteral("'<,'>");
    } else if (getCount() != 1) {
        // if a count is given, the range [current line] to [current line] +
        // count should be prepended to the command line
        initialText = QLatin1String(".,.+") + QString::number(getCount() - 1);
    }

    m_viInputModeManager->inputAdapter()->showViModeEmulatedCommandBar();
    m_viInputModeManager->inputAdapter()->viModeEmulatedCommandBar()->init(EmulatedCommandBar::Command, initialText);

    m_commandShouldKeepSelection = true;

    return true;
}

bool NormalViMode::commandSearchBackward()
{
    m_viInputModeManager->inputAdapter()->showViModeEmulatedCommandBar();
    m_viInputModeManager->inputAdapter()->viModeEmulatedCommandBar()->init(EmulatedCommandBar::SearchBackward);
    return true;
}

bool NormalViMode::commandSearchForward()
{
    m_viInputModeManager->inputAdapter()->showViModeEmulatedCommandBar();
    m_viInputModeManager->inputAdapter()->viModeEmulatedCommandBar()->init(EmulatedCommandBar::SearchForward);
    return true;
}

bool NormalViMode::commandUndo()
{
    // See BUG #328277
    m_viInputModeManager->clearCurrentChangeLog();

    if (doc()->undoCount() > 0) {
        const bool mapped = m_viInputModeManager->keyMapper()->isExecutingMapping();

        if (mapped) {
            doc()->editEnd();
        }
        doc()->undo();
        if (mapped) {
            doc()->editStart();
        }
        if (m_viInputModeManager->isAnyVisualMode()) {
            m_viInputModeManager->getViVisualMode()->setStart(KTextEditor::Cursor(-1, -1));
            m_view->clearSelection();
            startNormalMode();
        }
        return true;
    }
    return false;
}

bool NormalViMode::commandRedo()
{
    if (doc()->redoCount() > 0) {
        const bool mapped = m_viInputModeManager->keyMapper()->isExecutingMapping();

        if (mapped) {
            doc()->editEnd();
        }
        doc()->redo();
        if (mapped) {
            doc()->editStart();
        }
        if (m_viInputModeManager->isAnyVisualMode()) {
            m_viInputModeManager->getViVisualMode()->setStart(KTextEditor::Cursor(-1, -1));
            m_view->clearSelection();
            startNormalMode();
        }
        return true;
    }
    return false;
}

bool NormalViMode::commandSetMark()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    QChar mark = m_keys.at(m_keys.size() - 1);
    m_viInputModeManager->marks()->setUserMark(mark, c);

    return true;
}

bool NormalViMode::commandIndentLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    doc()->indent(KTextEditor::Range(c.line(), 0, c.line() + getCount(), 0), 1);

    return true;
}

bool NormalViMode::commandUnindentLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    doc()->indent(KTextEditor::Range(c.line(), 0, c.line() + getCount(), 0), -1);

    return true;
}

bool NormalViMode::commandIndentLines()
{
    const bool downwards = m_commandRange.startLine < m_commandRange.endLine;

    m_commandRange.normalize();

    int line1 = m_commandRange.startLine;
    int line2 = m_commandRange.endLine;
    int col = getLine(line2).length();
    doc()->indent(KTextEditor::Range(line1, 0, line2, col), getCount());

    if (downwards) {
        updateCursor(KTextEditor::Cursor(m_commandRange.startLine, m_commandRange.startColumn));
    } else {
        updateCursor(KTextEditor::Cursor(m_commandRange.endLine, m_commandRange.endColumn));
    }
    return true;
}

bool NormalViMode::commandUnindentLines()
{
    const bool downwards = m_commandRange.startLine < m_commandRange.endLine;

    m_commandRange.normalize();

    int line1 = m_commandRange.startLine;
    int line2 = m_commandRange.endLine;

    doc()->indent(KTextEditor::Range(line1, 0, line2, doc()->lineLength(line2)), -getCount());

    if (downwards) {
        updateCursor(KTextEditor::Cursor(m_commandRange.startLine, m_commandRange.startColumn));
    } else {
        updateCursor(KTextEditor::Cursor(m_commandRange.endLine, m_commandRange.endColumn));
    }
    return true;
}

bool NormalViMode::commandScrollPageDown()
{
    if (getCount() < m_scroll_count_limit) {
        for (int i = 0; i < getCount(); i++) {
            m_view->pageDown();
        }
    }
    return true;
}

bool NormalViMode::commandScrollPageUp()
{
    if (getCount() < m_scroll_count_limit) {
        for (int i = 0; i < getCount(); i++) {
            m_view->pageUp();
        }
    }
    return true;
}

bool NormalViMode::commandScrollHalfPageUp()
{
    if (getCount() < m_scroll_count_limit) {
        for (int i = 0; i < getCount(); i++) {
            m_viewInternal->pageUp(false, true);
        }
    }
    return true;
}

bool NormalViMode::commandScrollHalfPageDown()
{
    if (getCount() < m_scroll_count_limit) {
        for (int i = 0; i < getCount(); i++) {
            m_viewInternal->pageDown(false, true);
        }
    }
    return true;
}

bool NormalViMode::commandCenterView(bool onFirst)
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    const int virtualCenterLine = m_viewInternal->startLine() + linesDisplayed() / 2;
    const int virtualCursorLine = m_view->textFolding().lineToVisibleLine(c.line());

    scrollViewLines(virtualCursorLine - virtualCenterLine);
    if (onFirst) {
        c.setColumn(getFirstNonBlank());
        updateCursor(c);
    }
    return true;
}

bool NormalViMode::commandCenterViewOnNonBlank()
{
    return commandCenterView(true);
}

bool NormalViMode::commandCenterViewOnCursor()
{
    return commandCenterView(false);
}

bool NormalViMode::commandTopView(bool onFirst)
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    const int virtualCenterLine = m_viewInternal->startLine();
    const int virtualCursorLine = m_view->textFolding().lineToVisibleLine(c.line());

    scrollViewLines(virtualCursorLine - virtualCenterLine);
    if (onFirst) {
        c.setColumn(getFirstNonBlank());
        updateCursor(c);
    }
    return true;
}

bool NormalViMode::commandTopViewOnNonBlank()
{
    return commandTopView(true);
}

bool NormalViMode::commandTopViewOnCursor()
{
    return commandTopView(false);
}

bool NormalViMode::commandBottomView(bool onFirst)
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    const int virtualCenterLine = m_viewInternal->endLine();
    const int virtualCursorLine = m_view->textFolding().lineToVisibleLine(c.line());

    scrollViewLines(virtualCursorLine - virtualCenterLine);
    if (onFirst) {
        c.setColumn(getFirstNonBlank());
        updateCursor(c);
    }
    return true;
}

bool NormalViMode::commandBottomViewOnNonBlank()
{
    return commandBottomView(true);
}

bool NormalViMode::commandBottomViewOnCursor()
{
    return commandBottomView(false);
}

bool NormalViMode::commandAbort()
{
    m_pendingResetIsDueToExit = true;
    reset();
    return true;
}

bool NormalViMode::commandPrintCharacterCode()
{
    QChar ch = getCharUnderCursor();

    if (ch == QChar::Null) {
        message(QStringLiteral("NUL"));
    } else {
        int code = ch.unicode();

        QString dec = QString::number(code);
        QString hex = QString::number(code, 16);
        QString oct = QString::number(code, 8);
        if (oct.length() < 3) {
            oct.prepend(QLatin1Char('0'));
        }
        if (code > 0x80 && code < 0x1000) {
            hex.prepend((code < 0x100 ? QLatin1String("00") : QLatin1String("0")));
        }
        message(i18n("'%1' %2,  Hex %3,  Octal %4", ch, dec, hex, oct));
    }

    return true;
}

bool NormalViMode::commandRepeatLastChange()
{
    const int repeatCount = getCount();
    resetParser();
    if (repeatCount > 1) {
        m_oneTimeCountOverride = repeatCount;
    }
    doc()->editStart();
    m_viInputModeManager->repeatLastChange();
    doc()->editEnd();

    return true;
}

bool NormalViMode::commandAlignLine()
{
    const int line = m_view->cursorPosition().line();
    KTextEditor::Range alignRange(KTextEditor::Cursor(line, 0), KTextEditor::Cursor(line, 0));

    doc()->align(m_view, alignRange);

    return true;
}

bool NormalViMode::commandAlignLines()
{
    m_commandRange.normalize();

    KTextEditor::Cursor start(m_commandRange.startLine, 0);
    KTextEditor::Cursor end(m_commandRange.endLine, 0);

    doc()->align(m_view, KTextEditor::Range(start, end));

    return true;
}

bool NormalViMode::commandAddToNumber()
{
    addToNumberUnderCursor(getCount());

    return true;
}

bool NormalViMode::commandSubtractFromNumber()
{
    addToNumberUnderCursor(-getCount());

    return true;
}

bool NormalViMode::commandPrependToBlock()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    // move cursor to top left corner of selection
    m_commandRange.normalize();
    c.setColumn(m_commandRange.startColumn);
    c.setLine(m_commandRange.startLine);
    updateCursor(c);

    m_stickyColumn = -1;
    m_viInputModeManager->getViInsertMode()->setBlockPrependMode(m_commandRange);
    return startInsertMode();
}

bool NormalViMode::commandAppendToBlock()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    m_commandRange.normalize();
    if (m_stickyColumn == (unsigned int)KateVi::EOL) { // append to EOL
        // move cursor to end of first line
        c.setLine(m_commandRange.startLine);
        c.setColumn(doc()->lineLength(c.line()));
        updateCursor(c);
        m_viInputModeManager->getViInsertMode()->setBlockAppendMode(m_commandRange, AppendEOL);
    } else {
        m_viInputModeManager->getViInsertMode()->setBlockAppendMode(m_commandRange, Append);
        // move cursor to top right corner of selection
        c.setColumn(m_commandRange.endColumn + 1);
        c.setLine(m_commandRange.startLine);
        updateCursor(c);
    }

    m_stickyColumn = -1;

    return startInsertMode();
}

bool NormalViMode::commandGoToNextJump()
{
    KTextEditor::Cursor c = getNextJump(m_view->cursorPosition());
    updateCursor(c);

    return true;
}

bool NormalViMode::commandGoToPrevJump()
{
    KTextEditor::Cursor c = getPrevJump(m_view->cursorPosition());
    updateCursor(c);

    return true;
}

bool NormalViMode::commandSwitchToLeftView()
{
    switchView(Left);
    return true;
}

bool NormalViMode::commandSwitchToDownView()
{
    switchView(Down);
    return true;
}

bool NormalViMode::commandSwitchToUpView()
{
    switchView(Up);
    return true;
}

bool NormalViMode::commandSwitchToRightView()
{
    switchView(Right);
    return true;
}

bool NormalViMode::commandSwitchToNextView()
{
    switchView(Next);
    return true;
}

bool NormalViMode::commandSplitHoriz()
{
    return executeKateCommand(QStringLiteral("split"));
}

bool NormalViMode::commandSplitVert()
{
    return executeKateCommand(QStringLiteral("vsplit"));
}

bool NormalViMode::commandCloseView()
{
    return executeKateCommand(QStringLiteral("close"));
}

bool NormalViMode::commandSwitchToNextTab()
{
    QString command = QStringLiteral("bn");

    if (m_iscounted) {
        command = command + QLatin1Char(' ') + QString::number(getCount());
    }

    return executeKateCommand(command);
}

bool NormalViMode::commandSwitchToPrevTab()
{
    QString command = QStringLiteral("bp");

    if (m_iscounted) {
        command = command + QLatin1Char(' ') + QString::number(getCount());
    }

    return executeKateCommand(command);
}

bool NormalViMode::commandFormatLine()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    reformatLines(c.line(), c.line() + getCount() - 1);

    return true;
}

bool NormalViMode::commandFormatLines()
{
    reformatLines(m_commandRange.startLine, m_commandRange.endLine);
    return true;
}

bool NormalViMode::commandCollapseToplevelNodes()
{
    m_view->slotFoldToplevelNodes();
    return true;
}

bool NormalViMode::commandStartRecordingMacro()
{
    const QChar reg = m_keys[m_keys.size() - 1];
    m_viInputModeManager->macroRecorder()->start(reg);
    return true;
}

bool NormalViMode::commandReplayMacro()
{
    // "@<registername>" will have been added to the log; it needs to be cleared
    // *before* we replay the macro keypresses, else it can cause an infinite loop
    // if the macro contains a "."
    m_viInputModeManager->clearCurrentChangeLog();
    const QChar reg = m_keys[m_keys.size() - 1];
    const unsigned int count = getCount();
    resetParser();
    doc()->editStart();
    for (unsigned int i = 0; i < count; i++) {
        m_viInputModeManager->macroRecorder()->replay(reg);
    }
    doc()->editEnd();
    return true;
}

bool NormalViMode::commandCloseNocheck()
{
    return executeKateCommand(QStringLiteral("q!"));
}

bool NormalViMode::commandCloseWrite()
{
    return executeKateCommand(QStringLiteral("wq"));
}

bool NormalViMode::commandCollapseLocal()
{
    int line = m_view->cursorPosition().line();
    bool actionDone = false;
    while (!actionDone && line > -1) {
        actionDone = m_view->foldLine(line--).isValid();
    }
    return true;
}

bool NormalViMode::commandExpandAll()
{
    // FIXME: is toplevel same as all?
    m_view->slotExpandToplevelNodes();
    return true;
}

bool NormalViMode::commandExpandLocal()
{
    int line = m_view->cursorPosition().line();
    return m_view->unfoldLine(line);
}

bool NormalViMode::commandToggleRegionVisibility()
{
    // FIXME: is this equivalent to Vim (or a desired change)?
    m_view->slotToggleFolding();
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// MOTIONS
////////////////////////////////////////////////////////////////////////////////

Range NormalViMode::motionDown()
{
    return goLineDown();
}

Range NormalViMode::motionUp()
{
    return goLineUp();
}

Range NormalViMode::motionLeft()
{
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    m_stickyColumn = -1;
    Range r(cursor, ExclusiveMotion);
    r.endColumn -= getCount();

    if (r.endColumn < 0) {
        r.endColumn = 0;
    }

    return r;
}

Range NormalViMode::motionRight()
{
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    m_stickyColumn = -1;
    Range r(cursor, ExclusiveMotion);
    r.endColumn += getCount();

    // make sure end position isn't > line length
    if (r.endColumn > doc()->lineLength(r.endLine)) {
        r.endColumn = doc()->lineLength(r.endLine);
    }

    return r;
}

Range NormalViMode::motionPageDown()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);
    r.endLine += linesDisplayed();

    if (r.endLine >= doc()->lines()) {
        r.endLine = doc()->lines() - 1;
    }
    return r;
}

Range NormalViMode::motionPageUp()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);
    r.endLine -= linesDisplayed();

    if (r.endLine < 0) {
        r.endLine = 0;
    }
    return r;
}

Range NormalViMode::motionHalfPageDown()
{
    if (commandScrollHalfPageDown()) {
        KTextEditor::Cursor c = m_view->cursorPosition();
        m_commandRange.endLine = c.line();
        m_commandRange.endColumn = c.column();
        return m_commandRange;
    }
    return Range::invalid();
}

Range NormalViMode::motionHalfPageUp()
{
    if (commandScrollHalfPageUp()) {
        KTextEditor::Cursor c = m_view->cursorPosition();
        m_commandRange.endLine = c.line();
        m_commandRange.endColumn = c.column();
        return m_commandRange;
    }
    return Range::invalid();
}

Range NormalViMode::motionDownToFirstNonBlank()
{
    Range r = goLineDown();
    r.endColumn = getFirstNonBlank(r.endLine);
    return r;
}

Range NormalViMode::motionUpToFirstNonBlank()
{
    Range r = goLineUp();
    r.endColumn = getFirstNonBlank(r.endLine);
    return r;
}

Range NormalViMode::motionWordForward()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, ExclusiveMotion);

    m_stickyColumn = -1;

    // Special case: If we're already on the very last character in the document, the motion should be
    // inclusive so the last character gets included
    if (c.line() == doc()->lines() - 1 && c.column() == doc()->lineLength(c.line()) - 1) {
        r.motionType = InclusiveMotion;
    } else {
        for (int i = 0; i < getCount(); i++) {
            c = findNextWordStart(c.line(), c.column());

            // stop when at the last char in the document
            if (!c.isValid()) {
                c = doc()->documentEnd();
                // if we still haven't "used up the count", make the motion inclusive, so that the last char
                // is included
                if (i < getCount()) {
                    r.motionType = InclusiveMotion;
                }
                break;
            }
        }
    }

    r.endColumn = c.column();
    r.endLine = c.line();

    return r;
}

Range NormalViMode::motionWordBackward()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, ExclusiveMotion);

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        c = findPrevWordStart(c.line(), c.column());

        if (!c.isValid()) {
            c = KTextEditor::Cursor(0, 0);
            break;
        }
    }

    r.endColumn = c.column();
    r.endLine = c.line();

    return r;
}

Range NormalViMode::motionWORDForward()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, ExclusiveMotion);

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        c = findNextWORDStart(c.line(), c.column());

        // stop when at the last char in the document
        if (c.line() == doc()->lines() - 1 && c.column() == doc()->lineLength(c.line()) - 1) {
            break;
        }
    }

    r.endColumn = c.column();
    r.endLine = c.line();

    return r;
}

Range NormalViMode::motionWORDBackward()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, ExclusiveMotion);

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        c = findPrevWORDStart(c.line(), c.column());

        if (!c.isValid()) {
            c = KTextEditor::Cursor(0, 0);
        }
    }

    r.endColumn = c.column();
    r.endLine = c.line();

    return r;
}

Range NormalViMode::motionToEndOfWord()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        c = findWordEnd(c.line(), c.column());
    }

    if (!c.isValid()) {
        c = doc()->documentEnd();
    }

    r.endColumn = c.column();
    r.endLine = c.line();

    return r;
}

Range NormalViMode::motionToEndOfWORD()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        c = findWORDEnd(c.line(), c.column());
    }

    if (!c.isValid()) {
        c = doc()->documentEnd();
    }

    r.endColumn = c.column();
    r.endLine = c.line();

    return r;
}

Range NormalViMode::motionToEndOfPrevWord()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        c = findPrevWordEnd(c.line(), c.column());

        if (c.isValid()) {
            r.endColumn = c.column();
            r.endLine = c.line();
        } else {
            r.endColumn = 0;
            r.endLine = 0;
            break;
        }
    }

    return r;
}

Range NormalViMode::motionToEndOfPrevWORD()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    Range r(c, InclusiveMotion);

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        c = findPrevWORDEnd(c.line(), c.column());

        if (c.isValid()) {
            r.endColumn = c.column();
            r.endLine = c.line();
        } else {
            r.endColumn = 0;
            r.endLine = 0;
            break;
        }
    }

    return r;
}

void NormalViMode::stickStickyColumnToEOL()
{
    if (m_keys.size() == 1) {
        m_stickyColumn = KateVi::EOL;
    }
}

Range NormalViMode::motionToEOL()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    stickStickyColumnToEOL();

    unsigned int line = c.line() + (getCount() - 1);
    Range r(line, doc()->lineLength(line) - 1, InclusiveMotion);

    return r;
}
Range NormalViMode::motionToLastNonBlank()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    stickStickyColumnToEOL();

    unsigned int line = c.line() + (getCount() - 1);

    const auto text_line = doc()->plainKateTextLine(line);
    Range r(line, text_line.previousNonSpaceChar(text_line.length()), InclusiveMotion);
    return r;
}

Range NormalViMode::motionToColumn0()
{
    m_stickyColumn = -1;
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    Range r(cursor.line(), 0, ExclusiveMotion);

    return r;
}

Range NormalViMode::motionToFirstCharacterOfLine()
{
    m_stickyColumn = -1;

    KTextEditor::Cursor cursor(m_view->cursorPosition());
    int c = getFirstNonBlank();

    Range r(cursor.line(), c, ExclusiveMotion);

    return r;
}

Range NormalViMode::motionFindChar()
{
    m_lastTFcommand = m_keys;
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    QString line = getLine();

    m_stickyColumn = -1;

    int matchColumn = cursor.column();

    for (int i = 0; i < getCount(); i++) {
        matchColumn = line.indexOf(QStringView(m_keys).right(1), matchColumn + 1);
        if (matchColumn == -1) {
            break;
        }
    }

    Range r;

    if (matchColumn != -1) {
        r.endColumn = matchColumn;
        r.endLine = cursor.line();
    } else {
        return Range::invalid();
    }

    return r;
}

Range NormalViMode::motionFindCharBackward()
{
    m_lastTFcommand = m_keys;
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    QString line = getLine();

    m_stickyColumn = -1;

    int matchColumn = -1;

    int hits = 0;
    int i = cursor.column() - 1;

    while (hits != getCount() && i >= 0) {
        if (line.at(i) == m_keys.at(m_keys.size() - 1)) {
            hits++;
        }

        if (hits == getCount()) {
            matchColumn = i;
        }

        i--;
    }

    Range r(cursor, ExclusiveMotion);

    if (matchColumn != -1) {
        r.endColumn = matchColumn;
        r.endLine = cursor.line();
    } else {
        return Range::invalid();
    }

    return r;
}

Range NormalViMode::motionToChar()
{
    m_lastTFcommand = m_keys;
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    QString line = getLine();

    m_stickyColumn = -1;
    Range r;
    r.endColumn = -1;
    r.endLine = -1;

    int matchColumn = cursor.column() + (m_isRepeatedTFcommand ? 2 : 1);

    for (int i = 0; i < getCount(); i++) {
        const int lastColumn = matchColumn;
        matchColumn = line.indexOf(m_keys.right(1), matchColumn + ((i > 0) ? 1 : 0));
        if (matchColumn == -1) {
            if (m_isRepeatedTFcommand) {
                matchColumn = lastColumn;
            } else {
                return Range::invalid();
            }
            break;
        }
    }

    r.endColumn = matchColumn - 1;
    r.endLine = cursor.line();

    m_isRepeatedTFcommand = false;
    return r;
}

Range NormalViMode::motionToCharBackward()
{
    m_lastTFcommand = m_keys;
    KTextEditor::Cursor cursor(m_view->cursorPosition());
    QString line = getLine();

    const int originalColumn = cursor.column();
    m_stickyColumn = -1;

    int matchColumn = originalColumn - 1;

    int hits = 0;
    int i = cursor.column() - (m_isRepeatedTFcommand ? 2 : 1);

    Range r(cursor, ExclusiveMotion);

    while (hits != getCount() && i >= 0) {
        if (line.at(i) == m_keys.at(m_keys.size() - 1)) {
            hits++;
        }

        if (hits == getCount()) {
            matchColumn = i;
        }

        i--;
    }

    if (hits == getCount()) {
        r.endColumn = matchColumn + 1;
        r.endLine = cursor.line();
    } else {
        r.valid = false;
    }

    m_isRepeatedTFcommand = false;

    return r;
}

Range NormalViMode::motionRepeatlastTF()
{
    if (!m_lastTFcommand.isEmpty()) {
        m_isRepeatedTFcommand = true;
        m_keys = m_lastTFcommand;
        if (m_keys.at(0) == QLatin1Char('f')) {
            return motionFindChar();
        } else if (m_keys.at(0) == QLatin1Char('F')) {
            return motionFindCharBackward();
        } else if (m_keys.at(0) == QLatin1Char('t')) {
            return motionToChar();
        } else if (m_keys.at(0) == QLatin1Char('T')) {
            return motionToCharBackward();
        }
    }

    // there was no previous t/f command
    return Range::invalid();
}

Range NormalViMode::motionRepeatlastTFBackward()
{
    if (!m_lastTFcommand.isEmpty()) {
        m_isRepeatedTFcommand = true;
        m_keys = m_lastTFcommand;
        if (m_keys.at(0) == QLatin1Char('f')) {
            return motionFindCharBackward();
        } else if (m_keys.at(0) == QLatin1Char('F')) {
            return motionFindChar();
        } else if (m_keys.at(0) == QLatin1Char('t')) {
            return motionToCharBackward();
        } else if (m_keys.at(0) == QLatin1Char('T')) {
            return motionToChar();
        }
    }

    // there was no previous t/f command
    return Range::invalid();
}

Range NormalViMode::motionToLineFirst()
{
    Range r(getCount() - 1, 0, InclusiveMotion);
    m_stickyColumn = -1;

    if (r.endLine > doc()->lines() - 1) {
        r.endLine = doc()->lines() - 1;
    }
    r.jump = true;

    return r;
}

Range NormalViMode::motionToLineLast()
{
    Range r(doc()->lines() - 1, 0, InclusiveMotion);
    m_stickyColumn = -1;

    // don't use getCount() here, no count and a count of 1 is different here...
    if (m_count != 0) {
        r.endLine = m_count - 1;
    }

    if (r.endLine > doc()->lines() - 1) {
        r.endLine = doc()->lines() - 1;
    }
    r.jump = true;

    return r;
}

Range NormalViMode::motionToScreenColumn()
{
    m_stickyColumn = -1;

    KTextEditor::Cursor c(m_view->cursorPosition());

    int column = getCount() - 1;

    if (doc()->lineLength(c.line()) - 1 < (int)getCount() - 1) {
        column = doc()->lineLength(c.line()) - 1;
    }

    return Range(c.line(), column, ExclusiveMotion);
}

Range NormalViMode::motionToMark()
{
    Range r;

    m_stickyColumn = -1;

    QChar reg = m_keys.at(m_keys.size() - 1);

    KTextEditor::Cursor c = m_viInputModeManager->marks()->getMarkPosition(reg);
    if (c.isValid()) {
        r.endLine = c.line();
        r.endColumn = c.column();
    } else {
        error(i18n("Mark not set: %1", m_keys.right(1)));
        r.valid = false;
    }

    r.jump = true;

    return r;
}

Range NormalViMode::motionToMarkLine()
{
    Range r = motionToMark();
    r.endColumn = getFirstNonBlank(r.endLine);
    r.jump = true;
    m_stickyColumn = -1;
    return r;
}

Range NormalViMode::motionToMatchingItem()
{
    Range r;
    int lines = doc()->lines();

    // If counted, then it's not a motion to matching item anymore,
    // but a motion to the N'th percentage of the document
    if (isCounted()) {
        int count = getCount();
        if (count > 100) {
            return r;
        }
        r.endLine = qRound(lines * count / 100.0) - 1;
        r.endColumn = 0;
        return r;
    }

    KTextEditor::Cursor c(m_view->cursorPosition());

    QString l = getLine();
    int n1 = l.indexOf(m_matchItemRegex, c.column());

    m_stickyColumn = -1;

    if (n1 < 0) {
        return Range::invalid();
    }

    const auto bracketChar = l.at(n1);
    // use Kate's built-in matching bracket finder for brackets
    if (bracketChar == QLatin1Char('(') || bracketChar == QLatin1Char(')') || bracketChar == QLatin1Char('{') || bracketChar == QLatin1Char('}')
        || bracketChar == QLatin1Char('[') || bracketChar == QLatin1Char(']')) {
        // findMatchingBracket requires us to move the cursor to the
        // first bracket, but we don't want the cursor to really move
        // in case this is e.g. a yank, so restore it to its original
        // position afterwards.
        c.setColumn(n1);
        const KTextEditor::Cursor oldCursorPos = m_view->cursorPosition();
        updateCursor(c);

        // find the matching one
        c = m_viewInternal->findMatchingBracket();
        if (c > m_view->cursorPosition()) {
            c.setColumn(c.column() - 1);
        }
        m_view->setCursorPosition(oldCursorPos);
    } else {
        // text item we want to find a matching item for
        static const QRegularExpression boundaryRegex(QStringLiteral("\\b|\\s|$"), QRegularExpression::UseUnicodePropertiesOption);
        const int n2 = l.indexOf(boundaryRegex, n1);
        QString item = l.mid(n1, n2 - n1);
        QString matchingItem = m_matchingItems[item];

        int toFind = 1;
        int line = c.line();
        int column = n2 - item.length();
        bool reverse = false;

        if (matchingItem.startsWith(QLatin1Char('-'))) {
            matchingItem.remove(0, 1); // remove the '-'
            reverse = true;
        }

        // make sure we don't hit the text item we started the search from
        if (column == 0 && reverse) {
            column -= item.length();
        }

        int itemIdx;
        int matchItemIdx;

        while (toFind > 0) {
            if (reverse) {
                itemIdx = l.lastIndexOf(item, column - 1);
                matchItemIdx = l.lastIndexOf(matchingItem, column - 1);

                if (itemIdx != -1 && (matchItemIdx == -1 || itemIdx > matchItemIdx)) {
                    ++toFind;
                }
            } else {
                itemIdx = l.indexOf(item, column);
                matchItemIdx = l.indexOf(matchingItem, column);

                if (itemIdx != -1 && (matchItemIdx == -1 || itemIdx < matchItemIdx)) {
                    ++toFind;
                }
            }

            if (matchItemIdx != -1 || itemIdx != -1) {
                if (!reverse) {
                    column = qMin((unsigned int)itemIdx, (unsigned int)matchItemIdx);
                } else {
                    column = qMax(itemIdx, matchItemIdx);
                }
            }

            if (matchItemIdx != -1) { // match on current line
                if (matchItemIdx == column) {
                    --toFind;
                    c.setLine(line);
                    c.setColumn(column);
                }
            } else { // no match, advance one line if possible
                (reverse) ? --line : ++line;
                column = 0;

                if ((!reverse && line >= lines) || (reverse && line < 0)) {
                    r.valid = false;
                    break;
                } else {
                    l = getLine(line);
                }
            }
        }
    }

    r.endLine = c.line();
    r.endColumn = c.column();
    r.jump = true;

    return r;
}

Range NormalViMode::motionToNextBraceBlockStart()
{
    Range r;

    m_stickyColumn = -1;

    int line = findLineStartingWitchChar(QLatin1Char('{'), getCount());

    if (line == -1) {
        return Range::invalid();
    }

    r.endLine = line;
    r.endColumn = 0;
    r.jump = true;

    if (motionWillBeUsedWithCommand()) {
        // Delete from cursor (inclusive) to the '{' (exclusive).
        // If we are on the first column, then delete the entire current line.
        r.motionType = ExclusiveMotion;
        if (m_view->cursorPosition().column() != 0) {
            r.endLine--;
            r.endColumn = doc()->lineLength(r.endLine);
        }
    }

    return r;
}

Range NormalViMode::motionToPreviousBraceBlockStart()
{
    Range r;

    m_stickyColumn = -1;

    int line = findLineStartingWitchChar(QLatin1Char('{'), getCount(), false);

    if (line == -1) {
        return Range::invalid();
    }

    r.endLine = line;
    r.endColumn = 0;
    r.jump = true;

    if (motionWillBeUsedWithCommand()) {
        // With a command, do not include the { or the cursor position.
        r.motionType = ExclusiveMotion;
    }

    return r;
}

Range NormalViMode::motionToNextBraceBlockEnd()
{
    Range r;

    m_stickyColumn = -1;

    int line = findLineStartingWitchChar(QLatin1Char('}'), getCount());

    if (line == -1) {
        return Range::invalid();
    }

    r.endLine = line;
    r.endColumn = 0;
    r.jump = true;

    if (motionWillBeUsedWithCommand()) {
        // Delete from cursor (inclusive) to the '}' (exclusive).
        // If we are on the first column, then delete the entire current line.
        r.motionType = ExclusiveMotion;
        if (m_view->cursorPosition().column() != 0) {
            r.endLine--;
            r.endColumn = doc()->lineLength(r.endLine);
        }
    }

    return r;
}

Range NormalViMode::motionToPreviousBraceBlockEnd()
{
    Range r;

    m_stickyColumn = -1;

    int line = findLineStartingWitchChar(QLatin1Char('}'), getCount(), false);

    if (line == -1) {
        return Range::invalid();
    }

    r.endLine = line;
    r.endColumn = 0;
    r.jump = true;

    if (motionWillBeUsedWithCommand()) {
        r.motionType = ExclusiveMotion;
    }

    return r;
}

Range NormalViMode::motionToNextOccurrence()
{
    const QString word = getWordUnderCursor();
    Searcher *searcher = m_viInputModeManager->searcher();
    const Range match = searcher->findWordForMotion(word, false, getWordRangeUnderCursor().start(), getCount());
    if (searcher->lastSearchWrapped()) {
        m_view->showSearchWrappedHint(/*isReverseSearch*/ false);
    }

    return Range(match.startLine, match.startColumn, ExclusiveMotion);
}

Range NormalViMode::motionToPrevOccurrence()
{
    const QString word = getWordUnderCursor();
    Searcher *searcher = m_viInputModeManager->searcher();
    const Range match = searcher->findWordForMotion(word, true, getWordRangeUnderCursor().start(), getCount());
    if (searcher->lastSearchWrapped()) {
        m_view->showSearchWrappedHint(/*isReverseSearch*/ true);
    }

    return Range(match.startLine, match.startColumn, ExclusiveMotion);
}

Range NormalViMode::motionToFirstLineOfWindow()
{
    int lines_to_go;
    if (linesDisplayed() <= (unsigned int)m_viewInternal->endLine()) {
        lines_to_go = m_viewInternal->endLine() - linesDisplayed() - m_view->cursorPosition().line() + 1;
    } else {
        lines_to_go = -m_view->cursorPosition().line();
    }

    Range r = goLineUpDown(lines_to_go);
    r.endColumn = getFirstNonBlank(r.endLine);
    return r;
}

Range NormalViMode::motionToMiddleLineOfWindow()
{
    int lines_to_go;
    if (linesDisplayed() <= (unsigned int)m_viewInternal->endLine()) {
        lines_to_go = m_viewInternal->endLine() - linesDisplayed() / 2 - m_view->cursorPosition().line();
    } else {
        lines_to_go = m_viewInternal->endLine() / 2 - m_view->cursorPosition().line();
    }

    Range r = goLineUpDown(lines_to_go);
    r.endColumn = getFirstNonBlank(r.endLine);
    return r;
}

Range NormalViMode::motionToLastLineOfWindow()
{
    int lines_to_go;
    if (linesDisplayed() <= (unsigned int)m_viewInternal->endLine()) {
        lines_to_go = m_viewInternal->endLine() - m_view->cursorPosition().line();
    } else {
        lines_to_go = m_viewInternal->endLine() - m_view->cursorPosition().line();
    }

    Range r = goLineUpDown(lines_to_go);
    r.endColumn = getFirstNonBlank(r.endLine);
    return r;
}

Range NormalViMode::motionToNextVisualLine()
{
    return goVisualLineUpDown(getCount());
}

Range NormalViMode::motionToPrevVisualLine()
{
    return goVisualLineUpDown(-getCount());
}

Range NormalViMode::motionToPreviousSentence()
{
    KTextEditor::Cursor c = findSentenceStart();
    int linenum = c.line();
    int column = c.column();
    const bool skipSpaces = doc()->line(linenum).isEmpty();

    if (skipSpaces) {
        linenum--;
        if (linenum >= 0) {
            column = doc()->line(linenum).size() - 1;
        }
    }

    for (int i = linenum; i >= 0; i--) {
        const QString &line = doc()->line(i);

        if (line.isEmpty() && !skipSpaces) {
            return Range(i, 0, InclusiveMotion);
        }

        if (column < 0 && !line.isEmpty()) {
            column = line.size() - 1;
        }

        for (int j = column; j >= 0; j--) {
            if (skipSpaces || QStringLiteral(".?!").indexOf(line.at(j)) != -1) {
                c.setLine(i);
                c.setColumn(j);
                updateCursor(c);
                c = findSentenceStart();
                return Range(c, InclusiveMotion);
            }
        }
        column = line.size() - 1;
    }
    return Range(0, 0, InclusiveMotion);
}

Range NormalViMode::motionToNextSentence()
{
    KTextEditor::Cursor c = findSentenceEnd();
    int linenum = c.line();
    int column = c.column() + 1;
    const bool skipSpaces = doc()->line(linenum).isEmpty();

    for (int i = linenum; i < doc()->lines(); i++) {
        const QString &line = doc()->line(i);

        if (line.isEmpty() && !skipSpaces) {
            return Range(i, 0, InclusiveMotion);
        }

        for (int j = column; j < line.size(); j++) {
            if (!line.at(j).isSpace()) {
                return Range(i, j, InclusiveMotion);
            }
        }
        column = 0;
    }

    c = doc()->documentEnd();
    return Range(c, InclusiveMotion);
}

Range NormalViMode::motionToBeforeParagraph()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    int line = c.line();

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        // advance at least one line, but if there are consecutive blank lines
        // skip them all
        do {
            line--;
        } while (line >= 0 && getLine(line + 1).length() == 0);
        while (line > 0 && getLine(line).length() != 0) {
            line--;
        }
    }

    if (line < 0) {
        line = 0;
    }

    Range r(line, 0, InclusiveMotion);

    return r;
}

Range NormalViMode::motionToAfterParagraph()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    int line = c.line();

    m_stickyColumn = -1;

    for (int i = 0; i < getCount(); i++) {
        // advance at least one line, but if there are consecutive blank lines
        // skip them all
        do {
            line++;
        } while (line <= doc()->lines() - 1 && getLine(line - 1).length() == 0);
        while (line < doc()->lines() - 1 && getLine(line).length() != 0) {
            line++;
        }
    }

    if (line >= doc()->lines()) {
        line = doc()->lines() - 1;
    }

    // if we ended up on the last line, the cursor should be placed on the last column
    int column = (line == doc()->lines() - 1) ? qMax(getLine(line).length() - 1, 0) : 0;

    return Range(line, column, InclusiveMotion);
}

Range NormalViMode::motionToIncrementalSearchMatch()
{
    return Range(m_positionWhenIncrementalSearchBegan.line(),
                 m_positionWhenIncrementalSearchBegan.column(),
                 m_view->cursorPosition().line(),
                 m_view->cursorPosition().column(),
                 ExclusiveMotion);
}

////////////////////////////////////////////////////////////////////////////////
// TEXT OBJECTS
////////////////////////////////////////////////////////////////////////////////

Range NormalViMode::textObjectAWord()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    KTextEditor::Cursor c1 = c;

    bool startedOnSpace = false;
    if (doc()->characterAt(c).isSpace()) {
        startedOnSpace = true;
    } else {
        c1 = findPrevWordStart(c.line(), c.column() + 1, true);
        if (!c1.isValid()) {
            c1 = KTextEditor::Cursor(0, 0);
        }
    }
    KTextEditor::Cursor c2 = KTextEditor::Cursor(c.line(), c.column() - 1);
    for (int i = 1; i <= getCount(); i++) {
        c2 = findWordEnd(c2.line(), c2.column());
    }
    if (!c1.isValid() || !c2.isValid()) {
        return Range::invalid();
    }
    // Adhere to some of Vim's bizarre rules of whether to swallow ensuing spaces or not.
    // Don't ask ;)
    const KTextEditor::Cursor nextWordStart = findNextWordStart(c2.line(), c2.column());
    if (nextWordStart.isValid() && nextWordStart.line() == c2.line()) {
        if (!startedOnSpace) {
            c2 = KTextEditor::Cursor(nextWordStart.line(), nextWordStart.column() - 1);
        }
    } else {
        c2 = KTextEditor::Cursor(c2.line(), doc()->lineLength(c2.line()) - 1);
    }
    bool swallowCarriageReturnAtEndOfLine = false;
    if (c2.line() != c.line() && c2.column() == doc()->lineLength(c2.line()) - 1) {
        // Greedily descend to the next line, so as to swallow the carriage return on this line.
        c2 = KTextEditor::Cursor(c2.line() + 1, 0);
        swallowCarriageReturnAtEndOfLine = true;
    }
    const bool swallowPrecedingSpaces =
        (c2.column() == doc()->lineLength(c2.line()) - 1 && !doc()->characterAt(c2).isSpace()) || startedOnSpace || swallowCarriageReturnAtEndOfLine;
    if (swallowPrecedingSpaces) {
        if (c1.column() != 0) {
            const KTextEditor::Cursor previousNonSpace = findPrevWordEnd(c.line(), c.column());
            if (previousNonSpace.isValid() && previousNonSpace.line() == c1.line()) {
                c1 = KTextEditor::Cursor(previousNonSpace.line(), previousNonSpace.column() + 1);
            } else if (startedOnSpace || swallowCarriageReturnAtEndOfLine) {
                c1 = KTextEditor::Cursor(c1.line(), 0);
            }
        }
    }

    return Range(c1, c2, !swallowCarriageReturnAtEndOfLine ? InclusiveMotion : ExclusiveMotion);
}

Range NormalViMode::textObjectInnerWord()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    KTextEditor::Cursor c1 = findPrevWordStart(c.line(), c.column() + 1, true);
    if (!c1.isValid()) {
        c1 = KTextEditor::Cursor(0, 0);
    }
    // need to start search in column-1 because it might be a one-character word
    KTextEditor::Cursor c2(c.line(), c.column() - 1);

    for (int i = 0; i < getCount(); i++) {
        c2 = findWordEnd(c2.line(), c2.column(), true);
    }

    if (!c2.isValid()) {
        c2 = doc()->documentEnd();
    }

    // sanity check
    if (c1.line() != c2.line() || c1.column() > c2.column()) {
        return Range::invalid();
    }
    return Range(c1, c2, InclusiveMotion);
}

Range NormalViMode::textObjectAWORD()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    KTextEditor::Cursor c1 = c;

    bool startedOnSpace = false;
    if (doc()->characterAt(c).isSpace()) {
        startedOnSpace = true;
    } else {
        c1 = findPrevWORDStart(c.line(), c.column() + 1, true);
        if (!c1.isValid()) {
            c1 = KTextEditor::Cursor(0, 0);
        }
    }
    KTextEditor::Cursor c2 = KTextEditor::Cursor(c.line(), c.column() - 1);
    for (int i = 1; i <= getCount(); i++) {
        c2 = findWORDEnd(c2.line(), c2.column());
    }
    if (!c1.isValid() || !c2.isValid()) {
        return Range::invalid();
    }
    // Adhere to some of Vim's bizarre rules of whether to swallow ensuing spaces or not.
    // Don't ask ;)
    const KTextEditor::Cursor nextWordStart = findNextWordStart(c2.line(), c2.column());
    if (nextWordStart.isValid() && nextWordStart.line() == c2.line()) {
        if (!startedOnSpace) {
            c2 = KTextEditor::Cursor(nextWordStart.line(), nextWordStart.column() - 1);
        }
    } else {
        c2 = KTextEditor::Cursor(c2.line(), doc()->lineLength(c2.line()) - 1);
    }
    bool swallowCarriageReturnAtEndOfLine = false;
    if (c2.line() != c.line() && c2.column() == doc()->lineLength(c2.line()) - 1) {
        // Greedily descend to the next line, so as to swallow the carriage return on this line.
        c2 = KTextEditor::Cursor(c2.line() + 1, 0);
        swallowCarriageReturnAtEndOfLine = true;
    }
    const bool swallowPrecedingSpaces =
        (c2.column() == doc()->lineLength(c2.line()) - 1 && !doc()->characterAt(c2).isSpace()) || startedOnSpace || swallowCarriageReturnAtEndOfLine;
    if (swallowPrecedingSpaces) {
        if (c1.column() != 0) {
            const KTextEditor::Cursor previousNonSpace = findPrevWORDEnd(c.line(), c.column());
            if (previousNonSpace.isValid() && previousNonSpace.line() == c1.line()) {
                c1 = KTextEditor::Cursor(previousNonSpace.line(), previousNonSpace.column() + 1);
            } else if (startedOnSpace || swallowCarriageReturnAtEndOfLine) {
                c1 = KTextEditor::Cursor(c1.line(), 0);
            }
        }
    }

    return Range(c1, c2, !swallowCarriageReturnAtEndOfLine ? InclusiveMotion : ExclusiveMotion);
}

Range NormalViMode::textObjectInnerWORD()
{
    KTextEditor::Cursor c(m_view->cursorPosition());

    KTextEditor::Cursor c1 = findPrevWORDStart(c.line(), c.column() + 1, true);
    if (!c1.isValid()) {
        c1 = KTextEditor::Cursor(0, 0);
    }
    KTextEditor::Cursor c2(c);

    for (int i = 0; i < getCount(); i++) {
        c2 = findWORDEnd(c2.line(), c2.column(), true);
    }

    if (!c2.isValid()) {
        c2 = doc()->documentEnd();
    }

    // sanity check
    if (c1.line() != c2.line() || c1.column() > c2.column()) {
        return Range::invalid();
    }
    return Range(c1, c2, InclusiveMotion);
}

KTextEditor::Cursor NormalViMode::findSentenceStart()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    int linenum = c.line();
    int column = c.column();
    int prev = column;

    for (int i = linenum; i >= 0; i--) {
        const QString &line = doc()->line(i);
        const int lineLength = line.size();
        if (i != linenum) {
            column = lineLength;
        }

        // An empty line is the end of a paragraph.
        if (line.isEmpty()) {
            return KTextEditor::Cursor((i != linenum) ? i + 1 : i, prev);
        }

        prev = column;
        for (int j = column; j >= 0; j--) {
            if (j == lineLength || line.at(j).isSpace()) {
                int lastSpace = j--;
                for (; j >= 0 && QStringLiteral("\"')]").indexOf(line.at(j)) != -1; j--) {
                    ;
                }

                if (j >= 0 && QStringLiteral(".!?").indexOf(line.at(j)) != -1) {
                    if (lastSpace == lineLength) {
                        // If the line ends with one of .!?, then the sentence starts from the next line.
                        return KTextEditor::Cursor(i + 1, 0);
                    }

                    return KTextEditor::Cursor(i, prev);
                }
                j = lastSpace;
            } else {
                prev = j;
            }
        }
    }

    return KTextEditor::Cursor(0, 0);
}

KTextEditor::Cursor NormalViMode::findSentenceEnd()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    int linenum = c.line();
    int column = c.column();
    int j = 0;
    int prev = 0;

    for (int i = linenum; i < doc()->lines(); i++) {
        const QString &line = doc()->line(i);

        // An empty line is the end of a paragraph.
        if (line.isEmpty()) {
            return KTextEditor::Cursor(linenum, j);
        }

        // Iterating over the line to reach any '.', '!', '?'
        for (j = column; j < line.size(); j++) {
            if (QStringLiteral(".!?").indexOf(line.at(j)) != -1) {
                prev = j++;
                // Skip possible closing characters.
                for (; j < line.size() && QStringLiteral("\"')]").indexOf(line.at(j)) != -1; j++) {
                    ;
                }

                if (j >= line.size()) {
                    return KTextEditor::Cursor(i, j - 1);
                }

                // And hopefully we're done...
                if (line.at(j).isSpace()) {
                    return KTextEditor::Cursor(i, j - 1);
                }
                j = prev;
            }
        }
        linenum = i;
        prev = column;
        column = 0;
    }

    return KTextEditor::Cursor(linenum, j - 1);
}

KTextEditor::Cursor NormalViMode::findParagraphStart()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    const bool firstBlank = doc()->line(c.line()).isEmpty();
    int prev = c.line();

    for (int i = prev; i >= 0; i--) {
        if (doc()->line(i).isEmpty()) {
            if (i != prev) {
                prev = i + 1;
            }

            /* Skip consecutive empty lines. */
            if (firstBlank) {
                i--;
                for (; i >= 0 && doc()->line(i).isEmpty(); i--, prev--) {
                    ;
                }
            }
            return KTextEditor::Cursor(prev, 0);
        }
    }
    return KTextEditor::Cursor(0, 0);
}

KTextEditor::Cursor NormalViMode::findParagraphEnd()
{
    KTextEditor::Cursor c(m_view->cursorPosition());
    int prev = c.line();
    int lines = doc()->lines();
    const bool firstBlank = doc()->line(prev).isEmpty();

    for (int i = prev; i < lines; i++) {
        if (doc()->line(i).isEmpty()) {
            if (i != prev) {
                prev = i - 1;
            }

            /* Skip consecutive empty lines. */
            if (firstBlank) {
                i++;
                for (; i < lines && doc()->line(i).isEmpty(); i++, prev++) {
                    ;
                }
            }
            int length = doc()->lineLength(prev);
            return KTextEditor::Cursor(prev, (length <= 0) ? 0 : length - 1);
        }
    }
    return doc()->documentEnd();
}

Range NormalViMode::textObjectInnerSentence()
{
    Range r;
    KTextEditor::Cursor c1 = findSentenceStart();
    KTextEditor::Cursor c2 = findSentenceEnd();
    updateCursor(c1);

    r.startLine = c1.line();
    r.startColumn = c1.column();
    r.endLine = c2.line();
    r.endColumn = c2.column();
    return r;
}

Range NormalViMode::textObjectASentence()
{
    int i;
    Range r = textObjectInnerSentence();
    const QString &line = doc()->line(r.endLine);

    // Skip whitespaces and tabs.
    for (i = r.endColumn + 1; i < line.size(); i++) {
        if (!line.at(i).isSpace()) {
            break;
        }
    }
    r.endColumn = i - 1;

    // Remove preceding spaces.
    if (r.startColumn != 0) {
        if (r.endColumn == line.size() - 1 && !line.at(r.endColumn).isSpace()) {
            const QString &line = doc()->line(r.startLine);
            for (i = r.startColumn - 1; i >= 0; i--) {
                if (!line.at(i).isSpace()) {
                    break;
                }
            }
            r.startColumn = i + 1;
        }
    }
    return r;
}

Range NormalViMode::textObjectInnerParagraph()
{
    Range r;
    KTextEditor::Cursor c1 = findParagraphStart();
    KTextEditor::Cursor c2 = findParagraphEnd();
    updateCursor(c1);

    r.startLine = c1.line();
    r.startColumn = c1.column();
    r.endLine = c2.line();
    r.endColumn = c2.column();
    return r;
}

Range NormalViMode::textObjectAParagraph()
{
    Range r = textObjectInnerParagraph();
    int lines = doc()->lines();

    if (r.endLine + 1 < lines) {
        // If the next line is empty, remove all subsequent empty lines.
        // Otherwise we'll grab the next paragraph.
        if (doc()->line(r.endLine + 1).isEmpty()) {
            for (int i = r.endLine + 1; i < lines && doc()->line(i).isEmpty(); i++) {
                r.endLine++;
            }
            r.endColumn = 0;
        } else {
            KTextEditor::Cursor prev = m_view->cursorPosition();
            KTextEditor::Cursor c(r.endLine + 1, 0);
            updateCursor(c);
            c = findParagraphEnd();
            updateCursor(prev);
            r.endLine = c.line();
            r.endColumn = c.column();
        }
    } else if (doc()->lineLength(r.startLine) > 0) {
        // We went too far, but maybe we can grab previous empty lines.
        for (int i = r.startLine - 1; i >= 0 && doc()->line(i).isEmpty(); i--) {
            r.startLine--;
        }
        r.startColumn = 0;
        updateCursor(KTextEditor::Cursor(r.startLine, r.startColumn));
    } else {
        // We went too far and we're on empty lines, do nothing.
        return Range::invalid();
    }
    return r;
}

Range NormalViMode::textObjectAQuoteDouble()
{
    return findSurroundingQuotes(QLatin1Char('"'), false);
}

Range NormalViMode::textObjectInnerQuoteDouble()
{
    return findSurroundingQuotes(QLatin1Char('"'), true);
}

Range NormalViMode::textObjectAQuoteSingle()
{
    return findSurroundingQuotes(QLatin1Char('\''), false);
}

Range NormalViMode::textObjectInnerQuoteSingle()
{
    return findSurroundingQuotes(QLatin1Char('\''), true);
}

Range NormalViMode::textObjectABackQuote()
{
    return findSurroundingQuotes(QLatin1Char('`'), false);
}

Range NormalViMode::textObjectInnerBackQuote()
{
    return findSurroundingQuotes(QLatin1Char('`'), true);
}

Range NormalViMode::textObjectAParen()
{
    return findSurroundingBrackets(QLatin1Char('('), QLatin1Char(')'), false, QLatin1Char('('), QLatin1Char(')'));
}

Range NormalViMode::textObjectInnerParen()
{
    return findSurroundingBrackets(QLatin1Char('('), QLatin1Char(')'), true, QLatin1Char('('), QLatin1Char(')'));
}

Range NormalViMode::textObjectABracket()
{
    return findSurroundingBrackets(QLatin1Char('['), QLatin1Char(']'), false, QLatin1Char('['), QLatin1Char(']'));
}

Range NormalViMode::textObjectInnerBracket()
{
    return findSurroundingBrackets(QLatin1Char('['), QLatin1Char(']'), true, QLatin1Char('['), QLatin1Char(']'));
}

Range NormalViMode::textObjectACurlyBracket()
{
    return findSurroundingBrackets(QLatin1Char('{'), QLatin1Char('}'), false, QLatin1Char('{'), QLatin1Char('}'));
}

Range NormalViMode::textObjectInnerCurlyBracket()
{
    const Range allBetweenCurlyBrackets = findSurroundingBrackets(QLatin1Char('{'), QLatin1Char('}'), true, QLatin1Char('{'), QLatin1Char('}'));
    // Emulate the behaviour of vim, which tries to leave the closing bracket on its own line
    // if it was originally on a line different to that of the opening bracket.
    Range innerCurlyBracket(allBetweenCurlyBrackets);

    if (innerCurlyBracket.startLine != innerCurlyBracket.endLine) {
        const bool openingBraceIsLastCharOnLine = innerCurlyBracket.startColumn == doc()->line(innerCurlyBracket.startLine).length();
        const bool stuffToDeleteIsAllOnEndLine = openingBraceIsLastCharOnLine && innerCurlyBracket.endLine == innerCurlyBracket.startLine + 1;
        const QString textLeadingClosingBracket = doc()->line(innerCurlyBracket.endLine).mid(0, innerCurlyBracket.endColumn + 1);
        const bool closingBracketHasLeadingNonWhitespace = !textLeadingClosingBracket.trimmed().isEmpty();
        if (stuffToDeleteIsAllOnEndLine) {
            if (!closingBracketHasLeadingNonWhitespace) {
                // Nothing there to select - abort.
                return Range::invalid();
            } else {
                // Shift the beginning of the range to the start of the line containing the closing bracket.
                innerCurlyBracket.startLine++;
                innerCurlyBracket.startColumn = 0;
            }
        } else {
            if (openingBraceIsLastCharOnLine && !closingBracketHasLeadingNonWhitespace) {
                innerCurlyBracket.startLine++;
                innerCurlyBracket.startColumn = 0;
                m_lastMotionWasLinewiseInnerBlock = true;
            }
            {
                // The line containing the end bracket is left alone if the end bracket is preceded by just whitespace,
                // else we need to delete everything (i.e. end up with "{}")
                if (!closingBracketHasLeadingNonWhitespace) {
                    // Shrink the endpoint of the range so that it ends at the end of the line above,
                    // leaving the closing bracket on its own line.
                    innerCurlyBracket.endLine--;
                    innerCurlyBracket.endColumn = doc()->line(innerCurlyBracket.endLine).length();
                }
            }
        }
    }
    return innerCurlyBracket;
}

Range NormalViMode::textObjectAInequalitySign()
{
    return findSurroundingBrackets(QLatin1Char('<'), QLatin1Char('>'), false, QLatin1Char('<'), QLatin1Char('>'));
}

Range NormalViMode::textObjectInnerInequalitySign()
{
    return findSurroundingBrackets(QLatin1Char('<'), QLatin1Char('>'), true, QLatin1Char('<'), QLatin1Char('>'));
}

Range NormalViMode::textObjectAComma()
{
    return textObjectComma(false);
}

Range NormalViMode::textObjectInnerComma()
{
    return textObjectComma(true);
}

QRegularExpression NormalViMode::generateMatchingItemRegex() const
{
    QString pattern(QStringLiteral("\\[|\\]|\\{|\\}|\\(|\\)|"));

    for (QString s : std::as_const(m_matchingItems)) {
        if (s.startsWith(QLatin1Char('-'))) {
            s.remove(0, 1);
        }
        s.replace(QLatin1Char('*'), QStringLiteral("\\*"));
        s.replace(QLatin1Char('+'), QStringLiteral("\\+"));
        s.replace(QLatin1Char('['), QStringLiteral("\\["));
        s.replace(QLatin1Char(']'), QStringLiteral("\\]"));
        s.replace(QLatin1Char('('), QStringLiteral("\\("));
        s.replace(QLatin1Char(')'), QStringLiteral("\\)"));
        s.replace(QLatin1Char('{'), QStringLiteral("\\{"));
        s.replace(QLatin1Char('}'), QStringLiteral("\\}"));

        s.append(QLatin1Char('|'));
        pattern.append(s);
    }
    // remove extra "|" at the end
    pattern.chop(1);

    return QRegularExpression(pattern, QRegularExpression::UseUnicodePropertiesOption);
}

// returns the operation mode that should be used. this is decided by using the following heuristic:
// 1. if we're in visual block mode, it should be Block
// 2. if we're in visual line mode OR the range spans several lines, it should be LineWise
// 3. if neither of these is true, CharWise is returned
// 4. there are some motion that makes all operator charwise, if we have one of them mode will be CharWise
OperationMode NormalViMode::getOperationMode() const
{
    OperationMode m = CharWise;

    if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualBlockMode) {
        m = Block;
    } else if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualLineMode
               || (m_commandRange.startLine != m_commandRange.endLine && m_viInputModeManager->getCurrentViMode() != ViMode::VisualMode)) {
        m = LineWise;
    }

    if (m_commandWithMotion && !m_linewiseCommand) {
        m = CharWise;
    }

    if (m_lastMotionWasLinewiseInnerBlock) {
        m = LineWise;
    }

    return m;
}

bool NormalViMode::paste(PasteLocation pasteLocation, bool isgPaste, bool isIndentedPaste)
{
    KTextEditor::Cursor pasteAt(m_view->cursorPosition());
    KTextEditor::Cursor cursorAfterPaste = pasteAt;
    QChar reg = getChosenRegister(UnnamedRegister);

    OperationMode m = getRegisterFlag(reg);
    QString textToInsert = getRegisterContent(reg);
    const bool isTextMultiLine = textToInsert.count(QLatin1Char('\n')) > 0;

    // In temporary normal mode, p/P act as gp/gP.
    isgPaste |= m_viInputModeManager->getTemporaryNormalMode();

    if (textToInsert.isEmpty()) {
        error(i18n("Nothing in register %1", reg.toLower()));
        return false;
    }

    if (getCount() > 1) {
        textToInsert = textToInsert.repeated(getCount()); // FIXME: does this make sense for blocks?
    }

    if (m == LineWise) {
        pasteAt.setColumn(0);
        if (isIndentedPaste) {
            // Note that this does indeed work if there is no non-whitespace on the current line or if
            // the line is empty!
            static const QRegularExpression nonWhitespaceRegex(QStringLiteral("[^\\s]"));
            const QString pasteLineString = doc()->line(pasteAt.line());
            const QString leadingWhiteSpaceOnCurrentLine = pasteLineString.mid(0, pasteLineString.indexOf(nonWhitespaceRegex));
            const QString leadingWhiteSpaceOnFirstPastedLine = textToInsert.mid(0, textToInsert.indexOf(nonWhitespaceRegex));
            // QString has no "left trim" method, bizarrely.
            while (textToInsert[0].isSpace()) {
                textToInsert = textToInsert.mid(1);
            }
            textToInsert.prepend(leadingWhiteSpaceOnCurrentLine);
            // Remove the last \n, temporarily: we're going to alter the indentation of each pasted line
            // by doing a search and replace on '\n's, but don't want to alter this one.
            textToInsert.chop(1);
            textToInsert.replace(QLatin1Char('\n') + leadingWhiteSpaceOnFirstPastedLine, QLatin1Char('\n') + leadingWhiteSpaceOnCurrentLine);
            textToInsert.append(QLatin1Char('\n')); // Re-add the temporarily removed last '\n'.
        }
        if (pasteLocation == AfterCurrentPosition) {
            textToInsert.chop(1); // remove the last \n
            pasteAt.setColumn(doc()->lineLength(pasteAt.line())); // paste after the current line and ...
            textToInsert.prepend(QLatin1Char('\n')); // ... prepend a \n, so the text starts on a new line

            cursorAfterPaste.setLine(cursorAfterPaste.line() + 1);
        }
        if (isgPaste) {
            cursorAfterPaste.setLine(cursorAfterPaste.line() + textToInsert.count(QLatin1Char('\n')));
        }
    } else {
        if (pasteLocation == AfterCurrentPosition) {
            // Move cursor forward one before we paste.  The position after the paste must also
            // be updated accordingly.
            if (getLine(pasteAt.line()).length() > 0) {
                pasteAt.setColumn(pasteAt.column() + 1);
            }
            cursorAfterPaste = pasteAt;
        }
        const bool leaveCursorAtStartOfPaste = isTextMultiLine && !isgPaste;
        if (!leaveCursorAtStartOfPaste) {
            cursorAfterPaste = cursorPosAtEndOfPaste(pasteAt, textToInsert);
            if (!isgPaste) {
                cursorAfterPaste.setColumn(cursorAfterPaste.column() - 1);
            }
        }
    }

    doc()->editStart();
    if (m_view->selection()) {
        pasteAt = m_view->selectionRange().start();
        doc()->removeText(m_view->selectionRange());
    }
    doc()->insertText(pasteAt, textToInsert, m == Block);
    doc()->editEnd();

    if (cursorAfterPaste.line() >= doc()->lines()) {
        cursorAfterPaste.setLine(doc()->lines() - 1);
    }
    updateCursor(cursorAfterPaste);

    return true;
}

KTextEditor::Cursor NormalViMode::cursorPosAtEndOfPaste(const KTextEditor::Cursor pasteLocation, const QString &pastedText)
{
    KTextEditor::Cursor cAfter = pasteLocation;
    const int lineCount = pastedText.count(QLatin1Char('\n')) + 1;
    if (lineCount == 1) {
        cAfter.setColumn(cAfter.column() + pastedText.length());
    } else {
        const int lastLineLength = pastedText.size() - (pastedText.lastIndexOf(QLatin1Char('\n')) + 1);
        cAfter.setColumn(lastLineLength);
        cAfter.setLine(cAfter.line() + lineCount - 1);
    }
    return cAfter;
}

void NormalViMode::joinLines(unsigned int from, unsigned int to) const
{
    // make sure we don't try to join lines past the document end
    if (to >= (unsigned int)(doc()->lines())) {
        to = doc()->lines() - 1;
    }

    // joining one line is a no-op
    if (from == to) {
        return;
    }

    doc()->joinLines(from, to);
}

void NormalViMode::reformatLines(unsigned int from, unsigned int to) const
{
    // BUG #340550: Do not remove empty lines when reformatting
    KTextEditor::DocumentPrivate *document = doc();
    auto isNonEmptyLine = [](QStringView text) {
        for (int i = 0; i < text.length(); ++i) {
            if (!text.at(i).isSpace()) {
                return true;
            }
        }

        return false;
    };
    for (; from < to; ++from) {
        if (isNonEmptyLine(document->line(from))) {
            break;
        }
    }
    for (; to > from; --to) {
        if (isNonEmptyLine(document->line(to))) {
            break;
        }
    }

    document->editStart();
    joinLines(from, to);
    document->wrapText(from, to);
    document->editEnd();
}

int NormalViMode::getFirstNonBlank(int line) const
{
    if (line < 0) {
        line = m_view->cursorPosition().line();
    }

    // doc()->plainKateTextLine returns NULL if the line is out of bounds.
    Kate::TextLine l = doc()->plainKateTextLine(line);
    int c = l.firstChar();
    return (c < 0) ? 0 : c;
}

// Tries to shrinks toShrink so that it fits tightly around rangeToShrinkTo.
void NormalViMode::shrinkRangeAroundCursor(Range &toShrink, const Range &rangeToShrinkTo) const
{
    if (!toShrink.valid || !rangeToShrinkTo.valid) {
        return;
    }
    KTextEditor::Cursor cursorPos = m_view->cursorPosition();
    if (rangeToShrinkTo.startLine >= cursorPos.line()) {
        if (rangeToShrinkTo.startLine > cursorPos.line()) {
            // Does not surround cursor; aborting.
            return;
        }
        Q_ASSERT(rangeToShrinkTo.startLine == cursorPos.line());
        if (rangeToShrinkTo.startColumn > cursorPos.column()) {
            // Does not surround cursor; aborting.
            return;
        }
    }
    if (rangeToShrinkTo.endLine <= cursorPos.line()) {
        if (rangeToShrinkTo.endLine < cursorPos.line()) {
            // Does not surround cursor; aborting.
            return;
        }
        Q_ASSERT(rangeToShrinkTo.endLine == cursorPos.line());
        if (rangeToShrinkTo.endColumn < cursorPos.column()) {
            // Does not surround cursor; aborting.
            return;
        }
    }

    if (toShrink.startLine <= rangeToShrinkTo.startLine) {
        if (toShrink.startLine < rangeToShrinkTo.startLine) {
            toShrink.startLine = rangeToShrinkTo.startLine;
            toShrink.startColumn = rangeToShrinkTo.startColumn;
        }
        Q_ASSERT(toShrink.startLine == rangeToShrinkTo.startLine);
        if (toShrink.startColumn < rangeToShrinkTo.startColumn) {
            toShrink.startColumn = rangeToShrinkTo.startColumn;
        }
    }
    if (toShrink.endLine >= rangeToShrinkTo.endLine) {
        if (toShrink.endLine > rangeToShrinkTo.endLine) {
            toShrink.endLine = rangeToShrinkTo.endLine;
            toShrink.endColumn = rangeToShrinkTo.endColumn;
        }
        Q_ASSERT(toShrink.endLine == rangeToShrinkTo.endLine);
        if (toShrink.endColumn > rangeToShrinkTo.endColumn) {
            toShrink.endColumn = rangeToShrinkTo.endColumn;
        }
    }
}

Range NormalViMode::textObjectComma(bool inner) const
{
    // Basic algorithm: look left and right of the cursor for all combinations
    // of enclosing commas and the various types of brackets, and pick the pair
    // closest to the cursor that surrounds the cursor.
    Range r(0, 0, m_view->doc()->lines(), m_view->doc()->line(m_view->doc()->lastLine()).length(), InclusiveMotion);

    shrinkRangeAroundCursor(r, findSurroundingQuotes(QLatin1Char(','), inner));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char('('), QLatin1Char(')'), inner, QLatin1Char('('), QLatin1Char(')')));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char('{'), QLatin1Char('}'), inner, QLatin1Char('{'), QLatin1Char('}')));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char(','), QLatin1Char(')'), inner, QLatin1Char('('), QLatin1Char(')')));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char(','), QLatin1Char(']'), inner, QLatin1Char('['), QLatin1Char(']')));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char(','), QLatin1Char('}'), inner, QLatin1Char('{'), QLatin1Char('}')));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char('('), QLatin1Char(','), inner, QLatin1Char('('), QLatin1Char(')')));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char('['), QLatin1Char(','), inner, QLatin1Char('['), QLatin1Char(']')));
    shrinkRangeAroundCursor(r, findSurroundingBrackets(QLatin1Char('{'), QLatin1Char(','), inner, QLatin1Char('{'), QLatin1Char('}')));
    return r;
}

void NormalViMode::updateYankHighlightAttrib()
{
    if (!m_highlightYankAttribute) {
        m_highlightYankAttribute = new KTextEditor::Attribute;
    }
    const QColor &yankedColor = m_view->rendererConfig()->savedLineColor();
    m_highlightYankAttribute->setBackground(yankedColor);
    KTextEditor::Attribute::Ptr mouseInAttribute(new KTextEditor::Attribute());
    mouseInAttribute->setFontBold(true);
    m_highlightYankAttribute->setDynamicAttribute(KTextEditor::Attribute::ActivateMouseIn, mouseInAttribute);
    m_highlightYankAttribute->dynamicAttribute(KTextEditor::Attribute::ActivateMouseIn)->setBackground(yankedColor);
}

void NormalViMode::highlightYank(const Range &range, const OperationMode mode)
{
    clearYankHighlight();

    // current MovingRange doesn't support block mode selection so split the
    // block range into per-line ranges
    if (mode == Block) {
        for (int i = range.startLine; i <= range.endLine; i++) {
            addHighlightYank(KTextEditor::Range(i, range.startColumn, i, range.endColumn));
        }
    } else {
        addHighlightYank(KTextEditor::Range(range.startLine, range.startColumn, range.endLine, range.endColumn));
    }
}

void NormalViMode::addHighlightYank(KTextEditor::Range yankRange)
{
    KTextEditor::MovingRange *highlightedYank = m_view->doc()->newMovingRange(yankRange, Kate::TextRange::DoNotExpand);
    highlightedYank->setView(m_view); // show only in this view
    highlightedYank->setAttributeOnlyForViews(true);
    // use z depth defined in moving ranges interface
    highlightedYank->setZDepth(-10000.0);
    highlightedYank->setAttribute(m_highlightYankAttribute);

    highlightedYankForDocument().insert(highlightedYank);
}

void NormalViMode::clearYankHighlight()
{
    QSet<KTextEditor::MovingRange *> &pHighlightedYanks = highlightedYankForDocument();
    qDeleteAll(pHighlightedYanks);
    pHighlightedYanks.clear();
}

void NormalViMode::aboutToDeleteMovingInterfaceContent()
{
    QSet<KTextEditor::MovingRange *> &pHighlightedYanks = highlightedYankForDocument();
    // Prevent double-deletion in case this NormalMode is deleted.
    pHighlightedYanks.clear();
}

QSet<KTextEditor::MovingRange *> &NormalViMode::highlightedYankForDocument()
{
    // Work around the fact that both Normal and Visual mode will have their own m_highlightedYank -
    // make Normal's the canonical one.
    return m_viInputModeManager->getViNormalMode()->m_highlightedYanks;
}

bool NormalViMode::waitingForRegisterOrCharToSearch()
{
    // r, q, @ are never preceded by operators. There will always be a keys size of 1 for them.
    // f, t, F, T can be preceded by a delete/replace/yank/indent operator. size = 2 in that case.
    // f, t, F, T can be preceded by 'g' case/formatting operators. size = 3 in that case.
    const int keysSize = m_keys.size();
    if (keysSize < 1) {
        // Just being defensive there.
        return false;
    }
    if (keysSize > 1) {
        // Multi-letter operation.
        QChar cPrefix = m_keys[0];
        if (keysSize == 2) {
            // delete/replace/yank/indent operator?
            if (cPrefix != QLatin1Char('c') && cPrefix != QLatin1Char('d') && cPrefix != QLatin1Char('y') && cPrefix != QLatin1Char('=')
                && cPrefix != QLatin1Char('>') && cPrefix != QLatin1Char('<')) {
                return false;
            }
        } else if (keysSize == 3) {
            // We need to look deeper. Is it a g motion?
            QChar cNextfix = m_keys[1];
            if (cPrefix != QLatin1Char('g')
                || (cNextfix != QLatin1Char('U') && cNextfix != QLatin1Char('u') && cNextfix != QLatin1Char('~') && cNextfix != QLatin1Char('q')
                    && cNextfix != QLatin1Char('w') && cNextfix != QLatin1Char('@'))) {
                return false;
            }
        } else {
            return false;
        }
    }

    QChar ch = m_keys[keysSize - 1];
    return (ch == QLatin1Char('f') || ch == QLatin1Char('t') || ch == QLatin1Char('F')
            || ch == QLatin1Char('T')
            // c/d prefix unapplicable for the following cases.
            || (keysSize == 1 && (ch == QLatin1Char('r') || ch == QLatin1Char('q') || ch == QLatin1Char('@'))));
}

void NormalViMode::textInserted(KTextEditor::Document *document, KTextEditor::Range range)
{
    if (m_viInputModeManager->view()->viewInputMode() != KTextEditor::View::ViInputMode) {
        return;
    }

    Q_UNUSED(document)
    const bool isInsertReplaceMode =
        (m_viInputModeManager->getCurrentViMode() == ViMode::InsertMode || m_viInputModeManager->getCurrentViMode() == ViMode::ReplaceMode);
    const bool continuesInsertion = range.start().line() == m_currentChangeEndMarker.line() && range.start().column() == m_currentChangeEndMarker.column();
    const bool beginsWithNewline = doc()->text(range).at(0) == QLatin1Char('\n');
    if (!continuesInsertion) {
        KTextEditor::Cursor newBeginMarkerPos = range.start();
        if (beginsWithNewline && !isInsertReplaceMode) {
            // Presumably a linewise paste, in which case we ignore the leading '\n'
            newBeginMarkerPos = KTextEditor::Cursor(newBeginMarkerPos.line() + 1, 0);
        }
        m_viInputModeManager->marks()->setStartEditYanked(newBeginMarkerPos);
    }
    m_viInputModeManager->marks()->setLastChange(range.start());
    KTextEditor::Cursor editEndMarker = range.end();
    if (!isInsertReplaceMode) {
        editEndMarker.setColumn(editEndMarker.column() - 1);
    }
    m_viInputModeManager->marks()->setFinishEditYanked(editEndMarker);
    m_currentChangeEndMarker = range.end();
    if (m_isUndo) {
        const bool addsMultipleLines = range.start().line() != range.end().line();
        m_viInputModeManager->marks()->setStartEditYanked(KTextEditor::Cursor(m_viInputModeManager->marks()->getStartEditYanked().line(), 0));
        if (addsMultipleLines) {
            m_viInputModeManager->marks()->setFinishEditYanked(KTextEditor::Cursor(m_viInputModeManager->marks()->getFinishEditYanked().line() + 1, 0));
            m_viInputModeManager->marks()->setLastChange(KTextEditor::Cursor(m_viInputModeManager->marks()->getLastChange().line() + 1, 0));
        } else {
            m_viInputModeManager->marks()->setFinishEditYanked(KTextEditor::Cursor(m_viInputModeManager->marks()->getFinishEditYanked().line(), 0));
            m_viInputModeManager->marks()->setLastChange(KTextEditor::Cursor(m_viInputModeManager->marks()->getLastChange().line(), 0));
        }
    }
}

void NormalViMode::textRemoved(KTextEditor::Document *document, KTextEditor::Range range)
{
    if (m_viInputModeManager->view()->viewInputMode() != KTextEditor::View::ViInputMode) {
        return;
    }

    Q_UNUSED(document);
    const bool isInsertReplaceMode =
        (m_viInputModeManager->getCurrentViMode() == ViMode::InsertMode || m_viInputModeManager->getCurrentViMode() == ViMode::ReplaceMode);
    m_viInputModeManager->marks()->setLastChange(range.start());
    if (!isInsertReplaceMode) {
        // Don't go resetting [ just because we did a Ctrl-h!
        m_viInputModeManager->marks()->setStartEditYanked(range.start());
    } else {
        // Don't go disrupting our continued insertion just because we did a Ctrl-h!
        m_currentChangeEndMarker = range.start();
    }
    m_viInputModeManager->marks()->setFinishEditYanked(range.start());
    if (m_isUndo) {
        // Slavishly follow Vim's weird rules: if an undo removes several lines, then all markers should
        // be at the beginning of the line after the last line removed, else they should at the beginning
        // of the line above that.
        const int markerLineAdjustment = (range.start().line() != range.end().line()) ? 1 : 0;
        m_viInputModeManager->marks()->setStartEditYanked(
            KTextEditor::Cursor(m_viInputModeManager->marks()->getStartEditYanked().line() + markerLineAdjustment, 0));
        m_viInputModeManager->marks()->setFinishEditYanked(
            KTextEditor::Cursor(m_viInputModeManager->marks()->getFinishEditYanked().line() + markerLineAdjustment, 0));
        m_viInputModeManager->marks()->setLastChange(KTextEditor::Cursor(m_viInputModeManager->marks()->getLastChange().line() + markerLineAdjustment, 0));
    }
}

void NormalViMode::undoBeginning()
{
    m_isUndo = true;
}

void NormalViMode::undoEnded()
{
    m_isUndo = false;
}

bool NormalViMode::executeKateCommand(const QString &command)
{
    KTextEditor::Command *p = KateCmd::self()->queryCommand(command);

    if (!p) {
        return false;
    }

    QString msg;
    return p->exec(m_view, command, msg);
}

#define ADDCMD(STR, FUNC, FLGS) Command(QStringLiteral(STR), &NormalViMode::FUNC, FLGS)

#define ADDMOTION(STR, FUNC, FLGS) Motion(QStringLiteral(STR), &NormalViMode::FUNC, FLGS)

const std::vector<Command> &NormalViMode::commands()
{
    // init once, is expensive
    static const std::vector<Command> global{
        ADDCMD("a", commandEnterInsertModeAppend, IS_CHANGE),
        ADDCMD("A", commandEnterInsertModeAppendEOL, IS_CHANGE),
        ADDCMD("i", commandEnterInsertMode, IS_CHANGE),
        ADDCMD("<insert>", commandEnterInsertMode, IS_CHANGE),
        ADDCMD("I", commandEnterInsertModeBeforeFirstNonBlankInLine, IS_CHANGE),
        ADDCMD("gi", commandEnterInsertModeLast, IS_CHANGE),
        ADDCMD("v", commandEnterVisualMode, 0),
        ADDCMD("V", commandEnterVisualLineMode, 0),
        ADDCMD("<c-v>", commandEnterVisualBlockMode, 0),
        ADDCMD("gv", commandReselectVisual, SHOULD_NOT_RESET),
        ADDCMD("o", commandOpenNewLineUnder, IS_CHANGE),
        ADDCMD("O", commandOpenNewLineOver, IS_CHANGE),
        ADDCMD("J", commandJoinLines, IS_CHANGE),
        ADDCMD("c", commandChange, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("C", commandChangeToEOL, IS_CHANGE),
        ADDCMD("cc", commandChangeLine, IS_CHANGE),
        ADDCMD("s", commandSubstituteChar, IS_CHANGE),
        ADDCMD("S", commandSubstituteLine, IS_CHANGE),
        ADDCMD("dd", commandDeleteLine, IS_CHANGE),
        ADDCMD("d", commandDelete, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("D", commandDeleteToEOL, IS_CHANGE),
        ADDCMD("x", commandDeleteChar, IS_CHANGE),
        ADDCMD("<delete>", commandDeleteChar, IS_CHANGE),
        ADDCMD("X", commandDeleteCharBackward, IS_CHANGE),
        ADDCMD("gu", commandMakeLowercase, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("guu", commandMakeLowercaseLine, IS_CHANGE),
        ADDCMD("gU", commandMakeUppercase, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("gUU", commandMakeUppercaseLine, IS_CHANGE),
        ADDCMD("y", commandYank, NEEDS_MOTION),
        ADDCMD("yy", commandYankLine, 0),
        ADDCMD("Y", commandYankToEOL, 0),
        ADDCMD("p", commandPaste, IS_CHANGE),
        ADDCMD("P", commandPasteBefore, IS_CHANGE),
        ADDCMD("gp", commandgPaste, IS_CHANGE),
        ADDCMD("gP", commandgPasteBefore, IS_CHANGE),
        ADDCMD("]p", commandIndentedPaste, IS_CHANGE),
        ADDCMD("[p", commandIndentedPasteBefore, IS_CHANGE),
        ADDCMD("r.", commandReplaceCharacter, IS_CHANGE | REGEX_PATTERN),
        ADDCMD("R", commandEnterReplaceMode, IS_CHANGE),
        ADDCMD(":", commandSwitchToCmdLine, 0),
        ADDCMD("u", commandUndo, 0),
        ADDCMD("<c-r>", commandRedo, 0),
        ADDCMD("U", commandRedo, 0),
        ADDCMD("m.", commandSetMark, REGEX_PATTERN),
        ADDCMD(">>", commandIndentLine, IS_CHANGE),
        ADDCMD("<<", commandUnindentLine, IS_CHANGE),
        ADDCMD(">", commandIndentLines, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("<", commandUnindentLines, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("<c-f>", commandScrollPageDown, 0),
        ADDCMD("<pagedown>", commandScrollPageDown, 0),
        ADDCMD("<c-b>", commandScrollPageUp, 0),
        ADDCMD("<pageup>", commandScrollPageUp, 0),
        ADDCMD("<c-u>", commandScrollHalfPageUp, 0),
        ADDCMD("<c-d>", commandScrollHalfPageDown, 0),
        ADDCMD("z.", commandCenterViewOnNonBlank, 0),
        ADDCMD("zz", commandCenterViewOnCursor, 0),
        ADDCMD("z<return>", commandTopViewOnNonBlank, 0),
        ADDCMD("zt", commandTopViewOnCursor, 0),
        ADDCMD("z-", commandBottomViewOnNonBlank, 0),
        ADDCMD("zb", commandBottomViewOnCursor, 0),
        ADDCMD("ga", commandPrintCharacterCode, SHOULD_NOT_RESET),
        ADDCMD(".", commandRepeatLastChange, 0),
        ADDCMD("==", commandAlignLine, IS_CHANGE),
        ADDCMD("=", commandAlignLines, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("~", commandChangeCase, IS_CHANGE),
        ADDCMD("g~", commandChangeCaseRange, IS_CHANGE | NEEDS_MOTION),
        ADDCMD("g~~", commandChangeCaseLine, IS_CHANGE),
        ADDCMD("<c-a>", commandAddToNumber, IS_CHANGE),
        ADDCMD("<c-x>", commandSubtractFromNumber, IS_CHANGE),
        ADDCMD("<c-o>", commandGoToPrevJump, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDCMD("<c-i>", commandGoToNextJump, CAN_LAND_INSIDE_FOLDING_RANGE),

        ADDCMD("<c-w>h", commandSwitchToLeftView, 0),
        ADDCMD("<c-w><c-h>", commandSwitchToLeftView, 0),
        ADDCMD("<c-w><left>", commandSwitchToLeftView, 0),
        ADDCMD("<c-w>j", commandSwitchToDownView, 0),
        ADDCMD("<c-w><c-j>", commandSwitchToDownView, 0),
        ADDCMD("<c-w><down>", commandSwitchToDownView, 0),
        ADDCMD("<c-w>k", commandSwitchToUpView, 0),
        ADDCMD("<c-w><c-k>", commandSwitchToUpView, 0),
        ADDCMD("<c-w><up>", commandSwitchToUpView, 0),
        ADDCMD("<c-w>l", commandSwitchToRightView, 0),
        ADDCMD("<c-w><c-l>", commandSwitchToRightView, 0),
        ADDCMD("<c-w><right>", commandSwitchToRightView, 0),
        ADDCMD("<c-w>w", commandSwitchToNextView, 0),
        ADDCMD("<c-w><c-w>", commandSwitchToNextView, 0),

        ADDCMD("<c-w>s", commandSplitHoriz, 0),
        ADDCMD("<c-w>S", commandSplitHoriz, 0),
        ADDCMD("<c-w><c-s>", commandSplitHoriz, 0),
        ADDCMD("<c-w>v", commandSplitVert, 0),
        ADDCMD("<c-w><c-v>", commandSplitVert, 0),
        ADDCMD("<c-w>c", commandCloseView, 0),

        ADDCMD("gt", commandSwitchToNextTab, 0),
        ADDCMD("gT", commandSwitchToPrevTab, 0),

        ADDCMD("gqq", commandFormatLine, IS_CHANGE),
        ADDCMD("gq", commandFormatLines, IS_CHANGE | NEEDS_MOTION),

        ADDCMD("zo", commandExpandLocal, 0),
        ADDCMD("zc", commandCollapseLocal, 0),
        ADDCMD("za", commandToggleRegionVisibility, 0),
        ADDCMD("zr", commandExpandAll, 0),
        ADDCMD("zm", commandCollapseToplevelNodes, 0),

        ADDCMD("q.", commandStartRecordingMacro, REGEX_PATTERN),
        ADDCMD("@.", commandReplayMacro, REGEX_PATTERN),

        ADDCMD("ZZ", commandCloseWrite, 0),
        ADDCMD("ZQ", commandCloseNocheck, 0),
    };
    return global;
}

const std::vector<Motion> &NormalViMode::motions()
{
    // init once, is expensive
    static const std::vector<Motion> global{
        // regular motions
        ADDMOTION("h", motionLeft, 0),
        ADDMOTION("<left>", motionLeft, 0),
        ADDMOTION("<backspace>", motionLeft, 0),
        ADDMOTION("j", motionDown, 0),
        ADDMOTION("<down>", motionDown, 0),
        ADDMOTION("<enter>", motionDownToFirstNonBlank, 0),
        ADDMOTION("<return>", motionDownToFirstNonBlank, 0),
        ADDMOTION("k", motionUp, 0),
        ADDMOTION("<up>", motionUp, 0),
        ADDMOTION("-", motionUpToFirstNonBlank, 0),
        ADDMOTION("+", motionDownToFirstNonBlank, 0),
        ADDMOTION("l", motionRight, 0),
        ADDMOTION("<right>", motionRight, 0),
        ADDMOTION(" ", motionRight, 0),
        ADDMOTION("$", motionToEOL, 0),
        ADDMOTION("g_", motionToLastNonBlank, 0),
        ADDMOTION("<end>", motionToEOL, 0),
        ADDMOTION("0", motionToColumn0, 0),
        ADDMOTION("<home>", motionToColumn0, 0),
        ADDMOTION("^", motionToFirstCharacterOfLine, 0),
        ADDMOTION("f.", motionFindChar, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("F.", motionFindCharBackward, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("t.", motionToChar, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("T.", motionToCharBackward, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION(";", motionRepeatlastTF, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION(",", motionRepeatlastTFBackward, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("n", motionFindNext, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("N", motionFindPrev, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("gg", motionToLineFirst, 0),
        ADDMOTION("G", motionToLineLast, 0),
        ADDMOTION("w", motionWordForward, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("W", motionWORDForward, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("<c-right>", motionWordForward, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("<c-left>", motionWordBackward, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("b", motionWordBackward, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("B", motionWORDBackward, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("e", motionToEndOfWord, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("E", motionToEndOfWORD, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("ge", motionToEndOfPrevWord, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("gE", motionToEndOfPrevWORD, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("|", motionToScreenColumn, 0),
        ADDMOTION("%", motionToMatchingItem, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("`[a-zA-Z^><\\.\\[\\]]", motionToMark, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("'[a-zA-Z^><]", motionToMarkLine, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("[[", motionToPreviousBraceBlockStart, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("]]", motionToNextBraceBlockStart, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("[]", motionToPreviousBraceBlockEnd, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("][", motionToNextBraceBlockEnd, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("*", motionToNextOccurrence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("#", motionToPrevOccurrence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("H", motionToFirstLineOfWindow, 0),
        ADDMOTION("M", motionToMiddleLineOfWindow, 0),
        ADDMOTION("L", motionToLastLineOfWindow, 0),
        ADDMOTION("gj", motionToNextVisualLine, 0),
        ADDMOTION("g<down>", motionToNextVisualLine, 0),
        ADDMOTION("gk", motionToPrevVisualLine, 0),
        ADDMOTION("g<up>", motionToPrevVisualLine, 0),
        ADDMOTION("(", motionToPreviousSentence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION(")", motionToNextSentence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("{", motionToBeforeParagraph, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("}", motionToAfterParagraph, CAN_LAND_INSIDE_FOLDING_RANGE),

        // text objects
        ADDMOTION("iw", textObjectInnerWord, 0),
        ADDMOTION("aw", textObjectAWord, IS_NOT_LINEWISE),
        ADDMOTION("iW", textObjectInnerWORD, 0),
        ADDMOTION("aW", textObjectAWORD, IS_NOT_LINEWISE),
        ADDMOTION("is", textObjectInnerSentence, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("as", textObjectASentence, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("ip", textObjectInnerParagraph, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("ap", textObjectAParagraph, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i\"", textObjectInnerQuoteDouble, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a\"", textObjectAQuoteDouble, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i'", textObjectInnerQuoteSingle, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a'", textObjectAQuoteSingle, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i`", textObjectInnerBackQuote, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a`", textObjectABackQuote, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[()b]", textObjectInnerParen, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a[()b]", textObjectAParen, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[{}B]", textObjectInnerCurlyBracket, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a[{}B]", textObjectACurlyBracket, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[><]", textObjectInnerInequalitySign, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a[><]", textObjectAInequalitySign, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[\\[\\]]", textObjectInnerBracket, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a[\\[\\]]", textObjectABracket, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i,", textObjectInnerComma, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a,", textObjectAComma, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),

        ADDMOTION("/<enter>", motionToIncrementalSearchMatch, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("?<enter>", motionToIncrementalSearchMatch, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
    };
    return global;
}
