// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSCHECKADAPTORMODE_H
#define SYSCHECKADAPTORMODE_H

#include "homepagemodel.h"
#include "widgets/systemcheckdefinition.h"

#include <QObject>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>

class SysCheckAdaptorModel : public QObject
{
    Q_OBJECT
public:
    explicit SysCheckAdaptorModel(HomePageModel *model, QObject *parent = nullptr);
    ~SysCheckAdaptorModel();

    inline int getIssueCount() { return m_issueCount; }

    inline int getIssuePoint()
    {
        return CHECK_MAX_POINT > m_issuePoint ? CHECK_MAX_POINT - m_issuePoint : 0;
    }

    bool isFastFixValid();

Q_SIGNALS:
    void DiskExamFinished();
    void SSHExamFinished();
    void TrashExamFinished();
    void SysVersionExamFinished();
    void DevModeExamFinished();
    void AutoStartExamFinished();

    void checkStarted();
    void stageChanged(SysCheckItemID);
    void checkFinished();

    void fixItemStarted(SysCheckItemID);
    void fixItemFinished(SysCheckItemID);
    void onAutoStartAppCountChanged(int);

public Q_SLOTS:
    // 根据UI的设计，此处串行执行
    void startExam();
    void startDiskExam();
    void startSSHExam();
    void startTrashExam();
    void startSysVersionExam();
    void startDevModeExam();
    void startAutoStartExam();

    void onCheckStarted();
    void onCheckDone();
    void onCheckCanceled();

    void onRequestFixAll();
    void onRequestFixItem(SysCheckItemID);
    void onRequestSetIgnore(SysCheckItemID, bool);

    void onSysVersionCheckTimeout();

    const QStandardItemModel &getResultModel();

    bool isProcessingFastFixing();

private:
    void initConnection();
    void updateIssuePoint();
    void resetResultItems();

    void fixDisk();
    void fixTrash();
    void fixSysUpdate();
    void fixDevMode();
    void fixAutoStart();

private:
    HomePageModel *m_model;
    bool m_isDiskNormal;
    double m_totalTrashSize;
    bool m_isSysOutDate;
    bool m_isDevMode;
    int m_autoStartCount;
    // 用于检查页结果展示的数据
    QList<QStandardItem *> m_resultItems;
    QStandardItemModel m_resultModel;
    int m_issuePoint;      // 检查项累积扣分
    int m_processingIndex; // 检查进度索引，避免受外部信号影响检查过程
    int m_issueCount;
    QTimer m_sysVersionTimer; // 系统体检超时处理
    QString m_logProblemItemsInfo;
    QList<SysCheckItemID> m_fixingItems;
};

#endif // SYSCHECKADAPTORMODE_H
