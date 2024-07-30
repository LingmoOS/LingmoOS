/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_EMULATED_COMMAND_BAR_H
#define KATEVI_EMULATED_COMMAND_BAR_H

#include "kateviewhelpers.h"
#include <vimode/cmds.h>

namespace KTextEditor
{
class ViewPrivate;
class Command;
}

class QLabel;
class QLayout;

namespace KateVi
{
class MatchHighlighter;
class InteractiveSedReplaceMode;
class SearchMode;
class CommandMode;
class ActiveMode;
class Completer;

/**
 * A KateViewBarWidget that attempts to emulate some of the features of Vim's own command bar,
 * including insertion of register contents via ctr-r<registername>; dismissal via
 * ctrl-c and ctrl-[; bi-directional incremental searching, with SmartCase; interactive sed-replace;
 * plus a few extensions such as completion from document and navigable sed search and sed replace history.
 */
class EmulatedCommandBar : public KateViewBarWidget
{
public:
    enum Mode { NoMode, SearchForward, SearchBackward, Command };
    explicit EmulatedCommandBar(KateViInputMode *viInputMode, InputModeManager *viInputModeManager, QWidget *parent = nullptr);
    ~EmulatedCommandBar() override;
    void init(Mode mode, const QString &initialText = QString());
    bool isActive() const;
    KTEXTEDITOR_EXPORT void setCommandResponseMessageTimeout(long commandResponseMessageTimeOutMS);
    bool handleKeyPress(const QKeyEvent *keyEvent);
    bool isSendingSyntheticSearchCompletedKeypress();

    void startInteractiveSearchAndReplace(std::shared_ptr<SedReplace::InteractiveSedReplacer> interactiveSedReplace);
    KTEXTEDITOR_EXPORT QString executeCommand(const QString &commandToExecute);

    void setViInputModeManager(InputModeManager *viInputModeManager);

private:
    KateViInputMode *m_viInputMode;
    InputModeManager *m_viInputModeManager;
    bool m_isActive = false;
    bool m_wasAborted = true;
    Mode m_mode = NoMode;
    KTextEditor::ViewPrivate *m_view = nullptr;
    QLineEdit *m_edit = nullptr;

    QLabel *m_barTypeIndicator = nullptr;
    void showBarTypeIndicator(Mode mode);

    bool m_suspendEditEventFiltering = false;

    bool m_waitingForRegister = false;
    QLabel *m_waitingForRegisterIndicator;
    bool m_insertedTextShouldBeEscapedForSearchingAsLiteral = false;

    void hideAllWidgetsExcept(QWidget *widgetToKeepVisible);

    friend class ActiveMode;
    std::unique_ptr<MatchHighlighter> m_matchHighligher;

    std::unique_ptr<Completer> m_completer;

    std::unique_ptr<InteractiveSedReplaceMode> m_interactiveSedReplaceMode;
    std::unique_ptr<SearchMode> m_searchMode;
    std::unique_ptr<CommandMode> m_commandMode;

    void switchToMode(ActiveMode *newMode);
    ActiveMode *m_currentMode = nullptr;

    bool barHandledKeypress(const QKeyEvent *keyEvent);
    void insertRegisterContents(const QKeyEvent *keyEvent);
    bool eventFilter(QObject *object, QEvent *event) override;
    void deleteSpacesToLeftOfCursor();
    void deleteWordCharsToLeftOfCursor();
    bool deleteNonWordCharsToLeftOfCursor();

    void closed() override;
    void closeWithStatusMessage(const QString &exitStatusMessage);
    QTimer *m_exitStatusMessageDisplayHideTimer;
    QLabel *m_exitStatusMessageDisplay;
    long m_exitStatusMessageHideTimeOutMS = 4000;

    void createAndAddBarTypeIndicator(QLayout *layout);
    void createAndAddEditWidget(QLayout *layout);
    void createAndAddExitStatusMessageDisplay(QLayout *layout);
    void createAndInitExitStatusMessageDisplayTimer();
    void createAndAddWaitingForRegisterIndicator(QLayout *layout);

private:
    void editTextChanged(const QString &newText);
    void startHideExitStatusMessageTimer();
};
}

#endif /* KATEVI_EMULATED_COMMAND_BAR_H */
