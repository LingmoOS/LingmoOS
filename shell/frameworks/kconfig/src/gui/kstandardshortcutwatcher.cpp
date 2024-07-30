/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kstandardshortcutwatcher.h"

#include "kconfigwatcher.h"
#include "kstandardshortcut_p.h"

namespace KStandardShortcut
{
class StandardShortcutWatcherPrivate
{
public:
    KConfigWatcher::Ptr watcher = KConfigWatcher::create(KSharedConfig::openConfig());
};

StandardShortcutWatcher::StandardShortcutWatcher(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<StandardShortcutWatcherPrivate>())
{
    connect(d->watcher.get(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group, const QByteArrayList &keys) {
        if (group.name() != QStringLiteral("Shortcuts")) {
            return;
        }
        for (const auto &key : keys) {
            const StandardShortcut shortcut = KStandardShortcut::findByName(QString::fromUtf8(key));
            if (shortcut != KStandardShortcut::AccelNone) {
                initialize(shortcut);
                Q_EMIT shortcutChanged(shortcut, KStandardShortcut::shortcut(shortcut));
            }
        }
    });
}

StandardShortcutWatcher::~StandardShortcutWatcher() = default;

StandardShortcutWatcher *shortcutWatcher()
{
    static StandardShortcutWatcher watcher;
    return &watcher;
}

}

#include "moc_kstandardshortcutwatcher.cpp"
