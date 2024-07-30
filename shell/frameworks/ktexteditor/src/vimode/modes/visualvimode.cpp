/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateview.h"
#include <document/katedocument.h>
#include <inputmode/kateviinputmode.h>

#include <vimode/inputmodemanager.h>
#include <vimode/marks.h>
#include <vimode/modes/visualvimode.h>
#include <vimode/motion.h>
#include <vimode/range.h>

using namespace KateVi;

VisualViMode::VisualViMode(InputModeManager *viInputModeManager, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal)
    : NormalViMode(viInputModeManager, view, viewInternal)
{
    m_start.setPosition(-1, -1);
    m_mode = ViMode::VisualMode;

    connect(m_view, &KTextEditor::ViewPrivate::selectionChanged, this, &VisualViMode::updateSelection);
}

void VisualViMode::selectInclusive(const KTextEditor::Cursor c1, const KTextEditor::Cursor c2)
{
    if (c1 >= c2) {
        m_view->setSelection(KTextEditor::Range(c1.line(), c1.column() + 1, c2.line(), c2.column()));
    } else {
        m_view->setSelection(KTextEditor::Range(c1.line(), c1.column(), c2.line(), c2.column() + 1));
    }
}

void VisualViMode::selectBlockInclusive(const KTextEditor::Cursor c1, const KTextEditor::Cursor c2)
{
    m_view->setBlockSelection(true);

    if (c1.column() >= c2.column()) {
        m_view->setSelection(KTextEditor::Range(c1.line(), c1.column() + 1, c2.line(), c2.column()));
    } else {
        m_view->setSelection(KTextEditor::Range(c1.line(), c1.column(), c2.line(), c2.column() + 1));
    }
}

void VisualViMode::selectLines(KTextEditor::Range range)
{
    int sline = qMin(range.start().line(), range.end().line());
    int eline = qMax(range.start().line(), range.end().line());
    int ecol = m_view->doc()->lineLength(eline) + 1;

    m_view->setSelection(KTextEditor::Range(KTextEditor::Cursor(sline, 0), KTextEditor::Cursor(eline, ecol)));
}

void VisualViMode::goToPos(const Range &r)
{
    KTextEditor::Cursor c = m_view->cursorPosition();

    if (r.startLine != -1 && r.startColumn != -1 && c == m_start) {
        m_start.setLine(r.startLine);
        m_start.setColumn(r.startColumn);
        c.setLine(r.endLine);
        c.setColumn(r.endColumn);
    } else if (r.startLine != -1 && r.startColumn != -1 && m_motionCanChangeWholeVisualModeSelection) {
        const KTextEditor::Cursor textObjectBegin(r.startLine, r.startColumn);
        if (textObjectBegin < m_start) {
            m_start.setLine(r.startLine);
            m_start.setColumn(r.startColumn);
            c.setLine(r.endLine);
            c.setColumn(r.endColumn);
        }
    } else {
        c.setLine(r.endLine);
        c.setColumn(r.endColumn);
    }

    if (c.line() >= doc()->lines()) {
        c.setLine(doc()->lines() - 1);
    }

    updateCursor(c);

    // Setting range for a command
    m_commandRange = Range(m_start, c, m_commandRange.motionType);

    // If visual mode is blockwise
    if (isVisualBlock()) {
        selectBlockInclusive(m_start, c);

        // Need to correct command range to make it inclusive.
        if ((c.line() < m_start.line() && c.column() > m_start.column()) || (c.line() > m_start.line() && c.column() < m_start.column())) {
            qSwap(m_commandRange.endColumn, m_commandRange.startColumn);
        }
        return;
    } else {
        m_view->setBlockSelection(false);
    }

    // If visual mode is linewise
    if (isVisualLine()) {
        selectLines(KTextEditor::Range(m_start, c));
        return;
    }

    // If visual mode is charwise
    selectInclusive(m_start, c);
}

void VisualViMode::reset()
{
    m_mode = ViMode::VisualMode;

    // only switch to normal mode if still in visual mode. commands like c, s, ...
    // can have switched to insert mode
    if (m_viInputModeManager->isAnyVisualMode()) {
        saveRangeMarks();
        m_lastVisualMode = m_viInputModeManager->getCurrentViMode();

        // Return the cursor back to start of selection after.
        if (!m_pendingResetIsDueToExit) {
            KTextEditor::Cursor c = m_view->cursorPosition();
            if (m_start.line() != -1 && m_start.column() != -1) {
                if (m_viInputModeManager->getCurrentViMode() == ViMode::VisualLineMode) {
                    if (m_start.line() < c.line()) {
                        updateCursor(KTextEditor::Cursor(m_start.line(), 0));
                        m_stickyColumn = -1;
                    }
                } else {
                    updateCursor(qMin(m_start, c));
                    m_stickyColumn = -1;
                }
            }
        }

        if (m_viInputModeManager->getPreviousViMode() == ViMode::InsertMode) {
            startInsertMode();
        } else {
            startNormalMode();
        }
    }

    if (!m_commandShouldKeepSelection) {
        m_view->removeSelection();
    } else {
        m_commandShouldKeepSelection = false;
    }

    m_start.setPosition(-1, -1);
    m_pendingResetIsDueToExit = false;
}

void VisualViMode::saveRangeMarks()
{
    // DO NOT save these marks if the
    // action that exited visual mode deleted the selection
    if (m_deleteCommand == false) {
        m_viInputModeManager->marks()->setSelectionStart(m_start);
        m_viInputModeManager->marks()->setSelectionFinish(m_view->cursorPosition());
    }
}

void VisualViMode::init()
{
    // when using "gv" we already have a start position
    if (!m_start.isValid()) {
        m_start = m_view->cursorPosition();
    }

    if (isVisualLine()) {
        KTextEditor::Cursor c = m_view->cursorPosition();
        selectLines(KTextEditor::Range(c, c));
    }

    m_commandRange = Range(m_start, m_start, m_commandRange.motionType);
}

void VisualViMode::setVisualModeType(ViMode mode)
{
    Q_ASSERT(mode == ViMode::VisualMode || mode == ViMode::VisualLineMode || mode == ViMode::VisualBlockMode);
    m_mode = mode;
}

void VisualViMode::switchStartEnd()
{
    KTextEditor::Cursor c = m_start;
    m_start = m_view->cursorPosition();

    updateCursor(c);

    m_stickyColumn = -1;
}

void VisualViMode::goToPos(const KTextEditor::Cursor c)
{
    Range r(c, InclusiveMotion);
    goToPos(r);
}

void VisualViMode::updateSelection()
{
    if (!m_viInputModeManager->inputAdapter()->isActive()) {
        return;
    }
    if (m_viInputModeManager->isHandlingKeypress() && !m_isUndo) {
        return;
    }

    // If we are there it's already not VisualBlock mode.
    m_view->setBlockSelection(false);

    // If not valid going back to normal mode
    KTextEditor::Range r = m_view->selectionRange();
    if (!r.isValid()) {
        // Don't screw up the cursor's position. See BUG #337286.
        m_pendingResetIsDueToExit = true;
        reset();
        return;
    }

    // If already not in visual mode, it's time to go there.
    if (m_viInputModeManager->getCurrentViMode() != ViMode::VisualMode) {
        commandEnterVisualMode();
    }

    // Set range for commands
    m_start = (m_view->cursorPosition() == r.start()) ? r.end() : r.start();
    m_commandRange = Range(r.start(), r.end(), m_commandRange.motionType);
    // The end of the range seems to be one space forward of where it should be.
    m_commandRange.endColumn--;
}

#define ADDCMD(STR, FUNC, FLGS) Command(QStringLiteral(STR), &NormalViMode::FUNC, FLGS)

#define ADDMOTION(STR, FUNC, FLGS) Motion(QStringLiteral(STR), &NormalViMode::FUNC, FLGS)

const std::vector<Command> &VisualViMode::commands()
{
    // init once, is expensive
    static std::vector<Command> global{
        ADDCMD("J", commandJoinLines, IS_CHANGE),
        ADDCMD("c", commandChange, IS_CHANGE),
        ADDCMD("s", commandChange, IS_CHANGE),
        ADDCMD("C", commandChangeToEOL, IS_CHANGE),
        ADDCMD("S", commandChangeToEOL, IS_CHANGE),
        ADDCMD("d", commandDelete, IS_CHANGE),
        ADDCMD("<delete>", commandDelete, IS_CHANGE),
        ADDCMD("D", commandDeleteToEOL, IS_CHANGE),
        ADDCMD("x", commandDeleteChar, IS_CHANGE),
        ADDCMD("X", commandDeleteCharBackward, IS_CHANGE),
        ADDCMD("gu", commandMakeLowercase, IS_CHANGE),
        ADDCMD("u", commandMakeLowercase, IS_CHANGE),
        ADDCMD("gU", commandMakeUppercase, IS_CHANGE),
        ADDCMD("g~", commandChangeCaseRange, IS_CHANGE),
        ADDCMD("U", commandMakeUppercase, IS_CHANGE),
        ADDCMD("y", commandYank, 0),
        ADDCMD("Y", commandYankToEOL, 0),
        ADDCMD("p", commandPaste, IS_CHANGE),
        ADDCMD("P", commandPasteBefore, IS_CHANGE),
        ADDCMD("r.", commandReplaceCharacter, IS_CHANGE | REGEX_PATTERN),
        ADDCMD(":", commandSwitchToCmdLine, SHOULD_NOT_RESET),
        ADDCMD("m.", commandSetMark, REGEX_PATTERN | SHOULD_NOT_RESET),
        ADDCMD(">", commandIndentLines, IS_CHANGE),
        ADDCMD("<", commandUnindentLines, IS_CHANGE),
        ADDCMD("<c-c>", commandAbort, 0),
        ADDCMD("<c-[>", commandAbort, 0),
        ADDCMD("ga", commandPrintCharacterCode, SHOULD_NOT_RESET),
        ADDCMD("v", commandEnterVisualMode, SHOULD_NOT_RESET),
        ADDCMD("V", commandEnterVisualLineMode, SHOULD_NOT_RESET),
        ADDCMD("o", commandToOtherEnd, SHOULD_NOT_RESET | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDCMD("=", commandAlignLines, SHOULD_NOT_RESET),
        ADDCMD("~", commandChangeCase, IS_CHANGE),
        ADDCMD("I", commandPrependToBlock, IS_CHANGE),
        ADDCMD("A", commandAppendToBlock, IS_CHANGE),
        ADDCMD("gq", commandFormatLines, IS_CHANGE),
        ADDCMD("q.", commandStartRecordingMacro, REGEX_PATTERN | SHOULD_NOT_RESET),
        ADDCMD("@.", commandReplayMacro, REGEX_PATTERN | SHOULD_NOT_RESET),
        ADDCMD("z.", commandCenterViewOnNonBlank, 0),
        ADDCMD("zz", commandCenterViewOnCursor, 0),
        ADDCMD("z<return>", commandTopViewOnNonBlank, 0),
        ADDCMD("zt", commandTopViewOnCursor, 0),
        ADDCMD("z-", commandBottomViewOnNonBlank, 0),
        ADDCMD("zb", commandBottomViewOnCursor, 0),
    };
    return global;
}

const std::vector<Motion> &VisualViMode::motions()
{
    // init once, is expensive
    static std::vector<Motion> global{
        // regular motions
        ADDMOTION("h", motionLeft, 0),
        ADDMOTION("<left>", motionLeft, 0),
        ADDMOTION("<backspace>", motionLeft, 0),
        ADDMOTION("j", motionDown, 0),
        ADDMOTION("<down>", motionDown, 0),
        ADDMOTION("k", motionUp, 0),
        ADDMOTION("<up>", motionUp, 0),
        ADDMOTION("l", motionRight, 0),
        ADDMOTION("<right>", motionRight, 0),
        ADDMOTION(" ", motionRight, 0),
        ADDMOTION("$", motionToEOL, 0),
        ADDMOTION("<end>", motionToEOL, 0),
        ADDMOTION("g_", motionToLastNonBlank, 0),
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
        ADDMOTION("w", motionWordForward, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("W", motionWORDForward, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("<c-right>", motionWordForward, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("<c-left>", motionWordBackward, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("b", motionWordBackward, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("B", motionWORDBackward, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("e", motionToEndOfWord, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("E", motionToEndOfWORD, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("ge", motionToEndOfPrevWord, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("gE", motionToEndOfPrevWORD, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("|", motionToScreenColumn, 0),
        ADDMOTION("%", motionToMatchingItem, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("`.", motionToMark, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("'.", motionToMarkLine, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("[[", motionToPreviousBraceBlockStart, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("]]", motionToNextBraceBlockStart, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("[]", motionToPreviousBraceBlockEnd, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("][", motionToNextBraceBlockEnd, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("*", motionToNextOccurrence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("#", motionToPrevOccurrence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("<c-f>", motionPageDown, 0),
        ADDMOTION("<pagedown>", motionPageDown, 0),
        ADDMOTION("<c-b>", motionPageUp, 0),
        ADDMOTION("<pageup>", motionPageUp, 0),
        ADDMOTION("gj", motionToNextVisualLine, 0),
        ADDMOTION("g<down>", motionToNextVisualLine, 0),
        ADDMOTION("gk", motionToPrevVisualLine, 0),
        ADDMOTION("g<up>", motionToPrevVisualLine, 0),
        ADDMOTION("(", motionToPreviousSentence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION(")", motionToNextSentence, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("{", motionToBeforeParagraph, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("}", motionToAfterParagraph, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("<c-u>", motionHalfPageUp, 0),
        ADDMOTION("<c-d>", motionHalfPageDown, 0),

        // text objects
        ADDMOTION("iw", textObjectInnerWord, 0),
        ADDMOTION("aw", textObjectAWord, 0),
        ADDMOTION("iW", textObjectInnerWORD, 0),
        ADDMOTION("aW", textObjectAWORD, IS_NOT_LINEWISE),
        ADDMOTION("is", textObjectInnerSentence, IS_NOT_LINEWISE | CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("as", textObjectASentence, IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("ip", textObjectInnerParagraph, IS_NOT_LINEWISE | CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("ap", textObjectAParagraph, IS_NOT_LINEWISE | CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i\"", textObjectInnerQuoteDouble, CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a\"", textObjectAQuoteDouble, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i'", textObjectInnerQuoteSingle, CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a'", textObjectAQuoteSingle, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[()b]", textObjectInnerParen, REGEX_PATTERN | CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a[()b]", textObjectAParen, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[{}B]",
                  textObjectInnerCurlyBracket,
                  REGEX_PATTERN | IS_NOT_LINEWISE | CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a[{}B]", textObjectACurlyBracket, REGEX_PATTERN | IS_NOT_LINEWISE | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[><]",
                  textObjectInnerInequalitySign,
                  REGEX_PATTERN | IS_NOT_LINEWISE | CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i[\\[\\]]", textObjectInnerBracket, REGEX_PATTERN | CAN_CHANGE_WHOLE_VISUAL_MODE_SELECTION | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a[\\[\\]]", textObjectABracket, REGEX_PATTERN | CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("i,", textObjectInnerComma, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("a,", textObjectAComma, CAN_LAND_INSIDE_FOLDING_RANGE),

        ADDMOTION("/<enter>", motionToIncrementalSearchMatch, CAN_LAND_INSIDE_FOLDING_RANGE),
        ADDMOTION("?<enter>", motionToIncrementalSearchMatch, CAN_LAND_INSIDE_FOLDING_RANGE),
    };
    return global;
}
