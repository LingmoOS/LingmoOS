/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "ukmedia_main_widget.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStringList>
#include <QSpacerItem>
#include <QListView>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QProcess>
#include <QApplication>
#include <QSvgRenderer>
#include <QScrollBar>
#include <QtCore/qmath.h>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QFrame>
#include <KWindowEffects>
#include <kwindowsystem.h>
#include <kwindowsystem_export.h>


#define MAINWINDOW_WIDTH 420
#define MAINWINDOW_HEIGHT 476
#define WIDGET_RADIUS 12
#define CONTENTS_MARGINS_WIDTH 0,0,0,0
#define VOLUMELINE_POS 0,140
#define SETTINGLINE_POS 0,424
#define KEYBINDINGS_CUSTOM_SCHEMA "org.lingmo.media.sound"
#define KEYBINDINGS_CUSTOM_DIR "/org/lingmo/sound/keybindings/"

#define EVENT_SOUNDS_KEY "event-sounds"
#define MAX_CUSTOM_SHORTCUTS 1000

#define FILENAME_KEY "filename"
#define NAME_KEY "name"

double transparency = 0.8;
extern bool isCheckBluetoothInput;

UkmediaMainWidget::UkmediaMainWidget(QMainWindow *parent) :
    QMainWindow(parent),
    m_languageEnvStr(getenv("LANGUAGE")),
    m_version(UkmediaCommon::getInstance().getSystemVersion())
{
    //初始化系统托盘
    m_pVolumeControl = new UkmediaVolumeControl();

    //初始化gsetting
    initGsettings();
    initStrings();
    //初始化界面
    initWidget();
    initLabelAlignment();
    //初始化托盘图标
    initSystemTrayIcon();
    //Dbus注册
    UkmediaDbusRegister();
    //处理槽函数
    dealSlot();
}

bool UkmediaMainWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if(QApplication::activeWindow() != this) {
            this->hide();
        }
    }
    return QWidget::eventFilter(obj,event);
}

void UkmediaMainWidget::paintEvent(QPaintEvent *event)
{

}

void UkmediaMainWidget::initSystemTrayIcon()
{
    soundSystemTrayIcon = new UkmediaTrayIcon(this);
    ca_context_create(&caContext);
    //为系统托盘图标添加菜单静音和声音首选项
    soundSystemTrayIcon->setToolTip(tr("Output volume control"));
#if (QT_VERSION <= QT_VERSION_CHECK(5,6,1))
    m_pSoundPreferenceAction = new QAction(tr("Sound preference"),this);
#elif (QT_VERSION > QT_VERSION_CHECK(5,6,1))
    m_pSoundPreferenceAction = new QAction(tr("Sound preference"));
#endif
    QString settingsIconStr = "document-page-setup-symbolic";
    QIcon settingsIcon = QIcon::fromTheme(settingsIconStr);
    m_pSoundPreferenceAction->setIcon(settingsIcon);
    soundSystemTrayIcon->setVisible(true);
    initPanelGSettings();
    int nOutputValue = paVolumeToValue(m_pVolumeControl->getSinkVolume());

    bool outputStatus = m_pVolumeControl->getSinkMute();
    outputVolumeDarkThemeImage(nOutputValue,outputStatus);

    //初始化右键菜单
    systemTrayMenuInit();
}

/*
 * 初始化右键菜单
 */
void UkmediaMainWidget::systemTrayMenuInit()
{
    menu = new QMenu();
    menu->setAttribute(Qt::WA_NoMouseReplay);
    soundSystemTrayIcon->setContextMenu(menu);
    //设置右键菜单
    menu->addAction(m_pSoundPreferenceAction);
}

/*
 * 初始化界面
 */
void UkmediaMainWidget::initWidget()
{
    QWidget::installEventFilter(this);//为这个窗口安装过滤器
    this->setFixedSize(MAINWINDOW_WIDTH,MAINWINDOW_HEIGHT);
    this->setAttribute(Qt::WA_TranslucentBackground,true);  //透明
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::NoFocus);

    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)){
        QPainterPath path;
        auto rect = this->rect();
        path.addRoundedRect(rect, WIDGET_RADIUS, WIDGET_RADIUS);
        KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(path.toFillPolygon().toPolygon()));   // 背景模糊
    }

    osdWidget = new UkmediaOsdDisplayWidget();
    headsetWidget = new LingmoUIMediaSetHeadsetWidget;  // osd弹窗

    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->setFocusPolicy(Qt::StrongFocus);
    this->setCentralWidget(m_pTabWidget);
    m_pTabWidget->tabBar()->setFixedWidth(this->width()+1);
    systemWidget = new UkmediaSystemVolumeWidget(m_pTabWidget);
    appWidget = new ApplicationVolumeWidget(m_pTabWidget);
    //需求31268待2501再合入
#if 1
    systemWidget->m_pSystemVolumeSlider->setEnabled(false);
    appWidget->systemVolumeSlider->setEnabled(false);
    systemWidget->m_pSystemVolumeBtn->setEnabled(false);
    appWidget->systemVolumeBtn->setEnabled(false);
#endif

    volumeLine = new Divider(this);
    volumeSettingLine = new Divider(this);
    volumeLine->move(VOLUMELINE_POS);
    volumeSettingLine->move(SETTINGLINE_POS);

    m_pTabWidget->addTab(systemWidget, tr(""));
    m_pTabWidget->addTab(appWidget,tr(""));
    m_tabBarLayout = new QHBoxLayout(this);
    m_tabBarLayout->setContentsMargins(CONTENTS_MARGINS_WIDTH);
    systemWidgetLabel = new FixLabel(tr("System Volume"));
    systemWidgetLabel->setAlignment(Qt::AlignCenter);
    appWidgetLabel = new FixLabel(tr("App Volume"));
    appWidgetLabel->setAlignment(Qt::AlignCenter);
    m_tabBarLayout->addWidget(systemWidgetLabel);
    m_tabBarLayout->addWidget(appWidgetLabel);
    m_pTabWidget->tabBar()->setLayout(m_tabBarLayout);
    m_pTabWidget->tabBar()->setProperty("setRadius", WIDGET_RADIUS);    // tabbar界面内圆角
    m_pTabWidget->tabBar()->setProperty("useTabbarSeparateLine",false); // 去掉标签页中间的分隔线
    m_pTabWidget->setAttribute(Qt::WA_TranslucentBackground,true);      // 背景透明 解决切换黑屏问题
    paintWithTrans();
}

void UkmediaMainWidget::onTransChanged()
{
    m_pTransparency = m_pTransGsettings->get("transparency").toDouble() + 0.15;
    m_pTransparency = (m_pTransparency > 1) ? 1 : m_pTransparency;
    paintWithTrans();
}

void UkmediaMainWidget::paintWithTrans()
{
    QPalette pal = m_pTabWidget->palette();
    QColor color = qApp->palette().base().color();
    color.setAlphaF(m_pTransparency);
    pal.setColor(QPalette::Base, color);
    m_pTabWidget->setPalette(pal);

    QPalette tabPal = m_pTabWidget->tabBar()->palette();
    tabPal.setColor(QPalette::Base, color);

    QColor inactiveColor = qApp->palette().window().color();
    inactiveColor.setAlphaF(0.86 *m_pTransparency);
    tabPal.setColor(QPalette::Window, inactiveColor);

    m_pTabWidget->tabBar()->setPalette(tabPal);
}

/*
 * 添加分隔线
 */
void UkmediaMainWidget::myLine(QFrame *volumeLine,QFrame *volumeSettingLine)
{
    volumeLine = new QFrame(this);
    volumeLine->setFrameShape(QFrame::NoFrame);
    volumeLine->setFrameStyle(QFrame::HLine);
    volumeLine->setFixedSize(420,1);
    volumeLine->setParent(this);
    QPalette palette = volumeLine->palette();
    QColor color = QColor(239,239,239);
    color.setAlphaF(0.1);
    palette.setColor(QPalette::WindowText, color);
    volumeLine->setPalette(palette);

    volumeSettingLine = new QFrame(this);
    volumeSettingLine->setFrameShape(QFrame::NoFrame);
    volumeSettingLine->setFrameStyle(QFrame::HLine);
    volumeSettingLine->setFixedSize(420,1);
    volumeSettingLine->setParent(this);
    palette = volumeSettingLine->palette();
    palette.setColor(QPalette::WindowText, color);
    volumeSettingLine->setPalette(palette);

    volumeLine->move(0,140);
    volumeSettingLine->move(0,424);
}

/*
 * 初始化静音灯
 */
void UkmediaMainWidget::initVolumeLed()
{
    bool outputStatus = m_pVolumeControl->getSinkMute();//获取输出设备的静音状态
    if(outputStatus) {
        QDBusMessage message =QDBusMessage::createSignal("/","org.lingmo.media","systemOutputVolumeIsMute");
        message << "mute";
        QDBusConnection::systemBus().send(message);
    }
    else {
        QDBusMessage message =QDBusMessage::createSignal("/","org.lingmo.media","systemOutputVolumeIsMute");
        message << "no";
        QDBusConnection::systemBus().send(message);
    }
    themeChangeIcons();
}


/*
 * 注册dbus
 */
void UkmediaMainWidget::UkmediaDbusRegister()
{
    QDBusConnection::sessionBus().unregisterService("org.lingmo.media");

    MediaAdaptor(this);

    QDBusConnection con=QDBusConnection::sessionBus();

    if (!con.registerService("org.lingmo.media"))
        qDebug() << "registerService failed" << con.lastError().message();

    if (!con.registerObject("/org/lingmo/media", this,
                            QDBusConnection::ExportAllSlots|
                            QDBusConnection::ExportAllSignals))
        qDebug() << "registerObject failed" << con.lastError().message();

    m_pBluetoothDbus = new Bluetooth_Dbus();
}


/**
 * @brief UkmediaMainWidget::initVolumeSlider
 * 初始化音量滑动条和输出设备列表
 **/
void UkmediaMainWidget::initVolumeSlider()
{
    resetVolumeSliderRange();

    int sinkVolume = paVolumeToValue(m_pVolumeControl->getSinkVolume());
    QString percent;
    percent = QString::number(sinkVolume);
    appWidget->systemVolumeDisplayLabel->setText(percent+"%");
    systemWidget->m_pSystemVolumeSliderLabel->setText(percent+"%");
    appWidget->systemVolumeSlider->blockSignals(true);
    systemWidget->m_pSystemVolumeSlider->blockSignals(true);
    systemWidget->m_pSystemVolumeSlider->setValue(sinkVolume);
    appWidget->systemVolumeSlider->setValue(sinkVolume);
    systemWidget->m_pSystemVolumeSlider->blockSignals(false);
    appWidget->systemVolumeSlider->blockSignals(false);

    initOutputListWidgetItem();
    initInputListWidgetItem();
    themeChangeIcons();
}

/*
 * 初始化output/input list widget的选项
 */
void UkmediaMainWidget::initOutputListWidgetItem()
{
    QString outputCardName = findCardName(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->cardMap);
    QString outputPortLabel = findOutputPortLabel(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->sinkPortName);

    if (outputCardName == "histen.algo") {
        outputPortLabel = "HUAWEI Histen";
    }
    findOutputListWidgetItem(outputCardName,outputPortLabel);

    int vol = m_pVolumeControl->getSinkVolume();
    systemWidget->m_pSystemVolumeSlider->blockSignals(true);
    appWidget->systemVolumeSlider->blockSignals(true);
    appWidget->systemVolumeSlider->setValue(paVolumeToValue(vol));
    systemWidget->m_pSystemVolumeSlider->setValue(paVolumeToValue(vol));
    systemWidget->m_pSystemVolumeSlider->blockSignals(false);
    appWidget->systemVolumeSlider->blockSignals(false);
    systemWidget->m_pSystemVolumeSliderLabel->setText(QString::number(paVolumeToValue(vol))+"%");
    appWidget->systemVolumeDisplayLabel->setText(QString::number(paVolumeToValue(vol))+"%");

    qDebug() <<"initOutputListWidgetItem" << m_pVolumeControl->defaultOutputCard << outputCardName
            <<m_pVolumeControl->sinkPortName << outputPortLabel;
}

void UkmediaMainWidget::initInputListWidgetItem()
{
    QString inputCardName = findCardName(m_pVolumeControl->defaultInputCard,m_pVolumeControl->cardMap);
    QString inputPortLabel = findInputPortLabel(m_pVolumeControl->defaultInputCard,m_pVolumeControl->sourcePortName);
    findInputListWidgetItem(inputCardName,inputPortLabel);

    //bug#165600 接入两个及以上输入设备时，切换输入设备切换不成功（切换之后默认使用和音量值的仍是上个设备）
    //华为机器，降噪模块会将主设备映射在usb设备上
    if (inputCardName.contains("usb",Qt::CaseInsensitive))
    {
        m_pVolumeControl->getModuleIndexByName("module-echo-cancel");

        QTimer::singleShot(300,this,[=](){
            qDebug() << __func__ << "unload module echo cancel" << m_pVolumeControl->findModuleIndex;
            pa_context_unload_module(m_pVolumeControl->getContext(),m_pVolumeControl->findModuleIndex,nullptr,nullptr);
        });
    }

    qDebug() <<"initInputListWidgetItem" << m_pVolumeControl->defaultInputCard << inputCardName
            <<m_pVolumeControl->sourcePortName << inputPortLabel;
}

/*
 * 滑动条值转换成音量值
 */
int UkmediaMainWidget::valueToPaVolume(int value)
{
    return value / UKMEDIA_VOLUME_NORMAL * PA_VOLUME_NORMAL;
}

/*
 * 音量值转换成滑动条值
 */
int UkmediaMainWidget::paVolumeToValue(int value)
{
    return (value / PA_VOLUME_NORMAL * UKMEDIA_VOLUME_NORMAL) + 0.5;
}

QString UkmediaMainWidget::outputVolumeToIconStr(bool status,int volume)
{
    QString iconStr;
    if (status) {
        iconStr = iconNameOutputs[0];
    }
    else if (volume <= 0 && !shortcutMute) {
        iconStr = iconNameOutputs[0];
    }
    else if(volume <= 0 && shortcutMute){
        iconStr = iconNameOutputs[0];
    }
    else if (volume > 0 && volume <= 33) {
        iconStr = iconNameOutputs[1];
    }
    else if (volume >33 && volume <= 66) {
        iconStr = iconNameOutputs[2];
    }
    else {
        iconStr = iconNameOutputs[3];
    }
    return iconStr;
}

/*
 * 处理槽函数
 */
void UkmediaMainWidget::dealSlot()
{
    if (nullptr == m_pTimer) {
        m_pTimer = new QTimer(this);
    }
    m_pTimer->setInterval(100);
    m_pTimer->setSingleShot(true);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));

    QDBusInterface *iface = new QDBusInterface("org.freedesktop.login1",
                                               "/org/freedesktop/login1",
                                               "org.freedesktop.login1.Manager",
                                               QDBusConnection::systemBus(),
                                               this);
    connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));

    //根据设备检测发来的信号，显示相应的提示窗
    connect(m_pVolumeControl,SIGNAL(device_changed_signal(QString)),this,SLOT(deviceChangedShowSlot(QString)));

    //声卡包含headsetmic 端口时插入4段式mic需要设置对应的port
    QDBusConnection::sessionBus().connect( QString(), "/", "org.lingmo.media", "headsetJack", this, SLOT(setHeadsetPort(QString)));
    QTimer::singleShot(50, this, SLOT(initVolumeSlider()));

    connect(m_pVolumeControl,&UkmediaVolumeControl::updateMute,this,[=](bool state) {
            bool outputStatus = m_pVolumeControl->getSinkMute();//获取输出设备的静音状态
            if(outputStatus) {
                QDBusMessage message =QDBusMessage::createSignal("/","org.lingmo.media","systemOutputVolumeIsMute");
                message << "mute";
                QDBusConnection::systemBus().send(message);
            }
            else {
                QDBusMessage message =QDBusMessage::createSignal("/","org.lingmo.media","systemOutputVolumeIsMute");
                message << "no";
                QDBusConnection::systemBus().send(message);
            }
            themeChangeIcons();
            sendUpdateMuteSignal(SoundType::SINK, state);
        });

        connect(m_pVolumeControl,&UkmediaVolumeControl::updateSourceMute,this,[=](bool state) {
            bool inputStatus = m_pVolumeControl->getSourceMute();//获取输入设备的静音状态
            if(inputStatus) {
                QDBusMessage message =QDBusMessage::createSignal("/","org.lingmo.media","systemInputVolumeIsMute");
                message << "mute";
                QDBusConnection::systemBus().send(message);
            }
            else {
                QDBusMessage message =QDBusMessage::createSignal("/","org.lingmo.media","systemInputVolumeIsMute");
                message << "no";
                QDBusConnection::systemBus().send(message);
            }
            themeChangeIcons();
            sendUpdateMuteSignal(SoundType::SOURCE, state);
        });

    connect(m_pVolumeControl,&UkmediaVolumeControl::updateVolume,this,[=](int value){
        bool outputStatus = m_pVolumeControl->getSinkMute();
        qDebug() << "UpdateSinkVolume" << value << paVolumeToValue(value) << outputStatus;
        QString percent;
        percent = QString::number(paVolumeToValue(value));
        appWidget->systemVolumeDisplayLabel->setText(percent+"%");
        systemWidget->m_pSystemVolumeSliderLabel->setText(percent+"%");
        appWidget->systemVolumeSlider->blockSignals(true);
        systemWidget->m_pSystemVolumeSlider->blockSignals(true);
        systemWidget->m_pSystemVolumeSlider->setValue(paVolumeToValue(value));
        appWidget->systemVolumeSlider->setValue(paVolumeToValue(value));
        systemWidget->m_pSystemVolumeSlider->blockSignals(false);
        appWidget->systemVolumeSlider->blockSignals(false);
//        initOutputListWidgetItem();
        themeChangeIcons();
        sendUpdateVolumeSignal(SoundType::SINK, paVolumeToValue(value));

        /* isPlay避免第一次运行同步音量时响起提示音
         * m_pTimer快捷键等其他方式播放提示音 */
        if (isPlay && m_pTimer) {
            m_pTimer->start();
        } else if (!isPlay && !m_pVolumeControl->defaultSinkName.contains("auto_null")) {
            isPlay = true;
        }
    });

    connect(m_pVolumeControl,&UkmediaVolumeControl::updateSourceVolume,this,[=](int value){
        bool inputStatus = m_pVolumeControl->getSourceMute();
        qDebug() << "UpdateSourceVolume:" << value << paVolumeToValue(value) << inputStatus;
        initInputListWidgetItem();
        themeChangeIcons();
        sendUpdateVolumeSignal(SoundType::SOURCE, paVolumeToValue(value));
    });

    connect(QApplication::desktop(),&QDesktopWidget::resized,this,[=](){
        if(this->isHidden())
            return;
        else
            this->hide();
    });

    connect(QApplication::desktop(),&QDesktopWidget::screenCountChanged,this,[=](){
        if(this->isHidden())
            return;
        else
            this->hide();
    });

    connect(m_pVolumeControl, SIGNAL(addSinkInputSignal(QString, QString, int, int, int)), this, SLOT(addAppToAppwidget(QString, QString, int, int, int)));
    connect(m_pVolumeControl, SIGNAL(removeSinkInputSignal(QString)), this, SLOT(removeSubAppWidget(QString)));
    connect(m_pVolumeControl, SIGNAL(sinkInputVolumeChangedSignal(QString, QString, int)), this, SLOT(sinkInputVolumeChangedSlot(QString, QString, int)));
    connect(m_pVolumeControl,SIGNAL(deviceChangedSignal()),this,SLOT(updateListWidgetItemSlot()));
    QDBusConnection::sessionBus().connect( QString(), "/", "org.lingmo.music", "sinkInputVolumeChanged", this, SLOT(updateAppVolume(QString,int,bool)));
    QDBusConnection::sessionBus().connect( QString(), "/", "org.lingmo.video", "sinkInputVolumeChanged", this, SLOT(updateAppVolume(QString,int,bool)));

    connect(systemWidget->m_pOutputListWidget,&QListWidget::itemClicked,this,[=](){
        if (outputListWidgetRow != systemWidget->m_pOutputListWidget->currentRow()) {
            outputListWidgetCurrentRowChangedSlot(systemWidget->m_pOutputListWidget->currentRow());
            outputListWidgetRow = systemWidget->m_pOutputListWidget->currentRow();
        }
    });

    connect(systemWidget->m_pInputListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(inputListWidgetCurrentRowChangedSlot(int)));

    /*
     * 完整模式下,应用音量界面，当滑动条值改变时更改系统音量
     */
    connect(appWidget->systemVolumeSlider,SIGNAL(valueChanged(int)),this,SLOT(advancedSystemSliderChangedSlot(int)));

    connect(systemWidget->m_pSystemVolumeSlider,SIGNAL(valueChanged(int)),this,SLOT(systemVolumeSliderChangedSlot(int)));

    connect(systemWidget->m_pSystemVolumeSlider, &AudioSlider::blueValueChanged, this, &UkmediaMainWidget::systemVolumeSliderChangedSlotInBlue);
    connect(appWidget->systemVolumeSlider, &AudioSlider::blueValueChanged, this, &UkmediaMainWidget::systemVolumeSliderChangedSlotInBlue);

    connect(systemWidget->m_pSystemVolumeBtn,SIGNAL(clicked()),this,SLOT(systemVolumeButtonClickedSlot()));
    connect(appWidget->systemVolumeBtn,SIGNAL(clicked()),this,SLOT(appWidgetMuteButtonCLickedSlot()));

    /*
     * 当无任何控制音量的界面显示时，鼠标滚轮向上滚动音量托盘图标增加音量
     * 向下滚动减少音量值
     */
    connect(soundSystemTrayIcon,SIGNAL(wheelRollEventSignal(bool)),this,SLOT(trayWheelRollEventSlot(bool)));

    connect(soundSystemTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),\
            this,SLOT(activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason)));
    connect(m_pSoundPreferenceAction,SIGNAL(triggered()),this,SLOT(jumpControlPanelSlot()));
    connect(m_pVolumeControl,SIGNAL(updatePortSignal()),this,SLOT(updateDevicePort()));

    connect(m_pVolumeControl,SIGNAL(checkDeviceSelectionSianal(const pa_card_info *)),this,SLOT(checkAudioDeviceSelectionNeeded(const pa_card_info *)));

    connect(m_pVolumeControl,&UkmediaVolumeControl::removeSinkSignal,[=](){
        qDebug() << "removre sink signal";
    });
    connect(appWidget->volumeSettingButton,SIGNAL(clicked()),this,SLOT(jumpControlPanelSlot()));
    connect(systemWidget->volumeSettingButton,SIGNAL(clicked()),this,SLOT(jumpControlPanelSlot()));
    monitorSessionStatus();

    connect(m_pVolumeControl, &UkmediaVolumeControl::bluetoothBatteryChanged, this, &UkmediaMainWidget::batteryLevelChanged);
    connect(m_pVolumeControl, &UkmediaVolumeControl::initBlueDeviceVolumeSig, this, &UkmediaMainWidget::initBlueDeviceVolume);
}

void UkmediaMainWidget::initGsettings()
{
    //检测系统主题
    if (QGSettings::isSchemaInstalled(LINGMO_THEME_SETTING)){
        m_pThemeSetting = new QGSettings(LINGMO_THEME_SETTING);
        m_pFontSetting = new QGSettings(LINGMO_THEME_SETTING);
        QString fontType;
        if (m_pThemeSetting->keys().contains("styleName")) {
            mThemeName = m_pThemeSetting->get(LINGMO_THEME_NAME).toString();
        }
        if (m_pFontSetting->keys().contains("systemFont")) {
            fontType = m_pFontSetting->get("systemFont").toString();
        }
        if (m_pFontSetting->keys().contains("systemFontSize")) {
            int font = m_pFontSetting->get("system-font-size").toInt();
            QFont fontSize(fontType,font);
//            appWidget->systemVolumeLabel->setFont(fontSize);
        }
        connect(m_pFontSetting,SIGNAL(changed(const QString &)),this,SLOT(fontSizeChangedSlot(const QString &)));
        connect(m_pThemeSetting, SIGNAL(changed(const QString &)),this,SLOT(lingmoThemeChangedSlot(const QString &)));
    }
    //获取声音gsettings值
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        m_pSoundSettings = new QGSettings(KEY_SOUNDS_SCHEMA);
        if (m_pSoundSettings->keys().contains("dnsNoiseReduction")) {
            QTimer *timer = new QTimer();
            timer->start(1000);
            connect(timer,&QTimer::timeout,[=](){
                switchModuleEchoCancel();
                delete timer;
            });
        }

        connect(m_pVolumeControl, SIGNAL(paContextReady()), this, SLOT(switchModuleLoopBack()));
        connect(m_pVolumeControl, SIGNAL(paContextReady()),this,SLOT(switchMonoAudio()));
        connect(m_pSoundSettings, SIGNAL(changed(const QString &)),this,SLOT(soundSettingChanged(const QString &)));

    }
    //监听开机音乐的开关键值
    if (QGSettings::isSchemaInstalled(LINGMO_SWITCH_SETTING)) {
        m_pStartUpSetting = new QGSettings(LINGMO_SWITCH_SETTING);
        if (m_pStartUpSetting->keys().contains("startupMusic")) {
           bool state = m_pStartUpSetting->get(LINGMO_STARTUP_MUSIC_KEY).toBool();//开机音乐
           m_pVolumeControl->insertJson(JsonType::JSON_TYPE_USERINFO, AUDIO_STRATUP_SOUNDEFFECT_KEY, state);
        }
        connect(m_pStartUpSetting, SIGNAL(changed(const QString &)),this,SLOT(soundSettingChanged(const QString &)));
    }

    //获取透明度
    if(QGSettings::isSchemaInstalled(TRANSPARENCY_GSETTINGS)) {
        m_pTransGsettings = new QGSettings(TRANSPARENCY_GSETTINGS);
        if(m_pTransGsettings->keys().contains(QString("transparency"))) {
            m_pTransparency=m_pTransGsettings->get("transparency").toDouble() + 0.15;
            m_pTransparency = (m_pTransparency > 1) ? 1 : m_pTransparency;
            connect(m_pTransGsettings, SIGNAL(changed(const QString &)), this, SLOT(onTransChanged()));
        }
    }

    if(QGSettings::isSchemaInstalled(LINGMO_GLOBALTHEME_SETTINGS)){
        m_pSoundThemeSetting = new QGSettings(LINGMO_GLOBALTHEME_SETTINGS);
        if(m_pSoundThemeSetting->keys().contains("globalThemeName")){
            QString soundThemestr = m_pSoundThemeSetting->get(GLOBAL_THEME_NAME).toString();
            if(m_pSoundSettings->keys().contains("themeName") && soundThemestr != "custom"){
                m_pSoundSettings->set(SOUND_THEME_KEY,soundThemestr);
            }
        }
        connect(m_pSoundThemeSetting, SIGNAL(changed(const QString &)),this,SLOT(soundThemeChangedSlot(const QString &)));
    }
}

void UkmediaMainWidget::initStrings()
{
    const static QString s_login1Name = QStringLiteral("logind");
    const static QString s_login1Service = QStringLiteral("org.freedesktop.login1");
    const static QString s_login1Path = QStringLiteral("/org/freedesktop/login1");
    const static QString s_login1ManagerInterface = QStringLiteral("org.freedesktop.login1.Manager");
    const static QString s_login1SeatInterface = QStringLiteral("org.freedesktop.login1.Seat");
    const static QString s_login1SessionInterface = QStringLiteral("org.freedesktop.login1.Session");
    const static QString s_login1ActiveProperty = QStringLiteral("Active");
    const static QString s_dbusPropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");

    m_sessionControllerName = s_login1Name;
    m_sessionControllerService = s_login1Service;
    m_sessionControllerPath = s_login1Path;
    m_sessionControllerManagerInterface = s_login1ManagerInterface;
    m_sessionControllerSeatInterface = s_login1SeatInterface;
    m_sessionControllerSessionInterface = s_login1SessionInterface;
    m_sessionControllerActiveProperty = s_login1ActiveProperty;
    m_sessionControllerPropertiesInterface = s_dbusPropertiesInterface;
}

void UkmediaMainWidget::initLabelAlignment()
{
    // 哈维柯语言环境下默认做了label右对齐方式处理，因此只需要在其他语言环境下对这些label做左居中对齐方式处理即可
    if (!UkmediaCommon::getInstance().isHWKLanguageEnv(m_languageEnvStr)) {
        systemWidget->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        appWidget->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
}

/*
 *  当检测到4段式3.5mm耳机插入时(声卡包含headsetmic端口)
 */
void UkmediaMainWidget::setHeadsetPort(QString str)
{
    qDebug() << "setHeadsetPort " << str << m_pVolumeControl->sinkIndex;
    /*
     *output: headphone
     *input: intel mic
    */
    if (strcmp(str.toLatin1().data(),"headphone") == 0) {
        m_pVolumeControl->setSinkPort(m_pVolumeControl->defaultSinkName,m_pHeadphonesName);
        m_pVolumeControl->setSourcePort(m_pVolumeControl->defaultSourceName,m_pInternalmicName);

            /*
             *当需要设置的输入端口不在同一个sink上时，需要设置默认的输入设备
            */


    } /*output: headphone     input: headset mic*/
    else if (strcmp(str.toLatin1().data(),"headset") == 0) {
        m_pVolumeControl->setSinkPort(m_pVolumeControl->defaultSinkName,m_pHeadphonesName);
        m_pVolumeControl->setSourcePort(m_pVolumeControl->defaultSourceName,m_pHeadsetmicName);
            /*
             *当需要设置的输入端口不在同一个sink上时，需要设置默认的输入设备
            */


    }/*output: speaker     input: headphone mic*/
    else if (strcmp(str.toLatin1().data(),"headphone mic") == 0) {
        m_pVolumeControl->setSinkPort(m_pVolumeControl->defaultSinkName,m_pInternalspkName);
        m_pVolumeControl->setSourcePort(m_pVolumeControl->defaultSourceName,m_pHeadphonemicName);
            /*
             *当需要设置的输入端口不在同一个sink上时，需要设置默认的输入设备
            */

    }
}

/*
 * 音量改变提示：策略是停止滑动时，播放提示声
 */
void UkmediaMainWidget::handleTimeout()
{
    playAlertSound("audio-volume-change");
}

//2303侧边栏使用usd声音接口，不再需要此dbus
void UkmediaMainWidget::setVolumeSettingValue(int value)
{
    if (QGSettings::isSchemaInstalled(LINGMO_VOLUME_BRIGHTNESS_GSETTING_ID)) {
        if (m_pVolumeSetting->keys().contains("volumesize")) {
            if(value == 0)
                m_pVolumeControl->setSinkMute(true);
            if (m_pSoundSettings->keys().contains("volumeIncrease")) {
                if(m_pSoundSettings->get("volume-increase").toBool()){
                    int value_increase = (value / 1.25) +0.5;
                    m_pVolumeSetting->set(LINGMO_VOLUME_KEY,value_increase);
                }
                else
                    m_pVolumeSetting->set(LINGMO_VOLUME_KEY,value);
            }
        }
    }
}

void UkmediaMainWidget::setVolumeSettingMute(bool state)
{
    if (QGSettings::isSchemaInstalled(LINGMO_VOLUME_BRIGHTNESS_GSETTING_ID)) {
        if (m_pVolumeSetting->keys().contains("soundstate")) {
            m_pVolumeSetting->blockSignals(true);
            m_pVolumeSetting->set(LINGMO_VOLUME_STATE,state);
            m_pVolumeSetting->blockSignals(false);
        }
    }
}

void UkmediaMainWidget::themeChangeIcons()
{
    int nInputValue = paVolumeToValue(m_pVolumeControl->getSourceVolume());
    int nOutputValue = paVolumeToValue(m_pVolumeControl->getSinkVolume());
    bool inputStatus = m_pVolumeControl->getSourceMute();
    bool outputStatus = m_pVolumeControl->getSinkMute();

    inputVolumeDarkThemeImage(nInputValue,inputStatus);
    outputVolumeDarkThemeImage(nOutputValue,outputStatus);

    systemWidget->m_pSystemVolumeBtn->repaint();
    appWidget->systemVolumeBtn->repaint();
    soundSystemTrayIcon->setToolTip(tr("Current volume:")+QString::number(nOutputValue)+"%");
}

/*
 * 隐藏窗口
 */
void UkmediaMainWidget::hideWindow()
{
    this->hide();
}

/*
 * 字体改变需更新字体大小
 */
void UkmediaMainWidget::fontSizeChangedSlot(const QString &themeStr)
{
    QString fontType;
    if (m_pFontSetting->keys().contains("systemFont")) {
        fontType = m_pFontSetting->get("systemFont").toString();
    }
    if (m_pFontSetting->keys().contains("systemFontSize")) {
        int font = m_pFontSetting->get("system-font-size").toInt();
        QFont fontSize(fontType,font);
    }
}

void UkmediaMainWidget::volumeSettingChangedSlot(const QString &key)
{
    if(key != "volumesize")
        return;

    if (m_pVolumeSetting->keys().contains("volumesize")) {
        int valueSetting = m_pVolumeSetting->get(LINGMO_VOLUME_KEY).toInt();
        if (valueSetting == 0)
            m_pVolumeControl->setSinkMute(true);
        if (m_pSoundSettings->keys().contains("volumeIncrease")) {
            if(m_pSoundSettings->get("volume-increase").toBool()) {
                int value_increase = (valueSetting * 1.25) +0.5;
                systemWidget->m_pSystemVolumeSlider->setValue(value_increase);
            }
            else
                systemWidget->m_pSystemVolumeSlider->setValue(valueSetting);
        }
    }
    if (QGSettings::isSchemaInstalled(LINGMO_VOLUME_BRIGHTNESS_GSETTING_ID)) {
        if (m_pVolumeSetting->keys().contains("soundstate")) {
            bool status = m_pVolumeSetting->get(LINGMO_VOLUME_STATE).toBool();

        }
    }
}

/*
 * 系统主题改变
 */
void UkmediaMainWidget::lingmoThemeChangedSlot(const QString &themeStr)
{
    qDebug() << "lingmoThemeChangedSlot" << mThemeName;

    if (m_pThemeSetting->keys().contains("styleName")) {
        mThemeName = m_pThemeSetting->get(LINGMO_THEME_NAME).toString();
    }

    themeChangeIcons();
    onTransChanged();
    Q_EMIT qApp->paletteChanged(qApp->palette());
    this->repaint();
}

/*
 * 系统音效主题改变
 */
void UkmediaMainWidget::soundThemeChangedSlot(const QString &soundThemeStr)
{
    if(m_pSoundThemeSetting->keys().contains("globalThemeName")){
        QString soundstr = m_pSoundThemeSetting->get(GLOBAL_THEME_NAME).toString();
        if(m_pSoundSettings->keys().contains("themeName") && soundstr != "custom")
            m_pSoundSettings->set(SOUND_THEME_KEY,soundstr);
    }
}


/*
 * 滑动条控制输出音量
 */
void UkmediaMainWidget::systemVolumeSliderChangedSlot(int value)
{
    QString percent;
    percent = QString::number(value);

    appWidget->systemVolumeSlider->blockSignals(true);
    appWidget->systemVolumeSlider->setValue(value);
    appWidget->systemVolumeSlider->blockSignals(false);
    appWidget->systemVolumeDisplayLabel->setText(percent+"%");
    systemWidget->m_pSystemVolumeSliderLabel->setText(percent+"%");

    //蓝牙a2dp模式下滑动条跳动问题，以间隔为10设置音量
    if (m_pVolumeControl->defaultSinkName.contains("a2dp_sink"))
        return;

    int volume = valueToPaVolume(value);
    m_pVolumeControl->getDefaultSinkIndex();
    m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,volume);

    firstEnterSystem = false;
}

/*
 * 完整模式下，应用音量选项中系统音量控制输出音量值
 */
void UkmediaMainWidget::advancedSystemSliderChangedSlot(int value)
{
    QString percent;
    percent = QString::number(value);

    systemWidget->m_pSystemVolumeSlider->blockSignals(true);
    systemWidget->m_pSystemVolumeSlider->setValue(value);
    systemWidget->m_pSystemVolumeSlider->blockSignals(false);
    appWidget->systemVolumeDisplayLabel->setText(percent+"%");
    systemWidget->m_pSystemVolumeSliderLabel->setText(percent+"%");

    //蓝牙a2dp模式下滑动条跳动问题，以间隔为10设置音量
    if (m_pVolumeControl->defaultSinkName.contains("a2dp_sink"))
        return;

    int volume = valueToPaVolume(value);
    m_pVolumeControl->getDefaultSinkIndex();
    m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,volume);
}

void UkmediaMainWidget::systemVolumeSliderChangedSlotInBlue(int value)
{
    if (!m_pVolumeControl->defaultSinkName.contains("a2dp_sink")) {
        systemWidget->m_pSystemVolumeSlider->isMouseWheel = false;
        appWidget->systemVolumeSlider->isMouseWheel = false;
        return;
    }

    qDebug() << "Special Handling Adjust volume in Bluetooth a2dp mode" << value ;

    QString percent;
    percent = QString::number(value);

    appWidget->systemVolumeSlider->blockSignals(true);
    appWidget->systemVolumeSlider->setValue(value);
    appWidget->systemVolumeSlider->blockSignals(false);
    appWidget->systemVolumeDisplayLabel->setText(percent+"%");

    systemWidget->m_pSystemVolumeSlider->blockSignals(true);
    systemWidget->m_pSystemVolumeSlider->setValue(value);
    systemWidget->m_pSystemVolumeSlider->blockSignals(false);
    systemWidget->m_pSystemVolumeSliderLabel->setText(percent+"%");

    int volume = valueToPaVolume(value);
    m_pVolumeControl->getDefaultSinkIndex();
    m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,volume);

}

void UkmediaMainWidget::systemVolumeButtonClickedSlot()
{
    m_pVolumeControl->setSinkMute(!m_pVolumeControl->sinkMuted);

    QTimer * time = new QTimer;
    time->start(100);
    connect(time,&QTimer::timeout,[=](){

        themeChangeIcons();
        time->disconnect();
        delete time;
    });

}

/*
 * 完整模式中应用音量的静音控制
 */
void UkmediaMainWidget::appWidgetMuteButtonCLickedSlot()
{
    m_pVolumeControl->setSinkMute(!m_pVolumeControl->sinkMuted);

    QTimer * time = new QTimer;
    time->start(100);
    connect(time,&QTimer::timeout,[=](){

        themeChangeIcons();
        time->disconnect();
        delete time;
    });
}

/*
 * 声音托盘的触发事件，包括鼠标左键点击，双击，滚轮，右击
 */
void UkmediaMainWidget::activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason reason)
{
    QString platform = QGuiApplication::platformName();
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)){
        const KWindowInfo info(this->winId(), NET::WMState);
        if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager)) {
            KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
        }
    }

    switch(reason) {
    //鼠标中间键点击图标
    case QSystemTrayIcon::MiddleClick: {
        if (this->isHidden()) {
            mouseMeddleClickedTraySlot();
        }
        else {
            hideWindow();
        }
        break;
    }
        //鼠标左键点击图标
    case QSystemTrayIcon::Trigger: {
        if(this->isVisible()){
            hideWindow();
            break;
        }
        advancedWidgetShow();
        kdk::WindowManager::setSkipTaskBar(this->windowHandle(),true);
        kdk::WindowManager::setSkipSwitcher(this->windowHandle(),true);
        break;
    }
        //鼠标左键双击图标
    case QSystemTrayIcon::DoubleClick: {
        hideWindow();
        break;
    }
    case QSystemTrayIcon::Context:{
        if(this ->isHidden()) {
            menu->popup(QCursor::pos());
        }
        else{
            this->hideWindow();
        }
        break;
    }
    default:
        break;
    }
}

/*
 * 鼠标滚轮点击托盘图标，设置输出音量的静音状态
 */
void UkmediaMainWidget::mouseMeddleClickedTraySlot()
{
    m_pVolumeControl->setSinkMute(!m_pVolumeControl->sinkMuted);

    QTimer * time = new QTimer;
    time->start(100);
    connect(time,&QTimer::timeout,[=](){

        themeChangeIcons();
        time->disconnect();
        delete time;
    });
}

/*
 * 鼠标滚轮在声音托盘上滚动设置输出音量值
 */
void UkmediaMainWidget::trayWheelRollEventSlot(bool step)
{
    int volume = systemWidget->m_pSystemVolumeSlider->value();
    if (step) {
        systemWidget->m_pSystemVolumeSlider->setValue(volume+5);
    }
    else {
        systemWidget->m_pSystemVolumeSlider->setValue(volume-5);
    }
}

/*
 * 点击菜单中声音设置跳转到控制面板的声音控制
 */
void UkmediaMainWidget::jumpControlPanelSlot()
{
    m_process = new QProcess(0);
    m_process->start("lingmo-control-center -m Audio");
    m_process->waitForStarted();
    this->hide();
    return;
}

void UkmediaMainWidget::drawImagColorFromTheme(LingmoUIButtonDrawSvg *button, QString iconStr)
{
    button->themeIcon.image = QIcon::fromTheme(iconStr).pixmap(24,24).toImage();

    if (mThemeName == LINGMO_THEME_LIGHT)
        button->themeIcon.color = QColor(0,0,0,216);
    else
        button->themeIcon.color = QColor(255,255,255,216);
}

/*
    深色主题时输出音量图标
*/
void UkmediaMainWidget::outputVolumeDarkThemeImage(int value,bool status)
{
    QString iconStr = outputVolumeToIconStr(status,value);

    systemWidget->m_pSystemVolumeBtn->setIcon(QIcon::fromTheme(iconStr));
    appWidget->systemVolumeBtn->setIcon(QIcon::fromTheme(iconStr));
    bool isMute = m_pVolumeControl->getSinkMute();
    if(iconStr == "audio-volume-muted-symbolic" && isMute) {
        systemWidget->m_pSystemVolumeBtn->setChecked(false);
        appWidget->systemVolumeBtn->setChecked(false);
    }
    else {
        systemWidget->m_pSystemVolumeBtn->setChecked(true);
        appWidget->systemVolumeBtn->setChecked(true);
    }
    soundSystemTrayIcon->setIcon(QIcon::fromTheme(iconStr));

}

/*
    输入音量图标
*/
void UkmediaMainWidget::inputVolumeDarkThemeImage(int value,bool status)
{
    QString inputIconStr;
    if (status) {
        inputIconStr = iconNameInputs[0];
    }
    else if (value <= 0) {
        inputIconStr = iconNameInputs[0];
    }
    else if (value > 0 && value <= 33) {
        inputIconStr = iconNameInputs[1];
    }
    else if (value >33 && value <= 66) {
        inputIconStr = iconNameInputs[2];
    }
    else {
        inputIconStr = iconNameInputs[3];
    }
}

/*
 * 绘制高亮颜色图标
 */
QPixmap UkmediaMainWidget::drawLightColoredPixmap(const QPixmap &source)
{
    QColor gray(255,255,255);
    QColor standard (0,0,0);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

/*
 * 绘制深色颜色图标
 */
QPixmap UkmediaMainWidget::drawDarkColoredPixmap(const QPixmap &source)
{
    QColor gray(255,255,255);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                }
                else {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

void UkmediaMainWidget::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id)) {
        if (m_panelGSettings == nullptr) {
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
            }
            if (key == PANEL_SIZE_KEY) {
                m_panelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
            }
            if (this->isVisible()) {
                advancedWidgetShow();
            }
        });
    }
}

QRect UkmediaMainWidget::caculatePosition()
{
    QRect availableGeo = QGuiApplication::screenAt(QCursor::pos())->geometry();
    int x, y;
    int margin = 8;

    switch (m_panelPosition)
    {
    case PanelPosition::Top:
    {
        if (UkmediaCommon::getInstance().isHWKLanguageEnv(m_languageEnvStr)) {
            x = margin;
            y = availableGeo.y() + m_panelSize + margin;
        }
        else {
            x = availableGeo.x() + availableGeo.width() - this->width() - margin;
            y = availableGeo.y() + m_panelSize + margin;
        }

    }
        break;
    case PanelPosition::Bottom:
    {
        if (UkmediaCommon::getInstance().isHWKLanguageEnv(m_languageEnvStr)) {
            x = margin;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - margin;
        }
        else {
            x = availableGeo.x() + availableGeo.width() - this->width() - margin;
            y = availableGeo.y() + availableGeo.height() - m_panelSize - this->height() - margin;
        }

    }
        break;
    case PanelPosition::Left:
    {
        x = availableGeo.x() + m_panelSize + margin;
        y = availableGeo.y() + availableGeo.height() - this->height() - margin;
    }
        break;
    case PanelPosition::Right:
    {
        x = availableGeo.x() + availableGeo.width() - m_panelSize - this->width() - margin;
        y = availableGeo.y() + availableGeo.height() - this->height() - margin;
    }
        break;
    }

    return QRect(x, y, this->width(), this->height());
}

/*
 * 完整模式界面的显示
 */
void UkmediaMainWidget::advancedWidgetShow()
{
    kdk::WindowManager::setSkipSwitcher(this->windowHandle(),true);
    QRect rect = caculatePosition();
    QString platform = QGuiApplication::platformName();
    qDebug() << "platform:" << platform;
    if(!platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)){
        this->setGeometry(rect);
        this->showNormal();
    }
    else {
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
        this->show();
        kdk::WindowManager::setGeometry(this->windowHandle(), rect);
    }
    this->raise();
    this->activateWindow();
}

QString UkmediaMainWidget::appNameToIconName(QString appName, QString appIconName)
{
    QString iconName = "/usr/share/applications/";
    qDebug()<< "appNameToIconName" << appName << appIconName;

    iconName.append(appIconName);
    iconName.append(".desktop");

    /* Some applications desktop file exist in /etc/xdg/autostart/ path */
    QFileInfo file(iconName);
    if (!file.exists()) {
        iconName = "/etc/xdg/autostart/";
        iconName.append(appIconName);
        iconName.append(".desktop");
    }

    return iconName;
}

void UkmediaMainWidget::addAppToAppwidget(QString appName, QString appIconName, int index, int value, int channel)
{
    //设置应用的图标
    QString iconName = appNameToIconName(appName,appIconName);
    QString description = appName;
    QString pAppName = getAppName(iconName);
    QString pAppIcon = getAppIcon(iconName);
    initSubApplicationWidget(pAppIcon);
    qDebug() << "appName:" << appName << "appIconName:" << appIconName << "pAppIcon:" << pAppIcon << "iconName:" << iconName;

    appWidget->app_volume_list->append(appIconName);
    QString appSliderStr = appName;
    QString appMuteBtnlStr = appName;
    QString appVolumeLabelStr = appName;

    QStringList strList;
    strList<<appMuteBtnlStr<<appSliderStr;
    appWidget->appSlider->setObjectName(appSliderStr);
    appWidget->appMuteBtn->setObjectName(appIconName);
    appWidget->appVolumeLabel->setObjectName(appVolumeLabelStr);

    //初始化应用音量滑动条
    int sliderValue = paVolumeToValue(value);
    appWidget->appIconBtn->setCheckable(false);
    appWidget->appIconBtn->setChecked(false);
    appWidget->appSlider->blockSignals(true);
    appWidget->appSlider->setValue(sliderValue);
    appWidget->appVolumeLabel->setText(QString::number(sliderValue)+"%");
    appWidget->appSlider->blockSignals(false);
    qDebug() << "初始化应用音量滑动条:" << appWidget->appSlider->value() << appSliderStr;

    /*滑动条控制应用音量*/
    connect(appWidget->appSlider,&QSlider::valueChanged,[=](int value)
    {
        bool isMute = false;
        int volume = valueToPaVolume(value);
        QString appName = appSliderStr.split("Slider").at(0);

        qDebug() << "调节应用音量滑动条的index和appName" << index << appName;

        //因为一个应用可能产生多个sinkinput，所以要对相同description的所有index设置音量，遍历indexmap设置
        QMap<int, QString>::iterator it;
        bool isSinkInput = false;   //应用可能同时产生sinkinput和source-output，此时需要忽略设置输入音量
        for(it = m_pVolumeControl->sinkInputIndexMap.begin(); it != m_pVolumeControl->sinkInputIndexMap.end();){
            if(it.value() == appName){
                isSinkInput = true;
                m_pVolumeControl->setSinkInputVolume(it.key(),volume,channel);
            }
            it++;
        }
        for(it = m_pVolumeControl->sourceOutputIndexMap.begin(); it != m_pVolumeControl->sourceOutputIndexMap.end();){
            if(it.value() == appName && !isSinkInput){
                m_pVolumeControl->setSourceOutputVolume(it.key(),volume,channel);
            }
            it++;
        }

        if(lingmoVideoVolumeSignal) {
            lingmoVideoVolumeSignal = false;
        }
        else {
            QDBusMessage message =QDBusMessage::createSignal("/", "org.lingmo.media", "sinkVolumeChanged");
            message<<pAppIcon<<value<<false;
            QDBusConnection::sessionBus().send(message);
        }

        QLabel *label = appWidget->findChild<QLabel*> (appVolumeLabelStr);
        if (label == nullptr)
            return;
        label->setText(QString::number(value)+"%");

        QSlider *slider = appWidget->findChild<QSlider*>(appSliderStr);
        if (slider == nullptr)
            return;
        QPushButton *btn = appWidget->findChild<QPushButton*>(appMuteBtnlStr);
        if (btn == nullptr)
            return;
        QString audioIconStr = outputVolumeToIconStr(isMute,value);
        QSize iconSize(24,24);
        if (mThemeName == LINGMO_THEME_DEFAULT) {
            btn->setIcon(QIcon(drawDarkColoredPixmap((QIcon::fromTheme(audioIconStr).pixmap(iconSize)))));
        }
        else if (mThemeName == LINGMO_THEME_DARK || mThemeName == LINGMO_THEME_DEFAULT) {
            btn->setIcon(QIcon(drawLightColoredPixmap((QIcon::fromTheme(audioIconStr).pixmap(iconSize)))));
        }
    });

    connect(appWidget->appMuteBtn,&QPushButton::clicked,[=]()
    {
        bool isMute = false;
        int volume = -1;

        isMute = m_pVolumeControl->getSinkInputMuted(description);
        qDebug() << "appMuteBtn clicked" << "muted" << isMute << description <<isAppMuteBtnPress;

        volume = int(volume*100/65536.0 + 0.5);
        if (sinkInputMute != isMute) {
            m_pVolumeControl->setSinkInputMuted(description, !isMute);
            if (isAppMuteBtnPress) {
                isAppMuteBtnPress = false;
            }
        }
        else {
            if(lingmoVideoMuteSignal)
                lingmoVideoMuteSignal = false;
            else{
                QDBusMessage message =QDBusMessage::createSignal("/", "org.lingmo.media", "sinkVolumeChanged");
                message<<pAppIcon<<volume<<!isMute;
                QDBusConnection::sessionBus().send(message);
            }
        }
    });
}

void UkmediaMainWidget::sinkInputVolumeChangedSlot(QString appName, QString appIconName, int value)
{
    QString appSliderStr = appName;
    appSliderStr.append("Slider");
    QString appLabelStr = appName;
    appLabelStr.append("VolumeLabel");

    QLabel *label = appWidget->displayAppVolumeWidget->findChild<QLabel *>(appName);
    AudioSlider *slider = appWidget->displayAppVolumeWidget->findChild<AudioSlider *>(appName);

    if (slider == nullptr || appIconName == nullptr)
        return;

    slider->blockSignals(true);
    label->setText(QString::number(paVolumeToValue(value))+"%");
    slider->setValue(paVolumeToValue(value));
    slider->blockSignals(false);

    qDebug() << "sinkInputVolumeChangedSlot" << appName << appIconName << value << appSliderStr;
}

void UkmediaMainWidget::switchStartupPlayMusic()
{
    if (m_pStartUpSetting->keys().contains("startupMusic")) {
       bool state = m_pStartUpSetting->get(LINGMO_STARTUP_MUSIC_KEY).toBool();
       m_pVolumeControl->insertJson(JsonType::JSON_TYPE_USERINFO, AUDIO_STRATUP_SOUNDEFFECT_KEY, state);
       qDebug() << "the switch of startup sound effect is changed to " << state;
    }
}

void UkmediaMainWidget::soundSettingChanged(const QString &key)
{
    qDebug() << "soundSettingChanged" << key;
    if (key == "dnsNoiseReduction")
        switchModuleEchoCancel();
    else if(key == "loopback")
        switchModuleLoopBack();
    else if (key == "volumeIncrease")
        resetVolumeSliderRange();
    else if (key == "monoAudio")
        switchMonoAudio();
    else if (key == "startupMusic")
        switchStartupPlayMusic();
}

/*
 * 获取应用名称，从desktop全路径名下解析出应用名称
 */
QString UkmediaMainWidget::getAppName(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);

    char* name=g_key_file_get_locale_string(keyfile,"Desktop Entry","Name", nullptr, nullptr);
    QString namestr=QString::fromLocal8Bit(name);

    g_key_file_free(keyfile);
    return namestr;
}

/*
 * 获取应用图标，从desktop全路径名下解析出应用图标
 */
QString UkmediaMainWidget::getAppIcon(QString desktopfp)
{
    GError** error=nullptr;
    GKeyFileFlags flags=G_KEY_FILE_NONE;
    GKeyFile* keyfile=g_key_file_new ();

    QByteArray fpbyte=desktopfp.toLocal8Bit();
    char* filepath=fpbyte.data();
    g_key_file_load_from_file(keyfile,filepath,flags,error);

    char* name=g_key_file_get_locale_string(keyfile,"Desktop Entry","Icon", nullptr, nullptr);
    QString namestr=QString::fromLocal8Bit(name);

    g_key_file_free(keyfile);

    /* If we can't find the app icon
     * we need to set a default icon for it */
    namestr = (namestr == "") ? "application-x-desktop" : namestr;

    return namestr;
}

void UkmediaMainWidget::initSubApplicationWidget(QString pAppIcon)
{
    //widget显示应用音量
    QWidget *subAppwidget = new QWidget(appWidget->displayAppVolumeWidget);
    appWidget->appVolumeLabel = new QLabel(subAppwidget);
    appWidget->appVolumeLabel->setParent(subAppwidget);
    appWidget->appIconBtn = new QPushButton(subAppwidget);
    appWidget->appMuteBtn = new QPushButton(subAppwidget);
    appWidget->appMuteBtn->setVisible(false);
    appWidget->appSlider = new AudioSlider(subAppwidget);
    appWidget->appSlider->setOrientation(Qt::Horizontal);
    appWidget->appSlider->setProperty("needTranslucent", true); // Increase translucent effect
    appWidget->appIconBtn->setFixedSize(32,32);
    appWidget->appIconBtn->setFlat(true);
    appWidget->appIconBtn->setFocusPolicy(Qt::NoFocus);
    QSize icon_size(32,32);
    appWidget->appIconBtn->setIconSize(icon_size);
    appWidget->appIconBtn->setIcon(QIcon::fromTheme(pAppIcon));
    appWidget->appIconBtn->setAttribute(Qt::WA_TranslucentBackground);

    QPalette palete = appWidget->appIconBtn->palette();
    palete.setColor(QPalette::Highlight,Qt::transparent);
    palete.setBrush(QPalette::Button,QBrush(QColor(1,1,1,0)));
    appWidget->appIconBtn->setPalette(palete);

    QPalette paleteAppIcon =  appWidget->appIconBtn->palette();
    paleteAppIcon.setColor(QPalette::Highlight,Qt::transparent);
    paleteAppIcon.setBrush(QPalette::Button,QBrush(QColor(0,0,0,0)));
    appWidget->appIconBtn->setPalette(paleteAppIcon);

    appWidget->appSlider->setMaximum(100);
    appWidget->appSlider->setFixedSize(286,48);
    appWidget->appIconBtn->adjustSize();
    appWidget->appIconBtn->setProperty("useIconHighlightEffect",true);
    appWidget->appIconBtn->setProperty("iconHighlightEffectMode",true);
    appWidget->appIconBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                                                "padding-left:0px;}"
                                                "QPushButton:hover {"
                                                "background-color: #00000000;"
                                                "color: white;}");

    //音量滑动条
    QHBoxLayout *appSliderLayout = new QHBoxLayout(subAppwidget);

    subAppwidget->setFixedSize(412,48);
    appWidget->appVolumeLabel->setText("0%");
    appWidget->appVolumeLabel->setFixedSize(52,48);

    if (!UkmediaCommon::getInstance().isHWKLanguageEnv(m_languageEnvStr)) {
        appWidget->appVolumeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    appSliderLayout->addWidget(appWidget->appIconBtn);
    appSliderLayout->addSpacing(3);
    appSliderLayout->addWidget(appWidget->appSlider);
    appSliderLayout->addSpacing(0);
    appSliderLayout->addWidget(appWidget->appVolumeLabel);
    appSliderLayout->setSpacing(0);
    subAppwidget->setLayout(appSliderLayout);
    appSliderLayout->setContentsMargins(10,0,30,0);

    appWidget->m_pVlayout->addWidget(subAppwidget);
//    //设置布局的垂直间距以及设置vlayout四周的间距
    appWidget->displayAppVolumeWidget->resize(412,m_pVolumeControl->sinkInputList.count()*50);
    appWidget->m_pVlayout->setSpacing(2);
    appWidget->displayAppVolumeWidget->setLayout(appWidget->m_pVlayout);
    appWidget->m_pVlayout->update();
}

/*
 * 当播放或录制应用退出时删除在应用音量界面上该应用的显示
 */
void UkmediaMainWidget::removeSubAppWidget(QString m_pAppName)
{
    g_debug ("Removing application stream %s", m_pAppName);
    /* We could call bar_set_stream_control here, but that would pointlessly
         * invalidate the channel bar, so just remove it ourselves */
    int index = -1;
    for (int i=0;i<m_pVolumeControl->sinkInputList.count();i++) {
        if (m_pVolumeControl->sinkInputList.at(i) == m_pAppName) {
            index = i;
            m_pVolumeControl->sinkInputList.removeAt(i);
            break;
        }
    }
    if (index == -1)
        return;
    QLayoutItem *item;
    if ((item = appWidget->m_pVlayout->takeAt(index)) != 0) {
        QWidget *wid = item->widget();
        appWidget->m_pVlayout->removeWidget(wid);
        wid->setParent(nullptr);
        delete wid;
        delete item;
    }
    qDebug() << "removeSubAppWidget" << m_pAppName << m_pVolumeControl->sinkInputList.count();
    //设置布局的间距以及设置vlayout四周的间距
    appWidget->displayAppVolumeWidget->resize(404,m_pVolumeControl->sinkInputList.count()*50);
    appWidget->m_pVlayout->setSpacing(2);
    appWidget->m_pVlayout->setContentsMargins(0,0,0,0);
    appWidget->m_pVlayout->update();

}

/*
   In PulseAudio without ucm, ports will show up with the following names:
   Headphones - analog-output-headphones
   Headset mic - analog-input-headset-mic (was: analog-input-microphone-headset)
   Jack in mic-in mode - analog-input-headphone-mic (was: analog-input-microphone)

   However, since regular mics also show up as analog-input-microphone,
   we need to check for certain controls on alsa mixer level too, to know
   if we deal with a separate mic jack, or a multi-function jack with a
   mic-in mode (also called "headphone mic").
   We check for the following names:

   Headphone Mic Jack - indicates headphone and mic-in mode share the same jack,
     i e, not two separate jacks. Hardware cannot distinguish between a
     headphone and a mic.
   Headset Mic Phantom Jack - indicates headset jack where hardware can not
     distinguish between headphones and headsets
   Headset Mic Jack - indicates headset jack where hardware can distinguish
     between headphones and headsets. There is no use popping up a dialog in
     this case, unless we already need to do this for the mic-in mode.

   From the PA_PROCOTOL_VERSION=34, The device_port structure adds 2 members
   availability_group and type, with the help of these 2 members, we could
   consolidate the port checking and port setting for non-ucm and with-ucm
   cases.
*/

#define HEADSET_PORT_SET(dst, src) \
        do { \
                if (!(dst) || (dst)->priority < (src)->priority) \
                        dst = src; \
        } while (0)

#define GET_PORT_NAME(x) (x ? g_strdup (x->name) : NULL)

headsetPorts *
UkmediaMainWidget::getHeadsetPorts (const pa_card_info *c)
{
    headsetPorts *h;
    guint i;

    h = g_new0 (headsetPorts, 1);

    for (i = 0; i < c->n_ports; i++) {
        pa_card_port_info *p = c->ports[i];
        //                if (control->priv->server_protocol_version < 34) {
        if (g_str_equal (p->name, "analog-output-headphones") || g_str_equal (p->name, "[Out] Headphones1"))
            h->headphones = p;
        else if (g_str_equal (p->name, "analog-input-headset-mic") || g_str_equal (p->name, "[In] Headset"))
            h->headsetmic = p;
        else if (g_str_equal (p->name, "analog-input-headphone-mic") || g_str_equal (p->name, "[In] Headphones2"))
            h->headphonemic = p;
        else if (g_str_equal (p->name, "analog-input-internal-mic") || g_str_equal (p->name, "[In] Mic"))
            h->internalmic = p;
        else if (g_str_equal (p->name, "analog-output-speaker") || g_str_equal (p->name, "[Out] Speaker"))
            h->internalspk = p;
        //                } else {
#if (PA_PROTOCOL_VERSION >= 34)
        /* in the first loop, set only headphones */
        /* the microphone ports are assigned in the second loop */
        if (p->type == PA_DEVICE_PORT_TYPE_HEADPHONES) {
            if (p->availability_group)
                HEADSET_PORT_SET (h->headphones, p);
        } else if (p->type == PA_DEVICE_PORT_TYPE_SPEAKER) {
            HEADSET_PORT_SET (h->internalspk, p);
        } else if (p->type == PA_DEVICE_PORT_TYPE_MIC) {
            if (!p->availability_group)
                HEADSET_PORT_SET (h->internalmic, p);
        }
#else
        //                        g_warning_once ("libgnome-volume-control running against PulseAudio %u, "
        //                                        "but compiled against older %d, report a bug to your distribution",
        //                                        control->priv->server_protocol_version,
        //                                        PA_PROTOCOL_VERSION);
#endif
        //                }
    }

    return h;
}

gboolean
UkmediaMainWidget::verifyAlsaCard (int cardindex,
                                      gboolean *headsetmic,
                                      gboolean *headphonemic)
{
    char *ctlstr;
    snd_hctl_t *hctl;
    snd_ctl_elem_id_t *id;
    int err;

    *headsetmic = FALSE;
    *headphonemic = FALSE;

    ctlstr = g_strdup_printf ("hw:%i", cardindex);
    if ((err = snd_hctl_open (&hctl, ctlstr, 0)) < 0) {
        g_warning ("snd_hctl_open failed: %s", snd_strerror(err));
        g_free (ctlstr);
        return FALSE;
    }
    g_free (ctlstr);

    if ((err = snd_hctl_load (hctl)) < 0) {
//        if (hasNo && !hasOther)
        g_warning ("snd_hctl_load failed: %s", snd_strerror(err));
        snd_hctl_close (hctl);
        return FALSE;
    }

    snd_ctl_elem_id_alloca (&id);

    snd_ctl_elem_id_clear (id);
    snd_ctl_elem_id_set_interface (id, SND_CTL_ELEM_IFACE_CARD);
    snd_ctl_elem_id_set_name (id, "Headphone Mic Jack");
    if (snd_hctl_find_elem (hctl, id))
        *headphonemic = TRUE;

    snd_ctl_elem_id_clear (id);
    snd_ctl_elem_id_set_interface (id, SND_CTL_ELEM_IFACE_CARD);
    snd_ctl_elem_id_set_name (id, "Headset Mic Phantom Jack");
    if (snd_hctl_find_elem (hctl, id))
        *headsetmic = TRUE;

    if (*headphonemic) {
        snd_ctl_elem_id_clear (id);
        snd_ctl_elem_id_set_interface (id, SND_CTL_ELEM_IFACE_CARD);
        snd_ctl_elem_id_set_name (id, "Headset Mic Jack");
        if (snd_hctl_find_elem (hctl, id))
            *headsetmic = TRUE;
    }

    snd_hctl_close (hctl);
    return *headsetmic || *headphonemic;
}

void UkmediaMainWidget::checkAudioDeviceSelectionNeeded(const pa_card_info *info)
{
    headsetPorts *h;
    gboolean start_dialog, stop_dialog;
    qDebug() << "check_audio_device_selection_needed" <<info->name;

    start_dialog = FALSE;
    stop_dialog = FALSE;
    h = getHeadsetPorts(info);

    if (!h->headphones || (!h->headsetmic && !h->headphonemic)) {
        qDebug() << "no headset jack" ;
        /* Not a headset jack */
        goto out;
    } else {
        qDebug() << "headset jack" << h->headphonemic << h->headphones;
    }

    if (headsetCard != (int) info->index) {
        int cardindex;
        gboolean hsmic = TRUE;
        gboolean hpmic = TRUE;
        const char *s;

        s = pa_proplist_gets (info->proplist, "alsa.card");
        if (!s) {
            goto out;
        }

        cardindex = strtol(s, NULL, 10);
        if (cardindex == 0 && strcmp(s, "0") != 0) {
            goto out;
        }
        if (!verifyAlsaCard(cardindex, &hsmic, &hpmic)) {
            goto out;
        }

        headsetCard = info->index;
        hasHeadsetmic = hsmic && h->headsetmic;
        hasHeadphonemic = hpmic && h->headphonemic;
    } else {
        start_dialog = (h->headphones->available != PA_PORT_AVAILABLE_NO) && !headsetPluggedIn;
        stop_dialog = (h->headphones->available == PA_PORT_AVAILABLE_NO) && headsetPluggedIn;
    }

    headsetPluggedIn = h->headphones->available != PA_PORT_AVAILABLE_NO;
    m_pHeadphonesName = GET_PORT_NAME(h->headphones);
    m_pHeadsetmicName = GET_PORT_NAME(h->headsetmic);
    m_pHeadphonemicName = GET_PORT_NAME(h->headphonemic);
    m_pInternalspkName = GET_PORT_NAME(h->internalspk);
    m_pInternalmicName = GET_PORT_NAME(h->internalmic);

    if (firstLoad) {
        firstLoad = false;
    } else {
        if (headsetPluggedIn) {
            headsetWidget->showWindow();
        } else {
            if (headsetWidget->isShow) {
                headsetWidget->hide();
                headsetWidget->isShow = false;
            }
        }
    }
    qDebug() << "check_audio_device_selection_needed" <<m_pHeadphonesName <<m_pHeadsetmicName << m_pHeadphonemicName <<m_pInternalspkName << m_pInternalmicName << headsetPluggedIn;
    if (!start_dialog && !stop_dialog) {
        goto out;
    }

out:
    g_free (h);
}

void UkmediaMainWidget::freePrivPortNames ()
{
    g_clear_pointer (&m_pHeadphonesName, g_free);
    g_clear_pointer (&m_pHeadsetmicName, g_free);
    g_clear_pointer (&m_pHeadphonemicName, g_free);
    g_clear_pointer (&m_pInternalspkName, g_free);
    g_clear_pointer (&m_pInternalmicName, g_free);
}


void UkmediaMainWidget::onPrepareForSleep(bool sleep)
{
    ///系统休眠时，sink未进入suspend状态
    ///在系统休眠之前将默认的sink设置成suspend状态，在系统唤醒后在将默认的sink设置成running状态
    ///安全问题，pacmd没有权限无法执行，改用pactl

    QString cmd = "pactl suspend-sink ";
    cmd.append(m_pVolumeControl->defaultSinkName);
    qDebug() << "ctf ----- onPrepareForSleep"  << sleep << m_pVolumeControl->defaultSinkName;
    if(sleep)
    {
        cmd.append(" 1");
        system(cmd.toLatin1().data());
//        pa_context_suspend_sink_by_name(m_pVolumeControl->getContext(),m_pVolumeControl->defaultSinkName,1,nullptr,nullptr);
        uninhibit();
    }
    else
    {
        cmd.append(" 0");
        system(cmd.toLatin1().data());
//        pa_context_suspend_sink_by_name(m_pVolumeControl->getContext(),m_pVolumeControl->defaultSinkName,0,nullptr,nullptr);
        inhibit();
    }
}

/*
    输出设备提示弹窗
*/
void UkmediaMainWidget::deviceChangedShowSlot(QString dev_name)
{
    if (SystemVersion::SYSTEM_VERSION_OKNILE == m_version) {
        return;
    }
    osdWidget->setIcon(dev_name);
    osdWidget->dialogShow();
}

/*
 * 更新设备端口
 */
void UkmediaMainWidget::updateDevicePort()
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> temp;
    currentInputPortLabelMap.clear();
    currentOutputPortLabelMap.clear();
    if (firstEntry) {
        for (it = m_pVolumeControl->outputPortMap.begin();it!=m_pVolumeControl->outputPortMap.end();) {
            temp = it.value();
            for (at=temp.begin();at!=temp.end();) {
                qDebug() << "updateDevicePort" << firstEntry << it.key() << at.value() <<at.key();
                QString cardName = findCardName(it.key(),m_pVolumeControl->cardMap);
                addOutputListWidgetItem(at.key(),at.value(),cardName);
                ++at;
            }
            ++it;
        }
        for (it = m_pVolumeControl->inputPortMap.begin();it!=m_pVolumeControl->inputPortMap.end();) {
            temp = it.value();
            for (at=temp.begin();at!=temp.end();) {
                qDebug() << "updateDevicePort" << firstEntry << it.key() << at.value();
                QString cardName = findCardName(it.key(),m_pVolumeControl->cardMap);
                addInputListWidgetItem(at.value(),cardName);
                ++at;
            }
            ++it;
        }
    } else {
        //记录上一次output label
        for (int i=0;i<systemWidget->m_pOutputListWidget->count();i++) {
            QMap<int,QString>::iterator at;
            QListWidgetItem *item = systemWidget->m_pOutputListWidget->item(i);
            LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pOutputListWidget->itemWidget(item);
            int index;
            for (at=m_pVolumeControl->cardMap.begin();at!=m_pVolumeControl->cardMap.end();) {
                if (wid->deviceLabel->text() == at.value()) {
                    index = at.key();
                    break;
                }
                ++at;
            }
            currentOutputPortLabelMap.insertMulti(index,wid->portLabel->fullText);
        }

        for (int i=0;i<systemWidget->m_pInputListWidget->count();i++) {
            QListWidgetItem *item = systemWidget->m_pInputListWidget->item(i);
            LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pInputListWidget->itemWidget(item);
            int index;
            int count;
            QMap<int,QString>::iterator at;
            for (at=m_pVolumeControl->cardMap.begin();at!=m_pVolumeControl->cardMap.end();) {
                if (wid->deviceLabel->text() == at.value()) {
                    index = at.key();
                    break;
                }
                ++at;
                ++count;
            }
            currentInputPortLabelMap.insertMulti(index,wid->portLabel->fullText);
        }
        systemWidget->m_pInputListWidget->blockSignals(true);
        deleteNotAvailableOutputPort();
        addAvailableOutputPort();
        initOutputListWidgetItem();
        deleteNotAvailableInputPort();
        addAvailableInputPort();
        systemWidget->m_pInputListWidget->blockSignals(false);
        Q_EMIT updateDevSignal();
    }
    if (systemWidget->m_pOutputListWidget->count() > 0 /*|| m_pInputWidget->m_pInputListWidget->count()*/) {
        //需求31268待2501再合入
#if 1
        firstEntry = false;
        systemWidget->m_pSystemVolumeSlider->setEnabled(true);
        appWidget->systemVolumeSlider->setEnabled(true);
        systemWidget->m_pSystemVolumeBtn->setEnabled(true);
        appWidget->systemVolumeBtn->setEnabled(true);
    } else {
        systemWidget->m_pSystemVolumeSlider->setEnabled(false);
        appWidget->systemVolumeSlider->setEnabled(false);
        systemWidget->m_pSystemVolumeBtn->setEnabled(false);
        appWidget->systemVolumeBtn->setEnabled(false);
    }
#elif 0
        firstEntry = false;
    }
#endif
}

void UkmediaMainWidget::updateListWidgetItemSlot()
{
    qDebug() << "updateListWidgetItemSlot---------";
    initOutputListWidgetItem();
    initInputListWidgetItem();
    themeChangeIcons();
    switchModuleEchoCancel();
//    switchModuleLoopBack();
}

/*
 * output list widget选项改变，设置对应的输出设备
 */
void UkmediaMainWidget::outputListWidgetCurrentRowChangedSlot(int row)
{
    //当所有可用的输出设备全部移除，台式机才会出现该情况
    if (row == -1)
        return;

    QListWidgetItem *item = systemWidget->m_pOutputListWidget->item(row);
    if (item == nullptr) {
        qDebug() <<"output current item is null";
    }
    LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pOutputListWidget->itemWidget(item);
    QListWidgetItem *inputCurrrentItem = systemWidget->m_pInputListWidget->currentItem();
    LingmoUIListWidgetItem *inputWid = (LingmoUIListWidgetItem *)systemWidget->m_pInputListWidget->itemWidget(inputCurrrentItem);
    bool isContainBlue = inputDeviceContainBluetooth();

    setDeviceButtonState(row);

    //当输出设备从蓝牙切换到其他设备时，需将蓝牙声卡的配置文件切换为a2dp-sink
    if (isContainBlue && (strstr(m_pVolumeControl->defaultSourceName,"headset_head_unit") || strstr(m_pVolumeControl->defaultSourceName,"bt_sco_sink"))) {
        QString cardName = blueCardName();
        setCardProfile(cardName,"a2dp_sink");
    }
    QStringList comboboxPortNameList = wid->portLabel->text().split("（");
    QStringList inputComboboxPortNameList;
    if (inputWid) {
        inputComboboxPortNameList = inputWid->portLabel->text().split("（");
    }

    QMap<int,QMap<QString,QString>>::iterator outputProfileMap;
    QMap<int,QMap<QString,QString>>::iterator inputProfileMap;
    QMap<QString,QString> tempMap;
    QMap<QString,QString>::iterator at;
    QString endOutputProfile = "";
    QString endInputProfile = "";

    int currentCardIndex = findCardIndex(wid->deviceLabel->text(),m_pVolumeControl->cardMap);
    int count,i;

    for (outputProfileMap=m_pVolumeControl->profileNameMap.begin();outputProfileMap!= m_pVolumeControl->profileNameMap.end();) {
        if (currentCardIndex == outputProfileMap.key()) {
            tempMap = outputProfileMap.value();
            for (at=tempMap.begin(),i=0;at!= tempMap.end();++i) {
                if (at.key() == comboboxPortNameList.at(0)) {
                    count = i;
                    endOutputProfile = at.value();
                }
                ++at;
            }
        }
        ++outputProfileMap;
    }

    if (inputCurrrentItem != nullptr) {
        QMap <QString,QString>::iterator it;
        QMap <QString,QString> temp;
        int index = findCardIndex(inputWid->deviceLabel->text(),m_pVolumeControl->cardMap);
        for (inputProfileMap=m_pVolumeControl->inputPortProfileNameMap.begin(),count=0;inputProfileMap!= m_pVolumeControl->inputPortProfileNameMap.end();count++) {
            if (inputProfileMap.key() == index) {
                temp = inputProfileMap.value();
                for (it=temp.begin();it!=temp.end();) {
                    if (it.key() == inputComboboxPortNameList.at(0)) {
                        endInputProfile = it.value();
                    }
                    ++it;
                }
            }
            ++inputProfileMap;
        }
    }
    qDebug() << "outputListWidgetCurrentRowChangedSlot" << row << wid->deviceLabel->text() << endOutputProfile <<endInputProfile;
    //如果选择的输入输出设备为同一个声卡，则追加指定输入输出端口属于的配置文件
    if ((inputCurrrentItem != nullptr && wid->deviceLabel->text() == inputWid->deviceLabel->text()) || \
            wid->deviceLabel->text() == "alsa_card.platform-sound_DA_combine_v5" && inputWid->deviceLabel->text() == "3a.algo") {

        QString setProfile = endOutputProfile;
        if (!endOutputProfile.contains("input:analog-stereo") || !endOutputProfile.contains("HiFi")) {
            setProfile += "+";
            setProfile +=endInputProfile;
        }

        if (endOutputProfile.contains("a2dp-sink") && endInputProfile.contains("headset-head-unit")) {
            setProfile = endOutputProfile;
        }

        setCardProfile(wid->deviceLabel->text(),setProfile);
        setDefaultOutputPortDevice(wid->deviceLabel->text(),wid->portLabel->fullText);
    }
    //如果选择的输入输出设备不是同一块声卡，需要设置一个优先级高的配置文件
    else {
        int index = findCardIndex(wid->deviceLabel->text(),m_pVolumeControl->cardMap);
        QMap <int,QList<QString>>::iterator it;
        QString profileName;
        for (it=m_pVolumeControl->cardProfileMap.begin();it!=m_pVolumeControl->cardProfileMap.end();) {
            if (it.key() == index) {
                if (strstr(endOutputProfile.toLatin1().data(),"headset_head_unit")) {
                    endOutputProfile = "a2dp_sink";
                }
                profileName = findHighPriorityProfile(index,endOutputProfile);
            }
            ++it;
        }

        QString setProfile = profileName;
        setCardProfile(wid->deviceLabel->text(),setProfile);
        setDefaultOutputPortDevice(wid->deviceLabel->text(),wid->portLabel->fullText);
    }
    qDebug() << "active output port:" << wid->portLabel->fullText << wid->deviceLabel->text();
}

/*
 * input list widget选项改变，设置对应的输入设备
 */
void UkmediaMainWidget::inputListWidgetCurrentRowChangedSlot(int row)
{
    if (row == -1)
    {
        return;
    }

    QListWidgetItem *currentOutputItem = systemWidget->m_pOutputListWidget->currentItem();
    LingmoUIListWidgetItem *currentOutputItemWidget = (LingmoUIListWidgetItem *)systemWidget->m_pOutputListWidget->itemWidget(currentOutputItem);
    QString currentOutputCardName = currentOutputItemWidget->deviceLabel->text();
    QString currentOutputPortLabel = currentOutputItemWidget->portLabel->text().split("（").at(0);

    QListWidgetItem *destInputItem = systemWidget->m_pInputListWidget->item(row);
    LingmoUIListWidgetItem *destInputItemWidget = (LingmoUIListWidgetItem *)systemWidget->m_pInputListWidget->itemWidget(destInputItem);
    QString destInputCardName = destInputItemWidget->deviceLabel->text();
    QString destInputPortLabel = destInputItemWidget->portLabel->text().split("（").at(0);

    qDebug() << __func__ << row << "destPortLabel" << destInputPortLabel
             << "destCardName" << destInputCardName
             << "defaultSourceName" << m_pVolumeControl->defaultSourceName;

    if (inputDeviceContainBluetooth())
    {
        if (strstr(m_pVolumeControl->defaultSinkName, HEADSET_HEAD_UNIT) || strstr(m_pVolumeControl->defaultSourceName, HUAWEI_BTO_SOURCE))
        {
            qDebug() << "To switch Input to a non-Bluetooth device, need to change Bluetooth card profile to 'a2dp'";
            setCardProfile(blueCardName(), "a2dp_sink");
        }
    }

    isCheckBluetoothInput = destInputCardName.contains("bluez_card") ? true : false;

    QString endInputProfile = "";
    QString endOutputProfile = "";
    QMap <QString,QString> tempMap;
    QMap<QString,QString>::iterator at;
    QMap<int, QMap<QString,QString>>::iterator inputProfileMap;

    int destInputCardIndex = findCardIndex(destInputCardName, m_pVolumeControl->cardMap);
    for (inputProfileMap = m_pVolumeControl->inputPortProfileNameMap.begin(); inputProfileMap != m_pVolumeControl->inputPortProfileNameMap.end();)
    {
        if (inputProfileMap.key() == destInputCardIndex)
        {
            tempMap = inputProfileMap.value();
            for (at = tempMap.begin();at != tempMap.end();)
            {
                if (at.key() == destInputPortLabel)
                {
                    endInputProfile = at.value();
                }
                ++at;
            }
        }
        ++inputProfileMap;
    }

    if (nullptr != currentOutputItem)
    {
        QMap <QString,QString> tempMap;
        QMap <QString,QString>::iterator it;
        QMap<int, QMap<QString,QString>>::iterator outputProfileMap;

        int OutputCardIndex = findCardIndex(currentOutputItemWidget->deviceLabel->text(), m_pVolumeControl->cardMap);

        for (outputProfileMap = m_pVolumeControl->profileNameMap.begin(); outputProfileMap != m_pVolumeControl->profileNameMap.end();)
        {
            if (outputProfileMap.key() == OutputCardIndex)
            {
                tempMap = outputProfileMap.value();
                for(it = tempMap.begin(); it != tempMap.end();){
                    if(it.key() == currentOutputPortLabel)
                    {
                        endOutputProfile = it.value();
                    }
                    ++it;
                }
            }
            ++outputProfileMap;
        }
    }

    //如果选择的输入输出设备为同一个声卡，则追加指定输入输出端口属于的配置文件
    QString setProfile = "";
    if (currentOutputItem != nullptr && destInputCardName == currentOutputCardName)
    {
        //有些声卡的配置文件默认只有输入/输出设备或者配置文件包含了输出输入设备，因此只需要取其中一个配置文件即可
        if (endOutputProfile == "a2dp-sink" || endInputProfile == "headset_head_unit" || endOutputProfile == "HiFi")
        {
            setProfile += endInputProfile;
        }
        else
        {
            setProfile += endOutputProfile;
            setProfile += "+";
            setProfile += endInputProfile;
        }
    }
    //如果选择的输入输出设备不是同一块声卡，需要设置一个优先级高的配置文件
    else
    {
        QMap <int,QList<QString>>::iterator it;
        for(it = m_pVolumeControl->cardProfileMap.begin(); it != m_pVolumeControl->cardProfileMap.end();)
        {
            if (it.key() == destInputCardIndex)
            {
                QStringList list= it.value();
                setProfile = findHighPriorityProfile(destInputCardIndex, endInputProfile);
                if (list.contains(endOutputProfile))
                {

                }
            }
            ++it;
        }
    }

    setCardProfile(destInputCardName, setProfile);
    setDefaultInputPortDevice(destInputCardIndex, destInputItemWidget->portLabel->fullText);
}


void UkmediaMainWidget::updateAppVolume(QString str, int value, bool state)
{
    qDebug() << __func__ << str << value <<state;

    bool found = true;

    QSlider *slider = appWidget->findChild<QSlider*>(str);
    if (slider == nullptr) {
        found = false;
    }
    if (found) {
        lingmoVideoVolumeSignal = true;
        if(slider->value() == value) {
            lingmoVideoVolumeSignal = false;
        }
        slider->setValue(value);
    } else {
        value = -1;
    }
    qDebug() << "found app slider, set value ----" << value;

    QPushButton *btn = appWidget->findChild<QPushButton*>(str);
    if (btn == nullptr) {
        found = false;
    } else {
        qDebug() << "founded  app mute button ------------" << str << state;
        isAppMuteBtnPress = true;
        sinkInputMute = state;
        lingmoVideoMuteSignal = true;
        btn->setChecked(true);
        btn->click();
    }

    QDBusMessage message =QDBusMessage::createSignal("/", "org.lingmo.media", "sinkVolumeChanged");
    message<<str<<value<<state;
    QDBusConnection::sessionBus().send(message);
    qDebug() <<"sinkVolumeChanged11111111111" <<str <<value <<state;
}

void UkmediaMainWidget::setDeviceButtonState(int row)
{
    for (int i=0;i<systemWidget->m_pOutputListWidget->count();i++) {
        QListWidgetItem *item = systemWidget->m_pOutputListWidget->item(i);
        LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pOutputListWidget->itemWidget(item);
        if (i == row) {
            if (!wid->deviceButton->isChecked()) {
                wid->deviceButton->setChecked(true);
            }
            continue;
        }
        wid->deviceButton->setChecked(false);
    }
}

/*
 * 添加output port到output list widget
 */
void UkmediaMainWidget::addOutputListWidgetItem(QString portName, QString portLabel, QString cardName)
{
    LingmoUIListWidgetItem *itemW = new LingmoUIListWidgetItem(this);
    int i = systemWidget->m_pOutputListWidget->count();
    QListWidgetItem * item = new QListWidgetItem(systemWidget->m_pOutputListWidget);
    item->setSizeHint(QSize(200,48));
    itemW->setButtonIcon(portName);
    itemW->setLabelText(portLabel,cardName);
    systemWidget->m_pOutputListWidget->blockSignals(true);
    systemWidget->m_pOutputListWidget->setItemWidget(item, itemW);
    systemWidget->m_pOutputListWidget->insertItem(i,item);
    systemWidget->m_pOutputListWidget->blockSignals(false);

    connect(itemW->deviceButton, &QPushButton::clicked, this, [=](){
        int idx = indexOfOutputPortInOutputListWidget(itemW->portLabel->fullText);
        systemWidget->m_pOutputListWidget->setCurrentRow(idx);
        outputListWidgetCurrentRowChangedSlot(idx);
        outputListWidgetRow = idx;
    });
}

/*
 * 添加input port到input list widget
 */
void UkmediaMainWidget::addInputListWidgetItem(QString portName, QString cardName)
{
    LingmoUIListWidgetItem *itemW = new LingmoUIListWidgetItem(this);
    int i = systemWidget->m_pInputListWidget->count();
    QListWidgetItem * item = new QListWidgetItem(systemWidget->m_pInputListWidget);
    item->setSizeHint(QSize(200,48)); //QSize(120, 40) spacing: 12px;
    systemWidget->m_pInputListWidget->blockSignals(true);
    systemWidget->m_pInputListWidget->setItemWidget(item, itemW);
    systemWidget->m_pInputListWidget->blockSignals(false);
    itemW->setLabelText(portName,cardName);
    itemW->setButtonIcon(portName);
    systemWidget->m_pInputListWidget->blockSignals(true);
    systemWidget->m_pInputListWidget->insertItem(i,item);
    systemWidget->m_pInputListWidget->blockSignals(false);
}

int UkmediaMainWidget::indexOfOutputPortInOutputListWidget(QString portName)
{
    for (int row=0;row<systemWidget->m_pOutputListWidget->count();row++) {
        QListWidgetItem *item = systemWidget->m_pOutputListWidget->item(row);
        LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pOutputListWidget->itemWidget(item);
        if (wid->portLabel->fullText == portName) {
            return row;
        }
    }
    return -1;
}

int UkmediaMainWidget::indexOfInputPortInInputListWidget(QString portName)
{
    for (int row=0;row<systemWidget->m_pInputListWidget->count();row++) {

        QListWidgetItem *item = systemWidget->m_pInputListWidget->item(row);
        LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pInputListWidget->itemWidget(item);
        if (wid->portLabel->fullText == portName) {
            return row;
        }
    }
    return -1;
}

/*
 * 当前的输出端口是否应该在output list widget上删除
 */
bool UkmediaMainWidget::outputPortIsNeedDelete(int index, QString name)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
//    qDebug() << "outputPortIsNeedDelete" << index << name;
    for(it = m_pVolumeControl->outputPortMap.begin();it!=m_pVolumeControl->outputPortMap.end();)
    {
        if (it.key() == index) {
            portMap = it.value();
            for (at=portMap.begin();at!=portMap.end();) {
                if (name == at.value()) {
                    return false;
                }
                ++at;
            }
        }
        ++it;
    }
    return true;
}

/*
 * 当前的输出端口是否应该添加到output list widget上
 */
bool UkmediaMainWidget::outputPortIsNeedAdd(int index, QString name)
{
    QMap<int,QString>::iterator it;
    for(it=currentOutputPortLabelMap.begin();it!=currentOutputPortLabelMap.end();) {
        if ( index == it.key() && name == it.value()) {
            return false;
        }
        ++it;
    }
    return true;
}

/*
 * 当前的输出端口是否应该在input list widget上删除
 */
bool UkmediaMainWidget::inputPortIsNeedDelete(int index, QString name)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for(it = m_pVolumeControl->inputPortMap.begin();it!=m_pVolumeControl->inputPortMap.end();)
    {
        if (it.key() == index) {
            portMap = it.value();
            for (at=portMap.begin();at!=portMap.end();) {
                if (name == at.value()) {
                    return false;
                }
                ++at;
            }
        }
        ++it;
    }
    return true;
}

/*
 * 当前的输出端口是否应该添加到input list widget上
 */
bool UkmediaMainWidget::inputPortIsNeedAdd(int index, QString name)
{
    QMap<int,QString>::iterator it;
    for(it=currentInputPortLabelMap.begin();it!=currentInputPortLabelMap.end();) {
        if ( index == it.key() && name == it.value()) {
            return false;
        }
        ++it;
    }
    return true;
}


/*
 * 移除output list widget上不可用的输出端口
 */
void UkmediaMainWidget::deleteNotAvailableOutputPort()
{
    qDebug() << "deleteNotAvailableOutputPort";
    QMap<int,QString>::iterator it;
    for (it=currentOutputPortLabelMap.begin();it!=currentOutputPortLabelMap.end();) {
        if (outputPortIsNeedDelete(it.key(),it.value())) {
            qDebug() << "outputPortIsNeedDelete" << it.key() << it.value();
            int index = indexOfOutputPortInOutputListWidget(it.value());
            if (index == -1) {
                return;
            }
            systemWidget->m_pOutputListWidget->blockSignals(true);
            QListWidgetItem *item = systemWidget->m_pOutputListWidget->takeItem(index);
            systemWidget->m_pOutputListWidget->removeItemWidget(item);
            systemWidget->m_pOutputListWidget->blockSignals(false);
            it = currentOutputPortLabelMap.erase(it);
            continue;
        }
        ++it;
    }
}

/*
 * 添加可用的输出端口到output list widget
 */
void UkmediaMainWidget::addAvailableOutputPort()
{
    QMap<int,QMap<QString,QString>>::iterator at;
    QMap<QString,QString>::iterator it;
    QMap<QString,QString> tempMap;
    //增加端口
    for(at=m_pVolumeControl->outputPortMap.begin();at!=m_pVolumeControl->outputPortMap.end();)
    {
        tempMap = at.value();
        for (it=tempMap.begin();it!=tempMap.end();) {
            if (outputPortIsNeedAdd(at.key(),it.value())) {
                qDebug() << "add output list widget" << at.key()<< it.value() << it.key();
                addOutputListWidgetItem(it.key(),it.value(),findCardName(at.key(),m_pVolumeControl->cardMap));
                currentOutputPortLabelMap.insertMulti(at.key(),it.value());
            }
            ++it;
        }
        ++at;
    }
}

/*
 * 在input list widget删除不可用的端口
 */
void UkmediaMainWidget::deleteNotAvailableInputPort()
{
    //删除不可用的输入端口
    QMap<int,QString>::iterator it;
    for (it=currentInputPortLabelMap.begin();it!=currentInputPortLabelMap.end();) {
        //没找到，需要删除
        if (inputPortIsNeedDelete(it.key(),it.value())) {
            int index = indexOfInputPortInInputListWidget(it.value());
            if (index == -1)
                return;
            systemWidget->m_pInputListWidget->blockSignals(true);
            QListWidgetItem *item = systemWidget->m_pInputListWidget->takeItem(index);
            systemWidget->m_pInputListWidget->removeItemWidget(item);
            systemWidget->m_pInputListWidget->blockSignals(false);
            it = currentInputPortLabelMap.erase(it);
            continue;
        }
        ++it;
    }
//    m_pVolumeControl->removeInputProfile();
}

/*
 *  添加可用的输入端口到input list widget
 */
void UkmediaMainWidget::addAvailableInputPort()
{
    QMap<int,QMap<QString,QString>>::iterator at;
    QMap<QString,QString>::iterator it;
    QMap<QString,QString> tempMap;
    int i = systemWidget->m_pInputListWidget->count();
    //增加端口
    for (at=m_pVolumeControl->inputPortMap.begin();at!=m_pVolumeControl->inputPortMap.end();)
    {
        tempMap = at.value();
        for (it=tempMap.begin();it!=tempMap.end();) {
            //需添加到list widget
            if (inputPortIsNeedAdd(at.key(),it.value())) {
                LingmoUIListWidgetItem *itemW = new LingmoUIListWidgetItem(this);
                QListWidgetItem * item = new QListWidgetItem(systemWidget->m_pInputListWidget);
                item->setSizeHint(QSize(200,48)); //QSize(120, 40) spacing: 12px;
                systemWidget->m_pInputListWidget->blockSignals(true);
                systemWidget->m_pInputListWidget->setItemWidget(item, itemW);
                systemWidget->m_pInputListWidget->blockSignals(false);

                itemW->setLabelText(it.value(),findCardName(at.key(),m_pVolumeControl->cardMap));
                currentInputPortLabelMap.insertMulti(at.key(),it.value());
                systemWidget->m_pInputListWidget->blockSignals(true);
                systemWidget->m_pInputListWidget->insertItem(i,item);
                systemWidget->m_pInputListWidget->blockSignals(false);
            }
            ++it;
        }
        ++at;
    }
}


//查找指定声卡名的索引
int UkmediaMainWidget::findCardIndex(QString cardName, QMap<int,QString> cardMap)
{
    QMap<int, QString>::iterator it;

    for(it=cardMap.begin();it!=cardMap.end();) {
        if (it.value() == cardName) {
            return it.key();
        }
        ++it;
    }
    return -1;
}

/*
 * 根据声卡索引查找声卡名
 */
QString UkmediaMainWidget::findCardName(int index,QMap<int,QString> cardMap)
{
    QMap<int, QString>::iterator it;

    for(it=cardMap.begin();it!=cardMap.end();) {
        if (it.key() == index) {
            return it.value();
        }
        ++it;
    }
    return "";
}

QString UkmediaMainWidget::findHighPriorityProfile(int index, QString profile)
{
    int priority = 0;
    QString profileName = "";
    QMap<QString, int> profileNameMap;
    QMap<QString, int>::iterator tempMap;
    QMap<int, QMap<QString, int>>::iterator it;

    QString profileStr = findCardActiveProfile(index);
    QStringList list = profileStr.split("+");
    QString includeProfile = "";

    if (list.count() > 1)
    {
        if (profile.contains("output"))
            includeProfile = list.at(1);

        else if (profile.contains("input"))
            includeProfile = list.at(0);
    }
    else
        includeProfile = list.at(0);

    for (it = m_pVolumeControl->cardProfilePriorityMap.begin(); it != m_pVolumeControl->cardProfilePriorityMap.end(); ++it)
    {
        if (index == it.key())
        {
            profileNameMap = it.value();
            for (tempMap = profileNameMap.begin(); tempMap != profileNameMap.end(); ++tempMap)
            {
                if (!includeProfile.isEmpty() && tempMap.key().contains(includeProfile) && tempMap.key().contains(profile)
                        && !tempMap.key().contains(includeProfile + "-") && !tempMap.key().contains(profile + "-"))
                {
                    priority = tempMap.value();
                    profileName = tempMap.key();
                    qDebug() << "Status1: Find profileName" << profileName << "priority" << priority;
                }
                else if (tempMap.key().contains(profile) && tempMap.value() > priority)
                {
                    priority = tempMap.value();
                    profileName = tempMap.key();
                    qDebug() << "Status2: Find profileName" << profileName << "priority" << priority;
                }
            }
        }
    }

    qInfo() << __func__ << "Select profile is" << profileName << "index" << index << "includeProfile" << includeProfile;
    return profileName;
}

/*
 * Find the corresponding sink according to the port name
 */
QString UkmediaMainWidget::findPortSink(int index,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    QString sinkStr = "";
    for (it=m_pVolumeControl->sinkPortMap.begin();it!=m_pVolumeControl->sinkPortMap.end();) {
        if(it.key() == index) {
            portNameMap = it.value();
            for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
                if (tempMap.value() == portName) {
                    sinkStr = tempMap.key();
//                    qDebug() <<"find port sink" << tempMap.value() << portName << tempMap.key() <<sinkStr;
                    break;
                }
                ++tempMap;
            }
        }

        ++it;
    }
    if(sinkStr == "")
        qDebug() << "sinkStr not found ---";

    return sinkStr;
}

/*
 * Find the corresponding source according to the port name
 */
QString UkmediaMainWidget::findPortSource(int index, QString portName)
{
    QString sourceName = "";
    QMap<QString, QString> portNameMap;
    QMap<QString, QString>::iterator tempMap;
    QMap<int, QMap<QString, QString>>::iterator it;
    for (it = m_pVolumeControl->sourcePortMap.begin(); it != m_pVolumeControl->sourcePortMap.end(); ++it)
    {
        if(it.key() == index)
        {
            portNameMap = it.value();
            for (tempMap = portNameMap.begin(); tempMap != portNameMap.end(); ++tempMap)
            {
                if (tempMap.value() == portName)
                {
                    sourceName = tempMap.key();
                    break;
                }
            }
        }
    }

    if (sourceName.isEmpty())
        qDebug() << __func__ << "Can not find Source by index" << index << portName;

    return sourceName;
}

/*
    查找名称为PortLbael 的portName
*/
QString UkmediaMainWidget::findOutputPortName(int index,QString portLabel)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portName = "";
    for (it = m_pVolumeControl->outputPortMap.begin();it != m_pVolumeControl->outputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
                if (tempMap.value() == portLabel) {
                    portName = tempMap.key();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return portName;
}

/*
    查找名称为PortName 的portLabel
*/
QString UkmediaMainWidget::findOutputPortLabel(int index,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portLabel = "";
    for (it = m_pVolumeControl->outputPortMap.begin();it != m_pVolumeControl->outputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
//                qDebug() <<"findOutputPortLabel" <<portName <<tempMap.key() <<tempMap.value();
                if (tempMap.key() == portName) {
                    portLabel = tempMap.value();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return portLabel;
}

/*
    查找名称为PortLbael 的portName
*/
QString UkmediaMainWidget::findInputPortName(int index,QString portLabel)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portName = "";
    for (it = m_pVolumeControl->inputPortMap.begin();it != m_pVolumeControl->inputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
                if (tempMap.value() == portLabel) {
                    portName = tempMap.key();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }

    return portName;
}

/*
    查找名称为PortName 的portLabel
*/
QString UkmediaMainWidget::findInputPortLabel(int index,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portLabel = "";
    for (it = m_pVolumeControl->inputPortMap.begin();it != m_pVolumeControl->inputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
                if (tempMap.key() == portName) {
                    portLabel = tempMap.value();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return portLabel;
}

/*
 *  设置声卡的配置文件
 */
void UkmediaMainWidget::setCardProfile(QString name, QString profile)
{
    int index = findCardIndex(name, m_pVolumeControl->cardMap);
    m_pVolumeControl->setCardProfile(index, profile.toLatin1().data());
    qDebug() << "set profile" << name << profile << index ;
}

/*
 * 设置默认的输出设备端口
 */
void UkmediaMainWidget::setDefaultOutputPortDevice(QString devName, QString portName)
{
    int cardIndex = findCardIndex(devName,m_pVolumeControl->cardMap);
    QString portStr = findOutputPortName(cardIndex,portName);
    qDebug() << "setDefaultOutputPortDevice" << cardIndex << portStr;

    QTimer *timer = new QTimer;
    timer->start(300);
    connect(timer,&QTimer::timeout,[=](){
        QString sinkStr = findPortSink(cardIndex,portStr);
        qDebug() << "setDefaultOutputPortDevice"  << sinkStr;
        /*默认的stream 和设置的stream相同 需要更新端口*/
        if (strcmp(sinkStr.toLatin1().data(),m_pVolumeControl->defaultSinkName) == 0) {
            m_pVolumeControl->setSinkPort(sinkStr.toLatin1().data(),portStr.toLatin1().data());
        }
        else {
            m_pVolumeControl->setDefaultSink(sinkStr.toLatin1().data());
            m_pVolumeControl->setSinkPort(sinkStr.toLatin1().data(),portStr.toLatin1().data());
        }

        delete timer;
    });
}

/*
 * 设置默认的输入设备端口
 */
void UkmediaMainWidget::setDefaultInputPortDevice(int cardIndex, QString portLabel)
{
    QString portName = findInputPortName(cardIndex, portLabel);

    QTimer *timer = new QTimer;
    timer->start(100);
    connect(timer, &QTimer::timeout, [=]() {
        QString sourceName = findPortSource(cardIndex, portName);
        /* 默认stream相同，仅更新端口即可 */
        if (UKMedia_Equal(sourceName.toLatin1().data(), m_pVolumeControl->defaultSourceName))
        {
            m_pVolumeControl->setSourcePort(sourceName.toLatin1().data(), portName.toLatin1().data());
        }
        /* 默认stream不同，先设置stream再设置端口 */
        else
        {
            m_pVolumeControl->setDefaultSource(sourceName.toLatin1().data());
            m_pVolumeControl->setSourcePort(sourceName.toLatin1().data(), portName.toLatin1().data());
        }
        qDebug() << __func__ << "card" << cardIndex << "source" << sourceName << "port" << portName;
        delete timer;
    });
}

/*
 * 查找指定索引声卡的active profile
 */
QString UkmediaMainWidget::findCardActiveProfile(int index)
{
    QString activeProfileName = "";
    QMap<int,QString>::iterator it;
    for (it=m_pVolumeControl->cardActiveProfileMap.begin();it!=m_pVolumeControl->cardActiveProfileMap.end();) {
        if (it.key() == index) {
            activeProfileName = it.value();
            break;
        }
        ++it;
    }
    return activeProfileName;
}

void UkmediaMainWidget::findOutputListWidgetItem(QString cardName,QString portLabel)
{
    for (int row=0;row<systemWidget->m_pOutputListWidget->count();row++) {

        QListWidgetItem *item = systemWidget->m_pOutputListWidget->item(row);
        LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pOutputListWidget->itemWidget(item);
        qDebug() << "findOutputListWidgetItem" << "cardName:" << cardName << wid->deviceLabel->text() << "portName:" << portLabel << wid->portLabel->fullText;
        if (wid->deviceLabel->text() == cardName && portLabel == wid->portLabel->fullText) {
            systemWidget->m_pOutputListWidget->blockSignals(true);
            systemWidget->m_pOutputListWidget->setCurrentRow(row);
            setDeviceButtonState(row);
            systemWidget->m_pOutputListWidget->blockSignals(false);
            wid->deviceButton->repaint();
            break;
        } else if (wid->deviceLabel->text() == cardName && (wid->portLabel->fullText.compare("HUAWEI Histen") == 0)) {
            systemWidget->m_pOutputListWidget->blockSignals(true);
            systemWidget->m_pOutputListWidget->setCurrentRow(row);
            setDeviceButtonState(row);
            systemWidget->m_pOutputListWidget->blockSignals(false);
            wid->deviceButton->repaint();
            break;
        }
    }

    outputListWidgetRow = systemWidget->m_pOutputListWidget->currentRow();
}

void UkmediaMainWidget::findInputListWidgetItem(QString cardName,QString portLabel)
{
    for (int row=0;row<systemWidget->m_pInputListWidget->count();row++) {
        QListWidgetItem *item = systemWidget->m_pInputListWidget->item(row);
        LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pInputListWidget->itemWidget(item);
        qDebug() << "findInputListWidgetItem" << "card name:" << cardName << "deviceLabel:" << wid->deviceLabel->text() << "portname" << portLabel << "portLabel:" << wid->portLabel->fullText;
        if (wid->deviceLabel->text() == cardName && wid->portLabel->fullText == portLabel) {
            systemWidget->m_pInputListWidget->blockSignals(true);
            systemWidget->m_pInputListWidget->setCurrentRow(row);
            systemWidget->m_pInputListWidget->blockSignals(false);
            if (wid->deviceLabel->text().contains("bluez_card")) {
                isCheckBluetoothInput = true;
            }
            qDebug() << "set input list widget" << row;
            break;
        }
    }
}

/*
 * 输入设备中是否包含蓝牙设备
 */
bool UkmediaMainWidget::inputDeviceContainBluetooth()
{
    for (int row=0;row<systemWidget->m_pInputListWidget->count();row++) {

        QListWidgetItem *item = systemWidget->m_pInputListWidget->item(row);
        LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pInputListWidget->itemWidget(item);
        if (wid->deviceLabel->text().contains("bluez")) {
            return true;
        }
    }
    return false;
}

/*
 * 移除xml文件中不能识别的字符
 */
QString UkmediaMainWidget::stringRemoveUnrecignizedChar(QString str)
{
    str.remove(" ");
    str.remove("/");
    str.remove("(");
    str.remove(")");
    str.remove("[");
    str.remove("]");
    return str;
}


QString UkmediaMainWidget::blueCardName()
{
    for (int row=0;row<systemWidget->m_pInputListWidget->count();row++) {

        QListWidgetItem *item = systemWidget->m_pInputListWidget->item(row);
        LingmoUIListWidgetItem *wid = (LingmoUIListWidgetItem *)systemWidget->m_pInputListWidget->itemWidget(item);
        if (wid->deviceLabel->text().contains("bluez")) {
            return wid->deviceLabel->text();
        }
    }
    return "";
}

/**
 * @brief MainWindow::keyPressEvent 按esc键关闭主界面
 * @param event
 */
void UkmediaMainWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideWindow();
    }
    return QWidget::keyPressEvent(event);
}

/*
 *在path路径下找包含str字段的文件名(暂时用来寻找wine微信)
 */
QString UkmediaMainWidget::findFile(const QString path,QString str)
{
    QDir dir(path);
    QString filename = "";
    if(!dir.exists())
    {
        return "";
    }

    //获取filePath下所有文件夹和文件
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);//文件夹|文件|不包含./和../

    //排序文件夹优先
    dir.setSorting(QDir::DirsFirst);

    QStringList filer;
    QStringList list= dir.entryList(filer, QDir::Files | QDir::NoDotAndDotDot);

    if(list.size() == 0)
    {
        return "";
    }

    for(int i = 0; i < list.size(); i++)
    {
        QFileInfo fileInfo = list.at(i);
        if(fileInfo.isDir())//判断是否为文件夹
        {
            findFile(fileInfo.filePath(),str);//递归开始
        }
        else
        {
            if(fileInfo.fileName().contains(str) && !fileInfo.fileName().contains("企业微信") && !fileInfo.fileName().contains("卸载微信"))//设定后缀
            {
                filename = fileInfo.fileName();
                qDebug() << "在/usr/share/applications/找到文件" << filename;
                break;
            }
        }
    }
    return filename;
}

void UkmediaMainWidget::inhibit()
{
    if (m_inhibitFileDescriptor.isValid()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          QStringLiteral("Inhibit"));
    message.setArguments(QVariantList({QStringLiteral("sleep"),
                                       "Screen Locker",
                                       "Ensuring that the screen gets locked before going to sleep",
                                       QStringLiteral("delay")}));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    if (!reply.isValid()) {
        return;
    }
    reply.value().swap(m_inhibitFileDescriptor);
}

void UkmediaMainWidget::uninhibit()
{
    if (!m_inhibitFileDescriptor.isValid()) {
        return;
    }
    m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
}

void UkmediaMainWidget::switchModuleEchoCancel()
{
    if (UkmediaCommon::getInstance().isHuaweiPlatform())
        return;

    bool sourceState = false;
    bool switchState = false;

    if (m_pSoundSettings->keys().contains("dnsNoiseReduction"))
        switchState = m_pSoundSettings->get(DNS_NOISE_REDUCTION).toBool();

    if (strstr(m_pVolumeControl->defaultSourceName, "alsa_input") && !strstr(m_pVolumeControl->defaultSourceName, "input.usb"))
        sourceState = true;

    qDebug() << __func__ << isLoadEchoCancel << m_pVolumeControl->defaultSourceName << switchState << sourceState;
    if ((switchState && sourceState)) {
        pa_operation *o;
        isLoadEchoCancel = true;
        if (!(o = pa_context_load_module(m_pVolumeControl->getContext(),
                                         "module-echo-cancel",
                                         "use_master_format=1 aec_method=webrtc aec_args=analog_gain_control=0 source_name=noiseReduceSource",
                                         nullptr, nullptr))) {
            m_pVolumeControl->showError(QObject::tr("pa_context_load_module() failed").toUtf8().constData());
        }
    } else if ((strstr(m_pVolumeControl->defaultSourceName, "noiseReduceSource") && !switchState)) {
        isLoadEchoCancel = false;
        m_pVolumeControl->getModuleIndexByName("module-echo-cancel");

        QTimer::singleShot(300, this, [=](){
            qDebug() << "Unload module echo cancel" << m_pVolumeControl->findModuleIndex;
            pa_context_unload_module(m_pVolumeControl->getContext(), m_pVolumeControl->findModuleIndex, nullptr, nullptr);
        });
    } else {
        isLoadEchoCancel = false;
    }
}

void UkmediaMainWidget::switchModuleLoopBack()
{
    bool loopbackState = false;
    if (m_pSoundSettings->keys().contains(LOOP_BACK)) {
        loopbackState = m_pSoundSettings->get(LOOP_BACK).toBool();
    }
    if (loopbackState && !m_pVolumeControl->sourceOutputIndexMap.values().contains("Loopback")) {
        pa_operation *o;
        qDebug() << "load module loopback";
        if (!(o=pa_context_load_module(m_pVolumeControl->getContext(),"module-loopback",nullptr,nullptr,nullptr))) {
                m_pVolumeControl->showError(QObject::tr("pa_context_load_module() failed").toUtf8().constData());
        }
        m_pVolumeControl->isLoadLoopback = true;
    }
    else if (!loopbackState){
        m_pVolumeControl->getModuleIndexByName("module-loopback");
        QTimer::singleShot(300,this,[=](){
            qDebug() << "unload module loopback" << m_pVolumeControl->findModuleIndex;
            pa_context_unload_module(m_pVolumeControl->getContext(),m_pVolumeControl->findModuleIndex,nullptr,nullptr);
        });
        m_pVolumeControl->isLoadLoopback = false;
    }
}

/**
 * @brief UkmediaMainWidget::resetVolumeSliderRange
 * 重置系统音量滑动条范围(音量增强)
 * gsetting keys: volume-increase, volume-increase-value
 * volume-increase: 音量增强开关
 * volume-increase: 音量增强最大值
 **/
void UkmediaMainWidget::resetVolumeSliderRange()
{
    if (!QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA))
        return;

    bool status = m_pSoundSettings->get(VOLUME_INCREASE).toBool();
    appWidget->setVolumeSliderRange(status);
    systemWidget->setVolumeSliderRange(status);

    if (!status) {
        QMap<int, sinkInfo>::iterator it;
        for (it = m_pVolumeControl->sinkMap.begin(); it != m_pVolumeControl->sinkMap.end(); ++it) {
            sinkInfo info = it.value();
            int volume = (info.volume.channels >= 2) ? MAX(info.volume.values[0], info.volume.values[1]) : info.volume.values[0];
            if (volume > PA_VOLUME_NORMAL)
                m_pVolumeControl->setSinkVolume(info.index, PA_VOLUME_NORMAL);
        }
    }
}

void UkmediaMainWidget::switchMonoAudio()
{
    bool monoAudioState = false;

    if (m_pSoundSettings->keys().contains("monoAudio"))
        monoAudioState = m_pSoundSettings->get(MONO_AUDIO).toBool();

//   pactl load-module module-remap-sink sink_name=mono channels=2 channel_map=mono,monomonoAudioState = false;
    m_pVolumeControl->getModuleIndexByName("module-remap-sink");
    if(monoAudioState)
    {
        pa_operation *o;
        if (!(o=pa_context_load_module(m_pVolumeControl->getContext(),"module-remap-sink",\
                   "sink_name=mono channels=1 channel_map=mono",nullptr,nullptr))) {
                m_pVolumeControl->showError(QObject::tr("pa_context_load_module() failed").toUtf8().constData());
                qDebug() << "load module-remap-sink module error" << monoAudioState << m_pVolumeControl->findModuleIndex;
        }
//        m_pVolumeControl->setDefaultSink("mono");
        qDebug() << "1 monoAudioState " << monoAudioState;
    }
    else
    {
        if(m_pVolumeControl->masterSinkDev!="")
            m_pVolumeControl->setDefaultSink(m_pVolumeControl->masterSinkDev.toLatin1().data());

        QTimer::singleShot(300,this,[=](){
            qDebug() << "unload module-remap-sink" << m_pVolumeControl->findModuleIndex;
            pa_context_unload_module(m_pVolumeControl->getContext(),m_pVolumeControl->findModuleIndex,nullptr,nullptr);
        });
        qDebug() << "0 monoAudioState " << monoAudioState<< "unload module-remap-sink" << m_pVolumeControl->findModuleIndex;
    }
}


void UkmediaMainWidget::monitorSessionStatus()
{
    //get session path
//    m_sessionActiveBus = QDBusConnection::systemBus();
    QString methodName;
    QVariantList args;
    methodName = QStringLiteral("GetSessionByPID");
    args << (quint32) QCoreApplication::applicationPid();

    QDBusMessage message = QDBusMessage::createMethodCall(m_sessionControllerService,
                                                          m_sessionControllerPath,
                                                          m_sessionControllerManagerInterface,
                                                          methodName);
    message.setArguments(args);
    QDBusPendingReply<QDBusObjectPath> session = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(session, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *self) {
        QDBusPendingReply<QDBusObjectPath> reply = *self;
        self->deleteLater();
        if (!reply.isValid()) {
            qDebug()<< "The session is not registered with " << m_sessionControllerName << " " << reply.error().message();
            return;
        }
        m_sessionPath = reply.value().path();
        qDebug() << "Session path:" << m_sessionPath;

        //get sessionactive
        QDBusConnection::systemBus().connect(m_sessionControllerService,
                          m_sessionPath,
                          m_sessionControllerPropertiesInterface,
                          QStringLiteral("PropertiesChanged"),
                          this,
                          SLOT(getSessionActive()));
    });
}

void UkmediaMainWidget::getSessionActive()
{
    const static QString s_dbusPropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
    QDBusMessage message = QDBusMessage::createMethodCall(m_sessionControllerService,
                                                          m_sessionPath,
                                                          s_dbusPropertiesInterface,
                                                          QStringLiteral("Get"));
    message.setArguments(QVariantList({m_sessionControllerSessionInterface, m_sessionControllerActiveProperty}));
    QDBusPendingReply<QVariant> reply = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
        [this](QDBusPendingCallWatcher *self) {
            QDBusPendingReply<QVariant> reply = *self;
            self->deleteLater();
            if (!reply.isValid()) {
                qDebug() << "Failed to get Active Property of " << m_sessionControllerName << " session:" << reply.error().message();
                return;
            }
            const bool active = reply.value().toBool();
            onPrepareForSleep(!active);

            qDebug() << "active -----" <<active;
        }
    );
}

QString UkmediaMainWidget::getDefaultOutputDevice()
{
    if(m_pVolumeControl->defaultSinkName == ""){
        return "NULL";
    }
    return m_pVolumeControl->defaultSinkName;
}

QString UkmediaMainWidget::getDefaultInputDevice()
{
    if(m_pVolumeControl->defaultSourceName == ""){
        return "NULL";
    }
    return m_pVolumeControl->defaultSourceName;
}

QStringList UkmediaMainWidget::getAllOutputDevices()
{
    QMap<int, sinkInfo>::iterator it;
    QStringList devicesList;
    for (it=m_pVolumeControl->sinkMap.begin();it!=m_pVolumeControl->sinkMap.end();)  {
        devicesList.append(it.value().name);
        ++it;
    }
    return devicesList;
}

QStringList UkmediaMainWidget::getAllInputDevices()
{
    QMap<int, sourceInfo>::iterator it;
    QStringList devicesList;
    for (it=m_pVolumeControl->sourceMap.begin();it!=m_pVolumeControl->sourceMap.end();)  {
        devicesList.append(it.value().name);
        ++it;
    }
    return devicesList;
}

int UkmediaMainWidget::getDefaultOutputVolume()
{
    int value = m_pVolumeControl->sinkVolume;

    if (m_pSoundSettings->keys().contains("volumeIncrease")) {
        if(m_pSoundSettings->get("volume-increase").toBool())
            value = (m_pVolumeControl->sinkVolume) +0.5;
    }

    return paVolumeToValue(value);
}

int UkmediaMainWidget::getDefaultInputVolume()
{
    return paVolumeToValue(m_pVolumeControl->sourceVolume);
}

bool UkmediaMainWidget::getDefaultOutputMuteState()
{
    return m_pVolumeControl->sinkMuted;
}

bool UkmediaMainWidget::getDefaultInputMuteState()
{
    return m_pVolumeControl->sourceMuted;
}

bool UkmediaMainWidget::setDefaultOutputVolume(int value)
{
    if(value < 0 || value > 125){
        qDebug() << "setDefaultOutputVolume value is invalid!";
        return false;
    }

    if (m_pSoundSettings->keys().contains("volumeIncrease")) {
        if(!m_pSoundSettings->get("volume-increase").toBool() && value == 100)
            value = 100;
    }

    m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,valueToPaVolume(value));
    return true;
}

bool UkmediaMainWidget::setDefaultInputVolume(int value)
{
    if(value < 0 || value > 100){
        qDebug() << "setDefaultOutputVolume value is invalid!";
        return false;
    }

    m_pVolumeControl->setSourceVolume(m_pVolumeControl->sourceIndex,valueToPaVolume(value));
    return true;
}

bool UkmediaMainWidget::setDefaultOutputMuteState(bool mute)
{
    if(mute == m_pVolumeControl->sinkMuted)
        return false;

    m_pVolumeControl->setSinkMute(mute);
    return true;
}

bool UkmediaMainWidget::setDefaultInputMuteState(bool mute)
{
    if(mute == m_pVolumeControl->sourceMuted)
        return false;

    m_pVolumeControl->setSourceMute(mute);
    return true;
}

bool UkmediaMainWidget::setDefaultOutputDevice(QString deviceName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    bool isSucceed = false;

    for (it=m_pVolumeControl->sinkPortMap.begin();it!=m_pVolumeControl->sinkPortMap.end();) {
        portNameMap = it.value();
        if(portNameMap.keys().contains(deviceName)){
            isSucceed = m_pVolumeControl->setDefaultSink(deviceName.toLatin1().data());
            return isSucceed;
        }
        ++it;
    }
    return isSucceed;
}

bool UkmediaMainWidget::setDefaultInputDevice(QString deviceName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    bool isSucceed = false;

    for (it=m_pVolumeControl->sourcePortMap.begin();it!=m_pVolumeControl->sourcePortMap.end();) {
        portNameMap = it.value();
        if(portNameMap.keys().contains(deviceName)){
            isSucceed = m_pVolumeControl->setDefaultSource(deviceName.toLatin1().data());
            return isSucceed;
        }
        ++it;
    }
    return isSucceed;
}

void UkmediaMainWidget::sendUpdateVolumeSignal(int soundType, int value)
{
    switch (soundType) {
    case SoundType::SINK:
        Q_EMIT updateVolume(value);
        break;

    case SoundType::SOURCE:
        Q_EMIT updateSourceVolume(value);
        break;

    default:
        return;
    }
}

void UkmediaMainWidget::sendUpdateMuteSignal(int soundType, bool isMute)
{
    switch (soundType) {
    case SoundType::SINK:
        Q_EMIT updateMute(isMute);
        break;

    case SoundType::SOURCE:
        Q_EMIT updateSourceMute(isMute);
        break;

    default:
        return;
    }
}

void UkmediaMainWidget::batteryLevelChanged(QString macAddr, int battery)
{
    qDebug() << "batteryLevelChanged" << macAddr << battery;
    m_pBluetoothDbus->batteryLevel = battery;
    m_pBluetoothDbus->macAddress = macAddr;
    m_pBluetoothDbus->sendBatteryChangedSignal(macAddr, battery);
}

/**
 * @brief UkmediaMainWidget::initBlueDeviceVolume
 * 蓝牙耳机初始化音量(针对硬件音量反馈延迟处理)
 * signal: initBlueDeviceVolumeSig
 **/
void UkmediaMainWidget::initBlueDeviceVolume(int index, QString name)
{
    QTimer *m_timer = new QTimer();
    m_timer->start(200);

    connect(m_timer, &QTimer::timeout, this, [=]() {
        m_pVolumeControl->setSinkVolume(index, OUTPUT_VOLUME);
        m_pVolumeControl->customSoundFile->addXmlNode(name,false);
        m_timer->stop();
    });

    qDebug() << "Initialize the volume of the Bluetooth device " << index << name;
}

void UkmediaMainWidget::playAlertSound(QString soundEvent)
{
    QDBusInterface interface("org.lingmo.sound.theme.player","/org/lingmo/sound/theme/player","org.lingmo.sound.theme.player",QDBusConnection::sessionBus());

    if(!interface.isValid()){
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return;
    }

    QDBusReply<bool> reply = interface.call("playAlertSound", soundEvent);

    if(reply.isValid()){
        bool value = reply.value();
        qDebug() << QString("Method Client playAlertSound = %1").arg(value);
    }
    else
        qDebug() << "playAlertSound Method call failed !";
}

//  获取系统默认输出设备
QString UkmediaMainWidget::getSystemOutputDevice()
{
    return findOutputPortLabel(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->sinkPortName);
}

//  获取系统默认输入设备
QString UkmediaMainWidget::getSystemInputDevice()
{
    return findInputPortLabel(m_pVolumeControl->defaultInputCard,m_pVolumeControl->sourcePortName);
}

//  获取app名单
QStringList UkmediaMainWidget::getAppList()
{
    return m_pVolumeControl->sinkInputList;
}

QList<QVariant> UkmediaMainWidget::getPlaybackAppInfo()
{
    qDBusRegisterMetaType<appInfo>();
    QList<QVariant> infoList;
    QMap<int, appInfo>::iterator it;

    for(it = m_pVolumeControl->sinkInputMap.begin(); it != m_pVolumeControl->sinkInputMap.end(); it++) {
        QVariant info;
        info.setValue(it.value());
        infoList.append(info);
    }

    return infoList;
}

QList<QVariant> UkmediaMainWidget::getRecordAppInfo()
{
    qDBusRegisterMetaType<appInfo>();
    QList<QVariant> infoList;
    QMap<int, appInfo>::iterator it;

    for(it = m_pVolumeControl->sourceOutputMap.begin(); it != m_pVolumeControl->sourceOutputMap.end(); it++) {
        QVariant info;
        info.setValue(it.value());
        infoList.append(info);
    }

    return infoList;
}

//  获取应用音量
int UkmediaMainWidget::getAppVolume(QString appName)
{
    int volume = m_pVolumeControl->getSinkInputVolume(appName.toLatin1().data());
    return paVolumeToValue(volume);
}

//  获取应用静音状态
bool UkmediaMainWidget::getAppMuteState(QString appName)
{
    bool state = false;
    QMap<QString, int>::iterator it;

    for (it = m_pVolumeControl->sinkInputMuteMap.begin(); it != m_pVolumeControl->sinkInputMuteMap.end(); it++) {
        if (appName == it.key()) {
            state = it.value();
            break;
        }
    }

    return state;
}

//  获取应用对应的输出设备
QString UkmediaMainWidget::getAppOutputDevice(QString appName)
{
    int cardIndex = -1;
    int sinkIndex = -1;
    QString portName = "";
    QString portLabel = "";
    QMap<int, appInfo>::iterator it;
    QMap<int, QString>::iterator at;

    //  1. 获取应用对应sink name
    for(it = m_pVolumeControl->sinkInputMap.begin(); it != m_pVolumeControl->sinkInputMap.end(); it++) {
        appInfo info = it.value();
        if (appName == info.name) {
            sinkIndex = info.masterIndex;
            break;
        }
    }

    //  2.获取端口
    cardIndex = m_pVolumeControl->sinkMap.value(sinkIndex).card;
    portName = m_pVolumeControl->sinkMap.value(sinkIndex).active_port_description;
    for (at = currentOutputPortLabelMap.begin(); at != currentOutputPortLabelMap.end(); at++) {
        if (cardIndex == at.key() && at.value().contains(portName)) {
            portLabel = at.value();
            break;
        }
    }

    if (portLabel.isEmpty()) {
        QString masterDevName = m_pVolumeControl->sinkMap.value(sinkIndex).master_device;
        int cardIndex = m_pVolumeControl->findMasterDeviceCardIndex(masterDevName);
        for (at = currentOutputPortLabelMap.begin(); at != currentOutputPortLabelMap.end(); at++) {
            if (cardIndex == at.key()) {
                portLabel = at.value();
                break;
            }
        }
    }

    portLabel = (portLabel != "") ? portLabel : getSystemOutputDevice();
    return portLabel;
}

//  获取应用对应的输入设备
QString UkmediaMainWidget::getAppInputDevice(QString appName)
{
    int cardIndex = -1;
    int sourceIndex = -1;
    QString portName = "";
    QString portLabel = "";
    QMap<int, appInfo>::iterator it;
    QMap<int, QString>::iterator at;

    for(it = m_pVolumeControl->sourceOutputMap.begin(); it != m_pVolumeControl->sourceOutputMap.end(); it++) {
        appInfo info = it.value();
        if (appName == info.name) {
            sourceIndex = info.masterIndex;
            break;
        }
    }


    cardIndex = m_pVolumeControl->sourceMap.value(sourceIndex).card;
    portName = m_pVolumeControl->sourceMap.value(sourceIndex).active_port_description;

    if (sourceIndex == -1 && portName == "")
        return getSystemInputDevice();

    for (at = currentInputPortLabelMap.begin(); at != currentInputPortLabelMap.end(); at++) {
        if (cardIndex == at.key() && at.value().contains(portName)) {
            portLabel = at.value();
            break;
        }
    }

    portLabel = (portLabel != "") ? portLabel : getSystemInputDevice();
    return portLabel;
}

//  设置应用静音
bool UkmediaMainWidget::setAppMuteState(QString appName, bool state)
{
    bool n_SinkInputStatus = m_pVolumeControl->setSinkInputMuted(appName, state);
    bool n_SourceOutputStatus = m_pVolumeControl->setSourceOutputMuted(appName, state);
    return (n_SinkInputStatus || n_SourceOutputStatus) ? true : false;
}

//  设置应用音量
bool UkmediaMainWidget::setAppVolume(QString appName, int value)
{
    int appIndex = -1;
    QMap<int, appInfo>::iterator it;

    for(it = m_pVolumeControl->sinkInputMap.begin(); it != m_pVolumeControl->sinkInputMap.end(); it++) {
        appInfo info = it.value();
        if (appName == info.name)
            m_pVolumeControl->setSinkInputVolume(info.index, valueToPaVolume(value), info.channel);
    }

    if (appName == "lingmo-music" || appName == "lingmo-video") {
        QDBusMessage message =QDBusMessage::createSignal("/", "org.lingmo.media", "sinkVolumeChanged");
        message<< appName << value << getAppMuteState(appName);
        QDBusConnection::sessionBus().send(message);
        qDebug() << "Send SIGNAL: sinkVolumeChanged. Case: Synchronous volume status" << appName;
    }

    return (appIndex != -1) ? true : false;
}

//  设置系统默认输出设备
bool UkmediaMainWidget::setSystemOutputDevice(QString portLabel)
{
    int index = indexOfOutputPortInOutputListWidget(portLabel);

    if (index >= 0) {
        systemWidget->m_pOutputListWidget->setCurrentRow(index);
        outputListWidgetCurrentRowChangedSlot(index);
        outputListWidgetRow = index;
        return true;
    }

    return false;
}

//  设置系统默认输入设备
bool UkmediaMainWidget::setSystemInputDevice(QString portLabel)
{
    int index = indexOfInputPortInInputListWidget(portLabel);
    if (index >= 0) {
        systemWidget->m_pInputListWidget->setCurrentRow(index);
        return true;
    }

    return false;
}

//  设置应用默认输出设备
bool UkmediaMainWidget::setAppOutputDevice(QString appName, int cardIndex, QString sinkPortName)
{
    if (cardIndex == -1)
        return false;

    QString sinkStr = findPortSink(cardIndex, sinkPortName);

    if (sinkStr == "")
        return false;

    m_pVolumeControl->moveSinkInput(appName, sinkStr.toLatin1().data());
    return true;
}

// 设置应用默认输入设备
bool UkmediaMainWidget::setAppInputDevice(QString appName, int cardIndex, QString sourcePortName)
{
    if (cardIndex == -1)
        return false;

    QString sourceStr = findPortSource(cardIndex, sourcePortName);
    if (sourceStr == "")
        return false;

    m_pVolumeControl->moveSoureOutput(appName, sourceStr.toLatin1().data());
    return true;
}

/**
 * @brief UkmediaMainWidget::isPortHidingNeeded
 * UKCC应用音量UI是否需要隐藏某端口
 * soundType: 输入 or 输出
 * cardIndex: 声卡idx
 * portLabel: 端口描述名
 * return:    True隐藏，False无需隐藏
 **/
bool UkmediaMainWidget::isPortHidingNeeded(int soundType, int cardIndex, QString portLabel)
{
    if (cardIndex < 0 || portLabel.isEmpty())
        return true;

    portLabel = portLabel.split("（").at(0);

    if (soundType == SoundType::SINK) {
        QMap<int, sinkInfo>::iterator itor;
        for (itor = m_pVolumeControl->sinkMap.begin(); itor != m_pVolumeControl->sinkMap.end(); ++itor) {
            sinkInfo outInfo = itor.value();
            QString active_port = outInfo.active_port_description;
            QList<QString> list;

            for (int i = 0; i < outInfo.sink_port_list.size(); ++i)
                list.append(outInfo.sink_port_list.at(i).description);

            // 确认对应sink,如果为活跃端口则显示
            if (cardIndex == outInfo.card && list.contains(portLabel))
                return (portLabel == active_port) ? false : true;
        }
        return true;
    }

    else if (soundType == SoundType::SOURCE) {
        QMap<int, sourceInfo>::iterator itor;
        for (itor = m_pVolumeControl->sourceMap.begin(); itor != m_pVolumeControl->sourceMap.end(); ++itor) {

            sourceInfo inInfo = itor.value();
            QString active_port = inInfo.active_port_description;
            QList<QString> list;

            for (int i = 0; i < inInfo.source_port_list.size(); ++i)
                list.append(inInfo.source_port_list.at(i).description);

            if (cardIndex == inInfo.card && list.contains(portLabel))
                return (portLabel == active_port) ? false : true;
        }
        return true;
    }
}

UkmediaMainWidget::~UkmediaMainWidget()
{
    if (m_panelGSettings) {
        delete m_panelGSettings;
        m_panelGSettings = nullptr;
    }
    if (soundSystemTrayIcon) {
        delete soundSystemTrayIcon;
        soundSystemTrayIcon = nullptr;
    }
}
