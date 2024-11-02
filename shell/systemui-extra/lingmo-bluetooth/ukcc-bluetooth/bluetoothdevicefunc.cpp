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

#include "bluetoothdevicefunc.h"

#define RADIUS 6.0

bluetoothdevicefunc::bluetoothdevicefunc(QWidget *parent ,QString dev_address):
    QPushButton(parent),
    _MDev_addr(dev_address)
{
    initGsettings();
    initInterface();
}

bluetoothdevicefunc::~bluetoothdevicefunc()
{
    KyDebug() <<_MDev_addr ;
    _mStyle_GSettings->deleteLater();
}

void bluetoothdevicefunc::initGsettings()
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

        connect(_mStyle_GSettings,&QGSettings::changed,this,&bluetoothdevicefunc::mStyle_GSettingsSlot);
    }
}
void bluetoothdevicefunc::initBackground()
{
    this->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);
}

void bluetoothdevicefunc::initInterface()
{
    KyDebug();
    this->setFixedSize(36,36);
//    if (_themeIsBlack)
    initBackground();

    devMenuFunc = new QMenu(this);
    devMenuFunc->setMinimumWidth(160);
    //devMenuFunc->setMaximumWidth(160);
    connect(devMenuFunc,&QMenu::triggered,this,&bluetoothdevicefunc::MenuSignalDeviceFunction);
    connect(devMenuFunc,&QMenu::aboutToHide,this,&bluetoothdevicefunc::MenuSignalAboutToHide);
}

void bluetoothdevicefunc::MenuSignalDeviceFunction(QAction *action)
{
    KyDebug() << action->text();

    if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr))
    {
        KyWarning() << " bluetooth device is null";
        return;
    }

    if(action->text() == tr("SendFile"))
    {
        BlueToothDBusService::sendFiles(_MDev_addr);
    }
    else if(action->text() == tr("Remove"))
    {
        DevRemoveDialog::REMOVE_INTERFACE_TYPE mode ;
        if (bluetoothdevice::DEVICE_TYPE::phone == BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevType() ||
            bluetoothdevice::DEVICE_TYPE::computer == BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevType())
            mode = DevRemoveDialog::REMOVE_HAS_PIN_DEV;
        else
            mode = DevRemoveDialog::REMOVE_NO_PIN_DEV;
        showDeviceRemoveWidget(mode);
    }
    else if (action->text() == tr("Connect"))
    {
        KyDebug() << "To :" << _MDev_addr << "connect" ;
        if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr))
        {

            BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->setDevConnecting(true);
        }

        BlueToothDBusService::devConnect(_MDev_addr);
    }
    else if(action->text() == tr("Disconnect"))
    {
        KyDebug() << "To :" <<_MDev_addr << "disconnect" ;
        BlueToothDBusService::devDisconnect(_MDev_addr);
    }
    else if(action->text() == tr("Rename"))
    {
        KyDebug() << "To :" << _MDev_addr << "disconnect" ;
        showDeviceRenameWidget();
    }

//    emit devFuncOpertionSignal();
}

void bluetoothdevicefunc::MenuSignalAboutToHide()
{
    //KyWarning();
}

void bluetoothdevicefunc::showDeviceRemoveWidget(DevRemoveDialog::REMOVE_INTERFACE_TYPE mode)
{
    DevRemoveDialog *mesgBox = new DevRemoveDialog(mode,this);
    mesgBox->setModal(true);
    mesgBox->setDialogText(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevInterfaceShowName());

    connect(mesgBox,&DevRemoveDialog::accepted,this,[=]{
       KyDebug() << "To :" << _MDev_addr << "Remove" ;
       BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->setRemoving(true);
       BlueToothDBusService::devRemove(_MDev_addr);
       Q_EMIT devFuncOpertionRemoveSignal(_MDev_addr);
    });

    QDBusInterface iface("org.lingmo.panel",
                         "/panel/position",
                         "org.lingmo.panel",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QDBusReply<QVariantList> reply=iface.call("GetPrimaryScreenGeometry");
        kdk::WindowManager::setGeometry(mesgBox->windowHandle(),
                                        QRect((reply.value().at(2).toInt()-mesgBox->width())/2,
                                        (reply.value().at(3).toInt()-mesgBox->height())/2,
                                        mesgBox->width(), mesgBox->height()));
    }
    mesgBox->exec();
}

void bluetoothdevicefunc::showDeviceRenameWidget()
{
    KyDebug();

    DevRenameDialog *renameMesgBox = new DevRenameDialog(this);
    renameMesgBox->setDevName(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevInterfaceShowName());
    renameMesgBox->setRenameInterface(DevRenameDialog::DEVRENAMEDIALOG_BT_DEVICE);

    connect(renameMesgBox,&DevRenameDialog::nameChanged,this,[=](QString name){

        BlueToothDBusService::devRename(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevAddress(),name);

    });
    renameMesgBox->exec();

    KyDebug() << "end" ;
}

void bluetoothdevicefunc::mStyle_GSettingsSlot(const QString &key)
{
    KyDebug() << key;
    if ("iconThemeName" == key || "icon-theme-name" == key)
    {
        _mIconThemeName = _mStyle_GSettings->get("iconThemeName").toString();
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

    }
//    initBackground();
    this->update();

}

void bluetoothdevicefunc::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(this->palette().base().color());

    QPalette pal = qApp->palette();
    QColor color = pal.color(QPalette::Button);
    color.setAlphaF(0.6);
    pal.setColor(QPalette::Button, color);
    this->setPalette(pal);

    QPushButton::paintEvent(event);
}

void bluetoothdevicefunc::mousePressEvent(QMouseEvent *event)
{
//    KyWarning();
    //获取当前时间
    _pressCurrentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QPushButton::mousePressEvent(event);
}


void bluetoothdevicefunc:: mouseReleaseEvent(QMouseEvent *event)
{
//    KyWarning();

    if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(_MDev_addr))
        return;
    long long _releaseCurrentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //KyDebug() << "_releaseCurrentTime" << _releaseCurrentTime << "_pressCurrentTime:" << _pressCurrentTime;
    //点击超时取消操作
    if((_releaseCurrentTime - _pressCurrentTime) <= 300)
    {
        // 显示顺序
        /*
         * 断开或连接
         * 发送文件
         * 重命名设备
         * 移除
         */

        devMenuFunc->clear();
        if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected())
        {
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevType() !=  bluetoothdevice::DEVICE_TYPE::mouse  &&
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevType() !=  bluetoothdevice::DEVICE_TYPE::keyboard)
            {
                QAction * connAction = new QAction(devMenuFunc);
                connAction->setText(tr("Connect"));
                devMenuFunc->addAction(connAction);
            }
        }
        else
        {
            QAction * disconnAction = new QAction(devMenuFunc);
            disconnAction->setText(tr("Disconnect"));
            devMenuFunc->addAction(disconnAction);
        }

        if ((bluetoothdevice::DEVICE_TYPE::phone    == BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevType() ||
             bluetoothdevice::DEVICE_TYPE::computer == BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevType()) &&
             BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->getDevSendFileMark() &&
             HUAWEI != envPC)
        {
            QAction * sendFileAction = new QAction(devMenuFunc);
            sendFileAction->setText(tr("SendFile"));
            devMenuFunc->addAction(sendFileAction);
        }

        QAction * renameAction = new QAction(devMenuFunc);
        renameAction->setText(tr("Rename"));
        devMenuFunc->addAction(renameAction);
        renameAction->setEnabled(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[_MDev_addr]->isConnected());

        QAction * removeAction = new QAction(devMenuFunc);
        removeAction->setText(tr("Remove"));
        devMenuFunc->addAction(removeAction);


        //计算显示位置
        QPoint currentWPoint = this->pos();
        QPoint sreenPoint = QWidget::mapFromGlobal(currentWPoint);

//        qInfo() << this->x() << this->y() << "======x ======y";
//        qInfo() << sreenPoint.x() << sreenPoint.y() << "======sreenPoint.x ======sreenPoint.y";
//        qInfo() << devMenuFunc->width() << this->width() << this->height() << "======devMenuFunc->width ======this->width ======this->height";
//        qInfo() << (-1)*sreenPoint.x()+devMenuFunc->width()+this->width()+4<<(-1)*sreenPoint.y()+this->height();

        //devMenuFunc->move( this->x()+devMenuFunc->width()+this->width(),this->y()+devMenuFunc->height()+this->height());
        devMenuFunc->move((-1)*sreenPoint.x()+this->x()-2,(-1)*sreenPoint.y()+this->height()+2);
//        devMenuFunc->setFocus();
        devMenuFunc->exec();
    }

    QPushButton::mouseReleaseEvent(event);
    emit devBtnReleaseSignal();
}
