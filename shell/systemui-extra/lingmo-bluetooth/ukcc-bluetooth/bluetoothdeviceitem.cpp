/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "bluetoothdeviceitem.h"

//#define RADIUS 6.0
#define DEFAULT_FONT_WIDTH 280
/**
 * @brief
 *
 * @param parent
 * @param dev
 */
bluetoothdeviceitem::bluetoothdeviceitem( QString dev_address , QWidget *parent):
      _MDev_addr(dev_address),
      QPushButton(parent)
{
    KyDebug() << dev_address ;
    this->setFocusPolicy(Qt::NoFocus);

    if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(dev_address))
    {
        KyDebug() << dev_address << "not exist";
        return;
    }

    this->setObjectName(dev_address);

    devConnectiontimer = new QTimer(this);
    devConnectiontimer->setInterval(CONNECT_DEVICE_TIMING);
    connect(devConnectiontimer,&QTimer::timeout,this,[=]
    {
        devConnOperationTimeoutSlot();
    });

    devConnectionFail_timer = new QTimer(this);
    devConnectionFail_timer->setInterval(CONNECT_ERROR_TIMER_CLEAR);
    connect(devConnectionFail_timer,&QTimer::timeout,this,[=]
    {
        _mConnFailCount = 0 ;
        devConnectionFail_timer->stop();
    });

    bindDeviceChangedSignals();

    initGsettings();
    initInterface();
    refreshInterface();
}

/**
 * @brief
 *
 */
bluetoothdeviceitem::~bluetoothdeviceitem()
{
    KyDebug() << _MDev_addr ;
    _mStyle_GSettings->deleteLater();
    if (devConnectiontimer)
        devConnectiontimer->deleteLater();
    if (devConnectionFail_timer)
        devConnectionFail_timer->deleteLater();
}

void bluetoothdeviceitem::initGsettings()
{
    if (QGSettings::isSchemaInstalled(GSETTING_LINGMO_STYLE))
    {
        _mStyle_GSettings = new QGSettings(GSETTING_LINGMO_STYLE);

        if(_mStyle_GSettings->get("styleName").toString() == "lingmo-default" ||
           _mStyle_GSettings->get("style-name").toString() == "lingmo-light")
            _themeIsBlack = false;
        else
            _themeIsBlack = true;

        _mIconThemeName = _mStyle_GSettings->get("iconThemeName").toString();
        connect(_mStyle_GSettings,&QGSettings::changed,this,&bluetoothdeviceitem::mStyle_GSettingsSlot);
    }
}

void bluetoothdeviceitem::devStatusLoading()
{
    if (devStatusLabel->isVisible())
        devStatusLabel->hide();
    if (!devConnectiontimer->isActive())
        devConnectiontimer->start();
    if (devloadingLabel->isHidden())
        devloadingLabel->show();
    devloadingLabel->setTimerStart();
}

void bluetoothdeviceitem::devFuncOperationSlot()
{
//    KyDebug() << str_opt ;
    this->clearFocus();
//    if ("connect" == str_opt || "disconnect" == str_opt)
//    {
//        devStatusLoading();
//    }
}

void bluetoothdeviceitem::devBtnPressedSlot()
{
    this->update();
}
void bluetoothdeviceitem::devFuncOpertionRemoveSlot(QString address)
{
    Q_EMIT bluetoothDeviceItemRemove(address);
}

void bluetoothdeviceitem::devConnOperationTimeoutSlot()
{
    devloadingLabel->hide();
    devloadingLabel->setTimerStop();
    refreshDevCurrentStatus();
}

void bluetoothdeviceitem::initBackground()
{
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);
}

/**
 * @brief
 *
 */
void bluetoothdeviceitem::initInterface()
{
    KyDebug()  ;
    this->setMinimumSize(580, 56);
    initBackground();

    devItemHLayout = new QHBoxLayout(this);
    devItemHLayout->setContentsMargins(16,0,16,0);
    devItemHLayout->setSpacing(16);

    devIconLabel = new QLabel(this);
    devIconLabel->setPixmap(getDevTypeIcon());
    devItemHLayout->addWidget(devIconLabel);

    devNameLabel = new QLabel(this);
    devNameLabel->setContentsMargins(1,0,1,0);
    devNameLabel->resize(DEFAULT_FONT_WIDTH,this->height());
    devNameLabel->setText(getDevName());
    devItemHLayout->addWidget(devNameLabel);
    devItemHLayout->addStretch(100);
    devNameLabel->setFocus();

    devloadingLabel = new LoadingLabel(this);
    devloadingLabel->setFixedSize(16,16);
    devloadingLabel->setTimerStart();
    devItemHLayout->addWidget(devloadingLabel, 1, Qt::AlignRight);
    devloadingLabel->hide();//默认隐藏

    devStatusLabel = new QLabel(this);
    devStatusLabel->setText(getDevStatus());
    devItemHLayout->addWidget(devStatusLabel,Qt::AlignRight);
    devStatusLabel->hide();//默认隐藏

    devFuncBtn = new bluetoothdevicefunc(this,_MDev_addr);
    devItemHLayout->addWidget(devFuncBtn);
    bindInInterfaceUISignals();
}
/**
 * @brief
 *
 */
void bluetoothdeviceitem::bindInInterfaceUISignals()
{
    connect(devFuncBtn,SIGNAL(devFuncOpertionSignal()),this,SLOT(devFuncOperationSlot()));
    connect(devFuncBtn,SIGNAL(devBtnReleaseSignal()),this,SLOT(devBtnPressedSlot()));
    connect(devFuncBtn,SIGNAL(devFuncOpertionRemoveSignal(QString)),this,SIGNAL(bluetoothDeviceItemRemove(QString)));
}

/**
 * @brief
 *
 */

void bluetoothdeviceitem::refreshInterface()
{
    KyDebug();
    //优先级状态判断
    //if(_MDev->getDevPairing() || _MDev->getDevConnecting())
    if(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevConnecting())
    {
        devStatusLabel->hide();
        devConnectiontimer->start();
        devloadingLabel->show();
        devloadingLabel->setTimerStart();
    }
    else if(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired() ||
            BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected())
    {
        devStatusLabel->show();
        devloadingLabel->hide();
    }
    else
    {
        devStatusLabel->hide();
        devloadingLabel->hide();
    }
    devFuncBtn->setVisible(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired());
}

void bluetoothdeviceitem::devItemNameChanged(const QString &name)
{
    KyDebug() << name;

    if (devNameLabel)
    {
        QFontMetrics fontWidth(devNameLabel->font());//得到QLabel字符的度量
        int font_w = fontWidth.width(name);

        QString elidedNote = name ;

        if (font_w > DEFAULT_FONT_WIDTH)
            elidedNote = fontWidth.elidedText(name, Qt::ElideMiddle, DEFAULT_FONT_WIDTH);//获取处理后的文本

        if(elidedNote != name)
        {
            devNameLabel->setToolTip(name);
        }
        else
        {
            devNameLabel->setToolTip("");
        }

        devNameLabel->setText(elidedNote);
    }
}

void bluetoothdeviceitem::devItemTypeChanged(const bluetoothdevice::DEVICE_TYPE &type)
{
    KyDebug() << type;
    if (devIconLabel)
        devIconLabel->setPixmap(getDevTypeIcon());
    KyDebug() << "end" ;

}
void bluetoothdeviceitem::devItemStatusChanged(const QString &status)
{
    KyDebug()  << status;
    if (devStatusLabel)
        devStatusLabel->setText(status);
}

/**
 * @brief
 *
 */
void bluetoothdeviceitem::bindDeviceChangedSignals()
{
    KyDebug();

    if (BlueToothDBusService::m_default_bluetooth_adapter && BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr))
    {
        KyDebug() << "connect dev item";

        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::nameChanged,this,[=](const QString &name)
        {
            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     <<"nameChanged:" << name ;
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevShowName().isEmpty())
                devItemNameChanged(name);
        });

        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::showNameChanged,this,[=](const QString &name)
        {
            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     << "showNameChanged:" << name ;
            devItemNameChanged(name);
        });

        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::typeChanged,this,[=](bluetoothdevice::DEVICE_TYPE type)
        {
            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     << "typeChanged:" << type ;
            devItemTypeChanged(type);
        });

        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::pairedChanged,this,[=](bool paired)
        {
            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     << "pairedChanged:" << paired ;
            if (paired)
            {
                emit devPairedSuccess(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress());
            }
            refreshDevCurrentStatus();
        });

        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::connectedChanged,this,[=](bool connected)
        {
            emit this->devConnectedChanged(connected);

            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     << "connectedChanged:" << connected ;
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired())
            {
                emit devPairedSuccess(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress());
            }

            refreshDevCurrentStatus();
        });


        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::connectingChanged,this,[=](bool connecting)
        {
            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     << "connectingChanged:" << connecting ;
            refreshDevCurrentStatus();
            if(connecting)
            {
                BlueToothMainWindow::m_device_operating = true ;
            }
            else
            {
                BlueToothMainWindow::m_device_operating = false;
            }
            devFuncBtn->setEnabled(!connecting);
        });

        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::errorInfoRefresh,this,[=](int errorId , QString errorText)
        {
            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     <<"error:" << errorId << errorText ;
            refreshDevCurrentStatus();
            if (errorId && BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired())
                devConnectionFail();
        });

        connect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr],&bluetoothdevice::rssiChanged,this,[=](qint16 value)
        {
            KyDebug()<< BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()
                     << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress()
                     << "rssiChanged:" << value ;
            if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired())
                emit devRssiChanged(value);
        });
    }
}

/************************************************
 * @brief  getDevTypeIcon
 * @param  null
 * @return QPixmap
*************************************************/
QPixmap bluetoothdeviceitem::getDevTypeIcon()
{
    KyDebug();

    QPixmap icon;
    QString iconName;
    if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr)) {
        switch (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevType()) {
        case bluetoothdevice::DEVICE_TYPE::phone:
            iconName = "phone-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::computer:
            iconName = "video-display-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::headset:
        case bluetoothdevice::DEVICE_TYPE::headphones:
            iconName = "audio-headphones-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::audiovideo:
            iconName = "audio-speakers-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::keyboard:
            iconName = "input-keyboard-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::mouse:
            iconName = "input-mouse-symbolic";
            break;
        default:
            iconName = "bluetooth-symbolic";
            break;
        }
    }
    else
    {
        iconName = "bluetooth-symbolic";
    }

    if (_themeIsBlack)
    {
        icon = ukccbluetoothconfig::loadSvg(QIcon::fromTheme(iconName).pixmap(DEV_ICON_WH),ukccbluetoothconfig::WHITE);
    }
    else
    {
        icon = QIcon::fromTheme(iconName).pixmap(DEV_ICON_WH);
    }

    return icon;
}

/************************************************
 * @brief  getDevName
 * @param  null
 * @return QString
*************************************************/
QString bluetoothdeviceitem::getDevName()
{
    KyDebug()  ;
    QString name;
    if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr))
        name = BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevInterfaceShowName();
    else
        name = "Bluetooth Test Name";

    if(devNameLabel)
    {
        QFontMetrics fontWidth(devNameLabel->font());//得到QLabel字符的度量

        int font_w = fontWidth.width(name);

        QString elidedNote = name ;

        if (font_w > DEFAULT_FONT_WIDTH)
            elidedNote = fontWidth.elidedText(name, Qt::ElideMiddle, DEFAULT_FONT_WIDTH);//获取处理后的文本

        if(elidedNote != name)
        {
            devNameLabel->setToolTip(name);
            return elidedNote;
        }
        else
        {
            devNameLabel->setToolTip("");
        }
    }
    return name;
}


/**
 * @brief
 *
 * @return QString
 */
QString bluetoothdeviceitem::getDevStatus()
{
    KyDebug();
    QString strStatus;
    if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr))
    {
        //注意判断优先级
        //if (_MDev->getDevPairing() || _MDev->getDevConnecting())
        if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevConnecting())
        {
            strStatus = m_str_connecting;
        }
        else if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired() &&  0 != BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getErrorId())
        {
            strStatus = m_str_connectionfail;
        }
        else if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired() && !BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected())
        {
            strStatus = m_str_notconnected;
        }
        else if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired() && BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected())
        {
            strStatus = m_str_connected;
        }
        else
        {
            strStatus = m_str_notpaired;
        }
    }
    else
        strStatus = m_str_notpaired;
    return strStatus;
}


void bluetoothdeviceitem::refreshDevCurrentStatus()
{
    KyDebug()  ;

    if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevConnecting())
    {
        devStatusLoading();
    }
    else if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired() && !BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected())
    {
        devItemStatusChanged(getDevStatus());
        if (devStatusLabel->isHidden())
        {
            devStatusLabel->show();
        }
//        devStatusLabel->setEnabled(false);

        //if (devloadingLabel->isVisible())
        {
            devloadingLabel->hide();
            devloadingLabel->setTimerStop();
        }

//        if (devFuncFrame->isHidden())
//            devFuncFrame->show();
        if (devFuncBtn->isHidden())
            devFuncBtn->show();
        if (devConnectiontimer->isActive())
            devConnectiontimer->stop();
    }
    else if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isPaired() && BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected())
    {
        devItemStatusChanged(getDevStatus());
        if (devStatusLabel->isHidden())
        {
            devStatusLabel->show();
        }
//        devStatusLabel->setEnabled(true);

        //KyWarning() << "devloadingLabel->isVisible()" << "devFuncFrame->isHidden()" << "devFuncFrame->isVisible()";
        //KyWarning() << devloadingLabel->isVisible() << devloadingLabel->isHidden() << devFuncFrame->isHidden() << devFuncFrame->isVisible();

        //if (devloadingLabel->isHidden())
        {
            devloadingLabel->hide();
            devloadingLabel->setTimerStop();
        }

        //KyWarning() << devFuncFrame->isHidden() << devFuncFrame->isVisible();

//        if (devFuncFrame->isHidden())
//            devFuncFrame->show();
        if (devFuncBtn->isHidden())
            devFuncBtn->show();
        if (devConnectiontimer->isActive())
            devConnectiontimer->stop();
    }
    else if (0 != BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getErrorId())
    {
        devItemStatusChanged(getDevStatus());
        if (devStatusLabel->isHidden())
            devStatusLabel->show();

        //if (devloadingLabel->isVisible())
        {
            devloadingLabel->hide();
            devloadingLabel->setTimerStop();
        }

        if(devConnectiontimer->isActive())
            devConnectiontimer->stop();
    }
    else
    {
        if (devStatusLabel->isVisible())
            devStatusLabel->hide();

        if (devloadingLabel->isVisible())
        {
            devloadingLabel->hide();
            devloadingLabel->setTimerStop();
        }

        if (devConnectiontimer->isActive())
            devConnectiontimer->stop();
    }

    devNameLabel->setText(getDevName());
}

void bluetoothdeviceitem::devConnectionFail()
{
    KyDebug() << "_mConnFailCount :" << _mConnFailCount << "devConnectionFail_timer->isActive():" << devConnectionFail_timer->isActive();
    if ((0 == _mConnFailCount) && !devConnectionFail_timer->isActive())
        devConnectionFail_timer->start();

    _mConnFailCount++;

    if (_mConnFailCount > CONNECT_ERROR_COUNT)
    {
        _mConnFailCount = 0;
        devConnectionFail_timer->stop();
        showDeviceRemoveWidget(DevRemoveDialog::REMOVE_MANY_TIMES_CONN_FAIL_DEV);
    }
}

void bluetoothdeviceitem::showDeviceRemoveWidget(DevRemoveDialog::REMOVE_INTERFACE_TYPE mode)
{
    DevRemoveDialog *mesgBox = new DevRemoveDialog(mode,this);
    mesgBox->setModal(true);
    mesgBox->setDialogText(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevInterfaceShowName());

    //rejected
    connect(mesgBox,&DevRemoveDialog::rejected,this,[=]{
        _mConnFailCount = 0;
        devConnectionFail_timer->stop();
        devNameLabel->setFocus();
    });
    //accepted
    connect(mesgBox,&DevRemoveDialog::accepted,this,[=]{
       KyDebug()  << "To :" << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevName()  << "Remove" ;
       BlueToothDBusService::devRemove(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress());
    });
    //finished
    connect(mesgBox,&DevRemoveDialog::finished,this,[=](int result){
       KyDebug()  << "result:" <<result ;
       devNameLabel->setFocus();
    });
    mesgBox->exec();
}

void bluetoothdeviceitem::mStyle_GSettingsSlot(const QString &key)
{
    KyDebug() << key;
    if ("iconThemeName" == key || "icon-theme-name" == key)
    {
        _mIconThemeName = _mStyle_GSettings->get("iconThemeName").toString();
        if (devIconLabel)
            devIconLabel->setPixmap(getDevTypeIcon());
    }
    else if ("styleName" == key || "style-name" == key)
    {
        if(_mStyle_GSettings->get("style-name").toString() == "lingmo-default" ||
           _mStyle_GSettings->get("style-name").toString() == "lingmo-light")
        {
            _themeIsBlack = false;
        }
        else
        {
            _themeIsBlack = true;
        }

        if (devIconLabel)
            devIconLabel->setPixmap(getDevTypeIcon());
    }
    else if ("system-font-size" == key)
    {
        KyDebug() << key;
        devNameLabel->setText(getDevName());
    }
//    initBackground();
    this->update();
}

void bluetoothdeviceitem::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(this->palette().base().color());

    QPalette pal = qApp->palette();
    QColor color = pal.color(QPalette::Button);
    color.setAlphaF(0.5);
    pal.setColor(QPalette::Button, color);
    this->setPalette(pal);

    QPushButton::paintEvent(event);
}

void bluetoothdeviceitem::mousePressEvent(QMouseEvent *event)
{
    //获取当前时间
    _pressCurrentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    KyInfo() << _pressCurrentTime;

    QPushButton::mousePressEvent(event);
}

void bluetoothdeviceitem::mouseReleaseEvent(QMouseEvent *event)
{
    long long _releaseCurrentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    KyDebug()  << "_releaseCurrentTime" << _releaseCurrentTime << "_pressCurrentTime:" << _pressCurrentTime;

    if(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr) && (_releaseCurrentTime - _pressCurrentTime) <= 300)
    {//点击超时取消操作
        if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected() && !BlueToothMainWindow::m_device_operating)
        {
            devStatusLoading();
            BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->setDevConnecting(true);
            BlueToothDBusService::devConnect(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress());
        }
    }

    QPushButton::mouseReleaseEvent(event);
}


void bluetoothdeviceitem::mouseMoveEvent(QMouseEvent *e)
{
    KyDebug();

}
