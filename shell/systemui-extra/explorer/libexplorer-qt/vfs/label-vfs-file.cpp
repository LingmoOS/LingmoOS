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
#include <QDebug>

#include "label-vfs-file.h"
#include "label-vfs-file-enumerator.h"
#include "label-vfs-file-monitor.h"
#include "file-label-model.h"
#include "file-utils.h"


static void vfs_label_file_g_file_iface_init(GFileIface *iface);
static void label_vfs_file_enumerator_parse_uri(LabelVFSFileEnumerator *enumerator, const QString& uri);

GFile*              vfs_label_file_dup(GFile *file);
char*               vfs_label_file_get_uri(GFile *file);
char*               vfs_label_file_get_path(GFile *file);
gboolean            vfs_label_file_is_native(GFile *file);
GFile*              vfs_label_file_get_parent(GFile *file);
char*               vfs_label_file_get_schema (GFile* file);
void                vfs_label_file_dispose(GObject *object);
char*               vfs_label_file_get_basename (GFile* file);
gboolean            vfs_label_file_is_equal(GFile *file1, GFile *file2);
char*               vfs_label_file_get_relative_path (GFile* parent, GFile* descendant);
GFile*              vfs_label_file_resolve_relative_path(GFile *file, const char *relative_path);
GFileInputStream*   vfs_label_file_read_fn(GFile* file, GCancellable* cancellable, GError** error);
gboolean            vfs_label_file_delete (GFile* file, GCancellable* cancellable, GError** error);
GFileIOStream *     vfs_label_file_open_readwrite(GFile* file, GCancellable* cancellable, GError** error);
GMount *            vfs_label_file_find_enclosing_mount(GFile* file, GCancellable* cancellable, GError** error);
gboolean            vfs_label_file_make_symbolic_link(GFile* file, const char* svalue, GCancellable* cancellable, GError** error);
GFileMonitor*       vfs_label_file_monitor_directory (GFile* file, GFileMonitorFlags flags, GCancellable* cancellable, GError** error);
GFile*              vfs_label_file_set_display_name (GFile* file, const gchar* display_name, GCancellable* cancellable, GError** error);
GFileInfo*          vfs_label_file_query_filesystem_info(GFile* file, const char* attributes, GCancellable* cancellable, GError** error);
GFileInfo*          vfs_label_file_query_info(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
GFileEnumerator*    vfs_label_file_enumerate_children(GFile *file, const char *attribute, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
GFileOutputStream*  vfs_label_file_replace(GFile* file, const char* etag, gboolean make_backup, GFileCreateFlags flags, GCancellable* cancellable, GError** error);
GFileEnumerator*    vfs_label_file_enumerate_children_internal(GFile *file, const char *attribute, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
gboolean            vfs_label_file_move(GFile* source, GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress, GError** error);

G_DEFINE_TYPE_EXTENDED(LabelVFSFile, vfs_label_file, G_TYPE_OBJECT, 0, G_ADD_PRIVATE(LabelVFSFile) G_IMPLEMENT_INTERFACE(G_TYPE_FILE, vfs_label_file_g_file_iface_init));

static void vfs_label_file_init(LabelVFSFile* self)
{
    g_return_if_fail(VFS_IS_LABEL_FILE(self));
    LabelVFSFilePrivate *priv = (LabelVFSFilePrivate*)vfs_label_file_get_instance_private(self);
    self->priv = priv;

    self->priv->uri = nullptr;
}

static void vfs_label_file_class_init (LabelVFSFileClass* kclass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS (kclass);

    gobject_class->dispose = vfs_label_file_dispose;
}

static void vfs_label_file_g_file_iface_init(GFileIface *iface)
{
    iface->dup                      = vfs_label_file_dup;
    iface->move                     = vfs_label_file_move;
    iface->trash                    = vfs_label_file_delete;
    iface->equal                    = vfs_label_file_is_equal;
    iface->get_uri                  = vfs_label_file_get_uri;
    iface->get_path                 = vfs_label_file_get_path;
    iface->is_native                = vfs_label_file_is_native;
    iface->get_parent               = vfs_label_file_get_parent;
    iface->query_info               = vfs_label_file_query_info;
    iface->delete_file              = vfs_label_file_delete;
    iface->get_uri_scheme           = vfs_label_file_get_schema;
    iface->get_basename             = vfs_label_file_get_basename;
    iface->open_readwrite           = vfs_label_file_open_readwrite;
    iface->set_display_name         = vfs_label_file_set_display_name;
    iface->get_relative_path        = vfs_label_file_get_relative_path;
    iface->monitor_dir              = vfs_label_file_monitor_directory;
    iface->make_symbolic_link       = vfs_label_file_make_symbolic_link;
    iface->enumerate_children       = vfs_label_file_enumerate_children;
    iface->find_enclosing_mount     = vfs_label_file_find_enclosing_mount;
    iface->query_filesystem_info    = vfs_label_file_query_filesystem_info;
    iface->resolve_relative_path    = vfs_label_file_resolve_relative_path;

}

GFile *vfs_label_file_new_for_uri(const char *uri)
{
    auto vfs_label_file = VFS_LABEL_FILE(g_object_new(VFS_TYPE_LABEL_FILE, nullptr));
    vfs_label_file->priv->uri = g_strdup(uri);

    return G_FILE(vfs_label_file);
}

GFile* vfs_label_file_dup(GFile *file){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), g_file_new_for_uri("label:///"));
    auto vfs_file = VFS_LABEL_FILE(file);
    auto dup = VFS_LABEL_FILE(g_object_new(VFS_TYPE_LABEL_FILE, nullptr));
    dup->priv->uri = g_strdup(vfs_file->priv->uri);

    return G_FILE(dup);
}

char* vfs_label_file_get_uri(GFile *file){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), g_strdup("label:///"));

    auto vfsfile = VFS_LABEL_FILE (file);

    return g_strdup(Peony::FileUtils::urlDecode(vfsfile->priv->uri).toUtf8().constData());
}

char* vfs_label_file_get_path(GFile *file){
    return nullptr;
}

gboolean vfs_label_file_is_native(GFile *file){
    Q_UNUSED(file);
    return FALSE;
}

GFile* vfs_label_file_get_parent(GFile *file){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), nullptr);

    Q_UNUSED(file);

    QString uri = g_file_get_uri(file);

    if ("label:///" == uri) {
        return nullptr;
    }

    return g_file_new_for_uri("label:///");
}

char* vfs_label_file_get_schema (GFile* file){
    Q_UNUSED(file);
    return g_strdup("label");
}

void vfs_label_file_dispose(GObject *object){
    g_return_if_fail(VFS_IS_LABEL_FILE(object));

    auto vfsfile = VFS_LABEL_FILE (object);

    if (G_IS_FILE_MONITOR(vfsfile->priv->fileMonitor)) {
        g_file_monitor_cancel (vfsfile->priv->fileMonitor);
        vfsfile->priv->fileMonitor = nullptr;
    }

    if (vfsfile->priv->uri) {
        g_free(vfsfile->priv->uri);
        vfsfile->priv->uri = nullptr;
    }
}

char* vfs_label_file_get_basename (GFile* file){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), nullptr);

    char*       uri = vfs_label_file_get_uri (file);
    QString     baseName = nullptr;

    if (0 == strcmp("label:///", uri)) {
        return g_strdup("label:///");
    } else {
        QString url = uri;
        QString baseNamet = url.split("/").takeLast();
        if (baseNamet.contains("?schema=") && baseNamet.split("?schema=").size() >= 2) {
            baseName = baseNamet.split("?schema=").first();
        }
    }
    if (nullptr != uri) g_free(uri);

    return g_strdup (baseName.toUtf8().constData());
}

gboolean vfs_label_file_is_equal(GFile *file1, GFile *file2){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file1) || VFS_IS_LABEL_FILE(file2), false);

    char* f1 = g_file_get_uri(file1);
    char* f2 = g_file_get_uri(file2);
    int ret = g_strcmp0(f1, f2);

    if (nullptr != f1) {
        g_free(f1);
    }

    if (nullptr != f2) {
        g_free(f2);
    }

    return ret == 0;
}

char* vfs_label_file_get_relative_path (GFile* parent, GFile* descendant){
    return nullptr;
}

GFile* vfs_label_file_resolve_relative_path(GFile *file, const char *relativePath){
    QString tmp = relativePath;
    if (tmp.contains('/')) {
        if (relativePath) {
            return g_file_new_for_uri(relativePath);
        }
    } else if (!tmp.isEmpty()) {
        QString uri = "label:///" + tmp;
        return vfs_label_file_new_for_uri(uri.toUtf8().constData());
    }

    return g_file_new_for_uri("label:///");;
}

GFileInputStream* vfs_label_file_read_fn(GFile* file, GCancellable* cancellable, GError** error){
    QString str = QObject::tr("The virtual file system cannot be opened");
    *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_NOT_SUPPORTED, "%s\n", str.toUtf8().constData());
    return nullptr;
}

gboolean vfs_label_file_delete (GFile* file, GCancellable* cancellable, GError** error){
    QString uri = nullptr;
    GFileIface *iface = nullptr;

    g_return_val_if_fail (VFS_IS_LABEL_FILE(file), FALSE);

    if (g_cancellable_set_error_if_cancelled (cancellable, error)) {
        return FALSE;
    }

    iface = G_FILE_GET_IFACE (file);

    uri = g_file_get_uri (file);

    QString errorStr = QObject::tr("Operation not supported");

    if (iface->delete_file == NULL) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED, errorStr.toUtf8().constData());
        return FALSE;
    }

    /* 删除文件标记 */
    QUrl url(uri);
    QString realUri = QString("file:///").append(url.path().section("/", 2,-1));
    QString encodeUri = Peony::FileUtils::getEncodedUri(realUri);
    int labelId = FileLabelModel::getGlobalModel()->getLabelIdFromLabelName(url.path().section("/", 1, 1));
    FileLabelModel::getGlobalModel()->removeFileLabel(encodeUri, labelId);
    return TRUE;
}

GFileIOStream* vfs_label_file_open_readwrite(GFile* file, GCancellable* cancellable, GError** error){
    QString str = QObject::tr("The virtual file system cannot be opened");
    *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_NOT_SUPPORTED, "%s\n", str.toUtf8().constData());
    return nullptr;
}

GMount* vfs_label_file_find_enclosing_mount(GFile* file, GCancellable* cancellable, GError** error){
    return nullptr;
}

gboolean vfs_label_file_make_symbolic_link(GFile* file, const char* svalue, GCancellable* cancellable, GError** error){
    Q_UNUSED(file);
    Q_UNUSED(error);
    Q_UNUSED(svalue);
    Q_UNUSED(cancellable);

    // fixme:// Do not implement
    QString str = QObject::tr("Virtual file directories do not support move operations");
    *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_NOT_SUPPORTED, "%s\n", str.toUtf8().constData());
    return FALSE;
}

GFileMonitor* vfs_label_file_monitor_directory (GFile* file, GFileMonitorFlags flags, GCancellable* cancellable, GError** error){

    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), nullptr);

    LabelVFSFilePrivate* priv = VFS_LABEL_FILE((LabelVFSFile*)file)->priv;
    priv->fileMonitor = (GFileMonitor*)g_object_new (VFS_TYPE_LABEL_FILE_MONITOR, nullptr);
    QString uri = g_file_get_uri(file);
    vfs_label_file_monitor_dir(VFS_LABEL_FILE_MONITOR(priv->fileMonitor), uri);


    Q_UNUSED(file)
    Q_UNUSED(flags)
    Q_UNUSED(error)
    Q_UNUSED(cancellable)

    return priv->fileMonitor;
}

GFile* vfs_label_file_set_display_name (GFile* file, const gchar* display_name, GCancellable* cancellable, GError** error){
    Q_UNUSED(error);
    Q_UNUSED(cancellable);
    Q_UNUSED(display_name);

    return file;
}

GFileInfo* vfs_label_file_query_filesystem_info(GFile* file, const char* attributes, GCancellable* cancellable, GError** error){
    return vfs_label_file_query_info(file, attributes, G_FILE_QUERY_INFO_NONE, cancellable, error);
}

GFileInfo* vfs_label_file_query_info(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), nullptr);

    auto vfsfile = VFS_LABEL_FILE(file);

    GFileInfo* info = nullptr;
    QString trueUri = nullptr;
    QUrl url(vfs_label_file_get_uri(file));

    QString uriStr = url.toString();
    if (uriStr.startsWith("label:///") && uriStr.contains("?schema=")) {
        QStringList querys = url.query().split("&");
        for (int i = 0; i < querys.count(); ++i) {
            if (querys.at(i).contains("schema=")) {
                QString scheam = querys.at(i).split("=").last();
                if ("label" == scheam) {
                    trueUri = QString("%1://%2").arg("file").arg(url.path());
                } else {
                    trueUri = QString("%1:///%2").arg(scheam).arg(url.path().section("/",2,-1));
                }
                break;
            }
        }
    } else {
        info = g_file_info_new ();
        QString name;
        if("label:///"== url.toString()){
            name = QObject::tr("label");
            auto icon = g_themed_icon_new("label");
            g_file_info_set_icon(info, icon);
            g_object_unref(icon);
        }else{
            name = url.toString().section("/", -1,-1);
        }

        g_file_info_set_size(info, 0);
        g_file_info_set_is_hidden(info, FALSE);
        g_file_info_set_is_symlink(info, FALSE);
        g_file_info_set_file_type(info, G_FILE_TYPE_DIRECTORY);
        g_file_info_set_display_name(info, name.toUtf8().constData());

        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_STANDARD_IS_VIRTUAL, TRUE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE, TRUE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH, FALSE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE, TRUE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME, FALSE);

        return info;
    }

    if (url.path() == "/data/usershare") {
        trueUri = "file:///data/usershare";
    }

    if (!trueUri.isNull()) {
        GFile* file = g_file_new_for_uri (trueUri.toUtf8().constData());
        if (nullptr != file) {
            info = g_file_query_info(file, attributes, flags, cancellable, error);
            if (!info) {
                return nullptr;
            }
            g_file_info_set_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, trueUri.toUtf8().constData());
            qDebug()<<attributes;
        } else {
            g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, QObject::tr("File is not existed.").toUtf8().constData());
            return nullptr;
        }
        g_object_unref(file);
    } else {
        g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND, QObject::tr("File is not existed.").toUtf8().constData());
        return nullptr;
    }

    g_file_info_set_name (info, vfsfile->priv->uri);

    if (url.path() == "/data/usershare") {
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE, FALSE);
        g_file_info_set_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, QObject::tr("Share Data").toUtf8());
    }
    if (trueUri == "trash:///") {
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE, FALSE);
        g_file_info_set_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, QObject::tr("Trash").toUtf8());
    }
    if (trueUri == "recent:///") {
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE, FALSE);
        g_file_info_set_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, QObject::tr("Recent").toUtf8());
    }

    return info;
}

GFileEnumerator* vfs_label_file_enumerate_children(GFile *file, const char *attribute, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), nullptr);

    return vfs_label_file_enumerate_children_internal(file, attribute, flags, cancellable, error);
}

GFileOutputStream*  vfs_label_file_replace(GFile* file, const char* etag, gboolean make_backup, GFileCreateFlags flags, GCancellable* cancellable, GError** error){
    QString str = QObject::tr("Virtual file directories do not support move operations");
    *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_NOT_SUPPORTED, "%s\n", str.toUtf8().constData());
    return FALSE;
}

GFileEnumerator* vfs_label_file_enumerate_children_internal(GFile *file, const char *attribute, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error){
    g_return_val_if_fail(VFS_IS_LABEL_FILE(file), nullptr);
    Q_UNUSED(flags)
    Q_UNUSED(error)
    Q_UNUSED(attribute)
    Q_UNUSED(cancellable)

    QString uri = g_file_get_uri(file);
    if (uri.startsWith("label://")) {
        auto enumerator = VFS_LABEL_FILE_ENUMERATOR(g_object_new(VFS_TYPE_LABEL_FILE_ENUMERATOR, "container", file, nullptr));
        label_vfs_file_enumerator_parse_uri(enumerator, uri);
        return G_FILE_ENUMERATOR(enumerator);
    }

    return nullptr;
}

gboolean vfs_label_file_move(GFile* source, GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress, GError** error){
    return false;
}

gboolean vfs_label_file_is_exist(const char *uri)
{
    gboolean        ret = FALSE;
    GFile*          file = g_file_new_for_uri(uri);
    GFileInfo*      fileInfo = nullptr;

    if (VFS_IS_LABEL_FILE(file)) {
        auto vfsFile = VFS_LABEL_FILE(file);
        qDebug()<<vfsFile->priv->uri;
    }

    if (nullptr != file) {
        fileInfo = g_file_query_info(file, "*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
        if (nullptr != fileInfo) {
            gchar* targetUri = g_file_info_get_attribute_as_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
            ret = Peony::FileUtils::isFileExsit(targetUri);
            g_free(targetUri);
            g_object_unref(fileInfo);
        }
        g_object_unref(file);
    }

    return ret;
}

void label_vfs_file_enumerator_parse_uri(LabelVFSFileEnumerator *enumerator, const QString& uri){
    LabelVFSFileEnumeratorPrivate *priv = enumerator->priv;
    *priv->label_vfs_directory_uri = uri;
    auto tmp = priv->label_vfs_directory_uri->section("/", -1,-1);
    int labelId = FileLabelModel::getGlobalModel()->getLabelIdFromLabelName(tmp);
    QSet<QString> uriSet = FileLabelModel::getGlobalModel()->getFileUrisFromLabelId(labelId);
    for(const QString &str: uriSet){
        QUrl url = QUrl(str);
        QString uri1 = uri + url.path() + "?schema=" + url.scheme();
        if (vfs_label_file_is_exist(uri1.toUtf8())) {
            priv->enumerate_queue->enqueue(uri1);
        }
    }
}
