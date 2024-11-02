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

#include <QApplication>
#include <QProcess>
#include <QRegExp>
#include <file-info-job.h>
#include <file-info.h>

#include "file-operation.h"
#include "file-operation-manager.h"
#include "global-settings.h"

using namespace Peony;

QRegExp gInvalidName("[\\\\/:\\*\\?\\\"<>\\|\\n\\t]");/* 文件名或文件夹名中不能出现以下字符：\、/、:、*、?、"、<、>、| 、\n 、\t */

FileOperation::FileOperation(QObject *parent) : QObject (parent)
{
    m_cancellable_wrapper = wrapGCancellable(g_cancellable_new());
    setAutoDelete(true);
    connect(this, &FileOperation::operationPause, this, [=] () {
        m_is_pause.store(true);
    });
    connect(this, &FileOperation::operationResume, this, [=] () {
        m_is_pause.store(false);
        m_wait_condition.wakeOne();
    });
    connect(this, &FileOperation::operationCancel, this, [=] () {
        m_is_pause.store(false);
        m_wait_condition.wakeOne();
    });
    connect(this, &FileOperation::remoteFileEvent, FileOperationManager::getInstance(), &FileOperationManager::remoteFileEvent);
}

FileOperation::~FileOperation()
{

}

void FileOperation::run()
{

}

void FileOperation::setHasError(bool hasError)
{
    m_has_error = hasError;
    getOperationInfo().get()->m_has_error = hasError;
}

void FileOperation::cancel()
{
    g_cancellable_cancel(m_cancellable_wrapper.get()->get());
    m_is_cancelled = true;
}

void FileOperation::OperatorThreadPause()
{
    m_mutex.lock();
    while (m_is_pause) {
        m_wait_condition.wait(&m_mutex);
    }
    m_mutex.unlock();
}

bool FileOperation::nameIsValid (QString& name)
{
    if (nullptr == name) {
        return false;
    }

    return !name.contains(gInvalidName);
}

bool FileOperation::makeFileNameValidForDestFS(QString &srcPath, QString &destPath, QString *newFileName)
{
    FileInfoJob fileInfoJob(destPath);
    FileInfoJob fileInfoJobSrc(srcPath);
    fileInfoJob.querySync();
    fileInfoJobSrc.querySync();

    QString srcFileName = fileInfoJobSrc.getInfo()->displayName();
    *newFileName = srcFileName;
    QString fsType = fileInfoJob.getInfo()->fileSystemType();

    qDebug() << "target filesystem type is: " << fsType;

    if ("fat" == fsType || "vfat" == fsType || "fuse" == fsType || "ntfs" == fsType || "msdos" == fsType || "msdosfs" == fsType) {
        *newFileName = (*newFileName).replace(gInvalidName, "_");
        qDebug() << "uri:" << QUrl(srcPath).toDisplayString() << "target filesystem type is: " << fsType << "  old name:" << srcFileName << "  new name:" << *newFileName;
    }

    return *newFileName != srcFileName;
}

void FileOperation::fileSync(QString srcFile, QString destDir)
{
    if (srcFile == destDir)
        return;

    if (srcFile.endsWith("/")) {
        srcFile.chop(1);
    }

    QString destFile = "";
    g_autoptr (GFile) ddir = g_file_new_for_uri (destDir.toUtf8 ().constData ());
    if (G_FILE_TYPE_DIRECTORY & g_file_query_file_type (ddir, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL)) {
//        if (destDir.split("/").back() == srcFile.split("/").back()) {
//            // src and dest are directory
//            destFile = destDir;
//        } else {
//            destFile = destDir + "/" + srcFile.split("/").back();
//        }
        //fix bug#111385, rename folder sync wrong issue
        destFile = destDir;
    } else {
        g_autofree char* uri = g_file_get_uri (ddir);
        destFile = uri;
    }
    bool needSync = true;

//    if (!FileUtils::isFileExsit(destFile)) {
//        qDebug() << "file:" << destFile << " is not existed!";
//        return;
//    }

//    bool needSync = false;
    GFile* srcGfile = g_file_new_for_uri(srcFile.toUtf8().constData());
    GFile* destGfile = g_file_new_for_uri(destFile.toUtf8().constData());
    GMount* srcMount = g_file_find_enclosing_mount(srcGfile, nullptr, nullptr);
    GMount* destMount = g_file_find_enclosing_mount(destGfile, nullptr, nullptr);

    // xxxMount is null in root filesystem
    if ((srcMount != destMount) && (NULL != destMount)) {
        needSync = true;
    }

    if (needSync) {
        Q_EMIT operationStartSnyc();
        auto path = g_file_get_path(destGfile);
        qDebug() << "DJ- sync -- src: " << srcFile << "  ===  " << destDir << "  ===  " << destFile << "  path:" << path;
        if (path) {
            QProcess p;
            auto shellPath = g_shell_quote(path);
            qDebug() << "DJ- start execute: " << QString("sync -f %1").arg(shellPath);
            p.start(QString("/usr/bin/sync -f %1").arg(shellPath));
            qDebug() << "DJ- execute: " << QString("sync -f %1  ok!!!").arg(shellPath);
            g_free(path);
            g_free(shellPath);
            p.waitForFinished(-1);
        }
    }

    if (nullptr != srcMount) {
        g_object_unref(srcMount);
    }

    if (nullptr != destMount) {
        g_object_unref(destMount);
    }

    if (nullptr != srcGfile) {
        g_object_unref(srcGfile);
    }

    if (nullptr != destGfile) {
        g_object_unref(destGfile);
    }
}

void FileOperation::notifyFileWatcherOperationFinished()
{
    if (!qApp->allWidgets().isEmpty()) {
        // notify operation for file watchers.
        auto info = this->getOperationInfo();
        qDebug()<<info->m_src_dir_uri;
        if (info) {
            FileOperationManager::getInstance()->manuallyNotifyDirectoryChanged(info.get());
        }
    }
}

QStringList FileOperation::sortUris(const QStringList &uris, const int &type)
{
    QStringList sortedURIs = uris;
    std::sort(sortedURIs.begin(), sortedURIs.end(), URISorter(type));
    return sortedURIs;
}

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>
void FileOperation::sendSrcAndDestUrisOfCopyDspsFiles()
{
    bool sendUrisOfCopyDspsFiles = Peony::GlobalSettings::getInstance()->isExist(SEND_URIS_OF_COPY_DSPS)?
                Peony::GlobalSettings::getInstance()->getValue(SEND_URIS_OF_COPY_DSPS).toBool() : false;

    if(!sendUrisOfCopyDspsFiles)
        return;

    if(!m_srcUrisOfCopyDspsFiles.size() || !m_destUrisOfCopyDspsFiles.size())
        return;

    QDBusMessage msg = QDBusMessage::createMethodCall("org.lingmo.explor", "/org/lingmo/explor",
                     "org.lingmo.explor", "receiveSrcAndDestUrisOfCopy");
    QList<QVariant> args;
    args.append(QVariant(m_srcUrisOfCopyDspsFiles));
    args.append(QVariant(m_destUrisOfCopyDspsFiles));
    msg.setArguments(args);
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (!response.type() == QDBusMessage::ReplyMessage)
        qDebug()<<"fail to send source and dest uris of copy!";
}

bool FileOperation::URISorter::operator()(const QString &uri1, const QString &uri2) const {
    bool isFolder1 = FileUtils::isFileDirectory(uri1);
    bool isFolder2 = FileUtils::isFileDirectory(uri2);
    if (isFolder1 == isFolder2) {
        return true;
    }
    if (isFolder1 && !isFolder2) {
        return directoryType == 0 ? false : true;
    }
    if (!isFolder1 && isFolder2) {
        return directoryType == 0 ? true : false;
    }
    return isFolder1;
}
