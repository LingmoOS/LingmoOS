/*
    SPDX-FileCopyrightText: 2008-2011 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_INSERT_VI_MODE_H
#define KATEVI_INSERT_VI_MODE_H

#include <vimode/modes/modebase.h>

namespace KTextEditor
{
class ViewPrivate;
}
class KateViewInternal;

class QKeyEvent;

namespace KateVi
{
class Motion;

/**
 * Commands for the vi insert mode
 */
enum BlockInsert { None, Prepend, Append, AppendEOL };

class InsertViMode : public ModeBase
{
public:
    explicit InsertViMode(InputModeManager *viInputModeManager, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal);
    ~InsertViMode() override;

    bool handleKeypress(const QKeyEvent *e) override;

    bool commandInsertFromAbove();
    bool commandInsertFromBelow();

    bool commandDeleteWord();
    bool commandDeleteLine();
    bool commandNewLine();
    bool commandDeleteCharBackward();

    bool commandIndent();
    bool commandUnindent();

    bool commandToFirstCharacterInFile();
    bool commandToLastCharacterInFile();

    bool commandMoveOneWordLeft();
    bool commandMoveOneWordRight();

    bool commandCompleteNext();
    bool commandCompletePrevious();

    bool commandInsertContentOfRegister();
    bool commandSwitchToNormalModeForJustOneCommand();

    void setBlockPrependMode(Range blockRange);
    void setBlockAppendMode(Range blockRange, BlockInsert b);

    void setCount(int count)
    {
        m_count = count;
    }
    void setCountedRepeatsBeginOnNewLine(bool countedRepeatsBeginOnNewLine)
    {
        m_countedRepeatsBeginOnNewLine = countedRepeatsBeginOnNewLine;
    }

protected:
    void leaveInsertMode(bool force = false);
    void completionFinished();

protected:
    BlockInsert m_blockInsert;
    unsigned int m_eolPos; // length of first line in eol mode before text is appended
    Range m_blockRange;

    QString m_keys;
    bool m_waitingRegister;

    unsigned int m_count;
    bool m_countedRepeatsBeginOnNewLine;

    bool m_isExecutingCompletion;
    QString m_textInsertedByCompletion;
    KTextEditor::Cursor m_textInsertedByCompletionEndPos;

private:
    KTEXTEDITOR_NO_EXPORT
    void textInserted(KTextEditor::Document *document, KTextEditor::Range range);
};
}

#endif /* KATEVI_INSERT_VI_MODE_H */
