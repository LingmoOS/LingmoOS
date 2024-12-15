/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>
   Copyright (C) 2016 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kserviceactioncomponent.h"
#include "globalshortcutcontext.h"
#include "logging_p.h"

#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QDBusConnectionInterface>


namespace KdeDGlobalAccel {

KServiceActionComponent::KServiceActionComponent(
            const QString &serviceStorageId,
            const QString &friendlyName,
            GlobalShortcutsRegistry *registry)
    :   Component(serviceStorageId, friendlyName, registry),
        m_serviceStorageId(serviceStorageId),
        m_desktopFile(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kglobalaccel/") + serviceStorageId))
    {
    }


KServiceActionComponent::~KServiceActionComponent()
    {
    }

void KServiceActionComponent::emitGlobalShortcutPressed( const GlobalShortcut &shortcut )
{
    QDBusConnectionInterface *dbusDaemon = QDBusConnection::sessionBus().interface();
    const bool klauncherAvailable = dbusDaemon->isServiceRegistered(QStringLiteral("org.kde.klauncher5"));

    //we can't use KRun there as it depends from KIO and would create a circular dep
    if (shortcut.uniqueName() == QStringLiteral("_launch")) {
        QStringList parts = m_desktopFile.desktopGroup().readEntry(QStringLiteral("Exec"), QString()).split(QChar(' '));
        if (parts.isEmpty()) {
            return;
        }
        const QString command = parts.first();
        //sometimes entries have an %u for command line parameters
        if (parts.last().contains(QChar('%'))) {
            parts.pop_back();
        }
        parts.pop_front();

        if (klauncherAvailable) {
            QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                      QStringLiteral("/KLauncher"),
                                                      QStringLiteral("org.kde.KLauncher"),
                                                      QStringLiteral("exec_blind"));
            msg << command << parts;

            QDBusConnection::sessionBus().asyncCall(msg);
        } else {
            QProcess::startDetached(m_desktopFile.desktopGroup().readEntry(QStringLiteral("Exec"), QString()));
        }
        return;
    }
    foreach (const QString &action, m_desktopFile.readActions()) {
        if (action == shortcut.uniqueName()) {
            QStringList parts = m_desktopFile.actionGroup(action).readEntry(QStringLiteral("Exec"), QString()).split(QChar(' '));

            if (parts.isEmpty()) {
                return;
            }
            const QString command = parts.first();
            //sometimes entries have an %u for command line parameters
            if (parts.last().contains(QChar('%'))) {
                parts.pop_back();
            }
            parts.pop_front();

            if (klauncherAvailable) {
                QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                        QStringLiteral("/KLauncher"),
                                                        QStringLiteral("org.kde.KLauncher"),
                                                        QStringLiteral("exec_blind"));
                msg << command << parts;

                QDBusConnection::sessionBus().asyncCall(msg);
            } else {
                QProcess::startDetached(command, parts);
            }
            return;
        }
    }
}



void KServiceActionComponent::loadFromService()
    {

    QString shortcutString;

    QStringList shortcuts = m_desktopFile.desktopGroup().readEntry(QStringLiteral("X-KDE-Shortcuts"), QString()).split(QChar(','));
    if (shortcuts.size() > 0) {
        shortcutString = shortcuts.join(QChar('\t'));
    }

    GlobalShortcut *shortcut = registerShortcut(QStringLiteral("_launch"), m_desktopFile.readName(), shortcutString, shortcutString);
    shortcut->setIsPresent(true);

    foreach(const QString &action, m_desktopFile.readActions())
        {
        shortcuts = m_desktopFile.actionGroup(action).readEntry(QStringLiteral("X-KDE-Shortcuts"), QString()).split(QChar(','));
        if (shortcuts.size() > 0)
            {
            shortcutString = shortcuts.join(QChar('\t'));
            }

        GlobalShortcut *shortcut = registerShortcut(action, m_desktopFile.actionGroup(action).readEntry(QStringLiteral("Name")), shortcutString, shortcutString);
        shortcut->setIsPresent(true);
        }
    }

bool KServiceActionComponent::cleanUp()
{
    qCDebug(KGLOBALACCELD) << "Disabling desktop file";

    for (GlobalShortcut *shortcut : allShortcuts()) {
        shortcut->setIsPresent(false);
    }

    m_desktopFile.desktopGroup().writeEntry("NoDisplay", true);
    m_desktopFile.desktopGroup().sync();

    return Component::cleanUp();
}

} // namespace KdeDGlobalAccel

#include "moc_kserviceactioncomponent.cpp"
