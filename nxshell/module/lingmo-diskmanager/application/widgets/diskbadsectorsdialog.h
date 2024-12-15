// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef DISKBADSECTORSDIALOG_H
#define DISKBADSECTORSDIALOG_H

#include "partedproxy/dmdbushandler.h"

#include <DDialog>
#include <DComboBox>
#include <DLineEdit>
#include <DLabel>
#include <DSlider>
#include <DPushButton>
#include <DProgressBar>
#include <DSuggestButton>

#include <QWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class QStackedWidget;
class CylinderInfoWidget;
class QSettings;

/**
 * @class DiskBadSectorsDialog
 * @brief 坏道检测与修复主界面类
 */

class DiskBadSectorsDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DiskBadSectorsDialog(QWidget *parent = nullptr);

    enum StatusType {
        Normal,
        Check,
        StopCheck,
        Repair,
        StopRepair
    };

    void stopCheckRepair();

signals:

public slots:

protected:
    /**
     * @brief event:事件变化
     * @param event事件
     * @return 布尔
     */
    bool event(QEvent *event) override;

    /**
     * @brief 键盘按下事件
     * @param event事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 接收检测范围切换的槽函数
     * @param index 当前检测范围
     */
    void onVerifyChanged(int index);

    /**
     * @brief 接收检测方式切换的槽函数
     * @param index 当前检测方式
     */
    void onMethodChanged(int index);

    /**
     * @brief 滑动条数值改变响应的槽函数
     * @param value 检测次数
     */
    void onSliderValueChanged(int value);

    /**
     * @brief 检测次数输入框改变响应的槽函数
     * @param text 检测次数
     */
    void oncheckTimesChanged(const QString &text);

    /**
     * @brief 开始检测按钮点击响应的槽函数
     */
    void onStartVerifyButtonClicked();

    /**
     * @brief 退出按钮点击响应的槽函数
     */
    void onExitButtonClicked();

    /**
     * @brief 完成按钮点击响应的槽函数
     */
    void onDoneButtonClicked();

    /**
     * @brief 停止按钮点击响应的槽函数
     */
    void onStopButtonClicked();

    /**
     * @brief 继续按钮点击响应的槽函数
     */
    void onContinueButtonClicked();

    /**
     * @brief 重新检测按钮点击响应的槽函数
     */
    void onAgainVerifyButtonClicked();

    /**
     * @brief 复位按钮点击响应的槽函数
     */
    void onResetButtonClicked();

    /**
     * @brief 尝试修复按钮点击响应的槽函数
     */
    void onRepairButtonClicked();

    /**
     * @brief 坏道检测实时信息
     * @param cylinderNumber 柱面号
     * @param cylinderTimeConsuming 柱面耗时
     * @param cylinderStatus 柱面状态
     * @param cylinderErrorInfo 当前检测错误信息
     */
    void onCheckBadBlocksInfo(const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo);

    /**
     * @brief 检测完成响应的槽函数
     */
    void onCheckComplete();

    /**
     * @brief 坏道修复实时信息
     * @param cylinderNumber 柱面号
     * @param cylinderStatus 柱面修复后的状态
     * @param cylinderTimeConsuming 修复柱面耗时
     */
    void onRepairBadBlocksInfo(const QString &cylinderNumber, const QString &cylinderStatus, const QString &cylinderTimeConsuming);

    /**
     * @brief 修复完成响应的槽函数
     */
    void onRepairComplete();

    /**
     * @brief 定时器超时信号响应的槽函数
     */
    void onTimeOut();

    /**
     * @brief 定时器超时信号响应的槽函数
     */
    void onCheckTimeOut();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    /**
     * @brief 初始化界面
     */
    void initUI();

    /**
     * @brief 初始化信号连接
     */
    void initConnections();

    /**
     * @brief 判断检测范围和检测方式输入值是否有效
     * @return true有效，false则无效
     */
    bool inputValueIsEffective();

    /**
     * @brief 毫秒转换成时、分、秒的计算
     * @param msecs 毫秒
     * @param hour 时
     * @param minute 分
     * @param second 秒
     */
    void mSecsToTime(qint64 msecs, qint64 &hour, qint64 &minute, qint64 &second);

private:
    DComboBox *m_verifyComboBox;
    DLineEdit *m_startLineEdit;
    DLineEdit *m_endLineEdit;
    DLabel *m_verifyLabel;
    DLabel *m_methodLabel;
    DComboBox *m_methodComboBox;
    DSlider *m_slider;
    DLineEdit *m_checkTimesEdit;
    DLineEdit *m_timeoutEdit;
    QStackedWidget *m_methodStackedWidget;
    DLabel *m_checkInfoLabel;
    DPushButton *m_exitButton; // 退出
    DPushButton *m_resetButton; // 复位
    DPushButton *m_repairButton; // 尝试修复
    DSuggestButton *m_startButton; // 开始检测
    DPushButton *m_stopButton; // 停止
    DPushButton *m_continueButton; // 继续
    DPushButton *m_againButton; // 重新检测
    DSuggestButton *m_doneButton; // 完成
    QStackedWidget *m_buttonStackedWidget;
    DProgressBar *m_progressBar;
    DLabel *m_usedTimeLabel;
    DLabel *m_unusedTimeLabel;
    CylinderInfoWidget *m_cylinderInfoWidget;
    StatusType m_curType;
    int m_totalCheckNumber = 0;
    int m_curCheckNumber = 0;
    qint64 m_curCheckTime = 0;
    QSettings *m_settings;
    int m_totalRepairNumber = 0;
    int m_curRepairNumber = 0;
    qint64 m_curRepairTime = 0;
    int m_repairedCount = 0;
    QTimer m_timer;
    qint64 m_usedTime = 0;
    qint64 m_unusedTime = 0;
    QTimer m_checkTimer;
    int m_blockStart = 0;
    int m_blockEnd = 0;
    DeviceInfo m_deviceInfo;
};

#endif // DISKBADSECTORSDIALOG_H
