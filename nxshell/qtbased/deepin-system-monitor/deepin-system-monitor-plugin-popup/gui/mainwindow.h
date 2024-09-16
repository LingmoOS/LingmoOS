// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


// dsmPluginPopup
//#include "constants.h"
#include "dde-dock/constants.h"
#include "dbus/systemmonitordbusadaptor.h"
#include "dbus/dbusdockinterface.h"
#include "cpu_widget.h"
#include "net_widget.h"
#include "memory_widget.h"
#include "disk_widget.h"
#include "process_widget.h"

// Qt
#include <QListView>
#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
#include <DBlurEffectWithBorderWidget>
#else
#include <DBlurEffectWidget>
#endif
#include <DRegionMonitor>
#include <DWidget>
#include <DScrollArea>
#include <DtkGuis>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QPropertyAnimation)
QT_FORWARD_DECLARE_CLASS(QSequentialAnimationGroup)

/*!
 * \~chinese \class MainWindow
 * \~chinese \brief 主窗口类
 */
#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
class MainWindow : public DBlurEffectWithBorderWidget
#else
class MainWindow : public DBlurEffectWidget
#endif
{
    Q_OBJECT
    Q_PROPERTY(int width READ getWidth WRITE setFixedWidth)
    Q_PROPERTY(int x WRITE setX)
    Q_PROPERTY(bool sysMonPopVisible READ sysMonPopVisible NOTIFY sysMonPopVisibleChanged)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

Q_SIGNALS:
    void signal_geometry(int height);
    /*!
     * \brief sysMonPopVisibleChanged 系统监视器弹窗显示状态改变
     * \param visible
     */
    void sysMonPopVisibleChanged(bool visible);

public Q_SLOTS:
    /*!
     * \~chinese \name Toggle
     * \~chinese \brief 切换主窗口显示或隐藏的状态
     */
    void Toggle();
    /*!
     * \~chinese \name showAni
     * \~chinese \brief 主窗口显示时的动画
     */
    void showAni();
    /*!
     * \~chinese \name hideAni
     * \~chinese \brief 主窗口隐藏时的动画
     */
    void hideAni();

    void startLoader();

    void Show();
    void Hide();

private Q_SLOTS:
    /*!
     * \~chinese \name geometryChanged
     * \~chinese \brief 根据dock和屏幕的大小改变窗口的大小,根据dock的位置调整主窗口的位置
     */
    void geometryChanged();
    /*!
     * \~chinese \name setX
     * \~chinese \brief 设置主窗口X轴的起始坐标
     * \~chinese \param X轴坐标位置
     */
    void setX(int x);
    /*!
     * \~chinese \name CompositeChanged
     * \~chinese \brief 是否开启动画效果
     */
    void CompositeChanged();

    void registerMonitor();

    /*!
     * \brief sysMonPopVisible 系统监视器弹窗显示状态
     * \return
     */
    bool sysMonPopVisible() const{ return isVisible();}

private:
    /*!
     * \~chinese \name initUI
     * \~chinese \brief 初始化界面布局
     */
    void initUI();
    /*!
     * \~chinese \name initAni
     * \~chinese \brief 初始化动画效果
     */
    void initAni();
    /*!
     * \~chinese \name initConnect
     * \~chinese \brief 初始化信号与槽的连接
     */
    void initConnect();
    /*!
     * \~chinese \name adjustPosition
     * \~chinese \brief 调整主界面位置
     */
    void adjustPosition();
    /*!
     * \~chinese \name getDisplayScreen
     * \~chinese \brief 获取显示屏幕的坐标
     */
    QRect getDisplayScreen();


    int getWidth() const { return this->width(); }
//    int getX() const { return this->pos().x(); }


    bool initDBus();


protected:
    /*!
     * \~chinese \name initConnect
     * \~chinese \brief 重写mouseMoveEvent事件禁止窗口被移动
     */
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    virtual bool eventFilter(QObject *object, QEvent *event) override;

    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
private slots:
    void slotShowOrHideSystemMonitorPluginPopupWidget();
    //!
    //! \brief changeTheme 切换系统主题时，改变当前的显示内容
    //! \param themeType 主题类型
    //!
    void changeTheme(DApplicationHelper::ColorType themeType);
    //!
    //! \brief dbusPropertiesChanged 获取属性变化信号
    //! \param interface 接口，maps 数据，strs 名称
    //!
    void dbusPropertiesChanged(QString interface,QVariantMap maps,QStringList strs);

private:
    QDBusInterface *m_displayInter;
    //用于存储m_displayInter->monitor()中的内容，解决内存泄漏的问题
    QList<QDBusObjectPath> m_dbusPathList     {};
    QDBusInterface *m_daemonDockInter;
    DBusDockInterface *m_dockInter;
    SystemMonitorDBusAdaptor *m_systemMonitorDbusAdaptor;
    DRegionMonitor *m_regionMonitor;

//    IconButton *m_clearButton;
//    ListView *m_listview;
//    ClipboardModel *m_model;
//    ItemDelegate *m_itemDelegate;

    QPropertyAnimation *m_xAni;
    QPropertyAnimation *m_widthAni;
    QSequentialAnimationGroup *m_aniGroup;
    QRect m_rect;
    bool m_hasComposite = false;


    CpuWidget *m_cpuMonitor {};
    NetWidget *m_netWidget {};
    DiskWidget *m_diskWidget {};
    MemoryWidget *m_memoryWidget {};
    ProcessWidget *m_processWidget{};
    DScrollArea *m_scrollArea {};       // 滚动区域

    // 防止1000ms内重复按键
    QTimer *m_trickTimer;
    //最近一次拖动的距离
    int m_last_time_move = 0;


    // 5分钟计时，自动结束进程
    QTimer *m_processEndTimer;


};

#endif // MAINWINDOW_H
