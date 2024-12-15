// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERSCANWIDGET_H
#define DRIVERSCANWIDGET_H

#include <DWidget>
#include <DFrame>
#include <DLabel>
#include <DProgressBar>
#include <DSuggestButton>

#include <QBoxLayout>

DWIDGET_USE_NAMESPACE

class AnimationLabel;
class DriverScanWidget : public DFrame
{
    Q_OBJECT
public:
    explicit DriverScanWidget(DWidget *parent = nullptr);

    /**
     * @brief setScanningUI: 设置正在检测UI界面
     * @param scanInfo: 检测信息
     * @param progressValue: 进度值
     */
    void setScanningUI(const QString &scanInfo, int progressValue);

    /**
     * @brief setScanFailedUI: 设置检测失败UI界面
     */
    void setScanFailedUI();

    /**
     * @brief setNetworkErr: 设置网络异常UI界面
     */
    void setNetworkErr();

    /**
     * @brief setProgressFinish： progress 完成 设置100
     */
    void setProgressFinish();

    /**
     * @brief refreshProgress: 设置进度条value
     * @param info: 刷新信息
     * @param progress: 进度值
     */
    void refreshProgress(QString info, int progress);

public slots:
    /**
     * @brief slotFeedBack: 反馈槽函数
     */
    void slotFeedBack();

    /**
     * @brief slotReDetected: 重新检测槽函数
     */
    void slotReDetected();

private:
    /**
     * @brief initUI: 初始化界面
     */
    void initUI();

    /**
     * @brief initConnect: 初始化连接
     */
    void initConnect();

    /**
     * @brief hideAll: 隐藏所有空间
     */
    void hideAll();

signals:
    void redetected();
protected:

    // 重绘背景色
    void paintEvent(QPaintEvent *event) override;

    // resize 事件
    void resizeEvent(QResizeEvent* event) override;

private:
    AnimationLabel  *mp_ScanningPicLabel;    // 扫描图片Label
    DLabel          *mp_ErrPicLabel;          // 扫描出错图片Label
    DLabel          *mp_ScanningLabel;       // 正在扫描文字Label
    DLabel          *mp_CheckNetworkLabel;   // 请检查网络标签
    DLabel          *mp_FeedBackLabel;       // 反馈Label
    DLabel          *mp_ScanningInfoLabel;   // 扫描信息Label
    DProgressBar    *mp_ScanningProgress;    // 扫描进度条
    DSuggestButton  *mp_ReScanButton;        // 重新扫描button
    QWidget         *mp_ScrollWidget;
    QScrollBar      *mp_ScroBar;

    QHBoxLayout     *mp_HLayoutPic;          // 横向图片布局
    QHBoxLayout     *mp_HLayoutscLabel;      // 横向提示信息布局
    QHBoxLayout     *mp_HLayoutscProgress;   // 横向进度条布局
    QHBoxLayout     *mp_HLayoutInfoLabel;    // 横向检测信息布局
    QHBoxLayout     *mp_HLayout;             // 整体横向布局
    QVBoxLayout     *mp_VLayout;             // 整体纵向布局
};

#endif // DRIVERSCANWIDGET_H
