// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtimedloop.h"
#include <DObject>
#include <DObjectPrivate>
#include <dthreadutils.h>

#include <QTime>
#include <QTimer>
#include <QLoggingCategory>

DCORE_BEGIN_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logTimedLoop, "dtk.dtimedloop")
#else
Q_LOGGING_CATEGORY(logTimedLoop, "dtk.dtimedloop", QtInfoMsg)
#endif

class DTimedLoopPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DTimedLoop)
public:
    DTimedLoopPrivate(DTimedLoop *qq = nullptr);
    ~DTimedLoopPrivate();

    int m_returnCode = 0;
    QTime m_startTime;
    QTime m_stopTime;
    bool m_timeDumpFlag = false;
    char __padding[3];
    QString m_exectionName;

    void setExecutionName(const QString &executionName);

    class LoopGuard {
        DTimedLoopPrivate *m_p = nullptr;

    public:
        LoopGuard(DTimedLoopPrivate *p)
            : m_p (p)
        {
            m_p->m_startTime = QTime::currentTime();
        }
        ~LoopGuard() {
            m_p->m_stopTime = QTime::currentTime();
            if (!m_p->m_timeDumpFlag) {
                return;
            }
            if (Q_UNLIKELY(m_p->m_exectionName.isEmpty())) {
                qCDebug(logTimedLoop(),
                        "The execution time is %-5d ms",
                        m_p->m_startTime.msecsTo(QTime::currentTime()));
            } else {
                qCDebug(logTimedLoop(),
                        "The execution time is %-5d ms for \"%s\"",
                        m_p->m_startTime.msecsTo(QTime::currentTime()),
                        m_p->m_exectionName.toLocal8Bit().data());

                m_p->m_exectionName.clear();
            }
        }
    };
};

DTimedLoopPrivate::DTimedLoopPrivate(DTimedLoop *qq)
    : DObjectPrivate (qq)
{
}

DTimedLoopPrivate::~DTimedLoopPrivate()
{
}

void DTimedLoopPrivate::setExecutionName(const QString &executionName)
{
    m_exectionName = executionName;
}

DTimedLoop::DTimedLoop(QObject *parent) noexcept
    : QEventLoop (parent)
    , DObject (*new DTimedLoopPrivate(this))
{
}

DTimedLoop::DTimedLoop() noexcept
    : QEventLoop ()
    , DObject (*new DTimedLoopPrivate(this))
{
}

DTimedLoop::~DTimedLoop()
{
}

int DTimedLoop::runningTime() {
    Q_D(DTimedLoop);
    if (QEventLoop::isRunning()) {
        return d->m_startTime.msecsTo(QTime::currentTime());
    }
    return d->m_startTime.msecsTo(d->m_stopTime);
}

void DTimedLoop::setTimeDump(bool flag)
{
    Q_D(DTimedLoop);
    d->m_timeDumpFlag = flag;
}

void DTimedLoop::exit(int returnCode)
{
    // 避免在子线程中提前被执行
    DThreadUtil::runInMainThread([this, returnCode]{
        QEventLoop::exit(returnCode);
    });
}

int DTimedLoop::exec(QEventLoop::ProcessEventsFlags flags)
{
    Q_D(DTimedLoop);
    DTimedLoopPrivate::LoopGuard guard(d);
    return QEventLoop::exec(flags);
}

int DTimedLoop::exec(int durationTimeMs, QEventLoop::ProcessEventsFlags flags)
{
    Q_D(DTimedLoop);
    int runningTime = durationTimeMs < 0 ? 0 : durationTimeMs;
    QTimer::singleShot(runningTime, [this] {
        QEventLoop::exit(0);
    });
    DTimedLoopPrivate::LoopGuard guard(d);
    return QEventLoop::exec(flags);
}

int DTimedLoop::exec(const QString &executionName, QEventLoop::ProcessEventsFlags flags)
{
    Q_D(DTimedLoop);
    d->setExecutionName(executionName);
    return exec(flags);
}

int DTimedLoop::exec(int durationMs, const QString &executionName, QEventLoop::ProcessEventsFlags flags)
{
    Q_D(DTimedLoop);
    d->setExecutionName(executionName);
    return exec(durationMs, flags);
}

DCORE_END_NAMESPACE
