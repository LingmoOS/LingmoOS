/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 */

#include "custom-vfs-file.h"
#include "file-utils.h"
#include "custom-vfs-file-enumerator.h"
#include "custom-vfs-file-monitor.h"
#include "vfs-info-plugin-manager.h"
#include "vfs-info-plugin-iface.h"

#include <QDateTime>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

static void vfs_custom_file_g_file_iface_init(GFileIface *iface);

GFile*              vfs_custom_file_dup(GFile *file);
char*               vfs_custom_file_get_uri(GFile *file);
char*               vfs_custom_file_get_path(GFile *file);
gboolean            vfs_custom_file_is_native(GFile *file);
GFile*              vfs_custom_file_get_parent(GFile *file);
char*               vfs_custom_file_get_schema (GFile* file);
void                vfs_custom_file_dispose(GObject *object);
char*               vfs_custom_file_get_basename (GFile* file);
GFile*              vfs_custom_file_new_for_uri(const char *uri);
gboolean            vfs_custom_file_is_equal(GFile *file1, GFile *file2);
char*               vfs_custom_file_get_relative_path (GFile* parent, GFile* descendant);
GFile*              vfs_custom_file_resolve_relative_path(GFile *file, const char *relative_path);
GFileInputStream*   vfs_custom_file_read_fn(GFile* file, GCancellable* cancellable, GError** error);
gboolean            vfs_custom_file_delete (GFile* file, GCancellable* cancellable, GError** error);
GFileIOStream *     vfs_custom_file_open_readwrite(GFile* file, GCancellable* cancellable, GError** error);
gboolean            vfs_custom_file_make_directory(GFile* file, GCancellable* cancellable, GError** error);
GMount *            vfs_custom_file_find_enclosing_mount(GFile* file, GCancellable* cancellable, GError** error);
GFileOutputStream*  vfs_custom_file_create(GFile* file, GFileCreateFlags flags, GCancellable* cancellable, GError** error);
gboolean            vfs_custom_file_make_symbolic_link(GFile* file, const char* svalue, GCancellable* cancellable, GError** error);
GFileMonitor*       vfs_custom_file_monitor_directory (GFile* file, GFileMonitorFlags flags, GCancellable* cancellable, GError** error);
GFileMonitor*       vfs_custom_file_monitor_file (GFile* file, GFileMonitorFlags flags, GCancellable* cancellable, GError** error);
GFile*              vfs_custom_file_set_display_name (GFile* file, const gchar* display_name, GCancellable* cancellable, GError** error);
GFileInfo*          vfs_custom_file_query_filesystem_info(GFile* file, const char* attributes, GCancellable* cancellable, GError** error);
GFileInfo*          vfs_custom_file_query_info(GFile *file, const char *attributes, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
GFileEnumerator*    vfs_custom_file_enumerate_children(GFile *file, const char *attribute, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
GFileOutputStream*  vfs_custom_file_replace(GFile* file, const char* etag, gboolean make_backup, GFileCreateFlags flags, GCancellable* cancellable, GError** error);
GFileEnumerator*    vfs_custom_file_enumerate_children_internal(GFile *file, const char *attribute, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error);
gboolean            vfs_custom_file_copy(GFile* source, GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback pcallback, gpointer pcallbackdata, GError** error);
gboolean            vfs_custom_file_move(GFile* source, GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress, GError** error);

G_DEFINE_TYPE_EXTENDED(CustomVFSFile,
                       vfs_custom_file,
                       G_TYPE_OBJECT,
                       0,
                       G_ADD_PRIVATE(CustomVFSFile)
                       G_IMPLEMENT_INTERFACE(G_TYPE_FILE, vfs_custom_file_g_file_iface_init));

static void vfs_custom_file_init(CustomVFSFile *self) {
    CustomVFSFilePrivate *priv = (CustomVFSFilePrivate*)vfs_custom_file_get_instance_private(self);
    self->priv = priv;
    priv->uri = nullptr;
}

static void vfs_custom_file_class_init(CustomVFSFileClass *klass) {
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->dispose = vfs_custom_file_dispose;
}

void vfs_custom_file_dispose(GObject *object) {
    g_return_if_fail(VFS_IS_CUSTOM_VFS(object));

    auto vfsfile = VFS_CUSTOM_FILE(object);
    if (vfsfile->priv->uri) {
        g_free(vfsfile->priv->uri);
        vfsfile->priv->uri = nullptr;
    }
}

static void vfs_custom_file_g_file_iface_init(GFileIface *iface) {
    iface->dup                   = vfs_custom_file_dup;
    iface->move                  = vfs_custom_file_move;
    iface->copy                  = vfs_custom_file_copy;
    iface->trash                 = vfs_custom_file_delete;
    iface->equal                 = vfs_custom_file_is_equal;
    iface->create                = vfs_custom_file_create;
    iface->get_path              = vfs_custom_file_get_path;
    iface->get_uri               = vfs_custom_file_get_uri;
    iface->is_native             = vfs_custom_file_is_native;
    iface->query_info            = vfs_custom_file_query_info;
    iface->delete_file           = vfs_custom_file_delete;
    iface->get_parent            = vfs_custom_file_get_parent;
    iface->get_basename          = vfs_custom_file_get_basename;
    iface->set_display_name      = vfs_custom_file_set_display_name;
    iface->make_directory        = vfs_custom_file_make_directory;
    iface->get_relative_path     = vfs_custom_file_get_relative_path;
    iface->get_uri_scheme        = vfs_custom_file_get_schema;
    iface->enumerate_children    = vfs_custom_file_enumerate_children;
    iface->resolve_relative_path = vfs_custom_file_resolve_relative_path;
    iface->query_filesystem_info = vfs_custom_file_query_filesystem_info;
    iface->monitor_dir           = vfs_custom_file_monitor_directory;
    iface->monitor_file          = vfs_custom_file_monitor_file;
}

char* vfs_custom_file_get_uri(GFile *file) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), g_strdup("test:///"));

    auto vfsfile = VFS_CUSTOM_FILE(file);
    return g_strdup(vfsfile->priv->uri);
}

GFile *vfs_custom_file_new_for_uri(const char *uri)
{
    auto vfs_custom_file = VFS_CUSTOM_FILE(g_object_new(VFS_TYPE_CUSTOM_FILE, nullptr));
    vfs_custom_file->priv->uri = g_strdup(uri);
    return G_FILE(vfs_custom_file);
}

GFile *vfs_custom_file_dup(GFile *file) {
    if (!VFS_IS_CUSTOM_FILE(file)) {
        return g_file_new_for_uri("test:///");
    }
    auto vfs_file = VFS_CUSTOM_FILE(file);
    auto dup = VFS_CUSTOM_FILE(g_object_new(VFS_TYPE_CUSTOM_FILE, nullptr));
    dup->priv->uri = g_strdup(vfs_file->priv->uri);
    return G_FILE(dup);
}

gboolean vfs_custom_file_is_equal(GFile *file1, GFile *file2) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file1) || VFS_IS_CUSTOM_FILE(file2), false);

    char* f1 = g_file_get_uri(file1);
    char* f2 =  g_file_get_uri(file2);
    int ret = g_strcmp0(f1, f2);

    if (nullptr != f1) {
        g_free(f1);
    }

    if (nullptr != f2) {
        g_free(f2);
    }

    return ret == 0;
}

GFileInfo* vfs_custom_file_query_info(GFile *file,
        const char *attributes,
        GFileQueryInfoFlags flags,
        GCancellable *cancellable,
        GError **error) {
    Q_UNUSED(file);
    Q_UNUSED(attributes);
    Q_UNUSED(flags);
    Q_UNUSED(cancellable);
    Q_UNUSED(error);

    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);

    auto vfs_file = VFS_CUSTOM_FILE(file);
    GFileInfo *info = g_file_info_new();
    QString uri(vfs_file->priv->uri);
    QString scheme = uri.section(":", 0, -2) + "://";

    if (uri == scheme + "/") {
        scheme.chop(3);
        QString name = scheme;

        auto icon = g_themed_icon_new(name.toUtf8().constData());
        g_file_info_set_icon(info, icon);
        g_object_unref(icon);

        g_file_info_set_display_name(info, name.toUtf8().constData());
        g_file_info_set_size(info, 0);
        g_file_info_set_is_hidden(info, FALSE);
        g_file_info_set_is_symlink(info, FALSE);
        g_file_info_set_file_type(info, G_FILE_TYPE_DIRECTORY);

        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_STANDARD_IS_VIRTUAL, TRUE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE, TRUE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH, FALSE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE, TRUE);
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME, FALSE);
    } else {
        uri = Peony::FileUtils::urlDecode(uri);
        QStringList list = uri.remove(scheme).split('/');
        QString displayName = list.last();
        scheme.chop(3);

        Peony::VFSInfoPluginIface *iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(scheme);
        auto fileInfo = iface->queryFile(uri);
        if (!fileInfo) {
            qDebug() << __func__ << "fileInfo isNull";
            return nullptr;
        }

        g_file_info_set_size(info, fileInfo->size());
        g_file_info_set_display_name(info, displayName.toUtf8().constData());
        g_file_info_set_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE, TRUE);
        g_file_info_set_name(info, displayName.toUtf8().constData());

        QDateTime dateTime = QDateTime::fromString(fileInfo->modifiedTime(), Qt::ISODate);
        int year = dateTime.date().year();
        int month = dateTime.date().month();
        int day = dateTime.date().day();
        int hour = dateTime.time().hour();
        int sec = dateTime.time().second();
        int msec = dateTime.time().msec();
        GTimeZone *zone = g_time_zone_new_local();
        GDateTime *time = g_date_time_new(zone, year, month, day, hour, sec, msec);
        g_file_info_set_modification_date_time(info, time);
        g_time_zone_unref(zone);
        g_date_time_unref(time);

        guint64 atime = QDateTime::fromString(fileInfo->accessTime(), Qt::ISODate).toTime_t();
        g_file_info_set_attribute_uint64(info, G_FILE_ATTRIBUTE_TIME_ACCESS, atime);

        QMap<QString, QVariant> extendInfo = fileInfo->getAllExtendInfo();
        QMap<QString, QVariant>::iterator iter;
        for (iter = extendInfo.begin(); iter != extendInfo.end(); ++iter) {
            if (iter.key() == G_FILE_ATTRIBUTE_UNIX_MODE) {
                guint32 mode = iter.value().toUInt();
                g_file_info_set_attribute_uint32(info, G_FILE_ATTRIBUTE_UNIX_MODE, mode);
            } else if (iter.key() == G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE
                       || iter.key() == G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME
                       || iter.key() == G_FILE_ATTRIBUTE_STANDARD_IS_VIRTUAL) {
                gboolean value = iter.value().toBool();
                g_file_info_set_attribute_boolean(info, iter.key().toUtf8().constData(), value);
            } else if (iter.key() == G_FILE_ATTRIBUTE_OWNER_USER
                       || iter.key() == G_FILE_ATTRIBUTE_OWNER_GROUP) {
                QString str = iter.value().toString();
                g_file_info_set_attribute_string(info, iter.key().toUtf8().constData(), str.toUtf8().constData());
            }
        }

        GIcon* icon = nullptr;
        if (fileInfo->isDir()) {
            QString contentType = "inode/directory";
            QString iconName = "inode-directory";
            icon = g_themed_icon_new(iconName.toUtf8().constData());
            g_file_info_set_icon(info, icon);
            g_file_info_set_content_type(info, contentType.toUtf8().constData());
            g_file_info_set_file_type(info, G_FILE_TYPE_DIRECTORY);
            g_object_unref(icon);
            return info;
        }

        g_file_info_set_attribute_string(info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, fileInfo->targetUri().toUtf8().constData());

        icon = g_themed_icon_new(fileInfo->iconName().toUtf8().constData());
        g_file_info_set_content_type(info, fileInfo->contentType().toUtf8().constData());
        g_file_info_set_icon(info, icon);
        g_object_unref(icon);
        return info;
    }
    return info;
}

char* vfs_custom_file_get_path(GFile *file) {
    Q_UNUSED(file);
    return nullptr;
}

gboolean vfs_custom_file_is_native(GFile *file) {
    Q_UNUSED(file);
    return false;
}

GFile* vfs_custom_file_get_parent(GFile *file) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);

    auto vfs_file = VFS_CUSTOM_FILE(file);

    QString uri = g_file_get_uri(file);

    QString tUri = vfs_file->priv->uri;
    QString scheme = tUri.section(":", 0, -2) + ":///";
    tUri = Peony::FileUtils::urlDecode(tUri);
    qDebug()  << __func__ << tUri << scheme;
    if (scheme == uri) {
        return nullptr;
    }

    int lastIndex = tUri.lastIndexOf("/");
    QString realPath = tUri.left(lastIndex);
    if (realPath + "/" == scheme) {
        return g_file_new_for_uri(scheme.toUtf8().constData());
    }

    return g_file_new_for_uri(realPath.toUtf8().constData());
}

char* vfs_custom_file_get_basename (GFile* file) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);
    auto vfs_file = VFS_CUSTOM_VFS(file);
    QUrl url = QString(vfs_file->priv->uri);

    qDebug() << __func__ << url.path() << "url.fileName:" << url.fileName();
    return g_strdup(url.fileName().toUtf8().constData());
}

GFile* vfs_custom_file_set_display_name (GFile* file, const gchar* display_name, GCancellable* cancellable, GError** error) {
    Q_UNUSED(file);
    Q_UNUSED(display_name);
    Q_UNUSED(cancellable);
    Q_UNUSED(error);

    char *uri = g_file_get_uri(file);
    QString strUri = uri;
    QString name = display_name;
    QString scheme = strUri.section(":", 0, -2);
    Peony::VFSError *vfsError = nullptr;

    auto iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(scheme);
    if (iface) {
        vfsError = iface->handerFileRename(strUri, name);
    }

    if (vfsError) {
        if (!vfsError->message.isEmpty()) {
            *error = g_error_new(G_FILE_ERROR_FAILED, vfsError->code, "%s\n", vfsError->message.toUtf8().constData());
        } else {
            QString str = QObject::tr("Operation not supported");
            if(error){
                *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_CANCELLED, "%s\n", str.toUtf8().constData());
            };
        }
    }

    qDebug() << __func__ << strUri << name;

    return nullptr;
}

gboolean vfs_custom_file_make_directory(GFile* file, GCancellable* cancellable, GError** error){
    Q_UNUSED(file);
    Q_UNUSED(cancellable);
    Q_UNUSED(error);

    char *uri = g_file_get_uri(file);
    QString strUri = uri;
    QString scheme = strUri.section(":", 0, -2);
    Peony::VFSError *vfsError = nullptr;
    bool ret = false;

    if (g_cancellable_set_error_if_cancelled(cancellable, error)) {
        g_free(uri);
        return false;
    }

    Peony::VFSInfoPluginIface *iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(scheme);
    if (iface) {
        vfsError = iface->handerMakeDirectory(strUri, ret);
    }

    if (!ret) {
        if (vfsError && !vfsError->message.isEmpty()) {
            if (vfsError->code != Peony::ErrorType::ERROR_EXISTS) {
                *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_CANCELLED, "%s\n", vfsError->message.toUtf8().constData());
            } else {
                *error = g_error_new(G_FILE_ERROR_FAILED, vfsError->code, "%s\n", vfsError->message.toUtf8().constData());
            }
        } else {
            QString str = QObject::tr("Virtual file directories do not support move and copy operations");
            if(error){
                *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_CANCELLED, "%s\n", str.toUtf8().constData());
            };
        }
    }

    g_free(uri);
    return ret;
}

char* vfs_custom_file_get_relative_path (GFile* parent, GFile* descendant) {
    Q_UNUSED(parent);
    Q_UNUSED(descendant);
    return nullptr;
}

GFileOutputStream* vfs_custom_file_create(GFile* file, GFileCreateFlags flags, GCancellable* cancellable, GError** error) {
    Q_UNUSED(file);
    Q_UNUSED(flags);
    Q_UNUSED(cancellable);
    Q_UNUSED(error);

    char *destUri = g_file_get_uri(file);

    QString strDestUri = destUri;
    QString scheme = strDestUri.section(":", 0, -2);
    Peony::VFSError *vfsError = nullptr;
    bool ret = false;

    qDebug() << __func__ << scheme << strDestUri;

    if (g_cancellable_set_error_if_cancelled(cancellable, error)) {
        g_free(destUri);
        return nullptr;
    }

    Peony::VFSInfoPluginIface *iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(scheme);
    if (iface) {
        vfsError = iface->handerFileCreate(strDestUri, ret);
    }

    if (!ret) {
        if (vfsError && !vfsError->message.isEmpty()) {
            if (vfsError->code != Peony::ErrorType::ERROR_EXISTS) {
                *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_CANCELLED, "%s\n", vfsError->message.toUtf8().constData());
            } else {
                *error = g_error_new(G_FILE_ERROR_FAILED, vfsError->code, "%s\n", vfsError->message.toUtf8().constData());
            }
        } else {
            QString errorStr = QObject::tr("Operation not supported");
            if(error){
                *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_CANCELLED, "%s\n", errorStr.toUtf8().constData());
            };
        }
    }

    g_free(destUri);
    return nullptr;
}

gboolean vfs_custom_file_delete (GFile* file, GCancellable* cancellable, GError** error) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), FALSE);

    char *uri = g_file_get_uri(file);
    QString strUri = uri;
    QString scheme = strUri.section(":", 0, -2);
    Peony::VFSError *vfsError = nullptr;
    bool ret = false;

    if (g_cancellable_set_error_if_cancelled(cancellable, error)) {
        g_free(uri);
        return false;
    }

    Peony::VFSInfoPluginIface *iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(scheme);
    if (iface) {
        vfsError = iface->handerFileDelete(strUri, ret);
    }

    if (!ret) {
        if (vfsError && !vfsError->message.isEmpty()) {
            *error = g_error_new(G_FILE_ERROR_FAILED, vfsError->code, "%s\n", vfsError->message.toUtf8().constData());
        } else {
            QString errorStr = QObject::tr("Operation not supported");
            if(error){
                *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_CANCELLED, "%s\n", errorStr.toUtf8().constData());
            };
        }
    }

    qDebug() << __func__ << scheme << uri;
    g_free(uri);
    return true;
}

GFileInputStream* vfs_custom_file_read_fn(GFile* file, GCancellable* cancellable, GError** error) {
    Q_UNUSED(file);
    Q_UNUSED(cancellable);
    Q_UNUSED(error);

    gchar *sUri = g_file_get_uri(file);
    QString strUri = sUri;

    if (sUri) {
        g_free(sUri);
    }

    GFileInputStream *fileInputStream = nullptr;
    return fileInputStream;
}

gboolean vfs_custom_file_copy(GFile* source, GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback pcallback, gpointer pcallbackdata, GError** error) {
    Q_UNUSED(source);
    Q_UNUSED(destination);
    Q_UNUSED(flags);
    Q_UNUSED(cancellable);
    Q_UNUSED(pcallback);
    Q_UNUSED(pcallbackdata);
    Q_UNUSED(error);

    char *sourceFile = g_file_get_uri(source);
    char *destFile = g_file_get_uri(destination);
    QString strSourceFile = sourceFile;
    QString strDestFile = destFile;
    QString sourceScheme = strSourceFile.section(":", 0, -2);
    QString destScheme = strDestFile.section(":", 0, -2);
    Peony::VFSError *vfsError = nullptr;
    bool ret = false;

    QStringList keys = Peony::VFSInfoPluginManager::getInstance()->getAllPluginKeys();
    if (keys.contains(sourceScheme)) {
        auto iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(sourceScheme);
        if (iface) {
            vfsError = iface->handerFileCopy(strSourceFile, strDestFile, ret);
        }
    } else if (keys.contains(destScheme)) {
        auto iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(destScheme);
        if (iface) {
            vfsError = iface->handerFileCopy(strSourceFile, strDestFile, ret);
        }
    } else {
        QString str = QObject::tr("Virtual file directories do not support move and copy operations");
        *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_NOT_SUPPORTED, "%s\n", str.toUtf8().constData());
        g_free(sourceFile);
        g_free(destFile);
        return FALSE;
    }

    g_free(sourceFile);
    g_free(destFile);
    return ret;
}

gboolean vfs_custom_file_move(GFile* source, GFile* destination, GFileCopyFlags flags, GCancellable* cancellable, GFileProgressCallback progress_callback, gpointer progress, GError** error) {
    Q_UNUSED(source);
    Q_UNUSED(destination);
    Q_UNUSED(flags);
    Q_UNUSED(cancellable);
    Q_UNUSED(progress_callback);
    Q_UNUSED(progress);
    Q_UNUSED(error);


    gchar *sourceFile = g_file_get_uri(source);
    gchar *destFile = g_file_get_uri(destination);
    QString strSourceFile = sourceFile;
    QString strDestFile = destFile;
    QString sourceScheme = strSourceFile.section(":", 0, -2);
    QString destScheme = strDestFile.section(":", 0, -2);
    Peony::VFSError *vfsError = nullptr;
    bool ret = false;

    QStringList keys = Peony::VFSInfoPluginManager::getInstance()->getAllPluginKeys();
    if (keys.contains(sourceScheme)) {
        auto iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(sourceScheme);
        if (iface) {
            vfsError = iface->handerFileMove(strSourceFile, strDestFile, ret);
        }
    } else if (keys.contains(destScheme)) {
        auto iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(destScheme);
        if (iface) {
            vfsError = iface->handerFileMove(strSourceFile, strDestFile, ret);
        }
    } else {
        QString str = QObject::tr("Virtual file directories do not support move and copy operations");
        *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_NOT_SUPPORTED, "%s\n", str.toUtf8().constData());
        g_free(sourceFile);
        g_free(destFile);
        return FALSE;
    }

    g_free(sourceFile);
    g_free(destFile);

    if (!ret) {
        if (vfsError && !vfsError->message.isEmpty()) {
            *error = g_error_new(G_FILE_ERROR_FAILED, vfsError->code, "%s\n", vfsError->message.toUtf8().constData());
        } else {
            QString str = QObject::tr("Virtual file directories do not support move and copy operations");
            *error = g_error_new(G_FILE_ERROR_FAILED, G_IO_ERROR_NOT_SUPPORTED, "%s\n", str.toUtf8().constData());
        }
    }

    return ret;
}

char* vfs_custom_file_get_schema(GFile* file) {
    Q_UNUSED(file);
    return g_strdup("custom");
}

GFile* vfs_custom_file_resolve_relative_path(GFile *file, const char *relative_path) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);
    auto vfs_file = VFS_CUSTOM_FILE(file);
    QString path = relative_path;
    QString uri = vfs_file->priv->uri;
    QString scheme = uri.section(":", 0, -2) + ":///";
    QString realUri;

    gchar *szuri = g_file_get_uri(file);
    QString trueUri = szuri;
    if (szuri) {
        g_free(szuri);
    }

    if (scheme == trueUri) {
        realUri = QString("%1%2").arg(scheme).arg(path);
    } else {
        realUri = QString("%1/%2").arg(uri).arg(path);
    }

    //qDebug() << __func__ << realUri << uri << "relative_path:" << relative_path << "trueUri:" << trueUri;
    return g_file_new_for_uri(realUri.toUtf8().constData());
}

GFileInfo* vfs_custom_file_query_filesystem_info(GFile* file, const char* attributes, GCancellable* cancellable, GError** error) {
    return vfs_custom_file_query_info(file, attributes, G_FILE_QUERY_INFO_NONE, cancellable, error);
}

void vfs_custom_file_enumerator_set_uri(CustomVFSFileEnumerator *enumerator, const char *uri) {
    QString tUri(uri);
    QString scheme = tUri.section(":", 0, -2);
    QString tScheme = scheme + "://";
    tUri.remove(tScheme);

    Peony::VFSInfoPluginIface* iface = Peony::VFSInfoPluginManager::getInstance()->userSchemeGetPlugin(scheme);

    QStringList rootList = iface->fileEnumerator(tUri);
    //qDebug() << __func__ << QString(uri) << tUri;
    for (auto list : rootList) {
        QString str = tScheme + list;
        enumerator->priv->enumerate_queue->enqueue(str);
    }
}

GFileMonitor* vfs_custom_file_monitor_directory (GFile* file, GFileMonitorFlags flags, GCancellable* cancellable, GError** error) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);

    Q_UNUSED(file);
    Q_UNUSED(flags);
    Q_UNUSED(error);
    Q_UNUSED(cancellable);

    CustomVFSFilePrivate* priv = VFS_CUSTOM_FILE((CustomVFSFile*)file)->priv;
    char *uri = g_file_get_uri(file);
    QString strUri = uri;

    priv->dirMonitor = (GFileMonitor*) g_object_new(VFS_TYPE_CUSTOM_FILE_MONITOR, nullptr);

    vfs_custom_file_monitor_dir(VFS_CUSTOM_FILE_MONITOR(priv->dirMonitor), strUri);

    g_free(uri);

    return priv->dirMonitor;
}

GFileMonitor* vfs_custom_file_monitor_file (GFile* file, GFileMonitorFlags flags, GCancellable* cancellable, GError** error) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);

    Q_UNUSED(file);
    Q_UNUSED(flags);
    Q_UNUSED(error);
    Q_UNUSED(cancellable);

    char *uri = g_file_get_uri(file);
    QString strUri = uri;

    qDebug() << __func__ << strUri;

    g_free(uri);
    return nullptr;
}

GFileEnumerator* vfs_custom_file_enumerate_children(GFile *file,
              const char *attribute,
              GFileQueryInfoFlags flags,
              GCancellable *cancellable,
              GError **error) {
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);
    return vfs_custom_file_enumerate_children_internal(file, attribute, flags, cancellable, error);
}

GFileEnumerator *vfs_custom_file_enumerate_children_internal(GFile *file, const char *attribute, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error)
{
    g_return_val_if_fail(VFS_IS_CUSTOM_FILE(file), nullptr);
    Q_UNUSED(flags)
    Q_UNUSED(error)
    Q_UNUSED(attribute)
    Q_UNUSED(cancellable)

    auto vfsfile = VFS_CUSTOM_FILE(file);
    auto enumerator = VFS_CUSTOM_FILE_ENUMERATOR(g_object_new(VFS_TYPE_CUSTOM_FILE_ENUMERATOR, "container", file, nullptr));
    vfs_custom_file_enumerator_set_uri(enumerator, vfsfile->priv->uri);
    return G_FILE_ENUMERATOR(enumerator);
}
