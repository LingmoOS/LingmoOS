/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_LASTCHANGERECORDER_H
#define KATEVI_LASTCHANGERECORDER_H

#include "completion.h"
#include "keyevent.h"

#include <QKeyEvent>
#include <QList>
#include <QString>

namespace KateVi
{
class InputModeManager;

/**
 * In e.g. Insert mode, Qt seems to feed each keypress through twice; once as a ShortcutOverride (even if the key
 * doesn't actually appear to be a ShortcutOverride) and then, whether the "ShortcutOverride" was accepted or not,
 * again as a KeyPress.  We don't want to store both, so this helper helps to decide what to do.
 */
bool isRepeatOfLastShortcutOverrideAsKeyPress(const QKeyEvent &currentKeyPress, const QList<KeyEvent> &keyEventLog);

class LastChangeRecorder
{
public:
    explicit LastChangeRecorder(InputModeManager *viInputModeManager);

    void record(const QKeyEvent &event);
    void dropLast();
    void clear();

    QString encodedChanges() const;

    void replay(const QString &commands, const CompletionList &completions);
    bool isReplaying() const;

private:
    InputModeManager *m_viInputModeManager;

    QList<KeyEvent> m_changeLog;

    bool m_isReplaying;
};
}

#endif // KATEVI_LASTCHANGERECORDER_H
