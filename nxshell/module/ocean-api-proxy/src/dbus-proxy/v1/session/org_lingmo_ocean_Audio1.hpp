// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "session_org_lingmo_ocean_Audio1.h"

#include "session/org_lingmo_ocean_Audio1_Sink.hpp"
#include "session/org_lingmo_ocean_Audio1_Source.hpp"

class SessionAudio1Proxy : public DBusProxyBase {
public:
    SessionAudio1Proxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr)
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        // InitFilterProperies(QStringList({"SinkInputs", "CardsWithoutUnavailable", "DefaultSource", "MaxUIVolume", "DefaultSink", "BluetoothAudioModeOpts"}));
        // InitFilterMethods(QStringList({}));
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::lingmo::ocean::Audio1(m_dbusName, m_dbusPath, QDBusConnection::sessionBus(), this);
        return m_dbusProxy;
    }
    virtual void signalMonitorCustom()
    {
        QString subPathSinkInterface  = "org.lingmo.ocean.Audio1.Sink";
        QString subPathSourceInterface  = "org.lingmo.ocean.Audio1.Source";
        QString subPathSinkProxyInterface;
        QString subPathSourceProxyInterface;
        QString subPathProxyPathPrefix;
        if (m_proxyDbusName == "com.lingmo.daemon.Audio") {
            // V0 -> V1子PATH处理
            subPathSinkProxyInterface = "com.lingmo.daemon.Audio.Sink";
            subPathSourceProxyInterface = "com.lingmo.daemon.Audio.Source";
            subPathProxyPathPrefix = "/com/lingmo/daemon/Audio/";
        } else if (m_proxyDbusName == "org.lingmo.daemon.Audio1") {
            // V0.5 -> V1子PATH处理
            subPathSinkProxyInterface = "org.lingmo.daemon.Audio1.Sink";
            subPathSourceProxyInterface = "org.lingmo.daemon.Audio1.Source";
            subPathProxyPathPrefix = "/org/lingmo/daemon/Audio1/";
        }
        SubPathInit("Sinks", DBusProxySubPathInfo{
                subPathProxyPathPrefix,
                subPathSinkProxyInterface,
                subPathSinkInterface},
                sinkPathMap,
            [=](QString path, QString interface, QString proxyPath, QString proxyInterface){
                return new SessionAudio1SinkProxy(m_dbusName, path, interface,
                    m_proxyDbusName, proxyPath, proxyInterface, m_dbusType);
            }
        );
        SubPathInit("Sources", DBusProxySubPathInfo{
                subPathProxyPathPrefix,
                subPathSourceProxyInterface,
                subPathSourceInterface},
                sourcePathMap,
            [=](QString path, QString interface, QString proxyPath, QString proxyInterface){
                return new SessionAudio1SourceProxy(m_dbusName, path, interface,
                    m_proxyDbusName, proxyPath, proxyInterface, m_dbusType);
            }
        );
        SubPathPropInit("DefaultSink", subPathProxyPathPrefix);
        SubPathPropInit("DefaultSource", subPathProxyPathPrefix);
    }
private:
    org::lingmo::ocean::Audio1 *m_dbusProxy;
    QMap<QString, DBusProxyBase *> sinkPathMap;
    QMap<QString, DBusProxyBase *> sourcePathMap;
};


