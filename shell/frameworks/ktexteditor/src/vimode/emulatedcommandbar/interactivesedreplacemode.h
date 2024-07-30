/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_EMULATED_COMMAND_BAR_INTERACTIVESEDREPLACEMODE_H
#define KATEVI_EMULATED_COMMAND_BAR_INTERACTIVESEDREPLACEMODE_H

#include "activemode.h"

#include "../cmds.h"

class QKeyEvent;
class QLabel;

namespace KateVi
{
class EmulatedCommandBar;
class MatchHighlighter;

class InteractiveSedReplaceMode : public ActiveMode
{
public:
    InteractiveSedReplaceMode(EmulatedCommandBar *emulatedCommandBar,
                              MatchHighlighter *matchHighlighter,
                              InputModeManager *viInputModeManager,
                              KTextEditor::ViewPrivate *view);
    ~InteractiveSedReplaceMode() override
    {
    }
    void activate(std::shared_ptr<SedReplace::InteractiveSedReplacer> interactiveSedReplace);
    bool isActive() const
    {
        return m_isActive;
    }
    bool handleKeyPress(const QKeyEvent *keyEvent) override;
    void deactivate(bool wasAborted) override;
    QWidget *label();

private:
    void updateInteractiveSedReplaceLabelText();
    void finishInteractiveSedReplace();
    std::shared_ptr<SedReplace::InteractiveSedReplacer> m_interactiveSedReplacer;
    bool m_isActive;
    QLabel *m_interactiveSedReplaceLabel;
};
}

#endif
