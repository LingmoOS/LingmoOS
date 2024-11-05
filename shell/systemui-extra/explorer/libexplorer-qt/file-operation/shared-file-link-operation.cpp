/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#include "shared-file-link-operation.h"
#include "file-operation-manager.h"
#include "file-info.h"

#include "gerror-wrapper.h"
#include <gio/gdesktopappinfo.h>
#include <glib/gprintf.h>
#include <QUrl>
#include <QProcess>
#include <fcntl.h>
using namespace Peony;

SharedFileLinkOperation::SharedFileLinkOperation(QString srcUri, QString destDirUri, QObject *parent) : FileOperation (parent)
{
    m_src_uri = srcUri;
    QUrl url = srcUri;
    auto fileInfo = FileInfo::fromUri(m_src_uri);
    m_display_name = fileInfo->displayName();
    auto fileName = m_display_name.isEmpty() ? url.fileName() : m_display_name;
    //If it starts with a ".", add it directly to the end
    if(fileName.startsWith('.')){
        m_dest_uri = destDirUri + "/" + fileName + " - " + tr("Symbolic Link");
    }else{
        // Otherwise, add it directly to the front
        m_dest_uri = destDirUri + "/" + tr("Symbolic Link") + " - " + fileName;
    }

    //m_dest_uri = QUrl::fromEncoded(m_dest_uri.toUtf8()).toDisplayString();
    QStringList fake_uris;
    fake_uris<<srcUri;
    m_info = std::make_shared<FileOperationInfo>(fake_uris, destDirUri, FileOperationInfo::Link);
}

SharedFileLinkOperation::~SharedFileLinkOperation()
{

}

void SharedFileLinkOperation::run()
{
    operationStarted();
    createShareFilesSymbolicLink(m_src_uri);
    operationFinished();

}

void SharedFileLinkOperation::createShareFilesSymbolicLink(QString &srcUri)
{
    m_dest_uri += ".desktop";
    QUrl url(m_dest_uri);
    QString desktopfp =  url.path();
    if (!url.fragment().isEmpty()) {
        desktopfp = url.path() + "#" + url.fragment();
    }

    g_autoptr (GError) error = nullptr;
    if(g_file_test(desktopfp.toUtf8().constData(), G_FILE_TEST_EXISTS)) {
        error = g_error_new (1, G_IO_ERROR_EXISTS, "%s", QString(tr("The dest file \"%1\" has existed!")).arg(desktopfp).toUtf8().constData());
        FileOperationError except;
        except.srcUri = m_src_uri;
        except.errorType = ET_GIO;
        except.isCritical = true;
        except.op = FileOpLink;
        except.title = tr("Link file error");
        except.destDirUri = m_dest_uri;
        except.errorStr = error->message;
        except.dlgType = ED_WARNING;
        Q_EMIT errored(except);
        return;
    }

    GKeyFile* keyfile = g_key_file_new ();

    g_key_file_set_value(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_STARTUP_NOTIFY, "true");
    g_key_file_set_value(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TYPE, "Application");

    QUrl srcUrl = srcUri;
    QString exec = "explorer " + FileUtils::urlDecode(srcUri);
    g_key_file_set_value(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, exec.toUtf8().constData());
    g_key_file_set_value(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, "folder-remote");

    QString fileName = url.fileName();
    int index = fileName.lastIndexOf(".");
    fileName.truncate(index);
    if (!m_display_name.isEmpty()) {
        fileName = m_display_name;
    }
    g_key_file_set_value(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, fileName.toUtf8().constData());
    g_key_file_set_value(keyfile, G_KEY_FILE_DESKTOP_GROUP, "X-Peony-CMD", "true");

    // 可能会有路径重复的情况，后续需要做异常处理
    g_key_file_save_to_file(keyfile, desktopfp.toUtf8().constData(), &error);

    if (keyfile) {
        g_key_file_free(keyfile);
    }

    QString destUri = FileUtils::urlEncode(m_dest_uri);
    g_autoptr(GFile) destFile = g_file_new_for_uri(destUri.toUtf8().constData());
    if (destFile) {
        mode_t mod = S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP;
        g_file_set_attribute_uint32(destFile, G_FILE_ATTRIBUTE_UNIX_MODE, (guint32)mod, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
    }
}
