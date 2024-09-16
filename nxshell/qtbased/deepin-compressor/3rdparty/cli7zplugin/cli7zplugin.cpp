/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2009-2011 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 * Copyright (c) 2016 Vladyslav Batyrenko <mvlabat@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include "cli7zplugin.h"
#include "datamanager.h"

#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QDateTime>
#include <linux/limits.h>

Cli7zPluginFactory::Cli7zPluginFactory()
{
    registerPlugin<Cli7zPlugin>();
}

Cli7zPluginFactory::~Cli7zPluginFactory()
{

}

Cli7zPlugin::Cli7zPlugin(QObject *parent, const QVariantList &args)
    : CliInterface(parent, args)
{
    m_archiveType = ArchiveType7z;

    setupCliProperties();
}

Cli7zPlugin::~Cli7zPlugin()
{

}

void Cli7zPlugin::setupCliProperties()
{
    m_cliProps->setProperty("captureProgress", false);

    m_cliProps->setProperty("addProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("addSwitch", QStringList{QStringLiteral("a"),
                                                     QStringLiteral("-snl")});

    m_cliProps->setProperty("deleteProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("deleteSwitch", QStringLiteral("d"));

    m_cliProps->setProperty("extractProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("extractSwitch", QStringList{QStringLiteral("x")});
    m_cliProps->setProperty("progressarg", QStringList{QStringLiteral("-bsp1")});
    m_cliProps->setProperty("extractSwitchNoPreserve", QStringList{QStringLiteral("e")});

    m_cliProps->setProperty("listProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("listSwitch", QStringList{QStringLiteral("l"),
                                                      QStringLiteral("-slt")});

    m_cliProps->setProperty("moveProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("moveSwitch", QStringLiteral("rn"));

    m_cliProps->setProperty("testProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("testSwitch", QStringLiteral("t"));

    m_cliProps->setProperty("passwordSwitch", QStringList{QStringLiteral("-p$Password")});
    m_cliProps->setProperty("passwordSwitchHeaderEnc", QStringList{QStringLiteral("-p$Password"),
                                                                   QStringLiteral("-mhe=on")});
    m_cliProps->setProperty("compressionLevelSwitch", QStringLiteral("-mx=$CompressionLevel"));
    m_cliProps->setProperty("compressionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QStringLiteral("-m0=$CompressionMethod")},
        {QStringLiteral("application/zip"), QStringLiteral("-mm=$CompressionMethod")}
    });

    m_cliProps->setProperty("encryptionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QString()},
        {QStringLiteral("application/zip"), QStringLiteral("-mem=$EncryptionMethod")}
    });

    m_cliProps->setProperty("multiVolumeSwitch", QStringLiteral("-v$VolumeSizek"));
    m_cliProps->setProperty("testPassedPatterns", QStringList{QStringLiteral("^Everything is Ok$")});
    m_cliProps->setProperty("fileExistsFileNameRegExp", QStringList{QStringLiteral("^file \\./(.*)$"),
                                                                    QStringLiteral("^  Path:     \\./(.*)$")});
    m_cliProps->setProperty("fileExistsInput", QStringList{QStringLiteral("Y"),   //Overwrite
                                                           QStringLiteral("N"),   //Skip
                                                           QStringLiteral("A"),   //Overwrite all
                                                           QStringLiteral("S"),   //Autoskip
                                                           QStringLiteral("Q")}); //Cancel
    m_cliProps->setProperty("multiVolumeSuffix", QStringList{QStringLiteral("$Suffix.001")});
}

bool Cli7zPlugin::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed):"));
}

bool Cli7zPlugin::isWrongPasswordMsg(const QString &line)
{
    return line.contains(QLatin1String("Wrong password"));
}

bool Cli7zPlugin::isCorruptArchiveMsg(const QString &line)
{
    return (line == QLatin1String("Unexpected end of archive") || // 1.zip.001... 分卷包缺失
            line.startsWith(QLatin1String("ERROR = Missing volume :")) || // 1.zip 1.z01... 分卷包缺失
            line == QLatin1String("Headers Error"));
}

bool Cli7zPlugin::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device"));
}

bool Cli7zPlugin::isFileExistsMsg(const QString &line)
{
    return (line == QLatin1String("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ") ||
            line == QLatin1String("? (Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "));
}

bool Cli7zPlugin::isFileExistsFileName(const QString &line)
{
    return (line.startsWith(QLatin1String("file ./")) ||
            line.startsWith(QLatin1String("  Path:     ./")));
}

bool Cli7zPlugin::isMultiPasswordPrompt(const QString &line)
{
    Q_UNUSED(line);
    return false;
}

bool Cli7zPlugin::isOpenFileFailed(const QString &line)
{
    // 文件名过长情况下会输出
    return line.contains("ERROR: Can not open output file :");
}

void Cli7zPlugin::killProcess(bool emitFinished)
{
    Q_UNUSED(emitFinished);
    if (!m_process) {
        return;
    }

    if (!m_childProcessId.empty()) {
        for (int i = m_childProcessId.size() - 1; i >= 0; i--) {
            if (m_childProcessId[i] > 0) {
                kill(static_cast<__pid_t>(m_childProcessId[i]), SIGKILL);
            }
        }
    }

    qint64 processID = m_process->processId();
    // 结束进程，先continue再kill，保证能删除缓存文件
    // 使用SIGTERM，7z命令可以自动删除缓文件
    if (processID > 0) {
        kill(static_cast<__pid_t>(processID), SIGCONT);
        if (m_bWaitingPassword || m_workStatus == WT_Extract) {
            kill(static_cast<__pid_t>(processID), SIGKILL); // 等待输入密码时只有SIGKILL才能结束
        } else {
            kill(static_cast<__pid_t>(processID), SIGTERM);
        }
    }

    m_isProcessKilled = true;
}

bool Cli7zPlugin::readListLine(const QString &line)
{
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();

    static const QLatin1String archiveInfoDelimiter1("--"); // 7z 9.13+  分隔符后面是压缩包信息
    static const QLatin1String archiveInfoDelimiter2("----"); // 7z 9.04 分隔符后面是压缩包信息
    static const QLatin1String entryInfoDelimiter("----------"); // 分隔符后面是内部压缩文件信息

    // 加载时7z分卷文件不完整的情况
    if (line.startsWith(QLatin1String("Open ERROR: Can not open the file as [7z] archive"))) {
        m_eErrorType = ET_ArchiveDamaged;
        m_finishType = PFT_Error;
        return false;
    }

    const QRegularExpression rxVersionLine(QStringLiteral("^p7zip Version ([\\d\\.]+) .*$"));
    QRegularExpressionMatch matchVersion;

    switch (m_parseState) {
    case ParseStateTitle:
        matchVersion = rxVersionLine.match(line);
        if (matchVersion.hasMatch()) {  // 7z信息读取完，开始读压缩包相关信息
            m_parseState = ParseStateHeader;
        }
        break;
    case ParseStateHeader:
        if ((line == archiveInfoDelimiter1) || (line == archiveInfoDelimiter2)) {  // 开始读压缩包整体信息
            // 压缩包大小
            stArchiveData.qComressSize = line.mid(14).trimmed().toLongLong();
            m_parseState = ParseStateArchiveInformation;
        }
        break;
    case ParseStateArchiveInformation:
        if (line == entryInfoDelimiter) {  // 分隔压缩包信息和内部压缩文件信息
            m_parseState = ParseStateEntryInformation;
        } else if (line.startsWith(QLatin1String("Type = "))) {  // 获取压缩文件类型
            const QString type = line.mid(7).trimmed();
            if (type == QLatin1String("7z")) {
                m_archiveType = ArchiveType7z;
            } else if (type == QLatin1String("bzip2")) {
                m_archiveType = ArchiveTypeBZip2;
            } else if (type == QLatin1String("gzip")) {
                m_archiveType = ArchiveTypeGZip;
            } else if (type == QLatin1String("xz")) {
                m_archiveType = ArchiveTypeXz;
            } else if (type == QLatin1String("tar")) {
                m_archiveType = ArchiveTypeTar;
            } else if (type == QLatin1String("zip")) {
                m_archiveType = ArchiveTypeZip;
            } else if (type == QLatin1String("Rar")) {
                m_archiveType = ArchiveTypeRar;
            } else if (type == QLatin1String("Split")) {
//                setMultiVolume(true);
            } else if (type == QLatin1String("Udf")) {
                m_archiveType = ArchiveTypeUdf;
            } else if (type == QLatin1String("Iso")) {
                m_archiveType = ArchiveTypeIso;
            } else {
                // Should not happen
                return false;
            }
        }
        break;
    case ParseStateEntryInformation:  // 读压缩包内文件具体信息
        if (line.startsWith(QLatin1String("Path = "))) {
            // 文件在压缩包内绝对路径路径
            const QString entryFilename = QDir::fromNativeSeparators(line.mid(7).trimmed());
            m_fileEntry.strFullPath = entryFilename;

            // 文件名称
            const QStringList pieces = entryFilename.split(QLatin1Char('/'), QString::SkipEmptyParts);
            m_fileEntry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
        } else if (line.startsWith(QLatin1String("Size = "))) {
            // 单文件实际大小
            m_fileEntry.qSize = line.midRef(7).trimmed().toLongLong();

            // 压缩包内所有文件总大小
            stArchiveData.qSize += m_fileEntry.qSize;
        } else if (line.startsWith(QLatin1String("Modified = "))) {
            // 文件最后修改时间
            m_fileEntry.uLastModifiedTime = QDateTime::fromString(line.mid(11).trimmed(),
                                                                  QStringLiteral("yyyy-MM-dd hh:mm:ss")).toTime_t();
            if (ArchiveTypeIso == m_archiveType || ArchiveTypeUdf == m_archiveType) { // 读取的压缩包是iso、udf文件，读到Modified的时候已经结束了
//                QString name = m_fileEntry.strFullPath;

                handleEntry(m_fileEntry);
                // 获取第一层数据
//                if ((!name.contains(QDir::separator()) && name.size() > 0) || (name.count(QDir::separator()) == 1 && name.endsWith(QDir::separator()))) {
//                    stArchiveData.listRootEntry.push_back(m_fileEntry);
//                }

                // 存储总数据
                stArchiveData.mapFileEntry.insert(m_fileEntry.strFullPath, m_fileEntry);

                // clear m_fileEntry
                m_fileEntry.reset();
            }
        } else if (line.startsWith(QLatin1String("Attributes = "))) {  // 文件权限
            const QStringRef attributes = line.midRef(13).trimmed();

            // D开头为文件夹
            if (attributes.startsWith(QLatin1Char('D'))) {
                m_fileEntry.isDirectory = true;
                if (!m_fileEntry.strFullPath.endsWith(QLatin1Char('/'))) {  // 如果是文件夹且路径最后没有分隔符，手动添加
                    m_fileEntry.strFullPath = QString(m_fileEntry.strFullPath + QLatin1Char('/'));
                }
            } else {
                // 不是文件夹
                m_fileEntry.isDirectory = false;
            }
        } else if (line.startsWith(QLatin1String("Block = ")) || line.startsWith(QLatin1String("Version = "))) {  // 文件的最后一行信息
//            QString name = m_fileEntry.strFullPath;

            handleEntry(m_fileEntry);
            // 获取第一层数据
//            if ((!name.contains(QDir::separator()) && name.size() > 0) || (name.count(QDir::separator()) == 1 && name.endsWith(QDir::separator()))) {
//                stArchiveData.listRootEntry.push_back(m_fileEntry);
//            }

            // 存储总数据
            stArchiveData.mapFileEntry.insert(m_fileEntry.strFullPath, m_fileEntry);

            // clear m_fileEntry
            m_fileEntry.reset();
        } else if (line.startsWith(QLatin1String("Folder = "))) {  // 是否文件夹
            const QStringRef folder = line.midRef(9, 1);

            // "+" 为文件夹
            if (0 == folder.compare(QLatin1String("+"))) {
                m_fileEntry.isDirectory = true;
                if (!m_fileEntry.strFullPath.endsWith(QLatin1Char('/'))) {  // 如果是文件夹且路径最后没有分隔符，手动添加
                    m_fileEntry.strFullPath = QString(m_fileEntry.strFullPath + QLatin1Char('/'));
                }
            } else {
                // 不是文件夹
                m_fileEntry.isDirectory = false;
            }
        }
        break;
    }

    return true;
}

bool Cli7zPlugin::handleLine(const QString &line, WorkType workStatus)
{
    if (isPasswordPrompt(line)) {  // 提示需要输入密码
        if (workStatus == WT_List) {
            DataManager::get_instance().archiveData().isListEncrypted = true; // 列表加密文件
        }

        m_eErrorType = ET_NeedPassword;
        if (handlePassword() == PFT_Cancel) {
            m_finishType = PFT_Cancel;
            m_bWaitingPassword = true;
            return false;
        }
        m_bWaitingPassword = false;
        return true;
    }

    if (isWrongPasswordMsg(line)) {  // 提示密码错误
        m_eErrorType = ET_WrongPassword;
        if (workStatus != WT_Delete) { // 区分删除操作密码错误的处理
            m_finishType = PFT_Error;
            return false;
        }
    }

    if (isDiskFullMsg(line)) {
//        m_eErrorType = ;
        return false;
    }

    if (workStatus == WT_List) {
        if (isCorruptArchiveMsg(line)) {
            m_isCorruptArchive = true;
        }

        return readListLine(line);   // 加载压缩文件，处理命令行内容
    } else if (workStatus == WT_Add || workStatus == WT_Extract || workStatus == WT_Delete) { // 压缩、解压、删除进度
        if (isCorruptArchiveMsg(line) && workStatus == WT_Extract) {  // 解压分卷不完整的情况下出现
            m_eErrorType = ET_MissingVolume;
            m_finishType = PFT_Error;
//            return false; // 这里不return，直到解压完成
        }

        // 压缩包内没有数据
        if (isNoFilesArchive(line) && workStatus == WT_Extract && m_isEmptyArchive == false) {
            m_isEmptyArchive = true;
        }

        if (handleFileExists(line) && workStatus == WT_Extract) {  // 判断解压是否存在同名文件
            return true;
        }

        //-------文件名过长-------
        if (workStatus == WT_Extract && isOpenFileFailed(line)) {
            QByteArray diskPath = line.toLocal8Bit();
            diskPath = diskPath.mid(diskPath.indexOf(':', diskPath.indexOf(':', line.indexOf(':') + 1) + 1) + 2);
            QList<QByteArray> entryNameList = diskPath.split('/');
            foreach (auto &tmp, entryNameList) {
                // 判断文件名是否过长
                if (NAME_MAX < tmp.length()) {
                    m_finishType = PFT_Error;
                    m_eErrorType = ET_LongNameError;
                    emit signalCurFileName(diskPath);
                    return false;
                }
            }
        }

        // ------磁盘空间不足------
        // 解压：当读取到命令行最后一行 "ERROR: E_FAIL" 的时候，检测磁盘空间是否不足
        // 压缩：System ERROR:28 或 System ERROR:17（跟压缩方式、剩余空间、原文件大小有关。。）
        if (workStatus == WT_Add && line.startsWith("System ERROR:")
                && (ENOSPC == line.mid(line.lastIndexOf(':') + 1).toInt() || EEXIST == line.mid(line.lastIndexOf(':') + 1).toInt())) {
            m_finishType = PFT_Error;
            if(EEXIST == line.mid(line.lastIndexOf(':') + 1).toInt()) {
                m_eErrorType = ET_ExistVolume;
            } else {
                m_eErrorType = ET_InsufficientDiskSpace;
            }
            return false;
        }
        if ((workStatus == WT_Add && line.startsWith("E_FAIL")) || (workStatus == WT_Extract && line.startsWith("ERROR: E_FAIL"))) {
            m_finishType = PFT_Error;

            if (workStatus == WT_Extract) {
                if (isInsufficientDiskSpace(getTargetPath(), 10 * 1024 * 1024)) { // 暂取小于10M作为磁盘空间不足的判断标准
                    m_eErrorType = ET_InsufficientDiskSpace;
                }
            }

            return false;
        }

        // ------区分删除操作密码错误的处理------ 当读取到命令行最后一行 "E_FAIL" 的时候再killprocess，否则进程是CrashExit
        if (workStatus == WT_Delete && m_eErrorType == ET_WrongPassword && line.startsWith("E_FAIL")) {
            m_finishType = PFT_Error;
            return false;
        }

        // 文件名过长
        if (workStatus == WT_Add && line.contains("MAX_PATHNAME_LEN")) {
            m_finishType = PFT_Error;
            m_eErrorType = ET_LongNameError;
            return false;
        }

        handleProgress(line);
    }

    return true;
}

bool Cli7zPlugin::isNoFilesArchive(const QString &line)
{
    return line.startsWith(QLatin1String("No files to process"));
}
