/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2009-2011 Raphael Kubo da Costa <rakuco@FreeBSD.org>
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

#ifndef CLIINTERFACE_H
#define CLIINTERFACE_H

//#include "archiveinterface.h"
#include "cliproperties.h"
#include "kptyprocess.h"

#include <QScopedPointer>
#include <QTemporaryDir>
#include <QSet>

//enum WorkType {
//    WT_List,
//    WT_Extract,
//    WT_Add,
//    WT_Delete,
//    WT_Move,
//    WT_Copy,
//    WT_Comment,
//    WT_Test
//};

enum ParseState {
    ParseStateTitle = 0,
    ParseStateHeader,
    ParseStateArchiveInformation,
    ParseStateEntryInformation
};

class PasswordNeededQuery;

class CliInterface : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit CliInterface(QObject *parent, const QVariantList &args);
    ~CliInterface() override;

    // ReadOnlyArchiveInterface interface
public:
    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    virtual bool isPasswordPrompt(const QString &line) = 0;
    virtual bool isWrongPasswordMsg(const QString &line) = 0;
    virtual bool isCorruptArchiveMsg(const QString &line) = 0;
    virtual bool isDiskFullMsg(const QString &line) = 0;
    virtual bool isFileExistsMsg(const QString &line) = 0;
    virtual bool isFileExistsFileName(const QString &line) = 0;
    virtual bool isMultiPasswordPrompt(const QString &line) = 0;
    virtual bool isOpenFileFailed(const QString &line) = 0;

    /**
     * @brief pauseOperation    暂停操作
     */
    void pauseOperation() override;

    /**
     * @brief continueOperation 继续操作
     */
    void continueOperation() override;

    /**
     * @brief doKill 强行取消
     */
    bool doKill() override;

    // ReadWriteArchiveInterface interface
public:
    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    PluginFinishType renameFiles(const QList<FileEntry> &files) override;
    PluginFinishType addComment(const QString &comment) override;
    PluginFinishType updateArchiveData(const UpdateOptions &options) override;

protected:
    /**
     * @brief setListEmptyLines  需要解析空的命令行输出(rar格式含有注释的情况)
     * @param emptyLines  默认不需要解析
     */
    void setListEmptyLines(bool emptyLines = false);

    /**
     * @brief runProcess  执行命令
     * @param programName  命令
     * @param arguments  命令参数
     * @return
     */
    bool runProcess(const QString &programName, const QStringList &arguments);

    /**
     * @brief handleLine  处理命令行输出
     * @param line  行内容
     * @param workStatus 当前进程工作类型
     * @return
     */
    virtual bool handleLine(const QString &line, WorkType workStatus) = 0;

    /**
     * @brief deleteProcess 删除进程
     */
    void deleteProcess();

    /**
     * @brief killProcess  结束进程
     * @param emitFinished
     */
    virtual void killProcess(bool emitFinished = true) = 0;

    /**
     * @brief handleProgress  解析进度并发送进度信号
     * @param line
     */
    void handleProgress(const QString &line);

    /**
     * @brief handlePassword  需要密码是弹出密码框
     */
    PluginFinishType handlePassword();

    /**
     * @brief handleFileExists 处理解压存在同名文件
     * @param line
     * @return
     */
    bool handleFileExists(const QString &line);

    /**
     * @brief handleCorrupt 处理非致命损坏文件
     * @return
     */
    PluginFinishType handleCorrupt();

    /**
     * @brief writeToProcess 追加命令
     * @param data
     */
    void writeToProcess(const QByteArray &data);

    /**
     * @brief getTargetPath 获取解压目标路径
     * @return
     */
    QString getTargetPath();
	    /**
     * @brief extractFiles 业务解压
     */
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options, bool bDlnfs);

private:
    /**
     * @brief moveDroppedFilesToDest 提取文件后，将文件从临时目录移到目的路径
     * @param files  文件
     * @param options 解压选项
     * @return
     */
    bool moveExtractTempFilesToDest(const QList<FileEntry> &files, const ExtractionOptions &options);

    bool handleLongNameExtract(const QList<FileEntry> &files);

private slots:
    /**
     * @brief readStdout  读取命令行输出
     * @param handleAll  是否读取结束，默认未结束
     */
    virtual void readStdout(bool handleAll = false);

    /**
     * @brief processFinished  进程结束
     * @param exitCode   进程退出码
     * @param exitStatus  结束状态
     */
    virtual void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief extractProcessFinished 解压进程结束
     * @param exitCode   进程退出码
     * @param exitStatus  结束状态
     */
    void extractProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief getChildProcessIdNormal7z  对于调用7z出现多个子进程的处理
     * @param processid   运行bash命令的进程号(QString类型)
     * @param listKey     进程关键字（例如7z、tar等）
     * @param childprocessid  存储子进程号的容器
     */
    void getChildProcessId(qint64 processId, const QStringList &listKey, QVector<qint64> &childprocessid);

protected:
    CliProperties *m_cliProps = nullptr;  // 命令属性
    /*KProcess*/KPtyProcess *m_process = nullptr;  // 工作进程
    PluginFinishType m_finishType = PFT_Nomral; // 插件结束类型
    QString m_strEncryptedFileName = QString(); // 当前被解压的加密文件名
    QVector<qint64> m_childProcessId; // 压缩tar.7z文件的子进程Id
    bool m_isProcessKilled = false;  // 进程已经结束
    bool m_isEmptyArchive = false;  // 压缩包内无数据
    bool m_isCorruptArchive = false; // 是否非致命错误

private:
    QList<FileEntry> m_files; // 文件
    ExtractionOptions m_extractOptions; // 解压选项
    CompressOptions m_compressOptions; // 压缩选项

    bool m_listEmptyLines = false; // true:rar加载list， false:7z加载list
    QByteArray m_stdOutData;  // 存储命令行输出数据
    QString m_parseName;  // 解析后的文件名
    QScopedPointer<QTemporaryDir> m_extractTempDir;  // 临时文件夹
    QString m_rootNode = ""; // 待提取文件的节点

    qint64  m_processId;  // 进程Id
    bool m_isTar7z = false; // 是否是tar.7z文件
    qint64 m_filesSize; //选择需要压缩的文件大小，压缩tar.7z时使用
    QString m_rootEntry; // 追加压缩文件夹的时候记录上一层节点
    int m_indexOfListRootEntry = 0; // 右键解压到当前文件夹时的数据
    QMap<QString, int> m_mapLongName;       // 长文件名统计
    QMap<QString, int> m_mapLongDirName;    // 长文件夹统计
    QMap<QString, int> m_mapRealDirValue;    // 真实文件统计
};

#endif // CLIINTERFACE_H
