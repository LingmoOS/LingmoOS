/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_REPLACE_VI_MODE_H
#define KATEVI_REPLACE_VI_MODE_H

#include <vimode/modes/modebase.h>

namespace KateVi
{
/**
 * Commands for the vi replace mode
 */
class ReplaceViMode : public ModeBase
{
public:
    explicit ReplaceViMode(InputModeManager *viInputModeManager, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal);

    /// Update the track of overwritten characters with the \p s character.
    inline void overwrittenChar(const QChar &s)
    {
        m_overwritten += s;
    }

    void setCount(int count)
    {
        m_count = count;
    }

protected:
    /**
     * Checks if the key is a valid command in replace mode.
     *
     * @returns true if a command was completed and executed, false otherwise.
     */
    bool handleKeypress(const QKeyEvent *e) override;

private:
    /**
     * Replace the character at the current column with a character from
     * the same column but in a different line.
     *
     * @param offset The offset of the line to be picked. This value is
     * relative to the current line.
     * @returns true if the character could be replaced, false otherwise.
     */
    bool commandInsertFromLine(int offset);

    // Auxiliar methods for moving the cursor in replace mode.

    bool commandMoveOneWordLeft();
    bool commandMoveOneWordRight();

    // Auxiliar methods for removing modifications.

    void backspace();
    void commandBackWord();
    void commandBackLine();
    void leaveReplaceMode();

protected:
    unsigned int m_count;

private:
    /// Keeps track of the characters that have been overwritten so far.
    QString m_overwritten;
};

}

#endif /* KATEVI_REPLACE_VI_MODE_H */
