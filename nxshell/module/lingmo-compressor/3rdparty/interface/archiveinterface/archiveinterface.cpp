/*
 * Copyright (c) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
 * Copyright (c) 2008-2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (c) 2009-2012 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 * Copyright (c) 2016 Vladyslav Batyrenko <mvlabat@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "archiveinterface.h"
#include "datamanager.h"

#include <QDebug>
#include <sys/stat.h>
#include <QDir>
#include <QStorageInfo>

Q_DECLARE_METATYPE(KPluginMetaData)


ReadOnlyArchiveInterface::ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    if (args.count() == 3) {
        m_strArchiveName = args.first().toString();
        m_metaData = args.at(1).value<KPluginMetaData>();
        m_mimetype = args.at(2).value<CustomMimeType>();
    }

    m_common = new Common(this);
}

ReadOnlyArchiveInterface::~ReadOnlyArchiveInterface()
{

}

bool ReadOnlyArchiveInterface::waitForFinished()
{
    return m_bWaitForFinished;
}

void ReadOnlyArchiveInterface::setPassword(const QString &strPassword)
{
    m_strPassword = strPassword;
}


QString ReadOnlyArchiveInterface::getPassword()
{
    return m_strPassword;
}

ErrorType ReadOnlyArchiveInterface::errorType()
{
    return m_eErrorType;
}

bool ReadOnlyArchiveInterface::doKill()
{
    return false;   // 修改默认为未取消
}

bool ReadOnlyArchiveInterface::status()
{
    return m_bPause;   // true 暂停状态，false 非暂停状态
}

void ReadOnlyArchiveInterface::setWaitForFinishedSignal(bool value)
{
    m_bWaitForFinished = value;
}

QFileDevice::Permissions ReadOnlyArchiveInterface::getPermissions(const mode_t &perm)
{
    QFileDevice::Permissions pers = QFileDevice::Permissions();

    // bug 118731  zip_file_get_external_attributes获取文件属性异常
    // 644 和0 一样作无效值考虑
    if (0 == perm || 644 == perm) {
        pers |= (QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        return pers;
    }

    if (perm & S_IRUSR) {
        pers |= QFileDevice::ReadUser;
    }
    if (perm & S_IWUSR) {
        pers |= QFileDevice::WriteUser;
    }
    if (perm & S_IXUSR) {
        pers |= QFileDevice::ExeUser;
    }

    if (perm & S_IRGRP) {
        pers |= QFileDevice::ReadGroup;
    }
    if (perm & S_IWGRP) {
        pers |= QFileDevice::WriteGroup;
    }
    if (perm & S_IXGRP) {
        pers |= QFileDevice::ExeGroup;
    }

    if (perm & S_IROTH) {
        pers |= QFileDevice::ReadOther;
    }
    if (perm & S_IWOTH) {
        pers |= QFileDevice::WriteOther;
    }
    if (perm & S_IXOTH) {
        pers |= QFileDevice::ExeOther;
    }

    return pers;
}

void ReadOnlyArchiveInterface::handleEntry(const FileEntry &entry)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    if (!entry.strFullPath.contains(QDir::separator()) || (entry.strFullPath.count(QDir::separator()) == 1 && entry.strFullPath.endsWith(QDir::separator()))) {
        if (m_setHasRootDirs.contains(entry.strFullPath)) {
            // 先清除，再追加，保证数据和压缩包数据一致
            for (int i = 0; i < stArchiveData.listRootEntry.count(); ++i) {
                if (stArchiveData.listRootEntry[i].strFullPath == entry.strFullPath) {
                    stArchiveData.listRootEntry[i] = entry;
                    break;
                }
            }
        } else {
            // 获取第一层数据（不包含'/'或者只有末尾一个'/'）
            stArchiveData.listRootEntry.push_back(entry);
            m_setHasRootDirs.insert(entry.strFullPath);
        }
    } else {
        // 多层数据，处理加载时不出现文件夹的情况
        int iIndex = entry.strFullPath.lastIndexOf(QDir::separator());
        QString strDir = entry.strFullPath.left(iIndex + 1);

        // 若此路径未处理过，进行分割处理
        if (!m_setHasHandlesDirs.contains(strDir)) {
            m_setHasHandlesDirs.insert(strDir);
            // 对全路径进行分割，获取所有文件夹名称
            QStringList fileDirs = entry.strFullPath.split(QDir::separator());
            QString folderAppendStr = "";
            for (int i = 0 ; i < fileDirs.size() - 1; ++i) {
                folderAppendStr += fileDirs[i] + QDir::separator();

                // 构建文件数据
                FileEntry entryDir;
                entryDir.strFullPath = folderAppendStr;
                entryDir.strFileName = fileDirs[i];
                entryDir.isDirectory = true;

                // 若第一层数据中未包含此文件夹，写入第一层数据
                if (i == 0 && !m_setHasRootDirs.contains(folderAppendStr)) {
                    stArchiveData.listRootEntry.push_back(entryDir);
                    m_setHasRootDirs.insert(folderAppendStr);
                }

                // 写入数据全集中
                if (stArchiveData.mapFileEntry.find(entryDir.strFullPath) == stArchiveData.mapFileEntry.end()) {
                    stArchiveData.mapFileEntry[entryDir.strFullPath] = entryDir;
                }
            }
        }
    }
}

bool ReadOnlyArchiveInterface::isInsufficientDiskSpace(QString diskPath, qint64 standard)
{
    QStorageInfo storage(QFileInfo(diskPath).absolutePath());
    qInfo() << "Available DiskSpace:" << diskPath << storage << storage.bytesAvailable();
    if (storage.isValid() && storage.bytesAvailable() < standard) {
        return true;
    } else {
        return false;
    }
}

//bool ReadOnlyArchiveInterface::getHandleCurEntry() const
//{
//    return m_bHandleCurEntry;
//}

//void ReadOnlyArchiveInterface::getFileEntry(QList<FileEntry> &listRootEntry, QMap<QString, FileEntry> &mapEntry)
//{
//    listRootEntry.clear();
//    mapEntry.clear();

//    listRootEntry = m_listRootEntry;
//    mapEntry = m_mapEntry;
//}



ReadWriteArchiveInterface::ReadWriteArchiveInterface(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{

}

ReadWriteArchiveInterface::~ReadWriteArchiveInterface()
{

}

QString ReadWriteArchiveInterface::getArchiveName()
{
    return m_strArchiveName;
}
