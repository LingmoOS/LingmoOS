/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2009 Paul Gideon Dann <pdgiddie@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_VISUAL_VI_MODE_H
#define KATEVI_VISUAL_VI_MODE_H

#include <ktexteditor/range.h>
#include <vimode/modes/normalvimode.h>

namespace KateVi
{
class InputModeManager;

class VisualViMode : public NormalViMode
{
public:
    explicit VisualViMode(InputModeManager *viInputModeManager, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal);

    void init();

    bool isVisualLine() const
    {
        return m_mode == VisualLineMode;
    }

    bool isVisualBlock() const
    {
        return m_mode == VisualBlockMode;
    }

    void switchStartEnd();
    void reset() override;
    void setVisualModeType(const ViMode mode);
    void saveRangeMarks();

    void setStart(const KTextEditor::Cursor c)
    {
        m_start = c;
    }

    KTextEditor::Cursor getStart()
    {
        return m_start;
    }

    void goToPos(const KTextEditor::Cursor c);

    ViMode getLastVisualMode() const
    {
        return m_lastVisualMode;
    }

    const KTextEditor::Cursor getStart() const
    {
        return m_start;
    }

    // Selects all lines in range;
    void selectLines(KTextEditor::Range range);

    // Selects range between c1 and c2, but includes the end cursor position.
    void selectInclusive(const KTextEditor::Cursor c1, const KTextEditor::Cursor c2);

    // Select block between c1 and c2.
    void selectBlockInclusive(const KTextEditor::Cursor c1, const KTextEditor::Cursor c2);

protected:
    /**
     * Called when a motion/text object is used. Updates the cursor position
     * and modifies the range. A motion will only modify the end of the range
     * (i.e. move the cursor) while a text object may modify both the start and
     * the end. Overridden from the ModeBase class.
     */
    void goToPos(const Range &r) override;

    /**
     * Return commands available for this mode.
     * Overwritten in sub classes to replace them, must be a stable reference!
     */
    virtual const std::vector<Command> &commands() override;

    /**
     * Return motions available for this mode.
     * Overwritten in sub classes to replace them, must be a stable reference!
     */
    virtual const std::vector<Motion> &motions() override;

public:
    /**
     * Updates the visual mode's range to reflect a new cursor position. This
     * needs to be called if modifying the range from outside the vi mode, e.g.
     * via mouse selection.
     */
    void updateSelection();

private:
    KTextEditor::Cursor m_start;
    ViMode m_mode, m_lastVisualMode;
};
}

#endif /* KATEVI_VISUAL_VI_MODE_H */
