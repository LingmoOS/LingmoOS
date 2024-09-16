// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iowaitwatcher.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "xcursor_remap.h"

#ifdef __cplusplus
}
#endif

#include <float.h>

#include <QTimer>
#include <QFile>
#include <QDebug>

struct CPUState{
    float user;
    float system;
    float idle;
    float iowait;
    float count = 0.0;
};

QDebug operator<<(QDebug argument, const CPUState &state) {
    argument << "user: " << state.user
             << "system: " << state.system
             << "idle: " << state.idle
             << "iowait: " << state.iowait
             << "count: " << state.count;
    return argument;
}

/**
 * @brief IOWaitWatcher::IOWaitWatcher
 * @param parent
 * @note 展示当前cpu的iowait状态，当繁忙时，鼠标光标会变更样式，非繁忙状态时恢复
 */
IOWaitWatcher::IOWaitWatcher(QObject *parent)
    : QObject(parent)
    , m_maxStep(65.0)
    , m_timer(new QTimer(this))
{
    init();
}

void IOWaitWatcher::init()
{
    QByteArray env = qgetenv("DDE_MAX_IOWAIT");
    if (!env.isEmpty()) {
        bool ok;
        float ddeMaxIOWait = env.toFloat(&ok);
        if (ok) {
            m_maxStep = ddeMaxIOWait;
        }
    }

    m_timer->setInterval(4 * 1000);
    connect(m_timer, &QTimer::timeout, this, &IOWaitWatcher::onTimeOut);
}

void IOWaitWatcher::onTimeOut()
{
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "failed to open /proc/stat, reason: " << file.errorString();
        return;
    }

    const QString &line = file.readLine();
    const QStringList &list = line.split(" ", Qt::SkipEmptyParts);
    if (list.size() < 6) {
        qWarning() << "invalid format: " << line;
        return;
    }

    CPUState state;
    state.user = list[1].toFloat();
    state.system = list[3].toFloat();
    state.idle = list[4].toFloat();
    state.iowait = list[5].toFloat();
    state.count = state.user + state.system + state.idle + state.iowait;

    static CPUState lastState;
    if (lastState.count < FLT_EPSILON && lastState.count > -FLT_EPSILON) {
        lastState = state;
        return;
    }

    float count = state.count - lastState.count;
    float userStep = 100.0 * (state.user - lastState.user) / count;
    float systemStep = 100.0 * (state.system - lastState.system) / count;
    float iowaitStep = 100.0 * (state.iowait - lastState.iowait) / count;

    qDebug() << "state of proc: " << state << userStep << systemStep << iowaitStep;

    // init left ptr
    bool enabled = ((userStep >= m_maxStep) || (systemStep >= m_maxStep) || (iowaitStep >= m_maxStep));
    xcLeftPtrToWatch(enabled);
}

void IOWaitWatcher::xcLeftPtrToWatch(bool enabled)
{
    // 避免重复设置
    static bool isWatch = false;
    if (isWatch == enabled)
        return;

    // 更新或恢复鼠标光标样式
    int value = enabled ? 1 : 0;
    int ret = xc_left_ptr_to_watch(value);
    if (ret != 0) {
        qWarning() << "failed to map: " << (enabled ? "left_ptr_watch" : "left_ptr");
        return;
    }

    isWatch = enabled;
}

void IOWaitWatcher::start()
{
    m_timer->start();
}
