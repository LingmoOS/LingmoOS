/*
* ark -- archiver for the KDE project
*
* Copyright (C) 2009 Harald Hvaal <haraldhv@stud.ntnu.no>
* Copyright (C) 2010-2011,2014 Raphael Kubo da Costa <rakuco@FreeBSD.org>
* Copyright (C) 2015-2016 Ragnar Thomsen <rthomsen6@gmail.com>
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

#include "clirarplugin.h"
#include "datamanager.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <linux/limits.h>

CliRarPluginFactory::CliRarPluginFactory()
{
    registerPlugin<CliRarPlugin>();
}

CliRarPluginFactory::~CliRarPluginFactory()
{

}

CliRarPlugin::CliRarPlugin(QObject *parent, const QVariantList &args)
    : CliInterface(parent, args)
{
    // 如果rar压缩包含有注释信息，没有标志出comment，直接显示注释内容，需要根据空的line进行解析
    setListEmptyLines(true);
    setupCliProperties();
}

CliRarPlugin::~CliRarPlugin()
{

}

void CliRarPlugin::setupCliProperties()
{
    m_cliProps->setProperty("captureProgress", true);

    m_cliProps->setProperty("addProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("addSwitch", QStringList({QStringLiteral("a")}));

    m_cliProps->setProperty("deleteProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("deleteSwitch", QStringLiteral("d"));

    m_cliProps->setProperty("extractProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("extractSwitch", QStringList{QStringLiteral("x"),
                                                         QStringLiteral("-kb")/*,
                                                         QStringLiteral("-p-")*/});
    m_cliProps->setProperty("extractSwitchNoPreserve", QStringList{QStringLiteral("e"),
                                                                   QStringLiteral("-kb")/*,
                                                                   QStringLiteral("-p-")*/});

    m_cliProps->setProperty("listProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("listSwitch", QStringList{QStringLiteral("vt"),
                                                      QStringLiteral("-v")});

    m_cliProps->setProperty("moveProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("moveSwitch", QStringLiteral("rn"));

    m_cliProps->setProperty("testProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("testSwitch", QStringLiteral("t"));

    m_cliProps->setProperty("commentSwitch", QStringList{QStringLiteral("c"),
                                                         QStringLiteral("-z$CommentFile")});

    m_cliProps->setProperty("passwordSwitch", QStringList{QStringLiteral("-p$Password")});
    m_cliProps->setProperty("passwordSwitchHeaderEnc", QStringList{QStringLiteral("-hp$Password")});

    m_cliProps->setProperty("compressionLevelSwitch", QStringLiteral("-m$CompressionLevel"));
    m_cliProps->setProperty("compressionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/vnd.rar"), QStringLiteral("-ma$CompressionMethod")},
        {QStringLiteral("application/x-rar"), QStringLiteral("-ma$CompressionMethod")}
    });

    m_cliProps->setProperty("multiVolumeSwitch", QStringLiteral("-v$VolumeSizek"));

    m_cliProps->setProperty("testPassedPatterns", QStringList{QStringLiteral("^All OK$")});
    m_cliProps->setProperty("fileExistsFileNameRegExp", QStringList{QStringLiteral("^(.+) already exists. Overwrite it"),  // unrar 3 & 4
                                                                    QStringLiteral("^Would you like to replace the existing file (.+)$")}); // unrar 5
    m_cliProps->setProperty("fileExistsInput", QStringList{QStringLiteral("Y"),   //Overwrite
                                                           QStringLiteral("N"),   //Skip
                                                           QStringLiteral("A"),   //Overwrite all
                                                           QStringLiteral("E"),   //Autoskip
                                                           QStringLiteral("Q")}); //Cancel

    // rar will sometimes create multi-volume archives where first volume is
    // called name.part1.rar and other times name.part01.rar.
    m_cliProps->setProperty("multiVolumeSuffix", QStringList{QStringLiteral("part01.$Suffix"),
                                                             QStringLiteral("part1.$Suffix")});
}

bool CliRarPlugin::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed) for ")) && line.endsWith(": ");
}

bool CliRarPlugin::isWrongPasswordMsg(const QString &line)
{
    return line.startsWith(QLatin1String("The specified password is incorrect"))
           || line.startsWith(QLatin1String("Checksum error in the encrypted file"))
           || line.contains(QLatin1String("password incorrect"))
           /*|| line.contains(QLatin1String("wrong password"))*/;
}

bool CliRarPlugin::isCorruptArchiveMsg(const QString &line)
{
    return (line == QLatin1String("Unexpected end of archive") || line.contains(QLatin1String("the file header is corrupt"))
            || line.endsWith(QLatin1String("checksum error")));
}

bool CliRarPlugin::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device")) ||
           (line.startsWith(QLatin1String("Write error in the file"))
            && line.endsWith(QLatin1String("[R]etry, [A]bort ")));
}

bool CliRarPlugin::isFileExistsMsg(const QString &line)
{
    return (line == QLatin1String("[Y]es, [N]o, [A]ll, n[E]ver, [R]ename, [Q]uit "));
}

bool CliRarPlugin::isFileExistsFileName(const QString &line)
{
    return (line.startsWith(QLatin1String("Would you like to replace the existing file ")));
}

bool CliRarPlugin::isMultiPasswordPrompt(const QString &line)
{
    return line.contains("use current password ? [Y]es, [N]o, [A]ll");
}

bool CliRarPlugin::isOpenFileFailed(const QString &line)
{
    // 文件名过长情况下会输出
    return line.startsWith("Cannot create ");
}

void CliRarPlugin::killProcess(bool emitFinished)
{
    Q_UNUSED(emitFinished);
    if (!m_process) {
        return;
    }

    qint64 processID = m_process->processId();
    // 结束进程，先continue再kill，保证能删除缓存文件
    // 使用SIGTERM不能立马结束，需使用SIGKILL强制结束
    if (processID > 0) {
        kill(static_cast<__pid_t>(processID), SIGCONT);
        kill(static_cast<__pid_t>(processID), SIGKILL);
    }

    m_isProcessKilled = true;
}

bool CliRarPlugin::readListLine(const QString &line)
{
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();

    QRegularExpression rxVersionLine(QStringLiteral("^UNRAR (\\d+\\.\\d+)( beta \\d)? .*$"));
    QRegularExpressionMatch matchVersion = rxVersionLine.match(line);

    switch (m_parseState) {
    case ParseStateHeader:
    case ParseStateTitle:
        if (matchVersion.hasMatch()) {  // unrar信息读取完，开始读压缩包相关信息
            m_parseState = ParseStateArchiveInformation;
        }
        break;
    case ParseStateArchiveInformation:  // // 读取压缩包信息
        if (line.startsWith(QLatin1String("Archive:"))) { // 压缩包全路径
            // 压缩包注释信息
            m_comment = m_comment.trimmed();
            stArchiveData.strComment = m_comment;

            // 通过压缩包全路径，获取压缩包大小
            QFileInfo file(line.mid(9).trimmed());
            stArchiveData.qComressSize = file.size();
        } else if (line.startsWith(QLatin1String("Details:"))) {
            // 读完压缩包信息，开始读内部文件信息
            m_parseState = ParseStateEntryInformation;
        } else {
            // 读取注释信息
            m_comment.append(line + QLatin1Char('\n'));
        }
        break;
    case ParseStateEntryInformation:  // 读压缩包内文件具体信息
        // ':'左侧字符串
        QString parseLineLeft = line.section(QLatin1Char(':'), 0, 0).trimmed();
        // ':'右侧字符串
        QString parseLineRight = line.section(QLatin1Char(':'), -1).trimmed();
        if (parseLineLeft == QLatin1String("Name")) {
            // 文件在压缩包内绝对路径路径
            m_fileEntry.strFullPath = parseLineRight;

            // 文件名称
            const QStringList pieces = m_fileEntry.strFullPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
            m_fileEntry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
        } else if (parseLineLeft == QLatin1String("Type")) {
            if (parseLineRight == QLatin1String("Directory")) {
                // 是文件夹
                m_fileEntry.isDirectory = true;

                // 如果是文件夹且路径最后没有分隔符，手动添加
                if (!m_fileEntry.strFullPath.endsWith(QLatin1Char('/'))) {
                    m_fileEntry.strFullPath = m_fileEntry.strFullPath + QLatin1Char('/');
                }
            } else {
                // 不是文件夹
                m_fileEntry.isDirectory = false;
            }
        } else if (parseLineLeft == QLatin1String("Size")) {
            // 单文件实际大小
            m_fileEntry.qSize = parseLineRight.toLongLong();

            // 压缩包内所有文件总大小
            stArchiveData.qSize += m_fileEntry.qSize;
        } else if (parseLineLeft == QLatin1String("mtime")) {
            QString time = line.left((line.length() - 10));
            // 文件最后修改时间
            m_fileEntry.uLastModifiedTime = QDateTime::fromString(time.right(time.length() - 14),
                                                                  QStringLiteral("yyyy-MM-dd hh:mm:ss")).toTime_t();

            QString name = m_fileEntry.strFullPath;

            handleEntry(m_fileEntry);
            // 获取第一层数据
//            if (!name.contains(QDir::separator()) || (name.count(QDir::separator()) == 1 && name.endsWith(QDir::separator()))) {
//                stArchiveData.listRootEntry.push_back(m_fileEntry);
//            }

            // 存储总数据
            stArchiveData.mapFileEntry.insert(name, m_fileEntry);

            // clear m_fileEntry
            m_fileEntry.reset();
        }

        break;
    }

    return true;
}

bool CliRarPlugin::handleLine(const QString &line, WorkType workStatus)
{
    if (isPasswordPrompt(line)) {  // 提示需要输入密码 提示密码错误
        if (workStatus == WT_List) {
            m_strEncryptedFileName = m_strArchiveName; // list时文件名为压缩包名
            DataManager::get_instance().archiveData().isListEncrypted = true; // 列表加密文件
        } else { // 解压时的文件名通过解析命令行获取
            m_strEncryptedFileName = line.right(line.length() - 40).remove(": ");
        }

        m_eErrorType = ET_NeedPassword;
        if (handlePassword() == PFT_Cancel) {
            m_finishType = PFT_Cancel;
            return false;
        }

        return true;
    }

    if (isWrongPasswordMsg(line)) {  // 提示密码错误
        // RAR4密码错误直接结束
        if (line.startsWith(QLatin1String("Checksum error in the encrypted file"))) {
            m_eErrorType = ET_WrongPassword;
            m_finishType = PFT_Error;
            return false;
        } else { // RAR5密码错误反复输入新密码
            m_eErrorType = ET_WrongPassword;
            return true;
        }
    }

    if (workStatus == WT_List) {
        return readListLine(line);   // 加载压缩文件，处理命令行内容
    } else if (workStatus == WT_Extract) { // 解压进度
        if (isMultiPasswordPrompt(line)) { // rar多密码文件解压提示是否使用上一次输入的密码
            writeToProcess(QString("Y" + QLatin1Char('\n')).toLocal8Bit()); // 默认选择Y，使用上一次的密码
        }

        if (handleFileExists(line)) {  // 判断解压是否存在同名文件
            return true;
        }

        //-------文件名过长-------
        if (isOpenFileFailed(line)) {
            QByteArray diskPath = line.toLocal8Bit();
            diskPath = diskPath.mid(QString("Cannot create ").length());
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
        if (isDiskFullMsg(line)) {
            if (line.startsWith(QLatin1String("Write error in the file"))) {
                writeToProcess(QString("A" + QLatin1Char('\n')).toLocal8Bit()); // 默认选择A，终止解压
            }
            if (isInsufficientDiskSpace(getTargetPath(), 10 * 1024 * 1024)) { // 暂取小于10M作为磁盘空间不足的判断标准
                m_eErrorType = ET_InsufficientDiskSpace;
                emit signalFinished(PFT_Error);
                return false;
            }
        }

        handleProgress(line); // 处理进度
    }

    return true;
}
