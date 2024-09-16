// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dregionmonitor.h"
#include "private/dregionmonitor_p.h"

#include <QObject>
#include <QDebug>
#include <QGuiApplication>
#include <QtDBus/QtDBus>

DGUI_BEGIN_NAMESPACE

/*!
  \class Dtk::Gui::DRegionMonitor
  \inmodule dtkgui
  \brief 一个在指定区域内监视鼠标键盘动作的类.
 */

/*!
  \enum DRegionMonitor::RegisterdFlag
  DRegionMonitor::RegisterdFlag 定义了 DRegionMonitor 监听标志。

  \var Motion
  代表监听鼠标移动。

  \var Button
  代表监听鼠标按键。

  \var Key
  代表监听键盘按键。
 */

DRegionMonitor::DRegionMonitor(QObject *parent)
    : QObject(parent),
      DObject(*new DRegionMonitorPrivate(this))
{
    D_D(DRegionMonitor);

    d->init();
}

bool DRegionMonitor::registered() const
{
    D_DC(DRegionMonitor);

    return !d->registerKey.isEmpty();
}

QRegion DRegionMonitor::watchedRegion() const
{
    D_DC(DRegionMonitor);

    return d->watchedRegion;
}

DRegionMonitor::RegisterdFlags DRegionMonitor::registerFlags() const
{
    D_DC(DRegionMonitor);

    return d->registerdFlags;
}

DRegionMonitor::CoordinateType DRegionMonitor::coordinateType() const
{
    D_DC(DRegionMonitor);

    return d->type;
}

void DRegionMonitor::registerRegion()
{
    if (registered())
    {
        qWarning() << "region already registered!";
        return;
    }

    D_D(DRegionMonitor);

    d->registerMonitorRegion();
}

void DRegionMonitor::unregisterRegion()
{
    D_D(DRegionMonitor);

    d->unregisterMonitorRegion();
}

void DRegionMonitor::setWatchedRegion(const QRegion &region)
{
    D_D(DRegionMonitor);

    d->watchedRegion = region;
    if (registered())
        d->registerMonitorRegion();
}

/*!
  \property DRegionMonitor::registerdFlags

  \brief 监听模式属性.
  \brief 监听模式，需要注意 DRegionMonitor::Motion 监听鼠标移动会影响性能，默认包含，如果
  需要可通过此函数去掉 DRegionMonitor::Motion .

  registerdFlagChanged 信号会在监听标志 registerdFlags 被改变的时候被触发.

  \a flags 监听模式.

 */
void DRegionMonitor::setRegisterFlags(RegisterdFlags flags)
{
    D_D(DRegionMonitor);

    if (d->registerdFlags == flags)
        return;

    d->registerdFlags = flags;
    if (registered())
        d->registerMonitorRegion();
    Q_EMIT registerdFlagsChanged(flags);
}

void DRegionMonitor::setCoordinateType(DRegionMonitor::CoordinateType type)
{
    D_D(DRegionMonitor);

    d->type = type;
}

DRegionMonitorPrivate::DRegionMonitorPrivate(DRegionMonitor *q)
    : DObjectPrivate(q)
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.deepin.dde.XEventMonitor1"))) {
        eventInter = new XEventMonitor("org.deepin.dde.XEventMonitor1", "/org/deepin/dde/XEventMonitor1",
                                       "org.deepin.dde.XEventMonitor1", q);
    } else {
        eventInter = new XEventMonitor("com.deepin.api.XEventMonitor", "/com/deepin/api/XEventMonitor",
                                       "com.deepin.api.XEventMonitor", q);
    }

}

DRegionMonitorPrivate::~DRegionMonitorPrivate()
{
    if (registered())
        unregisterMonitorRegion();

    eventInter->deleteLater();
}

void DRegionMonitorPrivate::init()
{
    D_Q(DRegionMonitor);

    QObject::connect(eventInter, SIGNAL(ButtonPress(int,int,int,QString)), q, SLOT(_q_ButtonPress(const int, const int, const int, const QString&)));
    QObject::connect(eventInter, SIGNAL(ButtonRelease(int,int,int,QString)), q, SLOT(_q_ButtonRelease(const int, const int, const int, const QString&)));
    QObject::connect(eventInter, SIGNAL(CursorMove(int,int,QString)), q, SLOT(_q_CursorMove(const int, const int, const QString&)));
    QObject::connect(eventInter, SIGNAL(CursorInto(int,int,QString)), q, SLOT(_q_CursorEnter(const int, const int, const QString&)));
    QObject::connect(eventInter, SIGNAL(CursorOut(int,int,QString)), q, SLOT(_q_CursorLeave(const int, const int, const QString&)));
    QObject::connect(eventInter, SIGNAL(KeyPress(QString,int,int,QString)), q, SLOT(_q_KeyPress(const QString&, const int, const int, const QString&)));
    QObject::connect(eventInter, SIGNAL(KeyRelease(QString,int,int,QString)), q, SLOT(_q_KeyRelease(const QString&, const int, const int, const QString&)));
}

void DRegionMonitorPrivate::registerMonitorRegion()
{
    if (registered())
        unregisterMonitorRegion();

    if (watchedRegion.isEmpty())
    {
        // 将监听区域设置为最大
        registerKey = eventInter->RegisterArea(INT_MIN, INT_MIN, INT_MAX, INT_MAX, registerdFlags);
    } else {
        const QRect rect = watchedRegion.boundingRect();
        const int x1 = rect.x();
        const int y1 = rect.y();
        const int x2 = x1 + rect.width();
        const int y2 = y1 + rect.height();

        registerKey = eventInter->RegisterArea(x1, y1, x2, y2, registerdFlags);
    }
}

void DRegionMonitorPrivate::unregisterMonitorRegion()
{
    if (registerKey.isEmpty())
        return;

    eventInter->UnregisterArea(registerKey);
    registerKey.clear();
}

void DRegionMonitorPrivate::_q_ButtonPress(const int flag, const int x, const int y, const QString &key)
{
    if (registerKey != key)
        return;

    D_Q(DRegionMonitor);

    Q_EMIT q->buttonPress(deviceScaledCoordinate(QPoint(x, y), qApp->devicePixelRatio()), flag);
}

void DRegionMonitorPrivate::_q_ButtonRelease(const int flag, const int x, const int y, const QString &key)
{
    if (registerKey != key)
        return;

    D_Q(DRegionMonitor);

    Q_EMIT q->buttonRelease(deviceScaledCoordinate(QPoint(x, y), qApp->devicePixelRatio()), flag);
}

void DRegionMonitorPrivate::_q_CursorMove(const int x, const int y, const QString &key)
{
    if (registerKey != key)
        return;

    D_Q(DRegionMonitor);

    Q_EMIT q->cursorMove(deviceScaledCoordinate(QPoint(x, y), qApp->devicePixelRatio()));
}

void DRegionMonitorPrivate::_q_CursorEnter(const int x, const int y, const QString &key)
{
    if (registerKey != key)
        return;

    D_Q(DRegionMonitor);

    Q_EMIT q->cursorEnter(deviceScaledCoordinate(QPoint(x, y), qApp->devicePixelRatio()));
}

void DRegionMonitorPrivate::_q_CursorLeave(const int x, const int y, const QString &key)
{
    if (registerKey != key)
        return;

    D_Q(DRegionMonitor);

    Q_EMIT q->cursorLeave(deviceScaledCoordinate(QPoint(x, y), qApp->devicePixelRatio()));
}

void DRegionMonitorPrivate::_q_KeyPress(const QString &keyname, const int x, const int y, const QString &key)
{
    if (registerKey != key)
        return;

    Q_UNUSED(x);
    Q_UNUSED(y);

    D_Q(DRegionMonitor);

    Q_EMIT q->keyPress(keyname);
}

void DRegionMonitorPrivate::_q_KeyRelease(const QString &keyname, const int x, const int y, const QString &key)
{
    if (registerKey != key)
        return;

    Q_UNUSED(x);
    Q_UNUSED(y);

    D_Q(DRegionMonitor);

    Q_EMIT q->keyRelease(keyname);
}

const QPoint DRegionMonitorPrivate::deviceScaledCoordinate(const QPoint &p, const double ratio) const
{
    D_QC(DRegionMonitor);

    if (type == q->Original) {
        return p;
    }

    for (const auto *s : qApp->screens())
    {
        const QRect &g(s->geometry());
        const QRect realRect(g.topLeft(), g.size() * ratio);

        if (realRect.contains(p))
            return QPoint(realRect.topLeft() + (p - realRect.topLeft()) / ratio);
    }

    return p / ratio;
}

DGUI_END_NAMESPACE

#include "moc_dregionmonitor.cpp"
