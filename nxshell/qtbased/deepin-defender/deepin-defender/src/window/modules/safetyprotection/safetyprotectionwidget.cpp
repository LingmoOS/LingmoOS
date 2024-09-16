// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "safetyprotectionwidget.h"
#include "window/modules/common/invokers/invokerinterface.h"
#include "window/modules/common/gsettingkey.h"
#include "window/modules/common/common.h"

#include <DFontSizeManager>

#include <QPushButton>

SafetyProtectionWidget::SafetyProtectionWidget(InvokerFactoryInterface *factory, QWidget *parent)
    : QWidget(parent)
    , m_usbItem(new SafetyProtectionItem(this))
    , m_remRegisterItem(new SafetyProtectionItem(this))
    , m_remControlItem(new SafetyProtectionItem(this))
    , m_netControlItem(new SafetyProtectionItem(this))
    , m_dataUsageItem(new SafetyProtectionItem(this))
    , m_gsetting(nullptr)
    , m_monitorInterFaceServer(nullptr)
    , m_dataInterFaceServer(nullptr)
    , m_hmiScreen(nullptr)
{
    this->setAccessibleName("rightWidget_safetyWidget");

    m_monitorInterFaceServer = factory->CreateInvoker("com.deepin.defender.MonitorNetFlow", "/com/deepin/defender/MonitorNetFlow", "com.deepin.defender.MonitorNetFlow", ConnectType::SYSTEM, this);
    m_dataInterFaceServer = factory->CreateInvoker("com.deepin.defender.datainterface", "/com/deepin/defender/datainterface", "com.deepin.defender.datainterface", ConnectType::SESSION, this);
    m_hmiScreen = factory->CreateInvoker("com.deepin.defender.hmiscreen", "/com/deepin/defender/hmiscreen", "com.deepin.defender.hmiscreen", ConnectType::SESSION, this);
    // 初始化安全中心gsetting配置
    m_gsetting = factory->CreateSettings("com.deepin.dde.deepin-defender", QByteArray(), this);

    // 初始化UI
    initUI();

    // 初始化安全防护每一项开关状态
    initSwitchStatus();

    // 初始化信号槽
    initSignalSLot();

    // 初始化时ssh远程登陆端口状态 由系统开机后的状态决定
    m_monitorInterFaceServer->Invoke("AsyncGetRemRegisterStatus");
}

SafetyProtectionWidget::~SafetyProtectionWidget()
{
}

// 初始化界面
void SafetyProtectionWidget::initUI()
{
    // 安全防护功能 垂直布局
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    // 添加一个滑块区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->viewport()->setAccessibleName("scrollArea_viewWidget");
    scrollArea->setAccessibleName("safetyWidget_scrollArea");
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setObjectName(QStringLiteral("scrollArea"));
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    verticalLayout->addWidget(scrollArea);

    // 将mainWidget设置为滑动区域的主窗口
    QWidget *scrollAreaWidget = new QWidget(scrollArea);
    scrollArea->setWidget(scrollAreaWidget);

    // 滑块区域为垂直布局
    QVBoxLayout *layoutSafety = new QVBoxLayout(scrollArea);
    layoutSafety->setContentsMargins(34, 40, 34, 0);
    layoutSafety->setSpacing(0);

    QWidget *safeItemsContentWidget = new QWidget(scrollArea);
    safeItemsContentWidget->setAccessibleName("viewWidget_contentWidget");
    safeItemsContentWidget->setLayout(layoutSafety);
    scrollArea->setWidget(safeItemsContentWidget);

    // 样式加粗
    QFont ft;
    ft.setBold(true);
    /****************病毒防护***************/
    // 加入病毒防护名称
    QLabel *lbAntiavName = new QLabel(this);
    lbAntiavName->setAccessibleName("contentWidget_antiavLable");
    lbAntiavName->setText(tr("Antivirus Protection"));
    lbAntiavName->setContentsMargins(10, 0, 10, 0);
    lbAntiavName->setFont(ft);
    DFontSizeManager::instance()->bind(lbAntiavName, DFontSizeManager::T5);
    layoutSafety->addWidget(lbAntiavName, 0, Qt::AlignLeft);

    // 加入病毒防护选项 - USB存储设备保护
    layoutSafety->addSpacing(10);
    m_usbItem->setAccessibleName("contentWidget_usbWidget");
    m_usbItem->setAccessibleParentText("usbWidget");
    m_usbItem->setText(tr("USB storage devices"));
    m_usbItem->setSwitchStatus(true);
    m_usbItem->setTextClick();
    layoutSafety->addWidget(m_usbItem);

    // 加入病毒防护说明
    layoutSafety->addSpacing(4);
    DTipLabel *lbAntiavTitle = new DTipLabel("", this);
    lbAntiavTitle->setAccessibleName("contentWidget_antiavTitleLable");
    lbAntiavTitle->setText(tr("Scan the root directory of USB storage devices when connected"));
    lbAntiavTitle->setWordWrap(true);
    lbAntiavTitle->setMinimumWidth(662);
    lbAntiavTitle->setAlignment(Qt::AlignLeft);
    lbAntiavTitle->setContentsMargins(10, 0, 10, 0);
    DFontSizeManager::instance()->bind(lbAntiavTitle, DFontSizeManager::T8);
    layoutSafety->addWidget(lbAntiavTitle, 0, Qt::AlignLeft);

    /****************系统防护***************/
    // 加入系统防护名称
    layoutSafety->addSpacing(20);
    QLabel *lbSystemName = new QLabel(this);
    lbSystemName->setAccessibleName("contentWidget_systemLable");
    lbSystemName->setText(tr("System Protection"));
    lbSystemName->setContentsMargins(10, 0, 10, 0);
    lbSystemName->setFont(ft);
    DFontSizeManager::instance()->bind(lbSystemName, DFontSizeManager::T5);
    layoutSafety->addWidget(lbSystemName, 0, Qt::AlignLeft);

    // 加入系统防护选项 - 远程登录端口
    layoutSafety->addSpacing(10);
    m_remRegisterItem->setAccessibleName("contentWidget_registerWidget");
    m_remRegisterItem->setAccessibleParentText("registerWidget");
    m_remRegisterItem->setText(tr("Remote desktop port"));
    m_remRegisterItem->setSwitchStatus(true);
    layoutSafety->addWidget(m_remRegisterItem);

    // 加入系统防护说明
    layoutSafety->addSpacing(4);
    DTipLabel *lbSystemTitle = new DTipLabel("", this);
    lbSystemTitle->setAccessibleName("contentWidget_systemTitleLable");
    lbSystemTitle->setText(tr("Open or close the RDP port"));
    lbSystemTitle->setAlignment(Qt::AlignLeft);
    lbSystemTitle->setContentsMargins(10, 0, 0, 0);
    DFontSizeManager::instance()->bind(lbSystemTitle, DFontSizeManager::T8);
    layoutSafety->addWidget(lbSystemTitle, 0, Qt::AlignLeft);

    /****************网络防护***************/
    // 加入网络防护名称
    layoutSafety->addSpacing(20);
    QLabel *lbNetName = new QLabel(this);
    lbNetName->setAccessibleName("contentWidget_netLable");
    lbNetName->setText(tr("Internet Security"));
    lbNetName->setContentsMargins(10, 0, 10, 0);
    lbNetName->setFont(ft);
    DFontSizeManager::instance()->bind(lbNetName, DFontSizeManager::T5);
    layoutSafety->addWidget(lbNetName, 0, Qt::AlignLeft);

    QVariantList args = m_dataInterFaceServer->Invoke("GetUosResourceStatus", {}, BlockMode::BLOCK).arguments();
    bool bStatus = args.takeFirst().toBool();

    if (bStatus) {
        // 加入网络防护选项 - 远程访问控制
        layoutSafety->addSpacing(10);
        m_remControlItem->setAccessibleName("contentWidget_remControlWidget");
        m_remControlItem->setAccessibleParentText("remControlWidget");
        m_remControlItem->setText(tr("Remote connection"));
        m_remControlItem->setSwitchStatus(true);
        m_remControlItem->setTextClick();
        layoutSafety->addWidget(m_remControlItem);

        // 加入远程访问控制说明
        layoutSafety->addSpacing(4);
        DTipLabel *lbRemControlTitle = new DTipLabel("", this);
        lbRemControlTitle->setAccessibleName("contentWidget_remControlTitleLable");
        lbRemControlTitle->setText(tr("Specify whether remote access to your system is permitted or not"));
        lbRemControlTitle->setAlignment(Qt::AlignLeft);
        lbRemControlTitle->setContentsMargins(10, 0, 10, 0);
        DFontSizeManager::instance()->bind(lbRemControlTitle, DFontSizeManager::T8);
        layoutSafety->addWidget(lbRemControlTitle, 0, Qt::AlignLeft);

        // 加入网络防护选项 - 联网访问控制
        layoutSafety->addSpacing(10);
        m_netControlItem->setAccessibleName("contentWidget_netControlWidget");
        m_netControlItem->setAccessibleParentText("netControlWidget");
        m_netControlItem->setText(tr("Internet control"));
        m_netControlItem->setSwitchStatus(true);
        m_netControlItem->setTextClick();
        layoutSafety->addWidget(m_netControlItem);

        // 加入联网访问控制说明
        layoutSafety->addSpacing(4);
        DTipLabel *lbNetControlTitle = new DTipLabel("", this);
        lbNetControlTitle->setAccessibleName("contentWidget_netControlTitleLable");
        lbNetControlTitle->setText(tr("Allow or disable the Internet access of applications"));
        lbNetControlTitle->setAlignment(Qt::AlignLeft);
        lbNetControlTitle->setContentsMargins(10, 0, 10, 0);
        DFontSizeManager::instance()->bind(lbNetControlTitle, DFontSizeManager::T8);
        layoutSafety->addWidget(lbNetControlTitle, 0, Qt::AlignLeft);
    } else {
        m_remControlItem->setVisible(false);
        m_netControlItem->setVisible(false);
    }

    // 加入网络防护选项 - 流量详情
    layoutSafety->addSpacing(10);
    m_dataUsageItem->setAccessibleName("contentWidget_dataUsagelWidget");
    m_dataUsageItem->setAccessibleParentText("dataUsagelWidget");
    m_dataUsageItem->setText(tr("Data usage"));
    m_dataUsageItem->setSwitchStatus(true);
    m_dataUsageItem->setTextClick();
    layoutSafety->addWidget(m_dataUsageItem);

    // 加入流量详情说明
    layoutSafety->addSpacing(4);
    DTipLabel *lbDataUsageTitle = new DTipLabel("", this);
    lbDataUsageTitle->setAccessibleName("contentWidget_dataUsageTitleLable");
    lbDataUsageTitle->setText(tr("Enable or disable the monitoring of data usage"));
    lbDataUsageTitle->setAlignment(Qt::AlignLeft);
    lbDataUsageTitle->setContentsMargins(10, 0, 10, 0);
    DFontSizeManager::instance()->bind(lbDataUsageTitle, DFontSizeManager::T8);
    layoutSafety->addWidget(lbDataUsageTitle, 0, Qt::AlignLeft);

    // 安全防护底部加入弹簧
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layoutSafety->addItem(verticalSpacer);
}

// 初始化信号
void SafetyProtectionWidget::initSignalSLot()
{
    // USB存储设备保护状态改变
    connect(m_usbItem, &SafetyProtectionItem::switchPressed, this, &SafetyProtectionWidget::doUsbItemStatusChange);
    // 远程端口登陆状态改变
    connect(m_remRegisterItem, &SafetyProtectionItem::switchPressed, this, &SafetyProtectionWidget::doRemRegisterStatusChange);
    // 当ssh状态改变时
    m_monitorInterFaceServer->Connect("SendSSHStatus", this, SLOT(recSSHStatus(bool)));
    // 远程访问控制状态改变
    connect(m_remControlItem, &SafetyProtectionItem::switchPressed, this, &SafetyProtectionWidget::doRemControlStatusChange);
    // 联网控制状态改变
    connect(m_netControlItem, &SafetyProtectionItem::switchPressed, this, &SafetyProtectionWidget::doNetControlStatusChange);
    // 流量详情状态改变
    connect(m_dataUsageItem, &SafetyProtectionItem::switchPressed, this, &SafetyProtectionWidget::doDataUsageStatusChange);

    // USB存储设备保护标题点击信号
    connect(m_usbItem, &SafetyProtectionItem::sendTextClickedChange, this, &SafetyProtectionWidget::doUsbTextClicked);
    // 远程访问标题点击信号
    connect(m_remControlItem, &SafetyProtectionItem::sendTextClickedChange, this, &SafetyProtectionWidget::doRemControlTextClicked);
    // 联网管控标题点击信号
    connect(m_netControlItem, &SafetyProtectionItem::sendTextClickedChange, this, &SafetyProtectionWidget::doNetControlTextClicked);
    // 流量详情标题点击信号
    connect(m_dataUsageItem, &SafetyProtectionItem::sendTextClickedChange, this, &SafetyProtectionWidget::doDataUsageTextClicked);
}

// 初始化安全防护每一项开关状态
void SafetyProtectionWidget::initSwitchStatus()
{
    // USB存储设备保护/远程端口/远程控制/联网管控/流量监控 状态初始化
    m_usbItem->setSwitchStatus(m_gsetting->GetValue(AUTO_SCAN_USB).toBool());
    m_remRegisterItem->setSwitchStatus(m_gsetting->GetValue(REM_REGISTER_ON_OFF).toBool());
    m_remRegisterItem->enableSwitch(false);

    m_remControlItem->setSwitchStatus(m_gsetting->GetValue(REM_CONTROL_ON_OFF).toBool());
    m_netControlItem->setSwitchStatus(m_gsetting->GetValue(NET_CONTROL_ON_OFF).toBool());
    m_dataUsageItem->setSwitchStatus(m_gsetting->GetValue(DATA_USAGE_ON_OFF).toBool());
}

// USB存储设备保护状态改变
void SafetyProtectionWidget::doUsbItemStatusChange()
{
    // 状态取反
    bool isbStatus = !m_gsetting->GetValue(AUTO_SCAN_USB).toBool();
    m_gsetting->SetValue(AUTO_SCAN_USB, isbStatus);

    // 添加安全日志
    if (isbStatus) {
        writeSecurityLog(tr("Scan USB storage devices: ON"));
    } else {
        writeSecurityLog(tr("Scan USB storage devices: OFF"));
    }
}

// 远程端口登陆状态改变
void SafetyProtectionWidget::doRemRegisterStatusChange()
{
    // 状态取反
    bool status = !m_gsetting->GetValue(REM_REGISTER_ON_OFF).toBool();
    // 设置状态存在处理时间，为防止造成界面卡顿，在设置状态完成前禁用按钮
    m_remRegisterItem->enableSwitch(false);
    m_gsetting->SetValue(REM_REGISTER_ON_OFF, status);
    m_monitorInterFaceServer->Invoke("AsyncSetRemRegisterStatus", QList<QVariant>() << status, BlockMode::NOBLOCK);

    if (status) {
        writeSecurityLog(tr("Remote desktop port: ON"));
    } else {
        writeSecurityLog(tr("Remote desktop port: OFF"));
    }
}

void SafetyProtectionWidget::recSSHStatus(bool status)
{
    // 设置状态完成前启用按钮
    m_remRegisterItem->enableSwitch(true);
    // 设置远程登陆控制控件开关状态
    m_remRegisterItem->setSwitchStatus(status);

    m_gsetting->SetValue(REM_REGISTER_ON_OFF, status);
}

// 远程访问控制状态改变
void SafetyProtectionWidget::doRemControlStatusChange()
{
    // 状态取反
    bool isbStatus = !m_gsetting->GetValue(REM_CONTROL_ON_OFF).toBool();
    m_gsetting->SetValue(REM_CONTROL_ON_OFF, isbStatus);
    m_dataInterFaceServer->Invoke("setRemControlSwitchStatus", QList<QVariant>() << isbStatus, BlockMode::NOBLOCK);

    // 添加安全日志
    if (isbStatus) {
        writeSecurityLog(tr("Remote connection: ON"));
    } else {
        writeSecurityLog(tr("Remote connection: OFF"));
    }
}

// 联网控制状态改变
void SafetyProtectionWidget::doNetControlStatusChange()
{
    // 状态取反
    bool isbStatus = !m_gsetting->GetValue(NET_CONTROL_ON_OFF).toBool();
    m_gsetting->SetValue(NET_CONTROL_ON_OFF, isbStatus);
    m_dataInterFaceServer->Invoke("setNetControlSwitchStatus", QList<QVariant>() << isbStatus, BlockMode::NOBLOCK);

    // 添加安全日志
    if (isbStatus) {
        writeSecurityLog(tr("Internet control: ON"));
    } else {
        writeSecurityLog(tr("Internet control: OFF"));
    }
}

// 流量详情状态改变
void SafetyProtectionWidget::doDataUsageStatusChange()
{
    // 状态取反
    bool isbStatus = !m_gsetting->GetValue(DATA_USAGE_ON_OFF).toBool();
    m_gsetting->SetValue(DATA_USAGE_ON_OFF, isbStatus);
    m_monitorInterFaceServer->Invoke("EnableNetFlowMonitor", QList<QVariant>() << isbStatus, BlockMode::NOBLOCK);

    // 添加安全日志
    if (isbStatus) {
        writeSecurityLog(tr("Data usage: ON"));
    } else {
        writeSecurityLog(tr("Data usage: OFF"));
    }
}

// USB存储设备保护标题点击槽
void SafetyProtectionWidget::doUsbTextClicked()
{
    m_hmiScreen->Invoke("showFuncConnectRemControl", QList<QVariant>() << "");
}

// 远程访问控制标题点击槽
void SafetyProtectionWidget::doRemControlTextClicked()
{
    m_hmiScreen->Invoke("showFuncConnectRemControl", QList<QVariant>() << "");
}

// 联网管控标题点击槽
void SafetyProtectionWidget::doNetControlTextClicked()
{
    m_hmiScreen->Invoke("showFuncConnectNetControl", QList<QVariant>() << "");
}

// 流量详情标题点击槽
void SafetyProtectionWidget::doDataUsageTextClicked()
{
    m_hmiScreen->Invoke("showFuncConnectDataUsage");
}

void SafetyProtectionWidget::writeSecurityLog(const QString &info) const
{
    m_monitorInterFaceServer->Invoke("AddSecurityLog", QList<QVariant>() << SECURITY_LOG_TYPE_PROTECTION << info, BlockMode::NOBLOCK);
}
