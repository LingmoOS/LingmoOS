// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "brightnesscontroller.h"
#include "brightness-constants.h"
#include "monitor.h"
#include "brightnessmodel.h"

#include <QDebug>
#include <QtConcurrent>

#define GSETTINGS_MINIMUM_BRIGHTNESS "brightness-minimum"

const QString DisplayInterface("com.deepin.daemon.Display");
const QString DisplaySystemInterface("com.deepin.system.Display");
const QString DisplaySystemPath("/com/deepin/system/Display");

BrightnessController::BrightnessController(QObject *parent)
    : QObject(parent)
    , m_displayInter(DisplayInterface, "/com/deepin/daemon/Display", QDBusConnection::sessionBus(), this)
    , m_displayDBusInter(new QDBusInterface("com.deepin.daemon.Display",
                                            "/com/deepin/daemon/Display",
                                            "com.deepin.daemon.Display",
                                            QDBusConnection::sessionBus()))
    , m_gSettings(new QGSettings("com.deepin.dde.control-center", QByteArray(), this))
    , m_supportBrightness(false)
{

}

BrightnessController::~BrightnessController()
{

}

void BrightnessController::init()
{
    connect(&m_displayInter, &DisplayInter::MonitorsChanged, this, &BrightnessController::onMonitorListChanged);
    connect(&m_displayInter, &DisplayInter::BrightnessChanged, this, &BrightnessController::onMonitorsBrightnessChanged);
    connect(&m_displayInter, &DisplayInter::BrightnessChanged, &BrightnessModel::ref(), &BrightnessModel::setBrightnessMap);
    connect(&m_displayInter, &DisplayInter::DisplayModeChanged, this, [this](uchar value) {
        // 显示模式以m_displayInter.GetRealDisplayMode()为准
        BrightnessModel::ref().setDisplayMode(m_displayInter.GetRealDisplayMode());
    });
    connect(&m_displayInter, &DisplayInter::PrimaryChanged, &BrightnessModel::ref(), &BrightnessModel::setPrimary);

    // 获取内建屏幕
    QDBusMessage reply = m_displayInter.call("GetBuiltinMonitor");
    if (reply.type() != QDBusMessage::ErrorMessage) {
        auto outArgs = reply.arguments();
        if (outArgs.count() > 0) {
            BrightnessModel::ref().setBuildinMonitorName(outArgs.at(0).toString());
        } else {
            qCWarning(BRIGHTNESS) << "Call `GetBuiltinMonitor` failed, out arguments is empty";
        }
    } else {
        qCWarning(BRIGHTNESS) << "Call `GetBuiltinMonitor` error: " << reply.errorMessage();
    }

    BrightnessModel::ref().setMinimumBrightnessScale(m_gSettings->get(GSETTINGS_MINIMUM_BRIGHTNESS).toDouble());
    BrightnessModel::ref().setDisplayMode(m_displayInter.GetRealDisplayMode());
    BrightnessModel::ref().setPrimary(m_displayInter.primary());
    BrightnessModel::ref().setBrightnessMap(m_displayInter.brightness());
    onMonitorListChanged(m_displayInter.monitors());
}

void BrightnessController::onGSettingsChanged(const QString &key)
{
    const QVariant &value = m_gSettings->get(key);

    if (key == GSETTINGS_MINIMUM_BRIGHTNESS || key == "brightnessMinimum") {
        BrightnessModel::ref().setMinimumBrightnessScale(value.toDouble());
    }
}

void BrightnessController::onMonitorListChanged(const QList<QDBusObjectPath> &mons)
{
    QList<QString> ops;
    for (const auto *mon : m_monitors.keys()) {
        if (mon) {
            ops << mon->path();
        }
    }

    qCDebug(BRIGHTNESS) << "Monitor list changed, monitor list size: " << mons.size();
    QList<QString> pathList;
    for (const auto &op : mons) {
        const QString path = op.path();
        pathList << path;
        if (!ops.contains(path))
            monitorAdded(path);
    }

    for (const auto &op : ops)
        if (!pathList.contains(op))
            monitorRemoved(op);
}


void BrightnessController::monitorAdded(const QString &path)
{
    MonitorInter *inter = new MonitorInter(DisplayInterface, path, QDBusConnection::sessionBus(), this);
    Monitor *mon = new Monitor(this);
    connect(inter, &MonitorInter::NameChanged, mon, &Monitor::setName);
    connect(inter, &MonitorInter::EnabledChanged, mon, &Monitor::setMonitorEnable);
    // NOTE: DO NOT using async dbus call. because we need to have a unique name to distinguish each monitor
    Q_ASSERT(inter->isValid());
    mon->setName(inter->name());
    QDBusReply<bool> reply = m_displayDBusInter->call("CanSetBrightness", inter->name());
    mon->setCanBrightness(reply.value());
    mon->setMonitorEnable(inter->enabled());
    mon->setPath(path);
    mon->setPrimary(m_displayInter.primary());
    if (!BrightnessModel::ref().brightnessMap().isEmpty()) {
        mon->setBrightness(BrightnessModel::ref().brightnessMap()[mon->name()]);
    }
    mon->setBuildin(BrightnessModel::ref().buildinMonitor() == mon->name());

    BrightnessModel::ref().monitorAdded(mon);
    m_monitors.insert(mon, inter);
    updateSupportBrightnessState();

    inter->setSync(false);
}

void BrightnessController::monitorRemoved(const QString &path)
{
    Monitor *monitor = nullptr;
    for (auto it(m_monitors.cbegin()); it != m_monitors.cend(); ++it) {
        if (it.key() && it.key()->path() == path) {
            monitor = it.key();
            break;
        }
    }
    if (!monitor)
        return;

    BrightnessModel::ref().monitorRemoved(monitor);

    m_monitors[monitor]->deleteLater();
    m_monitors.remove(monitor);
    updateSupportBrightnessState();

    monitor->deleteLater();
}

void BrightnessController::updateSupportBrightnessState()
{
    bool support = false;
    for (auto monitor : m_monitors.keys()) {
        if (monitor->canBrightness()) {
            support = true;
            break;
        }
    }

    if (support != m_supportBrightness) {
        m_supportBrightness = support;
        Q_EMIT supportBrightnessChanged(support);
    }
}

void BrightnessController::onMonitorsBrightnessChanged(const BrightnessMap &brightness)
{
    if (brightness.isEmpty())
        return;

    for (auto it = m_monitors.begin(); it != m_monitors.end(); ++it) {
        if (it.key())
            it.key()->setBrightness(brightness[it.key()->name()]);
    }
}

void BrightnessController::setMonitorBrightness(Monitor *mon, const double brightness)
{
    if (!mon)
        return;

    double value = std::max(brightness, BrightnessModel::ref().minimumBrightnessScale());
    qCDebug(BRIGHTNESS) << "Set monitor brightness, receive request name: " << mon->name() << ", value: " << value;

    //前面亮度设置未完成，只记录最新请求
    QMutexLocker loker(&m_brightness.m_brightnessMutex);
    m_brightness.m_hasWaitingRequest = true;
    m_brightness.m_brightnessValue = value;
    m_brightness.m_monitorName = mon->name();
    if (!m_brightness.m_hasPendingRequest) {
        QTimer::singleShot(0, this, &BrightnessController::handleSetBrightnessRequest);
    }
}

void BrightnessController::handleSetBrightnessRequest() {
    qCInfo(BRIGHTNESS) << "Set monitor brightness enter";
    QMutexLocker loker(&m_brightness.m_brightnessMutex);
    if (m_brightness.m_hasPendingRequest) {
        return;
    }
    if (!m_brightness.m_hasWaitingRequest) {
        return;
    }

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<QDBusPendingReply<>>::finished, [this, watcher] {
        QMutexLocker loker(&m_brightness.m_brightnessMutex);
        m_brightness.m_hasPendingRequest = false;
        qCInfo(BRIGHTNESS) << "Set monitor brightness finish";
        if (m_brightness.m_hasWaitingRequest) {
            QTimer::singleShot(0, this, &BrightnessController::handleSetBrightnessRequest);
        }
        watcher->deleteLater();
    });

    QFuture<void> future = QtConcurrent::run([this] {
        QMutexLocker loker(&m_brightness.m_brightnessMutex);
        if (m_brightness.m_hasPendingRequest) {
            return;
        }
        if (!m_brightness.m_hasWaitingRequest) {
            return;
        }
        m_brightness.m_hasPendingRequest = true;
        m_brightness.m_hasWaitingRequest = false;
        QString name = m_brightness.m_monitorName;
        double value = m_brightness.m_brightnessValue;
        m_brightness.m_monitorName = "";
        m_brightness.m_brightnessValue = 0;
        loker.unlock();
        qCDebug(BRIGHTNESS) << "Set monitor brightness begin, name: " << name << ", value: " << value;
        m_displayInter.SetAndSaveBrightness(name, value).waitForFinished();
    });

    watcher->setFuture(future);
}
