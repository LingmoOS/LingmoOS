// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETECTEDSTATUSWIDGET_H
#define DETECTEDSTATUSWIDGET_H

#include "MacroDefinition.h"

#include <DFrame>
#include <DLabel>
#include <DSuggestButton>
#include <DIconButton>
#include <DProgressBar>
#include <DPushButton>
#include <DApplicationHelper>

#include <QWidget>
#include <QStringList>
#include <QBoxLayout>

class TitleLabel;
class TipsLabel;

DWIDGET_USE_NAMESPACE

/**
 * @brief The DetectedStatusWidget class 驱动安装主界面  UI 头部
 * 检测完成结果
 * 下载中
 * 安装中
 * 安装成功
 * 安装失败
 */

class DetectedStatusWidget : public DFrame
{
    Q_OBJECT
public:

    explicit DetectedStatusWidget(QWidget *parent = nullptr);

    ~DetectedStatusWidget();
    /**
     * @brief setDetectFinishUI: 检测结束头部UI
     * @param size 可安装更新的驱动个数
     * @param model 机器型号
     * @param hasInstall: 有可安装驱动
     */
    void setDetectFinishUI(const QString &size, const QString &model, bool hasInstall);

    /**
     * @brief setDownloadUI: 下载中头部UI
     * @param driverType: 驱动类型
     * @param speed: 速度
     * @param downloadSize: 下载大小
     * @param totalSize: 驱动总大小
     */
    void setDownloadUI(const DriverType &driverType
                       , const QString &speed
                       , const QString &downloadSize
                       , const QString &totalSize
                       , int progressValue);

    /**
     * @brief setInstallUI: 安装中头部UI
     * @param driverType: 驱动类型
     * @param driverDescription: 驱动描述
     */
    void setInstallUI(const DriverType &driverType, const QString &driverDescription, int progressValue);

    /**
     * @brief setInstallSuccessUI: 安装成功头部UI
     * @param success: 成功个数
     * @param failed: 失败个数
     */
    void setInstallSuccessUI(const QString &success, const QString &failed);

    /**
     * @brief setInstallFailedUI: 安装失败头部UI
     */
    void setInstallFailedUI();

    /**
     * @brief setNetworkErrorUI: 网络异常头部UI
     * @param speed: 速度
     * @param progressValue: 进度值
     */
    void setNetworkErrorUI(const QString &speed, int progressValue);

    /**
     * @brief setNoUpdateDriverUI: 驱动已是最新头部UI
     */
    void setNoUpdateDriverUI(const QString &model);

    /**
     * @brief refreshUI
     */
    void refreshUI(Status pageType);

    /**
     * @brief setInstallBtnEnable
     * @param enable
     */
    void setInstallBtnEnable(bool enable);
    void setBackupBtnEnable(bool enable);

    void setNoBackupDriverUI(int backableSize, int backedupSize);
    void setBackableDriverUI(int backableSize, int backedupSize);
    void setBackingUpDriverUI(const QString &driverDescription, int totalValue, int progressValue);
    void setBackupSuccessUI(int success, int failed);
    void setRestoreDriverUI(int restorableSize);
    void setRestoringUI(int progressValue = 0, QString driverDescription = "");
    void setReDetectEnable(bool enable);

signals:
    void redetected();
    void installAll();
    void backupAll();
    void cancelClicked();

public slots:
    /**
     * @brief slotReboot: 重启机器槽函数
     */
    void slotReboot();

    /**
     * @brief slotFeedBack: 反馈槽函数
     */
    void slotFeedBack();

    /**
     * @brief slotBackupPath: 打开备份路径
     */
    void slotBackupPath();

    /**
     * @brief slotInstall: 一键安装槽函数
     */
    void slotInstall();

    /**
     * @brief slotInstall: 一键备份槽函数
     */
    void slotBackup();

    /**
     * @brief slotReDetectSlot: 重新检测槽函数
     */
    void slotReDetectSlot();

    /**
     * @brief slotCancel: 取消槽函数
     */
    void slotCancel();

    /**
     * @brief onUpdateTheme 更新主题
     */
    void onUpdateTheme();

protected:

    // 重绘背景色
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief initUI: 初始化界面控件
     */
    void initUI();

    /**
     * @brief initConnect:初始化connect
     */
    void initConnect();

    /**
     * @brief hideAll: 隐藏所有控件,清除布局
     */
    void hideAll();

private:
    DLabel         *mp_PicLabel;              // 图片DLabel
    TitleLabel     *mp_UpdateLabel;           // 第一行 发现6个驱动可安装更新   或   驱动已是最新    或 正在下载驱动 等Label
    DLabel         *mp_TimeLabel;             // 第二行 检测时间
    TipsLabel      *mp_ModelLabel;            // 第三行 联想 Lenovo Product 台式机  或 下载速度 Label
    DLabel         *mp_RebootLabel;           // 重启Label
    DLabel         *mp_FeedBackLabel;         // 反馈Label
    DLabel         *mp_BackupPathLabel;       // 备份路径Label
    DSuggestButton *mp_InstallButton;         // 一键安装 Button
    DSuggestButton *mp_ReDetectedSgButton;    // 重新检测 Suggest Button
    DSuggestButton *mp_BackupSgButton;        // 一键备份 Suggest Button
    DPushButton    *mp_CancelButton;          // 取消 Button
    DIconButton    *mp_ReDetectedIconButton;  // 重新检测  Icon Button
    DProgressBar   *mp_Progress;              // 下载进度  安装进度

    QHBoxLayout    *mp_HLayoutTotal;         // 整体横向布局
    QHBoxLayout    *mp_HLayoutButton;        // Button 横向布局
    QVBoxLayout    *mp_VLayoutLabel;         // label 纵向布局
    QHBoxLayout    *mp_HLayoutLabel;         // label 横向布局

};

#endif // DETECTEDSTATUSWIDGET_H
