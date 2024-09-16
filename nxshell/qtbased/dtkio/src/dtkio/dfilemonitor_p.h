// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEMONITOR_P_H
#define DFILEMONITOR_P_H

#include <QUrl>

#include <gio/gio.h>

#include <DError>

#include "dtkio_global.h"
#include "dtkiotypes.h"
#include "dfileerror.h"

DIO_BEGIN_NAMESPACE
class DFileMonitor;
class DFileMonitorPrivate
{
public:
    explicit DFileMonitorPrivate(DFileMonitor *q);
    ~DFileMonitorPrivate();

    GFileMonitor *createMonitor(GFile *gfile, WatchType type);
    void setError(IOErrorCode code);

    static void watchCallback(GFileMonitor *gmonitor, GFile *child, GFile *other, GFileMonitorEvent event_type, gpointer user_data);

    DFileMonitor *q = nullptr;
    QUrl url;
    quint32 timeRate = 200;
    WatchType watchType = WatchType::Auto;
    GFileMonitor *gmonitor = nullptr;
    GFile *gfile = nullptr;

    DTK_CORE_NAMESPACE::DError error { IOErrorCode::NoError, IOErrorMessage(IOErrorCode::NoError) };
};
DIO_END_NAMESPACE

#endif   // DFILEMONITOR_P_H
