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
#include "vpnaddpage.h"
#include <QDebug>
#include "vpnconfigpage.h"

#define VPNADDPAGE_SIZE  520,272
#define MAINLAYOUT_MARGINS  24, 16, 24, 24
#define MAINLAYOUT_SPACE  24
#define NULL_MARGINS  0,0,0,0
#define NULL_SPACE  0
#define VPNADDPAGE_NAME_MAX_LENGTH 32
#define VPNADDPAGE_LABLE_FIXEDWIDTH 112
#define VPNADDPAGE_COMBOBOX_FIXEDWIDTH 360
#define VPNADDPAGE_INPUTBOX_FIXEDWIDTH 360


vpnAddPage::vpnAddPage(QWidget *parent) : QWidget(parent)
{
    m_vpnConnOperation = new KyVpnConnectOperation(this);
    initWindow();
    initUI();
    initConnection();
    centerToScreen();
}

void vpnAddPage::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    this->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

void vpnAddPage::initWindow()
{
    this->setFixedSize(VPNADDPAGE_SIZE);
    this->setWindowTitle(tr("create VPN"));
    this->setWindowFlag(Qt::WindowMinMaxButtonsHint,false);
    this->setAttribute(Qt::WA_DeleteOnClose);
}

void vpnAddPage::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(MAINLAYOUT_MARGINS);
    mainLayout->setSpacing(MAINLAYOUT_SPACE);

    initVpnTypeFrame();
    initVpnNameFrame();
    initVpnServerFrame();
    initButtonFrame();
    onSetConfimBtnEnable();

    mainLayout->addWidget(m_vpnTypeFrame);
    mainLayout->addWidget(m_vpnNameFrame);
    mainLayout->addWidget(m_vpnServerFrame);
    mainLayout->addStretch();
    mainLayout->addWidget(m_buttonFrame);
    this->setLayout(mainLayout);
}

void vpnAddPage::initVpnTypeFrame()
{
    m_vpnTypeFrame = new QFrame(this);
    m_vpnTypeFrame->setFrameShape(QFrame::Shape::NoFrame);

    QHBoxLayout *typeLayout = new QHBoxLayout(m_vpnTypeFrame);
    typeLayout->setContentsMargins(NULL_MARGINS);
    typeLayout->setSpacing(NULL_SPACE);

    QLabel *vpnTypeLabel = new QLabel(tr("VPN Type"), this);
    vpnTypeLabel->setFixedWidth(VPNADDPAGE_LABLE_FIXEDWIDTH);

    m_vpnTypeComboBox = new QComboBox(this);
    m_vpnTypeComboBox->setInsertPolicy(QComboBox::NoInsert);
    m_vpnTypeComboBox->setFixedWidth(VPNADDPAGE_COMBOBOX_FIXEDWIDTH);
    m_vpnTypeComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    m_vpnTypeComboBox->addItem("L2TP", L2TP);
    m_vpnTypeComboBox->addItem("open VPN", OPEN_VPN);
    m_vpnTypeComboBox->addItem("PPTP", PPTP);
    m_vpnTypeComboBox->addItem("strong-swan", STRONG_SWAN);


    typeLayout->addWidget(vpnTypeLabel);
    typeLayout->addWidget(m_vpnTypeComboBox);
    m_vpnTypeFrame->setLayout(typeLayout);
}

void vpnAddPage::initVpnNameFrame()
{
    m_vpnNameFrame = new QFrame(this);
    m_vpnNameFrame->setFrameShape(QFrame::Shape::NoFrame);

    QHBoxLayout *nameLayout = new QHBoxLayout(m_vpnNameFrame);
    nameLayout->setContentsMargins(NULL_MARGINS);
    nameLayout->setSpacing(NULL_SPACE);

    QLabel *vpnNameLabel = new QLabel(tr("VPN Name"), this);
    vpnNameLabel->setFixedWidth(VPNADDPAGE_LABLE_FIXEDWIDTH);

    m_vpnNameLineEdit = new QLineEdit(this);
    m_vpnNameLineEdit->setFixedWidth(VPNADDPAGE_INPUTBOX_FIXEDWIDTH);
    m_vpnNameLineEdit->setMaxLength(VPNADDPAGE_NAME_MAX_LENGTH);
    m_vpnNameLineEdit->setPlaceholderText(tr("Required")); //必填

    nameLayout->addWidget(vpnNameLabel);
    nameLayout->addWidget(m_vpnNameLineEdit);
    m_vpnNameFrame->setLayout(nameLayout);
}

void vpnAddPage::initVpnServerFrame()
{
    m_vpnServerFrame = new QFrame(this);
    m_vpnServerFrame->setFrameShape(QFrame::Shape::NoFrame);

    QHBoxLayout *serverLayout = new QHBoxLayout(m_vpnServerFrame);
    serverLayout->setContentsMargins(NULL_MARGINS);
    serverLayout->setSpacing(NULL_SPACE);

    QLabel *vpnServerLabel = new QLabel(tr("VPN Server"), this);
    vpnServerLabel->setFixedWidth(VPNADDPAGE_LABLE_FIXEDWIDTH);

    m_vpnServerLineEdit = new QLineEdit(this);
    m_vpnServerLineEdit->setFixedWidth(VPNADDPAGE_INPUTBOX_FIXEDWIDTH);
    m_vpnServerLineEdit->setPlaceholderText(tr("Required")); //必填

    serverLayout->addWidget(vpnServerLabel);
    serverLayout->addWidget(m_vpnServerLineEdit);
    m_vpnServerFrame->setLayout(serverLayout);

    // IP的正则格式限制
    QRegExp rxIp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    QRegExpValidator *ipExpVal = new QRegExpValidator(rxIp, this);
    m_vpnServerLineEdit->setValidator(ipExpVal);
}

void vpnAddPage::initButtonFrame()
{
    m_buttonFrame = new QFrame(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout(m_buttonFrame);
    buttonLayout->setContentsMargins(NULL_MARGINS);
    buttonLayout->setSpacing(NULL_SPACE);

    m_autoConnectBox = new QCheckBox(this);
    m_autoConnectBox->setText(tr("Auto Connection"));
    m_confimBtn = new QPushButton(this);
    m_confimBtn->setText(tr("Confirm"));
    m_cancelBtn = new QPushButton(this);
    m_cancelBtn->setText(tr("Cancel"));

    buttonLayout->addWidget(m_autoConnectBox);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addSpacing(16);
    buttonLayout->addWidget(m_confimBtn);
    m_buttonFrame->setLayout(buttonLayout);
}

void vpnAddPage::initConnection()
{
    if (m_vpnNameLineEdit != nullptr) {
        connect(m_vpnNameLineEdit, &QLineEdit::textChanged, this, &vpnAddPage::onSetConfimBtnEnable);
    }

    if (m_vpnServerLineEdit != nullptr) {
        connect(m_vpnServerLineEdit, &QLineEdit::textChanged, this, &vpnAddPage::onSetConfimBtnEnable);
    }

    if (m_cancelBtn != nullptr) {
        connect(m_cancelBtn, &QPushButton::clicked, [&] () {this->close();});
    }

    if (m_confimBtn != nullptr) {
        connect(m_confimBtn, &QPushButton::clicked, this, &vpnAddPage::onConfimBtnClicked);
    }
}

bool vpnAddPage::checkConfimBtnIsEnabled()
{
    if (m_vpnNameLineEdit->text().isEmpty()) {
        qDebug() << "ipv4address empty or invalid";
        return false;
    }

    if (m_vpnServerLineEdit->text().isEmpty()) {
        qDebug() << "ipv4 netMask empty or invalid";
        return false;
    }
    return true;
}

void vpnAddPage::onSetConfimBtnEnable()
{
    m_confimBtn->setEnabled(checkConfimBtnIsEnabled());
}

void vpnAddPage::onConfimBtnClicked()
{
    //新建有线连接
    qDebug() << "Confirm create vpn connect";
    if (!createVpnConnect()) {
//        setNetdetailSomeEnable(true);
//        return;
    }
    close();
}

bool vpnAddPage::createVpnConnect()
{
    KyVpnConfig vpnSetting;
    QString connectName = m_vpnNameLineEdit->text();
    vpnSetting.m_gateway = m_vpnServerLineEdit->text();
    vpnSetting.setConnectName(connectName);
    vpnSetting.m_vpnName = connectName;
    vpnSetting.m_isAutoConnect = m_autoConnectBox->isChecked();
    vpnSetting.m_vpnType = (KyVpnType)m_vpnTypeComboBox->currentData().toInt();

    m_vpnConnOperation->createVpnConnect(vpnSetting);
    return true;
}

vpnAddPage::~vpnAddPage()
{
    Q_EMIT this->closed();
}

void vpnAddPage::paintEvent(QPaintEvent *event)
{
    QPalette pal = qApp->palette();
    QPainter painter(this);
    painter.setBrush(pal.color(QPalette::Base));
    painter.drawRect(this->rect());
    painter.fillRect(rect(), QBrush(pal.color(QPalette::Base)));

    return QWidget::paintEvent(event);
}
