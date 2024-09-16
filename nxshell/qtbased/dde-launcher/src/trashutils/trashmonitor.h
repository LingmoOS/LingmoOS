// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#undef signals
#include <gio/gio.h>
#define signals Q_SIGNALS

class TrashMonitor: public QObject
{
    Q_OBJECT

public:
    explicit TrashMonitor(QObject * parent);
    ~TrashMonitor();

    int trashItemCount();

Q_SIGNALS:
    void trashAttributeChanged();

private:
    GFile * m_trash;
    GFileMonitor * m_trashMonitor;

    void onTrashMonitorChanged(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type);
    static void slot_onTrashMonitorChanged(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, gpointer user_data);
};
