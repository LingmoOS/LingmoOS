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
#include "joinhiddenwifipage.h"

#include <QApplication>
#include "kwindowsystem.h"
#include "kwindowsystem_export.h"

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
#define  WINDOW_WIDTH  480
#define  MIN_WINDOW_HEIGHT  368
#define  EAPMIN_WINDOW_HEIGHT  524
#define  TLS_WINDOW_HEIGHT  580
#define  LAYOUT_MARGINS  0, 0, 0, 0
#define  TOP_LAYOUT_MARGINS  24, 12, 24, 16
#define  CENTER_LAYOUT_MARGINS  24, 0, 24, 8
#define  BOTTOM_LAYOUT_MARGINS  24, 24, 24, 24
#define  LAYOUT_SPACING  16
#define  LABEL_MIN_WIDTH  146
#define  LABEL_MAX_WIDTH  434
#define  MAX_NAME_LENGTH 32
#define  SCROAREA_WIDTH  480
#define  MEDIUM_WEIGHT_VALUE  57

JoinHiddenWiFiPage::JoinHiddenWiFiPage(QString devName, KDialog *parent)
    :m_devName(devName),
      KDialog(parent)
{
    m_wirelessConnOpration = new KyWirelessConnectOperation(this);

    initUI();
    initComponent();

    setAttribute(Qt::WA_DeleteOnClose);
    KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);

    setJoinBtnEnable();
}

JoinHiddenWiFiPage::~JoinHiddenWiFiPage()
{

}

void JoinHiddenWiFiPage::closeEvent(QCloseEvent *event)
{
    Q_EMIT this->hiddenWiFiPageClose(m_devName);
    return QWidget::closeEvent(event);
}

void JoinHiddenWiFiPage::initUI()
{
    m_topWidget = new QWidget(this);
    m_centerWidget = new QWidget(this);
    m_bottomWidget = new QWidget(this);
    m_secuWidget = new SecurityPage(false, this);
    m_secuWidget->setSecurity(KySecuType::WPA_AND_WPA2_PERSONAL);

    m_descriptionLabel = new FixLabel(this);
    m_descriptionLabel->setFixedWidth(LABEL_MAX_WIDTH);
    m_nameLabel = new FixLabel(this);
    m_nameLabel->setFixedWidth(LABEL_MIN_WIDTH);
    m_nameEdit =new LineEdit(this);

    m_bottomDivider = new Divider(false, this);
    m_showListBtn = new KBorderlessButton(this);
    m_cancelBtn =new QPushButton(this);
    m_joinBtn =new QPushButton(this);

    m_hiddenWifiScrollArea = new QScrollArea(this);
    m_hiddenWifiScrollArea->setFrameShape(QFrame::NoFrame);
    m_hiddenWifiScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPalette pa = m_hiddenWifiScrollArea->palette();
    pa.setBrush(QPalette::Window, Qt::transparent);
    m_hiddenWifiScrollArea->setPalette(pa);

    m_pageLayout = new QVBoxLayout(this);
    m_pageLayout->setContentsMargins(LAYOUT_MARGINS);
    m_pageLayout->setSpacing(0);
    m_pageLayout->addWidget(m_topWidget);
    m_pageLayout->addWidget(m_hiddenWifiScrollArea);
    m_pageLayout->addWidget(m_bottomDivider);
    m_pageLayout->addWidget(m_bottomWidget);
    this->mainWidget()->setLayout(m_pageLayout);

    m_topLayout = new QHBoxLayout(m_topWidget);
    m_topLayout->setContentsMargins(TOP_LAYOUT_MARGINS);
    m_topLayout->setSpacing(0);
    m_topLayout->addWidget(m_descriptionLabel);
    m_topLayout->addStretch();

    QWidget *ssidWidget = new QWidget(this);
    QHBoxLayout *ssidLayout = new QHBoxLayout(ssidWidget);
    ssidLayout->setContentsMargins(LAYOUT_MARGINS);
    ssidLayout->setSpacing(0);
    m_nameLabel->setMinimumWidth(LABEL_MIN_WIDTH);
    ssidLayout->addWidget(m_nameLabel);
    ssidLayout->addWidget(m_nameEdit);

    m_centerVBoxLayout = new QVBoxLayout(m_centerWidget);
    m_centerVBoxLayout->setContentsMargins(CENTER_LAYOUT_MARGINS);
    m_centerVBoxLayout->setSpacing(0);
    m_centerVBoxLayout->addWidget(ssidWidget);
    m_centerVBoxLayout->addSpacing(LAYOUT_SPACING);
    m_centerVBoxLayout->addWidget(m_secuWidget);

    m_centerWidget->setFixedWidth(SCROAREA_WIDTH);
    m_hiddenWifiScrollArea->setFixedWidth(SCROAREA_WIDTH);
    m_hiddenWifiScrollArea->setWidget(m_centerWidget);
    m_hiddenWifiScrollArea->setWidgetResizable(true);

    //底部按钮
    m_bottomLayout = new QHBoxLayout(m_bottomWidget);
    m_bottomLayout->setContentsMargins(BOTTOM_LAYOUT_MARGINS);
    m_bottomLayout->setSpacing(LAYOUT_SPACING);
    m_bottomLayout->addWidget(m_showListBtn);
    m_bottomLayout->addStretch();
    m_bottomLayout->addWidget(m_cancelBtn);
    m_bottomLayout->addWidget(m_joinBtn);

    //请输入您想要加入的网络信息
   m_descriptionLabel->setLabelText(tr("Please enter the network information"));
   QFont font = m_descriptionLabel->font();
   font.setWeight(MEDIUM_WEIGHT_VALUE);
   m_descriptionLabel->setFont(font);

   m_nameLabel->setLabelText(tr("Network name(SSID)")); //网络名(SSID)
   m_showListBtn->setText(tr("Show Network List")); //显示网络列表
   m_cancelBtn->setText(tr("Cancel"));
   m_joinBtn->setText(tr("Join"));

   QRegExp nameRx("^.{0,32}$");
   QValidator *validator = new QRegExpValidator(nameRx, this);
   m_nameEdit->setValidator(validator);
   m_nameEdit->setPlaceholderText(tr("Required")); //必填

   this->setWindowTitle(tr("Find and Join WLAN"));
   this->setWindowIcon(QIcon::fromTheme("lingmo-network"));
   this->setFixedWidth(WINDOW_WIDTH);
   this->setFixedHeight(MIN_WINDOW_HEIGHT);
//   onPaletteChanged();
}

void JoinHiddenWiFiPage::initComponent()
{
    connect(m_cancelBtn, &QPushButton::clicked, this, [=] {
        close();
    });

    connect(m_joinBtn, SIGNAL(clicked()), this, SLOT(onBtnJoinClicked()));
    connect(m_showListBtn, SIGNAL(clicked()), this, SLOT(onBtnShowListClicked()));

    connect(m_secuWidget, &SecurityPage::secuTypeChanged, this, &JoinHiddenWiFiPage::onSecuTypeChanged);
    connect(m_secuWidget, &SecurityPage::eapTypeChanged, this, &JoinHiddenWiFiPage::onEapTypeChanged);
    connect(m_secuWidget, &SecurityPage::setSecuPageState, this, [ = ](bool status) {
       m_isSecuOk = status;
       setJoinBtnEnable();
    });
    connect(m_nameEdit, &LineEdit::textChanged, this, &JoinHiddenWiFiPage::setJoinBtnEnable);

#if 0
    connect(qApp, &QApplication::paletteChanged, this, &JoinHiddenWiFiPage::onPaletteChanged);

    const QByteArray id(THEME_SCHAME);
    if(QGSettings::isSchemaInstalled(id)){
        QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
        connect(fontSetting, &QGSettings::changed,[=](QString key) {
            if ("themeColor" == key) {
                onPaletteChanged();
            }
        });
    }
#endif
}

void JoinHiddenWiFiPage::setJoinBtnEnable()
{
    if (!m_nameEdit->text().isEmpty() && m_isSecuOk) {
        m_isJoinBtnEnable = true;
    } else {
        m_isJoinBtnEnable = false;
    }
    m_joinBtn->setEnabled(m_isJoinBtnEnable);
}

void JoinHiddenWiFiPage::onBtnJoinClicked()
{
    KyWirelessConnectSetting connSettingInfo;
    //基本信息
    connSettingInfo.m_ssid = m_nameEdit->text();
    connSettingInfo.setConnectName(connSettingInfo.m_ssid);
    connSettingInfo.setIfaceName(m_devName);
    connSettingInfo.m_secretFlag = 0;

    KySecuType secuType;
    KyEapMethodType eapType;
    m_secuWidget->getSecuType(secuType, eapType);

    if (secuType == WPA_AND_WPA2_ENTERPRISE) {
        if (eapType == TLS) {
            m_info.tlsInfo.devIfaceName = m_devName;
            m_secuWidget->updateTlsChange(m_info.tlsInfo);
            m_wirelessConnOpration->addAndActiveWirelessEnterPriseTlsConnect(m_info.tlsInfo, connSettingInfo, m_devName, true);
        } else if (eapType == PEAP) {
            m_secuWidget->updatePeapChange(m_info.peapInfo);
            m_wirelessConnOpration->addAndActiveWirelessEnterPrisePeapConnect(m_info.peapInfo, connSettingInfo, m_devName, true);
        } else if (eapType == TTLS) {
            m_secuWidget->updateTtlsChange(m_info.ttlsInfo);
            m_wirelessConnOpration->addAndActiveWirelessEnterPriseTtlsConnect(m_info.ttlsInfo, connSettingInfo, m_devName, true);
        } else if (eapType == LEAP) {
            m_secuWidget->updateLeapChange(m_info.leapInfo);
            m_wirelessConnOpration->addAndActiveWirelessEnterPriseLeapConnect(m_info.leapInfo, connSettingInfo, m_devName, true);
        } else if (eapType == PWD) {
            m_secuWidget->updatePwdChange(m_info.pwdInfo);
            m_wirelessConnOpration->addAndActiveWirelessEnterPrisePwdConnect(m_info.pwdInfo, connSettingInfo, m_devName, true);
        } else if (eapType == FAST) {
            m_secuWidget->updateFastChange(m_info.fastInfo);
            m_wirelessConnOpration->addAndActiveWirelessEnterPriseFastConnect(m_info.fastInfo, connSettingInfo, m_devName, true);
        }
    } else {
        m_secuWidget->updateSecurityChange(connSettingInfo);
        m_wirelessConnOpration->addAndActiveWirelessConnect(m_devName, connSettingInfo, true);
    }

    close();
}

void JoinHiddenWiFiPage::onBtnShowListClicked()
{
    Q_EMIT showWlanList(1); //WLAN_PAGE_INDEX
}

void JoinHiddenWiFiPage::onSecuTypeChanged(const KySecuType &type)
{
    if (type != KySecuType::WPA_AND_WPA2_ENTERPRISE) {
        this->setFixedHeight(MIN_WINDOW_HEIGHT);
    }
}

void JoinHiddenWiFiPage::onEapTypeChanged(const KyEapMethodType &type)
{
    if (type == KyEapMethodType::TLS || type == KyEapMethodType::FAST) {
        this->setFixedHeight(TLS_WINDOW_HEIGHT);
    } else {
        this->setFixedHeight(EAPMIN_WINDOW_HEIGHT);
    }
}

#if 0
void JoinHiddenWiFiPage::onPaletteChanged()
{
    QPalette pal = qApp->palette();

    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = lightPalette(this);
       }
    }
    this->setPalette(pal);
    setFramePalette(m_secuWidget, pal);
    setFramePalette(m_hiddenWifiScrollArea, pal);

    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}
#endif

void JoinHiddenWiFiPage::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
//    this->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
    kdk::WindowManager::setGeometry(this->windowHandle(), QRect(desk_x / 2 - x / 2 + desk_rect.left(),
                                                                desk_y / 2 - y / 2 + desk_rect.top(),
                                                                this->width(),
                                                                this->height()));
}
