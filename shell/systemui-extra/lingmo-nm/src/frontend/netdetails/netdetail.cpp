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
#include "netdetail.h"
//#include "xatom/xatom-helper.h"

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"

#include <QEvent>
#include <QMenu>
#include <QToolTip>
#include <QFontMetrics>
#include <QScrollBar>

#include "windowmanager/windowmanager.h"
#include "kwindowsystem.h"
#include "kwindowsystem_export.h"

#define  WINDOW_WIDTH  520
#define  WINDOW_HEIGHT 602
#define  ICON_SIZE 22,22
#define  TITLE_LAYOUT_MARGINS 9,9,0,0
#define  CENTER_LAYOUT_MARGINS 24,0,0,0
#define  BOTTOM_LAYOUT_MARGINS 24,0,24,0
#define  BOTTOM_LAYOUT_SPACING 16

#define  DETAIL_PAGE_NUM 0
#define  IPV4_PAGE_NUM 1
#define  IPV6_PAGE_NUM 2
#define  SECURITY_PAGE_NUM 3
#define  CONFIG_PAGE_NUM 4
#define  CREATE_NET_PAGE_NUM 5
#define  PAGE_MIN_HEIGHT 40
#define  PAGE_WIDTH 472
#define  LAN_TAB_WIDTH 180
#define  WLAN_TAB_WIDTH 240
#define  SCRO_WIDTH 496
#define  SCRO_HEIGHT 600
#define  PEAP_SCRO_HEIGHT  300
#define  TLS_SCRO_HEIGHT  480
#define  MAX_TAB_TEXT_LENGTH 44

#define  SYSTEM_DBUS_SERVICE  "com.lingmo.network.qt.systemdbus"
#define  SYSTEM_DBUS_PATH  "/"
#define  SYSTEM_DBUS_INTERFACE "com.lingmo.network.interface"

//extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

void NetDetail::showDesktopNotify(const QString &message, QString soundName)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QStringList actions;  //跳转动作
    actions.append("default");
    actions.append("lingmo-nm");          //默认动作：点击消息体时打开灵墨录音
    QMap<QString, QVariant> hints;
    if (!soundName.isEmpty()) {
        hints.insert("sound-name", soundName); //添加声音
    }
    QList<QVariant> args;
    args<<(tr("Lingmo NM"))
       <<((unsigned int) 0)
       <<QString("gnome-dev-ethernet")
       <<tr("lingmo network desktop message") //显示的是什么类型的信息
       <<message //显示的具体信息
       <<actions
       <<hints
       <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

void NetDetail::setNetdetailSomeEnable(bool on)
{
    detailPage->setEnabled(on);
    ipv4Page->setEnabled(on);
    ipv6Page->setEnabled(on);
    securityPage->setEnabled(on);
    createNetPage->setEnabled(on);
    cancelBtn->setEnabled(on);
    forgetBtn->setEnabled(on);
    confimBtn->setEnabled(on);
}

void NetDetail::startObjectThread()
{
    m_objectThread = new QThread();
    m_object = new ThreadObject(m_deviceName);
    m_object->moveToThread(m_objectThread);
    connect(m_objectThread, &QThread::finished, m_objectThread, &QObject::deleteLater);
    connect(m_objectThread, &QThread::finished, m_object, &QObject::deleteLater);
    if (m_isCreateNet && !isWlan) {
        connect(createNetPage, &CreatNetPage::ipv4EditFinished, this, [=](){
            createNetPage->startLoading();
        });
        connect(createNetPage, SIGNAL(ipv4EditFinished(const QString &)), m_object, SLOT(checkIpv4ConflictThread(const QString &)));
        connect(m_object, &ThreadObject::ipv4IsConflict, this, [=](bool ipv4IsConf) {
            createNetPage->stopLoading();
            createNetPage->showIpv4AddressConflict(ipv4IsConf);
        });
    } else {
        connect(ipv4Page, &Ipv4Page::ipv4EditFinished, this, [=](){
            ipv4Page->startLoading();
        });
        connect(ipv6Page, &Ipv6Page::ipv6EditFinished, this, [=](){
            ipv6Page->startLoading();
        });

        connect(ipv4Page, SIGNAL(ipv4EditFinished(const QString &)), m_object, SLOT(checkIpv4ConflictThread(const QString &)));
        connect(ipv6Page, SIGNAL(ipv6EditFinished(const QString &)), m_object, SLOT(checkIpv6ConflictThread(const QString &)));
        connect(this, SIGNAL(checkCurrentIpv4Conflict(const QString &)), m_object, SLOT(checkIpv4ConflictThread(const QString &)));
        connect(this, SIGNAL(checkCurrentIpv6Conflict(const QString &)), m_object, SLOT(checkIpv6ConflictThread(const QString &)));

        connect(m_object, &ThreadObject::ipv4IsConflict, this, [=](bool ipv4IsConf) {
            ipv4Page->stopLoading();
            ipv4Page->showIpv4AddressConflict(ipv4IsConf);
        });
        connect(m_object, &ThreadObject::ipv6IsConflict, this, [=](bool ipv6IsConf) {
            ipv6Page->stopLoading();
            ipv6Page->showIpv6AddressConflict(ipv6IsConf);
        });
    }
    m_objectThread->start();
}

NetDetail::NetDetail(QString interface, QString name, QString uuid, bool isActive, bool isWlan, bool isCreateNet, QWidget *parent)
    :m_deviceName(interface),
     m_name(name),
     m_uuid(uuid),
     isActive(isActive),
     isWlan(isWlan),
     m_isCreateNet(isCreateNet),
     QWidget(parent)
{
    //设置窗口无边框，阴影
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
//    MotifWmHints window_hints;
//    window_hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
//    window_hints.functions = MWM_FUNC_ALL;
//    window_hints.decorations = MWM_DECOR_BORDER;
//    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), window_hints);
//#else
//    this->setWindowFlags(Qt::Dialog /*| Qt::FramelessWindowHint*/);
//    this->setWindowFlag(Qt::Window);
    KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
//#endif
//    this->setProperty("useStyleWindowManager", false); //禁用拖动
//    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint );
//    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Dialog);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(WINDOW_WIDTH,WINDOW_HEIGHT);
    centerToScreen();

    if (!m_isCreateNet && name.isEmpty()) {
        m_isCreateNet = true;
    }
    m_netDeviceResource = new KyNetworkDeviceResourse(this);
    m_wirelessConnOpration = new KyWirelessConnectOperation(this);
    m_resource = new KyWirelessNetResource(this);
    m_connectOperation = new KyConnectOperation(this);
    m_wiredConnOperation = new KyWiredConnectOperation(this);
    initUI();
    loadPage();
    initComponent();
    getConInfo(m_info);
    startObjectThread();
    pagePadding(name,isWlan);

    connect(qApp, &QApplication::paletteChanged, this, &NetDetail::onPaletteChanged);

    isCreateOk = !(m_isCreateNet && !isWlan);
    isDetailOk = !(m_name.isEmpty());
    isIpv4Ok = true;
    isIpv6Ok = true;
    isSecuOk = false;
    if (!m_uuid.isEmpty() || (m_uuid.isEmpty() && m_info.secType == NONE)) {
        isSecuOk = true;
    }

    qDebug() << interface << name << uuid <<  "isWlan" << isWlan << "isCreateNet" <<m_isCreateNet;

    setConfirmEnable();
}

NetDetail::~NetDetail()
{
    if (m_objectThread->isRunning()) {
        m_objectThread->quit();
        m_objectThread->wait();
    }
}

void NetDetail::onPaletteChanged()
{
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Background, pal.base().color());
    this->setPalette(pal);

#if 0
    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = lightPalette(this);
       }
    }
    pal.setColor(QPalette::Background, pal.base().color());
    this->setPalette(pal);

    setFramePalette(detailPage, pal);
    setFramePalette(ipv4Page, pal);
    setFramePalette(ipv6Page, pal);
    setFramePalette(securityPage, pal);
    setFramePalette(createNetPage, pal);
    QToolTip::setPalette(pal);
#endif

    QPalette listwidget_pal(detailPage->m_listWidget->palette());
    listwidget_pal.setColor(QPalette::Base, pal.base().color());
    listwidget_pal.setColor(QPalette::AlternateBase, pal.alternateBase().color());
    detailPage->m_listWidget->setAlternatingRowColors(true);
    detailPage->m_listWidget->setPalette(listwidget_pal);
#if 0
    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }

#endif
}

void NetDetail::currentRowChangeSlot(int row)
{
    if (isActive) {
        if (row < 3) {
            stackWidget->setCurrentIndex(row);
        } else {
            if(isWlan) {
                stackWidget->setCurrentIndex(row);
            } else {
                stackWidget->setCurrentIndex(CONFIG_PAGE_NUM);
            }
        }
    } else {
        stackWidget->setCurrentIndex(row);
    }
}

void NetDetail::paintEvent(QPaintEvent *event)
{
//    QPalette pal = qApp->palette();
//    QPainter painter(this);
//    painter.setBrush(pal.color(QPalette::Base));
//    painter.drawRect(this->rect());
//    painter.fillRect(rect(), QBrush(pal.color(QPalette::Base)));
    return QWidget::paintEvent(event);
}

void NetDetail::closeEvent(QCloseEvent *event)
{
    Q_EMIT this->detailPageClose(m_deviceName, m_name, m_uuid);
    Q_EMIT this->createPageClose(m_deviceName);
    return QWidget::closeEvent(event);
}

void NetDetail::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    kdk::WindowManager::setGeometry(this->windowHandle(), QRect(desk_x / 2 - x / 2 + desk_rect.left(),
                                                                desk_y / 2 - y / 2 + desk_rect.top(),
                                                                this->width(),
                                                                this->height()));
}

void NetDetail::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,9,0,24);
    mainLayout->setSpacing(0);

    this->installEventFilter(this);
    pageFrame = new QFrame(this);
    centerWidget = new QWidget(this);
    bottomWidget = new QWidget(this);

    detailPage = new DetailPage(isWlan, m_name.isEmpty(), this);
    ipv4Page = new Ipv4Page(this);
    ipv6Page = new Ipv6Page(this);
    securityPage = new SecurityPage(this);
    createNetPage = new CreatNetPage(this);

    configPage = new ConfigPage(this);

    detailPage->setFixedWidth(PAGE_WIDTH);
    ipv4Page->setFixedWidth(PAGE_WIDTH);
    ipv6Page->setFixedWidth(PAGE_WIDTH);
    securityPage->setFixedWidth(PAGE_WIDTH);
    createNetPage->setFixedWidth(PAGE_WIDTH);
    configPage->setFixedWidth(PAGE_WIDTH);

    // 滚动区域
    m_secuPageScrollArea = new QScrollArea(centerWidget);
    m_secuPageScrollArea->setFixedWidth(SCRO_WIDTH);
    m_secuPageScrollArea->setFrameShape(QFrame::NoFrame);
    m_secuPageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_secuPageScrollArea->setWidget(securityPage);

    m_secuPageScrollArea->setWidgetResizable(true);

    m_ipv4ScrollArea = new QScrollArea(centerWidget);
    m_ipv4ScrollArea->setFixedWidth(SCRO_WIDTH);
    m_ipv4ScrollArea->setFrameShape(QFrame::NoFrame);
    m_ipv4ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ipv4ScrollArea->setWidget(ipv4Page);
    m_ipv4ScrollArea->setWidgetResizable(true);

    connect(ipv4Page, &Ipv4Page::scrollToBottom, this, [&](){
        QTimer::singleShot(50,this,[=]() {
            m_ipv4ScrollArea->verticalScrollBar()->setValue(m_ipv4ScrollArea->verticalScrollBar()->maximum());
        });
    });

    m_ipv6ScrollArea = new QScrollArea(centerWidget);
    m_ipv6ScrollArea->setFixedWidth(SCRO_WIDTH);
    m_ipv6ScrollArea->setFrameShape(QFrame::NoFrame);
    m_ipv6ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ipv6ScrollArea->setWidget(ipv6Page);
    m_ipv6ScrollArea->setWidgetResizable(true);

    connect(ipv6Page, &Ipv6Page::scrollToBottom, this, [&](){
        QTimer::singleShot(50,this,[=]() {
            m_ipv6ScrollArea->verticalScrollBar()->setValue(m_ipv6ScrollArea->verticalScrollBar()->maximum());
        });
    });

    m_createNetPageScrollArea = new QScrollArea(centerWidget);
    m_createNetPageScrollArea->setFixedWidth(SCRO_WIDTH);
    m_createNetPageScrollArea->setFrameShape(QFrame::NoFrame);
    m_createNetPageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_createNetPageScrollArea->setWidget(createNetPage);
    m_createNetPageScrollArea->setWidgetResizable(true);

    stackWidget = new QStackedWidget(centerWidget);
    stackWidget->addWidget(detailPage);
    stackWidget->addWidget(m_ipv4ScrollArea);
    stackWidget->addWidget(m_ipv6ScrollArea);
    stackWidget->addWidget(m_secuPageScrollArea);
    stackWidget->addWidget(configPage);
    stackWidget->addWidget(m_createNetPageScrollArea);

    // TabBar
    onPaletteChanged();
    m_networkMode = NetworkModeType(NetworkModeConfig::getInstance()->getNetworkModeConfig(m_uuid));
    m_netTabBar = new NetTabBar(this);
    m_netTabBar->addTab(tr("Detail")); //详情
    m_netTabBar->addTab(tr("IPv4"));//Ipv4
    m_netTabBar->addTab(tr("IPv6"));//Ipv6
    if (isWlan) {
        m_netTabBar->addTab(tr("Security"));//安全
        if (isActive && m_networkMode != DBUS_INVAILD && m_networkMode != NO_CONFIG) {
            m_netTabBar->addTab(tr("Config")); //配置
            m_netTabBar->setFixedWidth(WLAN_TAB_WIDTH + TAB_WIDTH);
        } else {
            m_netTabBar->setFixedWidth(WLAN_TAB_WIDTH);
        }
    } else {
        if (isActive && m_networkMode != DBUS_INVAILD && m_networkMode != NO_CONFIG) {
            m_netTabBar->addTab(tr("Config")); //配置
            m_netTabBar->setFixedWidth(LAN_TAB_WIDTH + TAB_WIDTH);
        } else {
            m_netTabBar->setFixedWidth(LAN_TAB_WIDTH);
        }
    }

    // TabBar关联选项卡页面
    connect(m_netTabBar, SIGNAL(currentChanged(int)), this, SLOT(currentRowChangeSlot(int)));
    setNetTabToolTip();

    confimBtn = new QPushButton(this);
    confimBtn->setText(tr("Confirm"));

    cancelBtn = new QPushButton(this);
    cancelBtn->setText(tr("Cancel"));
    forgetBtn = new QPushButton(this);

    QHBoxLayout *pageLayout = new QHBoxLayout(pageFrame);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->addWidget(m_netTabBar, Qt::AlignCenter);

    QVBoxLayout *centerlayout = new QVBoxLayout(centerWidget);
    centerlayout->setContentsMargins(CENTER_LAYOUT_MARGINS); // 右边距为0，为安全页滚动区域留出空间
    centerlayout->addWidget(stackWidget);

    Divider *divider = new Divider(false, this);

    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(BOTTOM_LAYOUT_MARGINS);
    bottomLayout->setSpacing(BOTTOM_LAYOUT_SPACING);
    bottomLayout->addWidget(forgetBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(cancelBtn);
    bottomLayout->addWidget(confimBtn);
    bottomWidget->setMinimumHeight(PAGE_MIN_HEIGHT);

    mainLayout->addWidget(pageFrame);
    mainLayout->addSpacing(24);
    mainLayout->addWidget(centerWidget);
    mainLayout->addWidget(divider);
    mainLayout->addSpacing(16);
    mainLayout->addWidget(bottomWidget);

    this->setAutoFillBackground(true);
//    this->setPalette(pal);
}

void NetDetail::loadPage()
{
    //判断是否创建网络页面
    if (m_isCreateNet && !isWlan) {
        pageFrame->hide();
        stackWidget->setCurrentIndex(CREATE_NET_PAGE_NUM);
        this->setWindowTitle(tr("Add LAN Connect"));
    } else {
        stackWidget->setCurrentIndex(DETAIL_PAGE_NUM);
        this->setWindowTitle(m_name);
        if (isWlan && m_name.isEmpty()) {
            this->setWindowTitle(tr("Connect Hidden WLAN"));
        }
    }
}

void NetDetail::initComponent()
{
    connect(cancelBtn, &QPushButton::clicked, this, [=] {
        close();
    });

    connect(confimBtn, SIGNAL(clicked()), this, SLOT(on_btnConfirm_clicked()));
    if (!m_uuid.isEmpty()) {
        if (isWlan) {
            forgetBtn->setText(tr("Forget this network"));
        } else {
            forgetBtn->setText(tr("Delete this network"));
        }
        forgetBtn->show();
        connect(forgetBtn, SIGNAL(clicked()), this, SLOT(on_btnForget_clicked()));
    } else {
        forgetBtn->hide();
    }

    connect(createNetPage, &CreatNetPage::setCreatePageState, this, [=](bool status) {
       isCreateOk = status;
       setConfirmEnable();
    });

    connect(detailPage, &DetailPage::setDetailPageState, this, [=](bool status) {
       isDetailOk = status;
       setConfirmEnable();
    });

    connect(ipv4Page, &Ipv4Page::setIpv4PageState, this, [=](bool status) {
       isIpv4Ok = status;
       setConfirmEnable();
    });

    connect(ipv6Page, &Ipv6Page::setIpv6PageState, this, [=](bool status) {
       isIpv6Ok = status;
       setConfirmEnable();
    });

    connect(securityPage, &SecurityPage::setSecuPageState, this, [=](bool status) {
       isSecuOk = status;
       setConfirmEnable();
    });

    const QByteArray id(THEME_SCHAME);
    if(QGSettings::isSchemaInstalled(id)){
        QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("systemFont" == key || "systemFontSize" ==key) {
                setNetTabToolTip();
            } else if ("themeColor" == key) {
                onPaletteChanged();
            }
        });
    }
}

void NetDetail::pagePadding(QString netName, bool isWlan)
{
    //网络详情页填充
    if(m_isCreateNet && !isWlan) {
        return;
    }

    detailPage->setSSID(netName);
    detailPage->setProtocol(m_info.strConType);
    detailPage->setSecType(m_info.strSecType);
    detailPage->setHz(m_info.strHz);
    detailPage->setChan(m_info.strChan);
    detailPage->setIpv4(m_info.strDynamicIpv4);
    detailPage->setIpv4Dns(m_info.strDynamicIpv4Dns);
    detailPage->setIpv6(m_info.strDynamicIpv6);
    detailPage->setMac(m_info.strMac);
    detailPage->setBandWidth(m_info.strBandWidth);
    detailPage->setAutoConnect(m_info.isAutoConnect);

    //ipv4页面填充
    ipv4Page->setUuid(m_uuid);
    ipv4Page->setMulDns(m_info.ipv4DnsList);
    if (m_info.ipv4ConfigType == CONFIG_IP_MANUAL) {
        Q_EMIT checkCurrentIpv4Conflict(m_info.strIPV4Address);
        ipv4Page->setIpv4Config(m_info.ipv4ConfigType);
        ipv4Page->setIpv4(m_info.strIPV4Address);
        ipv4Page->setNetMask(m_info.strIPV4NetMask);
//        ipv4Page->setIpv4FirDns(m_info.strIPV4FirDns);
//        ipv4Page->setIpv4SecDns(m_info.strIPV4SecDns);
        ipv4Page->setGateWay(m_info.strIPV4GateWay);
    } else {
        ipv4Page->setIpv4Config(m_info.ipv4ConfigType);
    }
    //ipv6页面填充
    ipv6Page->setMulDns(m_info.ipv6DnsList);
    if (m_info.ipv6ConfigType == CONFIG_IP_MANUAL) {
        Q_EMIT checkCurrentIpv6Conflict(m_info.strIPV6Address);
        ipv6Page->setIpv6Config(m_info.ipv6ConfigType);
        ipv6Page->setIpv6(m_info.strIPV6Address);
        ipv6Page->setIpv6Perfix(m_info.iIPV6Prefix);
//        ipv6Page->setIpv6FirDns(m_info.strIPV6FirDns);
//        ipv6Page->setIpv6SecDns(m_info.strIPV6SecDns);
        ipv6Page->setGateWay(m_info.strIPV6GateWay);
    } else {
        ipv6Page->setIpv6Config(m_info.ipv6ConfigType);
    }

    //安全页面
    if (isWlan) {
        securityPage->setSecurity(m_info.secType);
        qDebug() << "setSecurity" << m_info.secType;
        if (m_info.secType == WPA_AND_WPA2_ENTERPRISE) {
            if (m_info.enterpriseType == TLS) {
                securityPage->setTlsInfo(m_info.tlsInfo);
            } else if (m_info.enterpriseType == PEAP) {
                securityPage->setPeapInfo(m_info.peapInfo);
            } else if (m_info.enterpriseType == TTLS) {
                securityPage->setTtlsInfo(m_info.ttlsInfo);
            } else if (m_info.enterpriseType == LEAP) {
                securityPage->setLeapInfo(m_info.leapInfo);
            } else if (m_info.enterpriseType == PWD) {
                securityPage->setPwdInfo(m_info.pwdInfo);
            } else if (m_info.enterpriseType == FAST) {
                securityPage->setFastInfo(m_info.fastInfo);
            } else {
                qWarning() << "[NetDetail] unknown enterprise connection type" << m_info.enterpriseType;
            }
        }
    }

    //配置页面
    if (isActive  && m_networkMode != DBUS_INVAILD && m_networkMode != NO_CONFIG) {
        configPage->setConfigState(m_networkMode);
    }
}

//获取网路详情信息
void NetDetail::getConInfo(ConInfo &conInfo)
{
    if (m_isCreateNet && !isWlan) {
        return;
    }
    getBaseInfo(conInfo);
    getDynamicIpInfo(conInfo, isActive);
    getStaticIpInfo(conInfo,isActive);
}


//详情ssid 带宽 物理地址 无线额外(安全性 频带 通道)
void NetDetail::getBaseInfo(ConInfo &conInfo)
{
    //有线无线公有
    conInfo.strConName = m_name;

    QString hardAddress;
    int bandWith;
    m_netDeviceResource->getHardwareInfo(m_deviceName, hardAddress, bandWith);

    if (!hardAddress.isEmpty()) {
        conInfo.strBandWidth = QString("%1").arg(bandWith/1000) + "Mbps";
        conInfo.strMac = hardAddress;
    }

    if (!isWlan) {
        conInfo.strConType = "802-3-ethernet";
    } else {
        conInfo.strConType = "802-11-wireless";
        if (!isActive) {
            KyWirelessNetItem item;
            if (!m_resource->getWifiNetwork(m_deviceName, m_name, item)) {
                qDebug() << "getWifiNetWork failed device:" << m_deviceName << " name:" << m_name;
                return;
            } else {
                    conInfo.strHz = QString::number(item.m_frequency);
                    conInfo.strChan = QString::number(item.m_channel);
                    //无线特有
                    conInfo.strSecType = item.m_secuType;
                    if (conInfo.strSecType.isEmpty()) {
                        conInfo.strSecType = tr("None");
                    }

                    if (!item.m_isConfigured) {
                        conInfo.secType = item.m_kySecuType;
                    }
            }
        } else {
            uint iHz,iChan;
            QString strMac;
            m_netDeviceResource->getDeviceActiveAPInfo(m_deviceName, strMac, iHz, iChan, conInfo.strSecType);
            if (conInfo.strSecType.isEmpty()) {
                conInfo.strSecType = tr("None");
            }
            conInfo.strHz = QString::number(iHz);
            conInfo.strChan = QString::number(iChan);

        }



        KyKeyMgmt type = m_wirelessConnOpration->getConnectKeyMgmt(m_uuid);
        if (!m_uuid.isEmpty()) {
            KyKeyMgmt type = m_wirelessConnOpration->getConnectKeyMgmt(m_uuid);
            if (type == WpaNone || type == Unknown) {
                conInfo.secType = NONE;
            } else if (type == WpaPsk) {
                conInfo.secType = WPA_AND_WPA2_PERSONAL;
            } else if (type == SAE) {
                conInfo.secType = WPA3_PERSONAL;
            } else if (type == WpaEap) {
                conInfo.secType = WPA_AND_WPA2_ENTERPRISE;
            } else {
                qDebug() << "KeyMgmt not support now " << type;
            }
        }

        initSecuData();
    }
}

//详情ipv4 ipv6 ipv4Dns
void NetDetail::getDynamicIpInfo(ConInfo &conInfo, bool bActived)
{
    if (!bActived) {
        return;
    }
    //已激活的网络 详情页显示动态ipv4 ipv6 dns
    QString ipv4,ipv6;
    QList<QHostAddress> ipv4Dns,ipv6Dns;
    KyActiveConnectResourse *activeResourse = new KyActiveConnectResourse(this);
    activeResourse->getActiveConnectIpInfo(m_uuid,ipv4,ipv6);
    activeResourse->getActiveConnectDnsInfo(m_uuid,ipv4Dns,ipv6Dns);

    //Ipv6
    if (!ipv6.isEmpty()) {
        conInfo.strDynamicIpv6 = ipv6;
    }

    //IPv4
    if (!ipv4.isEmpty()) {
        conInfo.strDynamicIpv4 = ipv4;
    }

    if (!ipv4Dns.isEmpty()) {
        //conInfo.strDynamicIpv4Dns = ipv4Dns.at(0).toString();
        QString dnsList;
        dnsList.clear();
        for (QHostAddress str: ipv4Dns) {
            dnsList.append(str.toString());
            dnsList.append("; ");
        }
        dnsList.chop(2);
        conInfo.strDynamicIpv4Dns = dnsList;
    }
}

//ipv4+ipv6页面
void NetDetail::getStaticIpInfo(ConInfo &conInfo, bool bActived)
{
    KyConnectResourse *kyConnectResourse = new KyConnectResourse(this);
    KyConnectSetting  connetSetting;
    kyConnectResourse->getConnectionSetting(m_uuid,connetSetting);
    connetSetting.dumpInfo();

    //conInfo.ipv4ConfigType = connetSetting.m_ipv4ConfigIpType;
    //conInfo.ipv6ConfigType = connetSetting.m_ipv6ConfigIpType;
    conInfo.isAutoConnect  = connetSetting.m_isAutoConnect;
    conInfo.ipv4DnsList = connetSetting.m_ipv4Dns;
    conInfo.ipv6DnsList = connetSetting.m_ipv6Dns;

    //lingmo从第三方库读取有问题 改为ipv4/ipv6信息直接通过dbus获取
    KyConnectItem* item = kyConnectResourse->getConnectionItemByUuidWithoutActivateChecking(m_uuid);
    if (item == nullptr) {
        conInfo.ipv4ConfigType = CONFIG_IP_DHCP;
        conInfo.ipv6ConfigType = CONFIG_IP_DHCP;
    } else {
        getIpv4Ipv6Info(item->m_connectPath, conInfo);
    }
#if 0
    if (connetSetting.m_ipv4ConfigIpType == CONFIG_IP_MANUAL) {
        if (connetSetting.m_ipv4Address.size() > 0) {
            conInfo.strIPV4Address = connetSetting.m_ipv4Address.at(0).ip().toString();
            conInfo.strIPV4NetMask = connetSetting.m_ipv4Address.at(0).netmask().toString();
            conInfo.strIPV4GateWay = connetSetting.m_ipv4Address.at(0).gateway().toString();
        }
        if (connetSetting.m_ipv4Dns.size() == 1) {
            conInfo.strIPV4FirDns = connetSetting.m_ipv4Dns.at(0).toString();
        } else if (connetSetting.m_ipv4Dns.size() > 1) {
            conInfo.strIPV4FirDns = connetSetting.m_ipv4Dns.at(0).toString();
            conInfo.strIPV4SecDns = connetSetting.m_ipv4Dns.at(1).toString();
        }
    }

    if (connetSetting.m_ipv6ConfigIpType == CONFIG_IP_MANUAL) {
        if (connetSetting.m_ipv6Address.size() > 0) {
            conInfo.strIPV6Address = connetSetting.m_ipv6Address.at(0).ip().toString();
            conInfo.iIPV6Prefix = ipv6Page->getPerfixLength(connetSetting.m_ipv6Address.at(0).netmask().toString());
            conInfo.strIPV6GateWay = connetSetting.m_ipv6Address.at(0).gateway().toString();
        }
        if (connetSetting.m_ipv6Dns.size() == 1) {
            conInfo.strIPV6FirDns = connetSetting.m_ipv6Dns.at(0).toString();
        } else if (connetSetting.m_ipv6Dns.size() > 1) {
            conInfo.strIPV6FirDns = connetSetting.m_ipv6Dns.at(0).toString();
            conInfo.strIPV6SecDns = connetSetting.m_ipv6Dns.at(1).toString();
        }
    }
#endif
    if (!bActived) {
        conInfo.strDynamicIpv4 = conInfo.strIPV4Address.isEmpty() ? tr("Auto") : conInfo.strIPV4Address;
        conInfo.strDynamicIpv6 = conInfo.strIPV6Address.isEmpty() ? tr("Auto") : conInfo.strIPV6Address;
        conInfo.strDynamicIpv4Dns = conInfo.strIPV4FirDns.isEmpty() ? tr("Auto") : conInfo.strIPV4FirDns;
    }
}

void NetDetail::initSecuData()
{
    QString password("");
    int type = m_info.secType;
    switch (type) {
    case NONE:
        break;
    case WPA_AND_WPA2_PERSONAL:
    case WPA3_PERSONAL:
    case WPA_AND_WPA3:
        if (!m_uuid.isEmpty()) {
            NetworkManager::Setting::SecretFlags flag;
            if (m_wirelessConnOpration->getConnSecretFlags(m_uuid, flag)) {
                if (!flag) {
                    password = m_wirelessConnOpration->getPsk(m_uuid);
                }
            }
        }
        m_info.strPassword = password;
        securityPage->setPsk(password);
        break;
    case WPA_AND_WPA2_ENTERPRISE:
        if (!m_wirelessConnOpration->getEnterpiseEapMethod(m_uuid, m_info.enterpriseType)) {
            qDebug() << m_name << "not enterprise wifi";
        } else if (m_info.enterpriseType == TLS) {
            initTlsInfo(m_info);
        } else if (m_info.enterpriseType == PEAP) {
            initPeapInfo(m_info);
        } else if (m_info.enterpriseType == TTLS) {
            initTtlsInfo(m_info);
        } else if (m_info.enterpriseType == LEAP) {
            initLeapInfo(m_info);
        } else if (m_info.enterpriseType == PWD) {
            initPwdInfo(m_info);
        } else if (m_info.enterpriseType == FAST) {
            initFastInfo(m_info);
        } else {
            qWarning() << "[NetDetail] unknown enterprise connection type" << m_info.enterpriseType;
        }
        break;
    default:
        break;
    }
}

void NetDetail::initTlsInfo(ConInfo &conInfo)
{
    m_resource->getEnterPriseInfoTls(m_uuid, conInfo.tlsInfo);
}

void NetDetail::initPeapInfo(ConInfo &conInfo)
{
    m_resource->getEnterPriseInfoPeap(m_uuid, conInfo.peapInfo);
}

void NetDetail::initTtlsInfo(ConInfo &conInfo)
{
    m_resource->getEnterPriseInfoTtls(m_uuid, conInfo.ttlsInfo);
}

void NetDetail::initLeapInfo(ConInfo &conInfo)
{
    m_resource->getEnterPriseInfoLeap(m_uuid, conInfo.leapInfo);
}

void NetDetail::initPwdInfo(ConInfo &conInfo)
{
    m_resource->getEnterPriseInfoPwd(m_uuid, conInfo.pwdInfo);
}

void NetDetail::initFastInfo(ConInfo &conInfo)
{
    m_resource->getEnterPriseInfoFast(m_uuid, conInfo.fastInfo);
}

//点击了保存更改网络设置的按钮
void NetDetail::on_btnConfirm_clicked()
{
    qDebug() << "on_btnConfirm_clicked";
    setNetdetailSomeEnable(false);
    if (m_isCreateNet) {
        if (!isWlan) {
            //新建有线连接
            qDebug() << "Confirm create wired connect";
            if (!createWiredConnect()) {
                setNetdetailSomeEnable(true);
                return;
            }
        } else {
            //新建无线连接
            qDebug() << "Confirm create wireless connect";
            if (!createWirelessConnect()) {
                setNetdetailSomeEnable(true);
                return;
            }
        }
    } else {
        //更新连接
        qDebug() << "Confirm update connect";
        if (!updateConnect()) {
            setNetdetailSomeEnable(true);
            return;
        }
    }
    close();
}

//点击忘记网络
void NetDetail::on_btnForget_clicked()
{
    qDebug() << "user choose forget connection uuid = " << m_uuid;
    m_connectOperation->deleteConnect(m_uuid);
    close();
}

void NetDetail::setConfirmEnable()
{
    if (m_isCreateNet && !isWlan) {
            isConfirmBtnEnable = isCreateOk;
    } else {
        if (isDetailOk && isIpv4Ok && isIpv6Ok) {
            if (isWlan && !isSecuOk) {
                isConfirmBtnEnable = false;
            } else {
                isConfirmBtnEnable = true;
            }
        } else {
            isConfirmBtnEnable = false;
        }
    }
    qDebug() << "setConfirmEnable "<< isConfirmBtnEnable;
    confimBtn->setEnabled(isConfirmBtnEnable);
}

#if 0
bool NetDetail::checkIpv4Conflict(QString ipv4Address)
{
    showDesktopNotify(tr("start check ipv4 address conflict"), "networkwrong");
    bool isConflict = false;
    KyIpv4Arping* ipv4Arping = new KyIpv4Arping(m_deviceName, ipv4Address);

    if (ipv4Arping->ipv4ConflictCheck() >= 0) {
        isConflict =  ipv4Arping->ipv4IsConflict();
    } else {
        qWarning() << "checkIpv4Conflict internal error";
    }

    delete ipv4Arping;
    ipv4Arping = nullptr;
    return isConflict;
}

bool NetDetail::checkIpv6Conflict(QString ipv6address)
{
    showDesktopNotify(tr("start check ipv6 address conflict"), "networkwrong");
    bool isConflict = false;
    KyIpv6Arping* ipv46rping = new KyIpv6Arping(m_deviceName, ipv6address);

    if (ipv46rping->ipv6ConflictCheck() >= 0) {
        isConflict =  ipv46rping->ipv6IsConflict();
    } else {
        qWarning() << "checkIpv6Conflict internal error";
    }

    delete ipv46rping;
    ipv46rping = nullptr;
    return isConflict;
}
#endif

void NetDetail::updateWirelessPersonalConnect()
{
    KyWirelessConnectSetting setting;
    securityPage->updateSecurityChange(setting);
    bool isPwdChanged = !(m_info.strPassword == setting.m_psk);
    m_wirelessConnOpration->updateWirelessPersonalConnect(m_uuid, setting, isPwdChanged);
}

void NetDetail::updateWirelessEnterPriseConnect(KyEapMethodType enterpriseType)
{
    if (enterpriseType == TLS) {
        m_info.tlsInfo.devIfaceName = m_deviceName;
        securityPage->updateTlsChange(m_info.tlsInfo);
        m_wirelessConnOpration->updateWirelessEnterPriseTlsConnect(m_uuid, m_info.tlsInfo);
    } else if (enterpriseType == PEAP) {
        securityPage->updatePeapChange(m_info.peapInfo);
        m_wirelessConnOpration->updateWirelessEnterPrisePeapConnect(m_uuid, m_info.peapInfo);
    } else if (enterpriseType == TTLS) {
        securityPage->updateTtlsChange(m_info.ttlsInfo);
        m_wirelessConnOpration->updateWirelessEnterPriseTtlsConnect(m_uuid, m_info.ttlsInfo);
    } else if (enterpriseType == LEAP) {
        securityPage->updateLeapChange(m_info.leapInfo);
        m_wirelessConnOpration->updateWirelessEnterPriseLeapConnect(m_uuid, m_info.leapInfo);
    } else if (enterpriseType == PWD) {
        securityPage->updatePwdChange(m_info.pwdInfo);
        m_wirelessConnOpration->updateWirelessEnterPrisePwdConnect(m_uuid, m_info.pwdInfo);
    } else if (enterpriseType == FAST) {
        securityPage->updateFastChange(m_info.fastInfo);
        m_wirelessConnOpration->updateWirelessEnterPriseFastConnect(m_uuid, m_info.fastInfo);
    } else {
        qWarning() << "[NetDetail] unknown enterprise connection type" << enterpriseType;
    }
}

bool NetDetail::createWiredConnect()
{
    KyWirelessConnectSetting connetSetting;
    connetSetting.setIfaceName(m_deviceName);
    createNetPage->constructIpv4Info(connetSetting);
//    if (connetSetting.m_ipv4ConfigIpType != CONFIG_IP_DHCP) {
//        if (checkIpv4Conflict(connetSetting.m_ipv4Address.at(0).ip().toString())) {
//            qDebug() << "ipv4 conflict";
//            showDesktopNotify(tr("ipv4 address conflict!"), "networkwrong");
//            return false;
//        }
//    }
    m_wiredConnOperation->createWiredConnect(connetSetting);
    return true;
}

bool NetDetail::createWirelessConnect()
{
    KyWirelessConnectSetting connetSetting;
    KySecuType secuType;
    KyEapMethodType enterpriseType;
    securityPage->getSecuType(secuType, enterpriseType);
    //类型判断
    if (!m_name.isEmpty()) {
        if (!checkWirelessSecurity(secuType)) {
            return false;
        }
    }

    //基本信息
    QString ssid;
    if (m_name.isEmpty()) {
        detailPage->getSsid(ssid);
    } else {
        ssid = m_name;
    }
    connetSetting.setConnectName(ssid);
    connetSetting.setIfaceName(m_deviceName);
    if (detailPage->checkIsChanged(m_info)) {
        connetSetting.isAutoConnect = !m_info.isAutoConnect;
    } else {
        connetSetting.isAutoConnect = m_info.isAutoConnect;
    }
    qDebug() << "isAutoConnect" << connetSetting.isAutoConnect;
    connetSetting.m_ssid = ssid;
//    connetSetting.m_secretFlag = NetworkManager::Setting::None;
    //由于X.h的None与此处的None有歧义，此处直接使用值
    connetSetting.m_secretFlag = 0;

    //ipv4 & ipv6
    bool ipv4Change = ipv4Page->checkIsChanged(m_info, connetSetting);
    bool ipv6Change = ipv6Page->checkIsChanged(m_info, connetSetting);

    connetSetting.dumpInfo();

    qDebug() << "ipv4Changed" << ipv4Change << "ipv6Change" << ipv6Change;
//    if (ipv4Change && connetSetting.m_ipv4ConfigIpType == CONFIG_IP_MANUAL) {
//        if (checkIpv4Conflict(connetSetting.m_ipv4Address.at(0).ip().toString())) {
//            qDebug() << "ipv4 conflict";
//            showDesktopNotify(tr("ipv4 address conflict!"), "networkwrong");
//            return false;
//        }
//    }

//    if (ipv6Change && connetSetting.m_ipv6ConfigIpType == CONFIG_IP_MANUAL) {
//        if (checkIpv6Conflict(connetSetting.m_ipv6Address.at(0).ip().toString())) {
//            qDebug() << "ipv6 conflict";
//            showDesktopNotify(tr("ipv6 address conflict!"), "networkwrong");
//            return false;
//        }
//    }
    //wifi安全性
    if (secuType == WPA_AND_WPA2_ENTERPRISE) {
        connetSetting.m_type = WpaEap;
        if (enterpriseType == TLS) {
            m_info.tlsInfo.devIfaceName = m_deviceName;
            securityPage->updateTlsChange(m_info.tlsInfo);
            if (!m_name.isEmpty()) {
                qDebug() << "add new TLS connect";
                m_wirelessConnOpration->addTlsConnect(connetSetting, m_info.tlsInfo);
            } else {
                qDebug() << "addAndConnect TLS connect";
                m_wirelessConnOpration->addAndActiveWirelessEnterPriseTlsConnect(m_info.tlsInfo, connetSetting, m_deviceName, true);
            }
        } else if (enterpriseType == PEAP) {
            securityPage->updatePeapChange(m_info.peapInfo);
            if (!m_name.isEmpty()) {
                qDebug() << "add new PEAP connect";
                m_wirelessConnOpration->addPeapConnect(connetSetting, m_info.peapInfo);
            } else {
                qDebug() << "addAndConnect PEAP connect";
                m_wirelessConnOpration->addAndActiveWirelessEnterPrisePeapConnect(m_info.peapInfo, connetSetting, m_deviceName, true);
            }
        } else if (enterpriseType == TTLS) {
            securityPage->updateTtlsChange(m_info.ttlsInfo);
            if (!m_name.isEmpty()) {
                qDebug() << "add new TTLS connect";
                m_wirelessConnOpration->addTtlsConnect(connetSetting, m_info.ttlsInfo);
            } else {
                qDebug() << "addAndConnect TTLS connect";
                m_wirelessConnOpration->addAndActiveWirelessEnterPriseTtlsConnect(m_info.ttlsInfo, connetSetting, m_deviceName, true);
            }
        } else if (enterpriseType == LEAP) {
            securityPage->updateLeapChange(m_info.leapInfo);
            if (!m_name.isEmpty()) {
                qDebug() << "add new LEAP connect";
                m_wirelessConnOpration->addLeapConnect(connetSetting, m_info.leapInfo);
            } else {
                qDebug() << "addAndConnect LEAP connect";
                m_wirelessConnOpration->addAndActiveWirelessEnterPriseLeapConnect(m_info.leapInfo, connetSetting, m_deviceName, true);
            }
        } else if (enterpriseType == PWD) {
            securityPage->updatePwdChange(m_info.pwdInfo);
            if (!m_name.isEmpty()) {
                qDebug() << "add new PWD connect";
                m_wirelessConnOpration->addPwdConnect(connetSetting, m_info.pwdInfo);
            } else {
                qDebug() << "addAndConnect PWD connect";
                m_wirelessConnOpration->addAndActiveWirelessEnterPrisePwdConnect(m_info.pwdInfo, connetSetting, m_deviceName, true);
            }
        } else if (enterpriseType == FAST) {
            securityPage->updateFastChange(m_info.fastInfo);
            if (!m_name.isEmpty()) {
                qDebug() << "add new FAST connect";
                m_wirelessConnOpration->addFastConnect(connetSetting, m_info.fastInfo);
            } else {
                qDebug() << "addAndConnect FAST connect";
                m_wirelessConnOpration->addAndActiveWirelessEnterPriseFastConnect(m_info.fastInfo, connetSetting, m_deviceName, true);
            }
        } else {
            qWarning() << "[NetDetail] unknown enterprise connection type" << enterpriseType;
        }
    } else {
        securityPage->updateSecurityChange(connetSetting);
        if (!m_name.isEmpty()) {
            qDebug() << "add new personal connect";
            m_wirelessConnOpration->addConnect(connetSetting);
        } else {
            qDebug() << "addAndConnect personal connect" << m_deviceName;
            m_wirelessConnOpration->addAndActiveWirelessConnect(m_deviceName, connetSetting, true);
        }
    }
    return true;
}

bool NetDetail::updateConnect()
{
    KyConnectResourse *kyConnectResourse = new KyConnectResourse(this);
    KyConnectSetting  connetSetting;
    KySecuType secuType;
    KyEapMethodType enterpriseType;
    kyConnectResourse->getConnectionSetting(m_uuid,connetSetting);

    bool securityChange = false;
    if (isWlan) {
        securityChange = securityPage->checkIsChanged(m_info);
        if(securityChange) {
            securityPage->getSecuType(secuType, enterpriseType);
            if (!checkWirelessSecurity(secuType)) {
                return false;
            }
        }
    }

    if(!m_uuid.isEmpty() && detailPage->checkIsChanged(m_info)) {
        m_wirelessConnOpration->setWirelessAutoConnect(m_uuid, !m_info.isAutoConnect);
    }

    bool ipv4Change = ipv4Page->checkIsChanged(m_info, connetSetting);
    bool ipv6Change = ipv6Page->checkIsChanged(m_info, connetSetting);

    qDebug() << "ipv4Changed" << ipv4Change << "ipv6Change" << ipv6Change;
    if (ipv4Change || ipv6Change) {
        connetSetting.dumpInfo();
        m_wiredConnOperation->updateWiredConnect(m_uuid, connetSetting);
    }

    qDebug() << "securityChange" << securityChange;
    if (securityChange) {
        if (secuType == WPA_AND_WPA2_ENTERPRISE) {
            updateWirelessEnterPriseConnect(enterpriseType);
        } else {
            updateWirelessPersonalConnect();
        }
    }

    if (ipv4Change || ipv6Change || securityChange/* || ipv4Page->checkDnsSettingsIsChanged()*/) {
        if (isActive) {
            //信息变化 断开-重连 更新需要時間 不可以立即重連
//            sleep(1);
            QEventLoop eventloop;
            QTimer::singleShot(1000, &eventloop, SLOT(quit()));
            eventloop.exec();
            m_wirelessConnOpration->activateConnection(m_uuid, m_deviceName);
        }
    }

    if (m_networkMode != DBUS_INVAILD && m_networkMode != NO_CONFIG) {
        int configType = NetworkModeConfig::getInstance()->getNetworkModeConfig(m_uuid);
        bool configPageChange = configPage->checkIsChanged(configType);
        int currentConfigType = configPage->getConfigState();
//        qDebug () << Q_FUNC_INFO << __LINE__<< configPageChange;

        if (configPageChange) {
            NetworkModeConfig::getInstance()->setNetworkModeConfig(m_uuid, m_deviceName, m_name, currentConfigType);
//            qDebug () <<Q_FUNC_INFO << __LINE__ << m_uuid << m_deviceName << m_name << currentConfigType;
        }
    }
    return true;
}

bool NetDetail::checkWirelessSecurity(KySecuType secuType)
{
    if (secuType == WPA_AND_WPA2_ENTERPRISE) {
        if(m_info.strSecType.indexOf("802.1X") < 0) {
            showDesktopNotify(tr("this wifi no support enterprise type"), "networkwrong");
            return false;
        }
    } else {
        if (secuType == NONE && m_info.strSecType != tr("None")) {
            showDesktopNotify(tr("this wifi no support None type"), "networkwrong");
            return false;
        } else if (secuType == WPA_AND_WPA2_PERSONAL
                   && (m_info.strSecType.indexOf("WPA1") < 0 &&
                       m_info.strSecType.indexOf("WPA2") < 0)) {
            showDesktopNotify(tr("this wifi no support WPA2 type"), "networkwrong");
            return false;
        } else if (secuType == WPA3_PERSONAL && m_info.strSecType.indexOf("WPA3") < 0) {
            showDesktopNotify(tr("this wifi no support WPA3 type"), "networkwrong");
            return false;
        }
    }
    return true;
}

bool NetDetail::eventFilter(QObject *w, QEvent *event)
{
   // 回车键触发确定按钮点击事件
   if (event->type() == QEvent::KeyPress) {
       QKeyEvent *mEvent = static_cast<QKeyEvent *>(event);
       if (mEvent->key() == Qt::Key_Enter || mEvent->key() == Qt::Key_Return) {
           if (confimBtn->isEnabled()) {
               Q_EMIT confimBtn->clicked();
           }
           return true;
       } else if (mEvent->key() == Qt::Key_Escape) {
            close();
            return true;
       }
   }
   return QWidget::eventFilter(w, event);
}

void NetDetail::setNetTabToolTip()
{
    int tabCount = m_netTabBar->count();
    for (int i = 0; i< tabCount; ++i) {
        QFontMetrics fontMetrics(m_netTabBar->font());
        int fontSize = fontMetrics.width(m_netTabBar->tabText(i));
        if (fontSize > MAX_TAB_TEXT_LENGTH) {
            m_netTabBar->setTabToolTip(i, m_netTabBar->tabText(i));
        } else {
            m_netTabBar->setTabToolTip(i, "");
        }
    }
}

NetTabBar::NetTabBar(QWidget *parent)
    :KTabBar(KTabBarStyle::SegmentDark, parent)
{

}

NetTabBar::~NetTabBar()
{

}

QSize NetTabBar::sizeHint() const
{
    QSize size = KTabBar::sizeHint();
    size.setWidth(TAB_WIDTH);
    return size;
}

QSize NetTabBar::minimumTabSizeHint(int index) const
{
    Q_UNUSED(index)
    QSize size = KTabBar::minimumTabSizeHint(index);
    size.setWidth(TAB_WIDTH);
    return size;
}

ThreadObject::ThreadObject(QString deviceName, QObject *parent)
    :m_devName(deviceName), QObject(parent)
{
    m_isStop = false;
}

ThreadObject::~ThreadObject()
{

}

void ThreadObject::stop()
{
     m_isStop = true;
}

void ThreadObject::checkIpv4ConflictThread(const QString &ipv4Address)
{
    if (m_isStop) {
        return;
    }
    bool isConflict = false;

    QDBusInterface dbusInterface(SYSTEM_DBUS_SERVICE,
                                 SYSTEM_DBUS_PATH,
                                 SYSTEM_DBUS_INTERFACE,
                                 QDBusConnection::systemBus());

    if(!dbusInterface.isValid()) {
        qWarning ()<< "check IPv4 conflict failed, init lingmo.network.qt.systemdbus error";
        Q_EMIT ipv4IsConflict(isConflict);
        return;
    }

    KyNetworkDeviceResourse resource;
    QStringList devList, devList1, devList2, macList;
    resource.getNetworkDeviceList(NetworkManager::Device::Type::Ethernet, devList1);
    resource.getNetworkDeviceList(NetworkManager::Device::Type::Wifi, devList2);
    devList << devList1 << devList2;
    for (int i = 0; i < devList.size(); ++i) {
        QString hardAddress;
        int band;
        resource.getHardwareInfo(devList.at(i), hardAddress, band);
        macList << hardAddress;
    }

    QDBusReply <bool> reply = dbusInterface.call("checkIpv4IsConflict", m_devName, ipv4Address, macList);
    if (reply.isValid()) {
        isConflict = reply.value();
    } else {
        qWarning () << "check IPv4 conflict failed, dbus reply invalid";
    }

    Q_EMIT ipv4IsConflict(isConflict);
}

void ThreadObject::checkIpv6ConflictThread(const QString &ipv6Address)
{
    if (m_isStop) {
        return;
    }
    bool isConflict = false;
    QDBusInterface dbusInterface(SYSTEM_DBUS_SERVICE,
                                 SYSTEM_DBUS_PATH,
                                 SYSTEM_DBUS_INTERFACE,
                                 QDBusConnection::systemBus());

    if(!dbusInterface.isValid()) {
        qWarning () << "check IPv6 conflict failed, init lingmo.network.qt.systemdbus error";
    } else {
        QDBusReply <bool> reply = dbusInterface.call("checkIpv6IsConflict", m_devName, ipv6Address);
        if (reply.isValid()) {
            isConflict = reply.value();
        } else {
            qWarning () << "check IPv6 conflict failed, dbus reply invalid";
        }
    }

    Q_EMIT ipv6IsConflict(isConflict);
}

void NetDetail::getIpv4Ipv6Info(QString objPath, ConInfo &conInfo)
{
    QDBusInterface m_interface("org.freedesktop.NetworkManager",
                               objPath,
                               "org.freedesktop.NetworkManager.Settings.Connection",
                               QDBusConnection::systemBus());
    QDBusMessage result = m_interface.call("GetSettings");

    if (result.arguments().isEmpty()) { return; }
    const QDBusArgument &dbusArg1st = result.arguments().at( 0 ).value<QDBusArgument>();
    QMap<QString,QMap<QString,QVariant>> map;
    dbusArg1st >> map;

    for (QString key : map.keys() ) {
        QMap<QString,QVariant> innerMap = map.value(key);
        if (key == "ipv4") {
            for (QString innerKey : innerMap.keys()) {
                if (innerKey == "address-data") {
                    //ipv4地址 ipv4子网掩码
                    QMap<QString,QVariant> ipv4Map = getAddressDataFromMap(innerMap, innerKey);
                    if (!ipv4Map.isEmpty()) {
                        conInfo.strIPV4Address = ipv4Map.value("address").toString();
                        conInfo.strIPV4NetMask = ipv4Page->getNetMaskText(ipv4Map.value("prefix").toString());
                    }
                } else if (innerKey == "method") {
                    conInfo.ipv4ConfigType = getIpConfigTypeFromMap(innerMap, innerKey);

                } else if (innerKey == "dns") {
                    conInfo.ipv4DnsList = getIpv4DnsFromMap(innerMap, innerKey);

                } else if (innerKey == "gateway") {
                    conInfo.strIPV4GateWay = innerMap.value(innerKey).toString();
                }
            }
        }
        if (key == "ipv6") {
            for (QString innerKey : innerMap.keys()) {
                if (innerKey == "address-data"){
                    QMap<QString,QVariant> ipv6Map = getAddressDataFromMap(innerMap, innerKey);
                    if (!ipv6Map.isEmpty()) {
                        conInfo.strIPV6Address = ipv6Map.value("address").toString();
                        conInfo.iIPV6Prefix = ipv6Map.value("prefix").toString().toInt();
                    }

                } else if (innerKey == "method") {
                     conInfo.ipv6ConfigType = getIpConfigTypeFromMap(innerMap, innerKey);

                } else if (innerKey == "dns") {
                    conInfo.ipv6DnsList = getIpv6DnsFromMap(innerMap, innerKey);

                } else if (innerKey == "gateway") {
                    conInfo.strIPV6GateWay = innerMap.value(innerKey).toString();
                }
            }
        }
    }
}

QMap<QString, QVariant> NetDetail::getAddressDataFromMap(QMap<QString, QVariant> &innerMap, QString innerKey)
{
    //get address-data: {'address', 'prefix'}
    const QDBusArgument &dbusArg2nd = innerMap.value(innerKey).value<QDBusArgument>();
    QVector<QMap<QString,QVariant>> addressVector;

    dbusArg2nd.beginArray();
    while (!dbusArg2nd.atEnd()) {
        QMap<QString,QVariant> tempMap;
        dbusArg2nd >> tempMap;// append map to a vector here if you want to keep it
        addressVector.append(tempMap);
    }
    dbusArg2nd.endArray();

    return addressVector.size() >= 1 ? addressVector.at(0) : QMap<QString, QVariant>();
}

KyIpConfigType NetDetail::getIpConfigTypeFromMap(QMap<QString, QVariant> &innerMap, QString innerKey)
{
    // get 'method'
    QString strMethod = innerMap.value(innerKey).toString();
    if (strMethod == "auto") {
        return CONFIG_IP_DHCP;
    } else if (strMethod == "manual") {
        return CONFIG_IP_MANUAL;
    } else {
        //TODO: match other types
        return CONFIG_IP_DHCP;
    }
}

QList<QHostAddress> NetDetail::getIpv4DnsFromMap(QMap<QString, QVariant> &innerMap, QString innerKey)
{
    // get ipv4 'dns'
    const QDBusArgument &dbusArg2nd = innerMap.value(innerKey).value<QDBusArgument>();
    QList<QHostAddress> addressVector;

    dbusArg2nd.beginArray();
    while (!dbusArg2nd.atEnd()) {
        uint tmpVar;
        dbusArg2nd >> tmpVar;
        QString dnsi(inet_ntoa(*(struct in_addr *)&tmpVar));
        addressVector.append(QHostAddress(tmpVar));
    }
    dbusArg2nd.endArray();

    return addressVector;
}

QList<QHostAddress> NetDetail::getIpv6DnsFromMap(QMap<QString, QVariant> &innerMap, QString innerKey)
{
    // get ipv6 'dns'
    const QDBusArgument &dbusArg2nd = innerMap.value(innerKey).value<QDBusArgument>();
    QList<QHostAddress> addressVector;

    dbusArg2nd.beginArray();
    while (!dbusArg2nd.atEnd()) {
        QByteArray temArray;
        quint8 tmpVar[16];

        dbusArg2nd >> temArray;
        for (int i = 0; i< 16; ++i) {
            tmpVar[i] = temArray[i];
        }
        addressVector.append(QHostAddress(tmpVar));
    }
    dbusArg2nd.endArray();

    return addressVector;
}
