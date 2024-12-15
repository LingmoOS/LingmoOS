// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVEJOB_H
#define ARCHIVEJOB_H

#include <QObject>
#include "commonstruct.h"

class Query;

// 操作基类
class ArchiveJob : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveJob(QObject *parent = nullptr);
    ~ArchiveJob() override;

    // 操作类型
    enum JobType {
        JT_NoJob = 0,          // 创建压缩包
        JT_Create,          // 创建压缩包
        JT_Add,             // 添加压缩文件
        JT_Load,            // 加载压缩包
        JT_Extract,         // 解压
        JT_Delete,         // 删除
        JT_Rename,         // 重命名
        JT_BatchExtract,   // 批量解压
        JT_Open,            // 打开
        JT_Update,         // 更新
        JT_Comment,         // 注释
        JT_Convert,         // 转换
        JT_StepExtract,     // 分步解压
    };

    /**
     * @brief start     开始操作
     */
    virtual void start() = 0;

    /**
     * @brief doPause   暂停
     */
    virtual void doPause() = 0;

    /**
     * @brief doContinue    继续
     */
    virtual void doContinue() = 0;

    /**
     * @brief doCancel  取消
     */
    virtual void doCancel() {}

    /**
     * @brief kill  强行结束job
     */
    virtual void kill();

    /**
     * @brief status 状态
     * @return  状态
     */
    virtual bool status();

private:
    /**
     * @brief finishJob 结束job
     */
    void finishJob();

protected:
    /**
     * @brief doKill  强行结束job
     */
    virtual bool doKill();

Q_SIGNALS:
    /**
     * @brief slotFinished  操作结束处理
     * @param eType 结束类型
     */
    void signalJobFinshed();

    /**
     * @brief signalprogress    进度信号
     * @param iPercent  进度值
     */
    void signalprogress(double dPercentage);

    /**
     * @brief signalCurFileName 发送当前正在操作的文件名
     * @param strName   当前文件名
     */
    void signalCurFileName(const QString &strName);

    /**
     * @brief signalFileWriteErrorName 发送创建失败的文件名
     * @param strName   当前文件名
     */
    void signalFileWriteErrorName(const QString &strName);

    /**
     * @brief signalQuery   发送询问信号
     * @param query 询问类型
     */
    void signalQuery(Query *query);

public:
    JobType m_eJobType = JT_NoJob;     // 操作类型
    PluginFinishType m_eFinishedType = PFT_Nomral;
    ErrorType m_eErrorType = ET_NoError;

};

#endif // ARCHIVEJOB_H
