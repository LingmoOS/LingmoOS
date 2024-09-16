// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netcheckwidget.h"
#include "window/modules/common/common.h"
#include "window/modules/common/compixmap.h"
#include "netcheckitem.h"

#include <DLabel>
#include <DTipLabel>
#include <DLineEdit>
#include <DFontSizeManager>
#include <DFloatingMessage>
#include <DMessageManager>

#include <QVBoxLayout>
#include <QCloseEvent>

#define NET_SETTING_PUBLIC "public"
#define NET_SETTING_INTRANET "intranet"
NetCheckWidget::NetCheckWidget(NetCheckModel *model, QWidget *parent)
    : DFrame(parent)
    , m_model(model)
    , m_netSettingwidget(nullptr)
    , m_netCheckwidget(nullptr)
    , m_netSettingBtn(nullptr)
    , m_netSettingMenu(nullptr)
    , m_domainWidget(nullptr)
    , m_domainEdit(nullptr)
    , m_detectNowBtn(nullptr)
    , m_timer(nullptr)
    , m_netCheckInfo(nullptr)
    , m_timeLabel(nullptr)
    , m_netDeviceFrame(nullptr)
    , m_netSettingFrame(nullptr)
    , m_netDHCPFrame(nullptr)
    , m_netDNSFrame(nullptr)
    , m_netHostFrame(nullptr)
    , m_netConnFrame(nullptr)
    , m_cancelBtn(nullptr)
    , m_restartBtn(nullptr)
    , m_finishBtn(nullptr)
    , m_checkingFlag(false)
{
    // 初始化
    initUI();
    initData();
}

NetCheckWidget::~NetCheckWidget()
{
    if (nullptr != m_timer) {
        delete m_timer;
    }
}

// 初始化界面
void NetCheckWidget::initUI()
{
    setFocusPolicy(Qt::FocusPolicy::ClickFocus); // 设置可点击聚焦

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop);
    this->setLayout(mainLayout);

    // 加载界面
    mainLayout->addWidget(loadNetConnSettingUI());
    mainLayout->addWidget(loadNetConnCheckUI());
    m_netSettingwidget->setVisible(true);
    m_netCheckwidget->setVisible(false);
}

void NetCheckWidget::initData()
{
    // 网络类型改变
    connect(m_netSettingMenu, &QMenu::triggered, this, &NetCheckWidget::selectNetType);
    // 立即检测
    connect(m_detectNowBtn, &DSuggestButton::clicked, this, &NetCheckWidget::checkNetConnect);
    // 取消检测
    connect(m_cancelBtn, &DSuggestButton::clicked, this, &NetCheckWidget::cancelNetCheck);
    // 重新检测
    connect(m_restartBtn, &DSuggestButton::clicked, this, &NetCheckWidget::restartNetCheck);
    // 完成
    connect(m_finishBtn, &DSuggestButton::clicked, this, &NetCheckWidget::finishNetCheck);

    // 网络检测界面重置
    connect(m_model, &NetCheckModel::notifyNetCheckReset, this, &NetCheckWidget::resetNetCheckItem);
    connect(m_model, &NetCheckModel::notifyNetDeviceCheck, this, &NetCheckWidget::acceptNetDeviceCheck);
    connect(m_model, &NetCheckModel::notifyNetSettingCheck, this, &NetCheckWidget::acceptNetSettingCheck);
    connect(m_model, &NetCheckModel::notifyNetDHCPCheck, this, &NetCheckWidget::acceptNetDHCPCheck);
    connect(m_model, &NetCheckModel::notifyNetDNSCheck, this, &NetCheckWidget::acceptNetDNSCheck);
    connect(m_model, &NetCheckModel::notifyNetHostCheck, this, &NetCheckWidget::acceptNetHostCheck);
    connect(m_model, &NetCheckModel::notifyNetConnCheck, this, &NetCheckWidget::acceptNetConnCheck);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));

    QString defaultType;
    // 设置默认的网络类型
    if (m_model->getNetCheckType()) {
        defaultType = NET_SETTING_INTRANET;
        m_domainWidget->setVisible(true);
    } else {
        defaultType = NET_SETTING_PUBLIC;
        m_domainWidget->setVisible(false);
    }
    for (QAction *ttaction : m_netSettingBtn->menu()->actions()) {
        if (ttaction->objectName() == defaultType) {
            ttaction->setChecked(true);
            m_netSettingBtn->setText(ttaction->text());
            m_lastNetType = defaultType;
        } else {
            ttaction->setChecked(false);
        }
    }

    // 设置默认的域名
    QString domainText = m_model->getNetCheckIntranet();
    if (!domainText.isEmpty()) {
        m_domainEdit->setText(domainText);
    }
}

// 加载网络设置界面
QWidget *NetCheckWidget::loadNetConnSettingUI()
{
    m_netSettingwidget = new QWidget(this);
    QVBoxLayout *widgetLayout = new QVBoxLayout(m_netSettingwidget);
    widgetLayout->setContentsMargins(10, 10, 10, 10);
    widgetLayout->setSpacing(10);

    // 标题
    QLabel *title = new QLabel(tr("Network Detection"), this);
    QFont ft = title->font();
    ft.setBold(true);
    title->setFont(ft);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T5);
    widgetLayout->addWidget(title, 0, Qt::AlignLeft | Qt::AlignTop);

    // 标题说明
    DTipLabel *titleTip = new DTipLabel(tr("One-click detection, comprehensive network troubleshooting"), this);
    DFontSizeManager::instance()->bind(titleTip, DFontSizeManager::T8);
    widgetLayout->addWidget(titleTip, 0, Qt::AlignLeft | Qt::AlignTop);

    // 网络环境配置 -标题
    QLabel *netSettingTitle = new QLabel(tr("Network Environment Configuration"), this);
    QFont ftNet = netSettingTitle->font();
    ftNet.setBold(true);
    netSettingTitle->setFont(ftNet);
    DFontSizeManager::instance()->bind(netSettingTitle, DFontSizeManager::T6);
    widgetLayout->addWidget(netSettingTitle, 0, Qt::AlignLeft | Qt::AlignTop);

    // 网络环境配置
    DFrame *netSettingFrame = new DFrame(this);
    netSettingFrame->setBackgroundRole(DPalette::ItemBackground);
    netSettingFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    netSettingFrame->setLineWidth(0);
    widgetLayout->addWidget(netSettingFrame);

    // 网络环境配置布局
    QVBoxLayout *netSettingLayout = new QVBoxLayout;
    netSettingLayout->setContentsMargins(10, 10, 10, 10);
    netSettingLayout->setSpacing(10);
    netSettingLayout->setAlignment(Qt::AlignTop);
    netSettingFrame->setLayout(netSettingLayout);

    // 网络设置
    QWidget *netWidget = new QWidget(this);
    netWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    netSettingLayout->addWidget(netWidget);
    // 网络设置 - 布局
    QHBoxLayout *netLayout = new QHBoxLayout;
    netLayout->setContentsMargins(0, 0, 0, 0);
    netLayout->setSpacing(0);
    netLayout->setAlignment(Qt::AlignTop);
    netWidget->setLayout(netLayout);
    // 网络设置 - 标题
    QLabel *netTitle = new QLabel(tr("Network Environment"), this);
    DFontSizeManager::instance()->bind(netTitle, DFontSizeManager::T6);
    netLayout->addWidget(netTitle, 0, Qt::AlignLeft | Qt::AlignVCenter);
    netLayout->addStretch();
    // 网络设置 - 下拉框
    m_netSettingBtn = new QPushButton(this);
    m_netSettingBtn->setFixedSize(QSize(140, 36));
    m_netSettingMenu = new QMenu(this);
    m_netSettingBtn->setMenu(m_netSettingMenu);
    QAction *publicAction = new QAction(tr("Public Network"), this);
    publicAction->setObjectName(NET_SETTING_PUBLIC);
    publicAction->setCheckable(true);
    m_netSettingMenu->addAction(publicAction);
    QAction *intranetAction = new QAction(tr("Intranet"), this);
    intranetAction->setObjectName(NET_SETTING_INTRANET);
    intranetAction->setCheckable(true);
    m_netSettingMenu->addAction(intranetAction);
    netLayout->addWidget(m_netSettingBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    // 域名设置
    m_domainWidget = new QWidget(this);
    m_domainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    netSettingLayout->addWidget(m_domainWidget);
    // 域名设置 - 布局
    QHBoxLayout *domainLayout = new QHBoxLayout;
    domainLayout->setContentsMargins(0, 0, 0, 0);
    domainLayout->setSpacing(0);
    domainLayout->setAlignment(Qt::AlignTop);
    m_domainWidget->setLayout(domainLayout);
    // 域名设置 - 标题
    QLabel *domainTitle = new QLabel(tr("Intranet IP/Domain"), this);
    DFontSizeManager::instance()->bind(domainTitle, DFontSizeManager::T6);
    domainLayout->addWidget(domainTitle, 0, Qt::AlignLeft | Qt::AlignVCenter);
    domainLayout->addStretch();
    // 域名设置 - 文本框
    m_domainEdit = new DLineEdit(this);
    m_domainEdit->setFixedSize(QSize(280, 36));
    m_domainEdit->setPlaceholderText(tr("Example: 127.0.0.1 or www.uniontech.com"));
    domainLayout->addWidget(m_domainEdit, 0, Qt::AlignLeft | Qt::AlignVCenter);
    connect(m_domainEdit, &DLineEdit::textChanged, this, [=]() {
        if (m_domainEdit->text().isEmpty()) {
            m_domainEdit->setAlert(true);
        } else {
            m_domainEdit->setAlert(false);
        }
    });

    widgetLayout->addStretch();
    // 检测按钮
    m_detectNowBtn = new DSuggestButton(this);
    m_detectNowBtn->setFixedWidth(200);
    m_detectNowBtn->setText(tr("Detect Now"));
    widgetLayout->addWidget(m_detectNowBtn, 0, Qt::AlignRight | Qt::AlignBottom);

    return m_netSettingwidget;
}

// 加载网络检测界面
QWidget *NetCheckWidget::loadNetConnCheckUI()
{
    m_netCheckwidget = new QWidget(this);
    QVBoxLayout *widgetLayout = new QVBoxLayout(m_netCheckwidget);
    widgetLayout->setContentsMargins(30, 20, 30, 10);
    widgetLayout->setSpacing(10);

    // 网络检测信息 - 布局
    QHBoxLayout *netCheckInfoLayout = new QHBoxLayout;
    netCheckInfoLayout->setContentsMargins(0, 0, 0, 0);
    netCheckInfoLayout->setSpacing(20);
    widgetLayout->addLayout(netCheckInfoLayout);
    // 网络检测信息 - 图标
    QLabel *m_netCheckIcon = new QLabel(this);
    m_netCheckIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_netCheckIcon->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_netCheckIcon->setPixmap(QIcon::fromTheme(FILE_NET_CHECK_ICON).pixmap(96, 96));
    netCheckInfoLayout->addWidget(m_netCheckIcon);
    // 网络检测信息 - 子布局
    QVBoxLayout *netcheckTimeLayout = new QVBoxLayout;
    netcheckTimeLayout->setContentsMargins(0, 0, 0, 0);
    netcheckTimeLayout->setSpacing(10);
    netCheckInfoLayout->addLayout(netcheckTimeLayout);
    // 网络检测信息 - 子布局 - 信息
    m_netCheckInfo = new QLabel(tr("Network detecting, please wait"), this);
    QFont ft = m_netCheckInfo->font();
    ft.setBold(true);
    m_netCheckInfo->setFont(ft);
    DFontSizeManager::instance()->bind(m_netCheckInfo, DFontSizeManager::T5);
    netcheckTimeLayout->addWidget(m_netCheckInfo, 0, Qt::AlignLeft | Qt::AlignBottom);
    // 网络检测信息 - 子布局 - 时间
    QHBoxLayout *timeLayout = new QHBoxLayout;
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(0);
    netcheckTimeLayout->addLayout(timeLayout);
    QLabel *netCheckTime = new QLabel(tr("Detection Time"), this);
    netCheckTime->setAlignment(Qt::AlignLeft);
    netCheckTime->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    DFontSizeManager::instance()->bind(netCheckTime, DFontSizeManager::T8);
    timeLayout->addWidget(netCheckTime, 0, Qt::AlignLeft | Qt::AlignTop);
    m_timeLabel = new QLabel(": 00:00:00", this);
    m_timeLabel->setAlignment(Qt::AlignLeft);
    m_timeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    DFontSizeManager::instance()->bind(m_timeLabel, DFontSizeManager::T8);
    timeLayout->addWidget(m_timeLabel, 0, Qt::AlignLeft | Qt::AlignTop);
    timeLayout->addStretch();

    // 分割线
    QFrame *line = new QFrame();
    line->setFixedWidth(660);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    widgetLayout->addWidget(line, 0, Qt::AlignHCenter);

    // 网络设备检测
    m_netDeviceFrame = new NetCheckItem(tr("Network Device Detection"), this);
    widgetLayout->addWidget(m_netDeviceFrame);
    // 网络设置检测
    m_netSettingFrame = new NetCheckItem(tr("Network Settings Detection"), this);
    widgetLayout->addWidget(m_netSettingFrame);
    // DHCP服务
    m_netDHCPFrame = new NetCheckItem(tr("DHCP service"), this);
    widgetLayout->addWidget(m_netDHCPFrame);
    // DNS服务
    m_netDNSFrame = new NetCheckItem(tr("DNS service"), this);
    widgetLayout->addWidget(m_netDNSFrame);
    // Host文件配置
    m_netHostFrame = new NetCheckItem(tr("host file configuration"), this);
    widgetLayout->addWidget(m_netHostFrame);
    // 能否访问网络
    m_netConnFrame = new NetCheckItem(tr("Network accessibility"), this);
    widgetLayout->addWidget(m_netConnFrame);

    // 操作按钮 - 布局
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->setSpacing(10);
    widgetLayout->addLayout(btnLayout, Qt::AlignBottom);

    btnLayout->addStretch();
    // 取消检测
    m_cancelBtn = new QPushButton();
    m_cancelBtn->setFixedWidth(200);
    m_cancelBtn->setText(tr("Cancel detection"));
    btnLayout->addWidget(m_cancelBtn, 0, Qt::AlignRight | Qt::AlignBottom);
    // 重新检测
    m_restartBtn = new QPushButton();
    m_restartBtn->setFixedWidth(200);
    m_restartBtn->setText(tr("Retry"));
    btnLayout->addWidget(m_restartBtn, 0, Qt::AlignRight | Qt::AlignBottom);
    // 完成
    m_finishBtn = new DSuggestButton();
    m_finishBtn->setFixedWidth(200);
    m_finishBtn->setText(tr("Done"));
    btnLayout->addWidget(m_finishBtn, 0, Qt::AlignRight | Qt::AlignBottom);

    return m_netCheckwidget;
}

void NetCheckWidget::selectNetType(QAction *action)
{
    QString netType = action->objectName();
    if (m_lastNetType != netType) {
        m_lastNetType = netType;
        for (QAction *ttaction : m_netSettingBtn->menu()->actions()) {
            if (ttaction->objectName() == m_lastNetType) {
                ttaction->setChecked(true);
                m_netSettingBtn->setText(ttaction->text());
            } else {
                ttaction->setChecked(false);
            }
        }

        if (NET_SETTING_PUBLIC == m_lastNetType) {
            m_domainWidget->setVisible(false);
            m_model->setNetCheckType(0);
        } else {
            m_domainWidget->setVisible(true);
            m_model->setNetCheckType(1);
        }
    }
}

void NetCheckWidget::checkNetConnect()
{
    if (m_model->getNetCheckStatus()) {
        DFloatingMessage *floMsgSwitchOff = new DFloatingMessage(DFloatingMessage::TransientType);
        floMsgSwitchOff->setDuration(3000);
        floMsgSwitchOff->setIcon(QIcon::fromTheme(DIALOG_TIP_YELLOW));
        floMsgSwitchOff->setMessage(tr("Due to abnormal application exit, services need initialization, please wait."));
        DMessageManager::instance()->sendMessage(this->parentWidget()->parentWidget(), floMsgSwitchOff);
        DMessageManager::instance()->setContentMargens(this, QMargins(0, 0, 0, 10));
        return;
    }

    m_checkingFlag = false;
    m_cancelBtn->setEnabled(true);

    m_domainText = m_domainEdit->text();
    if (NET_SETTING_INTRANET == m_lastNetType && m_domainText.isEmpty()) {
        m_domainEdit->setAlert(true);
        m_domainEdit->showAlertMessage(tr("Intranet IP/Domain cannot be empty"), 3000);
        return;
    } else {
        m_model->setNetCheckIntranet(m_domainText);
    }

    m_netSettingwidget->setVisible(false);
    m_netCheckwidget->setVisible(true);

    m_model->startNetCheck(m_domainText);
}

// 重置检测项
void NetCheckWidget::resetNetCheckItem()
{
    m_netDeviceFrame->resetNetCheckInfo();
    m_netSettingFrame->resetNetCheckInfo();
    m_netDHCPFrame->resetNetCheckInfo();
    m_netDNSFrame->resetNetCheckInfo();
    m_netHostFrame->resetNetCheckInfo();
    m_netConnFrame->resetNetCheckInfo();

    m_countTime = 0;
    m_timer->start(1000);

    m_netCheckInfo->setText(tr("Network detecting, please wait"));
    showCheckWidget(false);
    m_netDeviceFrame->startNetChecking();
}

void NetCheckWidget::acceptNetDeviceCheck(int result)
{
    if (m_checkingFlag) {
        m_timer->stop();
        finishNetCheck();
        return;
    }

    if (ServiceResult::CheckSucessed == result) {
        m_netDeviceFrame->stopNetCheck(CheckStatus::CheckSucess, tr("Network device detected normal"));

        m_netSettingFrame->startNetChecking();
        m_model->startCheckNetSetting();
    } else {
        if (ServiceResult::ServiceBlock == result) {
            m_netDeviceFrame->stopNetCheck(CheckStatus::CheckFailed, tr("Network device disabled"));
        } else if (ServiceResult::NoFound == result) {
            m_netDeviceFrame->stopNetCheck(CheckStatus::CheckFailed, tr("No valid network device detected"));
        }

        m_netSettingFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));
        m_netDHCPFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));
        m_netDNSFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));
        m_netHostFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));
        m_netConnFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));

        m_netCheckInfo->setText(tr("Abnormalities exist, please fix and retry"));
        showCheckWidget(true);
    }
}
void NetCheckWidget::acceptNetSettingCheck(int result)
{
    if (m_checkingFlag) {
        m_timer->stop();
        finishNetCheck();
        return;
    }

    m_checkInfoErrorFlag = true;
    if (NetSettingResult::NoNetConn == result) {
        m_netSettingFrame->stopNetCheck(CheckStatus::CheckFailed, tr("Not connected to any network"));

        m_netDHCPFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));
        m_netDNSFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));
        m_netHostFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));
        m_netConnFrame->stopNetCheck(CheckStatus::NoCheck, tr("Not Detected"));

        m_netCheckInfo->setText(tr("Abnormalities exist, please fix and retry"));
        showCheckWidget(true);
    } else {
        if (NetSettingResult::NetSettingSkip == result) {
            m_netSettingFrame->stopNetCheck(CheckStatus::CheckSucess, tr("IP auto configuration, skip detection"));
        } else if (NetSettingResult::NetSettingCheckFailed == result) {
            m_netSettingFrame->stopNetCheck(CheckStatus::CheckFailed, tr("IP manual configuration, network settings abnormal"));
            m_checkInfoErrorFlag = false;
        } else {
            m_netSettingFrame->stopNetCheck(CheckStatus::CheckSucess, tr("IP manual configuration, network settings normal"));
        }
        m_netDHCPFrame->startNetChecking();
        m_model->startCheckNetDHCP();
    }
}
void NetCheckWidget::acceptNetDHCPCheck(int result)
{
    if (m_checkingFlag) {
        m_timer->stop();
        finishNetCheck();
        return;
    }

    if (DHCPResult::DHCPCheckFailed == result) {
        m_netDHCPFrame->stopNetCheck(CheckStatus::CheckFailed, tr("DHCP service abnormal"));
        m_checkInfoErrorFlag = false;
    } else {
        m_netDHCPFrame->stopNetCheck(CheckStatus::CheckSucess, tr("DHCP service normal"));
    }

    m_netDNSFrame->startNetChecking();
    m_model->startCheckNetDNS();
}
void NetCheckWidget::acceptNetDNSCheck(int result)
{
    if (m_checkingFlag) {
        m_timer->stop();
        finishNetCheck();
        return;
    }

    if (DNSResult::DNSCheckError == result) {
        m_netDNSFrame->stopNetCheck(CheckStatus::CheckFailed, tr("DNS server unreachable"));
        m_checkInfoErrorFlag = false;
    } else if (DNSResult::DNSCheckAbnormal == result) {
        m_netDNSFrame->stopNetCheck(CheckStatus::CheckFailed, tr("DNS server resolution abnormal"));
        m_checkInfoErrorFlag = false;
    } else {
        m_netDNSFrame->stopNetCheck(CheckStatus::CheckSucess, tr("DNS service normal"));
    }

    m_netHostFrame->startNetChecking();
    m_model->startCheckNetHostFile();
}
void NetCheckWidget::acceptNetHostCheck(int result)
{
    if (m_checkingFlag) {
        m_timer->stop();
        finishNetCheck();
        return;
    }

    if (HostResult::HostCheckFailed == result) {
        m_netHostFrame->stopNetCheck(CheckStatus::CheckFailed, tr("host file format error"));
        m_checkInfoErrorFlag = false;
    } else {
        m_netHostFrame->stopNetCheck(CheckStatus::CheckSucess, tr("host file configuration normal"));
    }

    m_netConnFrame->startNetChecking();
    m_model->startCheckNetConn();
}
void NetCheckWidget::acceptNetConnCheck(int result)
{
    if (m_checkingFlag) {
        m_timer->stop();
        finishNetCheck();
        return;
    }

    if (NetConnResult::NetConnSucessed == result) {
        m_netConnFrame->stopNetCheck(CheckStatus::CheckSucess, tr("Network access normal"));
    } else {
        if (m_model->checkVPNOrProxyOpen()) {
            m_netConnFrame->stopNetCheck(CheckStatus::CheckFailed, tr("Network access abnormal, please disable system proxy or VPN and retry"));
        } else {
            m_netConnFrame->stopNetCheck(CheckStatus::CheckFailed, tr("Network access abnormal"));
        }
        m_checkInfoErrorFlag = false;
    }

    if (m_checkInfoErrorFlag) {
        m_netCheckInfo->setText(tr("Detection completed, network normal"));
    } else {
        m_netCheckInfo->setText(tr("Abnormalities exist, please fix and retry"));
    }
    showCheckWidget(true);
}

// 检测开始/完成控件显示
void NetCheckWidget::showCheckWidget(bool status)
{
    m_cancelBtn->setVisible(!status);
    m_restartBtn->setVisible(status);
    m_finishBtn->setVisible(status);

    if (status) {
        m_timer->stop();
    }
}

void NetCheckWidget::updateTime()
{
    m_countTime++;

    // 小时
    int h = m_countTime / 3600;
    // 分钟
    int m = (m_countTime - h * 3600) / 60;
    // 秒
    int s = (m_countTime - h * 3600 - m * 60) % 60;
    m_timeLabel->setText(QString(": %1:%2:%3")
                             .arg(h, 2, 10, QChar('0'))
                             .arg(m, 2, 10, QChar('0'))
                             .arg(s, 2, 10, QChar('0')));
}

// 取消检测
void NetCheckWidget::cancelNetCheck()
{
    m_checkingFlag = true;
    m_cancelBtn->setEnabled(false);
}

// 重新检测
void NetCheckWidget::restartNetCheck()
{
    m_model->startNetCheck(m_domainText);
}

// 完成
void NetCheckWidget::finishNetCheck()
{
    m_model->setNetCheckStatus(false);
    m_netSettingwidget->setVisible(true);
    m_netCheckwidget->setVisible(false);
}

NetCheckMainWindow::NetCheckMainWindow(QWidget *pParent)
    : DefSecurityToolsMnd(pParent)
    , m_pNetCheckWidget(nullptr)
{
    NetCheckModel *pNetCheckModel = new NetCheckModel(this);
    m_pNetCheckWidget = new NetCheckWidget(pNetCheckModel);
    addContent(m_pNetCheckWidget);
    setFixedSize(960, 640);
}

NetCheckMainWindow::~NetCheckMainWindow()
{
}

void NetCheckMainWindow::closeEvent(QCloseEvent *event)
{
    Q_EMIT sendWindowClose();
    event->accept();
}
