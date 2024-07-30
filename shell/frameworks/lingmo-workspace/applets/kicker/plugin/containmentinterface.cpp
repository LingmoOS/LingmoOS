/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "containmentinterface.h"

#include <Lingmo/Applet>
#include <Lingmo/Containment>
#include <Lingmo/Corona>
#include <LingmoQuick/AppletQuickItem>

#include <KActionCollection>

// FIXME HACK TODO: Unfortunately we have no choice but to hard-code a list of
// applets we know to expose the correct interface right now -- this is slated
// for replacement with some form of generic service.
QStringList ContainmentInterface::m_knownTaskManagers{
    QLatin1String("org.kde.lingmo.taskmanager"),
    QLatin1String("org.kde.lingmo.icontasks"),
    QLatin1String("org.kde.lingmo.expandingiconstaskmanager"),
};

ContainmentInterface::ContainmentInterface(QObject *parent)
    : QObject(parent)
{
}

ContainmentInterface::~ContainmentInterface()
{
}

bool ContainmentInterface::mayAddLauncher(QObject *appletInterface, ContainmentInterface::Target target, const KService::Ptr &service)
{
    if (!appletInterface) {
        return false;
    }

    Lingmo::Applet *applet = appletInterface->property("_lingmo_applet").value<Lingmo::Applet *>();
    Lingmo::Containment *containment = applet->containment();

    if (!containment) {
        return false;
    }

    Lingmo::Corona *corona = containment->corona();

    if (!corona) {
        return false;
    }

    switch (target) {
    case Desktop: {
        containment = corona->containmentForScreen(containment->screen(), QString(), QString());

        if (containment) {
            return (containment->immutability() == Lingmo::Types::Mutable);
        }

        break;
    }
    case Panel: {
        if (containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel")) {
            return (containment->immutability() == Lingmo::Types::Mutable);
        }

        break;
    }
    case TaskManager: {
        if (service && containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel")) {
            auto *taskManager = findTaskManagerApplet(containment);

            if (!taskManager) {
                return false;
            }

            auto *taskManagerQuickItem = LingmoQuick::AppletQuickItem::itemForApplet(taskManager);

            if (!taskManagerQuickItem) {
                return false;
            }

            return taskManagerQuickItem->property("supportsLaunchers").toBool();
        }

        break;
    }
    }

    return false;
}

bool ContainmentInterface::hasLauncher(QObject *appletInterface, ContainmentInterface::Target target, const KService::Ptr &service)
{
    // Only the task manager supports toggle-able launchers
    if (target != TaskManager) {
        return false;
    }
    if (!appletInterface) {
        return false;
    }

    Lingmo::Applet *applet = appletInterface->property("_lingmo_applet").value<Lingmo::Applet *>();
    Lingmo::Containment *containment = applet->containment();

    if (!containment) {
        return false;
    }

    if (service && containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel")) {
        auto *taskManager = findTaskManagerApplet(containment);

        if (!taskManager) {
            return false;
        }

        auto *taskManagerQuickItem = LingmoQuick::AppletQuickItem::itemForApplet(taskManager);

        if (!taskManagerQuickItem) {
            return false;
        }

        QVariant ret;
        QMetaObject::invokeMethod(taskManagerQuickItem,
                                  "hasLauncher",
                                  Q_RETURN_ARG(QVariant, ret),
                                  Q_ARG(QVariant, QUrl(QLatin1String("applications:") + service->storageId())));
        return ret.toBool();
    }

    return false;
}

void ContainmentInterface::addLauncher(QObject *appletInterface, ContainmentInterface::Target target, const QString &entryPath)
{
    if (!appletInterface) {
        return;
    }

    Lingmo::Applet *applet = appletInterface->property("_lingmo_applet").value<Lingmo::Applet *>();
    Lingmo::Containment *containment = applet->containment();

    if (!containment) {
        return;
    }

    Lingmo::Corona *corona = containment->corona();

    if (!corona) {
        return;
    }

    switch (target) {
    case Desktop: {
        containment = corona->containmentForScreen(containment->screen(), QString(), QString());

        if (!containment) {
            return;
        }

        const QStringList &containmentProvides = containment->pluginMetaData().value(QStringLiteral("X-Lingmo-Provides"), QStringList());

        if (containmentProvides.contains(QLatin1String("org.kde.lingmo.filemanagement"))) {
            auto *folderQuickItem = LingmoQuick::AppletQuickItem::itemForApplet(containment);

            if (!folderQuickItem) {
                return;
            }

            QMetaObject::invokeMethod(folderQuickItem, "addLauncher", Q_ARG(QVariant, QUrl::fromLocalFile(entryPath)));
        } else {
            containment->createApplet(QStringLiteral("org.kde.lingmo.icon"), QVariantList() << QUrl::fromLocalFile(entryPath));
        }

        break;
    }
    case Panel: {
        if (containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel")) {
            containment->createApplet(QStringLiteral("org.kde.lingmo.icon"), QVariantList() << QUrl::fromLocalFile(entryPath));
        }

        break;
    }
    case TaskManager: {
        if (containment->pluginMetaData().pluginId() == QLatin1String("org.kde.panel")) {
            auto *taskManager = findTaskManagerApplet(containment);

            if (!taskManager) {
                return;
            }

            auto *taskManagerQuickItem = LingmoQuick::AppletQuickItem::itemForApplet(taskManager);

            if (!taskManagerQuickItem) {
                return;
            }

            QMetaObject::invokeMethod(taskManagerQuickItem, "addLauncher", Q_ARG(QVariant, QUrl::fromLocalFile(entryPath)));
        }

        break;
    }
    }
}

QObject *ContainmentInterface::screenContainment(QObject *appletInterface)
{
    if (!appletInterface) {
        return nullptr;
    }

    const Lingmo::Applet *applet = appletInterface->property("_lingmo_applet").value<Lingmo::Applet *>();
    Lingmo::Containment *containment = applet->containment();

    if (!containment) {
        return nullptr;
    }

    Lingmo::Corona *corona = containment->corona();

    if (!corona) {
        return nullptr;
    }

    return corona->containmentForScreen(containment->screen(), QString(), QString());
}

bool ContainmentInterface::screenContainmentMutable(QObject *appletInterface)
{
    const Lingmo::Containment *containment = static_cast<const Lingmo::Containment *>(screenContainment(appletInterface));

    if (containment) {
        return (containment->immutability() == Lingmo::Types::Mutable);
    }

    return false;
}

void ContainmentInterface::ensureMutable(Lingmo::Containment *containment)
{
    if (containment && containment->immutability() != Lingmo::Types::Mutable) {
        containment->internalAction(QStringLiteral("lock widgets"))->trigger();
    }
}

Lingmo::Applet *ContainmentInterface::findTaskManagerApplet(Lingmo::Containment *containment)
{
    const QList<Lingmo::Applet *> applets = containment->applets();
    const auto found = std::find_if(applets.cbegin(), applets.cend(), [](const Lingmo::Applet *applet) {
        return m_knownTaskManagers.contains(applet->pluginMetaData().pluginId());
    });
    return found != applets.cend() ? *found : nullptr;
}
