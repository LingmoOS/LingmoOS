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
#include "mobilehotspotwidget.h"
#include <QDebug>
#include <QFormLayout>

#define LABEL_RECT 17, 0, 105, 23
#define CONTENTS_MARGINS 0, 0, 0, 0
#define ITEM_MARGINS 16, 0, 16, 0
#define FRAME_MIN_SIZE 550, 60
#define FRAME_MAX_SIZE 16777215, 16777215
#define CONTECT_FRAME_MAX_SIZE 16777215, 60
#define HINT_TEXT_MARGINS 8, 0, 0, 0
#define FRAME_MIN_SIZE 550, 60
#define LABLE_MIN_WIDTH 188
#define COMBOBOX_MIN_WIDTH 200
#define LINE_MAX_SIZE 16777215, 1
#define LINE_MIN_SIZE 0, 1
#define ICON_SIZE   24,24
#define PASSWORD_FRAME_MIN_HIGHT 60
#define PASSWORD_FRAME_FIX_HIGHT 90
#define PASSWORD_FRAME_MIN_SIZE 550, 60
#define PASSWORD_FRAME_MAX_SIZE 16777215, 90
#define PASSWORD_ITEM_MARGINS 16, 10, 16, 10

#define WIRELESS   1

#define AP_NAME_MAX_LENGTH 32

#define REFRESH_MSEC 20*1000
#define FRAME_SPEED 150

#define LOG_HEAD "[MobileHotspotWidget]"

const QByteArray GSETTINGS_SCHEMA = "org.lingmo.lingmo-nm.switch";
const QString    WIRELESS_SWITCH          = "wirelessswitch";

void MobileHotspotWidget::showDesktopNotify(const QString &message)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<(tr("Settings"))
       <<((unsigned int) 0)
       <<QString("lingmo-control-center")
       <<tr("Settings desktop message") //显示的是什么类型的信息
       <<message //显示的具体信息
       <<QStringList()
       <<QVariantMap()
       <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

#define HOTSPOT_CONTROL

MobileHotspotWidget::MobileHotspotWidget(QWidget *parent) : QWidget(parent)
{
    m_Vlayout = new QVBoxLayout(this);
    m_Vlayout->setContentsMargins(CONTENTS_MARGINS);
    m_Vlayout->setSpacing(0);

    qDBusRegisterMetaType<QMap<QString, bool> >();
    qDBusRegisterMetaType<QMap<QString, int> >();
    qDBusRegisterMetaType<QVector<QStringList> >();
    qDBusRegisterMetaType<QMap<QString, QVector<QStringList> >>();

    initUI();

#ifdef HOTSPOT_CONTROL
    initConnectDevPage();
    initBlackListPage();
#endif

    m_switchBtn->installEventFilter(this);
    m_interface = new QDBusInterface("com.lingmo.network", "/com/lingmo/network",
                                     "com.lingmo.network",
                                     QDBusConnection::sessionBus());
    if(!m_interface->isValid()) {
        qDebug() << LOG_HEAD << "dbus interface com.lingmo.network is invaild";
        m_switchBtn->setChecked(false);
        setUiEnabled(false);
    }

    m_hostName = getHostName();

    initDbusConnect();

    initInterfaceInfo();
    getApInfo();

#ifdef HOTSPOT_CONTROL
    initNmDbus();
#endif

    this->setLayout(m_Vlayout);
    m_Vlayout->addStretch();

    connect(m_switchBtn, &KSwitchButton::stateChanged, this, &MobileHotspotWidget::setUiEnabled);
    connect(m_interfaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MobileHotspotWidget::onInterfaceChanged);
    connect(m_interfaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
        UkccCommon::buriedSettings("MobileHotspot", "Net card", QString("select"), m_interfaceComboBox->currentText());
        m_interfaceName = m_interfaceComboBox->currentText();
        updateBandCombox();
    });
    connect(m_freqBandComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](){
        if (m_isUserSelect && !m_freqBandComboBox->currentText().isEmpty()) {
            UkccCommon::buriedSettings("MobileHotspot", "Frequency band", QString("select"), m_freqBandComboBox->currentText());
        }
    });
    onInterfaceChanged();

#ifdef HOTSPOT_CONTROL
    m_connectDevPage->refreshStalist();
    m_blacklistPage->refreshBlacklist();
#endif
    this->update();
}

MobileHotspotWidget::~MobileHotspotWidget()
{
    deleteActivePathInterface();
    deleteSettingPathInterface();
    delete m_interface;
}

bool MobileHotspotWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick && watched == m_switchBtn) {
        return true;
    }

    if (watched == m_switchBtn) {
        if (event->type() == QEvent::MouseButtonRelease) {
            if (!m_interface->isValid()) {
                return true;
            }
            if (!m_switchBtn->isCheckable()) {
                showDesktopNotify(tr("wirless switch is close or no wireless device"));
                return true;
            }
            UkccCommon::buriedSettings("MobileHotspot", "Open", QString("settings"), !m_switchBtn->isChecked() ? "true":"false");
            if (m_switchBtn->isChecked()) {
//                showDesktopNotify(tr("start to close hotspot"));
                QDBusReply<void> reply = m_interface->call("deactiveWirelessAp", m_apNameLine->text(), m_uuid);
                if (!reply.isValid()) {
                    qDebug() << LOG_HEAD << "call deactiveWirelessAp failed ";
                    return true;
                }
#ifdef HOTSPOT_CONTROL
                deleteActivePathInterface();
                m_connectDevPage->setInterface(nullptr);
                m_connectDevPage->refreshStalist();
                m_blacklistPage->refreshBlacklist();
#endif
                this->update();
            } else {
                if (m_apNameLine->text().isEmpty() || m_interfaceName.isEmpty())
                {
                    showDesktopNotify(tr("hotpots name or device is invalid"));
                    return true;
                }
                if (m_pwdNameLine->text().length() < 8) {

                    return true;
                }
//                showDesktopNotify(tr("start to open hotspot ") + m_apNameLine->text());
                QDBusReply<void> reply = m_interface->call("activeWirelessAp",
                                                           m_apNameLine->text(),
                                                           m_pwdNameLine->text(),
                                                           m_freqBandComboBox->currentText(),
                                                           m_interfaceComboBox->currentText());
                if (!reply.isValid()) {
                    qDebug() << LOG_HEAD << "call activeWirelessAp failed ";
                    return true;
                }

            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MobileHotspotWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MobileHotspotWidget::resetFrameSize()
{
    int height = 0;
    for (int i = 0; i < m_hotspotFrame->layout()->count(); i ++) {
        QWidget *w = m_hotspotFrame->layout()->itemAt(i)->widget();
        if (w != nullptr && !w->isHidden()) {
            height += w->height();
        }
    }
    m_hotspotFrame->setFixedHeight(height);
}

void MobileHotspotWidget::initUI()
{
    m_hotspotFrame = new QFrame(this);
    m_hotspotFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_hotspotFrame->setMaximumSize(FRAME_MAX_SIZE);
    m_hotspotFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *hotspotLyt = new QVBoxLayout(this);
    hotspotLyt->setContentsMargins(0, 0, 0, 0);
    m_hotspotFrame->setLayout(hotspotLyt);

    m_hotspotTitleLabel = new TitleLabel(this);
    m_hotspotTitleLabel->setText(tr("Hotspot"));

    setSwitchFrame();
    setApNameFrame();
    setPasswordFrame();
    setFreqBandFrame();
    setInterFaceFrame();

    switchAndApNameLine = myLine();
    apNameAndPwdLine = myLine();
    pwdAndfreqBandLine = myLine();
    freqBandAndInterfaceLine = myLine();

    hotspotLyt->addWidget(m_switchFrame);
    hotspotLyt->addWidget(switchAndApNameLine);
    hotspotLyt->addWidget(m_ApNameFrame);
    hotspotLyt->addWidget(apNameAndPwdLine);
    hotspotLyt->addWidget(m_passwordFrame);
    hotspotLyt->addWidget(pwdAndfreqBandLine);
    hotspotLyt->addWidget(m_freqBandFrame);
    hotspotLyt->addWidget(freqBandAndInterfaceLine);
    hotspotLyt->addWidget(m_interfaceFrame);
    hotspotLyt->setSpacing(0);

    resetFrameSize();

    m_Vlayout->addWidget(m_hotspotTitleLabel);
    m_Vlayout->addSpacing(8);
    m_Vlayout->addWidget(m_hotspotFrame);

}


void MobileHotspotWidget::initDbusConnect()
{
    if(m_interface->isValid()) {
        connect(m_interface,SIGNAL(activateFailed(QString)), this, SLOT(onActivateFailed(QString)), Qt::QueuedConnection);
        connect(m_interface,SIGNAL(deactivateFailed(QString)), this, SLOT(onDeactivateFailed(QString)), Qt::QueuedConnection);
        connect(m_interface,SIGNAL(wirelessDeviceStatusChanged()), this, SLOT(onDeviceStatusChanged()), Qt::QueuedConnection);
        connect(m_interface,SIGNAL(deviceNameChanged(QString, QString, int)), this, SLOT(onDeviceNameChanged(QString, QString, int)), Qt::QueuedConnection);
        connect(m_interface,SIGNAL(hotspotDeactivated(QString, QString)), this, SLOT(onHotspotDeactivated(QString, QString)), Qt::QueuedConnection);
        connect(m_interface,SIGNAL(hotspotActivated(QString, QString, QString, QString, QString)), this, SLOT(onHotspotActivated(QString, QString, QString, QString, QString)), Qt::QueuedConnection);
        connect(m_interface,SIGNAL(hotspotDeactivating(QString, QString)), this, SLOT(startLoading()), Qt::QueuedConnection);
        connect(m_interface,SIGNAL(hotspotActivating(QString, QString)), this, SLOT(startLoading()), Qt::QueuedConnection);

        connect(m_interface, SIGNAL(wlanactiveConnectionStateChanged(QString, QString, QString, int)), this, SLOT(onActiveConnectionChanged(QString, QString, QString, int)), Qt::QueuedConnection);

        connect(m_interface, SIGNAL(wirelessSwitchBtnChanged(bool)), this, SLOT(onWirelessBtnChanged(bool)), Qt::QueuedConnection);
    }

    connect(m_apNameLine, &QLineEdit::textEdited, this, &MobileHotspotWidget::onApLineEditTextEdit);
#ifdef HOTSPOT_CONTROL
    connect(m_connectDevPage, SIGNAL(setStaIntoBlacklist(QString, QString)), m_blacklistPage, SLOT(onsetStaIntoBlacklist(QString, QString)));
#endif
    connect(m_pwdNameLine, SIGNAL(textChanged(QString)), this, SLOT(onPwdTextChanged()));
}

void MobileHotspotWidget::onApLineEditTextEdit(QString text)
{
    int count = 0;
    int i = 0;
    if (text.toLocal8Bit().length() <= AP_NAME_MAX_LENGTH) {
        return;
    }

    int index = m_apNameLine->cursorPosition();

    QString leftStr = text.left(index);
    QString rightStr = text.mid(index);

    if (rightStr.isEmpty()) {
        for ( ; i < text.length(); ++i) {
            count += text.mid(i,1).toLocal8Bit().length();
            if (count > AP_NAME_MAX_LENGTH) {
                m_apNameLine->setText(text.left(i));
                return;
            }
        }
    } else {
        count = rightStr.toLocal8Bit().length();
        for ( ; i < leftStr.length(); ++i) {
            count += leftStr.mid(i,1).toLocal8Bit().length();
            if (count > AP_NAME_MAX_LENGTH) {
                m_apNameLine->setText(leftStr.left(i) + rightStr);
                m_apNameLine->setCursorPosition(i);
                return;
            }
        }
    }
}

void MobileHotspotWidget::onPwdTextChanged()
{
    if (m_pwdNameLine->text().length() < 8) {
        m_passwordFrame->setFixedHeight(PASSWORD_FRAME_FIX_HIGHT);
        m_pwdHintLabel->show();
    } else {
        m_passwordFrame->setFixedHeight(PASSWORD_FRAME_MIN_HIGHT);
        m_pwdHintLabel->hide();
    }
    resetFrameSize();
    this->update();
}

void MobileHotspotWidget::onInterfaceChanged()
{
    m_interfaceName = m_interfaceComboBox->currentText();
    if(m_interface->isValid()) {
        QDBusReply<QVariantList> reply = m_interface->call(QStringLiteral("getWirelessList"), m_interfaceName);
        if(!reply.isValid())
        {
            qWarning() << "getWirelessList error:" << reply.error().message();
            return;
        }
        bool flag = false;

        QList<QStringList> variantList;
        for (int j = 0; j < reply.value().size(); ++j) {
            variantList << reply.value().at(j).toStringList();
        }

        if (!variantList.isEmpty() && variantList.at(0).size() > 1) {
            flag = true;
        }

        if (flag) {
            m_interfaceWarnLabel->setText(tr("use ") + m_interfaceName +
                                          tr(" share network, will interrupt local wireless connection"));
            m_interfaceFrame->setFixedHeight(PASSWORD_FRAME_FIX_HIGHT);
            m_warnWidget->show();
        } else {
            m_interfaceFrame->setFixedHeight(PASSWORD_FRAME_MIN_HIGHT);
            m_warnWidget->hide();
        }
        resetFrameSize();
    }

    updateBandCombox();
}

void MobileHotspotWidget::onActiveConnectionChanged(QString deviceName, QString ssid, QString uuid, int status)
{
    if(m_uuid == uuid && status == 4) {
        if(m_switchBtn->isChecked())
            showDesktopNotify(tr("hotspot already close"));
        m_switchBtn->setChecked(false);
        setUiEnabled(false);
        m_uuid.clear();
    }

    if (deviceName != m_interfaceName) {
        return;
    }

    if (m_interfaceComboBox && status == 2) {
        onInterfaceChanged();
    }
}

void MobileHotspotWidget::onWirelessBtnChanged(bool state)
{
    stopLoading();
    if (!state) {
        if(m_switchBtn->isChecked())
            showDesktopNotify(tr("hotspot already close"));
        m_switchBtn->setChecked(state);
        m_uuid.clear();
        m_switchBtn->setCheckable(false);
    } else {
        m_switchBtn->setCheckable(true);
    }
}

void MobileHotspotWidget::initInterfaceInfo()
{
    if(!m_interface->isValid()) {
        return;
    }

    if (m_interfaceComboBox->isVisible()) {
        m_interfaceComboBox->hidePopup();
    }
    m_interfaceComboBox->clear();
    QDBusReply<QVariantMap> reply = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),WIRELESS);
    if(!reply.isValid())  {
        qWarning() << "[WlanConnect]getWirelessDeviceList error:" << reply.error().message();
        return;
    }

    QMap<QString, bool> devMap;
    QVariantMap::const_iterator item = reply.value().cbegin();
    while (item != reply.value().cend()) {
        devMap.insert(item.key(), item.value().toBool());
        item ++;
    }


    QDBusReply<QVariantMap> capReply = m_interface->call("getWirelessDeviceCap");
    if (!capReply.isValid()) {
        qDebug()  << LOG_HEAD <<"execute dbus method 'getWirelessDeviceCap' is invalid in func initInterfaceInfo()" <<capReply.error().type() ;
        setWidgetHidden(true);
        return;
    }
    QMap<QString, int> devCapMap;
    QVariantMap::const_iterator itemIter = capReply.value().cbegin();
    while (itemIter != capReply.value().cend()) {
        devCapMap.insert(itemIter.key(), itemIter.value().toInt());
        itemIter ++;
    }

    if (devMap.isEmpty()) {
        qDebug() << LOG_HEAD << "no wireless device";
        setWidgetHidden(true);
        m_switchBtn->setCheckable(false);
    } else {
        QMap<QString, bool>::Iterator iter = devMap.begin();
        while (iter != devMap.end()) {
            if (!iter.value()) {
                iter++;
                continue;
            }
            QString interfaceName = iter.key();
            if (!(devCapMap[interfaceName] & 0x01)) {
                m_interfaceComboBox->addItem(interfaceName);
            }
            iter++;
        }
        if (m_interfaceComboBox->count() > 0) {
            setWidgetHidden(false);
            m_interfaceName = m_interfaceComboBox->currentText();
            if (m_interfaceComboBox->count() == 1) {
                updateBandCombox();
            }
        } else {
            qDebug() << LOG_HEAD << "no useable wireless device";
            setWidgetHidden(true);
        }
    }
}

void MobileHotspotWidget::getApInfo()
{
    if (!m_interface->isValid()) {
        return;
    }

    if (m_interfaceComboBox->count() <= 0) {
        if(m_switchBtn->isChecked())
            showDesktopNotify(tr("hotspot already close"));
        m_switchBtn->setChecked(false);
        setWidgetHidden(true);
        qWarning() << LOG_HEAD << "getApInfo but interface is empty";
        return;
    }


    QDBusReply<QStringList> reply = m_interface->call("getStoredApInfo");
    if (!reply.isValid()) {
        qDebug() << LOG_HEAD <<"execute dbus method 'getStoredApInfo' is invalid in func getObjectPath()";
    }

    QStringList apInfo = reply.value();

    if (apInfo.isEmpty()) {
        qDebug() << LOG_HEAD << "no stored hotspot info";
        m_apNameLine->setText(m_hostName);
        m_pwdNameLine->setText("12345678");
        return;
    } else {
        m_apNameLine->setText(apInfo.at(0));
        m_pwdNameLine->setText(apInfo.at(1));

        int index = m_interfaceComboBox->findText(apInfo.at(2));
        if (index >= 0) {
            m_interfaceComboBox->setCurrentIndex(index);
            m_interfaceName = apInfo.at(2);
        } else {
            qDebug() << LOG_HEAD << "no such interface " << apInfo.at(2);
        }

        if (apInfo.at(3) == "true") {
            if(!m_switchBtn->isChecked())
                showDesktopNotify(tr("hotspot already open"));
            m_switchBtn->setChecked(true);
            setUiEnabled(true);
            m_uuid = apInfo.at(4);
        } else {
            if(m_switchBtn->isChecked())
                showDesktopNotify(tr("hotspot already close"));
            m_switchBtn->setChecked(false);
            setUiEnabled(false);
            m_uuid = apInfo.at(4);
        }
        int i = m_freqBandComboBox->findText(apInfo.at(5));
        if (i >= 0) {
            m_isUserSelect = false;
            m_freqBandComboBox->setCurrentIndex(i);
            m_isUserSelect = true;
        }
    }
}

void MobileHotspotWidget::setSwitchFrame()
{
    /* Open */
    m_switchFrame = new QFrame(this);
    m_switchFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_switchFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_switchFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *switchLayout = new QHBoxLayout(m_switchFrame);

    m_switchLabel = new QLabel(tr("Open mobile hotspot"), this);
    m_switchLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_switchBtn = new KSwitchButton(this);
    m_statusLabel= new QLabel(this);
    m_statusLabel->hide();
    switchLayout->setContentsMargins(ITEM_MARGINS);
    switchLayout->addWidget(m_switchLabel);
    switchLayout->addStretch();
    switchLayout->addWidget(m_statusLabel);
    switchLayout->addSpacing(4);
    switchLayout->addWidget(m_switchBtn);

    m_switchFrame->setLayout(switchLayout);

    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-1-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-2-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-3-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-4-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-5-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-6-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-7-symbolic"));
    m_waitTimer = new QTimer(this);
    connect(m_waitTimer, &QTimer::timeout, this, &MobileHotspotWidget::updateLoadingIcon);
}

void MobileHotspotWidget::updateLoadingIcon()
{
    if (m_currentIconIndex > 6) {
        m_currentIconIndex = 0;
    }
    m_statusLabel->setPixmap(m_loadIcons.at(m_currentIconIndex).pixmap(16,16));
    m_currentIconIndex ++;
}

void MobileHotspotWidget::startLoading()
{
    m_waitTimer->start(FRAME_SPEED);
    m_switchBtn->hide();
    m_statusLabel->setFocus();
    m_statusLabel->show();
}

void MobileHotspotWidget::stopLoading()
{
    m_waitTimer->stop();
    m_statusLabel->clearFocus();
    m_statusLabel->hide();
    m_switchBtn->show();
}

void MobileHotspotWidget::setApNameFrame()
{
    /* ApName */
    m_ApNameFrame = new QFrame(this);
    m_ApNameFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_ApNameFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_ApNameFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *apNameHLayout = new QHBoxLayout(m_ApNameFrame);

    m_apNameLabel = new QLabel(tr("Wi-Fi Name"), this);
    m_apNameLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_apNameLine = new QLineEdit(this);
    m_apNameLine->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_apNameLine->setMaxLength(AP_NAME_MAX_LENGTH);
    apNameHLayout->setContentsMargins(ITEM_MARGINS);
    apNameHLayout->setSpacing(0);
    apNameHLayout->addWidget(m_apNameLabel);
    apNameHLayout->addWidget(m_apNameLine);
    m_ApNameFrame->setLayout(apNameHLayout);

}

void MobileHotspotWidget::setPasswordFrame()
{
    /* Password */
    m_passwordFrame = new QFrame(this);
    m_passwordFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_passwordFrame->setMinimumSize(PASSWORD_FRAME_MIN_SIZE);
    m_passwordFrame->setMaximumSize(PASSWORD_FRAME_MAX_SIZE);

    m_pwdLabel = new FixLabel(tr("Network Password"), this);
    m_pwdLabel->setFixedWidth(LABLE_MIN_WIDTH);
    m_pwdNameLine = new KPasswordEdit(this);
    m_pwdNameLine->setClearButtonEnabled(false);//禁用ClearBtn按钮X
    m_pwdNameLine->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_pwdHintLabel= new QLabel(this);
    m_pwdHintLabel->setFixedHeight(20);
    m_pwdHintLabel->setContentsMargins(HINT_TEXT_MARGINS);

    QPalette hintTextColor;
    hintTextColor.setColor(QPalette::WindowText, Qt::red);
    m_pwdHintLabel->setPalette(hintTextColor);
    m_pwdHintLabel->setText(tr("Contains at least 8 characters")); //至少包含8个字符

    QGridLayout *pwdLayout = new QGridLayout(m_passwordFrame);
    pwdLayout->setContentsMargins(PASSWORD_ITEM_MARGINS);
    pwdLayout->setSpacing(0);
    pwdLayout->addWidget(m_pwdLabel, 0, 0);
    pwdLayout->addWidget(m_pwdNameLine, 0, 1);
    pwdLayout->addWidget(m_pwdHintLabel, 1, 1);

    m_passwordFrame->setLayout(pwdLayout);
    //init
    m_passwordFrame->setFixedHeight(PASSWORD_FRAME_FIX_HIGHT);
    m_pwdHintLabel->show();

    m_pwdNameLine->installEventFilter(this);
}

void MobileHotspotWidget::setFreqBandFrame()
{
    /* frequency band */
    m_freqBandFrame = new QFrame(this);
    m_freqBandFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_freqBandFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_freqBandFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *freqBandHLayout = new QHBoxLayout(m_freqBandFrame);

    m_freqBandLabel = new FixLabel(this);
    m_freqBandLabel->setText(tr("Network Frequency band"));
    m_freqBandLabel->setFixedWidth(LABLE_MIN_WIDTH - 8);
    m_freqBandComboBox = new QComboBox(this);
    m_freqBandComboBox->setInsertPolicy(QComboBox::NoInsert);
    m_freqBandComboBox->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_freqBandComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    m_freqBandComboBox->addItem("2.4GHz");
    m_freqBandComboBox->addItem("5GHz");
    freqBandHLayout->setContentsMargins(ITEM_MARGINS);
    freqBandHLayout->setSpacing(8);
    freqBandHLayout->addWidget(m_freqBandLabel);
    freqBandHLayout->addWidget(m_freqBandComboBox);

    m_freqBandFrame->setLayout(freqBandHLayout);
}

void MobileHotspotWidget::setInterFaceFrame()
{
    /* key tips */
    m_interfaceFrame = new QFrame(this);
    m_interfaceFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_interfaceFrame->setMinimumSize(PASSWORD_FRAME_MIN_SIZE);
    m_interfaceFrame->setMaximumSize(PASSWORD_FRAME_MAX_SIZE);

    m_interfaceLabel = new FixLabel(tr("Shared NIC port"), this);
    m_interfaceLabel->setFixedWidth(LABLE_MIN_WIDTH);
    m_interfaceComboBox = new QComboBox(this);
    m_interfaceComboBox->setInsertPolicy(QComboBox::NoInsert);
    m_interfaceComboBox->setMinimumWidth(COMBOBOX_MIN_WIDTH);

    m_warnWidget = new QWidget(this);
    m_warnWidget->setFixedHeight(20);
    m_warnWidget->setContentsMargins(8,0,0,0);

    QHBoxLayout *warnTextHLayout = new QHBoxLayout(m_warnWidget);
    QLabel* warnIcon = new QLabel(this);
    warnIcon->setContentsMargins(0,0,0,0);
    warnIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16,16));

    m_interfaceWarnLabel= new FixLabel(this);
    m_interfaceWarnLabel->setFixedHeight(20);

    QPalette hintTextColor;
    hintTextColor.setColor(QPalette::WindowText, Qt::red);
    m_interfaceWarnLabel->setPalette(hintTextColor);

    warnTextHLayout->setSpacing(8);
    warnTextHLayout->setContentsMargins(0,0,0,0);
    warnTextHLayout->addWidget(warnIcon);
    warnTextHLayout->addWidget(m_interfaceWarnLabel);
    warnIcon->setFixedWidth(16);
    m_warnWidget->setLayout(warnTextHLayout);

    QGridLayout *interfaceFLayout = new QGridLayout(m_interfaceFrame);
    interfaceFLayout->setContentsMargins(PASSWORD_ITEM_MARGINS);
    interfaceFLayout->setSpacing(0);
    interfaceFLayout->addWidget(m_interfaceLabel, 0, 0);
    interfaceFLayout->addWidget(m_interfaceComboBox, 0, 1);
    interfaceFLayout->addWidget(m_warnWidget, 1, 1);

    m_interfaceFrame->setLayout(interfaceFLayout);

    m_warnWidget->hide();
}
void MobileHotspotWidget::onActivateFailed(QString errorMessage)
{
    if (errorMessage.indexOf("hotspot")) {
        //todo
//        showDesktopNotify(errorMessage);
    }
}

void MobileHotspotWidget::onDeactivateFailed(QString errorMessage)
{
    if (errorMessage.indexOf("hotspot")) {
//        showDesktopNotify(errorMessage);
    }
}

//设备插拔
void MobileHotspotWidget::onDeviceStatusChanged()
{
    initInterfaceInfo();
    getApInfo();
}

void MobileHotspotWidget::onDeviceNameChanged(QString oldName, QString newName, int type)
{
    if (WIRELESS != type) {
        return;
    }
    int index = m_interfaceComboBox->findText(oldName);
    if (index >= 0) {
        m_interfaceComboBox->setItemText(index, newName);
        if (m_interfaceName == oldName) {
            m_interfaceName = newName;
        }
    }
    QTimer::singleShot(100, this, [=]() {
        if (m_interfaceComboBox->currentText() == newName) {
            updateBandCombox();
        }
    });
}

//热点断开
void MobileHotspotWidget::onHotspotDeactivated(QString devName, QString ssid)
{
    stopLoading();
    if (!m_switchBtn->isChecked()) {
        return;
    }
    if (devName == m_interfaceComboBox->currentText() && ssid == m_apNameLine->text()) {
        if(m_switchBtn->isChecked())
            showDesktopNotify(tr("hotspot already close"));
        m_switchBtn->setChecked(false);
        m_uuid.clear();
//        setUiEnabled(true);
    }
}

//热点连接
void MobileHotspotWidget::onHotspotActivated(QString devName, QString ssid, QString uuid, QString activePath, QString settingPath)
{
    qDebug() << LOG_HEAD << "onHotspotActivated" <<devName << ssid << uuid;
    stopLoading();
    if (m_switchBtn->isChecked()) {
        return;
    }
#ifdef HOTSPOT_CONTROL
    if (activePath != nullptr) {
        deleteActivePathInterface();
        initActivePathInterface(activePath);
    }
    if (settingPath != nullptr) {
        deleteSettingPathInterface();
        initSettingPathInterface(settingPath);
    }

    m_connectDevPage->refreshStalist();
    m_blacklistPage->refreshBlacklist();
#endif
    this->update();

    if (devName == m_interfaceComboBox->currentText() && ssid == m_apNameLine->text()) {
        if(!m_switchBtn->isChecked())
            showDesktopNotify(tr("hotspot already open"));
        m_switchBtn->setChecked(true);
        m_uuid = uuid;
//        setUiEnabled(false);
    } else {
        QStringList info;
        if (!getApInfoBySsid(devName, ssid, info)) {
            return;
        }
        int index = m_interfaceComboBox->findText(devName);
        if (index >= 0) {
            if(!m_switchBtn->isChecked())
                showDesktopNotify(tr("hotspot already open"));
            m_apNameLine->setText(ssid);
            m_interfaceComboBox->setCurrentIndex(index);
            m_switchBtn->setChecked(true);
            m_switchBtn->setCheckable(true);
            m_pwdNameLine->setText(info.at(0));
            m_interfaceName = devName;
            updateBandCombox();
            index = m_freqBandComboBox->findText(info.at(1));
            if (index >= 0) {
                m_isUserSelect = false;
                m_freqBandComboBox->setCurrentIndex(index);
                m_isUserSelect = true;
            }
            m_uuid = uuid;
        } else {
            qDebug() << "no such device in combo box";
        }
    }
}

bool MobileHotspotWidget::getApInfoBySsid(QString devName, QString ssid, QStringList &info)
{
    info.clear();
    if(!m_interface->isValid()) {
        return false;
    }
    QDBusReply<QStringList> reply = m_interface->call("getApInfoBySsid", devName, ssid);
    if (!reply.isValid()) {
        qDebug()<<"execute dbus method 'getApInfoBySsid' is invalid in func getApInfoBySsid()";
    }
    info = reply.value();
    if (info.size() != 2) {
        return false;
    } else {
        return true;
    }
}

void MobileHotspotWidget::setUiEnabled(bool enable)
{
    qDebug() << "switch mode change to " << enable;
    if (enable) {
        m_pwdNameLine->setEnabled(false);
        m_freqBandComboBox->setEnabled(false);
        m_interfaceComboBox->setEnabled(false);
        m_apNameLine->setEnabled(false);
    } else {
        m_pwdNameLine->setEnabled(true);
        m_freqBandComboBox->setEnabled(true);
        m_interfaceComboBox->setEnabled(true);
        m_apNameLine->setEnabled(true);
    }
}

void MobileHotspotWidget::setWidgetHidden(bool isHidden)
{
    m_ApNameFrame->setHidden(isHidden);
    m_passwordFrame->setHidden(isHidden);
    m_freqBandFrame->setHidden(isHidden);
    m_interfaceFrame->setHidden(isHidden);

    switchAndApNameLine->setHidden(isHidden);
    apNameAndPwdLine->setHidden(isHidden);
    pwdAndfreqBandLine->setHidden(isHidden);
    freqBandAndInterfaceLine->setHidden(isHidden);

    if (isHidden) {
        m_switchBtn->setChecked(false);
        m_switchBtn->setCheckable(false);
        m_interfaceName = "";
        m_uuid = "";
    } else {
        m_switchBtn->setCheckable(true);
        QDBusReply<bool> reply = m_interface->call("getWirelessSwitchBtnState");
        bool state = reply.value();
        onWirelessBtnChanged(state);
    }
    resetFrameSize();

}

void MobileHotspotWidget::updateBandCombox()
{
    QString tmp = m_freqBandComboBox->currentText();
    m_freqBandComboBox->clear();
    QDBusReply<QVariantMap> capReply = m_interface->call("getWirelessDeviceCap");
    if (!capReply.isValid()) {
        qDebug()<<"execute dbus method 'getWirelessDeviceCap' is invalid in func initInterfaceInfo()" << capReply.error().message();
        setWidgetHidden(true);
        return;
    }
    m_isUserSelect = false;

    QMap<QString, int> devCapMap;
    QVariantMap::const_iterator itemIter = capReply.value().cbegin();
    while (itemIter != capReply.value().cend()) {
        devCapMap.insert(itemIter.key(), itemIter.value().toInt());
        itemIter ++;
    }

    if (devCapMap[m_interfaceName] & 0x02) {
         m_freqBandComboBox->addItem("2.4GHz");
    }
    if (devCapMap[m_interfaceName] & 0x04) {
        m_freqBandComboBox->addItem("5GHz");
    }
    int index = m_freqBandComboBox->findText(tmp);
    if (index >= 0) {
        m_freqBandComboBox->setCurrentIndex(index);
    }
    m_isUserSelect = true;
}

QFrame* MobileHotspotWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(LINE_MIN_SIZE));
    line->setMaximumSize(QSize(LINE_MAX_SIZE));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

QString MobileHotspotWidget::getHostName()
{
    int count = 0;
    while (count < 3) {
        QDBusInterface hostInterface("org.freedesktop.hostname1",
                                     "/org/freedesktop/hostname1",
                                     "org.freedesktop.hostname1",
                                     QDBusConnection::systemBus());
        if (hostInterface.isValid()) {
            return hostInterface.property("Hostname").value<QString>();
        } else {
            count++;
        }
    }
    return "default";
}

QString MobileHotspotWidget::getSettingPathByUuid()
{
    if (!m_interface->isValid()) {
        return nullptr;
    }

    QDBusReply<QString> reply = m_interface->call("getApConnectionPath", m_uuid);
    if (!reply.isValid()) {
        return nullptr;
    }

    return reply.value();
}

QString MobileHotspotWidget::getActivePathByUuid()
{
    if (!m_interface->isValid()) {
        return nullptr;
    }

    QDBusReply<QString> reply = m_interface->call("getActiveConnectionPath", m_uuid);
    if (!reply.isValid()) {
        return nullptr;
    }
    return reply.value();
}

void MobileHotspotWidget::initNmDbus()
{
    QString activePath = getActivePathByUuid();
    QString settingPath = getSettingPathByUuid();

    if (activePath != nullptr) {
        m_activePathInterface = new QDBusInterface("org.freedesktop.NetworkManager",
                                                    activePath,
                                                    "org.freedesktop.NetworkManager.Connection.Active",
                                                    QDBusConnection::systemBus());
        if (m_activePathInterface->isValid()) {
            m_connectDevPage->setInterface(m_activePathInterface);

            connect(m_activePathInterface, SIGNAL(NewStaConnected(bool, QString, QString)), m_connectDevPage, SLOT(onStaDevAdded(bool, QString, QString)), Qt::QueuedConnection);
            connect(m_activePathInterface, SIGNAL(StaRemoved(bool, QString, QString)), m_connectDevPage, SLOT(onStaDevRemoved(bool, QString, QString)), Qt::QueuedConnection);
        }
    }

    if (settingPath != nullptr) {
        m_settingPathInterface = new QDBusInterface("org.freedesktop.NetworkManager",
                                                    settingPath,
                                                    "org.freedesktop.NetworkManager.Settings.Connection",
                                                    QDBusConnection::systemBus());
        if (m_settingPathInterface->isValid()) {
            m_blacklistPage->setInterface(m_settingPathInterface);
        }
    }
}

void MobileHotspotWidget::initActivePathInterface(QString path)
{
    if (path != nullptr){
        m_activePathInterface = new QDBusInterface("org.freedesktop.NetworkManager",
                                                    path,
                                                    "org.freedesktop.NetworkManager.Connection.Active",
                                                    QDBusConnection::systemBus());
        if (m_activePathInterface->isValid()) {
            m_connectDevPage->setInterface(m_activePathInterface);

            connect(m_activePathInterface, SIGNAL(NewStaConnected(bool, QString, QString)), m_connectDevPage, SLOT(onStaDevAdded(bool, QString, QString)), Qt::QueuedConnection);
            connect(m_activePathInterface, SIGNAL(StaRemoved(bool, QString, QString)), m_connectDevPage, SLOT(onStaDevRemoved(bool, QString, QString)), Qt::QueuedConnection);
        }
    }
}

void MobileHotspotWidget::deleteActivePathInterface()
{
    if (m_activePathInterface != nullptr){
        disconnect(m_activePathInterface);
        delete m_activePathInterface;
        m_activePathInterface = nullptr;
    }
}

void MobileHotspotWidget::initSettingPathInterface(QString path)
{
    if (path != nullptr){
        m_settingPathInterface = new QDBusInterface("org.freedesktop.NetworkManager",
                                                    path,
                                                    "org.freedesktop.NetworkManager.Settings.Connection",
                                                    QDBusConnection::systemBus());
        if (m_settingPathInterface->isValid()) {
            m_blacklistPage->setInterface(m_settingPathInterface);
        }
    }
}

void MobileHotspotWidget::deleteSettingPathInterface()
{
    if (m_settingPathInterface != nullptr){
        disconnect(m_settingPathInterface);
        delete m_settingPathInterface;
        m_settingPathInterface = nullptr;
    }
}

void MobileHotspotWidget::initConnectDevPage()
{
    m_connectDevPage = new ConnectdevPage(this);
    m_Vlayout->addSpacing(32);
    m_Vlayout->addWidget(m_connectDevPage);
    connect(m_switchBtn, &KSwitchButton::stateChanged, m_connectDevPage, &ConnectdevPage::refreshStalist);
}

void MobileHotspotWidget::initBlackListPage()
{
    m_blacklistPage = new BlacklistPage(this);
    m_Vlayout->addSpacing(32);
    m_Vlayout->addWidget(m_blacklistPage);
}

