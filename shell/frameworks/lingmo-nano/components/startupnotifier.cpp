/*
 *   SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "startupnotifier.h"
#include <KApplicationTrader>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/lingmowindowmanagement.h>
#include <KWayland/Client/registry.h>
#include <KWindowSystem>

#include <QDebug>

StartupNotifier::StartupNotifier(QObject *parent)
    : QObject(parent)
{
    if (!KWindowSystem::isPlatformWayland()) {
        return;
    }
    using namespace KWayland::Client;
    ConnectionThread *connection = ConnectionThread::fromApplication(this);
    if (!connection) {
        return;
    }
    Registry *registry = new Registry(this);
    registry->create(connection);

    connect(registry, &Registry::lingmoActivationFeedbackAnnounced, this, [this, registry](quint32 name, quint32 version) {
        auto iface = registry->createLingmoActivationFeedback(name, version, this);

        connect(iface, &LingmoActivationFeedback::activation, this, [this](LingmoActivation *activation) {
            connect(activation, &LingmoActivation::applicationId, this, [this](const QString &appId) {
                const auto servicesFound = KApplicationTrader::query([&appId](const KService::Ptr &service) {
                    if (service->exec().isEmpty())
                        return false;

                    if (service->desktopEntryName().compare(appId, Qt::CaseInsensitive) == 0)
                        return true;

                    const auto idWithoutDesktop = QString(appId).remove(QStringLiteral(".desktop"));
                    if (service->desktopEntryName().compare(idWithoutDesktop, Qt::CaseInsensitive) == 0)
                        return true;

                    const auto renamedFrom = service->property<QStringList>(QStringLiteral("X-Flatpak-RenamedFrom"));
                    if (renamedFrom.contains(appId, Qt::CaseInsensitive) || renamedFrom.contains(idWithoutDesktop, Qt::CaseInsensitive))
                        return true;

                    return false;
                });

                if (!servicesFound.isEmpty()) {
                    Q_EMIT activationStarted(appId, servicesFound.constFirst()->icon());
                } else {
                    qDebug() << "Could not find" << appId;
                }
            });

            connect(activation, &LingmoActivation::finished, this, &StartupNotifier::activationFinished);
        });
    });

    registry->setup();
}

bool StartupNotifier::isValid() const
{
    return KWindowSystem::isPlatformWayland();
}
