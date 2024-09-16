/*
 * Copyright (c) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
 * Copyright (c) 2008-2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (c) 2010 Raphael Kubo da Costa <rakuco@FreeBSD.org>
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

#include "libarchiveplugin.h"
#include "queries.h"
#include "datamanager.h"
#include "common.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QUuid>

#include <archive_entry.h>
#include <linux/limits.h>

LibarchivePlugin::LibarchivePlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
{
//    m_bHandleCurEntry = true; //提取使用选中文件
//    connect(this, &ReadOnlyArchiveInterface::error, this, &LibarchivePlugin::slotRestoreWorkingDir);
//    connect(this, &ReadOnlyArchiveInterface::cancelled, this, &LibarchivePlugin::slotRestoreWorkingDir);
}

LibarchivePlugin::~LibarchivePlugin()
{
    deleteTempTarPkg(m_tars);
}

PluginFinishType LibarchivePlugin::list()
{
    qInfo() << "LibarchivePlugin插件加载压缩包数据";
    PluginFinishType eType;
    DataManager::get_instance().archiveData().reset();
    m_setHasHandlesDirs.clear();
    m_setHasRootDirs.clear();
    m_mapCode.clear();

    m_strOldArchiveName = m_strArchiveName; //保存原压缩包名
    QFileInfo fInfo(m_strArchiveName);
    QString fileName = fInfo.fileName();
    //因为tar.bz2、tar.lzma、tar.Z直接list时间较长，所以先用7z解压再list处理
    if (fileName.endsWith(".tar.bz2") || fileName.endsWith(".tar.lzma") || fileName.endsWith(".tar.Z")) {
        // 设置解压临时路径
        QString strProcessID = QString::number(QCoreApplication::applicationPid());   // 获取应用进程号
        QString tempFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                               + QDir::separator() + strProcessID + QDir::separator()
                               + QUuid::createUuid().toString(QUuid::Id128);
        QString tempFileName = tempFilePath + QDir::separator() + fileName.left(fileName.size() - fInfo.suffix().size() - 1);

        //QString commandLine = QString("%1%2%3%4").arg("7z x ").arg(filename()).arg(" -aoa -o").arg(tempFilePath);
        QStringList listArgs;
        listArgs << "x" << m_strArchiveName << "-aoa" << "-o" + tempFilePath;
        QString strProgram = QStandardPaths::findExecutable("7z");
        QProcess cmd;
        cmd.start(strProgram, listArgs);
        if (cmd.waitForFinished(-1)) {
            m_strArchiveName = tempFileName;
            if (!m_tars.contains(tempFileName)) {
                m_tars.push_back(tempFileName);
            }
            eType = list_New();
        } else {
            eType = PFT_Error;
        }
    } else {
        eType = list_New();
    }

    //emit signalFinished(eType);
    return eType;
}

PluginFinishType LibarchivePlugin::testArchive()
{
    return PFT_Nomral;
}

PluginFinishType LibarchivePlugin::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    m_eErrorType = ET_NoError;
    m_bOverwriteAll = false;        //是否全部覆盖
    m_bSkipAll = false;             // 是否全部跳过
    m_mapLongName.clear();
    m_setLongName.clear();
    m_mapLongDirName.clear();
    m_mapRealDirValue.clear();

    if (!initializeReader()) {
        m_eErrorType = ET_FileReadError;
        return PFT_Error;
    }

    ArchiveWrite writer(archive_write_disk_new());
    if (!writer.data()) {
        m_eErrorType = ET_FileReadError;
        return PFT_Error;
    }

    // 选择要保留的属性
    archive_write_disk_set_options(writer.data(), extractionFlags());

    bool isAtRootPath = false; //是否提取首层目录下的文件
    const bool extractAll = files.isEmpty();//如果是双击解压，则为false;如果是按钮解压，则为true
    if (extractAll) {  //全部解压

    } else { //部分解压、提取
        const QString &rootNode = options.strDestination;
        if (rootNode.isEmpty() || "/" == rootNode) {
            isAtRootPath = true;
        }
    }

    // 判断解压路径是否存在，不存在则创建文件夹
    if (QDir().exists(options.strTargetPath) == false) {
        if (!QDir().mkpath(options.strTargetPath)) {
            if (isInsufficientDiskSpace(options.strTargetPath, 10 * 1024 * 1024)) { // 暂取小于10M作为磁盘空间不足的判断标准
                m_eErrorType = ET_InsufficientDiskSpace;
            } else {
                m_eErrorType = ET_FileWriteError;
            }

            qInfo() << "Failed to create extractDestDir";
            return PFT_Error;
        }
    }

    // 更改应用工作目录，结束时自动恢复原来路径
    HandleWorkingDir handleWorkingDir(&m_oldWorkingDir);
    handleWorkingDir.change(options.strTargetPath);
    m_extractDestDir = options.strTargetPath;

    int extractedEntriesCount = 0; //记录已经解压的文件数量

    struct archive_entry *entry = nullptr;

    QString extractDst;
    bool bDlnfs = m_common->isSubpathOfDlnfs(options.strTargetPath);

    // Iterate through all entries in archive.
    int iIndex = 0;     // 存储索引值
    while (!QThread::currentThread()->isInterruptionRequested() && (archive_read_next_header(m_archiveReader.data(), &entry) == ARCHIVE_OK)) {
        // 空压缩包直接跳过
        if (!extractAll && 0 == m_ArchiveEntryCount) {
            break;
        }

        const bool entryIsDir = S_ISDIR(archive_entry_mode(entry)); //该条entry是否是文件夹

        const char *name = archive_entry_pathname(entry);
        QString entryName = m_common->trans2uft8(name, m_mapCode[QString(name)]); //该条entry在压缩包内文件名(全路径)

        // 右键解压到当前文件夹
        if (!options.bExistList && iIndex == 0) {
            FileEntry stEntry;
            stEntry.strFullPath = entryName;
            DataManager::get_instance().archiveData().listRootEntry << stEntry;
        }
        iIndex++;

        // Some archive types e.g. AppImage prepend all entries with "./" so remove this part.
        // 移除"./"开头的，例如rpm包
        if (entryName.startsWith(QLatin1String("./"))) {
            entryName.remove(0, 2);
        }
        if (entryName.isEmpty()) {
            continue;
        }

        // Static libraries (*.a) contain the two entries "/" and "//".
        // We just skip these to allow extracting this archive type.
        if (entryName == QLatin1String("/") || entryName == QLatin1String("//")) {
            archive_read_data_skip(m_archiveReader.data());
            continue;
        }

        // For now we just can't handle absolute filenames in a tar archive.
        // TODO: find out what to do here!!
        // 目前，无法处理tar归档文件中的绝对文件名
        if (entryName.startsWith(QLatin1Char('/'))) {
            return PFT_Error;
        }

        // Should the entry be extracted?
        // 解压:不需要过滤
        // 提取:过滤选中的文件以及选中目录下所有文件
        if (!extractAll) {
            bool flag = false;
            foreach (auto tmp, qAsConst(files)) {
                if (tmp.isDirectory) {
                    if (entryName.startsWith(tmp.strFullPath)) {
                        flag = true;
                        break;
                    }
                } else {
                    if (0 == entryName.compare(tmp.strFullPath)) {
                        flag = true;
                        break;
                    }
                }
            }

            // If entry is not found, skip entry.
            if (!flag) {
                continue;
            }
        }

        // entryFI is the fileinfo pointing to where the file will be
        // written from the archive.

        // 针对文件夹名称过长的情况，直接提示解压失败，文件夹名称过长
//        QStringList listPath = entryName.split(QDir::separator());
//        listPath.removeLast();
//        for (int i = 0; i < listPath.count(); ++i) {
//            if (NAME_MAX < QString(listPath[i]).toLocal8Bit().length()) {
//                emit signalCurFileName(entryName); // 发送当前正在解压的文件名
//                m_eErrorType = ET_LongNameError;
//                return PFT_Error;
//            }
//        }


        QString strFilePath;
        QString strTempFileName = entryName;
        int iSepIndex = entryName.lastIndexOf(QDir::separator());

        if (iSepIndex >= 0) {
            strFilePath = entryName.left(iSepIndex);
            strTempFileName = entryName.right(entryName.length() - iSepIndex - 1);
        }

        bool bLongName = false;
        QString tempFilePathName;
        QString strOriginName = entryName;

        if(!bDlnfs) {
            QString sDir = m_common->handleLongNameforPath(strFilePath, entryName, m_mapLongDirName, m_mapRealDirValue);
            if(sDir.length() > 0) {
               strFilePath = sDir.endsWith(QDir::separator())?sDir.left(sDir.length() -1):sDir;
               if(entryName.endsWith(QDir::separator())) {
                   entryName = sDir;
               } else if (NAME_MAX >= QString(strTempFileName).toLocal8Bit().length()) {
                   entryName = sDir + strTempFileName;
               }
               if(!m_mapLongDirName.isEmpty()) {
                   bLongName = true;
               }
            }

            if (NAME_MAX < QString(strTempFileName).toLocal8Bit().length() && !entryName.endsWith(QDir::separator())) {
                QString strTemp = strTempFileName.left(TRUNCATION_FILE_LONG);

                // 保存文件路径，不同目录下的同名文件分开计数,文件解压结束后才添加计数，
                tempFilePathName = strFilePath + QDir::separator() + strTemp;   // 路径加截取后的文件名
                if (m_mapLongName[tempFilePathName] >= LONGFILE_SAME_FILES) {
                    emit signalCurFileName(entryName); // 发送当前正在解压的文件名
                    m_eErrorType = ET_LongNameError;
                    return PFT_Error;
                }
                m_eErrorType = ET_LongNameError;
                bLongName = true;

                // bug 117553 tar格式含有多个长名称文件的压缩包，解压第一个同名文件编号是（000）
                strTempFileName = strTemp + QString("(%1)").arg(m_mapLongName[tempFilePathName] + 1, LONGFILE_SUFFIX_FieldWidth, BINARY_NUM, QChar('0')) + "." + QFileInfo(entryName).completeSuffix();
                entryName = strTempFileName;
                if (iIndex >= 0) {
                    entryName = strFilePath + QDir::separator() + strTempFileName;
                }
            }
        }
        QFileInfo entryFI(entryName);

        // If the file has a rootNode attached, remove it from file path.
        // 提取操作不需要保留上层目录
        if (!extractAll && !options.strDestination.isEmpty()) { //提取
            const QString &rootNode = options.strDestination;
            if (isAtRootPath) { //提取首层目录下文件
                archive_entry_copy_pathname(entry, entryName.toUtf8().constData());
            } else {
                if(bLongName) {
                    int nCnt = rootNode.count(QDir::separator());
                    int nIndex = 0;
                    for(int i = 0; i < nCnt; i++){
                        nIndex = entryName.indexOf(QDir::separator(), nIndex);
                        nIndex++;
                    }
                    const QString truncatedFilename(entryName.remove(0, nIndex));
                    archive_entry_copy_pathname(entry, QFile::encodeName(truncatedFilename).constData());
                    entryFI = QFileInfo(truncatedFilename);
                } else {
                    const QString truncatedFilename(entryName.remove(entryName.indexOf(rootNode), rootNode.size()));
                    archive_entry_copy_pathname(entry, QFile::encodeName(truncatedFilename).constData());
                    entryFI = QFileInfo(truncatedFilename);
                }
            }
        } else {
            archive_entry_copy_pathname(entry, entryName.toUtf8().constData());
        }

        if (bLongName) {
            emit signalCurFileName(strOriginName); // 发送当前正在解压的文件名
        } else {
            emit signalCurFileName(entryName); // 发送当前正在解压的文件名
        }

        // Check if the file about to be written already exists.
        if (!entryIsDir && entryFI.exists() && !m_setLongName.contains(entryName)) {
            if (m_bSkipAll) {
                archive_read_data_skip(m_archiveReader.data());
                archive_entry_clear(entry);
                continue;
            } else if (!m_bOverwriteAll) {
                OverwriteQuery query(entryName);
                emit signalQuery(&query);
                query.waitForResponse();

                if (query.responseCancelled()) { //取消结束
                    emit signalCancel();
                    archive_read_data_skip(m_archiveReader.data());
                    archive_entry_clear(entry);
                    m_eErrorType = ET_UserCancelOpertion;
                    return PFT_Cancel;
                } else if (query.responseSkip()) { //跳过
                    archive_read_data_skip(m_archiveReader.data());
                    archive_entry_clear(entry);
                    m_eErrorType = ET_NoError;
                    continue;
                } else if (query.responseSkipAll()) { //全部跳过
                    archive_read_data_skip(m_archiveReader.data());
                    archive_entry_clear(entry);
                    m_bSkipAll = true;
                    m_eErrorType = ET_NoError;
                    continue;
                } else if (query.responseOverwriteAll()) { //全部覆盖
                    m_bOverwriteAll = true;
                }
            }
        }

        // If there is an already existing directory.
        if (entryIsDir && entryFI.exists()) {
            if (entryFI.isWritable()) {
            } else {
                archive_entry_clear(entry);
                archive_read_data_skip(m_archiveReader.data());
                continue;
            }
        }

        if (bLongName) {
            m_setLongName << entryName;
        }

        // Write the entry header and check return value.
        const int returnCode = archive_write_header(writer.data(), entry); //创建文件
        switch (returnCode) {
        case ARCHIVE_OK: {
            m_mapLongName[tempFilePathName]++;   // 保存文件路径，不同目录下的同名文件分开计数，解压成功，添加计数

            copyDataFromSource(m_archiveReader.data(), writer.data(), QFileInfo(m_strArchiveName).size());

            // qInfo() <<  destinationDirectory + QDir::separator() + entryName;
            // 文件权限设置
            QFileDevice::Permissions per = getPermissions(archive_entry_perm(entry));
            if (entryIsDir) {
                per |= QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
            }
            QFile::setPermissions(/*destinationDirectory + QDir::separator() + */entryName, per);
        }
        break;
        case ARCHIVE_FAILED: {
            QList<QString> entryNameList = entryName.split('/');
            foreach (auto &tmp, entryNameList) {
                // 判断文件名是否过长
                if (NAME_MAX < tmp.toLocal8Bit().length()) {
                    m_eErrorType = ET_LongNameError;
                    return PFT_Error;
                }
            }

            m_eErrorType = ET_FileWriteError;
            emit error(("Filed error, extraction aborted."));
            return PFT_Error;
        }
        case ARCHIVE_FATAL: {
            if (isInsufficientDiskSpace(m_extractDestDir, FILE_MAX_SIZE)) { // 暂取小于10M作为磁盘空间不足的判断标准
                m_eErrorType = ET_InsufficientDiskSpace;
            } else {
                m_eErrorType = ET_FileWriteError;
            }

            emit error(("Fatal error, extraction aborted."));
            return PFT_Error;
        }
        default:
            break;
        }

        extractedEntriesCount++;
    }

    if (archive_read_close(m_archiveReader.data()) == ARCHIVE_OK) {
        emit signalprogress(1);
        return PFT_Nomral;
    } else {
        m_eErrorType = ET_FileReadError;
        return PFT_Error;
    }
}

PluginFinishType LibarchivePlugin::addFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    Q_UNUSED(files)
    Q_UNUSED(options)
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::moveFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::copyFiles(const QList<FileEntry> &files, const CompressOptions &options)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::deleteFiles(const QList<FileEntry> &files)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::renameFiles(const QList<FileEntry> &files)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::addComment(const QString &comment)
{
    return PFT_Error;
}

PluginFinishType LibarchivePlugin::updateArchiveData(const UpdateOptions &options)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    QString rootEntry;

    foreach (FileEntry entry, options.listEntry) {
        if (options.eType == UpdateOptions::Delete ) { // 删除，更新压缩包数据
            if (entry.isDirectory) { // 删除文件夹
                // 在map中查找该文件夹下的文件并删除
                QMap<QString, FileEntry>::iterator itor = stArchiveData.mapFileEntry.begin();
                while (itor != stArchiveData.mapFileEntry.end()) {
                    if (itor->strFullPath.startsWith(entry.strFullPath)) {
                        if (!itor->isDirectory) {
                            stArchiveData.qSize -= itor->qSize; // 更新压缩包内文件原始总大小
                        }
                        itor = stArchiveData.mapFileEntry.erase(itor);
                    } else {
                        ++itor;
                    }
                }

                // 更新文件夹第一层的数据
                if (entry.strFullPath.endsWith(QLatin1Char('/')) && entry.strFullPath.count(QLatin1Char('/')) == 1) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            } else { // 删除文件
                stArchiveData.qSize -= entry.qSize; // 更新压缩包内文件原始总大小
                stArchiveData.mapFileEntry.remove(entry.strFullPath); //在map中删除该文件
                // 更新文件夹第一层的数据
                if (!entry.strFullPath.contains(QLatin1Char('/'))) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            break;
                        }
                    }
                }
            }
        } else if (options.eType == UpdateOptions::Rename) { // 重命名，更新压缩包数据
            QMap<QString, FileEntry> tmpMapFileEntry;
            QString strAlias;
            if (entry.isDirectory) { // 重命名文件夹
                // 在map中查找该文件夹下的文件并重命名
                QMap<QString, FileEntry>::iterator itor = stArchiveData.mapFileEntry.begin();
                while (itor != stArchiveData.mapFileEntry.end()) {
                    if (itor->strFullPath.startsWith(entry.strFullPath)) {
                        QString strPath = QFileInfo(entry.strFullPath.left(entry.strFullPath.length() - 1)).path();
                        if(strPath == "."){
                            strAlias = entry.strAlias + QDir::separator();
                        } else {
                            strAlias = strPath + QDir::separator() + entry.strAlias + QDir::separator();
                        }
                        strAlias = strAlias + QString(itor->strFullPath).right(QString(itor->strFullPath).length()-entry.strFullPath.length());
                        FileEntry tmpEntry = itor.value();
                        tmpEntry.strFullPath = strAlias;
                        if(tmpEntry.isDirectory) {
                            tmpEntry.strFileName = QFileInfo(strAlias.left(strAlias.length() - 1)).fileName();
                        } else {
                            tmpEntry.strFileName = QFileInfo(strAlias).fileName();
                        }
                        tmpEntry.strAlias.clear();
                        tmpMapFileEntry.insert(strAlias, tmpEntry);
                        itor = stArchiveData.mapFileEntry.erase(itor);
                    } else {
                        ++itor;
                    }
                }
                if(!tmpMapFileEntry.isEmpty()) {
                    for (QString strFullPath: tmpMapFileEntry.keys()) {
                        stArchiveData.mapFileEntry.insert(strFullPath, tmpMapFileEntry.value(strFullPath));
                    }
                }
                // 更新文件夹第一层的数据
                if (entry.strFullPath.endsWith(QLatin1Char('/')) && entry.strFullPath.count(QLatin1Char('/')) == 1) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry移除
                            stArchiveData.listRootEntry.removeAt(i);
                            strAlias = entry.strAlias + QDir::separator();
                            entry.strFullPath = strAlias;
                            entry.strFileName = strAlias;
//                            entry.strAlias.clear();
                            stArchiveData.listRootEntry.append(entry);
                            break;
                        }
                    }
                }
            } else { // 重命名文件
//                stArchiveData.qSize -= entry.qSize; // 更新压缩包内文件原始总大小
                stArchiveData.mapFileEntry.remove(entry.strFullPath); //在map中重命名该文件
                QString strPath = QFileInfo(entry.strFullPath).path();
                if(strPath == "." || strPath.isEmpty() || strPath.isNull()) {
                    strAlias = entry.strAlias;
                } else {
                    strAlias = strPath + QDir::separator() + entry.strAlias;
                }
                FileEntry tmpEntry = entry;
                tmpEntry.strFullPath = strAlias;
                stArchiveData.mapFileEntry.insert(strAlias, tmpEntry);
                // 更新文件夹第一层的数据
                if (!entry.strFullPath.contains(QLatin1Char('/'))) {
                    for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                        if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一次层数据中找到entry重命名
                            stArchiveData.listRootEntry.removeAt(i);
                            strAlias = entry.strAlias;
                            entry.strFullPath = strAlias;
                            entry.strFileName = strAlias;
                            stArchiveData.listRootEntry.append(entry);
                            break;
                        }
                    }
                }
            }
        } else if (options.eType == UpdateOptions::Add) { // 追加，更新压缩包数据
            QString destinationPath = options.strParentPath; // 在压缩包中追加的路径
            QFileInfo file(entry.strFullPath); // 注意此时entry.strFullPath如果是是文件夹还没有'/'
            if (rootEntry.isEmpty()) { // 获取所有追加文件的父目录
                rootEntry = file.filePath().left(file.filePath().size() - file.fileName().size());
            }

            entry.strFullPath = destinationPath + entry.strFullPath.remove(rootEntry); // entry在压缩包中的全路径
            if (file.isDir()) { // 文件夹
                entry.strFullPath = entry.strFullPath + QDir::separator(); // 手动添加'/'
                //entry.qSize = QDir(entry.strFullPath).entryInfoList().count(); // 获取文件夹大小为遍历文件夹获取文件夹下子文件的数目
            } else {
//                entry.qSize = file.size(); // 文件大小
                // 更新压缩包内文件原始总大小
                stArchiveData.qSize -= stArchiveData.mapFileEntry.value(entry.strFullPath).qSize;
                stArchiveData.qSize += entry.qSize;
            }

            // 判断是否追加到第一层数据
            if (destinationPath.isEmpty() && ((entry.strFullPath.count('/') == 1 && entry.strFullPath.endsWith('/')) || entry.strFullPath.count('/') == 0)) {
                for (int i = 0; i < stArchiveData.listRootEntry.count(); i++) {
                    if (stArchiveData.listRootEntry.at(i).strFullPath == entry.strFullPath) { // 在第一层数据中找到entry，不添加数据
                        stArchiveData.listRootEntry.removeAt(i);
                        break;
                    }
                }

                // 在第一层数据中没有找到entry，在第一层数据中添加entry
                stArchiveData.listRootEntry.push_back(entry);
            }

            // 更新压缩包map
            stArchiveData.mapFileEntry.insert(entry.strFullPath, entry);

        }
    }

    stArchiveData.qComressSize = QFileInfo(m_strArchiveName).size(); // 更新压缩包大小

    return PFT_Nomral;
}

void LibarchivePlugin::pauseOperation()
{
    m_bPause = true;
}

void LibarchivePlugin::continueOperation()
{
    m_bPause = false;
}

bool LibarchivePlugin::doKill()
{
    m_bPause = false;
    return false;
}

bool LibarchivePlugin::initializeReader()
{
    m_archiveReader.reset(archive_read_new());

    if (!(m_archiveReader.data())) {
        emit error(("The archive reader could not be initialized."));
        return false;
    }

    if (archive_read_support_filter_all(m_archiveReader.data()) != ARCHIVE_OK) {
        return false;
    }

    if (archive_read_support_format_all(m_archiveReader.data()) != ARCHIVE_OK) {
        return false;
    }

    if (archive_read_open_filename(m_archiveReader.data(), QFile::encodeName(m_strArchiveName).constData(), 10240) != ARCHIVE_OK) {
        emit error(("Archive corrupted or insufficient permissions."));
        return false;
    }

    return true;
}

void LibarchivePlugin::copyDataFromSource(struct archive *source, struct archive *dest, const qlonglong &totalSize)
{
    char buff[10240]; //缓存大小
    auto readBytes = archive_read_data(source, buff, sizeof(buff)); //读压缩包数据到buff

    while (readBytes > 0 && !QThread::currentThread()->isInterruptionRequested()) {
        if (m_bPause) { //解压暂停
            sleep(1);
//            qInfo() << "pause";
            continue;
        }

        archive_write_data(dest, buff, static_cast<size_t>(readBytes)); //写数据
        if (archive_errno(dest) != ARCHIVE_OK) {
            return;
        }

        // 获取解压过程中的压缩和解压缩文件总大小、计算并发送进度（archive_filter_bytes 0：uncomp   -1：comp）
        emit signalprogress(double(archive_filter_bytes(source, -1)) / totalSize * 100);

        readBytes = archive_read_data(source, buff, sizeof(buff));
    }
}

PluginFinishType LibarchivePlugin::list_New()
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();

    if (!initializeReader()) {
        return PFT_Error;
    }

    QString compMethod = convertCompressionName(QString::fromUtf8(archive_filter_name(m_archiveReader.data(), 0)));
    if (!compMethod.isEmpty()) {
//        emit compressionMethodFound(compMethod);
    }

    m_ArchiveEntryCount = 0;
    m_numberOfEntries = 0;
    stArchiveData.qComressSize = QFileInfo(m_strArchiveName).size(); // 压缩包大小

    struct archive_entry *aentry;
    int result = ARCHIVE_RETRY;

    while (!QThread::currentThread()->isInterruptionRequested() &&
            (result = archive_read_next_header(m_archiveReader.data(), &aentry)) == ARCHIVE_OK) {
//        if (!m_emitNoEntries) {
        emitEntryForIndex(aentry);
//        m_listIndex++;
//        }

        m_ArchiveEntryCount++;

        //The call to `archive_read_data_skip()` here is not actually necessary, since libarchive will invoke it automatically
        //if you request the next header without reading the data for the last entry.
//        archive_read_data_skip(m_archiveReader.data());
    }

    // 恢复原压缩包名，如果不恢复，tar.bz2、tar.lzma、tar.Z对tmpFilleName处理
    m_strArchiveName = m_strOldArchiveName;

    if (result != ARCHIVE_EOF && result != ARCHIVE_WARN) {
        return PFT_Error;
    }

    return (archive_read_close(m_archiveReader.data()) == ARCHIVE_OK) ? PFT_Nomral : PFT_Error;
}

QString LibarchivePlugin::convertCompressionName(const QString &method)
{
    if (method == QLatin1String("gzip")) {
        return QStringLiteral("GZip");
    } else if (method == QLatin1String("bzip2")) {
        return QStringLiteral("BZip2");
    } else if (method == QLatin1String("xz")) {
        return QStringLiteral("XZ");
    } else if (method == QLatin1String("compress (.Z)")) {
        return QStringLiteral("Compress");
    } else if (method == QLatin1String("lrzip")) {
        return QStringLiteral("LRZip");
    } else if (method == QLatin1String("lzip")) {
        return QStringLiteral("LZip");
    } else if (method == QLatin1String("lz4")) {
        return QStringLiteral("LZ4");
    } else if (method == QLatin1String("lzop")) {
        return QStringLiteral("lzop");
    } else if (method == QLatin1String("lzma")) {
        return QStringLiteral("LZMA");
    } else if (method == QLatin1String("zstd")) {
        return QStringLiteral("Zstandard");
    }
    return QString();
}

void LibarchivePlugin::emitEntryForIndex(archive_entry *aentry)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    FileEntry m_archiveEntryStat; //压缩包内文件数据
    // 文件名
    const char *name = archive_entry_pathname(aentry);
    QString entryName  = m_common->trans2uft8(name, m_mapCode[QString(name)]);

    // Some archive types e.g. AppImage prepend all entries with "./" so remove this part.
    // 移除"./"开头的，例如rpm包
    if (entryName.startsWith(QLatin1String("./"))) {
        entryName.remove(0, 2);
    }
    if (entryName.isEmpty()) {
        return;
    }
    // 是否为文件夹
    m_archiveEntryStat.isDirectory = S_ISDIR(archive_entry_mode(aentry));

    if(m_archiveEntryStat.isDirectory && !entryName.endsWith(QLatin1String("/"))){
        entryName += QLatin1String("/");
    }
    m_archiveEntryStat.strFullPath = entryName;

    // 文件名
    const QStringList pieces = m_archiveEntryStat.strFullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    m_archiveEntryStat.strFileName = pieces.last();

    // 文件真实大小（文件夹显示项）
    if (m_archiveEntryStat.isDirectory) {
        m_archiveEntryStat.qSize = 0;
    } else {
        m_archiveEntryStat.qSize = static_cast<qlonglong>(archive_entry_size(aentry));
    }

    // 文件最后修改时间
    m_archiveEntryStat.uLastModifiedTime = static_cast<uint>(archive_entry_mtime(aentry));

    handleEntry(m_archiveEntryStat);

    // 压缩包原始大小
    stArchiveData.qSize += m_archiveEntryStat.qSize;
    // 构建压缩包数据map
    stArchiveData.mapFileEntry.insert(m_archiveEntryStat.strFullPath, m_archiveEntryStat);
}

void LibarchivePlugin::deleteTempTarPkg(const QStringList &tars)
{
    if (tars.size() > 0) {
        for (const QString &tar : tars) {
            QFile::remove(tar);
        }
    }
}

int LibarchivePlugin::extractionFlags() const
{
    int result = ARCHIVE_EXTRACT_TIME;
    result |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;

    // TODO: Don't use arksettings here
    /*if ( ArkSettings::preservePerms() )
    {
        result &= ARCHIVE_EXTRACT_PERM;
    }

    if ( !ArkSettings::extractOverwrite() )
    {
        result &= ARCHIVE_EXTRACT_NO_OVERWRITE;
    }*/

    return result;
}

HandleWorkingDir::HandleWorkingDir(QString *oldWorkingDir)
    : m_oldWorkingDir(oldWorkingDir)
{

}

void HandleWorkingDir::change(const QString &newWorkingDir)
{
    *m_oldWorkingDir = QDir::currentPath();
    QDir::setCurrent(newWorkingDir);
}

HandleWorkingDir::~HandleWorkingDir()
{
    if (!m_oldWorkingDir->isEmpty() && QDir::setCurrent(*m_oldWorkingDir)) {
        m_oldWorkingDir->clear();
    }
}
