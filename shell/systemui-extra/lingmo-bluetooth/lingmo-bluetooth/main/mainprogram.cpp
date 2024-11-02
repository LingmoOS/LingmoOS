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
#include "mainprogram.h"

MainProgram::MainProgram(bool env, QObject *parent) :
    QObject(parent)
{
    envPC = env;
    _sessionDbus = new BluetoothDbus(this);

    if (envPC == Environment::LAIKA || envPC == Environment::MAVIS)
        _mIntel = true;
    else
        _mIntel = false;

    _mConfig = new Config(this);

    initGSettingInfo();

    UI = TrayWidget::self(_sessionDbus, _mIntel);
    kdk::LingmoUIStyleHelper::self()->removeHeader(UI);

    errMsgWidget = new ErrorMessageWidget(UI);
    Config::setKdkGeometry(errMsgWidget->windowHandle(), errMsgWidget->width(), errMsgWidget->height(), true);
    errMsgWidget->show();
    Config::setKdkGeometry(errMsgWidget->windowHandle(), errMsgWidget->width(), errMsgWidget->height(), true);
    errMsgWidget->hide();

    replyTimer = new QTimer(this);

    if (!_mIntel)
        Config::setKdkGeometry(UI->windowHandle(), UI->width(), UI->height(), false);

    QDBusMessage dbusMsg =  QDBusMessage::createMethodCall(SYSTEM_ACTIVE_USER_DBUS,
                                                           SYSTEM_ACTIVE_USER_PATH,
                                                           SYSTEM_ACTIVE_USER_INTERFACE,
                                                           "GetActiveUser");
    QDBusMessage response = SYSTEM_CALL_METHOD(dbusMsg);

    if (response.type() == QDBusMessage::ReplyMessage)
        active_User = response.arguments().takeFirst().toString();
    else
        active_User = QString();
    current_User = QString(qgetenv("USER").toStdString().data());
    KyDebug() << active_User << current_User ;

    //非华为机器才存在文件功能
    if(envPC != Environment::HUAWEI)
    {
        connect(_sessionDbus, &BluetoothDbus::receiveFilesSignal, this, &MainProgram::receiveFileSlot);
        connect(_sessionDbus, &BluetoothDbus::sendTransferDeviceMesgSignal, this, &MainProgram::sendTransferDeviceMesgSlot);
        connect(_sessionDbus, &BluetoothDbus::sendTransferFilesMesgSignal, this, &MainProgram::sendTransferFilesMesgSlot);
    }

    connect(_sessionDbus, &BluetoothDbus::connectionErrorMsg, this, &MainProgram::connectionErrorSlot);
    connect(_sessionDbus, &BluetoothDbus::activeConnectionSignal, this, &MainProgram::activeConnectionSlot);
    connect(_sessionDbus, &BluetoothDbus::displayPasskeySignal, this, &MainProgram::displayPasskeySlot);
    connect(_sessionDbus, &BluetoothDbus::requestConfirmationSignal, this, &MainProgram::requestConfirmationSlot);
    connect(_sessionDbus, &BluetoothDbus::showTrayWidgetUISignal, this, [=]() {
        UI->showUI();
    });
    connect(_sessionDbus, &BluetoothDbus::activeUserChangedSignal, this, [=](QString activeUserName) {
        active_User = activeUserName;
    });
    connect(_sessionDbus, &BluetoothDbus::deviceAddSignal, this, [=](QMap<QString, QVariant> devAttr) {
       UI->layoutAddOnePairedDev(devAttr);
    });
    connect(errMsgWidget, &ErrorMessageWidget::messageBoxShowed, UI, &TrayWidget::setTrayIconClickEnable);
}

MainProgram::~MainProgram()
{
//    _sessionDbus->unregisterClient();
}

void MainProgram::initGSettingInfo()
{
    _mGsetting.clear();
    _mGsetting["switch"]                     = _mConfig->gsetting->get("switch");
    _mGsetting["finally-connect-the-device"] = _mConfig->gsetting->get("finally-connect-the-device");
    _mGsetting["adapter-address"]            = _mConfig->gsetting->get("adapter-address");
    _mGsetting["file-save-path"]             = _mConfig->gsetting->get("file-save-path");
    _mGsetting["adapter-address-list"]       = _mConfig->gsetting->get("adapter-address-list");

    if(_mConfig->gsetting->get("file-save-path").toString().isEmpty()){
        _mConfig->gsetting->set("file-save-path",QVariant::fromValue(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)));
    }

    connect(_mConfig->gsetting,SIGNAL(changed(QString)),this,SLOT(gsettingsChangedSlot(QString)));
}

void MainProgram::gsettingsChangedSlot(const QString &key)
{
    if (_mGsetting.contains("key")) {
        _mGsetting[key] = _mConfig->gsetting->get(key);
    }
}

void MainProgram::sendFiles(QString address, QStringList files) {
    _sessionDbus->sendFiles(address, files);
}

void MainProgram::receiveFileSlot(QString address, QString devname,
                                  QString filename, QString type, quint64 size)
{
    if (active_User != current_User)
        return;
    KyDebug();
    receiving_widget = new FileReceivingPopupWidget(address, devname, filename, type, size);
    KyDebug();
    connect(_sessionDbus,&BluetoothDbus::statusChangedSignal,receiving_widget,&FileReceivingPopupWidget::statusChangedSlot);
    connect(receiving_widget,&FileReceivingPopupWidget::accepted,this,[=]{
        KyDebug() << Q_FUNC_INFO <<  __LINE__;
        _sessionDbus->replyFileReceiving(address, true);
        connect(receiving_widget,&FileReceivingPopupWidget::cancel,this,[=]{
            _sessionDbus->cancelFileTransfer(address, 1);
            receiving_widget->deleteLater();
            receiving_widget = nullptr;
            KyDebug() << Q_FUNC_INFO << "cancel" <<  __LINE__;
        });
    });

    connect(receiving_widget,&FileReceivingPopupWidget::rejected,this,[=]{
        _sessionDbus->replyFileReceiving(address, false);
        receiving_widget->deleteLater();
        receiving_widget = nullptr;
    });

//    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//        kdk::LingmoUIStyleHelper::self()->removeHeader(receiving_widget);
//    }
    receiving_widget->show();
    receiving_widget->activateWindow();
    Config::setKdkGeometry(receiving_widget->windowHandle(),
                   receiving_widget->width(),
                   receiving_widget->height(),
                   true);
}

void MainProgram::sendTransferDeviceMesgSlot(QString address)
{
    if (active_User != current_User)
        return;
    selectedFiles.clear();
    KyFileDialog fd;
    QList<QUrl> list = fd.sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
             continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(&fd);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd,
    [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path) {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                       QFileInfo fi = wfilist.at(i);
                 //华为990、9a0需要屏蔽最小系统挂载的目录
                  if (fi.fileName() == "2691-6AB8")
                       continue;
                 mntUrlList << QUrl("file://" + fi.filePath());
             }
             fd.setSidebarUrls(list + mntUrlList);
             fd.update();
        });

        connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd]() {
            fd.setSidebarUrls(list);
        });

    //自己QFileDialog的用法，这里只是列子
    //fd.setNameFilter(QLatin1String("All Files (*)"));

    fd.setDirectory(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0));

    //这行要添加，设置左侧导航
    fd.setSidebarUrls(list + mntUrlList);
    fd.show();
    Config::setKdkGeometry(fd.windowHandle(),
                   fd.width(),
                   fd.height(),
                   true);
    if (fd.exec() == QDialog::Accepted) {
        KyDebug()<<fd.selectedFiles()<<fd.filter()<<fd.selectedNameFilter() << fd.selectedUrls();
        selectedFiles = fd.selectedFiles();
    }

    if(selectedFiles.size() != 0){
        if (BluetoothFileTransferWidget::isShow == false) {
            fileSendWidget = new BluetoothFileTransferWidget(selectedFiles,_sessionDbus->getSendableDevices(),address);
            connect(fileSendWidget,&BluetoothFileTransferWidget::sender_dev_name,this,&MainProgram::sendFiles);
            connect(fileSendWidget,&BluetoothFileTransferWidget::stopSendFile,this,[=](QString address){
                _sessionDbus->cancelFileTransfer(address, 0);
            });
            connect(_sessionDbus,&BluetoothDbus::statusChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::statusChangedSlot);
            connect(_sessionDbus,&BluetoothDbus::powerChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::powerChanged);
            connect(_sessionDbus,&BluetoothDbus::devAttrChanged, fileSendWidget, &BluetoothFileTransferWidget::devattrChanged);

//            if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//                kdk::LingmoUIStyleHelper::self()->removeHeader(fileSendWidget);
//            }
            fileSendWidget->show();
            fileSendWidget->activateWindow();
            Config::setKdkGeometry(fileSendWidget->windowHandle(), fileSendWidget->width(), fileSendWidget->height(), true);
        }
        else
        {
            KyDebug() << Q_FUNC_INFO << fileSendWidget->get_send_data_state() << __LINE__;
            if (BluetoothFileTransferWidget::_SEND_FAILURE == fileSendWidget->get_send_data_state()   ||
                BluetoothFileTransferWidget::_SEND_COMPLETE == fileSendWidget->get_send_data_state()  )
            {
                fileSendWidget->close();
                delete fileSendWidget;
                fileSendWidget = new BluetoothFileTransferWidget(selectedFiles,_sessionDbus->getSendableDevices(),address);
                connect(fileSendWidget,&BluetoothFileTransferWidget::sender_dev_name,this,&MainProgram::sendFiles);
                connect(fileSendWidget,&BluetoothFileTransferWidget::stopSendFile,this,[=](QString address){
                    _sessionDbus->cancelFileTransfer(address, 0);
                });
                connect(_sessionDbus,&BluetoothDbus::statusChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::statusChangedSlot);
                connect(_sessionDbus,&BluetoothDbus::devAttrChanged, fileSendWidget, &BluetoothFileTransferWidget::devattrChanged);
                connect(_sessionDbus,&BluetoothDbus::powerChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::powerChanged);

//                if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//                    kdk::LingmoUIStyleHelper::self()->removeHeader(fileSendWidget);
//                }
                fileSendWidget->show();
                fileSendWidget->activateWindow();
                Config::setKdkGeometry(fileSendWidget->windowHandle(), fileSendWidget->width(), fileSendWidget->height(), true);
            } else {
                fileSendWidget->insertNewFileList(selectedFiles);
            }
        }
    }
}

void MainProgram::sendTransferFilesMesgSlot(QStringList targetList)
{
    if (active_User != current_User)
        return;
    KyDebug() << "Select file:" << targetList;
    //selectedFiles = targetList;
    selectedFiles.clear();
    for (int i = 0 ; i < targetList.size() ; i++)
    {
        QString str = targetList.at(i);
        str = str.replace(QString("file://"), QString(""));
        selectedFiles.append(str);
    }
    KyDebug() << "Select file:" << selectedFiles;
    if(selectedFiles.size() != 0){
        if (BluetoothFileTransferWidget::isShow == false) {
            fileSendWidget = new BluetoothFileTransferWidget(selectedFiles,_sessionDbus->getSendableDevices(),"");
            connect(fileSendWidget,&BluetoothFileTransferWidget::sender_dev_name,this,&MainProgram::sendFiles);
            connect(fileSendWidget,&BluetoothFileTransferWidget::stopSendFile,this,[=](QString address){
                _sessionDbus->cancelFileTransfer(address, 0);
            });
            connect(_sessionDbus,&BluetoothDbus::statusChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::statusChangedSlot);
            connect(_sessionDbus,&BluetoothDbus::devAttrChanged, fileSendWidget, &BluetoothFileTransferWidget::devattrChanged);
            connect(_sessionDbus,&BluetoothDbus::powerChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::powerChanged);
           
//            if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//                kdk::LingmoUIStyleHelper::self()->removeHeader(fileSendWidget);
//            }
            fileSendWidget->show();
            fileSendWidget->activateWindow();
            Config::setKdkGeometry(fileSendWidget->windowHandle(), fileSendWidget->width(), fileSendWidget->height(), true);
        }
        else
        {
            if (BluetoothFileTransferWidget::_SEND_FAILURE == fileSendWidget->get_send_data_state()   ||
                BluetoothFileTransferWidget::_SEND_COMPLETE == fileSendWidget->get_send_data_state()  )
            {
                fileSendWidget->close();
                delete fileSendWidget;
                fileSendWidget = new BluetoothFileTransferWidget(selectedFiles,_sessionDbus->getSendableDevices(),"");
                connect(fileSendWidget,&BluetoothFileTransferWidget::sender_dev_name,this,&MainProgram::sendFiles);
                connect(fileSendWidget,&BluetoothFileTransferWidget::stopSendFile,this,[=](QString address){
                    _sessionDbus->cancelFileTransfer(address, 0);
                });
                connect(_sessionDbus,&BluetoothDbus::statusChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::statusChangedSlot);
                connect(_sessionDbus,&BluetoothDbus::devAttrChanged, fileSendWidget, &BluetoothFileTransferWidget::devattrChanged);
                connect(_sessionDbus,&BluetoothDbus::powerChangedSignal,fileSendWidget,&BluetoothFileTransferWidget::powerChanged);
                
//                if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//                    kdk::LingmoUIStyleHelper::self()->removeHeader(fileSendWidget);
//                }
                fileSendWidget->show();
                fileSendWidget->activateWindow();
                Config::setKdkGeometry(fileSendWidget->windowHandle(), fileSendWidget->width(), fileSendWidget->height(), true);
            } else {
                fileSendWidget->insertNewFileList(selectedFiles);
            }
        }
    }
}

void MainProgram::displayPasskeySlot(QString dev, QString name, QString passkey)
{
    if (active_User != current_User)
        return;
    KyDebug() << Q_FUNC_INFO << dev <<  name << passkey;
    if(m_Passkey == passkey)
        return;

    if(Keypincodewidget != nullptr){
        if (Keypincodewidget) {
            Keypincodewidget->updateUIInfo(name,passkey);
            return;
        }
    }

    Keypincodewidget = new PinCodeWidget(name,passkey,false, UI);
    QWindow *wi = Keypincodewidget->windowHandle();
    wi->setTransientParent(UI->windowHandle());
    connect(Keypincodewidget, &PinCodeWidget::msgShowedOver, this, [=] {
       Keypincodewidget->deleteLater();
       Keypincodewidget = nullptr;
    });

    disconnect(_sessionDbus, &BluetoothDbus::pairAgentCanceledSignal, nullptr, nullptr);

    connect(_sessionDbus, &BluetoothDbus::pairAgentCanceledSignal, this, [=](QString addr) {
        if(addr != QString("") && addr != dev)
            return;
        if(nullptr != Keypincodewidget)
            Keypincodewidget->pairFailureShow();
        disconnect(_sessionDbus, &BluetoothDbus::pairAgentCanceledSignal, nullptr, nullptr);
    });
    //保持在最前
    const KWindowInfo info(Keypincodewidget->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(Keypincodewidget->winId(), NET::SkipTaskbar | NET::SkipPager);
//    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//        kdk::LingmoUIStyleHelper::self()->removeHeader(Keypincodewidget);
//    }
    Keypincodewidget->show();
    Keypincodewidget->activateWindow();

    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(Keypincodewidget->winId(), NET::SkipTaskbar | NET::SkipPager);

    Config::setKdkGeometry(Keypincodewidget->windowHandle(),
                   Keypincodewidget->width(),
                   Keypincodewidget->height(),
                   true);
}

void MainProgram::requestConfirmationSlot(QString dev, QString name, QString passkey)
{
    if (active_User != current_User)
        return;

    KyDebug() << Q_FUNC_INFO << dev << name << passkey;

    if(pincodewidget != nullptr){
        KyDebug();
        return;
    }
    m_Passkey = passkey;
    pincodewidget = new PinCodeWidget(name,passkey, true, UI);
    QWindow *wi = pincodewidget->windowHandle();
    wi->setTransientParent(UI->windowHandle());
    connect(pincodewidget, &PinCodeWidget::msgShowedOver, this, [=] {
        if (pincodewidget != nullptr) {
            KyDebug();
            pincodewidget->deleteLater();
            pincodewidget = nullptr;
       }
    });
    connect(_sessionDbus, &BluetoothDbus::pairAgentCanceledSignal, this, [=](QString addr) {
        KyDebug();
        if(addr != QString("") && addr != dev)
            return;
        if (pincodewidget != nullptr) {
            if (pincodewidget->canceled)
                return;
            pincodewidget->canceled =  true;
            pincodewidget->pairFailureShow();
            pincodewidget = nullptr;
        }
    });

    connect(pincodewidget,&PinCodeWidget::accepted,this,[=]{
        _sessionDbus->pairFuncReply(dev, true);
    });

    connect(pincodewidget,&PinCodeWidget::rejected,this,[=]{
        _sessionDbus->pairFuncReply(dev, false);
    });
    connect(pincodewidget, &PinCodeWidget::destroyed, this, [=] {
        pincodewidget = nullptr;
    });

    //保持在最前
    const KWindowInfo info(pincodewidget->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(pincodewidget->winId(), NET::SkipTaskbar | NET::SkipPager);
//    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//        kdk::LingmoUIStyleHelper::self()->removeHeader(pincodewidget);
//    }
    pincodewidget->show();
    pincodewidget->activateWindow();

    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager))
        KWindowSystem::setState(pincodewidget->winId(), NET::SkipTaskbar | NET::SkipPager);

    Config::setKdkGeometry(pincodewidget->windowHandle(),
                   pincodewidget->width(),
                   pincodewidget->height(),
                   true);
}

void MainProgram::activeConnectionSlot(QString address, QString name, QString type, int rssi, int timeout) {
    if (active_User != current_User)
        return;
    if (activeConnectionWidget != nullptr)
        activeConnectionWidget->deleteLater();
    activeConnectionWidget = new ActiveConnectionWidget(address, name, type, rssi);

    disconnect(_sessionDbus, &BluetoothDbus::adapterAutoConnChanged, nullptr, nullptr);
    connect(_sessionDbus, &BluetoothDbus::adapterAutoConnChanged,
            activeConnectionWidget, &ActiveConnectionWidget::autoConnChanged);

    connect(activeConnectionWidget, &ActiveConnectionWidget::replyActiveConnection, this, [=](QString dev, bool v) {
        if (replyTimer->isActive())
            replyTimer->stop();
        _sessionDbus->activeConnectionReply(dev, v);
        disconnect(activeConnectionWidget, &ActiveConnectionWidget::replyActiveConnection, NULL, NULL);
        activeConnectionWidget = nullptr;
    });

    replyTimer->disconnect();
    connect(replyTimer, &QTimer::timeout, this, [=](){
        if (nullptr == activeConnectionWidget)
            return;
        disconnect(activeConnectionWidget, &ActiveConnectionWidget::replyActiveConnection, NULL, NULL);
        activeConnectionWidget->hide();
        activeConnectionWidget->deleteLater();
        activeConnectionWidget = nullptr;
    });

    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
        kdk::LingmoUIStyleHelper::self()->removeHeader(activeConnectionWidget);
    }
    activeConnectionWidget->show();
    activeConnectionWidget->activateWindow();
    Config::setKdkGeometry(activeConnectionWidget->windowHandle(),
                   activeConnectionWidget->width(),
                   activeConnectionWidget->height(),
                   false);
    replyTimer->start(timeout * 1000);
}

void MainProgram::connectionErrorSlot(QString dev, int errCode) {
    if (active_User != current_User)
        return;
    KyDebug();
    errMsgWidget->ErrCodeAnalysis(dev, errCode);
}
