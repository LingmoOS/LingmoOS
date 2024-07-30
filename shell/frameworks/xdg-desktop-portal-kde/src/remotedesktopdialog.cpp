/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#include "remotedesktopdialog.h"
#include "remotedesktopdialog_debug.h"
#include "utils.h"

#include <KLocalizedString>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QWindow>

RemoteDesktopDialog::RemoteDesktopDialog(const QString &appName, RemoteDesktopPortal::DeviceTypes deviceTypes, bool screenSharingEnabled, ScreenCastPortal::PersistMode persistMode, QObject *parent)
    : QuickDialog(parent)
{
    const QVariantMap props = {
        {QStringLiteral("title"), i18nc("Title of the dialog that requests remote input privileges", "Remote control requested")},
        {QStringLiteral("description"), buildDescription(appName, deviceTypes, screenSharingEnabled)},
        {QStringLiteral("persistenceRequested"), persistMode != ScreenCastPortal::PersistMode::NoPersist}
    };
    create(QStringLiteral("qrc:/RemoteDesktopDialog.qml"), props);
}

bool RemoteDesktopDialog::allowRestore() const
{
    return m_theDialog->property("allowRestore").toBool();
}

QString RemoteDesktopDialog::buildDescription(const QString &appName, RemoteDesktopPortal::DeviceTypes deviceTypes, bool screenSharingEnabled)
{
    const QString applicationName = Utils::applicationName(appName);
    QString description = applicationName.isEmpty()
        ? i18nc("Unordered list with privileges granted to an external process", "An application requested access to:\n")
        : i18nc("Unordered list with privileges granted to an external process, included the app's name",
                "%1 requested access to remotely control:\n",
                applicationName);
    if (screenSharingEnabled) {
        description += i18nc("Will allow the app to see what's on the outputs, in markdown", " - Screens\n");
    }
    if (deviceTypes != RemoteDesktopPortal::None) {
        description += i18nc("Will allow the app to send input events, in markdown", " - Input devices\n");
    }
    return description;
}
