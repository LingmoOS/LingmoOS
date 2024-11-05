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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "create-template-operation.h"

#include "file-operation-manager.h"
#include "file-operation-utils.h"
#include "global-settings.h"
#include <gio/gio.h>

#include <utime.h>

#include <QMessageBox>
#include <QProcess>
#include <file-utils.h>

using namespace Peony;

#define TEMPLATE_DIR "file://" + QString(g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES)) + "/"

void CreateTemplateOperation::handleDuplicate(const QString &uri)
{
    m_target_uri = m_dest_dir_uri + "/" + FileUtils::handleDuplicateName(uri);
}

CreateTemplateOperation::CreateTemplateOperation(const QString &destDirUri, Type type, const QString &templateName, QObject *parent) : FileOperation(parent)
{
    QString tDestDirUri = destDirUri;
    GFile *file = g_file_new_for_uri(destDirUri.toUtf8().constData());
    g_autofree char *uri = g_file_get_uri(file);
    if (uri) {
        tDestDirUri = uri;
    }
    g_object_unref(file);

    QString scheme = destDirUri.section(":", 0, -2) + ":///";
    if (tDestDirUri.endsWith("/") && tDestDirUri != scheme) {
        tDestDirUri.chop(1);
    }

    m_target_uri = tDestDirUri + "/" + templateName;
    qDebug() << __func__ << "tDestDirUri:" << tDestDirUri << "destUri:" << destDirUri << m_target_uri;
    QStringList srcUris;
    m_src_uri = TEMPLATE_DIR + templateName;
    srcUris << m_src_uri;
    m_dest_dir_uri = tDestDirUri;
    m_type = type;
    m_info = std::make_shared<FileOperationInfo>(srcUris, destDirUri, FileOperationInfo::Type::Copy);
}

void CreateTemplateOperation::run()
{
    Q_EMIT operationStarted();
    Q_EMIT operationPrepared();
    switch (m_type) {
    case EmptyFile: {
        m_target_uri = m_dest_dir_uri + "/" + tr("NewFile") + ".txt";
retry_create_empty_file:
        GError *err = nullptr;
        GFileOutputStream *newFile = g_file_create(wrapGFile(g_file_new_for_uri(FileUtils::urlEncode(m_target_uri).toUtf8())).get()->get(), G_FILE_CREATE_NONE, nullptr, &err);
        if (err) {
            FileOperationError except;
            if (err->code == G_IO_ERROR_EXISTS) {
                g_error_free(err);
                handleDuplicate(m_target_uri);
                goto retry_create_empty_file;
            } else {
                except.srcUri = m_src_uri;
                except.dlgType = ED_WARNING;
                except.destDirUri = m_dest_dir_uri;
                except.isCritical = true;
                except.op = FileOpCreateTemp;
                except.title = tr("Create file");
                except.errorCode = err->code;
                except.errorStr = err->message;
                except.errorType = ET_GIO;
                Q_EMIT errored(except);
                g_error_free(err);
            }
        }
        //fix bug 35145, function occupy udisk issue
        g_object_unref(newFile);
        break;
    }
    case EmptyFolder: {
        m_target_uri = m_dest_dir_uri + "/" + tr("NewFolder");
retry_create_empty_folder:
        GError *err = nullptr;
        g_file_make_directory(wrapGFile(g_file_new_for_uri(FileUtils::urlEncode(m_target_uri).toUtf8())).get()->get(),
                              nullptr,
                              &err);
        if (err) {
            // todo: Allow user naming
            if (err->code == G_IO_ERROR_EXISTS) {
                g_error_free(err);
                handleDuplicate(m_target_uri);
                goto retry_create_empty_folder;
            } else {
                FileOperationError except;
                except.srcUri = m_src_uri;
                except.dlgType = ED_WARNING;
                except.destDirUri = m_dest_dir_uri;
                except.isCritical = true;
                except.op = FileOpCreateTemp;
                except.title = tr("Create file error");
                except.errorCode = err->code;
                except.errorStr = err->message;
                except.errorType = ET_GIO;
                Q_EMIT errored(except);
                g_error_free(err);
            }
        }
        break;
    }
    case Template: {
retry_create_template:
        qDebug() << "create tmp";
        GError *err = nullptr;
        g_file_copy(wrapGFile(g_file_new_for_uri(FileUtils::urlEncode(m_src_uri).toUtf8())).get()->get(),
                    wrapGFile(g_file_new_for_uri(m_target_uri.toUtf8())).get()->get(),
                    GFileCopyFlags(G_FILE_COPY_TARGET_DEFAULT_PERMS | G_FILE_COPY_NOFOLLOW_SYMLINKS),
                    nullptr,
                    nullptr,
                    nullptr,
                    &err);
        if (err) {
            setHasError(true);
            if (err->code == G_IO_ERROR_EXISTS) {
                g_error_free(err);
                handleDuplicate(m_target_uri);
                goto retry_create_template;
            } else {
                FileOperationError except;
                except.srcUri = m_src_uri;
                except.dlgType = ED_WARNING;
                except.destDirUri = m_dest_dir_uri;
                except.isCritical = true;
                except.op = FileOpCreateTemp;
                except.title = tr("Create file error");
                except.errorCode = err->code;
                except.errorStr = err->message;
                except.errorType = ET_GIO;
                Q_EMIT errored(except);
                g_error_free(err);
            }
        } else {
            setHasError(false);
        }
        // change file's modify time and access time after copy templete file;
        time_t now_time = time(NULL);
        g_file_set_attribute_uint64(wrapGFile(g_file_new_for_uri(m_target_uri.toUtf8())).get()->get(),
                                    G_FILE_ATTRIBUTE_TIME_MODIFIED,
                                    (guint64)now_time,
                                    G_FILE_QUERY_INFO_NONE, nullptr, &err);

        //需要设置一下访问时间属性，修复前场反馈的在U盘创建wps文件，访问时间异常问题，时分秒为：000000
        g_file_set_attribute_uint64(wrapGFile(g_file_new_for_uri(m_target_uri.toUtf8())).get()->get(),
                                    G_FILE_ATTRIBUTE_TIME_ACCESS,
                                    (guint64)now_time,
                                    G_FILE_QUERY_INFO_NONE, nullptr, &err);
        break;
    }
    }
    FileUtils::saveCreateTime (m_target_uri);
    // judge if the operation should sync.
    bool needSync = false;
    GFile *src_first_file = g_file_new_for_uri(FileUtils::urlEncode(m_src_uri).toUtf8().constData());
    GMount *src_first_mount = g_file_find_enclosing_mount(src_first_file, nullptr, nullptr);
    if (src_first_mount) {
        needSync = g_mount_can_unmount(src_first_mount);
        g_object_unref(src_first_mount);
    } else {
        // maybe a vfs file.
        needSync = true;
    }
    g_object_unref(src_first_file);

    GFile *dest_dir_file = g_file_new_for_uri(FileUtils::urlEncode(m_dest_dir_uri).toUtf8().constData());
    GMount *dest_dir_mount = g_file_find_enclosing_mount(dest_dir_file, nullptr, nullptr);

    //fix create folder not sync issue, bug#128665, 132326
    if (m_type == EmptyFolder) {
        dest_dir_file = g_file_new_for_uri(FileUtils::urlEncode(m_target_uri).toUtf8().constData());
    }

    if (src_first_mount) {
        needSync = g_mount_can_unmount(dest_dir_mount);
        g_object_unref(dest_dir_mount);
    } else {
        needSync = true;
    }

    //needSync = true;

    if (needSync) {
        auto path = g_file_get_path(dest_dir_file);
        if (path) {
            operationStartSnyc();
            QProcess p;
            p.start(QString("/usr/bin/sync -f '%1'").arg(path));
            p.waitForFinished(-1);
            g_free(path);
        }
    }

    g_object_unref(dest_dir_file);

    // as target()
    m_info.get()->m_dest_dir_uri = m_target_uri;
    m_info.get()->m_dest_uris.clear();
    m_info.get()->m_dest_uris<<m_target_uri;

    setProperty("targetUri", m_target_uri);

    Q_EMIT operationFinished();
    notifyFileWatcherOperationFinished();
}
