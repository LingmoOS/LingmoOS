// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSPAGE_H
#define PROGRESSPAGE_H

#include "uistruct.h"

#include <DWidget>
#include <DLabel>
#include <DProgressBar>
#include <DSuggestButton>
#include <QElapsedTimer>

DWIDGET_USE_NAMESPACE

class CustomSuggestButton;
class CustomPushButton;

// 进度界面
class ProgressPage : public DWidget
{
    Q_OBJECT
public:
    explicit ProgressPage(QWidget *parent = nullptr);
    ~ProgressPage() override;

    /**
     * @brief setProgressType   设置进度类型
     * @param eType 类型
     */
    void setProgressType(Progress_Type eType);

    /**
     * @brief setTotalSize  设置总大小
     * @param qTotalSize
     */
    void setTotalSize(qint64 qTotalSize);

    /**
     * @brief setArchiveName    设置压缩包名称和文件总大小、显示图标
     * @param strArchiveName    压缩包名称(包含路径)
     */
    void setArchiveName(const QString &strArchiveName);

    /**
     * @brief archiveName   获取压缩包名称
     * @return
     */
    QString archiveName();

    /**
     * @brief setProgress   设置当前进度
     * @param dPercent  进度值
     */
    void setProgress(double dPercent);

    /**
     * @brief setCurrentFileName    设置当前文件名
     * @param strFileName   文件名
     */
    void setCurrentFileName(const QString &strFileName);

    /**
     * @brief resetProgress 重置进度
     */
    void resetProgress();
    /**
     * @brief startTimer 重启计时器
     */
    void restartTimer();

    /**
     * @brief setPushButtonCheckable 设置按钮是否可点击
     */
    void setPushButtonCheckable(bool a, bool b);
    /**
     * @brief isPause 进度页是否暂停
     */
    bool isPause();
private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief calSpeedAndRemainingTime  计算速度和剩余时间
     * @param dSpeed                    速度
     * @param qRemainingTime            剩余时间
     */
    void calSpeedAndRemainingTime(double &dSpeed, qint64 &qRemainingTime);

    /**
     * @brief displaySpeedAndTime   显示速度和剩余时间
     * @param speed                 速度
     * @param timeLeft              剩余时间
     */
    void displaySpeedAndTime(double dSpeed, qint64 qRemainingTime);

Q_SIGNALS:
    /**
     * @brief signalPause 暂停信号
     */
    void signalPause();

    /**
     * @brief signalContinue    继续
     */
    void signalContinue();

    /**
     * @brief signalCancel       取消
     */
    void signalCancel();

private Q_SLOTS:
    /**
     * @brief slotPauseClicked  点击暂停
     */
    void slotPauseClicked(bool bChecked);

    /**
     * @brief slotCancelClicked 点击取消
     */
    void slotCancelClicked();

private:
    DLabel *m_pPixmapLbl;       // 类型图片
    DLabel *m_pArchiveNameLbl;  // 压缩包名称
    DProgressBar *m_pProgressBar;   // 进度条
    DLabel *m_pFileNameLbl;     // 文件名称
    DLabel *m_pSpeedLbl;        // 速度
    DLabel *m_pRemainingTimeLbl;    // 剩余时间
    CustomPushButton *m_pCancelBtn;      // 取消按钮
    CustomSuggestButton *m_pPauseContinueButton; // 暂停继续按钮

    Progress_Type m_eType = PT_None;      // 进度类型
    qint64 m_qTotalSize = 0;         // 文件大小kB
    int m_iPerent = 0;      // 进度值
    QElapsedTimer m_timer;  //计时器
    qint64 m_qConsumeTime = 0; //消耗时间

    QString m_strArchiveName;
};

#endif // COMPRESSPAGE_H
