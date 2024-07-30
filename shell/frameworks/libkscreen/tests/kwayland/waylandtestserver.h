/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QObject>

// KWayland
#include <KWayland/Server/compositor_interface.h>
#include <KWayland/Server/display.h>
#include <KWayland/Server/dpms_interface.h>
#include <KWayland/Server/outputconfiguration_interface.h>
#include <KWayland/Server/outputdevice_interface.h>
#include <KWayland/Server/outputmanagement_interface.h>
#include <KWayland/Server/seat_interface.h>
#include <KWayland/Server/shell_interface.h>

namespace KScreen
{
static const QString s_socketName = QStringLiteral("libkscreen-test-wayland-backend-0");

using namespace KWayland::Server;

class WaylandTestServer : public QObject
{
    Q_OBJECT

public:
    explicit WaylandTestServer(QObject *parent = nullptr);
    ~WaylandTestServer() override;

    void setConfig(const QString &configfile);
    void start();
    void stop();

    void showOutputs();
    KWayland::Server::Display *display();
    QList<KWayland::Server::OutputDeviceInterface *> outputs() const;

    int outputCount() const;

    void suspendChanges(bool suspend);

Q_SIGNALS:
    void outputsChanged();

    void started();

    void configReceived();
    void configChanged();

private Q_SLOTS:
    void configurationChangeRequested(KWayland::Server::OutputConfigurationInterface *configurationInterface);

private:
    static QString modeString(KWayland::Server::OutputDeviceInterface *outputdevice, int mid);
    QString m_configFile;
    KWayland::Server::Display *m_display;
    QList<KWayland::Server::OutputDeviceInterface *> m_outputs;
    KWayland::Server::OutputManagementInterface *m_outputManagement;
    KWayland::Server::DpmsManagerInterface *m_dpmsManager;
    bool m_suspendChanges;
    KWayland::Server::OutputConfigurationInterface *m_waiting;
};

} // namespace
