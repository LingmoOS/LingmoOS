/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#include "vpndetail.h"

#define  WINDOW_WIDTH  520
#define  WINDOW_HEIGHT 562
#define  NO_LAYOUT_MARGINS 0,0,0,0
#define  HLAYOUT_MARGINS 24,0,24,0
#define  CENTER_LAYOUT_MARGINS 24,0,0,0
#define  NO_SPACE 0
#define  HLAYOUT_SPACING 16
#define  VLAYOUT_SPACING 24
#define  BOTTOM_WIDGET_HEIGHT 85
#define  TAB_HEIGHT 36
#define  TAB_WIDTH  80
#define  MAX_TAB_TEXT_LENGTH 44
#define  VPNTAB_WIDTH 240
#define  PAGE_WIDTH 472
#define  SCRO_WIDTH 496
#define  CONFIG_PAGE_NUM 0
#define  IPV4_PAGE_NUM 1
#define  IPV6_PAGE_NUM 2
#define  ADVANCED_PAGE_NUM 3
#define  NORMAL_PAGE_COUNT 3
#define  LOG_FLAG  "[VPN Detail]"
#define  TAB_HEIGHT_TABLET 48

VpnDetail::VpnDetail(QString vpnUuid, QString vpnName, QWidget *parent) :
    m_vpnName(vpnName),
    m_uuid(vpnUuid),
    QWidget(parent)
{
    initWindow();
    centerToScreen();
    getVpnConfig(m_uuid, m_vpnInfo);

    initUI();
    initConnection();
    pagePadding(m_isOpenVpn);

    m_isConfigOk = true;
    m_isIpv4Ok = true;
    m_isIpv6Ok = true;
    m_isAdvancedOk = true;
    setConfirmEnable();
}

void VpnDetail::paintEvent(QPaintEvent *event)
{
    QPalette pal = qApp->palette();
    QPainter painter(this);
    painter.setBrush(pal.color(QPalette::Base));
    painter.drawRect(this->rect());
    painter.fillRect(rect(), QBrush(pal.color(QPalette::Base)));

    return QWidget::paintEvent(event);
}


void VpnDetail::initWindow()
{
    this->setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    this->setWindowTitle(tr("VPN"));
    this->setAttribute(Qt::WA_DeleteOnClose);
}

void VpnDetail::initTabBar()
{
    //文本长度超出显示区域设置tooltip
    int tabCount = m_vpnTabBar->count();
    for (int i = 0; i< tabCount; ++i) {
        QFontMetrics fontMetrics(m_vpnTabBar->font());
        int fontSize = fontMetrics.width(m_vpnTabBar->tabText(i));
        if (fontSize > MAX_TAB_TEXT_LENGTH) {
            m_vpnTabBar->setTabToolTip(i, m_vpnTabBar->tabText(i));
        } else {
            m_vpnTabBar->setTabToolTip(i, "");
        }
    }
}

void VpnDetail::initUI()
{
    //控件
    m_topWidget = new QWidget(this);
    m_centerWidget = new QWidget(this);
    m_bottomWidget = new QWidget(this);
    m_divider = new Divider(false, this);
    m_vpnTabBar = new VpnTabBar(this);
    m_autoConnectBox = new QCheckBox(this);
    m_autoConnectLabel = new QLabel(this);
    m_cancelBtn = new QPushButton(this);
    m_confimBtn = new QPushButton(this);
    m_stackWidget = new QStackedWidget(m_centerWidget);

    m_configPage = new VpnConfigPage(this);
    m_ipv4Page = new VpnIpv4Page(this);
    m_ipv6Page = new VpnIpv6Page(this);
    m_advancedPage = new VpnAdvancedPage(this);

    m_configScroArea = new QScrollArea(m_centerWidget);
    m_configScroArea->setFrameShape(QFrame::NoFrame);
    m_configScroArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_configScroArea->setWidget(m_configPage);
    m_configScroArea->setWidgetResizable(true);

    m_ipv4ScroArea = new QScrollArea(m_centerWidget);
    m_ipv4ScroArea->setFrameShape(QFrame::NoFrame);
    m_ipv4ScroArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ipv4ScroArea->setWidget(m_ipv4Page);
    m_ipv4ScroArea->setWidgetResizable(true);

    m_ipv6ScroArea = new QScrollArea(m_centerWidget);
    m_ipv6ScroArea->setFrameShape(QFrame::NoFrame);
    m_ipv6ScroArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ipv6ScroArea->setWidget(m_ipv6Page);
    m_ipv6ScroArea->setWidgetResizable(true);

    m_advancedScroArea = new QScrollArea(m_centerWidget);
    m_advancedScroArea->setFrameShape(QFrame::NoFrame);
    m_advancedScroArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_advancedScroArea->setWidget(m_advancedPage);
    m_advancedScroArea->setWidgetResizable(true);

    QPalette pal = m_configScroArea->palette();
    pal.setBrush(QPalette::Window, Qt::transparent);
    m_configScroArea->setPalette(pal);
    m_ipv4ScroArea->setPalette(pal);
    m_ipv6ScroArea->setPalette(pal);
    m_advancedScroArea->setPalette(pal);

    m_configPage->setFixedWidth(PAGE_WIDTH);
    m_ipv4Page->setFixedWidth(PAGE_WIDTH);
    m_ipv6Page->setFixedWidth(PAGE_WIDTH);
    m_advancedPage->setFixedWidth(PAGE_WIDTH);
    m_configScroArea->setFixedWidth(SCRO_WIDTH);
    m_ipv4ScroArea->setFixedWidth(SCRO_WIDTH);
    m_ipv6ScroArea->setFixedWidth(SCRO_WIDTH);
    m_advancedScroArea->setFixedWidth(SCRO_WIDTH);

    m_stackWidget->addWidget(m_configScroArea);
    m_stackWidget->addWidget(m_ipv4ScroArea);
    m_stackWidget->addWidget(m_ipv6ScroArea);
    m_stackWidget->addWidget(m_advancedScroArea);

    //控件显示文本
    m_vpnTabBar->addTab(tr("VPN"));
    m_vpnTabBar->addTab(tr("IPv4"));
    if (m_isOpenVpn) {
        m_vpnTabBar->addTab(tr("IPv6"));
        m_vpnTabBar->setFixedWidth(VPNTAB_WIDTH + TAB_WIDTH);
    } else {
        m_vpnTabBar->setFixedWidth(VPNTAB_WIDTH);
    }
    m_vpnTabBar->addTab(tr("Advanced"));
    m_vpnTabBar->setFixedHeight(TAB_HEIGHT);
    initTabBar();

    m_autoConnectBox->setChecked(false);
    m_autoConnectLabel->setText(tr("Auto Connection"));
    m_cancelBtn->setText(tr("Cancel"));
    m_confimBtn->setText(tr("Confirm"));

    //布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    mainLayout->setSpacing(NO_SPACE);
    mainLayout->addSpacing(HLAYOUT_SPACING);
    mainLayout->addWidget(m_topWidget);
    mainLayout->addSpacing(VLAYOUT_SPACING);
    mainLayout->addWidget(m_centerWidget);
    mainLayout->addStretch();
    mainLayout->addWidget(m_divider);
    mainLayout->addWidget(m_bottomWidget);
    m_bottomWidget->setFixedHeight(BOTTOM_WIDGET_HEIGHT);

    QHBoxLayout *topLayout = new QHBoxLayout(m_topWidget);
    topLayout->setContentsMargins(HLAYOUT_MARGINS);
    topLayout->addWidget(m_vpnTabBar, Qt::AlignCenter);

    QVBoxLayout *centerlayout = new QVBoxLayout(m_centerWidget);
    centerlayout->setContentsMargins(CENTER_LAYOUT_MARGINS); // 右边距为0，为安全页滚动区域留出空间
    centerlayout->addWidget(m_stackWidget);

    QHBoxLayout *bottomLayout = new QHBoxLayout(m_bottomWidget);
    bottomLayout->setContentsMargins(HLAYOUT_MARGINS);
    bottomLayout->setSpacing(NO_SPACE);
    bottomLayout->addWidget(m_autoConnectBox);
    bottomLayout->addSpacing(8);
    bottomLayout->addWidget(m_autoConnectLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_cancelBtn);
    bottomLayout->addSpacing(HLAYOUT_SPACING);
    bottomLayout->addWidget(m_confimBtn);

    m_advancedPage->setVpnAdvancedPage(m_configPage->getVpnType());
}

void VpnDetail::initConnection()
{
    connect(m_vpnTabBar, &VpnTabBar::currentChanged, this, &VpnDetail::onTabCurrentRowChange);
    connect(m_cancelBtn, &QPushButton::clicked, [&] () {this->close();});
    connect(m_confimBtn, &QPushButton::clicked, this, &VpnDetail::onConfimBtnClicked);

    connect(m_configPage, &VpnConfigPage::setConfigPageState, this, [=] (bool state) {
        m_isConfigOk = state;
        setConfirmEnable();
    });
    connect(m_ipv4Page, &VpnIpv4Page::setIpv4PageState, this, [=] (bool state) {
        m_isIpv4Ok = state;
        setConfirmEnable();
    });
    connect(m_ipv6Page, &VpnIpv6Page::setIpv6PageState, this, [=] (bool state) {
        m_isIpv6Ok = state;
        setConfirmEnable();
    });
    connect(m_advancedPage, &VpnAdvancedPage::setAdvancedPageState, this, [=] (bool state) {
        m_isAdvancedOk = state;
        setConfirmEnable();
    });

    connect(m_configPage, &VpnConfigPage::vpnTypeChanged, m_advancedPage, &VpnAdvancedPage::setVpnAdvancedPage);

    connect(m_configPage, &VpnConfigPage::vpnTypeChanged, this, [=](VpnType type) {
        if (type == OPEN_VPN) {
            if (m_vpnTabBar->count() == NORMAL_PAGE_COUNT) {
                m_vpnTabBar->insertTab(IPV6_PAGE_NUM, tr("IPv6"));
                m_vpnTabBar->setFixedWidth(VPNTAB_WIDTH + TAB_WIDTH);
            }
        } else {
            if (m_vpnTabBar->count() > NORMAL_PAGE_COUNT) {
                m_vpnTabBar->removeTab(IPV6_PAGE_NUM);
                m_vpnTabBar->setFixedWidth(VPNTAB_WIDTH);
            }
        }
    });
}

void VpnDetail::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    this->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

void VpnDetail::setConfirmEnable()
{
    bool isConfirmBtnEnable = false;
    if (m_isConfigOk && m_isIpv4Ok && m_isAdvancedOk) {
        if (m_isOpenVpn && !m_isIpv6Ok) {
            isConfirmBtnEnable = false;
        } else {
            isConfirmBtnEnable = true;
        }
    }
    m_confimBtn->setEnabled(isConfirmBtnEnable);
}

void VpnDetail::getVpnConfig(QString connectUuid, KyVpnConfig &vpnInfo)
{
    KyVpnConnectOperation vpnConnect(this);
    vpnInfo = vpnConnect.getVpnConfig(connectUuid);
    m_isOpenVpn = m_vpnInfo.m_vpnType == KYVPNTYPE_OPENVPN;
}

void VpnDetail::pagePadding(bool isOpenVpn)
{
    m_autoConnectBox->setChecked(m_vpnInfo.m_isAutoConnect);

    //配置页面填充
    m_configPage->setVpnConfigInfo(m_vpnInfo);

    //IPv4页面填充
    m_ipv4Page->setVpnIpv4Info(m_vpnInfo);
    if (!m_vpnInfo.m_ipv4Dns.isEmpty()) {
        m_ipv4Page->setDns(m_vpnInfo.m_ipv4Dns.at(0).toString());
    }
    if (!m_vpnInfo.m_ipv4DnsSearch.isEmpty()) {
        m_ipv4Page->setSearchDomain(m_vpnInfo.m_ipv4DnsSearch.at(0));
    }
    m_ipv4Page->setDhcpClientId(m_vpnInfo.m_ipv4DhcpClientId);
    //IPv6页面填充
    if (isOpenVpn) {
        m_ipv6Page->setVpnIpv6Info(m_vpnInfo);
        if (!m_vpnInfo.m_ipv6Dns.isEmpty()) {
            m_ipv6Page->setDns(m_vpnInfo.m_ipv6Dns.at(0).toString());
        }
        if (!m_vpnInfo.m_ipv6DnsSearch.isEmpty()) {
            m_ipv6Page->setSearchDomain(m_vpnInfo.m_ipv6DnsSearch.at(0));
        }
    }

    //高级页面填充
    if (m_vpnInfo.m_vpnType == KYVPNTYPE_L2TP || m_vpnInfo.m_vpnType == KYVPNTYPE_PPTP) {
        m_advancedPage->setL2tpOrPptpAdiaInfo(m_vpnInfo);
    } else if (m_vpnInfo.m_vpnType == KYVPNTYPE_OPENVPN) {
        m_advancedPage->setOpenVpnAdiaInfo(m_vpnInfo);
    } else if (m_vpnInfo.m_vpnType == KYVPNTYPE_STRONGSWAN) {
        m_advancedPage->setStrongSwanAdiaInfo(m_vpnInfo);
    }
}

bool VpnDetail::updateVpnCnofig()
{
    KyVpnConnectOperation vpnConnect(this);
    KyVpnConfig vpnConfig = m_vpnInfo;
    bool needUpdate = false;
    VpnType vpnType = m_configPage->getVpnType();

    if (m_vpnInfo.m_isAutoConnect != m_autoConnectBox->isChecked()) {
        vpnConfig.m_isAutoConnect = m_autoConnectBox->isChecked();
        needUpdate = true;
    }

    if (m_configPage->checkIsChanged(m_vpnInfo)) {
        m_configPage->updateVpnConfigInfo(vpnConfig);
        needUpdate = true;
    }

    if (m_ipv4Page->checkIsChanged(m_vpnInfo)) {
        m_ipv4Page->updateVpnIpv4Info(vpnConfig);
        needUpdate = true;
    }

    if (vpnType == OPEN_VPN) {
        if (m_ipv6Page->checkIsChanged(m_vpnInfo)) {
            m_ipv6Page->updateVpnIpv6Info(vpnConfig);
            needUpdate = true;
        }
    }

    if (m_advancedPage->checkIsChanged(m_vpnInfo, vpnType)) {
        if (vpnType == L2TP || vpnType == PPTP) {
            m_advancedPage->updateL2tpOrPptpAdiaInfo(vpnConfig);
            needUpdate = true;
        } else if (vpnType == OPEN_VPN) {
            m_advancedPage->updateOpenVpnAdiaInfo(vpnConfig);
            needUpdate = true;
        } else if (vpnType == STRONG_SWAN) {
            m_advancedPage->updateStrongSwanAdiaInfo(vpnConfig);
            needUpdate = true;
        }
    }

    if (needUpdate) {
        vpnConnect.setVpnConfig(m_uuid, vpnConfig);
    }

    return true;
}

void VpnDetail::setVpndetailSomeEnable(bool on)
{
    m_configPage->setEnabled(on);
    m_ipv4Page->setEnabled(on);
    m_ipv6Page->setEnabled(on);
    m_advancedPage->setEnabled(on);
    m_cancelBtn->setEnabled(on);
    m_confimBtn->setEnabled(on);
}

void VpnDetail::onTabCurrentRowChange(int row)
{
    if (row < 2) {
        m_stackWidget->setCurrentIndex(row);
    } else {
        if (m_configPage->getVpnType() == OPEN_VPN) {
            m_stackWidget->setCurrentIndex(row);
        } else {
            m_stackWidget->setCurrentIndex(ADVANCED_PAGE_NUM);
        }
    }
}

void VpnDetail::onConfimBtnClicked()
{
    qDebug() << "onConfimBtnClicked";
    setVpndetailSomeEnable(false);
    //更新连接
    qDebug() << "Confirm update connect";
    if (!updateVpnCnofig()) {
        setVpndetailSomeEnable(true);
        return;
    }
    close();
}


VpnTabBar::VpnTabBar(QWidget *parent)
    :KTabBar(KTabBarStyle::SegmentDark, parent)
{
    //模式切换
    QDBusConnection::sessionBus().connect(QString("com.lingmo.statusmanager.interface"),
                                         QString("/"),
                                         QString("com.lingmo.statusmanager.interface"),
                                          QString("mode_change_signal"), this, SLOT(onModeChanged(bool)));
    //模式获取
    QDBusInterface interface(QString("com.lingmo.statusmanager.interface"),
                             QString("/"),
                             QString("com.lingmo.statusmanager.interface"),
                             QDBusConnection::sessionBus());
    if(!interface.isValid()) {
        this->setFixedHeight(TAB_HEIGHT);
        return;
    }
    QDBusReply<bool> reply = interface.call("get_current_tabletmode");
    if (!reply.isValid()) {
        this->setFixedHeight(TAB_HEIGHT);
        return;
    }
    onModeChanged(reply.value());
}

QSize VpnTabBar::sizeHint() const
{
    QSize size = KTabBar::sizeHint();
    size.setWidth(TAB_WIDTH);
    return size;
}

QSize VpnTabBar::minimumTabSizeHint(int index) const
{
    Q_UNUSED(index)
    QSize size = KTabBar::minimumTabSizeHint(index);
    size.setWidth(TAB_WIDTH);
    return size;
}

void VpnTabBar::onModeChanged(bool mode)
{
    if (mode) {
        this->setFixedHeight(TAB_HEIGHT_TABLET); // 平板模式
    } else {
        this->setFixedHeight(TAB_HEIGHT); // PC模式
    }

}
