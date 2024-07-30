/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_VI_INPUT_MODE_H
#define KATE_VI_INPUT_MODE_H

#include <memory>

#include "kateabstractinputmode.h"

#include <vimode/inputmodemanager.h>

namespace KateVi
{
class GlobalState;
class EmulatedCommandBar;
}
class KateViInputModeFactory;

class KTEXTEDITOR_EXPORT KateViInputMode : public KateAbstractInputMode
{
    explicit KateViInputMode(KateViewInternal *viewInternal, KateVi::GlobalState *global);
    friend KateViInputModeFactory;

public:
    KTextEditor::View::ViewMode viewMode() const override;
    QString viewModeHuman() const override;
    KTextEditor::View::InputMode viewInputMode() const override;
    QString viewInputModeHuman() const override;

    void activate() override;
    void deactivate() override;
    void reset() override;

    bool overwrite() const override;
    void overwrittenChar(const QChar &) override;

    void clearSelection() override;
    bool stealKey(QKeyEvent *) override;

    void gotFocus() override;
    void lostFocus() override;

    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;
    void updateRendererConfig() override;
    void updateConfig() override;
    void readWriteChanged(bool rw) override;

    void find() override;
    void findSelectedForwards() override;
    void findSelectedBackwards() override;
    void findReplace() override;
    void findNext() override;
    void findPrevious() override;

    void activateCommandLine() override;

    bool keyPress(QKeyEvent *) override;
    bool blinkCaret() const override;
    KTextEditor::caretStyles caretStyle() const override;

    void toggleInsert() override;
    void launchInteractiveCommand(const QString &command) override;

    QString bookmarkLabel(int line) const override;

public:
    void showViModeEmulatedCommandBar();
    KateVi::EmulatedCommandBar *viModeEmulatedCommandBar();
    inline KateVi::GlobalState *globalState() const
    {
        return m_viGlobal;
    }
    inline KateVi::InputModeManager *viInputModeManager() const
    {
        return m_viModeManager.get();
    }
    inline bool isActive() const
    {
        return m_activated;
    }
    void setCaretStyle(const KTextEditor::caretStyles caret);

private:
    KateVi::EmulatedCommandBar *m_viModeEmulatedCommandBar;
    KateVi::GlobalState *m_viGlobal;
    KTextEditor::caretStyles m_caret;

    bool m_nextKeypressIsOverriddenShortCut;

    // configs
    bool m_relLineNumbers;
    bool m_activated;

    std::unique_ptr<KateVi::InputModeManager> m_viModeManager;
};

#endif
