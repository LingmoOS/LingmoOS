// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BATCHJOB_H
#define BATCHJOB_H

#include "archivejob.h"

#include <QFileInfo>

class BatchJob : public ArchiveJob
{
    Q_OBJECT
public:
    explicit BatchJob(QObject *parent = nullptr);
    ~BatchJob() override;

protected:
    /**
     * @brief addSubjob     添加子操作
     * @param job
     * @return
     */
    virtual bool addSubjob(ArchiveJob *job);

    /**
     * @brief removeSubjob  删除子操作
     * @param job
     * @return
     */
    virtual bool removeSubjob(ArchiveJob *job);

    /**
     * @brief hasSubjobs    判断是否还有操作未执行
     * @return
     */
    bool hasSubjobs() const;

    /**
     * @brief subjobs       获取所以的子操作
     * @return
     */
    const QList<ArchiveJob *> &subjobs() const;

    /**
     * @brief clearSubjobs  清除子操作
     */
    void clearSubjobs();

    /**
     * @brief doPause       暂停
     */
    void doPause() override;

    /**
     * @brief doContinue    继续
     */
    void doContinue() override;

    /**
     * @brief status 状态
     * @return 状态
     */
    bool status() override;

protected:
    ArchiveJob *m_pCurJob = nullptr;              // 当前操作

private:
    QList<ArchiveJob *> m_listSubjobs;  // 所有子操作

};

class BatchExtractJob : public BatchJob
{
    Q_OBJECT
public:
    explicit BatchExtractJob(QObject *parent = nullptr);
    ~BatchExtractJob() override;

    /**
     * @brief start     开始批量解压
     */
    void start() override;

    /**
     * @brief setExtractPath    设置解压路径
     * @param strPath           解压选择路径
     * @param bAutoCreatDir     是否自动创建文件夹
     */
    void setExtractPath(const QString &strPath/*, bool bAutoCreatDir*/);

    /**
     * @brief setArchiveFiles       设置批量解压文件（先调用setExtractPath，再调用此函数）
     * @param listFile              文件名（全路径）
     * @return                      返回结果
     */
    bool setArchiveFiles(const QStringList &listFile);

private:
    /**
     * @brief addExtractItem    添加解压元素
     * @param fileInfo          压缩包文件数据
     * @return                  返回结果
     */
    bool addExtractItem(const QFileInfo &fileInfo);

Q_SIGNALS:
    /**
     * @brief signalCurArchiveName  当前正在操作的压缩包信号
     * @param strArchiveName        压缩包名称
     */
    void signalCurArchiveName(const QString &strArchiveName);

private Q_SLOTS:
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
     * @brief slotHandleSingleJobFinished       处理单个压缩包解压结束
     */
    void slotHandleSingleJobFinished();

private:
    QStringList m_listFiles;        // 批量解压的压缩包列表
    qint64 m_qBatchTotalSize = 0;     //批量解压压缩包总大小
    QString m_strExtractPath;       // 解压选择路径
//    bool m_bAutoCreatDir = false;       // 是否自动创建文件夹
    int m_iArchiveCount = 0;        // 压缩文件数目
    int m_iCurArchiveIndex = 0;         // 当前正在操作的压缩包索引
    double m_dLastPercentage = 0;       // 最后进度

};

#endif // BATCHJOB_H
