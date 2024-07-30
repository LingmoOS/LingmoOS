/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KSTANDARDSHORTCUTWATCHER_H
#define KSTANDARDSHORTCUTWATCHER_H

#include "kstandardshortcut.h"

#include <QObject>

#include <memory>

#include <kconfiggui_export.h>

namespace KStandardShortcut
{
class StandardShortcutWatcherPrivate;

/**
 * Watches for changes made to standard shortcuts and notifies about those changes.
 * @see KStandardShortcut::shortcutWatcher
 * @since 5.91
 */
class KCONFIGGUI_EXPORT StandardShortcutWatcher : public QObject
{
    Q_OBJECT
public:
    ~StandardShortcutWatcher();
Q_SIGNALS:
    /**
     * The standardshortcut @p id was changed to @p shortcut
     */
    void shortcutChanged(KStandardShortcut::StandardShortcut id, const QList<QKeySequence> &shortcut);

private:
    KCONFIGGUI_NO_EXPORT explicit StandardShortcutWatcher(QObject *parent = nullptr);

    friend KCONFIGGUI_EXPORT StandardShortcutWatcher *shortcutWatcher();
    std::unique_ptr<StandardShortcutWatcherPrivate> d;
};

/**
 * Returns the global KStandardShortcutWatcher instance of this program.
 * In addition to the notifying about changes it also keeps the information returned by the
 * functions in @p KStandardShortcut up to date.
 * The object is created by the first call to this function.
 * @since 5.91
 */
KCONFIGGUI_EXPORT StandardShortcutWatcher *shortcutWatcher();
}

#endif
