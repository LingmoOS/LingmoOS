/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "backenddbuswrapper.h"
#include "backendadaptor.h"
#include "kscreen_backendLauncher_debug.h"

#include "abstractbackend.h"
#include "config.h"
#include "configserializer_p.h"

#include <QDBusConnection>
#include <QDBusError>

BackendDBusWrapper::BackendDBusWrapper(KScreen::AbstractBackend *backend)
    : QObject()
    , mBackend(backend)
{
    connect(mBackend, &KScreen::AbstractBackend::configChanged, this, &BackendDBusWrapper::backendConfigChanged);

    mChangeCollector.setSingleShot(true);
    mChangeCollector.setInterval(200); // wait for 200 msecs without any change
                                       // before actually emitting configChanged
    connect(&mChangeCollector, &QTimer::timeout, this, &BackendDBusWrapper::doEmitConfigChanged);
}

BackendDBusWrapper::~BackendDBusWrapper()
{
}

bool BackendDBusWrapper::init()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    new BackendAdaptor(this);
    if (!dbus.registerObject(QStringLiteral("/backend"), this, QDBusConnection::ExportAdaptors)) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Failed to export backend to DBus: another launcher already running?";
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << dbus.lastError().message();
        return false;
    }

    return true;
}

QVariantMap BackendDBusWrapper::getConfig() const
{
    const KScreen::ConfigPtr config = mBackend->config();
    Q_ASSERT(!config.isNull());
    if (!config) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Backend provided an empty config!";
        return QVariantMap();
    }

    const QJsonObject obj = KScreen::ConfigSerializer::serializeConfig(mBackend->config());
    Q_ASSERT(!obj.isEmpty());
    return obj.toVariantMap();
}

QVariantMap BackendDBusWrapper::setConfig(const QVariantMap &configMap)
{
    if (configMap.isEmpty()) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Received an empty config map";
        return QVariantMap();
    }

    const KScreen::ConfigPtr config = KScreen::ConfigSerializer::deserializeConfig(configMap);
    mBackend->setConfig(config);

    mCurrentConfig = mBackend->config();
    QMetaObject::invokeMethod(this, "doEmitConfigChanged", Qt::QueuedConnection);

    // TODO: setConfig should return adjusted config that was actually applied
    const QJsonObject obj = KScreen::ConfigSerializer::serializeConfig(mCurrentConfig);
    Q_ASSERT(!obj.isEmpty());
    return obj.toVariantMap();
}

QByteArray BackendDBusWrapper::getEdid(int output) const
{
    const QByteArray edidData = mBackend->edid(output);
    if (edidData.isEmpty()) {
        return QByteArray();
    }

    return edidData;
}

void BackendDBusWrapper::backendConfigChanged(const KScreen::ConfigPtr &config)
{
    Q_ASSERT(!config.isNull());
    if (!config) {
        qCWarning(KSCREEN_BACKEND_LAUNCHER) << "Backend provided an empty config!";
        return;
    }

    mCurrentConfig = config;
    mChangeCollector.start();
}

void BackendDBusWrapper::doEmitConfigChanged()
{
    Q_ASSERT(!mCurrentConfig.isNull());
    if (mCurrentConfig.isNull()) {
        return;
    }

    const QJsonObject obj = KScreen::ConfigSerializer::serializeConfig(mCurrentConfig);
    Q_EMIT configChanged(obj.toVariantMap());

    mCurrentConfig.clear();
    mChangeCollector.stop();
}

#include "moc_backenddbuswrapper.cpp"
