// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEJOB_H
#define SINGLEJOB_H

#include "archivejob.h"
#include "archiveinterface.h"
#include "commonstruct.h"
#include "queries.h"

#include <QThread>
#include <QElapsedTimer>

class SingleJob;

// 工作线程
class SingleJobThread : public QThread
{
    Q_OBJECT
public:
    SingleJobThread(SingleJob *job, QObject *parent = nullptr)
        : QThread(parent)
        , q(job)
    {
    }

    /**
     * @brief run
     */
    void run() override;

private:
    SingleJob *q;
};

// 单个操作
class SingleJob : public ArchiveJob
{
    Q_OBJECT
public:
    explicit SingleJob(ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~SingleJob() override;

    /**
     * @brief doWork    执行操作
     */
    virtual void doWork() = 0;

    /**
     * @brief start     开始
     */
    void start() override;

    /**
     * @brief doPause   暂停
     */
    void doPause() override;

    /**
     * @brief doContinue    继续
     */
    void doContinue() override;

    /**
     * @brief status 状态
     * @return  状态
     */
    bool status() override;

    SingleJobThread *getdptr();
protected:
    /**
     * @brief initConnections   初始化插件和job的信号槽连接
     */
    void initConnections();

    /**
     * @brief doKill  强行结束job
     */
    bool doKill() override;

protected Q_SLOTS:

    /**
     * @brief slotFinished  操作结束处理
     * @param eType 结束类型
     */
    void slotFinished(PluginFinishType eType);

//Q_SIGNALS:

protected:
    ReadOnlyArchiveInterface *m_pInterface;
    SingleJobThread *const d;   // 线程
    QElapsedTimer jobTimer;     // 操作计时
};

// 加载操作
class LoadJob : public SingleJob
{
    Q_OBJECT
public:
    explicit LoadJob(ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~LoadJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:

};

// 压缩操作
class AddJob : public SingleJob
{
    Q_OBJECT
public:
    explicit AddJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent = nullptr);
    ~AddJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QList<FileEntry> m_vecFiles; //待压缩文件(夹)
    QString m_strDestination; //追加时当前压缩包内路径
    CompressOptions m_stCompressOptions; //压缩配置
};

// 创建压缩包操作
class CreateJob : public SingleJob
{
    Q_OBJECT
public:
    explicit CreateJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const CompressOptions &options, QObject *parent = nullptr);
    ~CreateJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

protected:
    bool doKill() override;

private:
    /**
     * @brief cleanCompressFileCancel 压缩取消时删除临时压缩包
     */
    void cleanCompressFileCancel();

private:
    QList<FileEntry> m_vecFiles;
    CompressOptions m_stCompressOptions;
};

// 解压操作
class ExtractJob : public SingleJob
{
    Q_OBJECT
public:
    explicit ExtractJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, const ExtractionOptions &options, QObject *parent = nullptr);
    ~ExtractJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

    bool errorcode = true;

private:
    QList<FileEntry> m_vecFiles;
    ExtractionOptions m_stExtractionOptions;
};

// 删除操作
class DeleteJob : public SingleJob
{
    Q_OBJECT
public:
    explicit DeleteJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~DeleteJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QList<FileEntry> m_vecFiles;

};

// 重命名操作
class RenameJob : public SingleJob
{
    Q_OBJECT
public:
    explicit RenameJob(const QList<FileEntry> &files, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~RenameJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QList<FileEntry> m_vecFiles;

};

// 打开操作
class OpenJob: public SingleJob
{
    Q_OBJECT
public:
    explicit OpenJob(const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~OpenJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

protected Q_SLOTS:
    /**
     * @brief slotFinished  操作结束处理
     * @param eType 结束类型
     */
    void slotFinished(PluginFinishType eType);

private:
    FileEntry m_stEntry;            // 待打开文件
    QString m_strTempExtractPath;   // 临时解压路径
    QString m_strProgram;           // 应用程序
};

// 更新操作（压缩包修改完之后更新缓存数据）
class UpdateJob: public SingleJob
{
    Q_OBJECT
public:
    explicit UpdateJob(const UpdateOptions &options, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~UpdateJob() override;

    /**
     * @brief start     开始
     */
    void start() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    UpdateOptions m_stOptions;      // 更新选项
};

// 注释操作
class CommentJob: public SingleJob
{
    Q_OBJECT
public:
    explicit CommentJob(const QString &strComment, ReadOnlyArchiveInterface *pInterface, QObject *parent = nullptr);
    ~CommentJob() override;

    /**
     * @brief doWork    执行操作
     */
    void doWork() override;

private:
    QString m_strComment;      // 注释
};


// 组合操作
// 例如：格式转换、分步解压
class ComplexJob: public ArchiveJob
{
    Q_OBJECT
public:
    explicit ComplexJob(const QString strOriginalArchiveFullPath, QObject *parent = nullptr);
    ~ComplexJob() override;

    /**
     * @brief start     开始
     */
    virtual void start() override = 0;

    /**
     * @brief doPause   暂停
     */
    void doPause() override;

    /**
     * @brief doContinue    继续
     */
    void doContinue() override;

    /**
     * @brief doKill  强行结束job
     */
    bool doKill() override;

protected Q_SLOTS:
    /**
     * @brief slotHandleSingleJobProgress       处理单个压缩包解压进度
     * @param dPercentage
     */
    void slotHandleSingleJobProgress(double dPercentage);

    /**
     * @brief slotHandleSingleJobCurFileName       处理单个压缩包当前解压文件名
     * @param strName
     */
    void slotHandleSingleJobCurFileName(const QString &strName);

    /**
     * @brief slotHandleExtractFinished       处理解压结束
     */
    virtual void slotHandleExtractFinished() = 0;

protected:
    ReadOnlyArchiveInterface *m_pIface = nullptr;
    QString m_strOriginalArchiveFullPath;   // 原始压缩包全路径
    int m_iStepNo = 0;
};

// 转换操作
class ConvertJob: public ComplexJob
{
    Q_OBJECT
public:
    explicit ConvertJob(const QString strOriginalArchiveFullPath, const QString strTargetFullPath, const QString strNewArchiveFullPath, QObject *parent = nullptr);
    ~ConvertJob() override;

    /**
     * @brief start     开始
     */
    void start() override;

private Q_SLOTS:
    /**
     * @brief slotHandleExtractFinished       处理解压结束
     */
    void slotHandleExtractFinished() override;

private:
    ExtractJob *m_pExtractJob = nullptr;  // 先解压
    CreateJob *m_pCreateJob = nullptr;    // 再压缩成想要的格式
    QString m_strTargetFullPath;                // 转换目标全路径
    QString m_strNewArchiveFullPath;   // 格式转换后压缩包全路径
};

// 分步解压操作
// 解压特殊压缩包，如tar.7z
// 注意：tar.bz2、tar.lzma、tar.Z不需要分步解压，直接使用libarchiveplugin
class StepExtractJob: public ComplexJob
{
    Q_OBJECT
public:
    explicit StepExtractJob(const QString strOriginalArchiveFullPath, const ExtractionOptions &stOptions, QObject *parent = nullptr);
    ~StepExtractJob() override;

    /**
     * @brief start     开始
     */
    void start() override;

protected:
    bool doKill() override;

private Q_SLOTS:

    /**
     * @brief slotHandleExtractFinished       处理解压结束
     */
    void slotHandleExtractFinished() override;

private:
    ExtractJob *m_pExtractJob = nullptr;  // 先解压成中间格式
    ExtractJob *m_pExtractJob2 = nullptr;  // 再完全解压
    QString m_strTempFilePath; // 临时解压路径
    ExtractionOptions m_stExtractionOptions;
};

#endif
