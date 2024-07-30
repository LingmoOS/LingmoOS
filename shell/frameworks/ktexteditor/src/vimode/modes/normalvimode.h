/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2009 Paul Gideon Dann <pdgiddie@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_NORMAL_VI_MODE_H
#define KATEVI_NORMAL_VI_MODE_H

#include <vimode/command.h>
#include <vimode/modes/modebase.h>
#include <vimode/motion.h>
#include <vimode/range.h>

#include <QHash>
#include <QList>
#include <QRegularExpression>
#include <QStack>

#include <vector>

#include <ktexteditor/range.h>

class QKeyEvent;
class KateViInputMode;

namespace KateVi
{
class KeyParser;
class InputModeManager;

/**
 * Commands for the vi normal mode
 */
class NormalViMode : public ModeBase
{
    friend KateViInputMode;

public:
    explicit NormalViMode(InputModeManager *viInputModeManager, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal);
    ~NormalViMode() override;

    bool handleKeypress(const QKeyEvent *e) override;

    bool commandEnterInsertMode();
    bool commandEnterInsertModeAppend();
    bool commandEnterInsertModeAppendEOL();
    bool commandEnterInsertModeBeforeFirstNonBlankInLine();
    bool commandEnterInsertModeLast();

    bool commandEnterVisualMode();
    bool commandEnterVisualLineMode();
    bool commandEnterVisualBlockMode();
    bool commandReselectVisual();
    bool commandToOtherEnd();

    bool commandEnterReplaceMode();

    bool commandDelete();
    bool commandDeleteToEOL();
    bool commandDeleteLine();

    bool commandMakeLowercase();
    bool commandMakeLowercaseLine();
    bool commandMakeUppercase();
    bool commandMakeUppercaseLine();
    bool commandChangeCase();
    bool commandChangeCaseRange();
    bool commandChangeCaseLine();

    bool commandOpenNewLineUnder();
    bool commandOpenNewLineOver();

    bool commandJoinLines();

    bool commandChange();
    bool commandChangeLine();
    bool commandChangeToEOL();
    bool commandSubstituteChar();
    bool commandSubstituteLine();

    bool commandYank();
    bool commandYankLine();
    bool commandYankToEOL();

    bool commandPaste();
    bool commandPasteBefore();

    bool commandgPaste();
    bool commandgPasteBefore();

    bool commandIndentedPaste();
    bool commandIndentedPasteBefore();

    bool commandDeleteChar();
    bool commandDeleteCharBackward();

    bool commandReplaceCharacter();

    bool commandSwitchToCmdLine();
    bool commandSearchBackward();
    bool commandSearchForward();
    bool commandUndo();
    bool commandRedo();

    bool commandSetMark();

    bool commandIndentLine();
    bool commandUnindentLine();
    bool commandIndentLines();
    bool commandUnindentLines();

    bool commandScrollPageDown();
    bool commandScrollPageUp();
    bool commandScrollHalfPageUp();
    bool commandScrollHalfPageDown();

    bool commandCenterView(bool onFirst);
    bool commandCenterViewOnNonBlank();
    bool commandCenterViewOnCursor();
    bool commandTopView(bool onFirst);
    bool commandTopViewOnNonBlank();
    bool commandTopViewOnCursor();
    bool commandBottomView(bool onFirst);
    bool commandBottomViewOnNonBlank();
    bool commandBottomViewOnCursor();

    bool commandAbort();

    bool commandPrintCharacterCode();

    bool commandRepeatLastChange();

    bool commandAlignLine();
    bool commandAlignLines();

    bool commandAddToNumber();
    bool commandSubtractFromNumber();

    bool commandPrependToBlock();
    bool commandAppendToBlock();

    bool commandGoToNextJump();
    bool commandGoToPrevJump();

    bool commandSwitchToLeftView();
    bool commandSwitchToUpView();
    bool commandSwitchToDownView();
    bool commandSwitchToRightView();
    bool commandSwitchToNextView();

    bool commandSplitHoriz();
    bool commandSplitVert();
    bool commandCloseView();

    bool commandSwitchToNextTab();
    bool commandSwitchToPrevTab();

    bool commandFormatLine();
    bool commandFormatLines();

    bool commandCollapseToplevelNodes();
    bool commandCollapseLocal();
    bool commandExpandAll();
    bool commandExpandLocal();
    bool commandToggleRegionVisibility();

    bool commandStartRecordingMacro();
    bool commandReplayMacro();

    bool commandCloseWrite();
    bool commandCloseNocheck();

    // MOTIONS

    Range motionLeft();
    Range motionRight();
    Range motionDown();
    Range motionUp();

    Range motionPageDown();
    Range motionPageUp();
    Range motionHalfPageDown();
    Range motionHalfPageUp();

    Range motionUpToFirstNonBlank();
    Range motionDownToFirstNonBlank();

    Range motionWordForward();
    Range motionWordBackward();
    Range motionWORDForward();
    Range motionWORDBackward();

    Range motionToEndOfWord();
    Range motionToEndOfWORD();
    Range motionToEndOfPrevWord();
    Range motionToEndOfPrevWORD();

    Range motionFindChar();
    Range motionFindCharBackward();
    Range motionToChar();
    Range motionToCharBackward();
    Range motionRepeatlastTF();
    Range motionRepeatlastTFBackward();

    Range motionToEOL();
    Range motionToLastNonBlank();
    Range motionToColumn0();
    Range motionToFirstCharacterOfLine();

    Range motionToLineFirst();
    Range motionToLineLast();

    Range motionToScreenColumn();

    Range motionToMark();
    Range motionToMarkLine();

    Range motionToMatchingItem();

    Range motionToPreviousBraceBlockStart();
    Range motionToNextBraceBlockStart();
    Range motionToPreviousBraceBlockEnd();
    Range motionToNextBraceBlockEnd();

    Range motionToNextOccurrence();
    Range motionToPrevOccurrence();

    Range motionToFirstLineOfWindow();
    Range motionToMiddleLineOfWindow();
    Range motionToLastLineOfWindow();

    Range motionToNextVisualLine();
    Range motionToPrevVisualLine();

    Range motionToPreviousSentence();
    Range motionToNextSentence();

    Range motionToBeforeParagraph();
    Range motionToAfterParagraph();

    Range motionToIncrementalSearchMatch();

    // TEXT OBJECTS

    Range textObjectAWord();
    Range textObjectInnerWord();
    Range textObjectAWORD();
    Range textObjectInnerWORD();

    Range textObjectInnerSentence();
    Range textObjectASentence();

    Range textObjectInnerParagraph();
    Range textObjectAParagraph();

    Range textObjectAQuoteDouble();
    Range textObjectInnerQuoteDouble();

    Range textObjectAQuoteSingle();
    Range textObjectInnerQuoteSingle();

    Range textObjectABackQuote();
    Range textObjectInnerBackQuote();

    Range textObjectAParen();
    Range textObjectInnerParen();

    Range textObjectABracket();
    Range textObjectInnerBracket();

    Range textObjectACurlyBracket();
    Range textObjectInnerCurlyBracket();

    Range textObjectAInequalitySign();
    Range textObjectInnerInequalitySign();

    Range textObjectAComma();
    Range textObjectInnerComma();

    virtual void reset();

    void beginMonitoringDocumentChanges();

protected:
    void resetParser();
    QRegularExpression generateMatchingItemRegex() const;
    void executeCommand(const Command *cmd);
    OperationMode getOperationMode() const;

    void highlightYank(const Range &range, const OperationMode mode = CharWise);
    void addHighlightYank(KTextEditor::Range range);

    bool motionWillBeUsedWithCommand() const
    {
        return !m_awaitingMotionOrTextObject.isEmpty();
    };

    void joinLines(unsigned int from, unsigned int to) const;
    void reformatLines(unsigned int from, unsigned int to) const;
    bool executeKateCommand(const QString &command);

    /**
     * Get the index of the first non-blank character from the given line.
     *
     * @param line The line to be picked. The current line will picked instead
     * if this parameter is set to a negative value.
     * @returns the index of the first non-blank character from the given line.
     * If a non-space character cannot be found, the 0 is returned.
     */
    int getFirstNonBlank(int line = -1) const;

    Range textObjectComma(bool inner) const;
    void shrinkRangeAroundCursor(Range &toShrink, const Range &rangeToShrinkTo) const;
    KTextEditor::Cursor findSentenceStart();
    KTextEditor::Cursor findSentenceEnd();
    KTextEditor::Cursor findParagraphStart();
    KTextEditor::Cursor findParagraphEnd();

    /**
     * Return commands available for this mode.
     * Overwritten in sub classes to replace them, must be a stable reference!
     */
    virtual const std::vector<Command> &commands();

    /**
     * Return motions available for this mode.
     * Overwritten in sub classes to replace them, must be a stable reference!
     */
    virtual const std::vector<Motion> &motions();

protected:
    // The 'current position' is the current cursor position for non-linewise pastes, and the current
    // line for linewise.
    enum PasteLocation { AtCurrentPosition, AfterCurrentPosition };
    bool paste(NormalViMode::PasteLocation pasteLocation, bool isgPaste, bool isIndentedPaste);
    static KTextEditor::Cursor cursorPosAtEndOfPaste(const KTextEditor::Cursor pasteLocation, const QString &pastedText);

    // set sticky column to a ridiculously high value so that the cursor will stick to EOL,
    // but only if it's a regular motion
    void stickStickyColumnToEOL();

protected:
    QString m_keys;
    QString m_lastTFcommand; // holds the last t/T/f/F command so that it can be repeated with ;/,

    unsigned int m_countTemp;
    int m_motionOperatorIndex;
    int m_scroll_count_limit;

    QList<int> m_matchingCommands;
    QList<int> m_matchingMotions;
    QStack<int> m_awaitingMotionOrTextObject;

    bool m_findWaitingForChar;
    bool m_isRepeatedTFcommand;
    bool m_linewiseCommand;
    bool m_commandWithMotion;
    bool m_lastMotionWasLinewiseInnerBlock;
    bool m_motionCanChangeWholeVisualModeSelection;
    bool m_commandShouldKeepSelection;
    bool m_deleteCommand;
    // Ctrl-c or ESC have been pressed, leading to a call to reset().
    bool m_pendingResetIsDueToExit;
    bool m_isUndo;
    bool waitingForRegisterOrCharToSearch();

    // item matching ('%' motion)
    QHash<QString, QString> m_matchingItems;
    QRegularExpression m_matchItemRegex;

    KeyParser *m_keyParser;

    KTextEditor::Attribute::Ptr m_highlightYankAttribute;
    QSet<KTextEditor::MovingRange *> m_highlightedYanks;
    QSet<KTextEditor::MovingRange *> &highlightedYankForDocument();

    KTextEditor::Cursor m_currentChangeEndMarker;
    KTextEditor::Cursor m_positionWhenIncrementalSearchBegan;

private:
    void textInserted(KTextEditor::Document *document, KTextEditor::Range range);
    void textRemoved(KTextEditor::Document *, KTextEditor::Range);
    void undoBeginning();
    void undoEnded();

    void updateYankHighlightAttrib();
    void clearYankHighlight();
    void aboutToDeleteMovingInterfaceContent();
};
}

#endif /* KATEVI_NORMAL_VI_MODE_H */
