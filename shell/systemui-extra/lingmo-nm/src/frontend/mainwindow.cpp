/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include "mainwindow.h"
#include "customstyle.h"
#include <KWindowEffects>
#include <QApplication>
#include <QDebug>
#include <QDBusReply>
#include <QKeyEvent>
#include <QProcess>
#include <QPainterPath>

#include "lingmonetworkdeviceresource.h"
#include "../backend/dbus-interface/lingmoagentinterface.h"

#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include "lingmosdk/lingmosdk-system/libkysysinfo.h"
#include "lingmoutil.h"

#define MAINWINDOW_WIDTH 420
#define MAINWINDOW_HEIGHT 476
#define LAYOUT_MARGINS 0,0,0,0
#define LOADING_TRAYICON_TIMER_MS 60
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
#define PANEL_SETTINGS "org.lingmo.panel.settings"
#define PANEL_SIZE_KEY "panelsize"
#define PANEL_POSITION_KEY "panelposition"

const QString v10Sp1 = "V10SP1";
const QString intel = "V10SP1-edu";

#define LANPAGE 0
#define WLANPAGE 1
#define AUTOSELET 2

#define KEY_PRODUCT_FEATURES "PRODUCT_FEATURES"

#define MW_EXCELLENT_SIGNAL 80
#define MW_GOOD_SIGNAL 55
#define MW_OK_SIGNAL 30
#define MW_LOW_SIGNAL 5
#define MW_NONE_SIGNAL 0

#define EXCELLENT_SIGNAL_ICON   "network-wireless-signal-excellent-symbolic"
#define GOOD_SIGNAL_ICON        "network-wireless-signal-good-symbolic"
#define OK_SIGNAL_ICON          "network-wireless-signal-ok-symbolic"
#define LOW_SIGNAL_ICON         "network-wireless-signal-weak-symbolic"
#define NONE_SIGNAL_ICON        "network-wireless-signal-none-symbolic"

#define EXCELLENT_SIGNAL_LIMIT_ICON   "lingmo-network-wireless-signal-excellent-error-symbolic"
#define GOOD_SIGNAL_LIMIT_ICON        "lingmo-network-wireless-signal-good-error-symbolic"
#define OK_SIGNAL_LIMIT_ICON          "lingmo-network-wireless-signal-ok-error-symbolic"
#define LOW_SIGNAL_LIMIT_ICON         "lingmo-network-wireless-signal-weak-error-symbolic"
#define NONE_SIGNAL_LIMIT_ICON        "lingmo-network-wireless-signal-none-error-symbolic"

#define EXCELLENT_SIGNAL_INTRANET_ICON   "lingmo-network-wireless-signal-excellent-intranet-symbolic"
#define GOOD_SIGNAL_INTRANET_ICON        "lingmo-network-wireless-signal-good-intranet-symbolic"
#define OK_SIGNAL_INTRANET_ICON          "lingmo-network-wireless-signal-ok-intranet-symbolic"
#define LOW_SIGNAL_INTRANET_ICON         "lingmo-network-wireless-signal-weak-intranet-symbolic"
#define NONE_SIGNAL_INTRANET_ICON        "lingmo-network-wireless-signal-none-intranet-symbolic"

#include <kwindowsystem.h>
#include <kwindowsystem_export.h>

MainWindow::MainWindow(QString display, QWidget *parent) : QMainWindow(parent), m_display(display)
{
    firstlyStart();
}

/**
 * @brief MainWindow::showMainwindow show主窗口，同时也作为dbus接口提供给外部组件调用
 */
void MainWindow::showMainwindow()
{
    if (!m_loadFinished) {
        m_secondaryStartTimer->stop();
        secondaryStart();
    }

    /**
     * 设置主界面跳过任务栏和分页器的属性，隐藏再次展示有可能辉冲刷掉该属性，需要展示时重新设置
     */
#if 0
    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        const KWindowInfo info(this->winId(), NET::WMState);
        if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager)) {
            KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
        }
    }
#endif

    this->showByWaylandHelper();
    this->raise();
    this->activateWindow();
    Q_EMIT this->mainWindowVisibleChanged(true);
#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())){
        if (nullptr != m_wlanWidget) {
            m_wlanWidget->hideSetting();
        }
        if (nullptr != m_lanWidget) {
            m_lanWidget->hideSetting();
        }
    } else {
        if (nullptr != m_wlanWidget) {
            m_wlanWidget->showSetting();
        }
        if (nullptr != m_lanWidget) {
            m_lanWidget->showSetting();
        }
    }
#endif

}

/**
 * @brief MainWindow::hideMainwindow 隐藏主页面时要进行的操作，后续可以添加到此函数
 */
void MainWindow::hideMainwindow()
{
    this->hide();
    Q_EMIT this->mainWindowVisibleChanged(false);
}

/**
 * @brief MainWindow::setWiredDefaultDevice 设置有线设备默认网卡
 */
void MainWindow::setWiredDefaultDevice(QString deviceName)
{
//    m_lanWidget->updateDefaultDevice(deviceName);
}

/**
 * @brief MainWindow::hideMainwindow 设置无线设备默认网卡
 */
void MainWindow::setWirelessDefaultDevice(QString deviceName)
{
//    m_wlanWidget->updateDefaultDevice(deviceName);
}

/**
 * @brief MainWindow::firstlyStart 一级启动，执行重要且不耗时的启动操作
 */
void MainWindow::firstlyStart()
{
    initWindowProperties();
    initTransparency();
    registerTrayIcon();
    initPanelGSettings();
    initUI();
    initDbusConnnect();
    initWindowTheme();
    initTrayIcon();
//    initPlatform();
    installEventFilter(this);
    m_secondaryStartTimer = new QTimer(this);
    connect(m_secondaryStartTimer, &QTimer::timeout, this, [ = ]() {
        m_secondaryStartTimer->stop();
        secondaryStart();//满足条件后执行比较耗时的二级启动
    });
    m_secondaryStartTimer->start(5 * 1000);

    m_createPagePtrMap.clear();

    m_networkMode = new NetworkMode(this);
    m_networkMode->initWiredNetworkMode();
    m_networkMode->initWirelessNetworkMode();

    //加载key ring
    agent_init();
}

/**
 * @brief MainWindow::secondaryStart 二级启动，可以将较耗时的初始化操作放到此处执行
 */
void MainWindow::secondaryStart()
{
    if (m_loadFinished)
        return;
    m_loadFinished = true;
}

/**
 * @brief MainWindow::initWindowProperties 初始化平台信息
 */
void MainWindow::initPlatform()
{
    char* projectName = kdk_system_get_projectName();
    QString strProjectName(projectName);
    free(projectName);
    projectName = NULL;
    if(v10Sp1.compare(strProjectName,Qt::CaseInsensitive) == 0) {
        unsigned int feature = kdk_system_get_productFeatures();
        if (feature == 3) {
            m_isShowInCenter = true;
        }
    } else if (intel.compare(strProjectName,Qt::CaseInsensitive) == 0) {
        m_isShowInCenter = true;
    }
    qDebug() << "projectName" << projectName << m_isShowInCenter;
}

/**
 * @brief MainWindow::initWindowProperties 初始化一些窗口属性
 */
void MainWindow::initWindowProperties()
{
    this->setWindowTitle(tr("lingmo-nm"));
    this->setWindowIcon(QIcon::fromTheme("lingmo-network", QIcon(":/res/x/setup.png")));
    this->setFixedSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT);
//    //绘制毛玻璃特效
    this->setAttribute(Qt::WA_TranslucentBackground, true);  //透明
    this->setFocusPolicy(Qt::NoFocus);

    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        QPainterPath path;
        auto rect = this->rect();
        //    path.addRoundedRect(rect, 12, 12);
        path.addRect(rect);
        KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));   //背景模糊
    }
}

/**
 * @brief MainWindow::registerTrayIcon 注册托盘图标
 */
void MainWindow::registerTrayIcon()
{
    m_registerCount++;
    if (QSystemTrayIcon::isSystemTrayAvailable() || m_registerCount > 10) {
        m_trayIcon = new QSystemTrayIcon();
        if (nullptr == m_trayIcon) {
            qWarning()<< "分配空间trayIcon失败";
            return ;
        }
        m_trayIcon->setIcon(QIcon::fromTheme("network-wired-signal-excellent-symbolic"));
        m_trayIcon->setToolTip(QString(tr("lingmo-nm")));

    } else {
        if (m_registerCount <= 10) {
            QTimer::singleShot(m_intervalTime,[this] {
                registerTrayIcon();
            });
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setPen(Qt::transparent);
//    auto rect = this->rect();
//    painter.drawRoundedRect(rect, 12, 12);      //窗口圆角
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        //失焦退出
        if (event->type() == QEvent::ActivationChange) {
            if (QApplication::activeWindow() != this) {
                hideMainwindow();
                return true;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void MainWindow::initTransparency()
{
    if(QGSettings::isSchemaInstalled(TRANSPARENCY_GSETTINGS)) {
        m_transGsettings = new QGSettings(TRANSPARENCY_GSETTINGS);
        if(m_transGsettings->keys().contains(QString("transparency"))) {
            m_transparency=m_transGsettings->get("transparency").toDouble() + 0.15;
            m_transparency = (m_transparency > 1) ? 1 : m_transparency;
            connect(m_transGsettings, &QGSettings::changed, this, &MainWindow::onTransChanged);
        }
    }
}

void MainWindow::onTransChanged()
{
    m_transparency = m_transGsettings->get("transparency").toDouble() + 0.15;
    m_transparency = (m_transparency > 1) ? 1 : m_transparency;
    paintWithTrans();
}

void MainWindow::paintWithTrans()
{
    QPalette pal = m_centralWidget->palette();
    QColor color = this->palette().base().color();
    color.setAlphaF(m_transparency);
    pal.setColor(QPalette::Base, color);
    m_centralWidget->setPalette(pal);

    QPalette tabPal = m_centralWidget->tabBar()->palette();
    tabPal.setColor(QPalette::Base, color);

    QColor inactiveColor = this->palette().window().color();
    inactiveColor.setAlphaF(0.86 *m_transparency);
    tabPal.setColor(QPalette::Window, inactiveColor);

    m_centralWidget->tabBar()->setPalette(tabPal);
}

/**
 * @brief MainWindow::initPanelGSettings 获取任务栏位置和大小
 */
void MainWindow::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        if (m_panelGSettings == nullptr) {
            m_panelGSettings = new QGSettings(id, QByteArray(), this);
        }
        if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        }
        if (m_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
            m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        }
        connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString &key) {
            if (key == PANEL_POSITION_KEY) {
                m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
            }
            if (key == PANEL_SIZE_KEY) {
                m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
            }
            if (this->isVisible()) {
                resetWindowPosition();
            }
        });
    }
}

/**
 * @brief MainWindow::initUI 初始化窗口内控件
 */
void MainWindow::initUI()
{
    setThemePalette();
    m_centralWidget = new QTabWidget(this);
    this->setCentralWidget(m_centralWidget);
    m_centralWidget->tabBar()->setFixedWidth(this->width()+1);
//    m_centralWidget->tabBar()->setProperty("setRadius", 12);
//    m_centralWidget->tabBar()->setStyleSheet("QTabBar::tab{min-height:40px}");
    m_lanWidget = new LanPage(m_centralWidget);
    m_wlanWidget = new WlanPage(m_centralWidget);
    connect(this, &MainWindow::mainWindowVisibleChanged, m_wlanWidget, &WlanPage::onMainWindowVisibleChanged);
//    m_centralWidget->addTab(m_lanWidget, QIcon::fromTheme("network-wired-connected-symbolic", QIcon::fromTheme("network-wired-symbolic", QIcon(":/res/l/network-online.svg"))), tr("LAN"));
//    m_centralWidget->addTab(m_wlanWidget, QIcon::fromTheme("network-wireless-signal-excellent-symbolic", QIcon(":/res/x/wifi-list-bg.svg")), tr("WLAN"));

    m_centralWidget->addTab(m_lanWidget, tr(""));
    m_centralWidget->addTab(m_wlanWidget,tr(""));
    m_tabBarLayout = new QHBoxLayout(this);
    m_tabBarLayout->setContentsMargins(LAYOUT_MARGINS);
    m_lanLabel = new QLabel(tr("LAN"));
    m_lanLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    m_wlanLabel = new QLabel(tr("WLAN"));
    m_wlanLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    m_tabBarLayout->addWidget(m_lanLabel);
    m_tabBarLayout->addWidget(m_wlanLabel);
    m_centralWidget->tabBar()->setLayout(m_tabBarLayout);
    m_centralWidget->tabBar()->setProperty("useTabbarSeparateLine", false);     // 去掉中间的分割线
    m_centralWidget->setAttribute(Qt::WA_TranslucentBackground, true);          // 背景透明 解决切换黑屏问题
    connect(m_centralWidget, &QTabWidget::currentChanged, m_wlanWidget, &WlanPage::onWlanPageVisibleChanged);
    paintWithTrans();
}

/**
 * @brief MainWindow::initTrayIcon 初始化托盘图标和托盘右键菜单
 */
void MainWindow::initTrayIcon()
{
    loadIcons.append(QIcon::fromTheme("lingmo-loading-0-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-1-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-2-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-3-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-4-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-5-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-6-symbolic"));
    loadIcons.append(QIcon::fromTheme("lingmo-loading-7-symbolic"));
    iconTimer = new QTimer(this);
    connect(iconTimer, &QTimer::timeout, this, &MainWindow::onSetTrayIconLoading);

    m_trayIconMenu = new QMenu();
//    m_showMainwindowAction = new QAction(tr("Show MainWindow"),this);
    m_showSettingsAction = new QAction(tr("Settings"),this);
    m_showConnectivityPageAction = new QAction(tr("Network Connectivity Detection"), this);

    m_showSettingsAction->setIcon(QIcon::fromTheme("document-page-setup-symbolic", QIcon(":/res/x/setup.png")) );
    m_showConnectivityPageAction->setIcon(QIcon::fromTheme("gnome-netstatus-txrx"));
//    m_trayIconMenu->addAction(m_showMainwindowAction);
    m_trayIconMenu->addAction(m_showSettingsAction);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    iconStatus = IconActiveType::LAN_CONNECTED;
    onRefreshTrayIcon();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
//    connect(m_showMainwindowAction, &QAction::triggered, this, &MainWindow::onShowMainwindowActionTriggled);
    connect(m_showSettingsAction, &QAction::triggered, this, &MainWindow::onShowSettingsActionTriggled);
//    connect(m_showConnectivityPageAction, &QAction::triggered, [=]() {
//        if (m_connectivityPage != nullptr) {
//            KWindowSystem::activateWindow(m_connectivityPage->winId());
//            KWindowSystem::raiseWindow(m_connectivityPage->winId());
//            return;
//        }
//        QString uri = getConnectivityCheckSpareUriByGDbus();
//        m_connectivityPage = new ConnectivityPage(uri, this);
//        connect(m_connectivityPage, &ConnectivityPage::pageClose, [&](){
//            m_connectivityPage = nullptr;
//        });
//        m_connectivityPage->show();
//    });

    m_trayIcon->show();
}

void MainWindow::initDbusConnnect()
{
    connect(m_lanWidget, &LanPage::deviceStatusChanged, this, &MainWindow::deviceStatusChanged);
    connect(m_lanWidget, &LanPage::deviceNameChanged, this, &MainWindow::deviceNameChanged);

    connect(m_wlanWidget, &WlanPage::wirelessDeviceStatusChanged, this, &MainWindow::wirelessDeviceStatusChanged);
    connect(m_wlanWidget, &WlanPage::deviceNameChanged, this, &MainWindow::deviceNameChanged);
    connect(m_wlanWidget, &WlanPage::wirelessSwitchBtnChanged, this, &MainWindow::wirelessSwitchBtnChanged);

    connect(m_wlanWidget, &WlanPage::activateFailed, this, &MainWindow::activateFailed);
    connect(m_wlanWidget, &WlanPage::deactivateFailed, this, &MainWindow::deactivateFailed);
    connect(m_lanWidget, &LanPage::activateFailed, this, &MainWindow::activateFailed);
    connect(m_lanWidget, &LanPage::deactivateFailed, this, &MainWindow::deactivateFailed);

    connect(m_lanWidget, &LanPage::lanAdd, this, &MainWindow::lanAdd);
    connect(m_lanWidget, &LanPage::lanRemove, this, &MainWindow::lanRemove);
    connect(m_lanWidget, &LanPage::lanUpdate, this, &MainWindow::lanUpdate);
    connect(m_lanWidget, &LanPage::lanActiveConnectionStateChanged, this, &MainWindow::lanActiveConnectionStateChanged);
    connect(m_lanWidget, &LanPage::lanConnectChanged, this, &MainWindow::onLanConnectStatusToChangeTrayIcon);


    connect(m_wlanWidget, &WlanPage::wlanAdd, this, &MainWindow::wlanAdd);
    connect(m_wlanWidget, &WlanPage::wlanRemove, this, &MainWindow::wlanRemove);
    connect(m_wlanWidget, &WlanPage::wlanActiveConnectionStateChanged, this, &MainWindow::wlanactiveConnectionStateChanged);
    connect(m_wlanWidget, &WlanPage::wlanConnectChanged, this, &MainWindow::onWlanConnectStatusToChangeTrayIcon);
    connect(m_wlanWidget, &WlanPage::hotspotDeactivated, this, &MainWindow::hotspotDeactivated);
    connect(m_wlanWidget, &WlanPage::hotspotActivated, this, &MainWindow::hotspotActivated);
    connect(m_wlanWidget, &WlanPage::secuTypeChange, this, &MainWindow::secuTypeChange);
    connect(m_wlanWidget, &WlanPage::signalStrengthChange, this, &MainWindow::signalStrengthChange);
    connect(m_wlanWidget, &WlanPage::timeToUpdate , this, &MainWindow::timeToUpdate);

    connect(m_wlanWidget, &WlanPage::timeToUpdate , this, &MainWindow::onTimeUpdateTrayIcon);
    connect(m_wlanWidget, &WlanPage::showMainWindow, this, &MainWindow::onShowMainWindow);
    connect(m_wlanWidget, &WlanPage::connectivityChanged, this, &MainWindow::onConnectivityChanged);
    connect(m_wlanWidget, &WlanPage::connectivityCheckSpareUriChanged, this, &MainWindow::onConnectivityCheckSpareUriChanged);

    connect(m_lanWidget, &LanPage::lanConnectChanged, this, &MainWindow::onRefreshTrayIconTooltip);
    connect(m_lanWidget, &LanPage::deviceStatusChanged, this, &MainWindow::onRefreshTrayIconTooltip);
    connect(m_wlanWidget, &WlanPage::wlanConnectChanged, this, &MainWindow::onRefreshTrayIconTooltip);
    connect(m_wlanWidget, &WlanPage::wirelessDeviceStatusChanged, this, &MainWindow::onRefreshTrayIconTooltip);

    //模式切换
    QDBusConnection::sessionBus().connect(QString("com.lingmo.statusmanager.interfacer"),
                                         QString("/"),
                                         QString("com.lingmo.statusmanager.interface"),
                                         QString("mode_change_signal"), this, SLOT(onTabletModeChanged(bool)));

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this,[&](WId activeWindowId){
        if (activeWindowId != this->winId() && activeWindowId != 0) {
            qDebug() << "tray recieve KWindowSystem activeWindowChanged" << activeWindowId << this->winId();
            hideMainwindow();
        }
    });

    QDBusConnection::sessionBus().connect(QString("com.lingmo.network"),
                                         QString("/com/lingmo/network"),
                                         QString("com.lingmo.network"),
                                          QString("showLingmoNMSignal"), this, SLOT(onShowLingmoNMSlot(QString,int)));

    QDBusConnection::sessionBus().connect(QString("com.lingmo.network"),
                                         QString("/com/lingmo/network"),
                                         QString("com.lingmo.network"),
                                          QString("showPropertyWidgetSignal"), this, SLOT(onShowPropertyWidgetSlot(QString,QString,QString)));

    QDBusConnection::sessionBus().connect(QString("com.lingmo.network"),
                                         QString("/com/lingmo/network"),
                                         QString("com.lingmo.network"),
                                          QString("showCreateWiredConnectWidgetSignal"), this, SLOT(onShowCreateWiredConnectWidgetSlot(QString,QString)));

    QDBusConnection::sessionBus().connect(QString("com.lingmo.network"),
                                         QString("/com/lingmo/network"),
                                         QString("com.lingmo.network"),
                                          QString("showAddOtherWlanWidgetSignal"), this, SLOT(onShowAddOtherWlanWidgetSlot(QString,QString)));

}

/**
 * @brief MainWindow::resetWindowPosition 重新计算窗口位置
 */
void MainWindow::resetWindowPosition()
{
    if (m_isShowInCenter) {
        QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
        QRect rect((availableGeometry.width() - this->width())/2, (availableGeometry.height() - this->height())/2,
                   this->width(), this->height());
        kdk::WindowManager::setGeometry(this->windowHandle(), rect);

        return;
    }

#define MARGIN 8
#define PANEL_TOP 1
#define PANEL_LEFT 2
#define PANEL_RIGHT 3
#define PANEL_Bottom 0
//#define PANEL_BOTTOM 4

    QRect availableGeo = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int x, y;

    switch (m_panelPosition)
    {
    case PANEL_TOP:
    {
        char *envStr = getenv("LANGUAGE");
        /* 维吾尔语 ug_CN
         * 哈萨克语 kk_KZ
         * 柯尔克孜语 ky_KG */
        if (strcmp(envStr, "ug_CN") == 0 || strcmp(envStr, "kk_KZ") == 0 || strcmp(envStr, "ky_KG") == 0) {
            x = MARGIN;
            y = availableGeo.y() + m_panelSize + MARGIN;
        }
        else {
            x = availableGeo.x() + availableGeo.width() - this->width() - MARGIN;
            y = availableGeo.y() + m_panelSize + MARGIN;
        }

    }
        break;
    case PANEL_Bottom:
    {
        char *envStr = getenv("LANGUAGE");
        if (strcmp(envStr, "ug_CN") == 0 || strcmp(envStr, "kk_KZ") == 0 || strcmp(envStr, "ky_KG") == 0) {
            x = MARGIN;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - MARGIN;
        }
        else {
            x = availableGeo.x() + availableGeo.width() - this->width() - MARGIN;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - MARGIN;
        }

    }
        break;
    case PANEL_LEFT:
    {
        x = availableGeo.x() + m_panelSize + MARGIN;
        y = availableGeo.y() + availableGeo.height() - this->height() - MARGIN;
    }
        break;
    case PANEL_RIGHT:
    {
        x = availableGeo.x() + availableGeo.width() - m_panelSize - this->width() - MARGIN;
        y = availableGeo.y() + availableGeo.height() - this->height() - MARGIN;
    }
        break;
    }

    kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x, y, this->width(), this->height()));
    qDebug() << " Position of lingmo-panel is " << m_panelPosition << "; Position of mainwindow is " << this->geometry() << "." << Q_FUNC_INFO << __LINE__;
}

/**
 * @brief MainWindow::resetTrayIconTool 重新获取网络连接状态并重新设置图标和tooltip
 */
void MainWindow::resetTrayIconTool()
{
    //ZJP_TODO 检测当前连接的是有线还是无线，是否可用，设置图标和tooltip,图标最好提前define
//    int connectivity = objKyDBus->getNetworkConectivity();
//    qDebug() << "Value of current network Connectivity property : "<< connectivity;
//    switch (connectivity) {
//    case UnknownConnectivity:
//    case Portal:
//    case Limited:
//        setTrayIcon(iconLanOnlineNoInternet);
//        trayIcon->setToolTip(QString(tr("Network Connected But Can Not Access Internet")));
//        break;
//    case NoConnectivity:
//    case Full:
//        setTrayIcon(iconLanOnline);
//        trayIcon->setToolTip(QString(tr("lingmo-nm")));
//        break;
//    }
    qDebug() << "Has set tray icon to be XXX." << Q_FUNC_INFO << __LINE__;
}


/**
 * @brief MainWindow::initWindowTheme 初始化窗口主题并创建信号槽
 */
void MainWindow::initWindowTheme()
{
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
        m_styleGsettings = new QGSettings(style_id);
//        resetWindowTheme();
        connect(m_styleGsettings, &QGSettings::changed, this, &MainWindow::onThemeChanged);
    } else {
        qWarning() << "Gsettings interface \"org.lingmo.style\" is not exist!" << Q_FUNC_INFO << __LINE__;
    }
}

/**
 * @brief MainWindow::resetWindowTheme 读取和设置窗口主题
 */
void MainWindow::resetWindowTheme()
{
    if (!m_styleGsettings) { return; }
    QString currentTheme = m_styleGsettings->get(COLOR_THEME).toString();
    auto app = static_cast<QApplication*>(QCoreApplication::instance());
    if(currentTheme == "lingmo-dark" || currentTheme == "lingmo-black"){
        app->setStyle(new CustomStyle("lingmo-dark"));
        qDebug() << "Has set color theme to lingmo-dark." << Q_FUNC_INFO << __LINE__;
        Q_EMIT qApp->paletteChanged(qApp->palette());
        return;
    }
    app->setStyle(new CustomStyle("lingmo-light"));
    qDebug() << "Has set color theme to " << currentTheme << Q_FUNC_INFO << __LINE__;
    Q_EMIT qApp->paletteChanged(qApp->palette());
    return;
}

/**
 * @brief MainWindow::showControlCenter 打开控制面板网络界面
 */
void MainWindow::showControlCenter()
{
    QProcess process;
    if (!m_lanWidget->lanIsConnected() && m_wlanWidget->checkWlanStatus(NetworkManager::ActiveConnection::State::Activated)){
        process.startDetached("lingmo-control-center -m wlanconnect");
    } else {
        process.startDetached("lingmo-control-center -m netconnect");
    }
}

void MainWindow::showByWaylandHelper()
{
    //去除窗管标题栏，传入参数为QWidget*
    kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    this->show();
    this->setFocus();
    QWindow* window = this->windowHandle();
    if (window) {
        //跳过任务栏和分页器的属性
        kdk::WindowManager::setSkipSwitcher(window, true);
        kdk::WindowManager::setSkipTaskBar(window, true);
    }
    resetWindowPosition();
    //设置窗体位置，传入参数为QWindow*，QRect

}

void MainWindow::setCentralWidgetType(IconActiveType iconStatus)
{
    if (iconStatus == WLAN_CONNECTED || iconStatus == WLAN_CONNECTED_LIMITED) {
         m_centralWidget->setCurrentIndex(WLAN_PAGE_INDEX);
     } else if (iconStatus == ACTIVATING) {
         if (m_wlanWidget->checkWlanStatus(NetworkManager::ActiveConnection::State::Activating)) {
             m_centralWidget->setCurrentIndex(WLAN_PAGE_INDEX);
         } else {
             m_centralWidget->setCurrentIndex(LAN_PAGE_INDEX);
         }
     } else {
         m_centralWidget->setCurrentIndex(LAN_PAGE_INDEX);
    }
}

/**
 * @brief MainWindow::assembleTrayIconTooltip 整理托盘图标tooltip内容
 * @param map <网卡名，网络状态>
 * @param tip tooltip
 */
void MainWindow::assembleTrayIconTooltip(QMap<QString, QString> &map, QString &tip)
{
    if (map.isEmpty()) {
        tip = QString(tr("Network tool"));
        return;
    }
    QMap<QString, QString>::iterator iter = map.begin();
    if (map.size() == 1) {
        tip = map.value(iter.key());
        if (tip.indexOf(":")) {
            tip = tip.mid(tip.indexOf(":") + 2); //单网卡显示时去掉“已连接: ”字样
        }
    } else if (map.size() > 1) {
        tip = "";
        int count = 0;
        while (iter != map.end()) {
            count += 1;
            tip += QString(tr("Network Card")) + QString("%1").arg(count) + "\n" + map.value(iter.key());
            ++iter;

            if (iter != map.end()) {
                tip += "\n";
            }
        }
    }
}

void MainWindow::setThemePalette()
{
    QPalette pal = qApp->palette();
    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id, QByteArray(), this);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = themePalette(true, this);
       }
    }
    this->setPalette(pal);
}

/**
 * @brief MainWindow::onTrayIconActivated 点击托盘图标的槽函数
 */
void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    setCentralWidgetType(iconStatus);
    if (reason == QSystemTrayIcon::ActivationReason::Context) {
            m_trayIconMenu->popup(QCursor::pos());
    } else {
        if (this->isVisible()) {
            qDebug() << "Received signal of tray icon activated, will hide mainwindow." << Q_FUNC_INFO << __LINE__;
            hideMainwindow();
            return;
        }
        qDebug() << "Received signal of tray icon activated, will show mainwindow." << Q_FUNC_INFO << __LINE__;
        this->showMainwindow();
    }
}

void MainWindow::onShowMainwindowActionTriggled()
{
    showMainwindow();
}

void MainWindow::onShowSettingsActionTriggled()
{
    showControlCenter();
}

void MainWindow::onThemeChanged(const QString &key)
{
    if (key == COLOR_THEME) {
        qDebug() << "Received signal of theme changed, will reset theme." << Q_FUNC_INFO << __LINE__;
//        resetWindowTheme();
        setThemePalette();
        paintWithTrans();
        Q_EMIT qApp->paletteChanged(qApp->palette());
    } else if ("themeColor" == key) {
        setThemePalette();
    } else {
        qDebug() << "Received signal of theme changed, key=" << key << " will do nothing." << Q_FUNC_INFO << __LINE__;
    }
}

void MainWindow::onRefreshTrayIcon()
{
    //更新托盘图标显示
    int signalStrength = 0;
    iconTimer->stop();
    if (m_lanWidget->lanIsConnected()) {
        m_trayIcon->setIcon(QIcon::fromTheme("network-wired-connected-symbolic"));
        iconStatus = IconActiveType::LAN_CONNECTED;
    } else if (m_wlanWidget->checkWlanStatus(NetworkManager::ActiveConnection::State::Activated)){
//        m_trayIcon->setIcon(QIcon::fromTheme("network-wireless-connected-symbolic"));
        signalStrength = m_wlanWidget->getActivateWifiSignal(m_wlanWidget->getCurrentDisplayDevice());
        if (signalStrength == -1) {
            signalStrength = m_wlanWidget->getActivateWifiSignal();
        }
        iconStatus = IconActiveType::WLAN_CONNECTED;
    } else {
        m_trayIcon->setIcon(QIcon::fromTheme("network-wired-disconnected-symbolic"));
        iconStatus = IconActiveType::NOT_CONNECTED;
    }

    NetworkManager::Connectivity connecttivity;
    m_wlanWidget->getConnectivity(connecttivity);
    if (connecttivity != NetworkManager::Connectivity::Full) {
        if (iconStatus == IconActiveType::LAN_CONNECTED) {
            m_trayIcon->setIcon(QIcon::fromTheme("network-error-symbolic"));
            iconStatus = IconActiveType::LAN_CONNECTED_LIMITED;
        } else if (iconStatus == IconActiveType::WLAN_CONNECTED) {
            //todo 信号强度
//            m_trayIcon->setIcon(QIcon::fromTheme("network-wireless-signal-excellent-error-symbolic"));
            iconStatus = IconActiveType::WLAN_CONNECTED_LIMITED;
        }
    }

    if (iconStatus == IconActiveType::WLAN_CONNECTED
            || iconStatus == IconActiveType::WLAN_CONNECTED_LIMITED) {
        if (signalStrength > MW_EXCELLENT_SIGNAL){
            m_trayIcon->setIcon(QIcon::fromTheme(EXCELLENT_SIGNAL_ICON));
        } else if (signalStrength > MW_GOOD_SIGNAL) {
            m_trayIcon->setIcon(QIcon::fromTheme(GOOD_SIGNAL_ICON));
        } else if (signalStrength > MW_OK_SIGNAL) {
            m_trayIcon->setIcon(QIcon::fromTheme(OK_SIGNAL_ICON));
        } else if (signalStrength > MW_LOW_SIGNAL) {
            m_trayIcon->setIcon(QIcon::fromTheme(LOW_SIGNAL_ICON));
        } else {
            m_trayIcon->setIcon(QIcon::fromTheme(NONE_SIGNAL_ICON));
        }
    } else if (iconStatus == IconActiveType::WLAN_CONNECTED_LIMITED) {
        if (signalStrength > MW_EXCELLENT_SIGNAL){
            m_trayIcon->setIcon(QIcon::fromTheme(EXCELLENT_SIGNAL_LIMIT_ICON));
        } else if (signalStrength > MW_GOOD_SIGNAL) {
            m_trayIcon->setIcon(QIcon::fromTheme(GOOD_SIGNAL_LIMIT_ICON));
        } else if (signalStrength > MW_OK_SIGNAL) {
            m_trayIcon->setIcon(QIcon::fromTheme(OK_SIGNAL_LIMIT_ICON));
        } else if (signalStrength > MW_LOW_SIGNAL) {
            m_trayIcon->setIcon(QIcon::fromTheme(LOW_SIGNAL_LIMIT_ICON));
        } else {
            m_trayIcon->setIcon(QIcon::fromTheme(NONE_SIGNAL_LIMIT_ICON));
        }
    }

//    if(!getConnectivityCheckSpareUriByGDbus().isEmpty()) {
//        if (iconStatus == IconActiveType::LAN_CONNECTED) {
//            m_trayIcon->setIcon(QIcon::fromTheme("network-intranet-symbolic"));
//        } else if (iconStatus == IconActiveType::WLAN_CONNECTED) {
//            if (signalStrength > MW_EXCELLENT_SIGNAL){
//                m_trayIcon->setIcon(QIcon::fromTheme(EXCELLENT_SIGNAL_INTRANET_ICON));
//            } else if (signalStrength > MW_GOOD_SIGNAL) {
//                m_trayIcon->setIcon(QIcon::fromTheme(GOOD_SIGNAL_INTRANET_ICON));
//            } else if (signalStrength > MW_OK_SIGNAL) {
//                m_trayIcon->setIcon(QIcon::fromTheme(OK_SIGNAL_INTRANET_ICON));
//            } else if (signalStrength > MW_LOW_SIGNAL) {
//                m_trayIcon->setIcon(QIcon::fromTheme(LOW_SIGNAL_INTRANET_ICON));
//            } else {
//                m_trayIcon->setIcon(QIcon::fromTheme(NONE_SIGNAL_INTRANET_ICON));
//            }
//        }
//    }

    if (signalStrength == -1) {
        m_trayIcon->setIcon(QIcon::fromTheme("network-wired-disconnected-symbolic"));
    }
    onRefreshTrayIconTooltip();

//    if (iconStatus > IconActiveType::NOT_CONNECTED) {
//        m_trayIconMenu->addAction(m_showConnectivityPageAction);
//    } else {
//        m_trayIconMenu->removeAction(m_showConnectivityPageAction);
//    }
}

void MainWindow::onSetTrayIconLoading()
{
    if (currentIconIndex > 7) {
        currentIconIndex = 0;
    }
    m_trayIcon->setIcon(loadIcons.at(currentIconIndex));
    iconStatus = IconActiveType::ACTIVATING;
    currentIconIndex ++;
}

void MainWindow::onLanConnectStatusToChangeTrayIcon(int state)
{
    qDebug() << "lan state:" << state << Q_FUNC_INFO << __LINE__;
    if (state==1 || state==3){
        m_lanIsLoading = true;
        iconTimer->start(LOADING_TRAYICON_TIMER_MS);
    } else {
        m_lanIsLoading = false;
        if (m_wlanIsLoading == false) {
            onRefreshTrayIcon();
        }
    }
}

void MainWindow::onWlanConnectStatusToChangeTrayIcon(int state)
{
    qDebug() << "wlan state:" << state << Q_FUNC_INFO << __LINE__;
    if (state==1 || state==3){
        m_wlanIsLoading = true;
        iconTimer->start(LOADING_TRAYICON_TIMER_MS);
    } else {
        if (m_wlanWidget->checkWlanStatus(NetworkManager::ActiveConnection::State::Activating)) {
            return;
        }
        m_wlanIsLoading = false;
        if (m_lanIsLoading == false) {
            onRefreshTrayIcon();
        }
    }
}

void MainWindow::onTabletModeChanged(bool mode)
{
    qDebug() << "TabletMode change" << mode;
    Q_UNUSED(mode)
    //模式切换时，隐藏主界面
    hideMainwindow();
}

/**
 * @brief MainWindow::onRefreshTrayIconTooltip 根据托盘图标调整其tooltip
 */
void MainWindow::onRefreshTrayIconTooltip()
{
    if (!m_trayIcon) {
        return;
    }

    QString trayIconToolTip = "";
    QMap<QString, QString> lanMap;
    QMap<QString, QString> wlanMap;
    switch(iconStatus) {
    case IconActiveType::NOT_CONNECTED:
        trayIconToolTip = QString(tr("Not connected to the network"));
        break;

    case LAN_CONNECTED:
    case IconActiveType::LAN_CONNECTED_LIMITED:
        m_lanWidget->getWiredDeviceConnectState(lanMap);
        assembleTrayIconTooltip(lanMap, trayIconToolTip);
        break;

    case IconActiveType::WLAN_CONNECTED:
    case IconActiveType::WLAN_CONNECTED_LIMITED:
        m_wlanWidget->getWirelssDeviceConnectState(wlanMap);
        assembleTrayIconTooltip(wlanMap, trayIconToolTip);
        break;

    case IconActiveType::ACTIVATING:
    default:
        trayIconToolTip = QString(tr("Network tool"));
        break;
    }

    m_trayIcon->setToolTip(trayIconToolTip);
}

void MainWindow::onShowMainWindow(int type)
{
    if (type == LANPAGE || type == WLANPAGE) {
        m_centralWidget->setCurrentIndex(type);

        if(QApplication::activeWindow() != this) {
            this->showMainwindow();
        }
    } else if (type == AUTOSELET) {
        onTrayIconActivated(QSystemTrayIcon::ActivationReason::Trigger);
    } else {
        qWarning() << "unsupport parameter";
    }
}

void MainWindow::onConnectivityChanged(NetworkManager::Connectivity connectivity)
{
    if (!m_trayIcon) {
        return;
    }

    if (iconStatus == ACTIVATING) {
        return;
    }

    onRefreshTrayIcon();
}

void MainWindow::onConnectivityCheckSpareUriChanged()
{
    if (!m_trayIcon) {
        return;
    }

    if (iconStatus == ACTIVATING) {
        return;
    }

    onRefreshTrayIcon();
}

void MainWindow::onTimeUpdateTrayIcon()
{
    if (!m_trayIcon) {
        return;
    }

    if (iconStatus == ACTIVATING || (iconStatus != WLAN_CONNECTED && iconStatus != WLAN_CONNECTED_LIMITED)) {
        return;
    }

    onRefreshTrayIcon();
}

/**
 * @brief MainWindow::keyPressEvent 按esc键关闭主界面
 * @param event
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideMainwindow();
    }
    return QWidget::keyPressEvent(event);
}

/**
 * @brief MainWindow::getWirelessList 获取wifi列表，供dbus调用
 * @param devName
 * @param list
 */
void MainWindow::getWirelessList(QString devName, QList<QStringList> &list)
{
    list.clear();
    if (nullptr != m_wlanWidget) {
        m_wlanWidget->getWirelessList(devName, list);
    }
}

bool MainWindow::getWirelessSwitchBtnState()
{
    if (nullptr != m_wlanWidget) {
        return m_wlanWidget->getWirelessSwitchBtnState();
    }
}

/**
 * @brief MainWindow::getWiredList 获取lan列表，供dbus调用
 * @param map
 */
void MainWindow::getWiredList(QString devName, QList<QStringList> &list)
{
    list.clear();
    if (nullptr != m_lanWidget) {
        m_lanWidget->getWiredList(devName, list);
    }
}

/**
 * @brief MainWindow::activeWirelessAp 开启热点，供dbus调用
 * @param apName
 * @param apPassword
 * @param apDevice
 */
void MainWindow::activeWirelessAp(const QString apName, const QString apPassword, const QString band, const QString apDevice)
{
    m_wlanWidget->activeWirelessAp(apName, apPassword, band, apDevice);
}

/**
 * @brief MainWindow::activeWirelessAp 断开热点，供dbus调用
 * @param apName
 */
void MainWindow::deactiveWirelessAp(const QString apName, const QString uuid)
{
    m_wlanWidget->deactiveWirelessAp(apName, uuid);
}

/**
 * @brief MainWindow::activeWirelessAp 获取热点，供dbus调用
 * @param list
 */
void MainWindow::getApInfoBySsid(QString devName, QString ssid, QStringList &list)
{
    m_wlanWidget->getApInfoBySsid(devName, ssid, list);
}

void MainWindow::getStoredApInfo(QStringList &list)
{
    m_wlanWidget->getStoredApInfo(list);
}

void MainWindow::getApConnectionPath(QString &path, QString uuid)
{
    m_wlanWidget->getApConnectionPath(path, uuid);
}

void MainWindow::getActiveConnectionPath(QString &path, QString uuid)
{
    m_wlanWidget->getActiveConnectionPath(path, uuid);
}

//无线开关
void MainWindow::setWirelessSwitchEnable(bool enable)
{
    m_wlanWidget->setWirelessSwitchEnable(enable);
}

void MainWindow::setWiredDeviceEnable(const QString& devName, bool enable)
{
    m_lanWidget->setWiredDeviceEnable(devName, enable);
}
void MainWindow::showPropertyWidget(QString devName, QString ssid)
{
    KyNetworkDeviceResourse *devResourse = new KyNetworkDeviceResourse();
    QStringList wiredDeviceList;
    wiredDeviceList.clear();
    devResourse->getNetworkDeviceList(NetworkManager::Device::Type::Ethernet, wiredDeviceList);
    if (wiredDeviceList.contains(devName)) {
      qDebug() <<   "showPropertyWidget device type wired device name " << devName << " uuid " << ssid;
      m_lanWidget->showDetailPage(devName, ssid);
      delete devResourse;
      devResourse = nullptr;
      return;
    }

    QStringList wirelessDeviceList;
    wirelessDeviceList.clear();
    devResourse->getNetworkDeviceList(NetworkManager::Device::Type::Wifi, wirelessDeviceList);
    if (wirelessDeviceList.contains(devName)) {
      qDebug() <<   "showPropertyWidget device type wireless device name " << devName << " ssid " << ssid;
      m_wlanWidget->showDetailPage(devName, ssid);
      delete devResourse;
      devResourse = nullptr;
      return;
    }

    qWarning() <<   "showPropertyWidget no such device " << devName;
    delete devResourse;
    devResourse = nullptr;
}

void MainWindow::showCreateWiredConnectWidget(const QString devName)
{
    qDebug() << "showCreateWiredConnectWidget! devName = " << devName;
    if (m_createPagePtrMap.contains(devName)) {
        if (m_createPagePtrMap[devName] != nullptr) {
            qDebug() << "showCreateWiredConnectWidget" << devName << "already create,just raise";

            KWindowSystem::activateWindow(m_createPagePtrMap[devName]->winId());
            KWindowSystem::raiseWindow(m_createPagePtrMap[devName]->winId());
            return;
        }
    }
    NetDetail *netDetail = new NetDetail(devName, "", "", false, false, true, this);
    connect(netDetail, &NetDetail::createPageClose, [&](QString interfaceName){
        if (m_createPagePtrMap.contains(interfaceName)) {
            m_createPagePtrMap[interfaceName] = nullptr;
        }
    });
    m_createPagePtrMap.insert(devName, netDetail);
    netDetail->show();
    KWindowSystem::raiseWindow(netDetail->winId());
    netDetail->centerToScreen();
    kdk::WindowManager::setSkipSwitcher(netDetail->windowHandle(), true);
    kdk::WindowManager::setSkipTaskBar(netDetail->windowHandle(), true);
    kdk::WindowManager::setIconName(netDetail->windowHandle(), "lingmo-network");
}

void MainWindow::showAddOtherWlanWidget(QString devName)
{
    qDebug() << "showAddOtherWlanWidget! devName = " << devName;
    m_wlanWidget->showAddOtherPage(devName);
}

void MainWindow::getWirelessDeviceCap(QMap<QString, int> &map)
{
    m_wlanWidget->getWirelessDeviceCap(map);
}

//有线连接删除
void MainWindow::deleteWired(const QString &connUuid)
{
    m_lanWidget->deleteWired(connUuid);
}

//有线连接断开
void MainWindow::activateWired(const QString& devName, const QString& connUuid)
{
    m_lanWidget->activateWired(devName, connUuid);
}
void MainWindow::deactivateWired(const QString& devName, const QString& connUuid)
{
    m_lanWidget->deactivateWired(devName, connUuid);
}

//无线连接断开
void MainWindow::activateWireless(const QString& devName, const QString& ssid)
{
    m_wlanWidget->activateWirelessConnection(devName, ssid);
}

void MainWindow::deactivateWireless(const QString& devName, const QString& ssid)
{
    m_wlanWidget->deactivateWirelessConnection(devName, ssid);
}

void MainWindow::rescan()
{
    m_wlanWidget->requestScan();
}

void MainWindow::keyRingInit()
{
    agent_init();
}

void MainWindow::keyRingClear()
{
    agent_clear();
}

void MainWindow::onShowLingmoNMSlot(QString display, int type)
{
    if (display == m_display) {
        onShowMainWindow(type);
    }
}

//唤起属性页 根据网卡类型 参数2 为ssid/uuid
void MainWindow::onShowPropertyWidgetSlot(QString display, QString devName, QString ssid)
{
    if (display == m_display) {
        showPropertyWidget(devName, ssid);
    }
}
//唤起新建有线连接界面
void MainWindow::onShowCreateWiredConnectWidgetSlot(QString display, QString devName)
{
    if (display == m_display) {
        showCreateWiredConnectWidget(devName);
    }
}
//唤起加入其他无线网络界面
void MainWindow::onShowAddOtherWlanWidgetSlot(QString display, QString devName)
{
    if (display == m_display) {
        showAddOtherWlanWidget(devName);
    }
}
