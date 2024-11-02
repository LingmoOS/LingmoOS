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
#include "wlanlistitem.h"
#include <QResizeEvent>
#include <kwindowsystem.h>
#include <kwindowsystem_export.h>

#define EMPTY_SSID "EMPTY_SSID"
#define LOG_FLAG "[WlanListItem]"
#define WAIT_US  10*1000
#define NAMELABLE_MAX_WIDTH_HOVER 176
#define NAMELABLE_MAX_WIDTH_ACTIVATED 142
#define NAMELABLE_MAX_WIDTH_DEACTIVATED 276

const QString ENTERPRICE_TYPE = "802.1X";
const QString WPA1_AND_WPA2 = "WPA";
const QString WPA3 = "WPA3";

WlanListItem::WlanListItem(KyWirelessNetItem &wirelessNetItem, QString device, bool isApMode, bool isShowWifi6Plus, QWidget *parent)
    : WlanListItem(wirelessNetItem, device, parent)
{
    m_isApMode = isApMode;
    m_isShowWifi6Plus = isShowWifi6Plus;
    refreshIcon(false); // 额外刷新一次图标，因为WlanListItem执行时，m_isApMode尚未赋值
}

WlanListItem::WlanListItem(KyWirelessNetItem &wirelessNetItem, QString device, bool isShowWifi6Plus, QWidget *parent) : ListItem(parent)
{
    m_wlanDevice = device;
    m_wirelessNetItem = wirelessNetItem;
    m_isShowWifi6Plus = isShowWifi6Plus;

    qDebug()<<"[WlanPage] wlan list item is created." << m_wirelessNetItem.m_NetSsid;

    initWlanUI();
    setExpanded(false);

//    connect(this->m_infoButton, &InfoButton::clicked, this, &WlanListItem::onInfoButtonClicked);
    connect(m_hoverButton, &FixPushButton::clicked, this, &WlanListItem::onNetButtonClicked);

    m_wirelessConnectOperation = new KyWirelessConnectOperation(this);
    m_deviceResource = new KyNetworkDeviceResourse(this);
}

WlanListItem::WlanListItem(QWidget *parent) : ListItem(parent)
{
    m_wirelessNetItem.m_NetSsid = EMPTY_SSID;

    qDebug()<<"[WlanPage] wlan list item is created." << m_wirelessNetItem.m_NetSsid;

    m_netButton->setButtonIcon(QIcon::fromTheme("network-wireless-offline-symbolic"));
    m_netButton->setActive(false);
//    m_netButton->setDefaultPixmap();
    const QString name = tr("Not connected");
    setExpanded(false);
    this->setName(name);
//    this->m_netButton->setEnabled(false);
//    this->m_infoButton->hide();
}

WlanListItem::~WlanListItem()
{
    qDebug()<<"[WlanPage] wlan list item is deleted." << m_wirelessNetItem.m_NetSsid;
}

QString WlanListItem::getSsid()
{
    return m_wirelessNetItem.m_NetSsid;
}

QString WlanListItem::getUuid()
{
    return m_wirelessNetItem.m_connectUuid;
}

QString WlanListItem::getPath()
{
    return m_wirelessNetItem.m_connDbusPath;
}

void WlanListItem::setSignalStrength(const int &signal)
{
    m_wirelessNetItem.m_signalStrength = signal;
    if (Activated == m_connectState) {
        refreshIcon(true);
    } else if (Deactivated == m_connectState) {
        refreshIcon(false);
    }

    return;
}

int WlanListItem::getSignalStrength()
{
    return m_wirelessNetItem.m_signalStrength;
}

bool WlanListItem::isConfigured()
{
    return m_wirelessNetItem.m_isConfigured;
}

void WlanListItem::setWlanState(const int &state)
{
//    m_wirelessNetItem.m_state = state; //ZJP_TODO 后端接口待补全
    refreshIcon(false);
}

void WlanListItem::setExpanded(const bool &expanded)
{
    if (!m_pwdFrame || !m_autoConnectFrame) {
        this->setFixedHeight(NORMAL_HEIGHT);
        return;
    }

    m_pwdFrame->setVisible(expanded);
    m_autoConnectFrame->setVisible(expanded);

    if (expanded) {
        m_pwdLineEdit->setFocus();
        setFixedHeight(EXPANDED_HEIGHT);
        m_hoverButton->hide();
        m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_DEACTIVATED);
    } else {
        setFixedHeight(NORMAL_HEIGHT);
    }

    Q_EMIT this->itemHeightChanged(expanded, m_wirelessNetItem.m_NetSsid);

    return;
}

void WlanListItem::resizeEvent(QResizeEvent *event)
{
    this->blockSignals(true);

    if (this->height() == EXPANDED_HEIGHT) {
        this->setExpanded(true);
    } else {
        this->setExpanded(false);
    }

    this->blockSignals(false);

    return ListItem::resizeEvent(event);
}

void WlanListItem::onRightButtonClicked()
{
    qDebug()<< LOG_FLAG <<"onRightButtonClicked";

    if (m_hoverButton->isVisible()) {
        m_hoverButton->hide();
    }

    if (!m_menu) {
        return;
    }

    m_menu->clear();

    if (Activated == m_connectState || Activating == m_connectState) {
        m_menu->addAction(new QAction(tr("Disconnect"), this));
    } else if (Deactivated == m_connectState) {
        m_menu->addAction(new QAction(tr("Connect"), this));
        qDebug() << "add connect action";
    } else {
        return;
    }

    if (m_wirelessNetItem.m_isConfigured) {
        m_menu->addAction(new QAction(tr("Property"), this));
        m_menu->addAction(new QAction(tr("Forget"), this));
    }

//    m_menu->move(cursor().pos());
    m_menu->popup(cursor().pos());

    return;
}

void WlanListItem::enterEvent(QEvent *event)
{
    //qDebug()<< LOG_FLAG <<"enterEvent" << m_wirelessNetItem.m_NetSsid;
    if (m_pwdFrame != nullptr && !m_pwdFrame->isVisible()) {
        if (Deactivated != m_connectState) {
            m_hoverButton->setProperty("useButtonPalette", true);
            m_hoverButton->setProperty("isImportant", false);
            m_hoverButton->setButtonText(tr("Disconnect"));
        } else {
            m_hoverButton->setProperty("isImportant", true);
            m_hoverButton->setProperty("useButtonPalette", false);
            m_hoverButton->setButtonText(tr("Connect"));
        }
        m_hoverButton->show();
        m_lbLoadUp->hide();
        m_lbLoadDown->hide();
        m_lbLoadDownImg->hide();
        m_lbLoadUpImg->hide();
    }
    m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_HOVER);
    return ListItem::enterEvent(event);
}

void WlanListItem::leaveEvent(QEvent *event)
{
    //qDebug()<< LOG_FLAG <<"leaveEvent"<< m_wirelessNetItem.m_NetSsid;
    m_hoverButton->hide();
    if (m_connectState == Activated || m_connectState == Deactivating) {
        m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_ACTIVATED);
        m_lbLoadUp->show();
        m_lbLoadDown->show();
        m_lbLoadDownImg->show();
        m_lbLoadUpImg->show();
    } else {
        m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_DEACTIVATED);
    }

    return ListItem::leaveEvent(event);
}

bool WlanListItem::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_autoConnectCheckBox) {
        if (event->type() == QEvent::FocusIn) {
            m_pwdLineEdit->setFocus();
        }
    }

    return QFrame::eventFilter(watched, event);
}

void WlanListItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (m_pwdFrame && m_pwdFrame->isVisible() && m_pwdLineEdit->text().length() >= PWD_LENGTH_LIMIT) {
            onConnectButtonClicked();
        }
    }
    return QFrame::keyPressEvent(event);
}

void WlanListItem::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();
    if (m_pwdLineEdit != nullptr) {
        pal.setColor(QPalette::Base, pal.color(QPalette::Base));
        pal.setColor(QPalette::Text, pal.color(QPalette::Text));
        m_pwdLineEdit->setPalette(pal);
    }

    return QWidget::paintEvent(event);
}

void WlanListItem::initWlanUI()
{
    m_hasPwd = (m_wirelessNetItem.m_secuType.isEmpty() || m_wirelessNetItem.m_secuType == "") ? false : true;
    //设置显示的Wlan名称
//    this->setName((m_wirelessNetItem.m_connName != "") ? m_wirelessNetItem.m_connName : m_wirelessNetItem.m_NetSsid);
    this->setName(m_wirelessNetItem.m_NetSsid);
    //刷新左侧按钮图标
    refreshIcon(false);

    this->onPaletteChanged();
#define PWD_AREA_HEIGHT 36
#define CONNECT_BUTTON_WIDTH 96
#define FRAME_CONTENT_MARGINS 56,0,16,4
#define FRAME_SPACING 8
#define LINEEDIT_WIDTH 220
#define PWD_CONTENT_MARGINS 8,0,34,0
#define SHOW_PWD_BUTTON_SIZE 24,24
#define PWD_LAYOUT_MARGINS 8,0,8,0
    //密码输入区域的UI
    m_pwdFrame = new QFrame(this);
    m_pwdFrameLyt = new QHBoxLayout(m_pwdFrame);
    m_pwdFrameLyt->setContentsMargins(FRAME_CONTENT_MARGINS);
    m_pwdFrameLyt->setSpacing(FRAME_SPACING);
    m_pwdFrame->setLayout(m_pwdFrameLyt);

    m_pwdLineEdit  = new KPasswordEdit(m_pwdFrame);
    m_pwdLineEdit->setFixedWidth(LINEEDIT_WIDTH);
    m_pwdLineEdit->setProperty("needTranslucent", true);

    m_pwdLineEdit->setClearButtonEnabled(false); //禁用ClearBtn按钮
    m_pwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, true);   //打开输入法
//    m_pwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
//    m_pwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QRegExp rx("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
    QRegExpValidator *latitude = new QRegExpValidator(rx, this);
    m_pwdLineEdit->setValidator(latitude);

    m_pwdLineEdit->installEventFilter(this);
    connect(m_pwdLineEdit, &QLineEdit::textChanged, this, &WlanListItem::onPwdEditorTextChanged);
    m_pwdLineEdit->setFixedHeight(PWD_AREA_HEIGHT);
    m_pwdFrameLyt->addWidget(m_pwdLineEdit);

    m_connectButton = new FixPushButton(m_pwdFrame);
    m_connectButton->setProperty("isImportant", true);
    m_connectButton->setProperty("needTranslucent", true);
    m_connectButton->setFixedSize(CONNECT_BUTTON_WIDTH, PWD_AREA_HEIGHT);
    m_connectButton->setButtonText(tr("Connect"));
    m_connectButton->setEnabled(false);
    connect(m_connectButton, &QPushButton::clicked, this, &WlanListItem::onConnectButtonClicked);
    m_pwdFrameLyt->addWidget(m_connectButton);
    m_pwdFrameLyt->addStretch();

    //自动连接选择区域UI
    m_autoConnectFrame = new QFrame(this);

    m_autoConnectFrameLyt = new QHBoxLayout(m_autoConnectFrame);
    m_autoConnectFrameLyt->setContentsMargins(FRAME_CONTENT_MARGINS);
    m_autoConnectFrameLyt->setSpacing(FRAME_SPACING);
    m_autoConnectFrame->setLayout(m_autoConnectFrameLyt);

    m_autoConnectCheckBox = new QCheckBox(m_autoConnectFrame);
    m_autoConnectCheckBox->installEventFilter(this);
    m_autoConnectCheckBox->setChecked(true);
    m_autoConnectCheckBox->setFixedSize(SHOW_PWD_BUTTON_SIZE);
    m_autoConnectFrameLyt->addWidget(m_autoConnectCheckBox);

    m_autoConnectLabel = new QLabel(m_autoConnectFrame);
    m_autoConnectLabel->setText(tr("Auto Connect"));
    m_autoConnectFrameLyt->addWidget(m_autoConnectLabel);
    m_autoConnectFrameLyt->addStretch();

    m_mainLayout->addWidget(m_pwdFrame);
    m_mainLayout->addWidget(m_autoConnectFrame);
    m_mainLayout->addStretch();

    m_pwdFrame->hide();
    m_autoConnectFrame->hide();

    this->m_freq->show();
    this->setFrequency();
}

QString getIcon(bool isEncrypted, int signalStrength, int category) {
    QString iconNameFirst,iconNameLast;
    if (category == 0){
        iconNameFirst = "network-wireless-signal-";
    } else if (category == 1) {
        iconNameFirst = "lingmo-wifi6-";
    } else {
        iconNameFirst = "lingmo-wifi6+-";
    }
    if (!isEncrypted) {
        if (signalStrength > EXCELLENT_SIGNAL){
            if (category == 0) {
                iconNameLast = "excellent-symbolic";
            } else {
                iconNameLast = "full-symbolic";
            }
        } else if (signalStrength > GOOD_SIGNAL) {
            if (category == 0) {
                iconNameLast = "good-symbolic";
            } else {
                iconNameLast = "high-symbolic";
            }
        } else if (signalStrength > OK_SIGNAL) {
            if (category == 0) {
                iconNameLast = "ok-symbolic";
            } else {
                iconNameLast = "medium-symbolic";
            }
       } else if (signalStrength > LOW_SIGNAL) {
            if (category == 0) {
                iconNameLast = "weak-symbolic";
            } else {
                iconNameLast = "low-symbolic";
            }
       } else {
            iconNameLast = "none-symbolic";
        }
    } else {
        if (signalStrength > EXCELLENT_SIGNAL){
            if (category == 0) {
                iconNameLast = "excellent-secure-symbolic";
            } else {
                iconNameLast = "full-pwd-symbolic";
            }
        } else if (signalStrength > GOOD_SIGNAL) {
            if (category == 0) {
                iconNameLast = "good-secure-symbolic";
            } else {
                iconNameLast = "high-pwd-symbolic";
            }
        } else if (signalStrength > OK_SIGNAL) {
            if (category == 0) {
                iconNameLast = "ok-secure-symbolic";
            } else {
                iconNameLast = "medium-pwd-symbolic";
            }
       } else if (signalStrength > LOW_SIGNAL) {
            if (category == 0) {
                iconNameLast = "weak-secure-symbolic";
            } else {
                iconNameLast = "low-pwd-symbolic";
            }
       } else {
            if (category == 0) {
                iconNameLast = "none-secure-symbolic";
            } else {
                iconNameLast = "none-pwd-symbolic";
            }
        }
    }
    return iconNameFirst + iconNameLast;
}

void WlanListItem::refreshIcon(bool isActivated)
{
    if (m_isApMode) {
        m_netButton->setButtonIcon(QIcon::fromTheme("network-wireless-hotspot-symbolic", QIcon(":/res/w/wifi-full.png")));
        m_netButton->setActive(isActivated);
        return;
    }

    int category = 0;
    int signalStrength = 0;
    QString uni = "";
    QString secuType = "";
    category = m_wirelessNetItem.getCategory(m_wirelessNetItem.m_uni);
    signalStrength = m_wirelessNetItem.m_signalStrength;

    if (isActivated) {
        if (m_deviceResource->getActiveConnectionInfo(m_wlanDevice, signalStrength, uni, secuType)) {
            category = m_wirelessNetItem.getCategory(uni);
            m_hasPwd = (secuType.isEmpty() || secuType == "") ? false : true;
        }
    }

    if (!m_isShowWifi6Plus && category == 2) {
        category = 1;
    }

    QString iconPath = getIcon(m_hasPwd, signalStrength, category);
    m_netButton->setButtonIcon(QIcon::fromTheme(iconPath));

    m_netButton->setActive(isActivated);
    qDebug() << "refreshIcon" << m_wirelessNetItem.m_NetSsid << "isActivated" << isActivated << "path" << iconPath;
}

void WlanListItem::onInfoButtonClicked()
{
    //ZJP_TODO 呼出无线详情页
//    if(netDetail != nullptr){
//        netDetail->activateWindow();
//        return;
//    }

    qDebug() << LOG_FLAG << "Net active or not:"<< m_connectState;
    qDebug() << LOG_FLAG << "On wlan info button clicked! ssid = "
             << m_wirelessNetItem.m_NetSsid << "; name = "
             << m_wirelessNetItem.m_connName << "." <<Q_FUNC_INFO << __LINE__;

#if 0
    bool isActive = false;
    if (Activated == m_connectState) {
        isActive = true;
    }

    netDetail = new NetDetail(m_wlanDevice, m_wirelessNetItem.m_NetSsid,
                                         m_wirelessNetItem.m_connectUuid, isActive, true,
                                         !m_wirelessNetItem.m_isConfigured);
    connect(netDetail, &NetDetail::destroyed, [&](){
        if (netDetail != nullptr) {
            netDetail = nullptr;
        }
    });
    netDetail->show();
#endif
    Q_EMIT this->detailShow(m_wlanDevice, m_wirelessNetItem.m_NetSsid);
}

void WlanListItem::onNetButtonClicked()
{
    qDebug() << LOG_FLAG << "onNetButtonClicked";
    if (m_wirelessNetItem.m_NetSsid == EMPTY_SSID) {
        return;
    }

    if (Deactivated != m_connectState) {
        m_wirelessConnectOperation->deActivateWirelessConnection(m_wlanDevice, m_wirelessNetItem.m_connectUuid);
        return;
    }

    //获取有配置网络的安全类型
    KyKeyMgmt type = m_wirelessConnectOperation->getConnectKeyMgmt(m_wirelessNetItem.m_connectUuid);
    KySecuType kySecuType = NONE;
    if (type == WpaNone || type == Unknown) {
        kySecuType = NONE;
    } else if (type == WpaPsk) {
        kySecuType = WPA_AND_WPA2_PERSONAL;
    } else if (type == SAE) {
        kySecuType = WPA3_PERSONAL;
    } else if (type == WpaEap) {
        kySecuType = WPA_AND_WPA2_ENTERPRISE;
    } else {
        qDebug() << "KeyMgmt not support now " << type;
    }

    //有配置或者无密码的wifi直接连接
    if (m_wirelessNetItem.m_isConfigured) {
        if (m_wirelessNetItem.m_kySecuType == kySecuType ||
                (m_wirelessNetItem.m_kySecuType == WPA_AND_WPA3 && (kySecuType == WPA_AND_WPA2_PERSONAL || kySecuType == WPA3_PERSONAL))) {
            //安全类型不变直接连接
            m_wirelessConnectOperation->activeWirelessConnect(m_wlanDevice, m_wirelessNetItem.m_connectUuid);
            qDebug()<<"[WlanListItem] Has configuration, will be activated. ssid = "
                   << m_wirelessNetItem.m_NetSsid << Q_FUNC_INFO << __LINE__;
            m_netButton->startLoading();
            return;
        } else {
            //安全类型改变则删除连接
            m_wirelessConnectOperation->deleteWirelessConnect(m_wirelessNetItem.m_connectUuid);
        }
    }

    if (!this->m_connectButton->isVisible() && m_wirelessNetItem.m_secuType != "") {
        if (m_wirelessNetItem.m_secuType.contains("802.1x", Qt::CaseInsensitive)) {
            if (isEnterpriseWlanDialogShow && enterpriseWlanDialog != nullptr) {
                qDebug() <<  LOG_FLAG <<"EnterpriseWlanDialog is show do not show again!";
                KWindowSystem::activateWindow(enterpriseWlanDialog->winId());
                KWindowSystem::raiseWindow(enterpriseWlanDialog->winId());
                return;
            } else {
                enterpriseWlanDialog = new EnterpriseWlanDialog(m_wirelessNetItem, m_wlanDevice);
                connect(enterpriseWlanDialog, &EnterpriseWlanDialog::enterpriseWlanDialogClose, this, &WlanListItem::onEnterpriseWlanDialogClose);
                enterpriseWlanDialog->show();
                isEnterpriseWlanDialogShow = true;
            }
        } else {
            this->setExpanded(true);
        }
    } else {
        onConnectButtonClicked();
    }

    return;
}

void WlanListItem::updateWirelessNetSecurity(QString ssid, QString securityType)
{
    if (ssid != m_wirelessNetItem.m_NetSsid) {
        return;
    }

    qDebug() << LOG_FLAG << "Security changed! ssid = " << m_wirelessNetItem.m_NetSsid
             << "; security = " << m_wirelessNetItem.m_secuType << "change to "<< securityType <<Q_FUNC_INFO << __LINE__;

    m_wirelessNetItem.m_secuType = securityType;
    bool newSecu = (m_wirelessNetItem.m_secuType.isEmpty() || m_wirelessNetItem.m_secuType == "") ? false : true;
    if (m_hasPwd^newSecu) {
        m_hasPwd = newSecu;
        refreshIcon(false);
    }

    m_wirelessNetItem.setKySecuType(securityType);

    return;
}


void WlanListItem::updateWirelessNetItem(KyWirelessNetItem &wirelessNetItem)
{
    m_wirelessNetItem = wirelessNetItem;
    return;
}

void WlanListItem::onPwdEditorTextChanged()
{
    if (!m_pwdLineEdit || !m_connectButton) {
        return;
    }

    if (m_pwdLineEdit->text().length() < PWD_LENGTH_LIMIT) {
        m_connectButton->setEnabled(false);
    } else {
        m_connectButton->setEnabled(true);
    }

    return;
}

void WlanListItem::onConnectButtonClicked()
{
    qDebug()<< LOG_FLAG << "onConnectButtonClicked";
    if ((Activating == m_connectState || Deactivating == m_connectState)) {
        qDebug() << LOG_FLAG << "On wlan  clicked! But there is nothing to do because it is already activating/deactivating!"
                 << Q_FUNC_INFO << __LINE__;
        return;
    }

    if (m_connectButton->isVisible() && !m_connectButton->isEnabled()) {
        qWarning() << "Connect wlan failed because of null pointer or button state!" << Q_FUNC_INFO << __LINE__;
        return;
    }

    KyWirelessConnectSetting settings;
    settings.m_connectName = m_wirelessNetItem.m_NetSsid;
    settings.m_ssid = m_wirelessNetItem.m_NetSsid;
    settings.isAutoConnect = m_autoConnectCheckBox->isChecked();
    settings.m_psk = m_pwdLineEdit->text();  
    if (m_wirelessNetItem.m_secuType.isEmpty() || m_wirelessNetItem.m_secuType == "") {
        settings.m_type = WpaNone;
    } else if (m_wirelessNetItem.m_secuType.contains("WPA1") || m_wirelessNetItem.m_secuType.contains("WPA2")) {
        settings.m_type = WpaPsk;
    } else if (m_wirelessNetItem.m_secuType.contains("WPA3")) {
        settings.m_type = SAE;
    }

    qDebug() << "[WlanListItem] On button connect clicked, will connect wlan. ssid = "
             << m_wirelessNetItem.m_NetSsid << Q_FUNC_INFO <<__LINE__;

    m_wirelessConnectOperation->addAndActiveWirelessConnect(m_wlanDevice, settings, false);
    setExpanded(false);
    m_netButton->startLoading();
    return;
}

ConnectState WlanListItem::getConnectionState()
{
    return m_connectState;
}

void WlanListItem::updateConnectState(ConnectState state)
{
    m_connectState = state;

    if (Activated == state) {
        m_netButton->stopLoading();
        m_netButton->setActive(true);
        m_hoverButton->setProperty("useButtonPalette", true);
        m_hoverButton->setProperty("isImportant", false);
        m_hoverButton->setButtonText(tr("Disconnect"));
        if (m_hoverButton->isHidden()) {
            m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_ACTIVATED);
        }
    } else if(Deactivated == state) {
        qDebug() << "[WlanListItem] stop loading connect state:" << state;
        m_netButton->stopLoading();
        m_netButton->setActive(false);
        m_hoverButton->setProperty("isImportant", true);
        m_hoverButton->setProperty("useButtonPalette", false);
        m_hoverButton->setButtonText(tr("Connect"));
        if (m_hoverButton->isHidden()) {
            m_nameLabel->setLabelMaximumWidth(NAMELABLE_MAX_WIDTH_DEACTIVATED);
        }
    } else {
        qDebug() << "[WlanListItem] start loading connect state:" << state;
        m_netButton->startLoading();
        m_hoverButton->setProperty("useButtonPalette", true);
        m_hoverButton->setProperty("isImportant", false);
        m_hoverButton->setButtonText(tr("Disconnect"));
    }

    return;
}

void WlanListItem::onMenuTriggered(QAction *action)
{
    if (action->text() == tr("Connect")) {
        this->onNetButtonClicked();
    } else if (action->text() == tr("Disconnect")) {
        m_wirelessConnectOperation->deActivateWirelessConnection(m_wlanDevice, m_wirelessNetItem.m_connectUuid);
        qDebug()<<"[WlanListItem] Clicked on connected wifi, it will be inactivated. ssid = "
                        << m_wirelessNetItem.m_NetSsid << Q_FUNC_INFO << __LINE__;
        m_netButton->startLoading();
    } else if (action->text() == tr("Forget")) {
        if (m_pwdLineEdit != nullptr) {
            m_pwdLineEdit->clear();
        }
        m_wirelessConnectOperation->deleteWirelessConnect(m_wirelessNetItem.m_connectUuid);
    } else if (action->text() == tr("Property")) {
        onInfoButtonClicked();
    }

    return;
}

void WlanListItem::onEnterpriseWlanDialogClose(bool isShow)
{
    isEnterpriseWlanDialogShow = isShow;

    return;
}

void WlanListItem::forgetPwd()
{
    if (!this->isConfigured()) {
        m_pwdLineEdit->setText("");
        return;
    }
}

void WlanListItem::setFrequency()
{
    uint freq = m_wirelessNetItem.m_frequency;
    bool isMix = m_wirelessNetItem.m_isMix;

    if (isMix) {
        this->m_freq->setText("2.4/5G");
        return;
    }
    if (freq < FREQ_5GHZ) {
        this->m_freq->setText("2.4G");
    } else {
        this->m_freq->setText("5G");
    }

    return;
}
