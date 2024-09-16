// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DREGIONMONITOR_P_H
#define DREGIONMONITOR_P_H

#include "dregionmonitor.h"
#include "xeventmonitor_interface.h"

#include <dtkgui_global.h>
#include <DObjectPrivate>

#include <QRegion>
#include <QScreen>

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

using XEventMonitor = ::com::deepin::api::XEventMonitor;

class DRegionMonitorPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DRegionMonitor)

public:
    DRegionMonitorPrivate(DRegionMonitor *q);
    ~DRegionMonitorPrivate();

    bool registered() const { return !registerKey.isEmpty(); }

    void init();
    void registerMonitorRegion();
    void unregisterMonitorRegion();

    void _q_ButtonPress(const int flag, const int x, const int y, const QString &key);
    void _q_ButtonRelease(const int flag, const int x, const int y, const QString &key);
    void _q_CursorMove(const int x, const int y, const QString &key);
    void _q_CursorEnter(const int x, const int y, const QString &key);
    void _q_CursorLeave(const int x, const int y, const QString &key);
    void _q_KeyPress(const QString &keyname, const int x, const int y, const QString &key);
    void _q_KeyRelease(const QString &keyname, const int x, const int y, const QString &key);

    const QPoint deviceScaledCoordinate(const QPoint &p, const double ratio) const;

    XEventMonitor *eventInter;
    QRegion watchedRegion;
    QString registerKey;
    DRegionMonitor::CoordinateType type = DRegionMonitor::ScaleRatio;
    DRegionMonitor::RegisterdFlags registerdFlags = DRegionMonitor::Motion | DRegionMonitor::Button | DRegionMonitor::Key;
};

DGUI_END_NAMESPACE

#endif // DREGIONMONITOR_P_H
