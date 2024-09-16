// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEDETAILPAGE_H
#define DEVICEDETAILPAGE_H

#include <QObject>
#include <DWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <DCommandLinkButton>
#include <DHorizontalLine>

#include "DeviceInfo.h"

class TextBrowser;
class DeviceBaseInfo;
class QVBoxLayout;

using namespace Dtk::Widget;

/**
 * @brief The DetailButton class
 * 控件，展开和收起
 */

class DetailButton: public DCommandLinkButton
{
    Q_OBJECT
public:
    explicit DetailButton(const QString &txt);

    /**
     * @brief updateText 更新按钮的显示信息
     */
    void updateText();

protected:
    void paintEvent(QPaintEvent *e) override;
};

/**
 * @brief The DetailSeperator class
 * 分割线
 */
class DetailSeperator: public DWidget
{
    Q_OBJECT
public:
    explicit DetailSeperator(DWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
};

/**
 * @brief The ScrollAreaWidget class
 * 滑动区域
 */
class ScrollAreaWidget: public DWidget
{
    Q_OBJECT
public:
    explicit ScrollAreaWidget(DWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;

};

/**
 * @brief The PageDetail class
 * UI 设备详细信息的展示
 */
class PageDetail : public DWidget
{
    Q_OBJECT
public:
    explicit PageDetail(QWidget *parent = nullptr);

    /**
     * @brief showDeviceInfo : 显示设备信息
     * @param lstInfo :需要显示的设备
     */
    void showDeviceInfo(const QList<DeviceBaseInfo *> &lstInfo);

    /**
     * @brief showInfoOfNum : 点击表头时，详细信息显示到相应的位置
     * @param index : 需要显示到相应的位置
     */
    void showInfoOfNum(int index);

    /**
     * @brief enableDevice
     * @param row
     * @param enable
     * @return
     */
    EnableDeviceStatus enableDevice(int row, bool enable);

    /**
     * @brief setWakeupMachine
     * @param row
     * @param wakeup
     */
    void setWakeupMachine(int row, bool wakeup);

    /**
     * @brief clearWidget : 清楚widget
     */
    void clearWidget();

protected:
    void paintEvent(QPaintEvent *e) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief addWidgets 添加widget到布局
     * @param widget ：需要添加的widget
     */
    void addWidgets(TextBrowser *widget, bool enable);

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();

private slots:

    /**
     * @brief slotBtnClicked:鼠标右键菜单槽函数
     */
    void slotBtnClicked();

    void slotCopyAllInfo();

private:
    QVBoxLayout      *mp_ScrollAreaLayout;
    QScrollArea      *mp_ScrollArea;
    ScrollAreaWidget *mp_ScrollWidget;

    QList<TextBrowser *>           m_ListTextBrowser;
    QList<QHBoxLayout *>           m_ListHlayout;
    QList<DetailButton *>          m_ListDetailButton;
    QList<DetailSeperator *>       m_ListDetailSeperator;
};

#endif // DEVICEDETAILPAGE_H
