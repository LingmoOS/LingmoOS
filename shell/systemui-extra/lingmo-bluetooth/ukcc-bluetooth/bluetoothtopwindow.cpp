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

#include "bluetoothtopwindow.h"

#include "common.h"

bool BluetoothTopWindow::m_defaultAdapterPowerStatus = false;

BluetoothTopWindow::BluetoothTopWindow(BlueToothDBusService * btServer,QWidget * parent) :
    m_btServer(btServer),
    QWidget(parent)
{
//    this->setFocusPolicy(Qt::NoFocus);
    this->setFocus(Qt::NoFocusReason);


    InitNormalWidgetTop();

    if (btServer->m_default_bluetooth_adapter)
        InitDisplayState();

    InitConnectionData();
}

BluetoothTopWindow::~BluetoothTopWindow()
{

}

void BluetoothTopWindow::InitNormalWidgetTop()
{
//    KyDebug();
    QVBoxLayout *NormalWidgetTopLayout = new QVBoxLayout(this);
    NormalWidgetTopLayout->setSpacing(8);
    NormalWidgetTopLayout->setContentsMargins(0,0,0,0);

    //~ contents_path /Bluetooth/Bluetooth
    QLabel *textBluetoothLabel = new QLabel(tr("Bluetooth"),this);
    textBluetoothLabel->resize(100,25);
    textBluetoothLabel->setContentsMargins(16,0,0,0);
    NormalWidgetTopLayout->addWidget(textBluetoothLabel,1,Qt::AlignTop);

    _MNormalFrameTop = new QFrame(this);
    _MNormalFrameTop->setMinimumWidth(MIN_WIDTH);
    _MNormalFrameTop->setFrameShape(QFrame::Shape::Box);
    NormalWidgetTopLayout->addWidget(_MNormalFrameTop);

    QVBoxLayout *NormalFrameLayout = new QVBoxLayout(this);
    NormalFrameLayout->setSpacing(0);
    NormalFrameLayout->setContentsMargins(0,0,0,0);
    _MNormalFrameTop->setLayout(NormalFrameLayout);

    //================================ init BtSwitchFrame =======================================
    QFrame *BtSwitchFrame = new QFrame(_MNormalFrameTop);
    BtSwitchFrame->setMinimumWidth(MIN_WIDTH);
    BtSwitchFrame->setFrameShape(QFrame::Shape::Box);
    BtSwitchFrame->setFixedHeight(MIN_HEIGTH);
//    BtSwitchFrame->setAutoFillBackground(true);
    NormalFrameLayout->addWidget(BtSwitchFrame,1,Qt::AlignTop);

    QHBoxLayout *BtSwitchFrameLayout = new QHBoxLayout(BtSwitchFrame);
    BtSwitchFrameLayout->setSpacing(0);
    BtSwitchFrameLayout->setContentsMargins(16,0,16,0);

    //~ contents_path /Bluetooth/Turn on :
    QLabel *BtSwitchLabel = new QLabel(tr("Turn on :"),BtSwitchFrame);
    BtSwitchFrameLayout->addWidget(BtSwitchLabel,1,Qt::AlignLeft);

    _BtNameLabel = new BluetoothNameLabel(BtSwitchFrame,300,38);
    BtSwitchFrameLayout->addWidget(_BtNameLabel,1,Qt::AlignLeft);

    BtSwitchFrameLayout->addStretch(25);


    _BtSwitchBtn = new KSwitchButton(BtSwitchFrame);
//    _BtSwitchBtn->setChecked(true);
    BtSwitchFrameLayout->addWidget(_BtSwitchBtn);

    //添加分割线
    BtSwitchLineFrame = new QFrame(_MNormalFrameTop);
    BtSwitchLineFrame->setFixedHeight(1);
    BtSwitchLineFrame->setMinimumWidth(MIN_WIDTH);
    BtSwitchLineFrame->setFrameStyle(QFrame::HLine);
    NormalFrameLayout->addWidget(BtSwitchLineFrame);
    //================================ end init BtSwitchFrame =======================================

    //================================ init BtAdapterListFrame ===========================================
    BtAdapterListFrame = new QFrame(_MNormalFrameTop);
    BtAdapterListFrame->setMinimumWidth(MIN_WIDTH);
    BtAdapterListFrame->setFrameShape(QFrame::Shape::Box);
    BtAdapterListFrame->setFixedHeight(MIN_HEIGTH);
//    BtAdapterListFrame->setAutoFillBackground(true);
    NormalFrameLayout->addWidget(BtAdapterListFrame,1,Qt::AlignTop);

    QHBoxLayout *BtAdapterListFrameLayout = new QHBoxLayout(BtAdapterListFrame);
    BtAdapterListFrameLayout->setSpacing(0);
    BtAdapterListFrameLayout->setContentsMargins(16,0,16,0);

    //~ contents_path /Bluetooth/Adapter List
    QLabel *textAdapterListLabel = new QLabel(tr("Adapter List"),BtAdapterListFrame);
    BtAdapterListFrameLayout->addWidget(textAdapterListLabel);
    BtAdapterListFrameLayout->addStretch(10);

    _AdapterListSelectComboBox = new QComboBox(BtAdapterListFrame);
    _AdapterListSelectComboBox->clear();
    _AdapterListSelectComboBox->setMinimumWidth(300);
    BtAdapterListFrameLayout->addWidget(_AdapterListSelectComboBox);

    //添加分割线
    BtAdapterListLineFrame = new QFrame(_MNormalFrameTop);
    BtAdapterListLineFrame->setFixedHeight(1);
    BtAdapterListLineFrame->setMinimumWidth(MIN_WIDTH);
    BtAdapterListLineFrame->setFrameStyle(QFrame::HLine);
    NormalFrameLayout->addWidget(BtAdapterListLineFrame);
    //================================ end init BtAdapterListFrame =======================================

    //================================ init BtTrayIconShowFrame ===========================================
    BtTrayIconShowFrame = new QFrame(_MNormalFrameTop);
    BtTrayIconShowFrame->setMinimumWidth(MIN_WIDTH);
    BtTrayIconShowFrame->setFrameShape(QFrame::Shape::Box);
    BtTrayIconShowFrame->setFixedHeight(MIN_HEIGTH);
//    BtTrayIconShowFrame->setAutoFillBackground(true);
    NormalFrameLayout->addWidget(BtTrayIconShowFrame,1,Qt::AlignTop);

    QHBoxLayout *BtTrayIconShowFrameLayout = new QHBoxLayout(BtTrayIconShowFrame);
    BtTrayIconShowFrameLayout->setSpacing(0);
    BtTrayIconShowFrameLayout->setContentsMargins(16,0,16,0);

    //~ contents_path /Bluetooth/Show icon on taskbar
    QLabel *BtTrayIconShowLabel = new QLabel(tr("Show icon on taskbar"),BtTrayIconShowFrame);
    BtTrayIconShowFrameLayout->addWidget(BtTrayIconShowLabel);

    _BtTrayIconShow = new KSwitchButton(BtTrayIconShowFrame);
    BtTrayIconShowFrameLayout->addStretch(10);
    BtTrayIconShowFrameLayout->addWidget(_BtTrayIconShow);

    BtTrayIconShowLineFrame = new QFrame(_MNormalFrameTop);
    BtTrayIconShowLineFrame->setFixedHeight(1);
    BtTrayIconShowLineFrame->setMinimumWidth(MIN_WIDTH);
    BtTrayIconShowLineFrame->setFrameStyle(QFrame::HLine);
    NormalFrameLayout->addWidget(BtTrayIconShowLineFrame);
    //================================ end init BtTrayIconShowFrame =======================================


    //================================ init BtDiscoverableFrame ===========================================
    BtDiscoverableFrame = new QFrame(_MNormalFrameTop);
    BtDiscoverableFrame->setMinimumWidth(MIN_WIDTH);
    BtDiscoverableFrame->setFrameShape(QFrame::Shape::Box);
    BtDiscoverableFrame->setFixedHeight(MIN_HEIGTH);
//    BtDiscoverableFrame->setAutoFillBackground(true);
    NormalFrameLayout->addWidget(BtDiscoverableFrame,1,Qt::AlignTop);

    QHBoxLayout *BtDiscoverableFrameLayout = new QHBoxLayout(BtDiscoverableFrame);
    BtDiscoverableFrameLayout->setSpacing(0);
    BtDiscoverableFrameLayout->setContentsMargins(16,0,16,0);

    //~ contents_path /Bluetooth/Discoverable by nearby Bluetooth devices
    QLabel *BtDiscoverableLabel = new QLabel(tr("Discoverable by nearby Bluetooth devices"),BtDiscoverableFrame);
    BtDiscoverableFrameLayout->addWidget(BtDiscoverableLabel);

    _BtDiscoverable = new KSwitchButton(BtDiscoverableFrame);
    BtDiscoverableFrameLayout->addStretch(10);
    BtDiscoverableFrameLayout->addWidget(_BtDiscoverable);

    //添加分割线
    BtDiscoverableLineFrame = new QFrame(_MNormalFrameTop);
    BtDiscoverableLineFrame->setFixedHeight(1);
    BtDiscoverableLineFrame->setMinimumWidth(MIN_WIDTH);
    BtDiscoverableLineFrame->setFrameStyle(QFrame::HLine);
    NormalFrameLayout->addWidget(BtDiscoverableLineFrame);
    //================================ end init BtDiscoverableFrame =======================================

    //================================ init BtAutomaticAudioConnectionFrame ===========================================
    BtAutomaticAudioConnectionFrame = new QFrame(_MNormalFrameTop);
    BtAutomaticAudioConnectionFrame->setMinimumWidth(MIN_WIDTH);
    BtAutomaticAudioConnectionFrame->setFrameShape(QFrame::Shape::Box);
    BtAutomaticAudioConnectionFrame->setFixedHeight(MIN_HEIGTH);
//    BtAutomaticAudioConnectionFrame->setAutoFillBackground(true);
    NormalFrameLayout->addWidget(BtAutomaticAudioConnectionFrame,1,Qt::AlignTop);

    QHBoxLayout *BtAutomaticAudioConnectionFrameLayout = new QHBoxLayout(BtAutomaticAudioConnectionFrame);
    BtAutomaticAudioConnectionFrameLayout->setSpacing(0);
    BtAutomaticAudioConnectionFrameLayout->setContentsMargins(16,0,16,0);

    //~ contents_path /Bluetooth/Auto discover Bluetooth audio devices
    QLabel *BtAutomaticAudioConnectionLabel = new QLabel(tr("Auto discover Bluetooth audio devices"),BtAutomaticAudioConnectionFrame);
    BtAutomaticAudioConnectionFrameLayout->addWidget(BtAutomaticAudioConnectionLabel);

    _BtAutoAudioConnBtn = new KSwitchButton(BtDiscoverableFrame);
    BtAutomaticAudioConnectionFrameLayout->addStretch(10);
    BtAutomaticAudioConnectionFrameLayout->addWidget(_BtAutoAudioConnBtn);
    //================================ end init BtAutomaticAudioConnectionFrame =======================================

    NormalWidgetTopLayout->addStretch(2);

}

void BluetoothTopWindow::InitDisplayState()
{
    if (!BlueToothDBusService::m_default_bluetooth_adapter)
        return;

    //bluetooth
    _BtNameLabel->set_dev_name(BlueToothDBusService::m_default_bluetooth_adapter->getDevName());
    KyWarning()<< "power status:" << BlueToothDBusService::m_default_bluetooth_adapter->getAdapterPower();
    _BtSwitchBtn->setChecked(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterPower());
    m_defaultAdapterPowerStatus = BlueToothDBusService::m_default_bluetooth_adapter->getAdapterPower();
    //开关界面显示加载
    BtSwitchLineFrame->setVisible(_BtSwitchBtn->isChecked());
    if (BlueToothDBusService::m_bluetooth_adapter_name_list.size() > 1)
    {
        BtAdapterListFrame->setVisible(_BtSwitchBtn->isChecked());
        BtAdapterListLineFrame->setVisible(_BtSwitchBtn->isChecked());
    }

    BtTrayIconShowFrame->setVisible(_BtSwitchBtn->isChecked());
    BtTrayIconShowLineFrame->setVisible(_BtSwitchBtn->isChecked());

    BtDiscoverableFrame->setVisible(_BtSwitchBtn->isChecked());
    BtDiscoverableLineFrame->setVisible(_BtSwitchBtn->isChecked());

    BtAutomaticAudioConnectionFrame->setVisible(_BtSwitchBtn->isChecked());

    //bluetooth adapter list
    KyWarning() << "m_bluetooth_adapter_name_list::" << BlueToothDBusService::m_bluetooth_adapter_name_list;
    if (BlueToothDBusService::m_bluetooth_adapter_name_list.size() <= 1)
    {
        _AdapterListSelectComboBox->clear();
        _AdapterListSelectComboBox->addItems(BlueToothDBusService::m_bluetooth_adapter_name_list);
        BtAdapterListFrame->hide();
        BtAdapterListLineFrame->hide();
    }
    else
    {
        _AdapterListSelectComboBox->clear();
        _AdapterListSelectComboBox->addItems(BlueToothDBusService::m_bluetooth_adapter_name_list);
        _AdapterListSelectComboBox->setCurrentText(BlueToothDBusService::m_default_bluetooth_adapter->getDevName());

        BtAdapterListFrame->setVisible(_BtSwitchBtn->isChecked());
        BtAdapterListLineFrame->setVisible(_BtSwitchBtn->isChecked());
    }

    //show taskbar
    _BtTrayIconShow->setChecked(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterTrayShow());

    //discoverable
    KyWarning()<< "_BtDiscoverable::" << _BtDiscoverable->isChecked() << BlueToothDBusService::m_default_bluetooth_adapter->getAdapterDiscoverable();
    _BtDiscoverable->setChecked(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterDiscoverable());

    //auto audio conncet
    _BtAutoAudioConnBtn->setChecked(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterAutoConn());

}

void BluetoothTopWindow::InitConnectionData()
{
    //name label
    connect(_BtNameLabel,&BluetoothNameLabel::sendAdapterName,this,&BluetoothTopWindow::setDefaultAdapterNameSlot);

    //btn
    connect(_BtSwitchBtn,SIGNAL(stateChanged(bool)),this,SLOT(_BtSwitchBtnSlot(bool)));
    connect(_BtSwitchBtn, &KSwitchButton::pressed, this, &BluetoothTopWindow::_BtSwitchBtnPressedSlot);
    connect(_BtSwitchBtn, &KSwitchButton::released, this, &BluetoothTopWindow::_BtSwitchBtnReleasedSlot);

    connect(_BtTrayIconShow,SIGNAL(stateChanged(bool)),this,SLOT(_BtTrayIconShowSlot(bool)));
    connect(_BtDiscoverable,SIGNAL(stateChanged(bool)),this,SLOT(_BtDiscoverableSlot(bool)));
    connect(_BtAutoAudioConnBtn,SIGNAL(stateChanged(bool)),this,SLOT(_BtAutoAudioConnBtnSlot(bool)));

    //ComboBox
    connect(_AdapterListSelectComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(_AdapterListSelectComboBoxSlot(int)));

    if(m_btServer)
    {
        connect(m_btServer,SIGNAL(adapterAddSignal(QString)),this,SLOT(adapterAddSlot(QString)));
        connect(m_btServer,SIGNAL(adapterRemoveSignal(int)),this,SLOT(adapterRemoveSlot(int)));
        connect(m_btServer,SIGNAL(defaultAdapterChangedSignal(int)),this,SLOT(defaultAdapterChangedSlot(int)));
        //adapter status change
        connect(m_btServer,SIGNAL(adapterNameChanged(QString)),this,SLOT(adapterNameChangedSlot(QString)));
        connect(m_btServer,SIGNAL(adapterNameChangedOfIndex(int,QString)),this,SLOT(adapterNameChangedOfIndexSlot(int,QString)));

        connect(m_btServer,SIGNAL(adapterPoweredChanged(bool)),this,SLOT(adapterPowerStatusChangedSlot(bool)));
        connect(m_btServer,SIGNAL(adapterTrayIconChanged(bool)),this,SLOT(adapterTrayIconSlot(bool)));
        connect(m_btServer,SIGNAL(adapterDiscoverableChanged(bool)),this,SLOT(adapterDiscoverableSlot(bool)));
        connect(m_btServer,SIGNAL(adapterActiveConnectionChanged(bool)),this,SLOT(adapterActiveConnectionSlot(bool)));
    }
}

void BluetoothTopWindow::adapterAddSlot(QString adapter_name)
{
    KyDebug() << "adapter_name:" << adapter_name
              << "adapter_address_list:" << BlueToothDBusService::m_bluetooth_adapter_address_list
              << "size :" << BlueToothDBusService::m_bluetooth_adapter_address_list.size();

    if(BlueToothDBusService::m_bluetooth_adapter_address_list.size())
    {
        _AdapterListSelectComboBox->addItem(adapter_name);

        if(BlueToothDBusService::m_bluetooth_adapter_address_list.size() >1)
        {
            if (BtAdapterListFrame->isHidden())
                BtAdapterListFrame->setVisible(_BtSwitchBtn->isChecked());
            if (BtAdapterListLineFrame->isHidden())
                BtAdapterListLineFrame->setVisible(_BtSwitchBtn->isChecked());
        }
    }
}

void BluetoothTopWindow::adapterRemoveSlot(int indx)
{

    KyDebug() << "adapter indx:" << indx << endl
              << "adapter_name_list size:" << BlueToothDBusService::m_bluetooth_adapter_name_list.size() << endl
              << "adapter_name_list:" << BlueToothDBusService::m_bluetooth_adapter_name_list;


    if (1 == BlueToothDBusService::m_bluetooth_adapter_name_list.size())
    {
        KyWarning() << "adapter indx:" << indx << endl
                  << "_AdapterListSelectComboBox currentIndex:" << _AdapterListSelectComboBox->currentIndex();

        if (indx == _AdapterListSelectComboBox->currentIndex())
        {
        //移除适配器是默认适配器
            adapterChangedRefreshInterface(0);
        }
        else
        {
            BtAdapterListFrame->hide();
            BtAdapterListLineFrame->hide();
            _AdapterListSelectComboBox->removeItem(indx);
        }


//        bool flag = false ;
//        if (indx == _AdapterListSelectComboBox->currentIndex())
//        {
//            flag = true;
//            _AdapterListSelectComboBox->disconnect();


//        }

//        _AdapterListSelectComboBox->removeItem(indx);

//        if(flag)
//            connect(_AdapterListSelectComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(_AdapterListSelectComboBoxSlot(int)));

    }
    else
    {
        if (indx == _AdapterListSelectComboBox->currentIndex())
        {
        //移除适配器是默认适配器
            adapterChangedRefreshInterface(0);
        }
        else
        {
            _AdapterListSelectComboBox->removeItem(indx);
        }

//        BtAdapterListFrame->show();
//        BtAdapterListLineFrame->show();
//        bool flag = false ;
//        if (indx == _AdapterListSelectComboBox->currentIndex())
//        {
//            flag = true;
//            _AdapterListSelectComboBox->disconnect();
//        }

//        _AdapterListSelectComboBox->removeItem(indx);

//        if (flag)
//            connect(_AdapterListSelectComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(_AdapterListSelectComboBoxSlot(int)));
    }

}

void BluetoothTopWindow::defaultAdapterChangedSlot(int indx)
{
    adapterChangedRefreshInterface(indx);
}


void BluetoothTopWindow::setDefaultAdapterNameSlot(QString name)
{
    KyDebug();
    BlueToothDBusService::setDefaultAdapterName(name);
}

void BluetoothTopWindow::sendBtPowerChangedSignal(bool status)
{
    m_defaultAdapterPowerStatus = status;
    Q_EMIT btPowerSwitchChanged(status);
}

void BluetoothTopWindow::_BtSwitchBtnSlot(bool status)
{
    KyDebug() << "status" << status << "_BtSwitchBtn status: "<< _BtSwitchBtn->isChecked();

    if (!_BTServiceReportPowerSwitchFlag && !status  && whetherNeedInfoUser())
    {
        KyInfo() << "Close bluetooth risk info!" ;

        QMessageBox box;
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("Using Bluetooth mouse or keyboard, Do you want to turn off bluetooth?"));
        QPushButton * cancelBtn= new QPushButton(tr("Cancel"));
        QPushButton * closeBluetoothBtn= new QPushButton(tr("Close bluetooth"));

        box.addButton(cancelBtn,QMessageBox::RejectRole);
        box.addButton(closeBluetoothBtn,QMessageBox::AcceptRole);
        int res = box.exec();
        if (!res)
        {
            _BtSwitchBtn->disconnect(_BtSwitchBtn,NULL,this,NULL);
            _BtSwitchBtn->setChecked(true);
            connect(_BtSwitchBtn,SIGNAL(stateChanged(bool)),this,SLOT(_BtSwitchBtnSlot(bool)));
            connect(_BtSwitchBtn, &KSwitchButton::pressed, this, &BluetoothTopWindow::_BtSwitchBtnPressedSlot);
            connect(_BtSwitchBtn, &KSwitchButton::released, this, &BluetoothTopWindow::_BtSwitchBtnReleasedSlot);
            return;
        }
    }

    BtSwitchLineFrame->setVisible(status);
    if (BlueToothDBusService::m_bluetooth_adapter_name_list.size() > 1)
    {
        BtAdapterListFrame->setVisible(status);
        BtAdapterListLineFrame->setVisible(status);
    }

    BtTrayIconShowFrame->setVisible(status);
    BtTrayIconShowLineFrame->setVisible(status);

    BtDiscoverableFrame->setVisible(status);
    BtDiscoverableLineFrame->setVisible(status);

    BtAutomaticAudioConnectionFrame->setVisible(status);

    //发送信号
    sendBtPowerChangedSignal(status);

    KyInfo() << "_BTServiceReportPowerSwitchFlag:" << _BTServiceReportPowerSwitchFlag ;
    if(!_BTServiceReportPowerSwitchFlag)//服务上报的状态不再次下发
    {
        m_SwitchBtnPressed = false;
        ukccbluetoothconfig::ukccBtBuriedSettings(QString("Bluetooth"),QString("BtSwitchBtn"),QString("clicked"),status?QString("true"):QString("false"));
        BlueToothDBusService::setDefaultAdapterSwitchStatus(status);
    }
    else
        _BTServiceReportPowerSwitchFlag = false;

}

void BluetoothTopWindow::_BtSwitchBtnPressedSlot()
{
    m_SwitchBtnPressed = true;
}

void BluetoothTopWindow::_BtSwitchBtnReleasedSlot()
{
    if (m_SwitchBtnPressed) {
        KyWarning() << "active click";
        _BtSwitchBtn->disconnect(_BtSwitchBtn,NULL,this,NULL);
        connect(_BtSwitchBtn,SIGNAL(stateChanged(bool)),this,SLOT(_BtSwitchBtnSlot(bool)));

        _BtSwitchBtn->click();
        connect(_BtSwitchBtn, &KSwitchButton::pressed, this, &BluetoothTopWindow::_BtSwitchBtnPressedSlot);
        connect(_BtSwitchBtn, &KSwitchButton::released, this, &BluetoothTopWindow::_BtSwitchBtnReleasedSlot);
    }

    m_SwitchBtnPressed = false;
}

void BluetoothTopWindow::_BtTrayIconShowSlot(bool status)
{
    KyDebug();
    if(!_BTServiceReportTrayIconSwitchFlag)//服务上报的状态不再次下发
    {
        ukccbluetoothconfig::ukccBtBuriedSettings(QString("Bluetooth"),QString("BtTrayIconShow"),QString("clicked"),status?QString("true"):QString("false"));
        BlueToothDBusService::setTrayIconShowStatus(status);
    }
    else
        _BTServiceReportTrayIconSwitchFlag = false;
}

void BluetoothTopWindow::_BtDiscoverableSlot(bool status)
{
    KyDebug();
    if(!_BTServiceReportDiscoverableSwtichFlag)
    {
        ukccbluetoothconfig::ukccBtBuriedSettings(QString("Bluetooth"),QString("BtDiscoverable"),QString("clicked"),status?QString("true"):QString("false"));
        BlueToothDBusService::setDefaultAdapterDiscoverableStatus(status);
    }
    else
        _BTServiceReportDiscoverableSwtichFlag = false;

}

void BluetoothTopWindow::_BtAutoAudioConnBtnSlot(bool status)
{
    KyDebug();
    if(!_BTServiceReportAutoAudioConnSwtichFlag)
    {
        ukccbluetoothconfig::ukccBtBuriedSettings(QString("Bluetooth"),QString("BtAutoAudioConnBtn"),QString("clicked"),status?QString("true"):QString("false"));
        BlueToothDBusService::setAutoConnectAudioDevStatus(status);
    }
    else
        _BTServiceReportAutoAudioConnSwtichFlag = false;
}

void BluetoothTopWindow::_AdapterListSelectComboBoxSlot(int indx)
{
    KyDebug() << indx << BlueToothDBusService::m_bluetooth_adapter_address_list.size()  << BlueToothDBusService::m_bluetooth_adapter_address_list;
    //KyWarning() << indx << BlueToothDBusService::m_bluetooth_adapter_address_list.size() << BlueToothDBusService::m_bluetooth_adapter_address_list;
    ukccbluetoothconfig::ukccBtBuriedSettings(QString("Bluetooth"),QString("AdapterListSelectComboBox"),QString("settings"),QString::number(indx));

    if (BlueToothDBusService::m_bluetooth_adapter_address_list.size() > indx)
    {
        KyInfo() << BlueToothDBusService::m_bluetooth_adapter_address_list.at(indx);
        BlueToothDBusService::setDefaultAdapter(BlueToothDBusService::m_bluetooth_adapter_address_list.at(indx));
    }

}

void BluetoothTopWindow::adapterNameChangedSlot(QString name)
{
    KyDebug() << name ;
    _BtNameLabel->set_dev_name(name);
    _AdapterListSelectComboBox->setCurrentText(name);
}

void BluetoothTopWindow::adapterPowerStatusChangedSlot(bool status)
{
    KyDebug() << status ;

    KyWarning() << "_BTServiceReportPowerSwitchFlag:"<<  _BTServiceReportPowerSwitchFlag ;
    _BTServiceReportPowerSwitchFlag = true;
    KyWarning() << "_BtSwitchBtn->isChecked:"<<  _BtSwitchBtn->isChecked() ;
    if(status != _BtSwitchBtn->isChecked())
        _BtSwitchBtn->setChecked(status);
    else
        Q_EMIT _BtSwitchBtn->stateChanged(status);
}

void BluetoothTopWindow::adapterNameChangedOfIndexSlot(int indx , QString name)
{
    KyWarning() << "changed indx:" << indx
              << "changed name:" << name;

    if (_AdapterListSelectComboBox->count() > indx)
        _AdapterListSelectComboBox->setItemText(indx,name);
}

void BluetoothTopWindow::adapterTrayIconSlot(bool status)
{
    KyDebug();

    _BTServiceReportTrayIconSwitchFlag = true;
    if(status != _BtTrayIconShow->isChecked())
        _BtTrayIconShow->setChecked(status);
    else
        Q_EMIT _BtTrayIconShow->stateChanged(status);

}

void BluetoothTopWindow::adapterDiscoverableSlot(bool status)
{
    KyDebug();

    _BTServiceReportDiscoverableSwtichFlag = true;
    if(status != _BtDiscoverable->isChecked())
        _BtDiscoverable->setChecked(status);
    else
        Q_EMIT _BtDiscoverable->stateChanged(status);

}

void BluetoothTopWindow::adapterActiveConnectionSlot(bool status)
{
    KyDebug();
    _BTServiceReportAutoAudioConnSwtichFlag = true;

    if(status != _BtAutoAudioConnBtn->isChecked())
        _BtAutoAudioConnBtn->setChecked(status);
    else
        Q_EMIT _BtAutoAudioConnBtn->stateChanged(status);
}

void BluetoothTopWindow::adapterChangedRefreshInterface(int indx)
{
    KyDebug() << indx;
    if (!BlueToothDBusService::m_default_bluetooth_adapter)
    {
        KyWarning() << "m_default_bluetooth_adapter is nullptr!";
        return;
    }

    if(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterPower() != _BtSwitchBtn->isChecked())
        _BTServiceReportPowerSwitchFlag = true;

    if(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterTrayShow() != _BtTrayIconShow->isChecked())
        _BTServiceReportTrayIconSwitchFlag = true;

    if(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterDiscoverable() != _BtDiscoverable->isChecked())
        _BTServiceReportDiscoverableSwtichFlag = true;

    if(BlueToothDBusService::m_default_bluetooth_adapter->getAdapterAutoConn() != _BtAutoAudioConnBtn->isChecked())
        _BTServiceReportAutoAudioConnSwtichFlag = true;

    _AdapterListSelectComboBox->disconnect();
    InitDisplayState();
    _AdapterListSelectComboBox->setCurrentIndex(indx);
    connect(_AdapterListSelectComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(_AdapterListSelectComboBoxSlot(int)));

}

void BluetoothTopWindow::reloadWindow()
{
    if (m_btServer && m_btServer->m_default_bluetooth_adapter)
    {
        int indx = m_btServer->m_bluetooth_adapter_address_list.indexOf(m_btServer->m_default_bluetooth_adapter->getDevAddress()) ;
        adapterChangedRefreshInterface(indx);
    }
}

void BluetoothTopWindow::quitWindow()
{

}


bool BluetoothTopWindow::whetherNeedInfoUser()
{
    KyDebug();
    unsigned int bluetoothMouseAmount = 0;
    unsigned int bluetoothKeyBoardAmount = 0 ;

    if (BlueToothDBusService::m_default_bluetooth_adapter)
    {
        for (auto var : BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list)
        {
            if (!var->getRemoving() && var->isPaired() && var->isConnected())
            {
                if (bluetoothdevice::DEVICE_TYPE::mouse == var->getDevType())
                {
                    bluetoothMouseAmount += 1;
                }
                else if (bluetoothdevice::DEVICE_TYPE::keyboard == var->getDevType())
                {
                    bluetoothKeyBoardAmount += 1;
                }
                else
                    continue;
            }
        }
    }
    KyInfo() << "Connected mouse amount:" << bluetoothMouseAmount
             << "Connected KeyBoard amount:" << bluetoothKeyBoardAmount;

    if (bluetoothMouseAmount == 0 && bluetoothKeyBoardAmount == 0)
    {
        KyDebug() << "Not connected KeyBoard and Mouse";
        return false;
    }

    unsigned int systemMouseAndTouchPadAmount = Common::getSystemCurrentMouseAndTouchPadDevCount();
    unsigned int systemKeyBoardAmount = Common::getSystemCurrentKeyBoardDevCount();
    KyInfo() << "Connected bluetooth Mouse amount:" << bluetoothMouseAmount
             << "Connected bluetooth KeyBoard amount:" << bluetoothKeyBoardAmount
             << "System Mouse and TouchPad amount:" << systemMouseAndTouchPadAmount
             << "System KeyBoard amount:" << systemKeyBoardAmount;

    if ((bluetoothMouseAmount != 0) && (systemMouseAndTouchPadAmount == bluetoothMouseAmount))
    {
        return true;
    }

    if ((bluetoothKeyBoardAmount != 0) && (systemKeyBoardAmount == bluetoothKeyBoardAmount))
    {
        return true;
    }

    return false;
}
