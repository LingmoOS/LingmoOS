/*
 * SPDX-FileCopyrightText: 2020 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2020 Jan Grulich <jgrulich@redhat.com>
 */

#include "userinfodialog.h"

#include "user_interface.h"

#include <sys/types.h>
#include <unistd.h>

#include <QFileInfo>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardPaths>

#include <KLocalizedString>

UserInfoDialog::UserInfoDialog(const QString &reason, QObject *parent)
    : QuickDialog(parent)
{
    QString ifacePath = QStringLiteral("/org/freedesktop/Accounts/User%1").arg(getuid());
    m_userInterface = new OrgFreedesktopAccountsUserInterface(QStringLiteral("org.freedesktop.Accounts"), ifacePath, QDBusConnection::systemBus(), this);

    QString image = QFileInfo::exists(m_userInterface->iconFile()) ? m_userInterface->iconFile() : QString();
    QVariantMap props = {{"title", i18n("Share Information")},
                         {"subtitle", i18n("Share your personal information with the requesting application?")},
                         {"reason", reason},
                         {"userName", m_userInterface->userName()},
                         {"realName", m_userInterface->realName()}};

    if (QFileInfo::exists(m_userInterface->iconFile())) {
        props.insert(QStringLiteral("iconName"), m_userInterface->iconFile());
    } else {
        props.insert(QStringLiteral("iconName"), QStringLiteral("user-identity"));
    }
    create("qrc:/UserInfoDialog.qml", props);
}

UserInfoDialog::~UserInfoDialog()
{
}

QString UserInfoDialog::id() const
{
    return m_userInterface->userName();
}

QString UserInfoDialog::image() const
{
    return QUrl::fromLocalFile(m_userInterface->iconFile()).toString();
}

QString UserInfoDialog::name() const
{
    return m_userInterface->realName();
}
