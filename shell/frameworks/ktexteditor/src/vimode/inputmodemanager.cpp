/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2009 Paul Gideon Dann <pdgiddie@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <vimode/inputmodemanager.h>

#include <QApplication>
#include <QString>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include "completionrecorder.h"
#include "completionreplayer.h"
#include "definitions.h"
#include "globalstate.h"
#include "jumps.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katerenderer.h"
#include "kateview.h"
#include "kateviewinternal.h"
#include "kateviinputmode.h"
#include "lastchangerecorder.h"
#include "macrorecorder.h"
#include "macros.h"
#include "marks.h"
#include "registers.h"
#include "searcher.h"
#include "variable.h"
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>
#include <vimode/keymapper.h>
#include <vimode/keyparser.h>
#include <vimode/modes/insertvimode.h>
#include <vimode/modes/normalvimode.h>
#include <vimode/modes/replacevimode.h>
#include <vimode/modes/visualvimode.h>

using namespace KateVi;

InputModeManager::InputModeManager(KateViInputMode *inputAdapter, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal)
    : m_inputAdapter(inputAdapter)
{
    m_currentViMode = ViMode::NormalMode;
    m_previousViMode = ViMode::NormalMode;

    m_viNormalMode = new NormalViMode(this, view, viewInternal);
    m_viInsertMode = new InsertViMode(this, view, viewInternal);
    m_viVisualMode = new VisualViMode(this, view, viewInternal);
    m_viReplaceMode = new ReplaceViMode(this, view, viewInternal);

    m_view = view;
    m_viewInternal = viewInternal;

    m_insideHandlingKeyPressCount = 0;

    m_keyMapperStack.push(std::make_shared<KeyMapper>(this, m_view->doc()));

    m_temporaryNormalMode = false;

    m_jumps = new Jumps();
    m_marks = new Marks(this);

    m_searcher = new Searcher(this);
    m_completionRecorder = new CompletionRecorder(this);
    m_completionReplayer = new CompletionReplayer(this);

    m_macroRecorder = new MacroRecorder(this);

    m_lastChangeRecorder = new LastChangeRecorder(this);

    // We have to do this outside of NormalMode, as we don't want
    // VisualMode (which inherits from NormalMode) to respond
    // to changes in the document as well.
    m_viNormalMode->beginMonitoringDocumentChanges();
}

InputModeManager::~InputModeManager()
{
    delete m_viNormalMode;
    delete m_viInsertMode;
    delete m_viVisualMode;
    delete m_viReplaceMode;
    delete m_jumps;
    delete m_marks;
    delete m_searcher;
    delete m_macroRecorder;
    delete m_completionRecorder;
    delete m_completionReplayer;
    delete m_lastChangeRecorder;
}

bool InputModeManager::handleKeypress(const QKeyEvent *e)
{
    m_insideHandlingKeyPressCount++;
    bool res = false;
    bool keyIsPartOfMapping = false;
    const bool isSyntheticSearchCompletedKeyPress = m_inputAdapter->viModeEmulatedCommandBar()->isSendingSyntheticSearchCompletedKeypress();

    // With macros, we want to record the keypresses *before* they are mapped, but if they end up *not* being part
    // of a mapping, we don't want to record them when they are played back by m_keyMapper, hence
    // the "!isPlayingBackRejectedKeys()". And obviously, since we're recording keys before they are mapped, we don't
    // want to also record the executed mapping, as when we replayed the macro, we'd get duplication!
    if (m_macroRecorder->isRecording() && !m_macroRecorder->isReplaying() && !isSyntheticSearchCompletedKeyPress && !keyMapper()->isExecutingMapping()
        && !keyMapper()->isPlayingBackRejectedKeys() && !lastChangeRecorder()->isReplaying()) {
        m_macroRecorder->record(*e);
    }

    if (!m_lastChangeRecorder->isReplaying() && !isSyntheticSearchCompletedKeyPress) {
        if (e->key() == Qt::Key_AltGr) {
            return true; // do nothing
        }

        // Hand off to the key mapper, and decide if this key is part of a mapping.
        if (e->key() != Qt::Key_Control && e->key() != Qt::Key_Shift && e->key() != Qt::Key_Alt && e->key() != Qt::Key_Meta) {
            const QChar key = KeyParser::self()->KeyEventToQChar(*e);
            if (keyMapper()->handleKeypress(key)) {
                keyIsPartOfMapping = true;
                res = true;
            }
        }
    }

    if (!keyIsPartOfMapping) {
        if (!m_lastChangeRecorder->isReplaying() && !isSyntheticSearchCompletedKeyPress) {
            // record key press so that it can be repeated via "."
            m_lastChangeRecorder->record(*e);
        }

        if (m_inputAdapter->viModeEmulatedCommandBar()->isActive()) {
            res = m_inputAdapter->viModeEmulatedCommandBar()->handleKeyPress(e);
        } else {
            res = getCurrentViModeHandler()->handleKeypress(e);
        }
    }

    m_insideHandlingKeyPressCount--;
    Q_ASSERT(m_insideHandlingKeyPressCount >= 0);

    return res;
}

void InputModeManager::feedKeyPresses(const QString &keyPresses) const
{
    int key;
    Qt::KeyboardModifiers mods;
    QString text;

    for (const QChar c : keyPresses) {
        QString decoded = KeyParser::self()->decodeKeySequence(QString(c));
        key = -1;
        mods = Qt::NoModifier;
        text.clear();

        if (decoded.length() > 1) { // special key

            // remove the angle brackets
            decoded.remove(0, 1);
            decoded.remove(decoded.indexOf(QLatin1Char('>')), 1);

            // check if one or more modifier keys where used
            if (decoded.indexOf(QLatin1String("s-")) != -1 || decoded.indexOf(QLatin1String("c-")) != -1 || decoded.indexOf(QLatin1String("m-")) != -1
                || decoded.indexOf(QLatin1String("a-")) != -1) {
                int s = decoded.indexOf(QLatin1String("s-"));
                if (s != -1) {
                    mods |= Qt::ShiftModifier;
                    decoded.remove(s, 2);
                }

                int c = decoded.indexOf(QLatin1String("c-"));
                if (c != -1) {
                    mods |= CONTROL_MODIFIER;
                    decoded.remove(c, 2);
                }

                int a = decoded.indexOf(QLatin1String("a-"));
                if (a != -1) {
                    mods |= Qt::AltModifier;
                    decoded.remove(a, 2);
                }

                int m = decoded.indexOf(QLatin1String("m-"));
                if (m != -1) {
                    mods |= META_MODIFIER;
                    decoded.remove(m, 2);
                }

                if (decoded.length() > 1) {
                    key = KeyParser::self()->vi2qt(decoded);
                } else if (decoded.length() == 1) {
                    key = int(decoded.at(0).toUpper().toLatin1());
                    text = decoded.at(0);
                }
            } else { // no modifiers
                key = KeyParser::self()->vi2qt(decoded);
            }
        } else {
            key = decoded.at(0).unicode();
            text = decoded.at(0);
        }

        if (key == -1) {
            continue;
        }

        // We have to be clever about which widget we dispatch to, as we can trigger
        // shortcuts if we're not careful (even if Vim mode is configured to steal shortcuts).
        QKeyEvent k(QEvent::KeyPress, key, mods, text);
        QWidget *destWidget = nullptr;
        if (QApplication::activePopupWidget()) {
            // According to the docs, the activePopupWidget, if present, takes all events.
            destWidget = QApplication::activePopupWidget();
        } else if (QApplication::focusWidget()) {
            if (QApplication::focusWidget()->focusProxy()) {
                destWidget = QApplication::focusWidget()->focusProxy();
            } else {
                destWidget = QApplication::focusWidget();
            }
        } else {
            destWidget = m_view->focusProxy();
        }
        QApplication::sendEvent(destWidget, &k);
    }
}

bool InputModeManager::isHandlingKeypress() const
{
    return m_insideHandlingKeyPressCount > 0;
}

void InputModeManager::storeLastChangeCommand()
{
    m_lastChange = m_lastChangeRecorder->encodedChanges();
    m_lastChangeCompletionsLog = m_completionRecorder->currentChangeCompletionsLog();
}

void InputModeManager::repeatLastChange()
{
    m_lastChangeRecorder->replay(m_lastChange, m_lastChangeCompletionsLog);
}

void InputModeManager::clearCurrentChangeLog()
{
    m_lastChangeRecorder->clear();
    m_completionRecorder->clearCurrentChangeCompletionsLog();
}

void InputModeManager::doNotLogCurrentKeypress()
{
    m_macroRecorder->dropLast();
    m_lastChangeRecorder->dropLast();
}

void InputModeManager::changeViMode(ViMode newMode)
{
    m_previousViMode = m_currentViMode;
    m_currentViMode = newMode;
}

ViMode InputModeManager::getCurrentViMode() const
{
    return m_currentViMode;
}

KTextEditor::View::ViewMode InputModeManager::getCurrentViewMode() const
{
    switch (m_currentViMode) {
    case ViMode::InsertMode:
        return KTextEditor::View::ViModeInsert;
    case ViMode::VisualMode:
        return KTextEditor::View::ViModeVisual;
    case ViMode::VisualLineMode:
        return KTextEditor::View::ViModeVisualLine;
    case ViMode::VisualBlockMode:
        return KTextEditor::View::ViModeVisualBlock;
    case ViMode::ReplaceMode:
        return KTextEditor::View::ViModeReplace;
    case ViMode::NormalMode:
    default:
        return KTextEditor::View::ViModeNormal;
    }
}

ViMode InputModeManager::getPreviousViMode() const
{
    return m_previousViMode;
}

bool InputModeManager::isAnyVisualMode() const
{
    return ((m_currentViMode == ViMode::VisualMode) || (m_currentViMode == ViMode::VisualLineMode) || (m_currentViMode == ViMode::VisualBlockMode));
}

::ModeBase *InputModeManager::getCurrentViModeHandler() const
{
    switch (m_currentViMode) {
    case ViMode::NormalMode:
        return m_viNormalMode;
    case ViMode::InsertMode:
        return m_viInsertMode;
    case ViMode::VisualMode:
    case ViMode::VisualLineMode:
    case ViMode::VisualBlockMode:
        return m_viVisualMode;
    case ViMode::ReplaceMode:
        return m_viReplaceMode;
    }
    return nullptr;
}

void InputModeManager::viEnterNormalMode()
{
    bool moveCursorLeft = (m_currentViMode == ViMode::InsertMode || m_currentViMode == ViMode::ReplaceMode) && m_viewInternal->cursorPosition().column() > 0;

    if (!m_lastChangeRecorder->isReplaying() && (m_currentViMode == ViMode::InsertMode || m_currentViMode == ViMode::ReplaceMode)) {
        // '^ is the insert mark and "^ is the insert register,
        // which holds the last inserted text
        KTextEditor::Range r(m_view->cursorPosition(), m_marks->getInsertStopped());

        if (r.isValid()) {
            QString insertedText = m_view->doc()->text(r);
            m_inputAdapter->globalState()->registers()->setInsertStopped(insertedText);
        }

        m_marks->setInsertStopped(KTextEditor::Cursor(m_view->cursorPosition()));
    }

    changeViMode(ViMode::NormalMode);

    if (moveCursorLeft) {
        m_viewInternal->cursorPrevChar();
    }
    m_inputAdapter->setCaretStyle(KTextEditor::caretStyles::Block);
    m_viewInternal->update();
}

void InputModeManager::viEnterInsertMode()
{
    changeViMode(ViMode::InsertMode);
    m_marks->setInsertStopped(KTextEditor::Cursor(m_view->cursorPosition()));
    if (getTemporaryNormalMode()) {
        // Ensure the key log contains a request to re-enter Insert mode, else the keystrokes made
        // after returning from temporary normal mode will be treated as commands!
        m_lastChangeRecorder->record(QKeyEvent(QEvent::KeyPress, Qt::Key_I, Qt::NoModifier, QStringLiteral("i")));
    }
    m_inputAdapter->setCaretStyle(KTextEditor::caretStyles::Line);
    setTemporaryNormalMode(false);
    m_viewInternal->update();
}

void InputModeManager::viEnterVisualMode(ViMode mode)
{
    changeViMode(mode);

    // If the selection is inclusive, the caret should be a block.
    // If the selection is exclusive, the caret should be a line.
    m_inputAdapter->setCaretStyle(KTextEditor::caretStyles::Block);
    m_viewInternal->update();
    getViVisualMode()->setVisualModeType(mode);
    getViVisualMode()->init();
}

void InputModeManager::viEnterReplaceMode()
{
    changeViMode(ViMode::ReplaceMode);
    m_marks->setStartEditYanked(KTextEditor::Cursor(m_view->cursorPosition()));
    m_inputAdapter->setCaretStyle(KTextEditor::caretStyles::Underline);
    m_viewInternal->update();
}

NormalViMode *InputModeManager::getViNormalMode()
{
    return m_viNormalMode;
}

InsertViMode *InputModeManager::getViInsertMode()
{
    return m_viInsertMode;
}

VisualViMode *InputModeManager::getViVisualMode()
{
    return m_viVisualMode;
}

ReplaceViMode *InputModeManager::getViReplaceMode()
{
    return m_viReplaceMode;
}

const QString InputModeManager::getVerbatimKeys() const
{
    QString cmd;

    switch (getCurrentViMode()) {
    case ViMode::NormalMode:
        cmd = m_viNormalMode->getVerbatimKeys();
        break;
    case ViMode::InsertMode:
    case ViMode::ReplaceMode:
        // ...
        break;
    case ViMode::VisualMode:
    case ViMode::VisualLineMode:
    case ViMode::VisualBlockMode:
        cmd = m_viVisualMode->getVerbatimKeys();
        break;
    }

    return cmd;
}

void InputModeManager::readSessionConfig(const KConfigGroup &config)
{
    m_jumps->readSessionConfig(config);
    m_marks->readSessionConfig(config);
}

void InputModeManager::writeSessionConfig(KConfigGroup &config)
{
    m_jumps->writeSessionConfig(config);
    m_marks->writeSessionConfig(config);
}

void InputModeManager::reset()
{
    if (m_viVisualMode) {
        m_viVisualMode->reset();
    }
}

KeyMapper *InputModeManager::keyMapper()
{
    return m_keyMapperStack.top().get();
}

void InputModeManager::updateCursor(const KTextEditor::Cursor c)
{
    m_inputAdapter->updateCursor(c);
}

GlobalState *InputModeManager::globalState() const
{
    return m_inputAdapter->globalState();
}

KTextEditor::ViewPrivate *InputModeManager::view() const
{
    return m_view;
}

void InputModeManager::pushKeyMapper(std::shared_ptr<KeyMapper> mapper)
{
    m_keyMapperStack.push(mapper);
}

void InputModeManager::popKeyMapper()
{
    m_keyMapperStack.pop();
}
