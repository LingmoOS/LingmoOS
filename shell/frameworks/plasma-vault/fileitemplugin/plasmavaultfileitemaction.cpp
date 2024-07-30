/*
 * Copyright (C) 2019 Ivan Cukic <ivan.cukic@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "plasmavaultfileitemaction.h"

#include <QAction>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QIcon>

#include <KConfig>
#include <KLocalizedString>
#include <KMountPoint>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(PlasmaVaultFileItemAction, "plasmavaultfileitemaction.json")

PlasmaVaultFileItemAction::PlasmaVaultFileItemAction(QObject *parent, const QVariantList &)
    : KAbstractFileItemActionPlugin(parent)
{
}

QList<QAction *> PlasmaVaultFileItemAction::actions(const KFileItemListProperties &fileItemInfos, QWidget *parentWidget)
{
    if (fileItemInfos.urlList().size() != 1 || !fileItemInfos.isDirectory() || !fileItemInfos.isLocal())
        return {};

    QList<QAction *> actions;
    const QIcon icon = QIcon::fromTheme(QStringLiteral("plasmavault"));

    auto fileItem = fileItemInfos.urlList()[0].toLocalFile();

    auto createAction = [this](const QIcon &icon, const QString &name, QString command, QString device, QWidget *parentWidget) {
        Q_UNUSED(parentWidget)
        QAction *action = new QAction(icon, name, this);

        connect(action, &QAction::triggered, this, [command, device]() {
            auto method = QDBusMessage::createMethodCall("org.kde.kded6", "/modules/plasmavault", "org.kde.plasmavault", command);
            method.setArguments({device});

            QDBusConnection::sessionBus().call(method, QDBus::NoBlock);
        });

        return action;
    };

    KConfig config("plasmavaultrc");
    for (auto group : config.groupList()) {
        auto mountPoint = config.entryMap(group)["mountPoint"];
        if (mountPoint == fileItem) {
            const auto currentMounts = KMountPoint::currentMountPoints();

            const bool mounted = std::any_of(currentMounts.begin(), currentMounts.end(), [mountPoint](const KMountPoint::Ptr &mount) {
                return mount->mountPoint() == mountPoint;
            });

            const QString command = mounted ? "closeVault" : "openVault";
            const QString title = mounted ? i18nc("@action Action to unmount a vault", "Close this Plasma Vault")
                                          : i18nc("@action Action to mount a vault", "Open this Plasma Vault");

            return {createAction(icon, title, command, group, parentWidget)};
        }
    }

    return {};
}

#include "plasmavaultfileitemaction.moc"

#include "moc_plasmavaultfileitemaction.cpp"
