// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libminizipplugin.h"
#include "common.h"
#include "queries.h"
#include "datamanager.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <qplatformdefs.h>
#include <QDirIterator>
#include <QTimer>
#include <QDataStream>
#include <QTextCodec>

#define MAX_FILENAME 512

//#include <zlib.h>

LibminizipPluginFactory::LibminizipPluginFactory()
{
    registerPlugin<LibminizipPlugin>();
}

LibminizipPluginFactory::~LibminizipPluginFactory()
{

}



LibminizipPlugin::LibminizipPlugin(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{
    qInfo() << "LibminizipPlugin";
    m_ePlugintype = PT_Libminizip;

}

LibminizipPlugin::~LibminizipPlugin()
{

}

PluginFinishType LibminizipPlugin::list()
{
    // 打开压缩包文件
    unzFile zipfile = unzOpen(QFile::encodeName(m_strArchiveName).constData());
    if (nullptr == zipfile) {
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    // 获取压缩包数据信息
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
        unzClose(zipfile);
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    // 循环加载数据
    uLong i;
    for (i = 0; i < global_info.number_entry; ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        handleArchiveData(zipfile);

        if ((i + 1) < global_info.number_entry) {
            if (unzGoToNextFile(zipfile) != UNZ_OK) {
                qInfo() << ("cound not read next file\n");
            }
        }
    }

    unzClose(zipfile);

    return PFT_Nomral;
}

PluginFinishType LibminizipPlugin::testArchive()
{
    m_workStatus = WT_Test;
    return PFT_Nomral;
}

PluginFinishType LibminizipPlugin::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    Q_UNUSED(files)

    qInfo() << "解压缩数据";
    m_workStatus = WT_Extract;
    m_bOverwriteAll = false;        //是否全部覆盖
    m_bSkipAll = false;             // 是否全部跳过

    // 打开压缩包数据
    unzFile zipfile = unzOpen(QFile::encodeName(m_strArchiveName).constData());
    if (nullptr == zipfile) {
        m_eErrorType = ET_ArchiveDamaged ;
        return PFT_Error;
    }

    // 获取压缩包数据信息
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
        unzClose(zipfile);
        m_eErrorType = ET_ArchiveDamaged ;
        return PFT_Error;
    }

    uLong nofEntries = 0;
    // 执行解压操作
    if (options.bAllExtract) {  // 全部解压
        qlonglong qExtractSize = 0;
        nofEntries = global_info.number_entry;

        for (uLong i = 0; i < nofEntries; ++i) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            // 获取单个文件信息
            unz_file_info file_info;
            char filename[ MAX_FILENAME ];
            if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
                unzClose(zipfile);
                return PFT_Error;
            }

            QByteArray strCode;
            QString strFileName = m_common->trans2uft8(filename, strCode);

            // 解压单个文件
            m_eErrorType = extractEntry(zipfile, file_info, options, qExtractSize, strFileName);

            // 方便右键解压时提示是否有数据解压出来
            if (!options.bExistList && 0 == i) {
                FileEntry entry;
                entry.strFullPath = strFileName;
                DataManager::get_instance().archiveData().listRootEntry << entry;
            }

            if (ET_NoError == m_eErrorType) {  // 无错误，继续解压下一个文件
                if ((i + 1) < nofEntries) {
                    if (unzGoToNextFile(zipfile) != UNZ_OK) {
                        continue;
                    }
                }
            } else if (ET_UserCancelOpertion == m_eErrorType) {    // 用户取消，结束解压，返回结束标志
                unzClose(zipfile);
                return PFT_Cancel;
            } else {    // 处理错误
                unzClose(zipfile);
                return PFT_Error;
            }
        }
    } else {    // 部分提取
        qlonglong qExtractSize = 0;
        QStringList listFiles = getSelFiles(files);
        nofEntries = uLong(listFiles.size());
        uLong i = 0;

        if (listFiles.count() > 0) {
            QByteArray strCode;
            qInfo() << unzLocateFile(zipfile, m_common->trans2uft8(listFiles[0].toUtf8().constData(), strCode).toLatin1(), 0);
        }

        for (const QString &strFileName : listFiles) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            // 获取文件信息
            unz_file_info file_info;
            char filename[ MAX_FILENAME ];
            if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
                unzClose(zipfile);
                return PFT_Error;
            }

            QString strFileNameTemp = strFileName;

            // 解压单个文件
            m_eErrorType = extractEntry(zipfile, file_info, options, qExtractSize, strFileNameTemp);

            if (ET_NoError == m_eErrorType) {  // 无错误，继续解压下一个文件
                if ((++i) < nofEntries) {
                    QByteArray strCode;
                    int error = unzLocateFile(zipfile, m_common->trans2uft8(listFiles[int(i)].toUtf8().data(), strCode).toLatin1(), 0);
                    if (error != UNZ_OK) {
                        continue;
                    }
                }
            } else if (ET_UserCancelOpertion == m_eErrorType) {    // 用户取消，结束解压，返回结束标志
                unzClose(zipfile);
                return PFT_Cancel;
            } else {    // 处理错误
                unzClose(zipfile);
                return PFT_Error;
            }
        }
    }

    unzClose(zipfile);
    return PFT_Nomral;
}

void LibminizipPlugin::pauseOperation()
{
    m_bPause = true;
}

void LibminizipPlugin::continueOperation()
{
    m_bPause = false;
}

bool LibminizipPlugin::doKill()
{
    m_bPause = false;
    m_bCancel = true;
    return false;
}

bool LibminizipPlugin::handleArchiveData(unzFile zipfile)
{
    unz_file_info file_info;

    char filename[ MAX_FILENAME ];

    // 获取文件信息
    if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
        qInfo() << ("could not read file info\n");
        return false;
    }

    // 对文件名进行编码探测并转码
    QByteArray strCode;
    QString name = m_common->trans2uft8(filename, strCode);

    FileEntry entry;
    // 全路径
    entry.strFullPath = name;
    // 文件名
    const QStringList pieces = entry.strFullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    entry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
    // 是否是文件夹
    entry.isDirectory = name.endsWith(QDir::separator());
    // 文件大小
    if (!entry.isDirectory) {
        entry.qSize = qlonglong(file_info.uncompressed_size);
        DataManager::get_instance().archiveData().qSize += entry.qSize;
        DataManager::get_instance().archiveData().qComressSize += file_info.compressed_size;
    } else {
        entry.qSize = 0;
    }
    // 最后一次修改时间
    QDateTime datetime;
    datetime.setDate(QDate(int(file_info.tmu_date.tm_year), int(file_info.tmu_date.tm_mon + 1), int(file_info.tmu_date.tm_mday)));
    datetime.setTime(QTime(int(file_info.tmu_date.tm_hour), int(file_info.tmu_date.tm_min), int(file_info.tmu_date.tm_sec)));
    entry.uLastModifiedTime = uint(datetime.toTime_t());

    handleEntry(entry);

    // 存储总数据
    DataManager::get_instance().archiveData().mapFileEntry[name] = entry;

    return true;
}

ErrorType LibminizipPlugin::extractEntry(unzFile zipfile, unz_file_info file_info, const ExtractionOptions &options, qlonglong &qExtractSize, QString &strFileName)
{
    // 提取
    if (!options.strDestination.isEmpty()) {
        strFileName = strFileName.remove(0, options.strDestination.size());
    }

    emit signalCurFileName(strFileName);        // 发送当前正在解压的文件名

    bool bIsDirectory = strFileName.endsWith(QDir::separator());    // 是否为文件夹

    // 判断解压路径是否存在，不存在则创建文件夹
    if (QDir().exists(options.strTargetPath) == false)
        QDir().mkpath(options.strTargetPath);

    // 解压完整文件名（含路径）
    QString strDestFileName = options.strTargetPath + QDir::separator() + strFileName;
    QFile file(strDestFileName);

    if (bIsDirectory) {     // 文件夹
        QDir dir;
        dir.mkpath(strDestFileName);
    } else {        // 普通文件
        // 判断是否有同名文件
        if (file.exists()) {
            if (m_bSkipAll) {       // 全部跳过
                return ET_NoError;
            } else {
                if (!m_bOverwriteAll) {     // 若不是全部覆盖，单条处理

                    OverwriteQuery query(strDestFileName);

                    emit signalQuery(&query);
                    query.waitForResponse();

                    if (query.responseCancelled()) {
                        emit signalCancel();
                        return ET_UserCancelOpertion;
                    } else if (query.responseSkip()) {
                        return ET_NoError;
                    } else if (query.responseSkipAll()) {
                        m_bSkipAll = true;
                        return ET_NoError;
                    }  else if (query.responseOverwriteAll()) {
                        m_bOverwriteAll = true;
                    }
                }
            }
        }

        // 若文件存在且不是可写权限，重新创建一个文件
        if (file.exists() && !file.isWritable()) {
            file.remove();
            file.setFileName(strDestFileName);
            file.setPermissions(QFileDevice::WriteUser);
        }

        // 对文件路径做判断，防止特殊包未先解压出文件夹，导致解压失败
        if (QDir().exists(QFileInfo(strDestFileName).path()) == false)
            QDir().mkpath(QFileInfo(strDestFileName).path());

        // 以只写的方式打开待解压的文件
        if (file.open(QIODevice::WriteOnly) == false) {
            QList<QString> entryNameList = strDestFileName.split('/');
            foreach (auto &tmp, entryNameList) {
                // 判断文件名是否过长
                if (NAME_MAX < tmp.toLocal8Bit().length()) {
                    return ET_LongNameError;
                }
            }

            emit signalFileWriteErrorName(QFileInfo(file.fileName()).fileName());
            return ET_FileOpenError;
        }

        // 写文件
        QDataStream out(&file);
        unsigned kb = 1024;
        uLong sum = 0;
        char buf[1024];
        int writeSize = 0;

        int iError = unzOpenCurrentFile(zipfile);

        if (iError != UNZ_OK) {
            return ET_ArchiveDamaged ;
        }

        while (sum != file_info.uncompressed_size) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }
            if (m_bPause) { //解压暂停
                sleep(1);
//                qInfo() << "pause";
                continue;
            }

            const auto readBytes = unzReadCurrentFile(zipfile, buf, kb);
            if (readBytes < 0) {
                file.close();
                emit signalFileWriteErrorName(QFileInfo(file.fileName()).fileName());
                return ET_FileWriteError;
            }

            if (out.writeRawData(buf, readBytes) != readBytes) {
                file.close();

                if (isInsufficientDiskSpace(options.strTargetPath, static_cast<qint64>(readBytes))) {  // 小于readBytes作为磁盘空间不足的判断标准
                    return ET_InsufficientDiskSpace;
                } else {
                    emit signalFileWriteErrorName(QFileInfo(file.fileName()).fileName());
                    return ET_FileWriteError;
                }
            }

            sum += uLong(readBytes);
            writeSize += readBytes;

            // 计算进度并显示（右键快捷解压使用压缩包大小，计算比例）
            if (!options.bExistList) {
                qExtractSize += readBytes * (double(file_info.compressed_size) / file_info.uncompressed_size);
                emit signalprogress((double(qExtractSize)) / options.qComressSize * 100);
            } else {
                qExtractSize += readBytes;
                emit signalprogress((double(qExtractSize)) / options.qSize * 100);
            }
        }

        file.close();
    }

    unzCloseCurrentFile(zipfile);
    return ET_NoError;
}

QStringList LibminizipPlugin::getSelFiles(const QList<FileEntry> &files)
{
    QStringList listSel;
    ArchiveData stArchiveData = DataManager::get_instance().archiveData();

    // 筛选待提取文件/文件夹索引
    for (FileEntry entry : files) {
        auto iter = stArchiveData.mapFileEntry.find(entry.strFullPath);
        for (; iter != stArchiveData.mapFileEntry.end();) {
            if (!iter.key().startsWith(entry.strFullPath)) {
                break;
            } else {
                listSel << iter.value().strFullPath;  // 保存文件名
                ++iter;

                // 如果文件夹，直接跳过
                if (!entry.strFullPath.endsWith(QDir::separator())) {
                    break;
                }
            }
        }
    }

    return listSel;
}
