/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "waylandtestserver.h"

#include "waylandconfigreader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QStandardPaths>

#include <utility>

#include "../src/edid.h"

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND_TESTSERVER, "kscreen.kwayland.testserver")

using namespace KScreen;
using namespace KWayland::Server;

WaylandTestServer::WaylandTestServer(QObject *parent)
    : QObject(parent)
    , m_configFile(QLatin1String(TEST_DATA) + QLatin1String("default.json"))
    , m_display(nullptr)
    , m_outputManagement(nullptr)
    , m_dpmsManager(nullptr)
    , m_suspendChanges(false)
    , m_waiting(nullptr)
{
}

WaylandTestServer::~WaylandTestServer()
{
    stop();
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Wayland server shut down.";
}

void WaylandTestServer::start()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new KWayland::Server::Display(this);
    if (qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        m_display->setSocketName(s_socketName);
    } else {
        m_display->setSocketName(QString::fromLatin1(qgetenv("WAYLAND_DISPLAY")));
    }
    m_display->start();

    auto manager = m_display->createDpmsManager();
    manager->create();

    m_outputManagement = m_display->createOutputManagement();
    m_outputManagement->create();
    connect(m_outputManagement, &OutputManagementInterface::configurationChangeRequested, this, &WaylandTestServer::configurationChangeRequested);

    KScreen::WaylandConfigReader::outputsFromConfig(m_configFile, m_display, m_outputs);
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << QStringLiteral("export WAYLAND_DISPLAY=") + m_display->socketName();
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << QStringLiteral("You can specify the WAYLAND_DISPLAY for this server by exporting it in the environment");
    // showOutputs();
}

void WaylandTestServer::stop()
{
    qDeleteAll(m_outputs);
    m_outputs.clear();
    // actually stop the Wayland server
    delete m_display;
    m_display = nullptr;
}

KWayland::Server::Display *WaylandTestServer::display()
{
    return m_display;
}

void WaylandTestServer::setConfig(const QString &configfile)
{
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Creating Wayland server from " << configfile;
    m_configFile = configfile;
}

int WaylandTestServer::outputCount() const
{
    return m_outputs.count();
}
QList<KWayland::Server::OutputDeviceInterface *> WaylandTestServer::outputs() const
{
    return m_outputs;
}

void WaylandTestServer::suspendChanges(bool suspend)
{
    if (m_suspendChanges == suspend) {
        return;
    }
    m_suspendChanges = suspend;
    if (!suspend && m_waiting) {
        m_waiting->setApplied();
        m_waiting = nullptr;
        Q_EMIT configChanged();
    }
}

void WaylandTestServer::configurationChangeRequested(KWayland::Server::OutputConfigurationInterface *configurationInterface)
{
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Server received change request, changes:" << configurationInterface->changes().count();
    Q_EMIT configReceived();

    auto changes = configurationInterface->changes();
    for (auto it = changes.constBegin(); it != changes.constEnd(); ++it) {
        auto outputdevice = it.key();
        auto c = it.value();
        if (c->enabledChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Setting enabled:";
            outputdevice->setEnabled(c->enabled());
        }
        if (c->modeChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Setting new mode:" << c->mode() << modeString(outputdevice, c->mode());
            outputdevice->setCurrentMode(c->mode());
        }
        if (c->transformChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Server setting transform: " << (int)(c->transform());
            outputdevice->setTransform(c->transform());
        }
        if (c->positionChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Server setting position: " << c->position();
            outputdevice->setGlobalPosition(c->position());
        }
        if (c->scaleChanged()) {
            qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "Setting scale:" << c->scaleF();
            outputdevice->setScaleF(c->scaleF());
        }
    }

    if (m_suspendChanges) {
        Q_ASSERT(!m_waiting);
        m_waiting = configurationInterface;
        return;
    }

    configurationInterface->setApplied();
    // showOutputs();
    Q_EMIT configChanged();
}

void WaylandTestServer::showOutputs()
{
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "******** Wayland server running: " << m_outputs.count() << " outputs. ********";
    for (const auto &o : std::as_const(m_outputs)) {
        bool enabled = (o->enabled() == KWayland::Server::OutputDeviceInterface::Enablement::Enabled);
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "  * Output id: " << o->uuid();
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "      Enabled: " << (enabled ? "enabled" : "disabled");
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "         Name: " << QStringLiteral("%2-%3").arg(o->manufacturer(), o->model());
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "         Mode: " << modeString(o, o->currentModeId());
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "          Pos: " << o->globalPosition();
        qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "         Edid: " << o->edid();
        // << o->currentMode().size();
    }
    qCDebug(KSCREEN_WAYLAND_TESTSERVER) << "******************************************************";
}

QString WaylandTestServer::modeString(KWayland::Server::OutputDeviceInterface *outputdevice, int mid)
{
    QString s;
    QString ids;
    int _i = 0;
    for (const auto &_m : outputdevice->modes()) {
        _i++;
        if (_i < 6) {
            ids.append(QString::number(_m.id) + QLatin1String(", "));
        } else {
            ids.append(QLatin1Char('.'));
        }
        if (_m.id == mid) {
            s = QStringLiteral("%1x%2 @%3").arg(QString::number(_m.size.width()), QString::number(_m.size.height()), QString::number(_m.refreshRate));
        }
    }
    return QStringLiteral("[%1] %2 (%4 modes: %3)").arg(QString::number(mid), s, ids, QString::number(outputdevice->modes().count()));
}

#include "moc_waylandtestserver.cpp"
