/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osdmanager.h"
#include "osd.h"
#include "osdaction.h"
#include "powerprofileosdserviceadaptor.h"

#include <QDBusConnection>
#include <QDBusMessage>

#include <QQmlEngine>

namespace PowerDevil
{
OsdManager::OsdManager(QObject *parent)
    : QObject(parent)
{
    qmlRegisterUncreatableMetaObject(PowerDevil::OsdAction::staticMetaObject,
                                     "org.kde.powerdevil",
                                     1,
                                     0,
                                     "OsdAction",
                                     QStringLiteral("Can't create OsdAction"));
    new PowerProfileOsdServiceAdaptor(this);

    // free up memory when the osd hasn't been used for more than 1 minute
    m_cleanupTimer->setInterval(60000);
    m_cleanupTimer->setSingleShot(true);
    connect(m_cleanupTimer, &QTimer::timeout, this, &OsdManager::quit);

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kde/powerdevil/powerProfileOsdService"), this, QDBusConnection::ExportAdaptors);
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.powerdevil.powerProfileOsdService"));
}

void OsdManager::hideOsd() const
{
    // Let QML engine finish execution of signal handlers, if any.
    QTimer::singleShot(0, this, &OsdManager::quit);
}

void OsdManager::quit()
{
    delete m_osd;
    qApp->quit();
}

void OsdManager::showOsd()
{
    // BUG: 483948 - Show OSD only once, prevent mem leak
    if (m_osd) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Solid.PowerManagement"),
                                                          QStringLiteral("/org/kde/Solid/PowerManagement/Actions/PowerProfile"),
                                                          QStringLiteral("org.kde.Solid.PowerManagement.Actions.PowerProfile"),
                                                          QStringLiteral("currentProfile"));
    auto reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        return;
    }
    QString currentProfile = reply.arguments().first().toString();

    m_osd = new PowerDevil::Osd(this);
    m_osd->showActionSelector(currentProfile);
    m_cleanupTimer->start();

    connect(m_osd, &Osd::osdActionSelected, this, [this](QString profile) {
        applyProfile(profile);
        hideOsd();
    });

    // Cancel and close, if focus was lost (eg. click outside of OSD)
    connect(m_osd, &Osd::osdActiveChanged, this, [this](bool active) {
        if (!active) {
            quit();
        }
    });
}

void OsdManager::applyProfile(const QString &profile)
{
    if (profile.isEmpty()) {
        return;
    }
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.Solid.PowerManagement"),
                                                          QStringLiteral("/org/kde/Solid/PowerManagement/Actions/PowerProfile"),
                                                          QStringLiteral("org.kde.Solid.PowerManagement.Actions.PowerProfile"),
                                                          QStringLiteral("setProfile"));
    message.setArguments({profile});
    QDBusConnection::sessionBus().call(message);
}

} // namespace PowerDevil

#include "moc_osdmanager.cpp"
