/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lastchangerecorder.h"
#include "completionreplayer.h"
#include <vimode/inputmodemanager.h>
#include <vimode/keyparser.h>

using namespace KateVi;

bool KateVi::isRepeatOfLastShortcutOverrideAsKeyPress(const QKeyEvent &currentKeyPress, const QList<KeyEvent> &keyEventLog)
{
    if (keyEventLog.empty()) {
        return false;
    }
    const KeyEvent &lastKeyPress = keyEventLog.last();
    if (lastKeyPress.type() == QEvent::ShortcutOverride && currentKeyPress.type() == QEvent::KeyPress && lastKeyPress.key() == currentKeyPress.key()
        && lastKeyPress.modifiers() == currentKeyPress.modifiers()) {
        return true;
    }
    return false;
}

LastChangeRecorder::LastChangeRecorder(InputModeManager *viInputModeManager)
    : m_viInputModeManager(viInputModeManager)
    , m_isReplaying(false)
{
}

void LastChangeRecorder::record(const QKeyEvent &e)
{
    if (isRepeatOfLastShortcutOverrideAsKeyPress(e, m_changeLog)) {
        return;
    }

    if (e.key() != Qt::Key_Shift && e.key() != Qt::Key_Control && e.key() != Qt::Key_Meta && e.key() != Qt::Key_Alt) {
        m_changeLog.append(KeyEvent::fromQKeyEvent(e));
    }
}

void LastChangeRecorder::dropLast()
{
    Q_ASSERT(!m_changeLog.isEmpty());
    m_changeLog.pop_back();
}

void LastChangeRecorder::clear()
{
    m_changeLog.clear();
}

QString LastChangeRecorder::encodedChanges() const
{
    QString result;

    QList<KeyEvent> keyLog = m_changeLog;

    for (int i = 0; i < keyLog.size(); i++) {
        int keyCode = keyLog.at(i).key();
        QString text = keyLog.at(i).text();
        int mods = keyLog.at(i).modifiers();
        QChar key;

        if (text.length() > 0) {
            key = text.at(0);
        }

        if (text.isEmpty() || (text.length() == 1 && text.at(0) < QChar(0x20)) || (mods != Qt::NoModifier && mods != Qt::ShiftModifier)) {
            QString keyPress;

            keyPress.append(QLatin1Char('<'));
            keyPress.append((mods & Qt::ShiftModifier) ? QStringLiteral("s-") : QString());
            keyPress.append((mods & CONTROL_MODIFIER) ? QStringLiteral("c-") : QString());
            keyPress.append((mods & Qt::AltModifier) ? QStringLiteral("a-") : QString());
            keyPress.append((mods & META_MODIFIER) ? QStringLiteral("m-") : QString());
            keyPress.append(keyCode <= 0xFF ? QChar(keyCode) : KeyParser::self()->qt2vi(keyCode));
            keyPress.append(QLatin1Char('>'));

            key = KeyParser::self()->encodeKeySequence(keyPress).at(0);
        }

        result.append(key);
    }

    return result;
}

bool LastChangeRecorder::isReplaying() const
{
    return m_isReplaying;
}

void LastChangeRecorder::replay(const QString &commands, const CompletionList &completions)
{
    m_isReplaying = true;
    m_viInputModeManager->completionReplayer()->start(completions);
    m_viInputModeManager->feedKeyPresses(commands);
    m_viInputModeManager->completionReplayer()->stop();
    m_isReplaying = false;
}
