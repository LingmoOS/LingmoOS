/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef UKMEDIA_DEVCONTROL_WIDGET_H
#define UKMEDIA_DEVCONTROL_WIDGET_H

#include "kwidget.h"
#include <QBoxLayout>
#include <QStandardItem>
#include <QList>
#include "knavigationbar.h"
#include <QPushButton>
#include <QStandardItem>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QListView>
#include "kswitchbutton.h"
#include <QScrollBar>
#include <QtDBus/QtDBus>
#include <QListWidget>
#include <QDialog>
#include <QStackedWidget>
#include <pulse/def.h>
#include <QGSettings>
#include "../lingmo_custom_style.h"
#include "../common/ukmedia_common.h"

using namespace kdk;
//记录单个设备类
class SubDevWidget : public QWidget
{
    Q_OBJECT

public:
    SubDevWidget(QWidget *parent = nullptr);
    ~SubDevWidget();
    friend class OutputDevWidget;
    friend class InputDevWidget;

    void setLabelText(QString deviceLabel);

private:
    int cardId;
    QString devPortName;
    QString devLabelName;
    KSwitchButton *m_pDevDisableButton;
    FixLabel *m_pDeviceNameLabel;

public Q_SLOTS:

};

//输出设备类
class OutputDevWidget : public QWidget
{
    Q_OBJECT

public:
    OutputDevWidget(QWidget *parent = nullptr);
    ~OutputDevWidget();
    friend class UkmediaDevControlWidget;

     QFrame *displayOutputDevWidget;    //显示子设备窗口
private:
    QListWidget *m_pOutputListWidget;   //用于存放itemWidget
    SubDevWidget* subDevWidget;
    QFrame *m_pOutputDevWidget;         //输出设备主窗口
    QFrame *m_pOutputDevControlWidget;  //设备禁用窗口

    TitleLabel *m_pOutputDevLabel;      //标签：输出设备
    QScrollArea *outputDevArea;         //可动态添加的窗口区域
    QVBoxLayout *m_pSubDevVlayout;      //设备窗口的动态垂直布局

    KSwitchButton *m_pDevDisableButton; //输出设备禁用按钮
    QPushButton *m_pOutputConfirmBtn;   //“确定”按钮
    QHBoxLayout *mConfirmBtnHLaout;     //按钮布局
    QFrame* myLine();

    void initOutputDevUi();
    void initOutputDevWidget();     //初始化输出设备页面
    void addSubOutputDevWidget(int card, QString portName, QString DeviceName, int Available, int itemNum);
    void connectSubDevButton(KSwitchButton *button);

    QList<QWidget*> subDevWidgets; // 保存子设备窗口的容器

public Q_SLOTS:
    void closeWindow();
    void updateOutputDev(); //移除子设备窗口
    void setDevDisable(int cardId,QString portName,int devState); //通过DBUS发送信号给pulseaudio，进行设备禁用
    void getPaDevInfo();    //通过DBUS从pulseaudio中获取设备信息
};

//输入设备类
class InputDevWidget : public QWidget
{
    Q_OBJECT

public:
    InputDevWidget(QWidget *parent = nullptr);
    ~InputDevWidget();
    friend class UkmediaDevControlWidget;

     QFrame *displayInputDevWidget;    //显示子设备窗口
private:
    QListWidget *m_pInputListWidget;   //用于存放itemWidget
    SubDevWidget* subDevWidget;
    QFrame *m_pInputDevWidget;         //输出设备主窗口
    QFrame *m_pInputDevControlWidget;  //设备禁用窗口

    TitleLabel *m_pInputDevLabel;      //标签：输出设备
    QScrollArea *inputDevArea;         //可动态添加的窗口区域
    QVBoxLayout *m_pSubDevVlayout;      //设备窗口的动态垂直布局

    KSwitchButton *m_pDevDisableButton; //输入设备禁用按钮
    QPushButton *m_pInputConfirmBtn;   //“确定”按钮
    QHBoxLayout *mConfirmBtnHLaout;     //按钮布局
    QFrame* myLine();

    void initInputDevUi();
    void initInputDevWidget();     //初始化输出设备页面
    void addSubInputDevWidget(int card, QString portName, QString DeviceName, int Available, int itemNum);
    void connectSubDevButton(KSwitchButton *button);

    QList<QWidget*> subDevWidgets; // 保存子设备窗口的容器

public Q_SLOTS:
    void closeWindow();
    void updateInputDev(); //移除子设备窗口
    void setDevDisable(int cardId,QString portName,int devState); //通过DBUS发送信号给pulseaudio，进行设备禁用
    void getPaDevInfo();    //通过DBUS从pulseaudio中获取设备信息
};

//设备管理主类
class UkmediaDevControlWidget : public KWidget
{
    Q_OBJECT

    enum KNavigationBarPosition {
        OutputItem = 1,
        InputItem
    };

public:
    explicit UkmediaDevControlWidget(QWidget *parent = nullptr);
    ~UkmediaDevControlWidget();

private:
    QFrame *m_pDevControlWidget;
    KNavigationBar* m_pOutputAndInputBar;
    OutputDevWidget *outputDevWidget;
    InputDevWidget *inputDevWidget;

    void initDevControlWidget();
};


#endif // UKMEDIA_DEVCONTROL_WIDGET_H
