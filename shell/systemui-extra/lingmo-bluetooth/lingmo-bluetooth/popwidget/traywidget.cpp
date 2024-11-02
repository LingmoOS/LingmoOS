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

#include "traywidget.h"

TrayWidget* TrayWidget::_MSelf = nullptr;
TrayWidget::TrayWidget(bool intel, BluetoothDbus *dBus, QWidget *parent):
    QWidget(parent),
    _SessionDBus(dBus),
    _config(new Config(this)),
    _Intel(intel)
{
    KyDebug();

//    窗管适配
    setProperty("useStyleWindowManager", false);
    if (!_Intel) {
        setProperty("useSystemStyleBlur", true);
        this->setAttribute(Qt::WA_TranslucentBackground);
//        this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    }
//    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
//        this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
//    } else {
//        MotifWmHints hints;
//        hints.flags = MWM_HINTS_DECORATIONS;
//        hints.decorations = MWM_DECOR_BORDER;
//        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
//    }
    initPanelGSettings();

    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager) || !info.hasState(NET::SkipSwitcher))
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    installEventFilter(this);
    this->setFocusPolicy(Qt::NoFocus);

    trayicon = new TrayIcon(_SessionDBus->getTrayIconShow(), _SessionDBus->getExistAdapter(), this);
    connect(_SessionDBus, &BluetoothDbus::existAdapter, trayicon, &TrayIcon::setAdapterExist);
    connect(this, &TrayWidget::trayIconStatusChanged, trayicon, &TrayIcon::SetTrayIcon);
    connect(trayicon, &TrayIcon::showTrayWidget, this, &TrayWidget::showUI);
    connect(trayicon, &TrayIcon::hideTrayWidget, this, &TrayWidget::hideUI);
    connect(trayicon, &TrayIcon::openBluetoothSettings, this, [=]{
         _SessionDBus->openBluetoothSettings();
    });
    connect(_SessionDBus, &BluetoothDbus::showTrayIcon, this, [=](bool show) {
        trayicon->SetAdapterFlag(show);
        if (!show)
            this->setVisible(show);
    });
    
    errMsgDialog = new ErrorMessageWidget();
    connect(errMsgDialog, &ErrorMessageWidget::closeInfoDialogConfirmed, this, &TrayWidget::reBindMBtn);

    connect(QApplication::desktop(), &QDesktopWidget::resized, this, [=] {
        Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(), false);
    });
    connect(QApplication::desktop(), &QDesktopWidget::primaryScreenChanged, this, [=] {
        Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(), false);
    });

    QGSettings *setttt;
    if (QGSettings::isSchemaInstalled("org.lingmo.SettingsDaemon.plugins.xsettings"))
        setttt = new QGSettings(QString("org.lingmo.SettingsDaemon.plugins.xsettings").toLocal8Bit());
    connect(setttt,&QGSettings::changed, this, [=] (const QString &key) {
        if (key == "scalingFactor") {
            repaint();
        }
    });

    if(QGSettings::isSchemaInstalled("org.lingmo.style")) {
        StyleSettings = new QGSettings("org.lingmo.style");
        connect(StyleSettings,&QGSettings::changed,this,&TrayWidget::GSettingsChanges);
    }

    const QByteArray transparency_id(TRANSPARENCY_SETTINGS);
    if(QGSettings::isSchemaInstalled(transparency_id)) {
        transparency_gsettings = new QGSettings(transparency_id);
    }

    if(QGSettings::isSchemaInstalled(transparency_id)) {
        getTransparency();
        connect(transparency_gsettings,&QGSettings::changed,this,[=](QString key) {
            KyDebug() << key;
            if (key == QString(TRANSPARENCY_KEY)) {
                getTransparency();
                this->update();
            }
        });
    }

    this->setFixedSize(420,520);
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("bluetooth"));
    pairedDevices = _SessionDBus->getPairedDevices();
    InitMemberVariables();
    connectUISignalAndSlot();
    layoutAddAllPairedDev();
    if (_Intel) {
        QRect availableGeometry = QApplication::screens().at(0)->geometry();
        KyDebug() << availableGeometry.x() << availableGeometry.y();
        manualMove++;
        move((availableGeometry.x() + (availableGeometry.width() - width())/2), (availableGeometry.y() + (availableGeometry.height() - height())/2));
    }
    emit trayIconStatusChanged(_SessionDBus->isPowered());
    _userAction = false;
    openBluetoothSlot(_SessionDBus->isPowered());
}

TrayWidget *TrayWidget::self( BluetoothDbus *dbus, bool intel)
{
    KyDebug();
    if (!_MSelf) {
        _MSelf = new TrayWidget(intel, dbus, nullptr);
    }
    return _MSelf;
}

TrayWidget::~TrayWidget()
{
    if (nullptr != m_panelGSettings) {
        delete m_panelGSettings;
        m_panelGSettings = nullptr;
    }
}

void TrayWidget::getTransparency() {
    double proportion = 1;
    if(QGSettings::isSchemaInstalled(TRANSPARENCY_SETTINGS))
        proportion = transparency_gsettings->get(TRANSPARENCY_KEY).toDouble();
    proportion = ((proportion > 0.85) ? 1 : (proportion + 0.15));
    KyDebug() << proportion;
    tran = proportion * 255;
}

void TrayWidget::openBluetoothSlot(bool v)
{
    KyDebug() << v <<__LINE__;
    emit trayIconStatusChanged(v);

    if (v != _SessionDBus->isPowered())
        _SessionDBus->setDefaultAdapterPower(v);

    if (!_Intel && !pairedDevices.size())
        return;
    if (_Intel) {
        if (v) {
            _MStackedWidget->setCurrentIndex(0);
        } else {
            _MStackedWidget->setCurrentIndex(1);
        }
        return;
    }
    _MStackedWidget->setVisible(v);
    lineFrame->setVisible(v);
    if (_MSelf) {
        _MSelf->setFixedHeight(v ? selfHeight : 102);
        if (QLocale::system().name() == "bo_CN")
            _NomalWidget->setFixedHeight(60 + selfHeight - 184);
        else
            _NomalWidget->setFixedHeight((pairedDevices.size()>= 7) ? (50 + selfHeight - 160) : (55 + selfHeight - 160));
    }
    Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(), false);
    update();
}

void TrayWidget::reBindMBtn(bool confirm) {
    manualSwitch = false;
    if (!confirm) {
        _MBtn->disconnect(_MBtn,&KSwitchButton::stateChanged,this,NULL);
        _MBtn->setChecked(false);
        openBluetoothSlot(false);
        connect(_MBtn, &KSwitchButton::stateChanged, this, &TrayWidget::manualOpenBt);
    }
}

void TrayWidget::manualOpenBt(bool open) {
    if (!open) {
        if (whetherNeedInfoUser())
        {
            _MBtn->disconnect(_MBtn,&KSwitchButton::stateChanged,this,NULL);
            _MBtn->setChecked(true);
            connect(_MBtn, &KSwitchButton::stateChanged, this, &TrayWidget::manualOpenBt);
            errMsgDialog->showCloseInfoDialog();
            return;
        }
    }
    openBluetoothSlot(open);
}

void TrayWidget::GSettingsChanges(const QString &key)
{
    KyDebug() << key;
    if(key == "styleName"){
        if(StyleSettings->get("style-name").toString() == "lingmo-default"){
            _MBtn->setTranslucent(true);
            QPalette palette = this->palette();
            palette.setColor(QPalette::BrightText,QColor(255,255,255));
            _MBtn->setPalette(palette);
        } else {
            _MBtn->setTranslucent(true);
            QPalette switchButton_pl;
            switchButton_pl.setColor(QPalette::ButtonText, this->palette().color(QPalette::Active, QPalette::ButtonText));
            switchButton_pl.setColor(QPalette::Button, this->palette().color(QPalette::Active, QPalette::Button));
            _MBtn->setPalette(switchButton_pl);
        }
        QPalette _pl;
        if(StyleSettings->get("style-name").toString() == "lingmo-default"){
            _pl.setColor(QPalette::WindowText, Qt::white);
        } else
            _pl.setColor(QPalette::WindowText, this->palette().color(QPalette::Active, QPalette::Text));
        _MLabel->setPalette(_pl);
        this->update();
        trayicon->updateTrayiconMenuStyle();
    }
    if (key == "switch") {
        if (_config.gsetting->get("switch").toString() != _SessionDBus->isPowered())
            _MBtn->setChecked(_config.gsetting->get("switch").toBool());
    }
}

void TrayWidget::InitMemberVariables()
{
    _MBtn           = new KSwitchButton(this);
    _MLoad          = new QLabel(this);
    _MStackedWidget = new QStackedWidget(this);
    _MLabel         = new QLabel(this);
    _MMainLayout    = new QVBoxLayout(this);
    _MSettingBtn    = new BluetoothSettingLabel(this);

    _MLoad->setVisible(false);
    _MBtn->setChecked(_SessionDBus->isPowered());
    _MBtn->setFocusPolicy(Qt::NoFocus);
    if(StyleSettings->get("style-name").toString() == "lingmo-default"){
        _MBtn->setTranslucent(true);
        QPalette palette = this->palette();
        palette.setColor(QPalette::BrightText,QColor(255,255,255));
    //    palette.setColor(QPalette::ButtonText,QColor(0,255,0));
    //    palette.setColor(QPalette::Button,QColor(0,0,255));
        _MBtn->setPalette(palette);
    } else {
        _MBtn->setTranslucent(true);
    }

    connect(_MSettingBtn, &BluetoothSettingLabel::clickSignal, this, [=]{
        _SessionDBus->openBluetoothSettings();
    });
    connect(_MSettingBtn,&BluetoothSettingLabel::hideFeature,this,[=]{
        _MSelf->hide();
    });
    connect(_MBtn, &KSwitchButton::stateChanged, this,&TrayWidget::manualOpenBt);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(24,16,25,12);
    topLayout->setSpacing(0);

    InitErrorWidget();
    InitNomalWidget();

    _MStackedWidget->setFixedSize(420,445);
    _MStackedWidget->addWidget(_NomalWidget);
    if (_Intel)
        _MStackedWidget->addWidget(_ErrorWidget);
    _MStackedWidget->setCurrentIndex(0);

    if(QGSettings::isSchemaInstalled("org.lingmo.style")
            && StyleSettings->get("style-name").toString() == "lingmo-default"){
        QPalette _pl;
        _pl.setColor(QPalette::WindowText, Qt::white);
        _MLabel->setPalette(_pl);
    }
    _MLabel->setText(tr("Bluetooth"));

    _MMainLayout->setMargin(0);
    _MMainLayout->setContentsMargins(0,0,0,0);
    _MMainLayout->setSpacing(0);

    topLayout->addWidget(_MLabel);
    topLayout->addStretch(1);
    topLayout->addWidget(_MBtn);
    topLayout->addWidget(_MLoad);

    lineFrame = new KyHLine(this);
    lineFrame2 = new KyHLine(this);

    _MMainLayout->addLayout(topLayout);
    _MMainLayout->addWidget(lineFrame2);
    _MMainLayout->addWidget(_MStackedWidget);
    _MMainLayout->addWidget(lineFrame);
    _MMainLayout->addWidget(_MSettingBtn);
}

void TrayWidget::InitNomalWidget()
{
    _NomalWidget    = new QWidget(_MStackedWidget);
    _PairedWidget   = new QWidget(_NomalWidget);
    _PairedLayout   = new QVBoxLayout(_PairedWidget);
    _ScrollArea     = new QScrollArea(_NomalWidget);
    QVBoxLayout *_ErrorWidgetLayout = new QVBoxLayout(_NomalWidget);
    QVBoxLayout *_ErrorWidgetChildLayout = new QVBoxLayout(_NomalWidget);
    QLabel *_ErrorWidgetTipLabel = new QLabel(_NomalWidget);

    _ErrorWidgetLayout->setAlignment(0);
    _ErrorWidgetLayout->setContentsMargins(8,12,8,0);
    _ErrorWidgetLayout->setSpacing(5);

    _ErrorWidgetChildLayout->setAlignment(0);
    _ErrorWidgetChildLayout->setContentsMargins(16,0,0,0);

    if (QLocale::system().name() == "bo_CN")
        _ErrorWidgetTipLabel->setFixedSize(420,38);
    else
        _ErrorWidgetTipLabel->setFixedSize(420,25);
    _ErrorWidgetTipLabel->setText(tr("My Device"));
    QPalette pl;
    pl.setColor(QPalette::WindowText, QColor("#818181"));
    _ErrorWidgetTipLabel->setPalette(pl);

    _NomalWidget->setFixedWidth(420);
//    _PairedWidget->setAttribute(Qt::WA_TranslucentBackground);

    //统一滑动模块的样式
    _ScrollArea->setAttribute(Qt::WA_TranslucentBackground);
    _ScrollArea->setProperty("drawScrollBarGroove",false);
    _ScrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);

    _ScrollArea->setFixedWidth(420);
    _ScrollArea->setWidgetResizable(true);
    _ScrollArea->setAlignment(Qt::AlignCenter);
    _ScrollArea->setWidget(_PairedWidget);
    _ScrollArea->setFrameShape(QFrame::NoFrame);
    _ScrollArea->horizontalScrollBar()->hide();
    _ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _ScrollArea->setPalette(QColor(Qt::transparent));
    _ScrollArea->setFocusPolicy(Qt::NoFocus);

    _PairedLayout->setAlignment(0);
    _PairedLayout->setContentsMargins(0,0,0,0);
    _PairedLayout->setSpacing(2);

    QDBusInterface *m_interface = new QDBusInterface("com.lingmo.statusmanager.interface","/",
                                        "com.lingmo.statusmanager.interface",
                                        QDBusConnection::sessionBus(),this);

    if (m_interface->isValid()) {
        QDBusReply<bool> status = m_interface->call("get_current_tabletmode");
        if (status) {
            _ErrorWidgetLayout->setContentsMargins(8,12,8,0);
            _ScrollArea->setFixedWidth(430);
        }
    }

    _ErrorWidgetChildLayout->addWidget(_ErrorWidgetTipLabel);
    _ErrorWidgetLayout->addLayout(_ErrorWidgetChildLayout);
    _ErrorWidgetLayout->addWidget(_ScrollArea);
}

void TrayWidget::InitErrorWidget()
{
    _ErrorWidget       = new QWidget(_MStackedWidget);
    QHBoxLayout* _ErrorWidgeeLayout = new QHBoxLayout(_ErrorWidget);
    QLabel* _ErrorIcon  = new QLabel(_ErrorWidget);

    _ErrorWidget->setFixedSize(446,420);

    _ErrorWidgeeLayout->setAlignment(0);
    _ErrorWidgeeLayout->setContentsMargins(0,0,0,0);
    _ErrorWidgeeLayout->setSpacing(0);

    _ErrorIcon->setPixmap(QPixmap("/usr/share/lingmo-bluetooth/no-bluetooth.svg"));

    _ErrorWidgeeLayout->addWidget(_ErrorIcon,1,Qt::AlignCenter);
    if (!_Intel)
        _ErrorIcon->setVisible(false);
}

void TrayWidget::connectUISignalAndSlot()
{
    connect(_SessionDBus,&BluetoothDbus::devRemoveSignal,this,&TrayWidget::removePairedDevUIByAddress);
    connect(_SessionDBus,&BluetoothDbus::adapterAddSignal,this,&TrayWidget::updatePairedDev);
    connect(_SessionDBus,&BluetoothDbus::powerChangedSignal,this,[=](bool value){
        KyDebug() << value <<__LINE__;
        if (!_Intel)
            Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(), false);
        if (value != _MBtn->isChecked())
            _MBtn->setChecked(value);
        emit trayIconStatusChanged(value);
        _SessionDBus->InitDefaultAdapterDevicesAttr();
        updatePairedDev();
    });
    connect(_SessionDBus,&BluetoothDbus::defaultAdapterChanged,this,[=](){
        _MBtn->setChecked(_SessionDBus->isPowered());
        updatePairedDev();
    });
}

/************************************************
 * @brief  已配对的设备添加到布局中显示
 * @param  null
 * @return null
*************************************************/
void TrayWidget::addNewDevItem(QString addr) {
    QDevItem *dev = new QDevItem(addr, pairedDevices.value(addr), _PairedWidget);
    connect(_SessionDBus, &BluetoothDbus::devAttrChanged, dev, [=](QString address, QMap<QString, QVariant> devAttr) {
        if (address != addr)
            return;

        if (devAttr.contains("Connected") && dev->isConnected() != devAttr.value("Connected").toBool()) {
            if (devAttr.value("Connected").toBool()) {
                _PairedLayout->removeWidget(dev);
                _PairedLayout->insertWidget(0, dev);
            } else {
                _PairedLayout->removeWidget(dev);
                _PairedLayout->insertWidget(_SessionDBus->getOrderList().size() - 1, dev);
            }
        }

        if (dev)
            dev->attrChangedSlot(devAttr);
    });
//            connect(_SessionDBus,&BluetoothDbus::adapterCanOperat,dev,[=](bool value){
//                if (dev) {
//                    dev->setClickEnabled(value);
//                }
//            });
    connect(dev, &QDevItem::devConnect, this, [=](bool connectdev){
       if(connectdev) {
           _SessionDBus->devConnect(addr);
       } else {
           _SessionDBus->devDisconnect(addr);
       }
    });
    _PairedLayout->addWidget(dev);
}

void TrayWidget::layoutAddAllPairedDev()
{
    int count = 0;
    bool isNull = true;
    KyDebug() << pairedDevices.size() << __LINE__;
    QStringList firstOderList = _SessionDBus->getOrderList();
    if (!_PairedWidget->findChildren<QDevItem *>().size()) {
        for (auto item : firstOderList) {
            addNewDevItem(item);
            isNull = false;
            count++;
        }
        for (auto item : pairedDevices.keys()) {
            if (firstOderList.contains(item))
                continue;
            addNewDevItem(item);
            isNull = false;
            count++;
        }
        _PairedLayout->addStretch();
        if (_Intel)
            return;
        isNull = isNull || !_MBtn->isChecked();
        _MStackedWidget->setVisible(!isNull);
        lineFrame->setVisible(!isNull);
        if (_MSelf) {
            if (QLocale::system().name() == "bo_CN")
                selfHeight = 184 + count * 52;
            else
                selfHeight = 160 + count * 52;
            if (count >= 7)
                selfHeight = 515;
            _MSelf->setFixedHeight((isNull ? 102 : selfHeight));
            if (QLocale::system().name() == "bo_CN")
                _NomalWidget->setFixedHeight(60 + selfHeight - 184);
            else
                _NomalWidget->setFixedHeight((count >= 7) ? (50 + selfHeight - 160) : (55 + selfHeight - 160));
        }
        update();
        Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(), false);
    }
}

void TrayWidget::layoutAddOnePairedDev(QMap<QString, QVariant> devAttr)
{
    updatePairedDev();
}

void TrayWidget::updatePairedDev() {
    pairedDevices = _SessionDBus->getPairedDevices();
    clearPairedWidget();
    layoutAddAllPairedDev();
}

/************************************************
 * @brief  清空现在显示的所有配对设备UI
 * @param  null
 * @return null
*************************************************/
void TrayWidget::clearPairedWidget()
{
    KyDebug() << __LINE__;
    QLayoutItem *child;
    QDevItem *tmp;
    while ((child = _PairedLayout->takeAt(0)) != 0)
    {
        if(child->isEmpty())
            return;
        if(child->widget())
        {
            tmp = qobject_cast<QDevItem*>(child->widget());
            disconnect(tmp, &QDevItem::devConnect, tmp, nullptr);
            disconnect(_SessionDBus, &BluetoothDbus::devAttrChanged, tmp, nullptr);
            child->widget()->setParent(NULL);
        }
        delete child;
        delete tmp;
        tmp = nullptr;
        child = nullptr;
    }
}

/************************************************
 * @brief  根据BluezQt::DevicePtr清空现在显示的配对设备UI
 * @param  devPtr 配对设备指针
 * @return null
*************************************************/
void TrayWidget::removePairedDevUIByAddress(QString address)
{
    if (!pairedDevices.contains(address))
        return;
    KyDebug() << address << __LINE__;
    QDevItem *item = _PairedWidget->findChild<QDevItem *>(address);
    if (item) {
        _PairedLayout->removeWidget(item);
        item->setParent(NULL);
        item->deleteLater();
        item = nullptr;
    }
    pairedDevices.remove(address);
    updatePairedDev();
}

void TrayWidget::setTrayIconClickEnable(bool enable) {
    trayicon->setClickEnable(enable);
}

void TrayWidget::showUI()
{
    if (this->isActiveWindow()) {
        hide();
        return;
    }

    if (!_Intel) {
        int count = pairedDevices.size();
        if (QLocale::system().name() == "bo_CN")
            selfHeight = 184 + count * 52;
        else
            selfHeight = 160 + count * 52;
        if (count >= 7)
            selfHeight = 515;
        this->setFixedHeight(((_MBtn->isChecked()) && (count != 0) ?  selfHeight : 102));
        if (QLocale::system().name() == "bo_CN")
            _NomalWidget->setFixedHeight(60 + selfHeight - 184);
        else
            _NomalWidget->setFixedHeight((count >= 7) ? (50 + selfHeight - 160) : (55 + selfHeight - 160));
        //2209中窗管在hide界面时会刷新属性，需要重新设置无图标属性
        const KWindowInfo info(this->winId(), NET::WMState);
        if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager) || !info.hasState(NET::SkipSwitcher))
            KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
        showNormal();
        activateWindow();
//        const KWindowInfo info(this->winId(), NET::WMState);
        if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager) || !info.hasState(NET::SkipSwitcher))
            KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
        Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(), false);
        return;
    }
    QApplication::desktop()->update();
    QRect availableGeometry = QApplication::screens().at(0)->geometry();
    KyDebug() << availableGeometry.x() << availableGeometry.y();
    move((availableGeometry.x() + (availableGeometry.width() - width())/2), (availableGeometry.y() + (availableGeometry.height() - height())/2));

    //2209中窗管在hide界面时会刷新属性，需要重新设置无图标属性
    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager) || !info.hasState(NET::SkipSwitcher))
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
    showNormal();
}

bool TrayWidget::whetherNeedInfoUser()
{
    KyDebug();
    unsigned int bluetoothMouseAmount = 0;
    unsigned int bluetoothKeyBoardAmount = 0 ;

    if (_SessionDBus->getExistAdapter())
    {
        pairedDevices = _SessionDBus->getPairedDevices();
        for (auto addr : pairedDevices.keys())
        {
            if (pairedDevices.value(addr).value("Paired").toBool()
                    && pairedDevices.value(addr).value("Connected").toBool())
            {
                if (Type::Mouse == pairedDevices.value(addr).value("Type").toInt())
                {
                    bluetoothMouseAmount += 1;
                }
                else if (Type::Keyboard == pairedDevices.value(addr).value("Type").toInt())
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

    unsigned int systemMouseAndTouchPadAmount = x11Devices::getSystemCurrentMouseAndTouchPadDevCount();
    unsigned int systemKeyBoardAmount = x11Devices::getSystemCurrentKeyBoardDevCount();
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

void TrayWidget::kbUDEvent(bool opt) {
    if (pressCnt != 1)
        return;
    QList<QDevItem*> l = _PairedWidget->findChildren<QDevItem *>();
    int size = l.size();
    if (selectedDev != nullptr)
        selectedDev->setFocusStyle(false);

    if (opt) {
        if (++listCnt > size) {
            l.at(size - 1)->setFocusStyle(false);
            listCnt = 1;
        }
        KyDebug() << "press Down" << listCnt;
    } else {
        if (--listCnt < 1) {
            l.at(0)->setFocusStyle(false);
            listCnt = size;
        }
        KyDebug() << "press Up" << listCnt;
    }

    selectedDev = l.at(listCnt - 1);
    selectedDev->setFocusStyle(true);

    autoChangePage(opt);
}

void TrayWidget::hideUI() {
    this->hide();
}

void TrayWidget::kbTabEvent() {
    QList<QDevItem*> l = _PairedWidget->findChildren<QDevItem *>();
    int size = l.size();

    if (!_MBtn->isChecked())
        size = 0;

    if (++pressCnt > 3)
        pressCnt = 1;

    if (size == 0 && pressCnt == 1) {
        ++pressCnt;
    }

    KyDebug() << "press Tab"<< pressCnt;

    switch (pressCnt) {
    case 1:
        //预选框唯一处理
        scrollTo = 0;
        scrollCnt = 0;
        selectedDev = nullptr;
        MBtnFocused = false;
        this->update();
        kbUDEvent(true);
        break;
    case 2:
        if (listCnt >= 1)
            l.at(listCnt - 1)->setFocusStyle(false);
        MBtnFocused = false;
        this->update();
        _MSettingBtn->setFocusStyle(true);
        listCnt = 0;
        break;
    case 3:
        _MSettingBtn->setFocusStyle(false);
        MBtnFocused = true;
        this->update();
        break;
    default:
        return;
    }
}

void TrayWidget::kbEnterEvent() {
    if (pressCnt == 0)
        return;

    QList<QDevItem*> l = _PairedWidget->findChildren<QDevItem *>();

    if(pressCnt == 3 &&
       _MBtn->isCheckable() &&
       _MBtn->isChecked() == _SessionDBus->isPowered())
    {
        _SessionDBus->setDefaultAdapterPower(!_MBtn->isChecked());
    } else if (pressCnt == 2) {
        _SessionDBus->openBluetoothSettings();
        this->hide();
    }
    else {
        if(listCnt < 1)
            return;
        l.at(listCnt - 1)->pressEnterCallback();
    }
}

void TrayWidget::kbHomeAndEndEvent(bool opt) {
    QList<QDevItem*> l = _PairedWidget->findChildren<QDevItem *>();
    int size = l.size();
    if (pressCnt != 1)
        return;
    if (!_MBtn->isChecked() && size == 0)
        return;
    if (selectedDev != nullptr)
        selectedDev->setFocusStyle(false);
    if (opt)
        listCnt = 1;
    else
        listCnt = size;
    selectedDev = l.at(listCnt - 1);
    selectedDev->setFocusStyle(true);
    autoChangePage(opt);
}

void TrayWidget::kbPageEvent(bool opt) {
    if (pressCnt != 1)
        return;
    QList<QDevItem*> l = _PairedWidget->findChildren<QDevItem *>();
    int size = l.size();
    if (selectedDev != nullptr)
        selectedDev->setFocusStyle(false);

    if (opt) {
        listCnt += 6;
        if (listCnt > size) {
            listCnt = size;
        }
        KyDebug() << "press Down" << listCnt;
    } else {
        listCnt -= 6;
        if (listCnt < 1) {
            listCnt = 1;
        }
        KyDebug() << "press Up" << listCnt;
    }

    selectedDev = l.at(listCnt - 1);
    selectedDev->setFocusStyle(true);
    scrollTo = opt ? -1 : 1;
    scrollCnt = 6;
    //自动翻页
    if(opt && listCnt == 1) {
        _ScrollArea->verticalScrollBar()->setSliderPosition(0);
        return;
    }
    if (!opt && listCnt == size) {
        _ScrollArea->verticalScrollBar()->setSliderPosition((size - 7) * 52);
        return;
    }

    if (opt && listCnt > 7){
        KyDebug() << "向下翻页" << scrollCnt;
        _ScrollArea->verticalScrollBar()->setSliderPosition(52 * (listCnt - 7));
    }
    if (!opt && listCnt <= size - 7) {
        KyDebug() << "向上翻页" << scrollCnt;
            _ScrollArea->verticalScrollBar()->setSliderPosition(52 * (listCnt - 1));
    }
}

void TrayWidget::autoChangePage(bool opt) {
    QList<QDevItem*> l = _PairedWidget->findChildren<QDevItem *>();
    int size = l.size();

    //自动翻页
    if(opt && listCnt == 1) {
        _ScrollArea->verticalScrollBar()->setSliderPosition(0);
        return;
    }
    if (!opt && listCnt == size) {
        _ScrollArea->verticalScrollBar()->setSliderPosition((size - 7) * 52);
        return;
    }

    if (opt){
        if (scrollTo == 1 && scrollCnt != 0) {
            --scrollCnt;
            return;
        }
        if (scrollTo == -1 && scrollCnt != 6) {
            ++scrollCnt;
            return;
        }
        scrollCnt = 6;
        scrollTo = -1;
        KyDebug() << "向下翻页" << scrollCnt;
        if (listCnt > 7)
            _ScrollArea->verticalScrollBar()->setSliderPosition(52 * (listCnt - 7));
        else
            _ScrollArea->verticalScrollBar()->setSliderPosition(0);
    }
    if (!opt) {
        if (scrollTo == -1 && scrollCnt != 0) {
            --scrollCnt;
            return;
        }
        if (scrollTo == 1 && scrollCnt != 6) {
            ++scrollCnt;
            return;
        }
        scrollCnt = 6;
        scrollTo = 1;
        KyDebug() << "向上翻页" << scrollCnt;
        if (listCnt <= size - 7)
            _ScrollArea->verticalScrollBar()->setSliderPosition(52 * (listCnt - 1));
        else
            _ScrollArea->verticalScrollBar()->setSliderPosition((size - 7) * 52);
    }
}

void TrayWidget::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        if (nullptr == m_panelGSettings) {
            m_panelGSettings = new QGSettings(id);
        }

        if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        }

        if (m_panelGSettings->keys().contains(PANEL_SIZE_KEY)) {
            m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
        }

        connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString &key) {
            if (key == PANEL_POSITION_KEY) {
                m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
                Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(),
                                       m_panelPosition, m_panelSize, false);
            }
            if (key == PANEL_SIZE_KEY) {
                m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
                Config::setKdkGeometry(this->windowHandle(), this->width(), this->height(),
                                       m_panelPosition, m_panelSize, false);
            }

        });
    }
}

bool TrayWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ActivationChange)
    {
        if(QApplication::activeWindow() != this)
        {
            this->hide();
        }
    }
    return QWidget::eventFilter(o,e);
}

void TrayWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    QColor col;
    if(QGSettings::isSchemaInstalled("org.lingmo.style")
            && StyleSettings->get("style-name").toString() == "lingmo-default"){
        col = QColor("#232426");
    } else
        col = this->palette().color(QPalette::Active, QPalette::Window);
    QPainterPath rectPath;

    if (_Intel)
        rectPath.addRect(this->rect());
    else {
        col.setAlpha(tran);
        int radius = StyleSettings->get("window-radius").toInt();
        rectPath.addRoundedRect(this->rect(),radius, radius);
    }

    painter.setBrush(col);
    painter.drawPath(rectPath);

    //焦点预选框
    if (!MBtnFocused)
        return;
    QPainter pt(this);
    QColor color = QColor("#818181");
    pt.setPen(color);
    QRect rect;
    rect.setLeft(_MBtn->geometry().left() - 1);
    rect.setTop(_MBtn->geometry().top() - 1);
    rect.setWidth(_MBtn->width() + 2);
    rect.setHeight(_MBtn->height() + 2);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.drawRoundedRect(rect,12,12,Qt::AbsoluteSize);
}

void TrayWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_PageUp:
        kbPageEvent(false);
        break;
    case Qt::Key_PageDown:
        kbPageEvent(true);
        break;
    case Qt::Key_End:
        kbHomeAndEndEvent(false);
        break;
    case Qt::Key_Home:
        kbHomeAndEndEvent(true);
        break;
    case Qt::Key_Tab:
        kbTabEvent();
        break;
    case Qt::Key_Down:
        kbUDEvent(true);
        break;
    case Qt::Key_Up:
        kbUDEvent(false);
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
        KyDebug() << "press Enter";
        kbEnterEvent();
        break;
    case Qt::Key_Escape:
        KyDebug() << "press Esc";
        this->hide();
        break;
    default:
        break;
    }
}

void TrayWidget::showEvent(QShowEvent *event) {
    Q_UNUSED(event)
    _ScrollArea->clearFocus();
    _PairedWidget->clearFocus();
}

void TrayWidget::hideEvent(QHideEvent *event) {
    //初始化键盘交互界面，清除操作记录
    Q_UNUSED(event)
    if (pressCnt == 0)
        return;
    QList<QDevItem*> l = _PairedWidget->findChildren<QDevItem *>();
    int size = l.size();

    if (!_MBtn->isChecked())
        size = 0;

    switch (pressCnt) {
    case 1:
        if(listCnt < 1)
            return;
        if (selectedDev != nullptr) {
            selectedDev->setFocusStyle(false);
            selectedDev = nullptr;
        }
        scrollCnt = 0;
        scrollTo = 0;
        listCnt = 0;
        break;
    case 2:
        _MSettingBtn->setFocusStyle(false);
        break;
    case 3:
        MBtnFocused = false;
        this->update();
        break;
    default:
        break;
    }
    pressCnt = 0;
    _ScrollArea->verticalScrollBar()->setSliderPosition(0);
}

void TrayWidget::moveEvent(QMoveEvent *event) {
    KyDebug() << "-=-=-=-=-=-=-=-=--==" ;
}
