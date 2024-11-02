/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include "ukmedia_app_device_ctrl.h"
#include <QDebug>

UkmediaAppCtrlWidget::UkmediaAppCtrlWidget(QWidget *parent)
    : KWidget(parent)
{
    initData();
    initUI();
    dealSlot();
}

void UkmediaAppCtrlWidget::initData()
{
    interface = new QDBusInterface(LINGMO_MEDIA_SERVICE,
                                   LINGMO_MEDIA_PATH,
                                   LINGMO_MEDIA_INTERFACE,
                                   QDBusConnection::sessionBus());

    if (!interface->isValid())
        return;

    //1.获取app列表
    getAppList();

    //2.获取所有可用端口信息
    getAllPortInfo();

    //3.获取可用输出端口
    getAllOutputPort();

    //4.获取可用输入端口
    getAllInputPort();
}

void UkmediaAppCtrlWidget::initUI()
{
    setWidgetName(tr("App Sound Control"));
    setIcon(QIcon::fromTheme("lingmo-control-center"));
    setWindowFlags(Qt::Dialog);

    stackWidget = new QStackedWidget(this);
    stackWidget->setFixedSize(560, 520);
    m_pAppSoundCtrlBar = new KNavigationBar(this);
    m_pAppSoundCtrlBar->setFixedSize(188,520);

    QBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_pAppSoundCtrlBar);
    sideBar()->setLayout(vLayout);

    vLayout = new QVBoxLayout;
    vLayout->addWidget(stackWidget);
    baseBar()->setLayout(vLayout);
    this->setLayoutType(HorizontalType);

    //  添加应用到导航栏
    for (QString appName : appList)
        addItem(appName);

    //  初始化UI 默认选中第一项
    QStandardItemModel* m_mode = m_pAppSoundCtrlBar->model();  //获取mode
    m_pAppSoundCtrlBar->listview()->setCurrentIndex(m_mode->item(0,0)->index());
}

void UkmediaAppCtrlWidget::addItem(QString appName)
{
    //  获取应用 name
    QString iconName = getAppIcon(appName);
    QString pAppName = getAppName(appName);

    //  1.左边导航栏增加标题
    QStandardItem *item = new QStandardItem(QIcon::fromTheme(iconName), pAppName);
    m_pAppSoundCtrlBar->addItem(item);

    //  2.右边添加窗口
    UkmediaAppItemWidget* widget = new UkmediaAppItemWidget();
    widget->setTitleName(pAppName);
    widget->setChildObjectName(appName);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    stackWidget->addWidget(widget);

    //  3.设置app对应音量滑动条值
    widget->setSliderValue(getAppVolume(appName));
    widget->outputVolumeDarkThemeImage(getAppVolume(appName), getAppMuteState(appName));

    //  4.设置app对应输出输入设备
    for (QString sinkPortLabel : sinkPortList)
        widget->addOutputCombobox(sinkPortLabel);

    for (QString sourcePortLabel : sourcePortList)
        widget->addInputCombobox(sourcePortLabel);

    QString defaultInPort = getAppInputDevice(appName);
    QString defaultOutPort = getAppOutputDevice(appName);
    widget->m_pOutputCombobox->setCurrentText(defaultOutPort);
    widget->m_pInputCombobox->setCurrentText(defaultInPort);

    //  5.slots
    connect(widget->m_pVolumeSlider, &KSlider::valueChanged, this, &UkmediaAppCtrlWidget::setAppVolume);
    connect(widget->m_pVolumeBtn, &LingmoUIButtonDrawSvg::clicked, this, &UkmediaAppCtrlWidget::setAppMuteState);
    connect(widget->m_pInputCombobox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &UkmediaAppCtrlWidget::setAppInputDevice);
    connect(widget->m_pOutputCombobox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &UkmediaAppCtrlWidget::setAppOutputDevice);
    connect(widget->m_pConfirmBtn, &QPushButton::clicked, this, [=](){
        this->close();
    });

    //需求31268待2501再合入
#if 1
    if (appName.compare(QString(SYSTEM_VOLUME_CTRL)) == 0) {
        if (!sinkPortList.isEmpty()) {
            if (QString(sinkPortList.at(0)).compare(QString(tr("None"))) == 0) {
                widget->m_pVolumeSlider->setEnabled(false);
                widget->m_pVolumeBtn->setEnabled(false);
            } else {
                widget->m_pVolumeSlider->setEnabled(true);
                widget->m_pVolumeBtn->setEnabled(true);
            }
        }
    }
#endif

    // 6.区分App类型
    int appType = findAppDirection(appName);

    switch (appType) {
    case PA_STREAM_PLAYBACK:
        item->setData(PA_STREAM_PLAYBACK);
        widget->m_pInputCombobox->setDisabled(true);
        break;
    case PA_STREAM_RECORD:
        item->setData(PA_STREAM_RECORD);
        widget->m_pVolumeWidget->hide();
        widget->m_pOutputCombobox->setDisabled(true);
        break;
    default:
        item->setData(PA_STREAM_NODIRECTION);
        break;
    }

#ifdef PA_PROP_APPLICATION_MOVE
    widget->setInputHintWidgetShow(checkAppMoveStatus(PA_STREAM_RECORD, appName));
    widget->setOutputHintWidgetShow(checkAppMoveStatus(PA_STREAM_PLAYBACK, appName));
#endif
    qDebug() << __func__ << "Application:" << appName << "Type:" << item->data().toInt();
}

void UkmediaAppCtrlWidget::removeAppItem(QString appName)
{
    int index = -1;
    for (int i = 0; i < appList.count(); i++) {
        if (appList.at(i) == appName) {
            index = i;
            appList.removeAt(i);
            break;
        }
    }

    if (index == -1)
        return;

    QWidget* w = stackWidget->widget(index);
    stackWidget->removeWidget(w);
    w->close();
    delete w;
    m_pAppSoundCtrlBar->model()->removeRow(index);
    stackWidget->setCurrentIndex(m_pAppSoundCtrlBar->listview()->currentIndex().row());
    qDebug() << __func__ << appName << "stackWidget:" << stackWidget->children();
}

void UkmediaAppCtrlWidget::dealSlot()
{
    //  导航栏点击应用切换对应窗口slot
    connect(m_pAppSoundCtrlBar->listview(), &QListView::clicked, this, [=](const QModelIndex modelIndex) {
        int index = m_pAppSoundCtrlBar->listview()->currentIndex().row();
        stackWidget->setCurrentIndex(index);
    });;

    QDBusConnection::sessionBus().connect(QString(),
                                          LINGMO_MEDIA_CONTROL_PATH,
                                          LINGMO_MEDIA_SERVICE,
                                          "updatePortSignal",
                                          this,
                                          SLOT(updatePort()));

    QDBusConnection::sessionBus().connect(QString(),
                                          LINGMO_MEDIA_CONTROL_PATH,
                                          LINGMO_MEDIA_SERVICE,
                                          "updateMute",
                                          this,
                                          SLOT(updateSystemMuteState(bool)));

    QDBusConnection::sessionBus().connect(QString(),
                                          LINGMO_MEDIA_CONTROL_PATH,
                                          LINGMO_MEDIA_SERVICE,
                                          "updateVolume",
                                          this,
                                          SLOT(updateSystemVolume(int)));

    QDBusConnection::sessionBus().connect(QString(),
                                          LINGMO_MEDIA_CONTROL_PATH,
                                          LINGMO_MEDIA_SERVICE,
                                          "updateApp",
                                          this,
                                          SLOT(updateAppItem(QString)));

    QDBusConnection::sessionBus().connect(QString(),
                                          LINGMO_MEDIA_CONTROL_PATH,
                                          LINGMO_MEDIA_SERVICE,
                                          "removeSinkInputSignal",
                                          this,
                                          SLOT(removeAppItem(QString)));

//    QDBusConnection::sessionBus().connect(QString(),
//                                          LINGMO_MEDIA_CONTROL_PATH,
//                                          LINGMO_MEDIA_SERVICE,
//                                          "sinkInputVolumeChangedSignal",
//                                          this,
//                                          SLOT(appVolumeChangedSlot(QString, QString, int)));
}

//  获取运行应用名单
void UkmediaAppCtrlWidget::getAppList()
{
    //  首先添加系统音量
    appList << SYSTEM_VOLUME_CTRL;

    QDBusReply<QStringList> reply = interface->call("getAppList");

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return;
    }

    appList << reply.value();
    getPlaybackAppInfo();
    getRecordAppInfo();
}

void UkmediaAppCtrlWidget::getPlaybackAppInfo()
{
    qRegisterMetaType<appInfo>("appInfo");
    qDBusRegisterMetaType<appInfo>();
    QDBusReply<QList<QVariant>> reply = interface->call("getPlaybackAppInfo");
    QList<QVariant> list = reply.value();

    for (int i = 0; i < list.count(); i++) {
        const QDBusArgument &dbusArgs = list.at(i).value<QDBusArgument>();
        while (!dbusArgs.atEnd()) {
            appInfo info;
            dbusArgs.beginStructure();
            dbusArgs >> info.index;
            dbusArgs >> info.volume;
            dbusArgs >> info.channel;
            dbusArgs >> info.mute;
            dbusArgs >> info.direction;
#ifdef PA_PROP_APPLICATION_MOVE
            dbusArgs >> info.move;
#endif
            dbusArgs >> info.name;
            dbusArgs >> info.masterIndex;
            dbusArgs >> info.masterDevice;
            playbackAppInfoList.push_back(info);
            dbusArgs.endStructure();
            qDebug() << __func__ << info.index << info.name << info.volume << info.masterDevice;
        }
    }
}

void UkmediaAppCtrlWidget::getRecordAppInfo()
{
    qRegisterMetaType<appInfo>("appInfo");
    qDBusRegisterMetaType<appInfo>();
    QDBusReply<QList<QVariant>> reply = interface->call("getRecordAppInfo");
    QList<QVariant> list = reply.value();

    for (int i = 0; i < list.count(); i++) {
        const QDBusArgument &dbusArgs = list.at(i).value<QDBusArgument>();
        while (!dbusArgs.atEnd()) {
            appInfo info;
            dbusArgs.beginStructure();
            dbusArgs >> info.index;
            dbusArgs >> info.volume;
            dbusArgs >> info.channel;
            dbusArgs >> info.mute;
            dbusArgs >> info.direction;
#ifdef PA_PROP_APPLICATION_MOVE
            dbusArgs >> info.move;
#endif
            dbusArgs >> info.name;
            dbusArgs >> info.masterIndex;
            dbusArgs >> info.masterDevice;
            recordAppInfoList.push_back(info);
            dbusArgs.endStructure();
            qDebug() << __func__ << info.index << info.name << info.volume << info.masterDevice << info.direction;
        }
    }
}

void UkmediaAppCtrlWidget::getAllPortInfo()
{
    // 注册自定义结构体类型
    qRegisterMetaType<pa_device_port_info>("pa_device_port_info");
    qDBusRegisterMetaType<pa_device_port_info>();
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qDebug() << "Cannot connect to D-Bus.";
        return ;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(PULSEAUDIO_DEVICECONTROL_SERVICE,
                                                          PULSEAUDIO_DEVICECONTROL_PATH,
                                                          PULSEAUDIO_DEVICECONTROL_INTERFACE,
                                                          "GetAllDeviceInfo");
    QDBusMessage reply = bus.call(message);

    const QDBusArgument &dbusArgs = reply.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>();
    QList<pa_device_port_info> devsInfo;

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        pa_device_port_info info;
        dbusArgs.beginStructure();
        dbusArgs >> info.card;
        dbusArgs >> info.direction;
        dbusArgs >> info.available;
        dbusArgs >> info.plugged_stauts;
        dbusArgs >> info.name;
        dbusArgs >> info.description;
        dbusArgs >> info.device_description;
        dbusArgs >> info.device_product_name;
        devsInfo.push_back(info);
        dbusArgs.endStructure();
    }
    dbusArgs.endArray();

    //  添加端口至portInfoMap
    int count = 0;
    for (pa_device_port_info &info : devsInfo) {
        if (info.available == PA_PORT_AVAILABLE_YES || info.available == PA_PORT_AVAILABLE_UNKNOWN) {
            QString portLabel = info.description + "（" + info.device_description + "）";
            info.description = portLabel;
            portInfoMap.insert(count, info);
            count++;
        }
    }
}

//  获取所有可用输出端口
QStringList UkmediaAppCtrlWidget::getAllOutputPort()
{
    QMap<int, pa_device_port_info>::iterator it;

    for (it = portInfoMap.begin(); it != portInfoMap.end(); it++) {
        if (it.value().direction == PA_DIRECTION_OUTPUT) {
            sinkPortList << it.value().description;

            // Usb声卡和蓝牙声卡存在多种配置文件，保留声卡当前选择的配置文件端口
            QDBusReply<bool> reply = interface->call("isPortHidingNeeded", SoundType::SINK, it.value().card, it.value().description);
            if (reply.value())
                sinkPortList.removeAll(it.value().description);
        }
    }

    if (sinkPortList.isEmpty())
        sinkPortList.append(tr("None"));

    return sinkPortList;
}

//  获取所有可用输入端口
QStringList UkmediaAppCtrlWidget::getAllInputPort()
{
    QMap<int, pa_device_port_info>::iterator it;

    for (it = portInfoMap.begin(); it != portInfoMap.end(); it++) {
        if (it.value().direction == PA_DIRECTION_INPUT) {
            sourcePortList << it.value().description;

            QDBusReply<bool> reply = interface->call("isPortHidingNeeded", SoundType::SOURCE, it.value().card, it.value().description);
            if (reply.value())
                sourcePortList.removeAll(it.value().description);
        }
    }

    if (sourcePortList.isEmpty())
        sourcePortList.append(tr("None"));

    return sourcePortList;
}

//  获取系统音量
int UkmediaAppCtrlWidget::getSystemVolume()
{
    QDBusReply<int> reply = interface->call("getDefaultOutputVolume");

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return 0;
    }

    return reply.value();
}

//  获取应用音量
int UkmediaAppCtrlWidget::getAppVolume(QString app)
{
    if (app == SYSTEM_VOLUME_CTRL)
        return getSystemVolume();

    QDBusReply<int> reply = interface->call("getAppVolume", app);

    if(!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return 0;
    }

    return reply.value();
}

bool UkmediaAppCtrlWidget::getSystemMuteState()
{
    QDBusReply<bool> reply = interface->call("getDefaultOutputMuteState");

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    return reply.value();
}

//  获取应用静音状态
bool UkmediaAppCtrlWidget::getAppMuteState(QString app)
{
    if (app == SYSTEM_VOLUME_CTRL)
        return getSystemMuteState();

    QDBusReply<bool> reply = interface->call("getAppMuteState", app);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    return reply.value();
}

//  获取系统默认输出设备
QString UkmediaAppCtrlWidget::getSystemOutputDevice()
{
    QDBusReply<QString> reply = interface->call("getSystemOutputDevice");

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return "";
    }

    return reply.value();
}

//  获取系统默认输入设备
QString UkmediaAppCtrlWidget::getSystemInputDevice()
{
    QDBusReply<QString> reply = interface->call("getSystemInputDevice");

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return "";
    }

    return reply.value();
}

//  获取应用默认输出设备
QString UkmediaAppCtrlWidget::getAppOutputDevice(QString app)
{
    if (app == SYSTEM_VOLUME_CTRL)
        return getSystemOutputDevice();

    QDBusReply<QString> reply = interface->call("getAppOutputDevice", app);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return "";
    }

    return reply.value();
}

//  获取应用默认输入设备
QString UkmediaAppCtrlWidget::getAppInputDevice(QString app)
{
    if (app == SYSTEM_VOLUME_CTRL)
        return getSystemInputDevice();

    QDBusReply<QString> reply = interface->call("getAppInputDevice", app);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return "";
    }

    return reply.value();
}

//  设置系统音量
bool UkmediaAppCtrlWidget::setSystemVolume(int value)
{
    QDBusReply<bool> reply = interface->call("setDefaultOutputVolume", value);

    if (!reply.isValid())
        return false;

    return reply.value();
}

//  设置应用音量
bool UkmediaAppCtrlWidget::setAppVolume(int value)
{
    QSlider *slider = qobject_cast<QSlider *>(sender());
    QString app = slider->objectName();
    UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(app);
    w->outputVolumeDarkThemeImage(value, getAppMuteState(app));

    if (app == SYSTEM_VOLUME_CTRL)
        return setSystemVolume(value);

    QDBusReply<bool> reply = interface->call("setAppVolume", app, value);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    return reply.value();
}

//  设置应用静音
bool UkmediaAppCtrlWidget::setAppMuteState()
{
    LingmoUIButtonDrawSvg *btn = qobject_cast<LingmoUIButtonDrawSvg *>(sender());
    QString app = btn->objectName();

    //  1.系统静音设置
    if (app == SYSTEM_VOLUME_CTRL) {
        QDBusReply<bool> reply1 = interface->call("getDefaultOutputMuteState");

        if (!reply1.isValid())
            return false;

        bool state = reply1.value();

        QDBusReply<bool> reply2 = interface->call("setDefaultOutputMuteState", !state);

        if (!reply2.isValid())
            return false;

        btn->outputVolumeDarkThemeImage(getAppVolume(app), !state);

        return reply2.value();
    }

    //  2.应用静音设置
    QDBusReply<bool> reply1 = interface->call("getAppMuteState", app);

    if (!reply1.isValid())
        return false;

    bool state = reply1.value();

    QDBusReply<bool> reply = interface->call("setAppMuteState", app, !state);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    btn->outputVolumeDarkThemeImage(getAppVolume(app), !state);

    return reply.value();
}

//  设置系统输入设备
bool UkmediaAppCtrlWidget::setSystemInputDevice(QString portLabel)
{
    QDBusReply<bool> reply = interface->call("setSystemInputDevice", portLabel);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    return reply.value();
}

//  设置系统输出设备
bool UkmediaAppCtrlWidget::setSystemOutputDevice(QString portLabel)
{
    QDBusReply<bool> reply = interface->call("setSystemOutputDevice", portLabel);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    return reply.value();
}

//  设置应用输出设备
bool UkmediaAppCtrlWidget::setAppOutputDevice(QString portLabel)
{
    QComboBox *combobox = qobject_cast<QComboBox *>(sender());
    QString app = combobox->objectName().split("-output").at(0);

    if (app == SYSTEM_VOLUME_CTRL)
        return setSystemOutputDevice(portLabel);

    int cardIndex = -1;
    QString sinkPortName;
    QMap<int, pa_device_port_info>::iterator it;

    for (it = portInfoMap.begin(); it != portInfoMap.end(); it++) {
        pa_device_port_info info = it.value();
        if (portLabel == info.description && info.direction == 1) {
            cardIndex = info.card;
            sinkPortName = info.name;
        }
    }

    QDBusReply<bool> reply = interface->call("setAppOutputDevice", app, cardIndex, sinkPortName);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    if (!reply.value()) {
        combobox->blockSignals(true);
        combobox->setCurrentText(getAppOutputDevice(app));
        combobox->blockSignals(false);
        qDebug() << __func__ << "failed";
        return false;
    }

    return reply.value();
}

//  设置应用输入设备
bool UkmediaAppCtrlWidget::setAppInputDevice(QString portLabel)
{
    QComboBox *combobox = qobject_cast<QComboBox *>(sender());
    QString app = combobox->objectName().split("-input").at(0);

    if (app == SYSTEM_VOLUME_CTRL)
        return setSystemInputDevice(portLabel);

    int cardIndex = -1;
    QString sourcePortName;
    QMap<int, pa_device_port_info>::iterator it;

    for (it = portInfoMap.begin(); it != portInfoMap.end(); it++) {
        pa_device_port_info info = it.value();
        if (portLabel == info.description && info.direction == 2) {
            cardIndex = info.card;
            sourcePortName = info.name;
        }
    }

    QDBusReply<bool> reply = interface->call("setAppInputDevice", app, cardIndex, sourcePortName);

    if (!reply.isValid()) {
        qWarning() << __func__ << "failed";
        return false;
    }

    if (!reply.value()) {
        combobox->blockSignals(true);
        combobox->setCurrentText(getAppInputDevice(app));
        combobox->blockSignals(false);
        qDebug() << __func__ << "failed";
        return false;
    }

    return reply.value();
}

void UkmediaAppCtrlWidget::updateAppItem(QString appName)
{
    playbackAppInfoList.clear();
    recordAppInfoList.clear();
    getPlaybackAppInfo();
    getRecordAppInfo();

    if (!appList.contains(appName)) {
        appList << appName;
        addItem(appName);
        return;
    }
    else {
        UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(appName);
        w->setSliderValue(getAppVolume(appName));
        w->outputVolumeDarkThemeImage(getAppVolume(appName), getAppMuteState(appName));
        int appType = findAppDirection(appName);

        switch (appType) {
        case PA_STREAM_PLAYBACK:
            w->m_pVolumeWidget->show();
            w->m_pInputCombobox->setDisabled(true);
            w->m_pOutputCombobox->setDisabled(false);
            qDebug() << __func__ << appName << "PA_STREAM_PLAYBACK";
            break;
        case PA_STREAM_RECORD:
            w->m_pVolumeWidget->hide();
            w->m_pInputCombobox->setDisabled(false);
            w->m_pOutputCombobox->setDisabled(true);
            qDebug() << __func__ << appName << "PA_STREAM_RECORD";
            break;
        default:
            w->m_pVolumeWidget->show();
            w->m_pInputCombobox->setDisabled(false);
            w->m_pOutputCombobox->setDisabled(false);
            qDebug() << __func__ << appName << "PA_STREAM_NODIRECTION";
            break;
        }
#ifdef PA_PROP_APPLICATION_MOVE
        w->setInputHintWidgetShow(checkAppMoveStatus(PA_STREAM_RECORD, appName));
        w->setOutputHintWidgetShow(checkAppMoveStatus(PA_STREAM_PLAYBACK, appName));
#endif
    }
}

//  同步系统音量
void UkmediaAppCtrlWidget::updateSystemVolume(int value)
{
    UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(SYSTEM_VOLUME_CTRL);
    w->setSliderValue(paVolumeToValue(value));
    w->outputVolumeDarkThemeImage(paVolumeToValue(value), getSystemMuteState());

    for (int i = 0; i < appList.count(); i++) {
        QString appName = appList.at(i);
        UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(appName);
        w->m_pOutputCombobox->blockSignals(true);
        w->m_pOutputCombobox->setCurrentText(getAppOutputDevice(appName));
        w->m_pOutputCombobox->blockSignals(false);
        w->m_pInputCombobox->blockSignals(true);
        w->m_pInputCombobox->setCurrentText(getAppInputDevice(appName));
        w->m_pInputCombobox->blockSignals(false);
    }
}

//  同步应用音量
void UkmediaAppCtrlWidget::appVolumeChangedSlot(QString app, QString appId, int volume)
{
    QSlider *slider = stackWidget->findChild<QSlider*>(app);
    QLabel *sliderLabel = stackWidget->findChild<QLabel*>(app+"-label");

    if (slider == nullptr || sliderLabel == nullptr)
        return;

    slider->blockSignals(true);
    slider->setValue(paVolumeToValue(volume));
    slider->blockSignals(false);
    QString percent = QString::number(paVolumeToValue(volume));
    sliderLabel->setText(percent+"%");

    for (QString appName : appList) {
        QComboBox *outputBox = stackWidget->findChild<QComboBox *>(appName + "-output");
        QComboBox *inputBox = stackWidget->findChild<QComboBox *>(appName + "-output");
        inputBox->blockSignals(true);
        outputBox->blockSignals(true);
        QString defaultOutPort = getAppOutputDevice(appName);
        QString defaultInPort = getAppInputDevice(appName);
        inputBox->setCurrentText(defaultInPort);
        outputBox->setCurrentText(defaultOutPort);
        outputBox->blockSignals(false);
        inputBox->blockSignals(false);
    }
    qDebug() << __func__ << app << volume << stackWidget->children();
}

//  同步系统静音状态
void UkmediaAppCtrlWidget::updateSystemMuteState(bool state)
{
    LingmoUIButtonDrawSvg *btn = stackWidget->findChild<LingmoUIButtonDrawSvg *>(SYSTEM_VOLUME_CTRL);
    btn->outputVolumeDarkThemeImage(getSystemVolume(), state);
}

//  同步应用静音状态
void UkmediaAppCtrlWidget::updateAppMuteState(QString app, bool state)
{
    LingmoUIButtonDrawSvg *btn = stackWidget->findChild<LingmoUIButtonDrawSvg *>(app);
    btn->outputVolumeDarkThemeImage(getAppVolume(app), state);
}


void UkmediaAppCtrlWidget::updatePort()
{
    qDebug() << __func__ << sinkPortList;
    QStringList currentSinkPortList = sinkPortList;
    QStringList currentSourcePortList = sourcePortList;

    portInfoMap.clear();
    sinkPortList.clear();
    sourcePortList.clear();
    getAllPortInfo();
    getAllInputPort();
    getAllOutputPort();

    //  1. 删除不可用输出端口
    for (int i = 0; i < currentSinkPortList.count(); i++) {
        QString port = currentSinkPortList.at(i);
        if (!sinkPortList.contains(port)) {
            for (int a = 0; a < appList.count(); a++) {
                QString appName = appList.at(a);
                UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(appName);
                int index = indexOfItemCombobox(port, w->m_pOutputCombobox);
                w->m_pOutputCombobox->blockSignals(true);
                w->m_pOutputCombobox->removeItem(index);
                w->m_pOutputCombobox->blockSignals(false);
            }
        }
    }

    //  2. 删除不可用输入端口
    for (int i = 0; i < currentSourcePortList.count(); i++) {
        QString port = currentSourcePortList.at(i);
        if (!sourcePortList.contains(port)) {
            for (int a = 0; a < appList.count(); a++) {
                QString appName = appList.at(a);
                UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(appName);
                int index = indexOfItemCombobox(port, w->m_pInputCombobox);
                w->m_pInputCombobox->blockSignals(true);
                w->m_pInputCombobox->removeItem(index);
                w->m_pInputCombobox->blockSignals(false);
            }
        }
    }

    //  3. 增加可用输出端口
    for (int i = 0; i < sinkPortList.count(); i++) {
        QString port = sinkPortList.at(i);
        if (!currentSinkPortList.contains(port)) {
            for (int a = 0; a < appList.count(); a++) {
                QString appName = appList.at(a);
                UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(appName);
                w->m_pOutputCombobox->blockSignals(true);
                w->m_pOutputCombobox->addItem(port);
                w->m_pOutputCombobox->blockSignals(false);
            }
        }
    }

    //  4. 增加可用输入端口
    for (int i = 0; i < sourcePortList.count(); i++) {
        QString port = sourcePortList.at(i);
        if (!currentSourcePortList.contains(port)) {
            for (int a = 0; a < appList.count(); a++) {
                QString appName = appList.at(a);
                UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(appName);
                w->m_pInputCombobox->blockSignals(true);
                w->m_pInputCombobox->addItem(port);
                w->m_pInputCombobox->blockSignals(false);
            }
        }
    }

    //  5. 选择默认输出设备
    for (int i = 0; i < appList.count(); i++) {
        QString appName = appList.at(i);
        UkmediaAppItemWidget *w = stackWidget->findChild<UkmediaAppItemWidget*>(appName);
        w->m_pOutputCombobox->blockSignals(true);
        w->m_pOutputCombobox->setCurrentText(getAppOutputDevice(appName));
        w->m_pOutputCombobox->blockSignals(false);
        w->m_pInputCombobox->blockSignals(true);
        w->m_pInputCombobox->setCurrentText(getAppInputDevice(appName));
        w->m_pInputCombobox->blockSignals(false);

        //需求31268待2501再合入
#if 1
        if (appName.compare(QString(SYSTEM_VOLUME_CTRL)) == 0) {
            if (!sinkPortList.isEmpty()) {
                if (QString(sinkPortList.at(0)).compare(QString(tr("None"))) == 0) {
                    w->m_pVolumeSlider->setEnabled(false);
                    w->m_pVolumeBtn->setEnabled(false);
                } else {
                    w->m_pVolumeSlider->setEnabled(true);
                    w->m_pVolumeBtn->setEnabled(true);
                }
            }
        }
#endif
    }
}


int UkmediaAppCtrlWidget::indexOfItemCombobox(QString port, QComboBox *box)
{
    for (int index = 0; index < box->count(); ++index)
    {
        QString textport = box->itemText(index);
        if (textport == port) {
            return index;
        }
    }
    return -1;
}

QString UkmediaAppCtrlWidget::getAppName(QString appName)
{
    GError* error = nullptr;
    GKeyFileFlags flag = G_KEY_FILE_NONE;
    GKeyFile* keyfile = g_key_file_new();

    appName = AppDesktopFileAdaption(appName);
    QString path = "/usr/share/applications/";
    path.append(appName);
    path.append(".desktop");

    /* Some applications desktop file exist in /etc/xdg/autostart/ path */
    QFileInfo file(path);
    if (!file.exists()) {
        path = "/etc/xdg/autostart/";
        path.append(appName);
        path.append(".desktop");
    }

    QByteArray fpbyte = path.toLocal8Bit();
    char* filepath = fpbyte.data();

    if (!g_key_file_load_from_file(keyfile, filepath, flag, &error))
        qDebug() << "g_key_file_load_from_file() failed" << error->message;

    char* name= g_key_file_get_locale_string(keyfile, "Desktop Entry", "Name", nullptr, nullptr);
    QString namestr=QString::fromLocal8Bit(name);

    g_key_file_free(keyfile);

    if (appName == SYSTEM_VOLUME_CTRL)
        namestr = tr("System Volume");

    namestr = (namestr != "") ? namestr : appName;

    return namestr;
}

QString UkmediaAppCtrlWidget::getAppIcon(QString appName)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    appName = AppDesktopFileAdaption(appName);
    QString path = "/usr/share/applications/";
    path.append(appName);
    path.append(".desktop");

    /* Some applications desktop file exist in /etc/xdg/autostart/ path */
    QFileInfo file(path);
    if (!file.exists()) {
        path = "/etc/xdg/autostart/";
        path.append(appName);
        path.append(".desktop");
    }

    QByteArray fpbyte=path.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);

    char* name=g_key_file_get_locale_string(keyfile,"Desktop Entry","Icon", nullptr, nullptr);
    QString namestr=QString::fromLocal8Bit(name);

    g_key_file_free(keyfile);

    if (appName == SYSTEM_VOLUME_CTRL)
        namestr = SYSTEM_VOLUME_CTRL;

    /* If we can't find the app icon
     * we need to set a default icon for it */
    namestr = (namestr != "") ? namestr : "application-x-desktop";

    return namestr;
}

int UkmediaAppCtrlWidget::findAppDirection(QString appName)
{
    int stream = PA_STREAM_NODIRECTION;

    for (appInfo info : playbackAppInfoList) {
        if (info.name == appName) {
            stream = PA_STREAM_PLAYBACK;
            break;
        }
    }

    for (appInfo info : recordAppInfoList) {
        if (info.name == appName) {
            if (stream == PA_STREAM_PLAYBACK)
                stream = PA_STREAM_NODIRECTION;
            else
                stream = PA_STREAM_RECORD;
            break;
        }
    }
    return stream;
}

#ifdef PA_PROP_APPLICATION_MOVE
bool UkmediaAppCtrlWidget::checkAppMoveStatus(int appType, QString appName)
{
    bool moveStatus = false;

    if (appName == SYSTEM_VOLUME_CTRL)
        return true;

    if (appType == PA_STREAM_PLAYBACK) {

        //start #227383
        if ((appName == "lingmo-recorder") && (findAppDirection(appName) == PA_STREAM_NODIRECTION)) {
            return false;
        }
        //end

        for (appInfo info : playbackAppInfoList) {
            if (info.name == appName) {
                moveStatus = (info.move == "yes") ? true : false;
                break;
            }
        }
    }
    else {
        for (appInfo info : recordAppInfoList) {

            // #214516 Circumvention deals with lingmo-recorder
            if (appName == "lingmo-recorder")
                return false;

            if (info.name == appName) {
                moveStatus = (info.move == "yes") ? true : false;
                break;
            }
        }
    }

    return moveStatus;
}
#endif

int UkmediaAppCtrlWidget::valueToPaVolume(int value)
{
    return value / UKMEDIA_VOLUME_NORMAL * PA_VOLUME_NORMAL;
}

int UkmediaAppCtrlWidget::paVolumeToValue(int value)
{
    return (value / PA_VOLUME_NORMAL * UKMEDIA_VOLUME_NORMAL) + 0.5;
}

QString UkmediaAppCtrlWidget::AppDesktopFileAdaption(QString appName)
{
    //某些第三方应用获取到appName与它的desktop文件名不一致，需手动适配
    if (appName.contains("qaxbrowser"))
        appName = "qaxbrowser-safe";

    return appName;
}

UkmediaAppCtrlWidget::~UkmediaAppCtrlWidget()
{

}
