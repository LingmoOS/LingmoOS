/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#include "favorite-vfs-file-monitor.h"

#include <glib.h>
#include <glib/gprintf.h>

#include <QDebug>
#include <bookmark-manager.h>

enum
{
    VFS_FAVORITE_FILE_MONITOR_CHANGE,
    VFS_FAVORITE_FILE_MONITOR_LAST
};

struct _QueuedEvent
{
    GFileMonitorEvent       eventType;
    GFile*                  child;
    GFile*                  other;
};

struct _PendingChange
{
    gchar*                  child;
    guint64                 lastEmission    : 63;
    guint64                 dirty           : 1;
};

struct _FavoriteVFSFileMonitorSource
{
    GSource                 source;
    GMutex                  lock;
    GWeakRef                instanceRef;
    GFileMonitorFlags       flags;
    gchar*                  dirname;
    gchar*                  basename;
    gchar*                  filename;
    GSequence*              pendingChanges;
    GHashTable*             pendingChangesTable;
    GQueue                  eventQueue;
    gint64                  rateLimit;
};

struct _FavoriteVFSFileMonitorPrivate
{
    GList*                  fileList;
};

typedef struct _QueuedEvent     QueuedEvent;
typedef struct _PendingChange   PendingChange;

#define DEFAULT_RATE_LIMIT                              800 * G_TIME_SPAN_MILLISECOND
#define VIRTUAL_CHANGES_DONE_DELAY                      2   * G_TIME_SPAN_SECOND

G_DEFINE_TYPE (FavoriteVFSFileMonitor, vfs_favorite_file_monitor, G_TYPE_FILE_MONITOR)

static guint str_hash0 (gconstpointer str);
static void pending_change_free (gpointer data);
static void queued_event_free (QueuedEvent *event);
static gboolean str_equal0 (gconstpointer a, gconstpointer b);
static void vfs_favorite_file_monitor_dispose (GObject* obj);
static void vfs_favorite_file_monitor_finalize (GObject* obj);
static void vfs_favorite_file_monitor_source_finalize (GSource *source);
static void vfs_favorite_file_monitor_g_file_monitor_init (GFileMonitor* iface);
static gint64 g_file_monitor_source_get_ready_time (FavoriteVFSFileMonitorSource* fms);
static void g_file_monitor_source_update_ready_time (FavoriteVFSFileMonitorSource* fms);
static int pending_change_compare_ready_time (gconstpointer a_p, gconstpointer b_p, gpointer udata);
static gint64 pending_change_get_ready_time (const PendingChange* change, FavoriteVFSFileMonitorSource* fms);
static gboolean vfs_favorite_file_monitor_source_dispatch (GSource* source, GSourceFunc callback, gpointer udata);
static void g_file_monitor_source_remove_pending_change (FavoriteVFSFileMonitorSource* fms, GSequenceIter* iter, const gchar* child);
FavoriteVFSFileMonitor* vfs_favorite_file_monitor_new_for_path(const char *path, gboolean isDirectory, GFileMonitorFlags flags, GError **);
static void vfs_favorite_file_monitor_source_queue_event (FavoriteVFSFileMonitorSource* fms, GFileMonitorEvent event_type, const gchar* child, GFile* other);
static void vfs_favorite_file_monitor_start (FavoriteVFSFileMonitor* monitor, const gchar* filename, gboolean isDir, GFileMonitorFlags flags, GMainContext* ctx);

static guint vfs_favorite_file_monitor_signals [VFS_FAVORITE_FILE_MONITOR_LAST] = {0};

static void vfs_favorite_file_monitor_init (FavoriteVFSFileMonitor* self)
{
    g_return_if_fail(VFS_IS_FAVORITE_FILE_MONITOR(self));

    self->fileList = nullptr;
}

static void vfs_favorite_file_monitor_class_init (FavoriteVFSFileMonitorClass* self)
{
    GObjectClass* selfClass = G_OBJECT_CLASS (self);

    selfClass->dispose = vfs_favorite_file_monitor_dispose;
    selfClass->finalize = vfs_favorite_file_monitor_finalize;
}

static void vfs_favorite_file_monitor_dispose (GObject* obj)
{
    g_return_if_fail(VFS_IS_FAVORITE_FILE_MONITOR(obj));
    FavoriteVFSFileMonitor* self = VFS_FAVORITE_FILE_MONITOR(obj);

    if (nullptr != self->fileList) {
        g_list_free_full(self->fileList, g_object_unref);
        self->fileList = nullptr;
    }

    QObject::disconnect(self->add);
    QObject::disconnect(self->remove);
}

static void vfs_favorite_file_monitor_finalize (GObject* obj)
{
    g_return_if_fail(VFS_IS_FAVORITE_FILE_MONITOR(obj));
    G_OBJECT_CLASS (vfs_favorite_file_monitor_parent_class)->finalize (obj);
}

static FavoriteVFSFileMonitor* vfs_favorite_file_monitor_new ()
{
    return VFS_FAVORITE_FILE_MONITOR(g_object_new (VFS_TYPE_FAVORITE_FILE_MONITOR, nullptr));
}

gboolean vfs_favorite_file_monitor_source_handle_event(FavoriteVFSFileMonitorSource *, GFileMonitorEvent, const gchar *child, const gchar *renameTo, GFile *other, gint64 eTime)
{
    return FALSE;
}

FavoriteVFSFileMonitor* vfs_favorite_file_monitor_new_for_path(const char *path, gboolean isDirectory, GFileMonitorFlags flags, GError** error)
{
    FavoriteVFSFileMonitor* monitor = nullptr;

    monitor = vfs_favorite_file_monitor_new ();

    if (monitor) {
        vfs_favorite_file_monitor_start (monitor, path, isDirectory, flags, (GMainContext*)*error);
    }

    return monitor;
}

static void vfs_favorite_file_monitor_start (FavoriteVFSFileMonitor* monitor, const gchar* filename, gboolean isDir, GFileMonitorFlags flags, GMainContext* ctx)
{
    g_return_if_fail(VFS_IS_FAVORITE_FILE_MONITOR(monitor));

    FavoriteVFSFileMonitorClass* classs = VFS_FAVORITE_FILE_MONITOR_GET_CLASS (monitor);
    FavoriteVFSFileMonitorSource* source = nullptr;

    g_assert (!monitor->source);

    Peony::BookMarkManager* gm = Peony::BookMarkManager::getInstance();

    gm->connect(gm, &Peony::BookMarkManager::bookMarkAdded, [=] (const QString &uri, bool successed) {
        if (successed) {
            GFile* file = g_file_new_for_uri(uri.toUtf8().constData());
            g_file_monitor_emit_event(&(monitor->parent_instance), file, nullptr, G_FILE_MONITOR_EVENT_CREATED);
        }
    });
}

static FavoriteVFSFileMonitorSource* vfs_favorite_file_monitor_source_new (gpointer instance, const gchar* filename, GFileMonitorFlags flags)
{
    FavoriteVFSFileMonitorSource*   fms = nullptr;
    GSource*                        source = nullptr;
    static GSourceFuncs             sourceFuncs = {nullptr, nullptr, vfs_favorite_file_monitor_source_dispatch, vfs_favorite_file_monitor_source_finalize};

    source = g_source_new (&sourceFuncs, sizeof (FavoriteVFSFileMonitorSource));
    fms = (FavoriteVFSFileMonitorSource*) source;

    g_source_set_name (source, "FavoriteVFSFileMonitorSource");

    g_mutex_init (&fms->lock);
    g_weak_ref_init (&fms->instanceRef, instance);
    fms->pendingChanges = g_sequence_new (pending_change_free);
    fms->pendingChangesTable = g_hash_table_new (str_hash0, str_equal0);
    fms->rateLimit = DEFAULT_RATE_LIMIT;
    fms->flags = flags;

    if (flags & G_FILE_MONITOR_WATCH_HARD_LINKS) {
        fms->dirname = NULL;
        fms->basename = NULL;
        fms->filename = g_strdup (filename);
    } else {
        fms->dirname = g_path_get_dirname (filename);
        fms->basename = g_path_get_basename (filename);
        fms->filename = NULL;
    }

    return fms;
}

static gboolean vfs_favorite_file_monitor_source_dispatch (GSource* source, GSourceFunc callback, gpointer udata)
{
    gint64                          now = 0;
    GQueue                          eventQueue;
    QueuedEvent*                    event = nullptr;
    GFileMonitor*                   instance = nullptr;
    FavoriteVFSFileMonitorSource*   vms = (FavoriteVFSFileMonitorSource*) source;

    instance = (GFileMonitor*) g_weak_ref_get (&vms->instanceRef);
    if (nullptr == instance) {
        return FALSE;
    }

    now = g_source_get_time (source);
    g_mutex_lock (&vms->lock);

    while (!g_sequence_is_empty (vms->pendingChanges)) {
        GSequenceIter* iter = g_sequence_get_begin_iter (vms->pendingChanges);
        PendingChange* pending = (PendingChange*) g_sequence_get (iter);

        if (pending_change_get_ready_time (pending, vms) > now) {
            break;
        }
        if (pending->dirty) {
            vfs_favorite_file_monitor_source_queue_event (vms, G_FILE_MONITOR_EVENT_CHANGED, pending->child, NULL);
            pending->lastEmission = now;
            pending->dirty = FALSE;
            g_sequence_sort_changed (iter, pending_change_compare_ready_time, vms);
        } else {
            vfs_favorite_file_monitor_source_queue_event (vms, G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT, pending->child, NULL);
            g_file_monitor_source_remove_pending_change (vms, iter, pending->child);
        }
    }

    memcpy (&eventQueue, &vms->eventQueue, sizeof eventQueue);
    memset (&vms->eventQueue, 0, sizeof vms->eventQueue);

    g_file_monitor_source_update_ready_time (vms);

    g_clear_object (&instance);
    g_mutex_unlock (&vms->lock);


    while ((event = (QueuedEvent*) g_queue_pop_head (&eventQueue))) {
        instance = (GFileMonitor*) g_weak_ref_get (&vms->instanceRef);
        if (instance != NULL) {
            g_file_monitor_emit_event (instance, event->child, event->other, event->eventType);
        }

        g_clear_object (&instance);
        queued_event_free (event);
    }

    return TRUE;
}

static void vfs_favorite_file_monitor_source_finalize (GSource *source)
{
    FavoriteVFSFileMonitorSource* fms = (FavoriteVFSFileMonitorSource*) source;

    g_assert (nullptr == g_weak_ref_get (&fms->instanceRef));
    g_weak_ref_clear (&fms->instanceRef);

    g_assert (g_sequence_is_empty (fms->pendingChanges));
    g_assert (g_hash_table_size (fms->pendingChangesTable) == 0);
    g_assert (fms->eventQueue.length == 0);

    g_hash_table_unref (fms->pendingChangesTable);
    g_sequence_free (fms->pendingChanges);

    g_free (fms->dirname);
    g_free (fms->basename);
    g_free (fms->filename);

    g_mutex_clear (&fms->lock);
}

static gint64 pending_change_get_ready_time (const PendingChange* change, FavoriteVFSFileMonitorSource* fms)
{
    if (change->dirty) {
        return change->lastEmission + fms->rateLimit;
    } else {
        return change->lastEmission + VIRTUAL_CHANGES_DONE_DELAY;
    }
}

static void vfs_favorite_file_monitor_source_queue_event (FavoriteVFSFileMonitorSource* fms, GFileMonitorEvent event_type, const gchar* child, GFile* other)
{
    QueuedEvent *event;

    event = g_slice_new (QueuedEvent);
    event->eventType = event_type;
    event->other = other;

    if (other) {
        g_object_ref (other);
    }

    g_queue_push_tail (&fms->eventQueue, event);
}

static guint str_hash0 (gconstpointer str)
{
    return str ? g_str_hash (str) : 0;
}

static gboolean str_equal0 (gconstpointer a, gconstpointer b)
{
    return g_strcmp0 ((const char*)a, (const char*)b) == 0;
}

static void pending_change_free (gpointer data)
{
    PendingChange* change = (PendingChange*)data;

    g_free (change->child);

    g_slice_free (PendingChange, change);
}

static int pending_change_compare_ready_time (gconstpointer a_p, gconstpointer b_p, gpointer udata)
{
    FavoriteVFSFileMonitorSource *fms = (FavoriteVFSFileMonitorSource*)udata;
    const PendingChange *a = (PendingChange*)a_p;
    const PendingChange *b = (PendingChange*)b_p;
    gint64 ready_time_a;
    gint64 ready_time_b;

    ready_time_a = pending_change_get_ready_time (a, fms);
    ready_time_b = pending_change_get_ready_time (b, fms);

    if (ready_time_a < ready_time_b) {
        return -1;
    } else {
        return ready_time_a > ready_time_b;
    }
}

static void g_file_monitor_source_remove_pending_change (FavoriteVFSFileMonitorSource* fms, GSequenceIter* iter, const gchar* child)
{
    g_hash_table_remove (fms->pendingChangesTable, child);
    g_sequence_remove (iter);
}

static void g_file_monitor_source_update_ready_time (FavoriteVFSFileMonitorSource* fms)
{
    g_source_set_ready_time ((GSource*) fms, g_file_monitor_source_get_ready_time (fms));
}

static gint64 g_file_monitor_source_get_ready_time (FavoriteVFSFileMonitorSource* fms)
{
    GSequenceIter *iter;

    if (fms->eventQueue.length) {
        return 0;
    }

    iter = g_sequence_get_begin_iter (fms->pendingChanges);
    if (g_sequence_iter_is_end (iter)) {
        return -1;
    }

    return pending_change_get_ready_time ((const PendingChange*)g_sequence_get (iter), fms);
}

static void queued_event_free (QueuedEvent *event)
{
    if (event->child) {
        g_object_unref (event->child);
    }

    if (event->other) {
        g_object_unref (event->other);
    }

    g_slice_free (QueuedEvent, event);
}

void vfs_favorite_file_monitor_free_gfile(FavoriteVFSFileMonitor* obj, GFile* file)
{
    FavoriteVFSFileMonitor* self = VFS_FAVORITE_FILE_MONITOR(obj);

    if (nullptr != file && G_IS_FILE(file)) {
        self->fileList = g_list_append(self->fileList, file);
    }
}

void vfs_favorite_file_monitor_dir(FavoriteVFSFileMonitor *obj)
{
    g_return_if_fail(VFS_IS_FAVORITE_FILE_MONITOR(obj));
    Peony::BookMarkManager* gm = Peony::BookMarkManager::getInstance();
    obj->add = QObject::connect(gm, &Peony::BookMarkManager::bookMarkAdded, [=] (const QString &uri, bool successed) {
        if (successed) {
            GFile* file = g_file_new_for_uri(uri.toUtf8().constData());
            g_file_monitor_emit_event(G_FILE_MONITOR(obj), file, nullptr, G_FILE_MONITOR_EVENT_CREATED);
            vfs_favorite_file_monitor_free_gfile (VFS_FAVORITE_FILE_MONITOR(obj), G_FILE(file));
        }
    });

    obj->remove = QObject::connect(gm, &Peony::BookMarkManager::bookMarkRemoved, [=] (const QString &uri, bool successed) {
        if (successed) {
            GFile* file = g_file_new_for_uri(uri.toUtf8().constData());
            g_file_monitor_emit_event(G_FILE_MONITOR(obj), file, nullptr, G_FILE_MONITOR_EVENT_DELETED);
            vfs_favorite_file_monitor_free_gfile (VFS_FAVORITE_FILE_MONITOR(obj), G_FILE(file));
        }
    });
}
