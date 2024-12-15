// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashmonitor.h"

TrashMonitor::TrashMonitor(QObject *parent)
    : QObject(parent)
    , m_trash(g_file_new_for_uri("trash:///"))
    , m_trashMonitor(g_file_monitor_file(m_trash, G_FILE_MONITOR_NONE, NULL, NULL))
{
    g_signal_connect(m_trashMonitor, "changed", G_CALLBACK(slot_onTrashMonitorChanged), this);
}

TrashMonitor::~TrashMonitor()
{
    g_object_unref(m_trashMonitor);
    g_object_unref(m_trash);
}

int TrashMonitor::trashItemCount()
{
    GFileInfo *info;
    gint file_count = 0;

    info = g_file_query_info(m_trash, G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT, G_FILE_QUERY_INFO_NONE, NULL, NULL);
    if (info != NULL) {
        file_count = g_file_info_get_attribute_uint32(info, G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT);
        g_object_unref(info);
    }

    return file_count;
}

void TrashMonitor::onTrashMonitorChanged(GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type)
{
    Q_UNUSED(monitor)
    Q_UNUSED(file)
    Q_UNUSED(other_file)

    if (event_type == G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED) {
        Q_EMIT trashAttributeChanged();
    }
}

void TrashMonitor::slot_onTrashMonitorChanged(GFileMonitor *monitor, GFile *file,
                                         GFile *other_file, GFileMonitorEvent event_type,
                                         gpointer user_data)
{
    TrashMonitor * that = reinterpret_cast<TrashMonitor*>(user_data);
    that->onTrashMonitorChanged(monitor, file, other_file, event_type);
}
