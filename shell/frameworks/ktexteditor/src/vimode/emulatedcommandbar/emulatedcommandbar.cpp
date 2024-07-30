/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <vimode/emulatedcommandbar/emulatedcommandbar.h>

#include "../commandrangeexpressionparser.h"
#include "../globalstate.h"
#include "commandmode.h"
#include "interactivesedreplacemode.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "kateview.h"
#include "matchhighlighter.h"
#include "searchmode.h"
#include <inputmode/kateviinputmode.h>
#include <vimode/inputmodemanager.h>
#include <vimode/keyparser.h>
#include <vimode/modes/normalvimode.h>

#include "../registers.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

using namespace KateVi;

namespace
{
/**
 * @return \a originalRegex but escaped in such a way that a Qt regex search for
 * the resulting string will match the string \a originalRegex.
 */
QString escapedForSearchingAsLiteral(const QString &originalQtRegex)
{
    QString escapedForSearchingAsLiteral = originalQtRegex;
    escapedForSearchingAsLiteral.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    escapedForSearchingAsLiteral.replace(QLatin1Char('$'), QLatin1String("\\$"));
    escapedForSearchingAsLiteral.replace(QLatin1Char('^'), QLatin1String("\\^"));
    escapedForSearchingAsLiteral.replace(QLatin1Char('.'), QLatin1String("\\."));
    escapedForSearchingAsLiteral.replace(QLatin1Char('*'), QLatin1String("\\*"));
    escapedForSearchingAsLiteral.replace(QLatin1Char('/'), QLatin1String("\\/"));
    escapedForSearchingAsLiteral.replace(QLatin1Char('['), QLatin1String("\\["));
    escapedForSearchingAsLiteral.replace(QLatin1Char(']'), QLatin1String("\\]"));
    escapedForSearchingAsLiteral.replace(QLatin1Char('\n'), QLatin1String("\\n"));
    return escapedForSearchingAsLiteral;
}
}

EmulatedCommandBar::EmulatedCommandBar(KateViInputMode *viInputMode, InputModeManager *viInputModeManager, QWidget *parent)
    : KateViewBarWidget(false, parent)
    , m_viInputMode(viInputMode)
    , m_viInputModeManager(viInputModeManager)
    , m_view(viInputModeManager->view())
{
    QHBoxLayout *layout = new QHBoxLayout(centralWidget());
    layout->setContentsMargins(0, 0, 0, 0);

    createAndAddBarTypeIndicator(layout);
    createAndAddEditWidget(layout);
    createAndAddExitStatusMessageDisplay(layout);
    createAndInitExitStatusMessageDisplayTimer();
    createAndAddWaitingForRegisterIndicator(layout);

    m_matchHighligher.reset(new MatchHighlighter(m_view));

    m_completer.reset(new Completer(this, m_view, m_edit));

    m_interactiveSedReplaceMode.reset(new InteractiveSedReplaceMode(this, m_matchHighligher.get(), m_viInputModeManager, m_view));
    layout->addWidget(m_interactiveSedReplaceMode->label());
    m_searchMode.reset(new SearchMode(this, m_matchHighligher.get(), m_viInputModeManager, m_view, m_edit));
    m_commandMode.reset(
        new CommandMode(this, m_matchHighligher.get(), m_viInputModeManager, m_view, m_edit, m_interactiveSedReplaceMode.get(), m_completer.get()));

    m_edit->installEventFilter(this);
    connect(m_edit, &QLineEdit::textChanged, this, &EmulatedCommandBar::editTextChanged);
}

EmulatedCommandBar::~EmulatedCommandBar() = default;

void EmulatedCommandBar::init(EmulatedCommandBar::Mode mode, const QString &initialText)
{
    m_mode = mode;
    m_isActive = true;
    m_wasAborted = true;

    showBarTypeIndicator(mode);

    if (mode == KateVi::EmulatedCommandBar::SearchBackward || mode == SearchForward) {
        switchToMode(m_searchMode.get());
        m_searchMode->init(mode == SearchBackward ? SearchMode::SearchDirection::Backward : SearchMode::SearchDirection::Forward);
    } else {
        switchToMode(m_commandMode.get());
    }

    m_edit->setFocus();
    m_edit->setText(initialText);
    m_edit->show();

    m_exitStatusMessageDisplay->hide();
    m_exitStatusMessageDisplayHideTimer->stop();

    // A change in focus will have occurred: make sure we process it now, instead of having it
    // occur later and stop() m_commandResponseMessageDisplayHide.
    // This is generally only a problem when feeding a sequence of keys without human intervention,
    // as when we execute a mapping, macro, or test case.
    QApplication::processEvents();
}

bool EmulatedCommandBar::isActive() const
{
    return m_isActive;
}

void EmulatedCommandBar::setCommandResponseMessageTimeout(long int commandResponseMessageTimeOutMS)
{
    m_exitStatusMessageHideTimeOutMS = commandResponseMessageTimeOutMS;
}

void EmulatedCommandBar::closed()
{
    m_matchHighligher->updateMatchHighlight(KTextEditor::Range::invalid());
    m_completer->deactivateCompletion();
    m_isActive = false;

    if (m_currentMode) {
        m_currentMode->deactivate(m_wasAborted);
        m_currentMode = nullptr;
    }
}

void EmulatedCommandBar::switchToMode(ActiveMode *newMode)
{
    if (newMode == m_currentMode) {
        return;
    }
    if (m_currentMode) {
        m_currentMode->deactivate(false);
    }
    m_currentMode = newMode;
    m_completer->setCurrentMode(newMode);
}

bool EmulatedCommandBar::barHandledKeypress(const QKeyEvent *keyEvent)
{
    if ((keyEvent->modifiers() == CONTROL_MODIFIER && keyEvent->key() == Qt::Key_H) || keyEvent->key() == Qt::Key_Backspace) {
        if (m_edit->text().isEmpty()) {
            Q_EMIT hideMe();
        }
        m_edit->backspace();
        return true;
    }
    if (keyEvent->modifiers() != CONTROL_MODIFIER) {
        return false;
    }
    if (keyEvent->key() == Qt::Key_B) {
        m_edit->setCursorPosition(0);
        return true;
    } else if (keyEvent->key() == Qt::Key_E) {
        m_edit->setCursorPosition(m_edit->text().length());
        return true;
    } else if (keyEvent->key() == Qt::Key_W) {
        deleteSpacesToLeftOfCursor();
        if (!deleteNonWordCharsToLeftOfCursor()) {
            deleteWordCharsToLeftOfCursor();
        }
        return true;
    } else if (keyEvent->key() == Qt::Key_R || keyEvent->key() == Qt::Key_G) {
        m_waitingForRegister = true;
        m_waitingForRegisterIndicator->setVisible(true);
        if (keyEvent->key() == Qt::Key_G) {
            m_insertedTextShouldBeEscapedForSearchingAsLiteral = true;
        }
        return true;
    }
    return false;
}

void EmulatedCommandBar::insertRegisterContents(const QKeyEvent *keyEvent)
{
    if (keyEvent->key() != Qt::Key_Shift && keyEvent->key() != Qt::Key_Control) {
        const QChar key = KeyParser::self()->KeyEventToQChar(*keyEvent).toLower();

        const int oldCursorPosition = m_edit->cursorPosition();
        QString textToInsert;
        if (keyEvent->modifiers() == CONTROL_MODIFIER && keyEvent->key() == Qt::Key_W) {
            textToInsert = m_view->doc()->wordAt(m_view->cursorPosition());
        } else {
            textToInsert = m_viInputModeManager->globalState()->registers()->getContent(key);
        }
        if (m_insertedTextShouldBeEscapedForSearchingAsLiteral) {
            textToInsert = escapedForSearchingAsLiteral(textToInsert);
            m_insertedTextShouldBeEscapedForSearchingAsLiteral = false;
        }
        m_edit->setText(m_edit->text().insert(m_edit->cursorPosition(), textToInsert));
        m_edit->setCursorPosition(oldCursorPosition + textToInsert.length());
        m_waitingForRegister = false;
        m_waitingForRegisterIndicator->setVisible(false);
    }
}

bool EmulatedCommandBar::eventFilter(QObject *object, QEvent *event)
{
    // The "object" will be either m_edit or m_completer's popup.
    if (m_suspendEditEventFiltering) {
        return false;
    }
    Q_UNUSED(object);
    if (event->type() == QEvent::KeyPress) {
        // Re-route this keypress through Vim's central keypress handling area, so that we can use the keypress in e.g.
        // mappings and macros.
        return m_viInputMode->keyPress(static_cast<QKeyEvent *>(event));
    }
    return false;
}

void EmulatedCommandBar::deleteSpacesToLeftOfCursor()
{
    while (m_edit->cursorPosition() != 0 && m_edit->text().at(m_edit->cursorPosition() - 1) == QLatin1Char(' ')) {
        m_edit->backspace();
    }
}

void EmulatedCommandBar::deleteWordCharsToLeftOfCursor()
{
    while (m_edit->cursorPosition() != 0) {
        const QChar charToTheLeftOfCursor = m_edit->text().at(m_edit->cursorPosition() - 1);
        if (!charToTheLeftOfCursor.isLetterOrNumber() && charToTheLeftOfCursor != QLatin1Char('_')) {
            break;
        }

        m_edit->backspace();
    }
}

bool EmulatedCommandBar::deleteNonWordCharsToLeftOfCursor()
{
    bool deletionsMade = false;
    while (m_edit->cursorPosition() != 0) {
        const QChar charToTheLeftOfCursor = m_edit->text().at(m_edit->cursorPosition() - 1);
        if (charToTheLeftOfCursor.isLetterOrNumber() || charToTheLeftOfCursor == QLatin1Char('_') || charToTheLeftOfCursor == QLatin1Char(' ')) {
            break;
        }

        m_edit->backspace();
        deletionsMade = true;
    }
    return deletionsMade;
}

bool EmulatedCommandBar::handleKeyPress(const QKeyEvent *keyEvent)
{
    if (m_waitingForRegister) {
        insertRegisterContents(keyEvent);
        return true;
    }
    const bool completerHandled = m_completer->completerHandledKeypress(keyEvent);
    if (completerHandled) {
        return true;
    }

    if (keyEvent->modifiers() == CONTROL_MODIFIER && (keyEvent->key() == Qt::Key_C || keyEvent->key() == Qt::Key_BracketLeft)) {
        Q_EMIT hideMe();
        return true;
    }

    // Is this a built-in Emulated Command Bar keypress e.g. insert from register, ctrl-h, etc?
    const bool barHandled = barHandledKeypress(keyEvent);
    if (barHandled) {
        return true;
    }

    // Can the current mode handle it?
    const bool currentModeHandled = m_currentMode->handleKeyPress(keyEvent);
    if (currentModeHandled) {
        return true;
    }

    // Couldn't handle this key event.
    // Send the keypress back to the QLineEdit.  Ideally, instead of doing this, we would simply return "false"
    // and let Qt re-dispatch the event itself; however, there is a corner case in that if the selection
    // changes (as a result of e.g. incremental searches during Visual Mode), and the keypress that causes it
    // is not dispatched from within KateViInputModeHandler::handleKeypress(...)
    // (so KateViInputModeManager::isHandlingKeypress() returns false), we lose information about whether we are
    // in Visual Mode, Visual Line Mode, etc.  See VisualViMode::updateSelection( ).
    if (m_edit->isVisible()) {
        if (m_suspendEditEventFiltering) {
            return false;
        }
        m_suspendEditEventFiltering = true;
        QKeyEvent keyEventCopy(keyEvent->type(), keyEvent->key(), keyEvent->modifiers(), keyEvent->text(), keyEvent->isAutoRepeat(), keyEvent->count());
        qApp->notify(m_edit, &keyEventCopy);
        m_suspendEditEventFiltering = false;
    }
    return true;
}

bool EmulatedCommandBar::isSendingSyntheticSearchCompletedKeypress()
{
    return m_searchMode->isSendingSyntheticSearchCompletedKeypress();
}

void EmulatedCommandBar::startInteractiveSearchAndReplace(std::shared_ptr<SedReplace::InteractiveSedReplacer> interactiveSedReplace)
{
    Q_ASSERT_X(interactiveSedReplace->currentMatch().isValid(),
               "startInteractiveSearchAndReplace",
               "KateCommands shouldn't initiate an interactive sed replace with no initial match");
    switchToMode(m_interactiveSedReplaceMode.get());
    m_interactiveSedReplaceMode->activate(interactiveSedReplace);
}

void EmulatedCommandBar::showBarTypeIndicator(EmulatedCommandBar::Mode mode)
{
    QChar barTypeIndicator = QChar::Null;
    switch (mode) {
    case SearchForward:
        barTypeIndicator = QLatin1Char('/');
        break;
    case SearchBackward:
        barTypeIndicator = QLatin1Char('?');
        break;
    case Command:
        barTypeIndicator = QLatin1Char(':');
        break;
    default:
        Q_ASSERT(false && "Unknown mode!");
    }
    m_barTypeIndicator->setText(barTypeIndicator);
    m_barTypeIndicator->show();
}

QString EmulatedCommandBar::executeCommand(const QString &commandToExecute)
{
    return m_commandMode->executeCommand(commandToExecute);
}

void EmulatedCommandBar::closeWithStatusMessage(const QString &exitStatusMessage)
{
    // Display the message for a while.  Become inactive, so we don't steal keys in the meantime.
    m_isActive = false;

    m_exitStatusMessageDisplay->show();
    m_exitStatusMessageDisplay->setText(exitStatusMessage);
    hideAllWidgetsExcept(m_exitStatusMessageDisplay);

    m_exitStatusMessageDisplayHideTimer->start(m_exitStatusMessageHideTimeOutMS);
}

void EmulatedCommandBar::editTextChanged(const QString &newText)
{
    Q_ASSERT(!m_interactiveSedReplaceMode->isActive());
    m_currentMode->editTextChanged(newText);
    m_completer->editTextChanged(newText);
}

void EmulatedCommandBar::startHideExitStatusMessageTimer()
{
    if (m_exitStatusMessageDisplay->isVisible() && !m_exitStatusMessageDisplayHideTimer->isActive()) {
        m_exitStatusMessageDisplayHideTimer->start(m_exitStatusMessageHideTimeOutMS);
    }
}

void EmulatedCommandBar::setViInputModeManager(InputModeManager *viInputModeManager)
{
    m_viInputModeManager = viInputModeManager;
    m_searchMode->setViInputModeManager(viInputModeManager);
    m_commandMode->setViInputModeManager(viInputModeManager);
    m_interactiveSedReplaceMode->setViInputModeManager(viInputModeManager);
}

void EmulatedCommandBar::hideAllWidgetsExcept(QWidget *widgetToKeepVisible)
{
    const QList<QWidget *> widgets = centralWidget()->findChildren<QWidget *>();
    for (QWidget *widget : widgets) {
        if (widget != widgetToKeepVisible) {
            widget->hide();
        }
    }
}

void EmulatedCommandBar::createAndAddBarTypeIndicator(QLayout *layout)
{
    m_barTypeIndicator = new QLabel(this);
    m_barTypeIndicator->setObjectName(QStringLiteral("bartypeindicator"));
    layout->addWidget(m_barTypeIndicator);
}

void EmulatedCommandBar::createAndAddEditWidget(QLayout *layout)
{
    m_edit = new QLineEdit(this);
    m_edit->setObjectName(QStringLiteral("commandtext"));
    layout->addWidget(m_edit);
}

void EmulatedCommandBar::createAndAddExitStatusMessageDisplay(QLayout *layout)
{
    m_exitStatusMessageDisplay = new QLabel(this);
    m_exitStatusMessageDisplay->setObjectName(QStringLiteral("commandresponsemessage"));
    m_exitStatusMessageDisplay->setAlignment(Qt::AlignLeft);
    layout->addWidget(m_exitStatusMessageDisplay);
}

void EmulatedCommandBar::createAndInitExitStatusMessageDisplayTimer()
{
    m_exitStatusMessageDisplayHideTimer = new QTimer(this);
    m_exitStatusMessageDisplayHideTimer->setSingleShot(true);
    connect(m_exitStatusMessageDisplayHideTimer, &QTimer::timeout, this, &EmulatedCommandBar::hideMe);
    // Make sure the timer is stopped when the user switches views. If not, focus will be given to the
    // wrong view when KateViewBar::hideCurrentBarWidget() is called as a result of m_commandResponseMessageDisplayHide
    // timing out.
    connect(m_view, &KTextEditor::ViewPrivate::focusOut, m_exitStatusMessageDisplayHideTimer, &QTimer::stop);
    // We can restart the timer once the view has focus again, though.
    connect(m_view, &KTextEditor::ViewPrivate::focusIn, this, &EmulatedCommandBar::startHideExitStatusMessageTimer);
}

void EmulatedCommandBar::createAndAddWaitingForRegisterIndicator(QLayout *layout)
{
    m_waitingForRegisterIndicator = new QLabel(this);
    m_waitingForRegisterIndicator->setObjectName(QStringLiteral("waitingforregisterindicator"));
    m_waitingForRegisterIndicator->setVisible(false);
    m_waitingForRegisterIndicator->setText(QStringLiteral("\""));
    layout->addWidget(m_waitingForRegisterIndicator);
}
