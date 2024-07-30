/*
 * SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "globalaccel.h"

#include <QAction>
#include <QFile>
#include <QStandardPaths>

#include <KGlobalAccel>
#include <KLocalizedString>

void GlobalAccel::changeMenuEntryShortcut(const KService::Ptr &service, const QKeySequence &shortcut)
{
    if (!service) {
        return;
    }

    const QString desktopFile = QStringLiteral("%1.desktop").arg(service->desktopEntryName());

    if (!KGlobalAccel::isComponentActive(desktopFile)) {
        const QString destination = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kglobalaccel/") + desktopFile;
        QFile::copy(service->entryPath(), destination);
    }
    QAction action(i18n("Launch %1", service->name()));
    action.setProperty("componentName", desktopFile);
    action.setProperty("componentDisplayName", service->name());
    action.setObjectName(QStringLiteral("_launch"));
    // Make sure that the action is marked active
    KGlobalAccel::self()->setShortcut(&action, {shortcut});
    action.setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setShortcut(&action, {shortcut}, KGlobalAccel::NoAutoloading);
}

QKeySequence GlobalAccel::getMenuEntryShortcut(const KService::Ptr &service)
{
    const QString desktopFile = QStringLiteral("%1.desktop").arg(service->desktopEntryName());
    const QList<QKeySequence> shortcut = KGlobalAccel::self()->globalShortcut(desktopFile, QStringLiteral("_launch"));
    if (!shortcut.isEmpty()) {
        return shortcut[0];
    }
    return QKeySequence();
}
