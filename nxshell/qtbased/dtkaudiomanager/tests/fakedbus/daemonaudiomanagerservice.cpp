// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daemonaudiomanagerservice.h"
#include "daemonhelpers.hpp"

#include <QDebug>
#include <QDBusError>
#include <QDBusConnection>

FakeDaemonService::FakeDaemonService(const QString &path,
                                     const QString &interface, QObject *parent)
    : QObject (parent)
    , m_serviceName(DDaemonInternal::AudioServiceName)
    , m_servicePath(path.isEmpty() ? DDaemonInternal::AudioPath : path)
    , m_serviceInterface(interface.isEmpty() ? DDaemonInternal::AudioServiceInterface : interface)
{
    registerService();
}

FakeDaemonService::~FakeDaemonService()
{
    unregisterService();
}

void FakeDaemonService::registerService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(m_serviceName)) {
        QString errorMsg = bus.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";
        qWarning() << QString("Can't register the %1 service, %2.").arg(m_serviceName).arg(errorMsg);
    }
    if (!bus.registerObject(m_servicePath, m_serviceInterface, this,
                            QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(m_servicePath);
    }
}

void FakeDaemonService::unregisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(m_servicePath);
    bus.unregisterService(m_serviceName);
}

FakeDaemonAudioManagerService::FakeDaemonAudioManagerService(QObject *parent)
    : FakeDaemonService (QString(), QString(), parent)
{
    init();
}

FakeDaemonAudioManagerService::~FakeDaemonAudioManagerService()
{
}

void FakeDaemonAudioManagerService::init()
{
    m_Cards = u8R"([
              {
                  "Id": 0,
                  "Name": "HDA Intel PCH",
                  "Ports": [
                      {
                          "Name": "analog-input-front-mic",
                          "Enabled": true,
                          "Bluetooth": false,
                          "Description": "前麦克风",
                          "Direction": 2
                      },
                      {
                          "Name": "analog-input-rear-mic",
                          "Enabled": true,
                          "Bluetooth": false,
                          "Description": "后麦克风",
                          "Direction": 2
                      }
                  ]
              }
          ])";

    auto sink = new FakeDaemonAudioSinkService(DDaemonInternal::AudioPath + "/" + DDaemonInternal::SinkFlag + "11",
                                                    this);
    sink->setCard(0);
    m_Sinks << sink->path();
    Q_EMIT SinkInputsChanged(m_Sinks);

    auto sinkInput = new FakeDaemonAudioSinkInputService(DDaemonInternal::AudioPath + "/" + DDaemonInternal::SinkInputFlag + "111", this);
    sinkInput->setSink(11);
    m_SinkInputs << sinkInput->path();
    Q_EMIT SinkInputsChanged(m_SinkInputs);

    auto source = new FakeDaemonAudioSourceService(DDaemonInternal::AudioPath + "/" + DDaemonInternal::SourceFlag + "21",
                                                        this);
    source->setCard(0);
    m_Sources << source->path();
    Q_EMIT SinkInputsChanged(m_Sources);
}

FakeDaemonAudioDeviceService::FakeDaemonAudioDeviceService(const QString &path, const QString &interface, QObject *parent)
    : FakeDaemonService (path, interface, parent)
{

}

FakeDaemonAudioSourceService::FakeDaemonAudioSourceService(const QString &path, QObject *parent)
    : FakeDaemonAudioDeviceService (path, DDaemonInternal::AudioServiceSourceInterface, parent)
{
    init();
}

void FakeDaemonAudioSourceService::init()
{
}

FakeDaemonAudioSinkService::FakeDaemonAudioSinkService(const QString &path, QObject *parent)
    : FakeDaemonAudioDeviceService (path, DDaemonInternal::AudioServiceSinkInterface, parent)
{
    init();
}

void FakeDaemonAudioSinkService::init()
{
}

FakeDaemonAudioSinkInputService::FakeDaemonAudioSinkInputService(const QString &path, QObject *parent)
    : FakeDaemonAudioStreamService(path, DDaemonInternal::AudioServiceSinkInputInterface, parent)
{
    init();
}

FakeDaemonAudioSourceOutputService::FakeDaemonAudioSourceOutputService(const QString &path, QObject *parent)
    : FakeDaemonAudioStreamService(path, DDaemonInternal::AudioServiceSourceOutputInterface, parent)
{
    init();
}

FakeDaemonAudioStreamService::FakeDaemonAudioStreamService(const QString &path, const QString &interface, QObject *parent)
    : FakeDaemonService (path, interface, parent)
{

}
