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
 * Authors: Ling Yang <yangling@kylinos.cn>
 *
 */

#include <QUrl>

#include "label-vfs-file-enumerator.h"
#include "file-label-model.h"
#include "label-vfs-file.h"
#include "file-utils.h"


G_DEFINE_TYPE_WITH_PRIVATE(LabelVFSFileEnumerator, vfs_label_file_enumerator, G_TYPE_FILE_ENUMERATOR);

static void next_async_op_free(GList *files);
void vfs_label_file_enumerator_dispose(GObject *object);
static gboolean enumerator_close(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error);
static GFileInfo *enumerate_next_file(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error);
static GList* vfs_label_file_enumerator_next_files_finished(GFileEnumerator* enumerator, GAsyncResult* result, GError** error);
static void next_files_thread(GTask* task, gpointer source_object, gpointer task_data, GCancellable *cancellable);
static void vfs_label_file_enumerator_next_files_async(GFileEnumerator* enumerator, int num_files, int io_priority, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data);

static void vfs_label_file_enumerator_init(LabelVFSFileEnumerator* self){
    g_return_if_fail(VFS_IS_LABEL_FILE_ENUMERATOR(self));
    LabelVFSFileEnumeratorPrivate* priv = (LabelVFSFileEnumeratorPrivate*) vfs_label_file_enumerator_get_instance_private(self);

    self->priv = priv;
    self->priv->label_vfs_directory_uri = new QString;
    self->priv->enumerate_queue = new QQueue<QString>;    
}

static void vfs_label_file_enumerator_class_init(LabelVFSFileEnumeratorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GFileEnumeratorClass *enumerator_class = G_FILE_ENUMERATOR_CLASS(klass);

    gobject_class->dispose = vfs_label_file_enumerator_dispose;
    enumerator_class->next_file = enumerate_next_file;
    enumerator_class->close_fn = enumerator_close;

    enumerator_class->next_files_async = vfs_label_file_enumerator_next_files_async;
    enumerator_class->next_files_finish = vfs_label_file_enumerator_next_files_finished;
}

static void next_async_op_free(GList *files){
    if (files) {
        g_list_free_full (files, g_object_unref);
    }
}

void vfs_label_file_enumerator_dispose(GObject *object){
    LabelVFSFileEnumerator *self = VFS_LABEL_FILE_ENUMERATOR(object);
    delete self->priv->label_vfs_directory_uri;
    self->priv->enumerate_queue->clear();
    delete self->priv->enumerate_queue;
}

static gboolean enumerator_close(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error){
    LabelVFSFileEnumerator *self = VFS_LABEL_FILE_ENUMERATOR(enumerator);
    return true;
}
static GFileInfo *enumerate_next_file(GFileEnumerator *enumerator, GCancellable *cancellable, GError **error){
    g_return_val_if_fail(VFS_IS_LABEL_FILE_ENUMERATOR(enumerator), nullptr);
    if (cancellable && g_cancellable_is_cancelled(cancellable)) {
        *error = g_error_new_literal(G_IO_ERROR, G_IO_ERROR_CANCELLED, "cancelled");
        return nullptr;
    }

    GFileInfo* fileInfo = nullptr;

    auto ve = VFS_LABEL_FILE_ENUMERATOR(enumerator);
    auto eq = ve->priv->enumerate_queue;

    if (!eq->isEmpty()) {
        GFile* file = g_file_new_for_uri(eq->dequeue().toUtf8());
        if (nullptr != file) {
            fileInfo = g_file_query_info(file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
            g_object_unref(file);
        }
    }

    return fileInfo;
}

static GList* vfs_label_file_enumerator_next_files_finished(GFileEnumerator* enumerator, GAsyncResult* result, GError** error){
    g_return_val_if_fail (g_task_is_valid(result, enumerator), NULL);

    return (GList*)g_task_propagate_pointer(G_TASK (result), error);
}

static void next_files_thread(GTask* task, gpointer source_object, gpointer task_data, GCancellable *cancellable){
    auto enumerator = G_FILE_ENUMERATOR(source_object);
    int num_files = GPOINTER_TO_INT (task_data);

    int i = 0;
    GList *files = nullptr;
    GError *error = nullptr;
    GFileInfo *info = nullptr;
    GFileEnumeratorClass* c = G_FILE_ENUMERATOR_GET_CLASS (enumerator);

    for (i = 0; i < num_files; ++i) {
        if (g_cancellable_set_error_if_cancelled (cancellable, &error)) {
            info = NULL;
        } else {
            info = c->next_file (enumerator, cancellable, &error);
        }

        if (nullptr == info) {
            break;
        } else {
            files = g_list_prepend (files, info);
        }
    }

    if (error) {
        g_task_return_error (task, error);
    } else {
        g_task_return_pointer (task, files, (GDestroyNotify) next_async_op_free);
    }
}


static void vfs_label_file_enumerator_next_files_async(GFileEnumerator* enumerator, int num_files, int io_priority, GCancellable* cancellable, GAsyncReadyCallback callback, gpointer user_data){
    GTask* task = g_task_new (enumerator, cancellable, callback, user_data);
    g_task_set_source_tag (task, (gpointer) vfs_label_file_enumerator_next_files_async);
    g_task_set_task_data (task, GINT_TO_POINTER (num_files), NULL);
    g_task_set_priority (task, io_priority);

    g_task_run_in_thread (task, next_files_thread);

    if (task) {
        g_object_unref (task);
    }
}
