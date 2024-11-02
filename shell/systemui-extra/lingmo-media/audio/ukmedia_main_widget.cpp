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
#include "ukmedia_main_widget.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QStringList>
#include <QSpacerItem>
#include <QListView>
#include <QScrollBar>
#include <QGSettings>
#include <QPixmap>
#include <qmath.h>

#define GVC_SOUND_SOUND    (xmlChar *) "sound"
#define GVC_SOUND_NAME     (xmlChar *) "name"
#define GVC_SOUND_FILENAME (xmlChar *) "filename"

#define KEYBINDINGS_CUSTOM_SCHEMA "org.lingmo.media.sound"
#define KEYBINDINGS_CUSTOM_DIR "/org/lingmo/sound/keybindings/"
#define SOUND_THEME_DIR "/usr/share/sounds"

#define SOUND_FILE_STARTUP_PATH "/usr/share/lingmo-media/sounds/startup.ogg"
#define SOUND_FILE_SHUTDOWN_PATH "/usr/share/lingmo-media/sounds/shutdown.ogg"
#define SOUND_FILE_LOGOUT_PATH "/usr/share/lingmo-media/sounds/logout.ogg"
#define SOUND_FILE_WEAKUP_PATH "/usr/share/lingmo-media/sounds/weakup.ogg"

#define MAX_CUSTOM_SHORTCUTS 1000

#define FILENAME_KEY "filename"
#define NAME_KEY "name"

guint appnum = 0;
extern bool isCheckBluetoothInput;

enum {
    SOUND_TYPE_UNSET,
    SOUND_TYPE_OFF,
    SOUND_TYPE_DEFAULT_FROM_THEME,
    SOUND_TYPE_BUILTIN,
    SOUND_TYPE_CUSTOM
};

UkmediaMainWidget::UkmediaMainWidget(QWidget *parent) :
    QWidget(parent),
    m_languageEnvStr(getenv("LANGUAGE")),
    m_version(UkmediaCommon::getInstance().getSystemVersion())
{
    m_pVolumeControl = new UkmediaVolumeControl;

    //  获取声音gsettings值
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA))
        m_pSoundSettings = new QGSettings(KEY_SOUNDS_SCHEMA);

    //  初始化界面
    initWidget();
    initLabelAlignment();
    //  初始化系统音效主题
    setupThemeSelector(this);
    updateTheme(this);

    //  初始化自定义音效
    initAlertSound(SOUND_THEME_DIR);

    //  初始化gsetting值
    initGsettings();

    time = new QTimer();

    //  处理槽函数
    dealSlot();
}

/*
 * 初始化界面
 */
void UkmediaMainWidget::initWidget()
{
    m_pOutputWidget = new UkmediaOutputWidget();
    m_pInputWidget = new UkmediaInputWidget();
    m_pSoundWidget = new UkmediaSoundEffectsWidget();
    m_pSettingsWidget = new UkmediaSettingsWidget();
    cboxfirstEntry = true;
    mThemeName = LINGMO_THEME_LIGHT;
    QVBoxLayout *m_pvLayout = new QVBoxLayout();
    m_pvLayout->addWidget(m_pOutputWidget);
    m_pvLayout->addWidget(m_pInputWidget);
    m_pvLayout->addWidget(m_pSoundWidget);
    m_pvLayout->addWidget(m_pSettingsWidget);
    m_pvLayout->setSpacing(40);
    m_pvLayout->addStretch();

    this->setLayout(m_pvLayout);
    this->setMinimumWidth(0);
    this->setMaximumWidth(16777215);
    this->layout()->setContentsMargins(0,0,0,0);

    m_pInputWidget->m_pInputLevelProgressBar->setMaximum(101);

    /* The following functions are not integrated into lingmo.
     * Therefore, need to hide the corresponding modules */
    if (SystemVersion::SYSTEM_VERSION_OKNILE == m_version)
    {
        m_pOutputWidget->m_pMonoLine->hide();
        m_pOutputWidget->m_pMonoAudioWidget->hide();
        m_pInputWidget->m_pLoopbackLine->hide();
        m_pInputWidget->m_pLoopBackWidget->hide();
        m_pSettingsWidget->hide();
    }
}

QList<char *> UkmediaMainWidget::listExistsPath()
{
    char ** childs;
    int len;

    DConfClient * client = dconf_client_new();
    childs = dconf_client_list (client, KEYBINDINGS_CUSTOM_DIR, &len);
    g_object_unref (client);

    QList<char *> vals;

    for (int i = 0; childs[i] != NULL; i++){
        if (dconf_is_rel_dir (childs[i], NULL)){
            char * val = g_strdup (childs[i]);

            vals.append(val);
        }
    }
    g_strfreev (childs);
    return vals;
}

QString UkmediaMainWidget::findFreePath(){
    int i = 0;
    char * dir;
    bool found;
    QList<char *> existsdirs;

    existsdirs = listExistsPath();

    for (; i < MAX_CUSTOM_SHORTCUTS; i++){
        found = true;
        dir = QString("custom%1/").arg(i).toLatin1().data();
        for (int j = 0; j < existsdirs.count(); j++)
            if (!g_strcmp0(dir, existsdirs.at(j))){
                found = false;
                break;
            }
        if (found)
            break;
    }

    if (i == MAX_CUSTOM_SHORTCUTS){
        qDebug() << "Keyboard Shortcuts" << "Too many custom shortcuts";
        return "";
    }

    return QString("%1%2").arg(KEYBINDINGS_CUSTOM_DIR).arg(QString(dir));
}

void UkmediaMainWidget::addValue(QString name,QString filename)
{
    //在创建setting表时，先判断是否存在该设置，存在时不创建
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();

            g_warning("full path: %s", allpath);
            qDebug() << filenameStr << FILENAME_KEY <<NAME_KEY << nameStr;
            if (nameStr == name) {
                return;
            }
            delete settings;
        }
        else {
            continue;
        }

    }
    QString availablepath = findFreePath();

    const QByteArray id(KEYBINDINGS_CUSTOM_SCHEMA);
    const QByteArray idd(availablepath.toUtf8().data());
    if(QGSettings::isSchemaInstalled(id))
    {
        QGSettings * settings = new QGSettings(id, idd);
        settings->set(FILENAME_KEY, filename);
        settings->set(NAME_KEY, name);
    }
}

void UkmediaMainWidget::initLabelAlignment()
{
    if (!UkmediaCommon::getInstance().isHWKLanguageEnv(m_languageEnvStr)) {
        m_pInputWidget->setLabelAlignment(Qt::AlignCenter);
        m_pOutputWidget->setLabelAlignment(Qt::AlignCenter);
    }
    else {
        m_pInputWidget->setLabelAlignment(Qt::AlignLeft | Qt::AlignCenter);
        m_pOutputWidget->setLabelAlignment(Qt::AlignLeft | Qt::AlignCenter);
    }
}

/*
 * 初始化gsetting
 */
void UkmediaMainWidget::initGsettings()
{
    //获取声音gsettings值
    bool status;
    if (m_pSoundSettings->keys().contains("eventSounds")) {
        status = m_pSoundSettings->get(EVENT_SOUNDS_KEY).toBool();
        m_pSoundWidget->m_pAlertSoundSwitchButton->setChecked(status);
        m_pSoundWidget->alertSoundWidgetShow(status);//初始化提示音窗口部件是否显示
    }
    if (m_pSoundSettings->keys().contains("dnsNoiseReduction")) {
        status = m_pSoundSettings->get(DNS_NOISE_REDUCTION).toBool();
        m_pInputWidget->m_pNoiseReducteButton->setChecked(status);
    }
    if (m_pSoundSettings->keys().contains("loopback")) {
        status = m_pSoundSettings->get(LOOP_BACK).toBool();
        m_pInputWidget->m_pLoopBackButton->setChecked(status);
    }
    if (m_pSoundSettings->keys().contains("monoAudio")) {
        status = m_pSoundSettings->get(MONO_AUDIO).toBool();
        m_pOutputWidget->m_pMonoAudioButton->setChecked(status);
        //需求31268待2501再合入
#if 1
        if (m_pVolumeControl->outputPortMap.count() == 0) {
            m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
            m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
        }
        else {
            if (status) {
                m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
            }
            else {
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
            }
        }
#elif 0
        if (status) {
            m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
            m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
        } else {
            m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
        }
#endif
    }
    if (m_pSoundSettings->keys().contains("volumeIncrease")) {
        status = m_pSoundSettings->get(VOLUME_INCREASE).toBool();
        m_pOutputWidget->m_pVolumeIncreaseButton->setChecked(status);
        m_pOutputWidget->setVolumeSliderRange(status);
    }
    if(m_pSoundSettings->keys().contains("themeName")) {
       QString soundThemeStr = m_pSoundSettings->get(SOUND_THEME_KEY).toString();
        int index = m_pSoundWidget->m_pSoundThemeCombobox->findData(soundThemeStr);
        m_pSoundWidget->m_pSoundThemeCombobox->setCurrentIndex(index);
    }
    if (m_pSoundSettings->keys().contains("customTheme")) {
       if (m_pSoundSettings->get(SOUND_CUSTOM_THEME_KEY).toBool()) {
           m_pSoundWidget->m_pSoundThemeCombobox->blockSignals(true);
           m_pSoundWidget->m_pSoundThemeCombobox->setCurrentText(tr("Custom"));
           m_pSoundWidget->m_pSoundThemeCombobox->blockSignals(false);
       }
    }
    if (m_pSoundSettings->keys().contains("notificationGeneral")) {
        QString notification = m_pSoundSettings->get(NOTIFICATION_GENGERAL).toString();
        int index = m_pSoundWidget->m_pNotificationCombobox->findData(notification);
        m_pSoundWidget->m_pNotificationCombobox->setCurrentIndex(index);
    }
    if (m_pSoundSettings->keys().contains("audioVolumeChange")) {
        QString volumeControl = m_pSoundSettings->get(AUDIO_VOLUME_CHANGE).toString();
        int index = m_pSoundWidget->m_pVolumeChangeCombobox->findData(volumeControl);
        m_pSoundWidget->m_pVolumeChangeCombobox->setCurrentIndex(index);
    }

    connect(m_pSoundSettings, SIGNAL(changed(const QString &)),this,SLOT(onKeyChanged(const QString &)));

    //  检测系统全局主题
    if (QGSettings::isSchemaInstalled(LINGMO_GLOBALTHEME_SETTINGS)) {
        m_pGlobalThemeSetting = new QGSettings(LINGMO_GLOBALTHEME_SETTINGS);
        QString globalTheme = m_pGlobalThemeSetting->get(GLOBAL_THEME_NAME).toString();
        if (globalTheme != "custom") {
            m_pSoundSettings->set(SOUND_THEME_KEY, globalTheme);
            m_pSoundSettings->set(SOUND_CUSTOM_THEME_KEY, false);
        }
        connect(m_pGlobalThemeSetting, SIGNAL(changed(const QString &)), this, SLOT(globalThemeChangedSlot(QString)));
    }

    //检测系统主题
    if (QGSettings::isSchemaInstalled(LINGMO_THEME_SETTING)){
        m_pThemeSetting = new QGSettings(LINGMO_THEME_SETTING);
        if (m_pThemeSetting->keys().contains("styleName")) {
            mThemeName = m_pThemeSetting->get(LINGMO_THEME_NAME).toString();
        }
        connect(m_pThemeSetting, SIGNAL(changed(const QString &)),this,SLOT(lingmoThemeChangedSlot(const QString &)));
    }

    //检测设计开关机音乐
    if (QGSettings::isSchemaInstalled(LINGMO_SWITCH_SETTING)) {
        m_pBootSetting = new QGSettings(LINGMO_SWITCH_SETTING);
//        if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
//            if (m_pSoundSettings->keys().contains("startupMusic")) {
//               m_pSoundWidget->m_pStartupButton->setChecked(m_pSoundSettings->get(STARTUP_MUSIC).toBool());//开机音乐
//            }
//        }
        if (m_pBootSetting->keys().contains("startupMusic")) {
           bool startup = m_pBootSetting->get(LINGMO_STARTUP_MUSIC_KEY).toBool();//开机音乐
           m_pSoundWidget->m_pStartupButton->setChecked(startup);
        }
        if (m_pBootSetting->keys().contains("poweroffMusic")) {
            bool poweroff = m_pBootSetting->get(LINGMO_POWEROFF_MUSIC_KEY).toBool();//关机音乐
            m_pSoundWidget->m_pPoweroffButton->setChecked(poweroff);
        }
        if (m_pBootSetting->keys().contains("logoutMusic")) {
            bool logout = m_pBootSetting->get(LINGMO_LOGOUT_MUSIC_KEY).toBool();//注销音乐
            m_pSoundWidget->m_pLogoutButton->setChecked(logout);
        }
        if (m_pBootSetting->keys().contains("weakupMusic")) {
            bool m_hasMusic = m_pBootSetting->get(LINGMO_WAKEUP_MUSIC_KEY).toBool();//休眠音乐
            m_pSoundWidget->m_pWakeupMusicButton->setChecked(m_hasMusic);
        }
        connect(m_pBootSetting,SIGNAL(changed(const QString &)),this,SLOT(bootMusicSettingsChanged(const QString &)));
    }
}

void UkmediaMainWidget::initButtonSliderStatus(QString key)
{
    if (key == "eventSounds") {
        if (m_pSoundSettings->keys().contains("eventSounds"))
            m_pSoundWidget->m_pAlertSoundSwitchButton->setChecked(m_pSoundSettings->get(EVENT_SOUNDS_KEY).toBool());
    }
    if (key == "dnsNoiseReduction") {
        if (m_pSoundSettings->keys().contains("dnsNoiseReduction"))
            m_pInputWidget->m_pNoiseReducteButton->setChecked(m_pSoundSettings->get(DNS_NOISE_REDUCTION).toBool());
    }
    else if (key == "loopback") {
        if (m_pSoundSettings->keys().contains("loopback"))
            m_pInputWidget->m_pLoopBackButton->setChecked(m_pSoundSettings->get(LOOP_BACK).toBool());
    }
    else if (key == "volumeIncrease") {
        if (m_pSoundSettings->keys().contains("volumeIncrease")) {
            bool status = m_pSoundSettings->get(VOLUME_INCREASE).toBool();
            m_pOutputWidget->m_pVolumeIncreaseButton->setChecked(status);
            m_pOutputWidget->setVolumeSliderRange(status);
        }
    }
    else if (key == "monoAudio") {
        if (m_pSoundSettings->keys().contains("monoAudio")) {
            m_pOutputWidget->m_pMonoAudioButton->setChecked(m_pSoundSettings->get(MONO_AUDIO).toBool());
        }
    }
    else if (key == "themeName") {
        int index = m_pSoundWidget->m_pSoundThemeCombobox->findData(m_pSoundSettings->get(SOUND_THEME_KEY).toString());
        m_pSoundWidget->m_pSoundThemeCombobox->blockSignals(true);
        m_pSoundWidget->m_pSoundThemeCombobox->setCurrentIndex(index);
        m_pSoundWidget->m_pSoundThemeCombobox->blockSignals(false);
    }
    else if (key == "customTheme") {
        if (m_pSoundSettings->get(SOUND_CUSTOM_THEME_KEY).toBool())
            m_pSoundWidget->m_pSoundThemeCombobox->setCurrentText(tr("Custom"));
    }
    else if (key == "audioVolumeChange") {
        int index = m_pSoundWidget->m_pVolumeChangeCombobox->findData(m_pSoundSettings->get(AUDIO_VOLUME_CHANGE).toString());
        m_pSoundWidget->m_pVolumeChangeCombobox->setCurrentIndex(index);
    }
    else if (key == "notificationGeneral") {
        int index = m_pSoundWidget->m_pNotificationCombobox->findData(m_pSoundSettings->get(NOTIFICATION_GENGERAL).toString());
        m_pSoundWidget->m_pNotificationCombobox->setCurrentIndex(index);
    }
}


/*
 * 处理槽函数
 */
void UkmediaMainWidget::dealSlot()
{
    QTimer::singleShot(500, this, SLOT(initVoulmeSlider()));
    m_boxRepeatTime.start();

    //  输出模块槽函数
    m_boxoutputRepeatTimer.setInterval(1000 * 1);
    m_boxoutputRepeatTimer.setSingleShot(true);
    connect(m_pOutputWidget->m_pDeviceSelectBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){
        m_boxoutputRow = index;
        if (m_boxRepeatTime.elapsed() < (1000 * 1))
        {
            //start 频繁点击重置开启
            m_boxoutputRepeatTimer.stop();
            m_boxoutputRepeatTimer.start();
            //end
        }
        else
        {
            //start 单点一次直接响应
            cboxoutputListWidgetCurrentRowChangedSlot();
            //end
        }
        m_boxRepeatTime.start();
    });
    connect(&m_boxoutputRepeatTimer,&QTimer::timeout,this,&UkmediaMainWidget::cboxoutputListWidgetCurrentRowChangedSlot);
    connect(m_pOutputWidget->m_pOutputIconBtn,SIGNAL(clicked()),this,SLOT(outputMuteButtonSlot()));
    connect(m_pOutputWidget->m_pOpVolumeSlider,SIGNAL(valueChanged(int)),this,SLOT(outputWidgetSliderChangedSlot(int)));
    connect(m_pOutputWidget->m_pOpVolumeSlider,SIGNAL(blueValueChanged(int)),this,SLOT(outputWidgetSliderChangedSlotInBlue(int)));
    connect(m_pOutputWidget->m_pVolumeIncreaseButton,SIGNAL(stateChanged(bool)),this,SLOT(volumeIncreaseBtuuonSwitchChangedSlot(bool)));
    connect(m_pOutputWidget->m_pOpBalanceSlider,SIGNAL(valueChanged(int)),this,SLOT(balanceSliderChangedSlot(int)));
    connect(m_pOutputWidget->m_pMonoAudioButton,SIGNAL(stateChanged(bool)),this,SLOT(monoAudioBtuuonSwitchChangedSlot(bool)));

    //  输入模块槽函数
    m_boxinputRepeatTimer.setInterval(1000*1);
    m_boxinputRepeatTimer.setSingleShot(true);
    connect(m_pInputWidget->m_pInputDeviceSelectBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){
        m_boxinputRow = index;
        if (m_boxRepeatTime.elapsed() < (1000 * 1))
        {
            //start 频繁点击重置开启
            m_boxinputRepeatTimer.stop();
            m_boxinputRepeatTimer.start();
            //end
        }
        else
        {
            //start 单点一次直接响应
            cboxinputListWidgetCurrentRowChangedSlot();
            //end
        }
        m_boxRepeatTime.start();
    });
    connect(&m_boxinputRepeatTimer,&QTimer::timeout,this,&UkmediaMainWidget::cboxinputListWidgetCurrentRowChangedSlot);
    connect(m_pInputWidget->m_pInputIconBtn,SIGNAL(clicked()),this,SLOT(inputMuteButtonSlot()));
    connect(m_pInputWidget->m_pIpVolumeSlider,SIGNAL(valueChanged(int)),this,SLOT(inputWidgetSliderChangedSlot(int)));
    connect(m_pVolumeControl,SIGNAL(peakChangedSignal(double)),this,SLOT(peakVolumeChangedSlot(double)));   //  输入反馈槽函数
    connect(m_pInputWidget->m_pNoiseReducteButton,SIGNAL(stateChanged(bool)),this,SLOT(noiseReductionButtonSwitchChangedSlot(bool)));
    connect(m_pInputWidget->m_pLoopBackButton,SIGNAL(stateChanged(bool)),this,SLOT(loopbackButtonSwitchChangedSlot(bool)));

    //  系统音效模块槽函数
    connect(m_pSoundWidget->m_pStartupButton,SIGNAL(stateChanged(bool)),this,SLOT(startupButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pPoweroffButton,SIGNAL(stateChanged(bool)),this,SLOT(poweroffButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pLogoutButton,SIGNAL(stateChanged(bool)),this,SLOT(logoutMusicButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pWakeupMusicButton,SIGNAL(stateChanged(bool)),this,SLOT(wakeButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pAlertSoundSwitchButton,SIGNAL(stateChanged(bool)),this,SLOT(alertSoundButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pSoundThemeCombobox,SIGNAL(currentIndexChanged(int)),this,SLOT(themeComboxIndexChangedSlot(int)));
    connect(m_pSoundWidget->m_pVolumeChangeCombobox,SIGNAL(currentIndexChanged(int)),this,SLOT(customSoundEffectsSlot(int)));
    connect(m_pSoundWidget->m_pNotificationCombobox,SIGNAL(currentIndexChanged(int)),this,SLOT(customSoundEffectsSlot(int)));

    //  更新设备端口
    connect(m_pVolumeControl,SIGNAL(updatePortSignal()),this,SLOT(updateCboxDevicePort()));
    connect(m_pVolumeControl,SIGNAL(deviceChangedSignal()),this,SLOT(updateComboboxListWidgetItemSlot()));

    timeSlider = new QTimer(this);
    connect(timeSlider,SIGNAL(timeout()),this,SLOT(timeSliderSlot()));

    //判断设备是否为单声道来显示/隐藏 单声道模块
    connect(m_pVolumeControl,&UkmediaVolumeControl::updateMonoAudio,this,[=](bool show){
        if (SystemVersion::SYSTEM_VERSION_OKNILE == m_version) {
            return;
        }

        if (show) {
            m_pOutputWidget->m_pAudioBlanceWidget->show();
            m_pOutputWidget->m_pMonoAudioWidget->show();
        }
        else {
            m_pOutputWidget->m_pAudioBlanceWidget->hide();
            m_pOutputWidget->m_pMonoAudioWidget->hide();
        }
        qDebug() << "mono audio show:" << show;
    });

    //bug#254769/256035 输入设备为内置mic或为空时，需要隐藏侦听功能（控制面板代码实现）并卸载侦听模块
    connect(m_pVolumeControl,&UkmediaVolumeControl::updateLoopBack,this,[=](bool show){
        if (ukcc::UkccCommon::isOpenlingmo())
            return;

        if (show) {
            m_pInputWidget->m_pLoopbackLine->show();
            m_pInputWidget->m_pLoopBackWidget->show();
        }
        else {
            m_pInputWidget->m_pLoopBackButton->setChecked(false);
            m_pInputWidget->m_pLoopbackLine->hide();
            m_pInputWidget->m_pLoopBackWidget->hide();
        }
        qDebug() << "loopback show:" << show;
    });

    connect(m_pVolumeControl,&UkmediaVolumeControl::updateSinkMute,this,[=](bool state) {
            themeChangeIcons();
        });

    connect(m_pVolumeControl,&UkmediaVolumeControl::updateSourceMute,this,[=](bool state) {
            themeChangeIcons();
        });

    //切换输出设备或者音量改变时需要同步更新音量
    connect(m_pVolumeControl,&UkmediaVolumeControl::updateVolume,this,[=](int value,bool state){
        qDebug() << "Sink Volume Changed" << value << paVolumeToValue(value) << state;

        bool isChecked = m_pOutputWidget->m_pVolumeIncreaseButton->isChecked();
        if(!isChecked && value > PA_VOLUME_NORMAL){
            m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,PA_VOLUME_NORMAL);
            return;
        }

        QString percent = QString::number(paVolumeToValue(value));
        float balanceVolume = m_pVolumeControl->getBalanceVolume();

        m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent+"%");
        m_pOutputWidget->m_pOpVolumeSlider->blockSignals(true);
        m_pOutputWidget->m_pOpBalanceSlider->blockSignals(true);
        handleBalanceSlider(balanceVolume);

        m_pOutputWidget->m_pOpVolumeSlider->setValue(paVolumeToValue(value));

        m_pOutputWidget->m_pOpVolumeSlider->blockSignals(false);
        m_pOutputWidget->m_pOpBalanceSlider->blockSignals(false);
        initOutputComboboxItem();
        themeChangeIcons();
    });

    connect(m_pVolumeControl,&UkmediaVolumeControl::updateSourceVolume,this,[=](int value,bool state){
        qDebug() << "Source Volume Changed" << value << paVolumeToValue(value) << state;
        QString percent = QString::number(paVolumeToValue(value));

        m_pInputWidget->m_pIpVolumePercentLabel->setText(percent+"%");

        m_pInputWidget->m_pIpVolumeSlider->blockSignals(true);
        m_pInputWidget->m_pIpVolumeSlider->setValue(paVolumeToValue(value));
        m_pInputWidget->m_pIpVolumeSlider->blockSignals(false);

        //当所有可用的输入设备全部移除，台式机才会出现该情况
        if (strstr(m_pVolumeControl->defaultSourceName,"monitor")) {
            m_pInputWidget->m_pInputLevelProgressBar->setValue(0);
        }

        //start 无输入设备时需要自动关闭侦听,此操作要在pa的回调中执行，否则会误关闭
        if (m_pVolumeControl->sourcePortName =="") {
            m_pSoundSettings->set(LOOP_BACK,false);
        }
        //end

        initInputComboboxItem();
        themeChangeIcons();
    });

    //音频设备管控窗口
    connect(m_pSettingsWidget->m_pDevControlDetailsBtn, &QPushButton::clicked, this, [=](){
        UkmediaDevControlWidget *m_pDevControlWidget = new UkmediaDevControlWidget(this);
        m_pDevControlWidget->setFixedSize(SETTINGSWIDGET_WIDTH, SETTINGSWIDGET_HEIGHT);
        m_pDevControlWidget->setAttribute(Qt::WA_DeleteOnClose);
        m_pDevControlWidget->setWindowModality(Qt::WindowModality::WindowModal);
        m_pDevControlWidget->show();
    });

    // 应用声音管控窗口
    connect(m_pSettingsWidget->m_pAppSoundDetailsBtn, &QPushButton::clicked, this, [=](){
        UkmediaAppCtrlWidget *appCtrlWidget = new UkmediaAppCtrlWidget(this);
        appCtrlWidget->setFixedSize(SETTINGSWIDGET_WIDTH, SETTINGSWIDGET_HEIGHT);
        appCtrlWidget->setAttribute(Qt::WA_DeleteOnClose);
        appCtrlWidget->setWindowModality(Qt::WindowModality::WindowModal);
        appCtrlWidget->show();
    });
}

/*
 * 初始化滑动条的值
 */
void UkmediaMainWidget::initVoulmeSlider()
{
    int sinkVolume = paVolumeToValue(m_pVolumeControl->getSinkVolume());
    int sourceVolume = paVolumeToValue(m_pVolumeControl->getSourceVolume());
    QString percent = QString::number(sinkVolume);
    float balanceVolume = m_pVolumeControl->getBalanceVolume();
    qDebug() <<"initVolumeSlider"  << "sourceVolume" << sourceVolume << "sinkVolume" << sinkVolume;
    m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent+"%");
    percent = QString::number(sourceVolume);
    m_pInputWidget->m_pIpVolumePercentLabel->setText(percent+"%");
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(true);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(true);
    m_pInputWidget->m_pIpVolumeSlider->blockSignals(true);
    handleBalanceSlider(balanceVolume);

    m_pOutputWidget->m_pOpVolumeSlider->setValue(sinkVolume);
    m_pInputWidget->m_pIpVolumeSlider->setValue(sourceVolume);
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(false);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(false);
    m_pInputWidget->m_pIpVolumeSlider->blockSignals(false);

    initOutputComboboxItem();
    initInputComboboxItem();
    themeChangeIcons();
    this->update();
}

void UkmediaMainWidget::handleBalanceSlider(float balanceVolume)
{
    bool status;
    if (m_pSoundSettings->keys().contains("monoAudio")) {
        status = m_pSoundSettings->get(MONO_AUDIO).toBool();
        m_pOutputWidget->m_pMonoAudioButton->blockSignals(true);
        m_pOutputWidget->m_pMonoAudioButton->setChecked(status);
        m_pOutputWidget->m_pMonoAudioButton->blockSignals(false);
        //需求31268待2501再合入
#if 1
        if (m_pVolumeControl->outputPortMap.count() == 0) {
            m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
            m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
        }
        else {
            if (status) {
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
                m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
                //setAllSinkMono(); #bug232332
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
            } else {
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
                m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
            }
        }
#elif 0
        if (status) {
            m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
            m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
            //setAllSinkMono(); #bug232332
            m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
        } else {
            m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
            m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
        }
#endif
    }
}

void UkmediaMainWidget::themeChangeIcons()
{
    int nInputValue = paVolumeToValue(m_pVolumeControl->getSourceVolume());
    int nOutputValue = paVolumeToValue(m_pVolumeControl->getSinkVolume());
    bool inputStatus = m_pVolumeControl->getSourceMute();
    bool outputStatus = m_pVolumeControl->getSinkMute();

    qDebug() << "themeChangeIcons" << nInputValue << inputStatus;
    inputVolumeDarkThemeImage(nInputValue,inputStatus);
    outputVolumeDarkThemeImage(nOutputValue,outputStatus);

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


QPixmap UkmediaMainWidget::drawDarkColoredPixmap(const QPixmap &source)
{
//    QColor currentcolor=HighLightEffect::getCurrentSymbolicColor();
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

QPixmap UkmediaMainWidget::drawLightColoredPixmap(const QPixmap &source)
{
//    QColor currentcolor=HighLightEffect::getCurrentSymbolicColor();
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
 *  初始化combobox的值
 */
#if 0
void UkmediaMainWidget::comboboxCurrentTextInit()
{
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            int index = 0;
            for (int i=0;i<m_pSoundList->count();i++) {
                QString str = m_pSoundList->at(i);
                if (str.contains(filenameStr,Qt::CaseSensitive)) {
                    index = i;
                    break;
                }
            }
            if (nameStr == "alert-sound") {
                QString displayName = m_pSoundNameList->at(index);
                m_pSoundWidget->m_pAlertSoundCombobox->setCurrentText(displayName);
                continue;
            }
            if (nameStr == "window-close") {
                QString displayName = m_pSoundNameList->at(index);
                continue;
            }
            else if (nameStr == "volume-changed") {
                QString displayName = m_pSoundNameList->at(index);
                m_pSoundWidget->m_pVolumeChangeCombobox->setCurrentText(displayName);
                continue;
            }
            else if (nameStr == "system-setting") {
                QString displayName = m_pSoundNameList->at(index);
                continue;
            }
        }
        else {
            continue;
        }
    }
}
#endif

/*
 *    是否播放开机音乐
 */
void UkmediaMainWidget::startupButtonSwitchChangedSlot(bool status)
{
//    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
//           if (m_pSoundSettings->keys().contains("startupMusic")) {
//               m_pSoundSettings->set(STARTUP_MUSIC,status);

//           }
//       }
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("startupMusic")) {
        bBootStatus = m_pBootSetting->get(LINGMO_STARTUP_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(LINGMO_STARTUP_MUSIC_KEY,status);
        }
    }
    m_pSoundWidget->m_pStartupButton->setObjectName("m_pSoundWidget->m_pStartupButton");
    ukcc::UkccCommon::buriedSettings("Audio", m_pSoundWidget->m_pStartupButton->objectName(), QString("settings"),status?"true":"false");

    if (status)
    {
        playAlretSoundFromPath(this, SOUND_FILE_STARTUP_PATH);
    }
}

/*
 *   是否播放关机音乐
 */
void UkmediaMainWidget::poweroffButtonSwitchChangedSlot(bool status)
{
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("poweroffMusic")) {
        bBootStatus = m_pBootSetting->get(LINGMO_POWEROFF_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(LINGMO_POWEROFF_MUSIC_KEY,status);
        }
    }
    m_pSoundWidget->m_pPoweroffButton->setObjectName("m_pSoundWidget->m_pPoweroffButton");
    ukcc::UkccCommon::buriedSettings("Audio", m_pSoundWidget->m_pPoweroffButton->objectName(), QString("settings"),status?"true":"false");

    if (status)
    {
        playAlretSoundFromPath(this, SOUND_FILE_SHUTDOWN_PATH);
    }
}

/*
 *   是否播放注销音乐
 */
void UkmediaMainWidget::logoutMusicButtonSwitchChangedSlot(bool status)
{
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("logoutMusic")) {
        bBootStatus = m_pBootSetting->get(LINGMO_LOGOUT_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(LINGMO_LOGOUT_MUSIC_KEY,status);
        }
    }
    m_pSoundWidget->m_pLogoutButton->setObjectName("m_pSoundWidget->m_pLogoutButton");
    ukcc::UkccCommon::buriedSettings("Audio", m_pSoundWidget->m_pLogoutButton->objectName(), QString("settings"),status?"true":"false");

    if (status)
    {
        playAlretSoundFromPath(this, SOUND_FILE_LOGOUT_PATH);
    }
}

/*
 *   是否播放唤醒音乐
 */
void UkmediaMainWidget::wakeButtonSwitchChangedSlot(bool status)
{
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("weakupMusic")) {
        bBootStatus = m_pBootSetting->get(LINGMO_WAKEUP_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(LINGMO_WAKEUP_MUSIC_KEY,status);
        }
    }
    m_pSoundWidget->m_pWakeupMusicButton->setObjectName("m_pSoundWidget->m_pWakeupMusicButton");
    ukcc::UkccCommon::buriedSettings("Audio", m_pSoundWidget->m_pWakeupMusicButton->objectName(), QString("settings"),status?"true":"false");

    if (status)
    {
        playAlretSoundFromPath(this, SOUND_FILE_WEAKUP_PATH);
    }
}

/*
 *   提示音的开关
 */
void UkmediaMainWidget::alertSoundButtonSwitchChangedSlot(bool status)
{
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        if (m_pSoundSettings->keys().contains("eventSounds")) {
            m_pSoundSettings->set(EVENT_SOUNDS_KEY,status);
        }
    }

    m_pSoundWidget->alertSoundWidgetShow(status);
    m_pSoundWidget->m_pAlertSoundSwitchButton->setObjectName("m_pSoundWidget->m_pAlertSoundSwitchButton");
    ukcc::UkccCommon::buriedSettings("Audio", m_pSoundWidget->m_pAlertSoundSwitchButton->objectName(), QString("settings"),status?"true":"false");
}

/*
 * 开启单声道，遍历sinkMap，将所有双声道sink设置为单声道输出
 */
void UkmediaMainWidget::setAllSinkMono()
{
    QMap<int, sinkInfo>::iterator it;
    for(it=m_pVolumeControl->sinkMap.begin(); it!=m_pVolumeControl->sinkMap.end();)
    {
        int sinkVolume;
        if (it.value().volume.channels >= 2)
            sinkVolume = MAX(it.value().volume.values[0], it.value().volume.values[1]);
        else
            sinkVolume = it.value().volume.values[0];

        balanceSliderChangedMono(0, sinkVolume, it.key());
        ++it;
    }
}

void UkmediaMainWidget::monoAudioBtuuonSwitchChangedSlot(bool status)
{
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        if (m_pSoundSettings->keys().contains("monoAudio")) {
            m_pSoundSettings->set(MONO_AUDIO,status);
            //需求31268待2501再合入
#if 1
            if (m_pVolumeControl->outputPortMap.count() == 0) {
                m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
            }
            else {
                if(status){
                    m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
                    setAllSinkMono();
                    m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
                }
                else{
                    float balanceVolume = m_pVolumeControl->getBalanceVolume();
                    m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
                    m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
                }
            }
#elif 0
            if (status) {
                m_pOutputWidget->m_pOpBalanceSlider->setValue(0);
                setAllSinkMono();
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
            } else {
                float balanceVolume = m_pVolumeControl->getBalanceVolume();
                m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
                m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
            }
#endif
        }
    }
    m_pOutputWidget->m_pMonoAudioButton->setObjectName("m_pOutputWidget->m_pMonoAudioButton");
    ukcc::UkccCommon::buriedSettings("Audio",m_pOutputWidget->m_pMonoAudioButton->objectName(), QString("settings"),status?"true":"false");
}

void UkmediaMainWidget::noiseReductionButtonSwitchChangedSlot(bool status)
{
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        if (m_pSoundSettings->keys().contains("dnsNoiseReduction")) {
            m_pSoundSettings->set(DNS_NOISE_REDUCTION,status);
        }
    }
    m_pInputWidget->m_pNoiseReducteButton->setObjectName("m_pInputWidget->m_pNoiseReducteButton");
    ukcc::UkccCommon::buriedSettings("Audio",m_pInputWidget->m_pNoiseReducteButton->objectName(), QString("settings"),status?"true":"false");
}

void UkmediaMainWidget::loopbackButtonSwitchChangedSlot(bool status)
{
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        if (m_pSoundSettings->keys().contains("loopback")) {
            m_pSoundSettings->set(LOOP_BACK,status);
        }
    }
    m_pInputWidget->m_pLoopBackButton->setObjectName("m_pInputWidget->m_pLoopBackButton");
    ukcc::UkccCommon::buriedSettings("Audio",m_pInputWidget->m_pLoopBackButton->objectName(), QString("settings"),status?"true":"false");
}

void UkmediaMainWidget::volumeIncreaseBtuuonSwitchChangedSlot(bool status)
{
    // 1. Refresh UI (Tips: UKCC Audio only refresh UI, Volume adjustment is done by lingmo-media)
    m_pOutputWidget->setVolumeSliderRange(status);

    // 2. Update gsetting value
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA) && m_pSoundSettings->keys().contains("volumeIncrease"))
        m_pSoundSettings->set(VOLUME_INCREASE, status);

    // 3. Data burial point
    m_pOutputWidget->m_pVolumeIncreaseButton->setObjectName("m_pOutputWidget->m_pVolumeIncreaseButton");
    ukcc::UkccCommon::buriedSettings("Audio", m_pOutputWidget->m_pVolumeIncreaseButton->objectName(), QString("settings"), status ? "true" : "false");
}

void UkmediaMainWidget::bootMusicSettingsChanged(const QString &key)
{
    bool bBootStatus = true;

    if (QGSettings::isSchemaInstalled(LINGMO_SWITCH_SETTING)){
        if (key == "startupMusic"){
            if (m_pBootSetting->keys().contains("startupMusic"))
                m_pSoundWidget->m_pStartupButton->setChecked(m_pBootSetting->get(LINGMO_STARTUP_MUSIC_KEY).toBool());
        }
        else if (key == "poweroffMusic"){
            if (m_pBootSetting->keys().contains("poweroffMusic"))
                m_pSoundWidget->m_pPoweroffButton->setChecked(m_pBootSetting->get(LINGMO_POWEROFF_MUSIC_KEY).toBool());
        }
        else if (key == "logoutMusic"){
            if (m_pBootSetting->keys().contains("logoutMusic"))
                m_pSoundWidget->m_pLogoutButton->setChecked(m_pBootSetting->get(LINGMO_LOGOUT_MUSIC_KEY).toBool());
        }
        else if (key == "weakupMusic"){
            if (m_pBootSetting->keys().contains("weakupMusic"))
                m_pSoundWidget->m_pWakeupMusicButton->setChecked(m_pBootSetting->get(LINGMO_WAKEUP_MUSIC_KEY).toBool());
        }
    }
}

/*
 *   系统主题更改
 */
void UkmediaMainWidget::lingmoThemeChangedSlot(const QString &themeStr)
{
    if (m_pThemeSetting->keys().contains("styleName")) {
        mThemeName = m_pThemeSetting->get(LINGMO_THEME_NAME).toString();
    }
    int nInputValue = getInputVolume();
    int nOutputValue = getOutputVolume();
    bool inputStatus = m_pVolumeControl->getSourceMute();
    bool outputStatus = m_pVolumeControl->getSinkMute();
    inputVolumeDarkThemeImage(nInputValue,inputStatus);
    outputVolumeDarkThemeImage(nOutputValue,outputStatus);
    Q_EMIT qApp->paletteChanged(qApp->palette());
    this->repaint();
}

/*
 *   获取输入音量值
 */
int UkmediaMainWidget::getInputVolume()
{
    return m_pInputWidget->m_pIpVolumeSlider->value();
}

/*
 *   获取输出音量值
 */
int UkmediaMainWidget::getOutputVolume()
{
    return m_pOutputWidget->m_pOpVolumeSlider->value();
}

/*
 *   深色主题时输出音量图标
 */
void UkmediaMainWidget::outputVolumeDarkThemeImage(int value,bool status)
{
    QImage image;
    QColor color = QColor(0,0,0,216);
    if (mThemeName == LINGMO_THEME_LIGHT || mThemeName == LINGMO_THEME_DEFAULT) {
        color = QColor(0,0,0,216);
    }
    else if (mThemeName == LINGMO_THEME_DARK) {
        color = QColor(255,255,255,216);
    }

    if (status) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value <= 0) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value > 0 && value <= 33) {
        image = QIcon::fromTheme("audio-volume-low-symbolic").pixmap(24,24).toImage();
    }
    else if (value >33 && value <= 66) {
        image = QIcon::fromTheme("audio-volume-medium-symbolic").pixmap(24,24).toImage();
    }
    else {
        image = QIcon::fromTheme("audio-volume-high-symbolic").pixmap(24,24).toImage();
    }

    m_pOutputWidget->m_pOutputIconBtn->refreshIcon(image, color);
}

/*
 *   输入音量图标
 */
void UkmediaMainWidget::inputVolumeDarkThemeImage(int value,bool status)
{
    QImage image;
    QColor color = QColor(0,0,0,190);
    if (mThemeName == LINGMO_THEME_LIGHT || mThemeName == LINGMO_THEME_DEFAULT) {
        color = QColor(0,0,0,190);
    }
    else if (mThemeName == LINGMO_THEME_DARK) {
        color = QColor(255,255,255,190);
    }

    if (status) {
        image = QIcon::fromTheme("microphone-sensitivity-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value <= 0) {
        image = QIcon::fromTheme("microphone-sensitivity-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value > 0 && value <= 33) {
        image = QIcon::fromTheme("microphone-sensitivity-low-symbolic").pixmap(24,24).toImage();
    }
    else if (value >33 && value <= 66) {
        image = QIcon::fromTheme("microphone-sensitivity-medium-symbolic").pixmap(24,24).toImage();
    }
    else {
        image = QIcon::fromTheme("microphone-sensitivity-high-symbolic").pixmap(24,24).toImage();
    }

    m_pInputWidget->m_pInputIconBtn->refreshIcon(image, color);
}

//void UkmediaMainWidget::onKeyChanged (GSettings *settings,gchar *key,UkmediaMainWidget *m_pWidget)
void UkmediaMainWidget::onKeyChanged (const QString &key)
{
//    Q_UNUSED(settings);
    qDebug() << "onKeyChanged" <<key;
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        if (!strcmp (key.toLatin1().data(), EVENT_SOUNDS_KEY) ||
                !strcmp (key.toLatin1().data(), SOUND_THEME_KEY)) {
            updateTheme (this);
        }
        initButtonSliderStatus(key);
    }
}

void UkmediaMainWidget::globalThemeChangedSlot(const QString &key)
{
    if (! strcmp(key.toLatin1().data(), "globalThemeName")) {
        QString globalTheme = m_pGlobalThemeSetting->get(GLOBAL_THEME_NAME).toString();
        if (globalTheme != "custom") {
            int index = m_pSoundWidget->m_pSoundThemeCombobox->findData(globalTheme);
            m_pSoundWidget->m_pSoundThemeCombobox->setCurrentIndex(index);
        }
    }
}

/*
 *   更新主题
 */
void UkmediaMainWidget::updateTheme (UkmediaMainWidget *m_pWidget)
{
    g_debug("update theme");
//    char *pThemeName;
    QString pThemeName;
    gboolean feedBackEnabled;
    gboolean eventsEnabled;
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        if (m_pWidget->m_pSoundSettings->keys().contains("eventSounds")) {
            eventsEnabled = m_pWidget->m_pSoundSettings->get(EVENT_SOUNDS_KEY).toBool();
        }
        if (eventsEnabled) {
            if (m_pWidget->m_pSoundSettings->keys().contains("themeName")) {
                pThemeName = m_pWidget->m_pSoundSettings->get(SOUND_THEME_KEY).toString();
            }
        } else {
            pThemeName = g_strdup (NO_SOUNDS_THEME_NAME);
        }
    }
    qDebug() << "updateTheme" << pThemeName;
    //设置combox的主题
    setComboxForThemeName (m_pWidget, pThemeName.toLatin1().data());
    updateAlertsFromThemeName (m_pWidget, pThemeName.toLatin1().data());
}

/*
 *   设置主题名到combox
 */
void UkmediaMainWidget::setupThemeSelector (UkmediaMainWidget *m_pWidget)
{
    g_debug("setup theme selector");
    GHashTable  *hash;
    const char * const *dataDirs;
    const char *m_pDataDir;
    char *dir;
    guint i;

    /* Add the theme names and their display name to a hash table,
     * makes it easy to avoid duplicate themes */
    hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    dataDirs = g_get_system_data_dirs ();
    for (i = 0; dataDirs[i] != nullptr; i++) {
        dir = g_build_filename (dataDirs[i], "sounds", nullptr);
        soundThemeInDir (m_pWidget,hash, dir);
    }

    m_pDataDir = g_get_user_data_dir ();
    dir = g_build_filename (m_pDataDir, "sounds", nullptr);
    soundThemeInDir (m_pWidget,hash, dir);

    /* If there isn't at least one theme, make everything
     * insensitive, LAME! */
    if (g_hash_table_size (hash) == 0) {
        g_warning ("Bad setup, install the freedesktop sound theme");
        g_hash_table_destroy (hash);
        return;
    }
    /* Add the themes to a combobox */
    g_hash_table_destroy (hash);
}

/*
 *   主题名所在目录
 */
void UkmediaMainWidget::soundThemeInDir (UkmediaMainWidget *m_pWidget,GHashTable *hash,const char *dir)
{
    Q_UNUSED(hash);
    qDebug() << "sound theme in dir" << dir;
    GDir *d;
    const char *m_pName;
    d = g_dir_open (dir, 0, nullptr);
    if (d == nullptr) {
        return;
    }
    while ((m_pName = g_dir_read_name (d)) != nullptr) {
        char *m_pDirName, *m_pIndex, *m_pIndexName;
        /* Look for directories */
        m_pDirName = g_build_filename (dir, m_pName, nullptr);
        if (g_file_test (m_pDirName, G_FILE_TEST_IS_DIR) == FALSE) {
            continue;
        }

        /* Look for index files */
        m_pIndex = g_build_filename (m_pDirName, "index.theme", nullptr);

        /* Check the name of the theme in the index.theme file */
        m_pIndexName = loadIndexThemeName (m_pIndex, nullptr);
        if (m_pIndexName == nullptr) {
            continue;
        }
        if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
            QString themeName;
            if (m_pWidget->m_pSoundSettings->keys().contains("themeName")) {
                themeName = m_pWidget->m_pSoundSettings->get(SOUND_THEME_KEY).toString();
            }
            qDebug() << "sound theme in dir" << "displayname:" << m_pIndexName << "theme name:" << m_pName << "theme："<< themeName;
            if (m_pName && !strstr(m_pName,"ubuntu") && !strstr(m_pName,"freedesktop") && !strstr(m_pName,"custom")) {
                m_pWidget->m_pThemeNameList.append(m_pName);
                m_pWidget->m_pSoundWidget->m_pSoundThemeCombobox->addItem(m_pIndexName,m_pName);
            }
        }
    }

    g_dir_close (d);

    m_pWidget->m_pSoundWidget->m_pSoundThemeCombobox->blockSignals(true);
    m_pWidget->m_pSoundWidget->m_pSoundThemeCombobox->addItem(tr("Custom"), "custom");
    m_pWidget->m_pSoundWidget->m_pSoundThemeCombobox->blockSignals(false);
}

/*
 *   加载下标的主题名
 */
char *UkmediaMainWidget::loadIndexThemeName (const char *index,char **parent)
{
    g_debug("load index theme name");
    GKeyFile *file;
    char *indexname = nullptr;
    gboolean hidden;

    file = g_key_file_new ();
    if (g_key_file_load_from_file (file, index, G_KEY_FILE_KEEP_TRANSLATIONS, nullptr) == FALSE) {
        g_key_file_free (file);
        return nullptr;
    }
    /* Don't add hidden themes to the list */
    hidden = g_key_file_get_boolean (file, "Sound Theme", "Hidden", nullptr);
    if (!hidden) {
        indexname = g_key_file_get_locale_string (file,"Sound Theme","Name",nullptr,nullptr);
        /* Save the parent theme, if there's one */
        if (parent != nullptr) {
            *parent = g_key_file_get_string (file,"Sound Theme","Inherits",nullptr);
        }
    }
    g_key_file_free (file);
    return indexname;
}

/*
 *   设置combox的主题名
 */
void UkmediaMainWidget::setComboxForThemeName (UkmediaMainWidget *m_pWidget,const char *name)
{
    g_debug("set combox for theme name");
    gboolean      found;
    int count = 0;
    /* If the name is empty, use "freedesktop" */
    if (name == nullptr || *name == '\0') {
        name = "freedesktop";
    }
    QString value;
    int index = -1;
    while(!found) {
        value = m_pWidget->m_pThemeNameList.at(count);
        found = (value != "" && value == name);
        count++;
        if( count >= m_pWidget->m_pThemeNameList.size() || found) {
            count = 0;
            break;
        }
    }
    if (m_pWidget->m_pThemeNameList.contains(name)) {
        index = m_pWidget->m_pThemeNameList.indexOf(name);
        value = m_pWidget->m_pThemeNameList.at(index);
        m_pWidget->m_pSoundWidget->m_pSoundThemeCombobox->setCurrentIndex(index);
    }
    /* When we can't find the theme we need to set, try to set the default
     * one "freedesktop" */
    if (found) {
    } else if (strcmp (name, "freedesktop") != 0) {//设置为默认的主题
        qDebug() << "设置为默认的主题" << "freedesktop";
        g_debug ("not found, falling back to fdo");
        setComboxForThemeName (m_pWidget, "freedesktop");
    }
}

/*
 *   更新报警音
 */
void UkmediaMainWidget::updateAlertsFromThemeName (UkmediaMainWidget *m_pWidget,const gchar *m_pName)
{
    g_debug("update alerts from theme name");
    if (strcmp (m_pName, CUSTOM_THEME_NAME) != 0) {
            /* reset alert to default */
        updateAlert (m_pWidget, DEFAULT_ALERT_ID);
    } else {
        int   sound_type;
        char *linkname;
        linkname = nullptr;
        sound_type = getFileType ("bell-terminal", &linkname);
        g_debug ("Found link: %s", linkname);
        if (sound_type == SOUND_TYPE_CUSTOM) {
            updateAlert (m_pWidget, linkname);
        }
    }
}

/*
    更新报警声音
*/
void UkmediaMainWidget::updateAlert (UkmediaMainWidget *pWidget,const char *alertId)
{
    Q_UNUSED(alertId)
    g_debug("update alert");
    QString themeStr;
    char *theme;
    char *parent;
    gboolean      is_custom;
    gboolean      is_default;
    gboolean add_custom = false;
    gboolean remove_custom = false;
    QString nameStr;
    int index = -1;
    /* Get the current theme's name, and set the parent */
    index = pWidget->m_pSoundWidget->m_pSoundThemeCombobox->currentIndex();
    if (index != -1 && pWidget->m_pThemeNameList.count() > 0) {
        themeStr = pWidget->m_pThemeNameList.at(index);
        nameStr = pWidget->m_pThemeNameList.at(index);
    }
    else {
        themeStr = "freedesktop";
        nameStr = "freedesktop";
    }
    QByteArray ba = themeStr.toLatin1();
    theme = ba.data();

    QByteArray baParent = nameStr.toLatin1();
    parent = baParent.data();

    is_custom = strcmp (theme, CUSTOM_THEME_NAME) == 0;
    is_default = strcmp (alertId, DEFAULT_ALERT_ID) == 0;

    if (! is_custom && is_default) {
        /* remove custom just in case */
        remove_custom = TRUE;
    } else if (! is_custom && ! is_default) {
        createCustomTheme (parent);
        saveAlertSounds(pWidget->m_pSoundWidget->m_pSoundThemeCombobox, alertId);
        add_custom = TRUE;
    } else if (is_custom && is_default) {
        saveAlertSounds(pWidget->m_pSoundWidget->m_pSoundThemeCombobox, alertId);
        /* after removing files check if it is empty */
        if (customThemeDirIsEmpty ()) {
            remove_custom = TRUE;
        }
    } else if (is_custom && ! is_default) {
        saveAlertSounds(pWidget->m_pSoundWidget->m_pSoundThemeCombobox, alertId);
    }

    if (add_custom) {
        setComboxForThemeName (pWidget, CUSTOM_THEME_NAME);
    } else if (remove_custom) {
        setComboxForThemeName (pWidget, parent);
    }
}

/*
    获取声音文件类型
*/
int UkmediaMainWidget::getFileType (const char *sound_name,char **linked_name)
{
    g_debug("get file type");
    char *name, *filename;
    *linked_name = nullptr;
    name = g_strdup_printf ("%s.disabled", sound_name);
    filename = customThemeDirPath (name);
    if (g_file_test (filename, G_FILE_TEST_IS_REGULAR) != FALSE) {
        return SOUND_TYPE_OFF;
    }
    /* We only check for .ogg files because those are the
     * only ones we create */
    name = g_strdup_printf ("%s.ogg", sound_name);
    filename = customThemeDirPath (name);
    g_free (name);

    if (g_file_test (filename, G_FILE_TEST_IS_SYMLINK) != FALSE) {
        *linked_name = g_file_read_link (filename, nullptr);
        g_free (filename);
        return SOUND_TYPE_CUSTOM;
    }
    g_free (filename);
    return SOUND_TYPE_BUILTIN;
}

/*
    自定义主题路径
*/
char *UkmediaMainWidget::customThemeDirPath (const char *child)
{
    g_debug("custom theme dir path");
    static char *dir = nullptr;
    const char *data_dir;

    if (dir == nullptr) {
        data_dir = g_get_user_data_dir ();
        dir = g_build_filename (data_dir, "sounds", CUSTOM_THEME_NAME, nullptr);
    }
    if (child == nullptr)
        return g_strdup (dir);

    return g_build_filename (dir, child, nullptr);
}

/*
    获取报警声音文件的路径
*/
#if 0
void UkmediaMainWidget::populateModelFromDir (UkmediaMainWidget *m_pWidget,const char *dirname)//从目录查找报警声音文件
{
    g_debug("populate model from dir");
    GDir *d;
    const char *name;
    char *path;
    d = g_dir_open (dirname, 0, nullptr);
    if (d == nullptr) {
        return;
    }
    while ((name = g_dir_read_name (d)) != nullptr) {
        if (! g_str_has_suffix (name, ".xml")) {
            continue;
        }
        QString themeName = name;
        QStringList temp = themeName.split("-");
        themeName = temp.at(0);
        if (!m_pWidget->m_soundThemeList.contains(themeName)) {
            m_pWidget->m_soundThemeList.append(themeName);
            m_pWidget->m_soundThemeDirList.append(dirname);
            m_pWidget->m_soundThemeXmlNameList.append(name);
        }
        path = g_build_filename (dirname, name, nullptr);
    }
    if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)){
        QString pThemeName;
        if (m_pWidget->m_pSoundSettings->keys().contains("themeName")) {
            pThemeName = m_pWidget->m_pSoundSettings->get(SOUND_THEME_KEY).toString();
        }

        int themeIndex;
        if(m_pWidget->m_soundThemeList.contains(pThemeName)) {
             themeIndex =  m_pWidget->m_soundThemeList.indexOf(pThemeName);
             if (themeIndex < 0 )
                 return;

        }
        else {
            themeIndex = 1;
        }
        themeIndex=0;
        QString dirName = m_pWidget->m_soundThemeDirList.at(themeIndex);
        QString xmlName = m_pWidget->m_soundThemeXmlNameList.at(themeIndex);
        path = g_build_filename (dirName.toLatin1().data(), xmlName.toLatin1().data(), nullptr);
        m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->blockSignals(true);
        m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->clear();
        m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->blockSignals(false);
    }

    populateModelFromFile (m_pWidget, path);
    //初始化声音主题

    g_free (path);
    g_dir_close (d);
}

/*
    获取报警声音文件
*/
void UkmediaMainWidget::populateModelFromFile (UkmediaMainWidget *m_pWidget,const char *filename)
{
    g_debug("populate model from file");
    xmlDocPtr  doc;
    xmlNodePtr root;
    xmlNodePtr child;
    gboolean   exists;
    qDebug() << "populateModelFromFile" <<filename;
    exists = g_file_test (filename, G_FILE_TEST_EXISTS);
    if (! exists) {
        return;
    }
    doc = xmlParseFile (filename);
    if (doc == nullptr) {
        return;
    }
    root = xmlDocGetRootElement (doc);
    for (child = root->children; child; child = child->next) {
        if (xmlNodeIsText (child)) {
                continue;
        }
        if (xmlStrcmp (child->name, GVC_SOUND_SOUND) != 0) {
                continue;
        }
        populateModelFromNode (m_pWidget, child);
    }
    xmlFreeDoc (doc);
}

/*
    从节点查找声音文件并加载到组合框中
*/
void UkmediaMainWidget::populateModelFromNode (UkmediaMainWidget *m_pWidget,xmlNodePtr node)
{
    g_debug("populate model from node");
    xmlNodePtr child;
    xmlChar   *filename;
    xmlChar   *name;

    filename = nullptr;
    name = xmlGetAndTrimNames (node);
    for (child = node->children; child; child = child->next) {
        if (xmlNodeIsText (child)) {
            continue;
        }

        if (xmlStrcmp (child->name, GVC_SOUND_FILENAME) == 0) {
            filename = xmlNodeGetContent (child);
        } else if (xmlStrcmp (child->name, GVC_SOUND_NAME) == 0) {
                /* EH? should have been trimmed */
        }
    }
    //将找到的声音文件名设置到combox中
    if (filename != nullptr && name != nullptr) {
        m_pWidget->m_pSoundList->append((const char *)filename);
        m_pWidget->m_pSoundNameList->append((const char *)name);
        m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->addItem((char *)name);
        m_pWidget->m_pSoundWidget->m_pLagoutCombobox->addItem((char *)name);
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->addItem((char *)name);
    }
    xmlFree (filename);
    xmlFree (name);
}
#endif


/*****************************************************************
* 函数名称： populateModelFromDir
* 功能描述： 从目录中查找系统音效文件
* 参数说明： dirname: 目录路径
******************************************************************/
void UkmediaMainWidget::populateModelFromDir (UkmediaMainWidget *m_pWidget,const char *dirname)
{
    GDir *dir = nullptr;
    const char *name;
    char *path;

    dir = g_dir_open(dirname, 0, nullptr);

    if (nullptr == dir) {
        qDebug() << "populateModelFromDir dir is null !";
        return;
    }

    while ((name = g_dir_read_name(dir)) != nullptr) {
        if (!g_str_has_suffix (name, ".xml"))
            continue;

        QString themeName = name;
        QStringList temp = themeName.split(".");
        themeName = temp.at(0);

        if (!m_pWidget->m_soundThemeList.contains(themeName)) {
            m_pWidget->m_soundThemeList.append(themeName);
            m_pWidget->m_soundThemeDirList.append(dirname);
            m_pWidget->m_soundThemeXmlNameList.append(name);
        }
        path = g_build_filename (dirname, name, nullptr);
    }

    //初始化声音主题
    populateModelFromFile (m_pWidget, path);

    g_free (path);
    g_dir_close (dir);
}


/*****************************************************************
* 函数名称： populateModelFromFile
* 功能描述： 根据对应主题的xml文件获取音效文件
* 参数说明： filename: 音效主题目录下对应主题xml文件路径
******************************************************************/
void UkmediaMainWidget::populateModelFromFile (UkmediaMainWidget *m_pWidget,const char *filename)
{
    xmlDocPtr  doc;
    xmlNodePtr root;
    xmlNodePtr child;

    if (! g_file_test(filename, G_FILE_TEST_EXISTS)) {
        qDebug() << "populateModelFromFile filename is not exist !";
        return;
    }

    doc = xmlParseFile(filename);  //载入xml文件
    if (nullptr == doc) {
        qDebug() << "populateModelFromFile xmlParseFile xml failed !";
        return;
    }

    root = xmlDocGetRootElement(doc);  //获得根节点
    for (child = root->children; child; child = child->next) {
        if (xmlNodeIsText(child)) {
            continue;
        }
        if (xmlStrcmp (child->name, GVC_SOUND_SOUND) != 0) {
                continue;
        }
        populateModelFromNode (m_pWidget, child);
    }

    xmlFreeDoc (doc);
}


/*****************************************************************
* 函数名称： populateModelFromNode
* 功能描述： 从节点查找声音文件并加载到组合框中
* 参数说明： node
******************************************************************/
void UkmediaMainWidget::populateModelFromNode (UkmediaMainWidget *m_pWidget,xmlNodePtr node)
{
    xmlNodePtr child;
    xmlChar *name = nullptr;
    xmlChar *filename = nullptr;

    name = xmlGetAndTrimNames (node);
    for (child = node->children; child; child = child->next) {
        if (xmlNodeIsText (child)) {
            continue;
        }

        if (xmlStrcmp (child->name, GVC_SOUND_FILENAME) == 0) {
            filename = xmlNodeGetContent (child);
        } else if (xmlStrcmp (child->name, GVC_SOUND_NAME) == 0) {
            /* EH? should have been trimmed */
        }
    }

    //  先添加无自定义音效选项
    if (m_pWidget->m_pSoundNameList.isEmpty() && m_pWidget->m_pSoundList.isEmpty()) {
        m_pWidget->m_pSoundList.append("");
        m_pWidget->m_pSoundNameList.append(tr("None"));
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(true);
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->addItem(tr("None"), "");
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(false);
        m_pWidget->m_pSoundWidget->m_pNotificationCombobox->blockSignals(true);
        m_pWidget->m_pSoundWidget->m_pNotificationCombobox->addItem(tr("None"), "");
        m_pWidget->m_pSoundWidget->m_pNotificationCombobox->blockSignals(false);
    }

    //  将找到的声音文件名设置到combox中
    if (filename != nullptr && name != nullptr) {
        m_pWidget->m_pSoundNameList.append((const char *)name);
        m_pWidget->m_pSoundList.append((const char *)filename);
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(true);
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->addItem((char *)name, (char *)filename);
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(false);
        m_pWidget->m_pSoundWidget->m_pNotificationCombobox->blockSignals(true);
        m_pWidget->m_pSoundWidget->m_pNotificationCombobox->addItem((char *)name, (char *)filename);
        m_pWidget->m_pSoundWidget->m_pNotificationCombobox->blockSignals(false);
    }

    xmlFree (filename);
    xmlFree (name);
}


/* Adapted from yelp-toc-pager.c */
xmlChar *UkmediaMainWidget::xmlGetAndTrimNames (xmlNodePtr node)
{
    g_debug("xml get and trim names");
    xmlNodePtr cur;
    xmlChar *keep_lang = nullptr;
    xmlChar *value;
    int j, keep_pri = INT_MAX;
    const gchar * const * langs = g_get_language_names ();

    value = nullptr;
    for (cur = node->children; cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, GVC_SOUND_NAME)) {
            xmlChar *cur_lang = nullptr;
            int cur_pri = INT_MAX;
            cur_lang = xmlNodeGetLang (cur);

            if (cur_lang) {
                for (j = 0; langs[j]; j++) {
                    if (g_str_equal (cur_lang, langs[j])) {
                        cur_pri = j;
                        break;
                    }
                }
            } else {
                cur_pri = INT_MAX - 1;
            }

            if (cur_pri <= keep_pri) {
                if (keep_lang)
                    xmlFree (keep_lang);
                if (value)
                    xmlFree (value);

                value = xmlNodeGetContent (cur);

                keep_lang = cur_lang;
                keep_pri = cur_pri;
            } else {
                if (cur_lang)
                    xmlFree (cur_lang);
            }
        }
    }

    /* Delete all GVC_SOUND_NAME nodes */
    cur = node->children;
    while (cur) {
            xmlNodePtr p_this = cur;
            cur = cur->next;
            if (! xmlStrcmp (p_this->name, GVC_SOUND_NAME)) {
                xmlUnlinkNode (p_this);
                xmlFreeNode (p_this);
            }
    }
    return value;
}


/*
    点击combox播放声音
*/
#if 0
void UkmediaMainWidget::comboxIndexChangedSlot(int index)
{
    g_debug("combox index changed slot");
    QString sound_name = m_pSoundList->at(index);
    updateAlert(this,sound_name.toLatin1().data());
    playAlretSoundFromPath(this,sound_name);

    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
//            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "alert-sound") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }

}

/*
    设置窗口关闭的提示音
*/
void UkmediaMainWidget::windowClosedComboboxChangedSlot(int index)
{
    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
//            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "window-close") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }
}

/*
    设置音量改变的提示声音
*/
void UkmediaMainWidget::volumeChangedComboboxChangeSlot(int index)
{
    QString sound_name = m_pSoundList->at(index);
//    updateAlert(this,sound_name.toLatin1().data());
    playAlretSoundFromPath(this,sound_name);

    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();
    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);
        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
//            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "volume-changed") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }
}

void UkmediaMainWidget::settingMenuComboboxChangedSlot(int index)
{
    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();
    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);
        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "system-setting") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }
}
#endif

/*
    点击输入音量按钮静音
*/
void UkmediaMainWidget::inputMuteButtonSlot()
{
     m_pVolumeControl->setSourceMute(!m_pVolumeControl->sourceMuted);
     inputVolumeDarkThemeImage(paVolumeToValue(m_pVolumeControl->sourceVolume),!m_pVolumeControl->sourceMuted);
     m_pInputWidget->m_pInputIconBtn->setObjectName("m_pInputWidget->m_pInputIconBtn");
     ukcc::UkccCommon::buriedSettings("Audio",m_pInputWidget->m_pInputIconBtn->objectName(), QString("settings"),\
                                      !m_pVolumeControl->sourceMuted?"true":"false");
}

/*
    点击输出音量按钮静音
*/
void UkmediaMainWidget::outputMuteButtonSlot()
{
     m_pVolumeControl->setSinkMute(!m_pVolumeControl->sinkMuted);
     outputVolumeDarkThemeImage(paVolumeToValue(m_pVolumeControl->sinkVolume),!m_pVolumeControl->sinkMuted);
     m_pOutputWidget->m_pOutputIconBtn->setObjectName("m_pOutputWidget->m_pOutputIconBtn");
     ukcc::UkccCommon::buriedSettings("Audio",m_pOutputWidget->m_pOutputIconBtn->objectName(), QString("settings"),\
                                      !m_pVolumeControl->sinkMuted?"true":"false");
}


/*
    点击声音主题实现主题切换
*/
void UkmediaMainWidget::themeComboxIndexChangedSlot(int index)
{
    if (-1 == index || nullptr == m_pGlobalThemeSetting)
        return;

    QString theme = m_pSoundWidget->m_pSoundThemeCombobox->currentData().toString();
    QString globalTheme = m_pGlobalThemeSetting->get(GLOBAL_THEME_NAME).toString();

    //  切换主题时，将os全局主题切换成自定义
    if (theme != globalTheme)
        m_pGlobalThemeSetting->set(GLOBAL_THEME_NAME, "custom");

    //  切换至自定义音效时，custom-theme开启
    if (theme == "custom") {
        m_pSoundSettings->blockSignals(true);
        m_pSoundSettings->set(SOUND_CUSTOM_THEME_KEY, true);
        m_pSoundSettings->blockSignals(true);
        return;
    }

    //  设置系统主题，并重置自定义音效为默认音效
    m_pSoundSettings->blockSignals(true);
    m_pSoundSettings->set(SOUND_THEME_KEY, theme);
    m_pSoundSettings->set(SOUND_CUSTOM_THEME_KEY, false);
    m_pSoundSettings->blockSignals(false);

    if (!resetCustomSoundEffects(theme, AUDIO_VOLUME_CHANGE)) {
        m_pSoundSettings->set(AUDIO_VOLUME_CHANGE, "");
        m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(true);
        m_pSoundWidget->m_pVolumeChangeCombobox->setCurrentIndex(0);
        m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(false);
        qWarning("%s resetCustomSoundEffects failed !", AUDIO_VOLUME_CHANGE);
    }

    if (!resetCustomSoundEffects(theme, NOTIFICATION_GENGERAL)) {
        m_pSoundSettings->set(NOTIFICATION_GENGERAL, "");
        m_pSoundWidget->m_pNotificationCombobox->blockSignals(true);
        m_pSoundWidget->m_pNotificationCombobox->setCurrentIndex(0);
        m_pSoundWidget->m_pNotificationCombobox->blockSignals(false);
        qWarning("%s resetCustomSoundEffects failed !", NOTIFICATION_GENGERAL);
    }

    m_pOutputWidget->m_pDeviceSelectBox->setObjectName("m_pSoundWidget->m_pSoundThemeCombobox");
    ukcc::UkccCommon::buriedSettings("Audio", m_pSoundWidget->m_pSoundThemeCombobox->objectName(),"select",m_pThemeNameList.at(index));
}

/*
    滚动输出音量滑动条
*/
void UkmediaMainWidget::outputWidgetSliderChangedSlot(int value)
{
    QString percent;
    percent = QString::number(value);
    outputVolumeDarkThemeImage(value,false);
    m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent+"%");

    //蓝牙a2dp模式下滑动条跳动问题，以间隔为10设置音量
    if (m_pVolumeControl->defaultSinkName.contains("a2dp_sink"))
        return;

    int volume = valueToPaVolume(value);
    m_pVolumeControl->getDefaultSinkIndex();
    m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,volume);
    qDebug() << "outputWidgetSliderChangedSlot" << value <<volume;

    m_pOutputWidget->m_pOpVolumeSlider->setObjectName("m_pOutputWidget->m_pOpVolumeSlider");
    ukcc::UkccCommon::buriedSettings("Audio", m_pOutputWidget->m_pOpVolumeSlider->objectName(),"slider",QString::number(value));
}

void UkmediaMainWidget::outputWidgetSliderChangedSlotInBlue(int value)
{
    if (!m_pVolumeControl->defaultSinkName.contains("a2dp_sink")) {
        m_pOutputWidget->m_pOpVolumeSlider->isMouseWheel = false;
        return;
    }

    qDebug() << "Special Handling Adjust volume in Bluetooth a2dp mode" << value ;

    int volume = valueToPaVolume(value);
    m_pVolumeControl->getDefaultSinkIndex();
    m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,volume);

    QString percent;
    percent = QString::number(value);
    outputVolumeDarkThemeImage(value,false);
    m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent+"%");

    m_pOutputWidget->m_pOpVolumeSlider->setObjectName("m_pOutputWidget->m_pOpVolumeSlider");
    ukcc::UkccCommon::buriedSettings("Audio", m_pOutputWidget->m_pOpVolumeSlider->objectName(),"slider",QString::number(value));
}

void UkmediaMainWidget::timeSliderSlot()
{
    if(mouseReleaseState){
        int value = m_pOutputWidget->m_pOpVolumeSlider->value();

        QString percent;
        bool status = false;
        percent = QString::number(value);
        int volume = value*65536/100;
        if (value <= 0) {
            status = true;

            percent = QString::number(0);
        }
        else {
            if (firstEnterSystem) {

            }
            else {

            }
        }
        firstEnterSystem = false;
        outputVolumeDarkThemeImage(value,status);
        percent.append("%");
        m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent);

        mouseReleaseState = false;
        mousePress = false;
        timeSlider->stop();
    }
    else{
        timeSlider->start(50);
    }
}

/*
    滚动输入滑动条
*/
void UkmediaMainWidget::inputWidgetSliderChangedSlot(int value)
{
    int volume = valueToPaVolume(value);
    m_pVolumeControl->getDefaultSinkIndex();
    m_pVolumeControl->setSourceVolume(m_pVolumeControl->sourceIndex,volume);
    qDebug() << "inputWidgetSliderChangedSlot" << value <<volume;

    //输入图标修改成深色主题
    inputVolumeDarkThemeImage(value,m_pVolumeControl->sourceMuted);
    m_pInputWidget->m_pInputIconBtn->repaint();
    QString percent = QString::number(value);
    percent.append("%");
    m_pInputWidget->m_pIpVolumePercentLabel->setText(percent);

    m_pInputWidget->m_pIpVolumeSlider->setObjectName("m_pInputWidget->m_pIpVolumeSlider");
    ukcc::UkccCommon::buriedSettings("Audio", m_pInputWidget->m_pIpVolumeSlider->objectName(),"slider",QString::number(value));
}

/*
 *  开启单声道
 */
void UkmediaMainWidget::balanceSliderChangedMono(int balanceSliderValue, int sinkVolume,int sinkIndex)
{
    gdouble balanceVolume = balanceSliderValue/100.0;
//    int value = valueToPaVolume(m_pOutputWidget->m_pOpVolumeSlider->value());
    m_pVolumeControl->setBalanceVolume(sinkIndex, sinkVolume, balanceVolume);
    m_pOutputWidget->m_pOpBalanceSlider->setObjectName("m_pOutputWidget->m_pOpBalanceSlider");
    ukcc::UkccCommon::buriedSettings("Audio", m_pOutputWidget->m_pOpBalanceSlider->objectName(),"slider",QString::number(balanceVolume));
}

/*
 *  平衡值改变
 */
void UkmediaMainWidget::balanceSliderChangedSlot(int balanceSliderValue)
{
    gdouble volume = balanceSliderValue/100.0;
    int value = valueToPaVolume(m_pOutputWidget->m_pOpVolumeSlider->value());
    m_pVolumeControl->setBalanceVolume(m_pVolumeControl->sinkIndex, value, volume);
    m_pOutputWidget->m_pOpBalanceSlider->setObjectName("m_pOutputWidget->m_pOpBalanceSlider");
    ukcc::UkccCommon::buriedSettings("Audio", m_pOutputWidget->m_pOpBalanceSlider->objectName(),"slider",QString::number(volume));
}

/*
 * 输入等级
 */
void UkmediaMainWidget::peakVolumeChangedSlot(double v)
{
    if (v >= 0) {
        m_pInputWidget->m_pInputLevelProgressBar->setEnabled(true);
        //因为有些电脑MIC增益太大会引起反馈条消失先暂时固定最大值为101
        //int value = qRound(v * m_pInputWidget->m_pInputLevelProgressBar->maximum());
        int value = qRound(v * 100);
        m_pInputWidget->m_pInputLevelProgressBar->setValue(value);
    } else {
        m_pInputWidget->m_pInputLevelProgressBar->setEnabled(false);
        m_pInputWidget->m_pInputLevelProgressBar->setValue(0);
    }
}

gboolean UkmediaMainWidget::saveAlertSounds (QComboBox *combox,const char *id)
{
    const char *sounds[3] = { "bell-terminal", "bell-window-system", NULL };
    char *path;

    if (strcmp (id, DEFAULT_ALERT_ID) == 0) {
        deleteOldFiles (sounds);
        deleteDisabledFiles (sounds);
    } else {
        deleteOldFiles (sounds);
        deleteDisabledFiles (sounds);
        addCustomFile (sounds, id);
    }

    /* And poke the directory so the theme gets updated */
    path = customThemeDirPath(NULL);
    if (utime (path, NULL) != 0) {
        g_warning ("Failed to update mtime for directory '%s': %s",
                   path, g_strerror (errno));
    }
    g_free (path);

    return FALSE;
}

void UkmediaMainWidget::deleteOldFiles (const char **sounds)
{
    guint i;
    for (i = 0; sounds[i] != NULL; i++) {
        deleteOneFile (sounds[i], "%s.ogg");
    }
}

void UkmediaMainWidget::deleteOneFile (const char *sound_name, const char *pattern)
{
        GFile *file;
        char *name, *filename;

        name = g_strdup_printf (pattern, sound_name);
        filename = customThemeDirPath(name);
        g_free (name);
        file = g_file_new_for_path (filename);
        g_free (filename);
        cappletFileDeleteRecursive (file, NULL);
        g_object_unref (file);
}

void UkmediaMainWidget::deleteDisabledFiles (const char **sounds)
{
    guint i;
    for (i = 0; sounds[i] != NULL; i++) {
        deleteOneFile (sounds[i], "%s.disabled");
    }
}

void UkmediaMainWidget::addCustomFile (const char **sounds, const char *filename)
{
    guint i;

    for (i = 0; sounds[i] != NULL; i++) {
        GFile *file;
        char *name, *path;

        /* We use *.ogg because it's the first type of file that
                 * libcanberra looks at */
        name = g_strdup_printf ("%s.ogg", sounds[i]);
        path = customThemeDirPath(name);
        g_free (name);
        /* In case there's already a link there, delete it */
        g_unlink (path);
        file = g_file_new_for_path (path);
        g_free (path);

        /* Create the link */
        g_file_make_symbolic_link (file, filename, NULL, NULL);
        g_object_unref (file);
    }
}

/**
 * A utility routine to delete files and/or directories,
 * including non-empty directories.
 **/
gboolean UkmediaMainWidget::cappletFileDeleteRecursive (GFile *file, GError **error)
{
    GFileInfo *info;
    GFileType type;

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    info = g_file_query_info (file,
                              G_FILE_ATTRIBUTE_STANDARD_TYPE,
                              G_FILE_QUERY_INFO_NONE,
                              NULL, error);
    if (info == NULL) {
        return FALSE;
    }

    type = g_file_info_get_file_type (info);
    g_object_unref (info);

    if (type == G_FILE_TYPE_DIRECTORY) {
        return directoryDeleteRecursive (file, error);
    }
    else {
        return g_file_delete (file, NULL, error);
    }
}

gboolean UkmediaMainWidget::directoryDeleteRecursive (GFile *directory, GError **error)
{
    GFileEnumerator *enumerator;
    GFileInfo *info;
    gboolean success = TRUE;

    enumerator = g_file_enumerate_children (directory,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, error);
    if (enumerator == NULL)
        return FALSE;

    while (success &&
           (info = g_file_enumerator_next_file (enumerator, NULL, NULL))) {
        GFile *child;

        child = g_file_get_child (directory, g_file_info_get_name (info));

        if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY) {
            success = directoryDeleteRecursive (child, error);
        }
        g_object_unref (info);

        if (success)
            success = g_file_delete (child, NULL, error);
    }
    g_file_enumerator_close (enumerator, NULL, NULL);

    if (success)
        success = g_file_delete (directory, NULL, error);

    return success;
}

void UkmediaMainWidget::createCustomTheme (const char *parent)
{
    GKeyFile *keyfile;
    char     *data;
    char     *path;

    /* Create the custom directory */
    path = customThemeDirPath(NULL);
    g_mkdir_with_parents (path, 0755);
    g_free (path);

    /* Set the data for index.theme */
    keyfile = g_key_file_new ();
    g_key_file_set_string (keyfile, "Sound Theme", "Name", _("Custom"));
    g_key_file_set_string (keyfile, "Sound Theme", "Inherits", parent);
    g_key_file_set_string (keyfile, "Sound Theme", "Directories", ".");
    data = g_key_file_to_data (keyfile, NULL, NULL);
    g_key_file_free (keyfile);

    /* Save the index.theme */
    path = customThemeDirPath ("index.theme");
    g_file_set_contents (path, data, -1, NULL);
    g_free (path);
    g_free (data);

    customThemeUpdateTime ();
}

/* This function needs to be called after each individual
 * changeset to the theme */
void UkmediaMainWidget::customThemeUpdateTime (void)
{
    char *path;
    path = customThemeDirPath (NULL);
    utime (path, NULL);
    g_free (path);
}

gboolean UkmediaMainWidget::customThemeDirIsEmpty (void)
{
    char            *dir;
    GFile           *file;
    gboolean         is_empty;
    GFileEnumerator *enumerator;
    GFileInfo       *info;
    GError          *error = NULL;

    dir = customThemeDirPath(NULL);
    file = g_file_new_for_path (dir);
    g_free (dir);

    is_empty = TRUE;

    enumerator = g_file_enumerate_children (file,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, &error);
    if (enumerator == NULL) {
        g_warning ("Unable to enumerate files: %s", error->message);
        g_error_free (error);
        goto out;
    }

    while (is_empty &&
           (info = g_file_enumerator_next_file (enumerator, NULL, NULL))) {

        if (strcmp ("index.theme", g_file_info_get_name (info)) != 0) {
            is_empty = FALSE;
        }

        g_object_unref (info);
    }
    g_file_enumerator_close (enumerator, NULL, NULL);

out:
    g_object_unref (file);

    return is_empty;
}

int UkmediaMainWidget::caPlayForWidget(UkmediaMainWidget *w, uint32_t id, ...)
{
    va_list ap;
    int ret;
    ca_proplist *p;

    if ((ret = ca_proplist_create(&p)) < 0)
    {
        qDebug() << "ca_proplist_create ret:" << ret;
        return ret;
    }

    if ((ret = caProplistSetForWidget(p, w)) < 0)
    {
        qDebug() << "caProplistSetForWidget ret:" << ret;
        return -1;
    }

    va_start(ap, id);
    ret = caProplistMergeAp(p, ap);
    va_end(ap);
    if (ret < 0)
    {
        qDebug() << "caProplistMergeAp ret:" << ret;
        return -1;
    }

    if (nullptr != m_caContext)
    {
        ca_context_cancel(m_caContext, 0);
        ca_context_destroy(m_caContext);
    }
    ca_context_create(&m_caContext);
    ret = ca_context_play_full(m_caContext, id, p, NULL, NULL);
    qDebug() << "ca_context_play_full ret:" << ret;

    return ret;
}

int UkmediaMainWidget::caProplistMergeAp(ca_proplist *p, va_list ap)
{
    int ret;
    for (;;) {
        const char *key, *value;

        if (!(key = va_arg(ap, const char*)))
            break;

        if (!(value = va_arg(ap, const char*)))
            return CA_ERROR_INVALID;

        if ((ret = ca_proplist_sets(p, key, value)) < 0)
            return ret;
    }

    return CA_SUCCESS;
}

int UkmediaMainWidget::caProplistSetForWidget(ca_proplist *p, UkmediaMainWidget *widget)
{
    int ret;
    const char *t;
    QScreen *screen;
    gint x = -1;
    gint y = -1;
    gint width = -1;
    gint height = -1;
    gint screen_width = -1;
    gint screen_height = -1;

    if ((t = widget->windowTitle().toLatin1().data()))
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_NAME, t)) < 0)
            return ret;

    if (t)
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_ID, t)) < 0)
            return ret;

    if ((t = widget->windowIconText().toLatin1().data()))
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_ICON_NAME, t)) < 0)
            return ret;
    if (screen = qApp->primaryScreen()) {
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_X11_SCREEN, "%i", 0)) < 0)
            return ret;
    }

    width = widget->size().width();
    height = widget->size().height();

    if (width > 0)
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_WIDTH, "%i", width)) < 0)
            return ret;
    if (height > 0)
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_HEIGHT, "%i", height)) < 0)
            return ret;

    if (x >= 0 && width > 0) {
        screen_width = qApp->primaryScreen()->size().width();

        x += width/2;
        x = CA_CLAMP(x, 0, screen_width-1);

        /* We use these strange format strings here to avoid that libc
                         * applies locale information on the formatting of floating
                         * numbers. */

        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_HPOS, "%i.%03i",
                                    (int) (x/(screen_width-1)), (int) (1000.0*x/(screen_width-1)) % 1000)) < 0)
            return ret;
    }

    if (y >= 0 && height > 0) {
        screen_height = qApp->primaryScreen()->size().height();

        y += height/2;
        y = CA_CLAMP(y, 0, screen_height-1);

        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_VPOS, "%i.%03i",
                                    (int) (y/(screen_height-1)), (int) (1000.0*y/(screen_height-1)) % 1000)) < 0)
            return ret;
    }
    return CA_SUCCESS;
}

UkmediaMainWidget::~UkmediaMainWidget()
{

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
                qDebug() <<"findOutputPortLabel" <<portName <<tempMap.key() <<tempMap.value();
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
 * 输入设备中是否包含蓝牙设备
 */
bool UkmediaMainWidget::inputComboboxDeviceContainBluetooth()
{
    for (int row=0;row<m_pInputWidget->m_pInputDeviceSelectBox->count();row++) {

        QString inputComboboxCardName = m_pInputWidget->m_pInputDeviceSelectBox->itemData(row).toString();
        if (inputComboboxCardName.contains("bluez")) {
            return true;
        }
    }
    return false;
}

QString UkmediaMainWidget::blueCardNameInCombobox()
{
    for (int row=0;row<m_pInputWidget->m_pInputDeviceSelectBox->count();row++) {

        QString inputComboboxCardName = m_pInputWidget->m_pInputDeviceSelectBox->itemData(row).toString();
        if (inputComboboxCardName.contains("bluez")) {
            return inputComboboxCardName;
        }
    }
    return "";
}

/*
    记录输入stream的card name
*/
void UkmediaMainWidget::inputStreamMapCardName (QString streamName,QString cardName)
{
    if (inputCardStreamMap.count() == 0) {
        inputCardStreamMap.insertMulti(streamName,cardName);
    }
    QMap<QString, QString>::iterator it;

    for (it=inputCardStreamMap.begin();it!=inputCardStreamMap.end();) {
        if (it.value() == cardName) {
            break;
        }
        if (it == inputCardStreamMap.end()-1) {
            qDebug() << "inputCardSreamMap " << streamName << cardName;
            inputCardStreamMap.insertMulti(streamName,cardName);
        }
        ++it;
    }
}

/*
    记录输出stream的card name
*/
void UkmediaMainWidget::outputStreamMapCardName(QString streamName, QString cardName)
{
    if (outputCardStreamMap.count() == 0) {
        outputCardStreamMap.insertMulti(streamName,cardName);
    }
    QMap<QString, QString>::iterator it;

    for (it=outputCardStreamMap.begin();it!=outputCardStreamMap.end();) {
        if (it.value() == cardName) {
            break;
        }
        if (it == outputCardStreamMap.end()-1) {
            qDebug() << "outputCardStreamMap " << streamName << cardName;
            outputCardStreamMap.insertMulti(streamName,cardName);
        }
        ++it;
    }
}

/*
    找输入stream对应的card name
*/
QString UkmediaMainWidget::findInputStreamCardName(QString streamName)
{
    QString cardName;
    QMap<QString, QString>::iterator it;
    for (it=inputCardStreamMap.begin();it!=inputCardStreamMap.end();) {
        if (it.key() == streamName) {
            cardName = it.value();
            qDebug() << "findInputStreamCardName:" << cardName;
            break;
        }
        ++it;
    }
    return cardName;
}

/*
    找输出stream对应的card name
*/
QString UkmediaMainWidget::findOutputStreamCardName(QString streamName)
{
    QString cardName;
    QMap<QString, QString>::iterator it;
    for (it=outputCardStreamMap.begin();it!=outputCardStreamMap.end();) {
        if (it.key() == streamName) {
            cardName = it.value();
            break;
        }
        ++it;
    }
    return cardName;
}

/*
 *  设置声卡的配置文件
 */
void UkmediaMainWidget::setCardProfile(QString name, QString profile)
{
    int index = findCardIndex(name,m_pVolumeControl->cardMap);
    m_pVolumeControl->setCardProfile(index,profile.toLatin1().data());
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
void UkmediaMainWidget::setDefaultInputPortDevice(QString devName, QString portName)
{
    int cardIndex = findCardIndex(devName,m_pVolumeControl->cardMap);
    QString portStr = findInputPortName(cardIndex,portName);

    QTimer *timer = new QTimer;
    timer->start(100);
    connect(timer,&QTimer::timeout,[=](){
        QString sourceStr = findPortSource(cardIndex,portStr);

        /*默认的stream 和设置的stream相同 需要更新端口*/
        if (strcmp(sourceStr.toLatin1().data(),m_pVolumeControl->defaultSourceName) == 0) {
            m_pVolumeControl->setSourcePort(sourceStr.toLatin1().data(),portStr.toLatin1().data());
        }
        else {
            m_pVolumeControl->setDefaultSource(sourceStr.toLatin1().data());
            m_pVolumeControl->setSourcePort(sourceStr.toLatin1().data(),portStr.toLatin1().data());
        }
        qDebug() << "set default input"  << portName << cardIndex << portStr << devName <<sourceStr;
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

/*
 * Find the corresponding sink according to the port name
 */
QString UkmediaMainWidget::findPortSink(int cardIndex,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    QString sinkStr = "";
    for (it=m_pVolumeControl->sinkPortMap.begin();it!=m_pVolumeControl->sinkPortMap.end();) {
        if (it.key() == cardIndex) {
            portNameMap = it.value();
            for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
                if (tempMap.value() == portName) {
                    sinkStr = tempMap.key();
                    qDebug() <<"find port sink" << it.value() << portName<< it.key() <<tempMap.value() << sinkStr;
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return sinkStr;

}

/*
 * Find the corresponding source according to the port name
 */
QString UkmediaMainWidget::findPortSource(int cardIndex,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    QString sourceStr = "";
    for (it=m_pVolumeControl->sourcePortMap.begin();it!=m_pVolumeControl->sourcePortMap.end();) {
        if (it.key() == cardIndex) {
            portNameMap = it.value();
            for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
                if (tempMap.value() == portName) {
                    sourceStr = tempMap.key();
                    qDebug() << "find port source" <<tempMap.value() << portName << sourceStr;
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return sourceStr;
}

/*
 *  初始化输出combobox选项框
 */
void UkmediaMainWidget::initOutputComboboxItem()
{
    if (m_pVolumeControl->defaultOutputCard == -1 && !m_pOutputWidget->m_pDeviceSelectBox->count()) {
        addNoneItem(SoundType::SINK);
    }

    QString outputCardName = findCardName(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->cardMap);
    QString outputPortLabel = findOutputPortLabel(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->sinkPortName);
    findOutputComboboxItem(outputCardName,outputPortLabel);

    int vol = m_pVolumeControl->getSinkVolume();
    float balanceVolume = m_pVolumeControl->getBalanceVolume();
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(true);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(true);
    m_pOutputWidget->m_pOpVolumeSlider->setValue(paVolumeToValue(vol));
    handleBalanceSlider(balanceVolume);

    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(false);
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(false);
    m_pOutputWidget->m_pOpVolumePercentLabel->setText(QString::number(paVolumeToValue(vol))+"%");

    qDebug() <<"initComboboxItem(Output)" << m_pVolumeControl->defaultOutputCard << outputCardName
            <<m_pVolumeControl->sinkPortName << outputPortLabel;
}

void UkmediaMainWidget::initInputComboboxItem()
{
    if (m_pVolumeControl->defaultInputCard == -1 && !m_pInputWidget->m_pInputDeviceSelectBox->count()) {
        addNoneItem(SoundType::SOURCE);
    }

    QString inputCardName = findCardName(m_pVolumeControl->defaultInputCard,m_pVolumeControl->cardMap);
    QString inputPortLabel = findInputPortLabel(m_pVolumeControl->defaultInputCard,m_pVolumeControl->sourcePortName);
    findInputComboboxItem(inputCardName,inputPortLabel);

    //当所有可用的输入设备全部移除，台式机才会出现该情况
    if (strstr(m_pVolumeControl->defaultSourceName,"monitor")) {
        m_pInputWidget->m_pInputLevelProgressBar->setValue(0);
    }

    qDebug() <<"initComboboxItem(Input)" << m_pVolumeControl->defaultInputCard << inputCardName
            <<m_pVolumeControl->sourcePortName << inputPortLabel;
}

void UkmediaMainWidget::findOutputComboboxItem(QString cardName,QString portLabel)
{
    for (int row=0;row<m_pOutputWidget->m_pDeviceSelectBox->count();row++) {
        QString comboboxcardname = m_pOutputWidget->m_pDeviceSelectBox->itemData(row).toString();
        QString comboboxportname = m_pOutputWidget->m_pDeviceSelectBox->itemText(row);
        if (comboboxcardname == cardName && comboboxportname == portLabel) {
            m_pOutputWidget->m_pDeviceSelectBox->blockSignals(true);
            m_pOutputWidget->m_pDeviceSelectBox->setCurrentIndex(row);
            m_pOutputWidget->m_pDeviceSelectBox->blockSignals(false);

            break;
        }
    }

    m_boxoutputRow = m_pOutputWidget->m_pDeviceSelectBox->currentIndex();
}

void UkmediaMainWidget::addNoneItem(int soundType)
{
    int index = -1;

    switch (soundType) {
    case SoundType::SINK:
        //需求31268待2501再合入
#if 1
        m_pOutputWidget->m_pOpVolumeSlider->setEnabled(false);
        m_pOutputWidget->m_pOpBalanceSlider->setDisabled(true);
#endif
        index = m_pOutputWidget->m_pDeviceSelectBox->findText(tr("None"));
        if (index != -1)
            return;

        m_pOutputWidget->m_pDeviceSelectBox->blockSignals(true);
        m_pOutputWidget->m_pDeviceSelectBox->addItem(tr("None"));
        m_pOutputWidget->m_pDeviceSelectBox->blockSignals(false);
        break;

    case SoundType::SOURCE:
        //需求31268待2501再合入
#if 1
        m_pInputWidget->m_pIpVolumeSlider->setEnabled(false);
#endif
        index = m_pInputWidget->m_pInputDeviceSelectBox->findText(tr("None"));
        if (index != -1)
            return;

        m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(true);
        m_pInputWidget->m_pInputDeviceSelectBox->addItem(tr("None"));
        m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(false);
        break;

    default:
        return;
    }
}

void UkmediaMainWidget::removeNoneItem(int soundType)
{
    int index = -1;

    switch (soundType) {
    case SoundType::SINK:
        //需求31268待2501再合入
#if 1
        m_pOutputWidget->m_pOpVolumeSlider->setEnabled(true);
        m_pOutputWidget->m_pOpBalanceSlider->setDisabled(false);
#endif
        index = m_pOutputWidget->m_pDeviceSelectBox->findText(tr("None"));
        m_pOutputWidget->m_pDeviceSelectBox->blockSignals(true);
        m_pOutputWidget->m_pDeviceSelectBox->removeItem(index);
        m_pOutputWidget->m_pDeviceSelectBox->blockSignals(false);
        break;

    case SoundType::SOURCE:
        //需求31268待2501再合入
#if 1
        m_pInputWidget->m_pIpVolumeSlider->setEnabled(true);
#endif
        index = m_pInputWidget->m_pInputDeviceSelectBox->findText(tr("None"));
        m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(true);
        m_pInputWidget->m_pInputDeviceSelectBox->removeItem(index);
        m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(false);
        break;

    default:
        return;
    }
}


/*
 * 当前的输出端口是否应该添加到Combobox output list widget上
 */
bool UkmediaMainWidget::comboboxOutputPortIsNeedAdd(int index, QString name)
{
    QMap<int,QString>::iterator it;
    for(it=currentCboxOutputPortLabelMap.begin();it!=currentCboxOutputPortLabelMap.end();) {
        if (index == it.key() && name == it.value()) {
            return false;
        }
        ++it;
    }
    return true;
}


/*
 * 当前的输出端口是否应该在output list widget上删除
 */
bool UkmediaMainWidget::comboboxOutputPortIsNeedDelete(int index, QString name)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
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
    if(name == tr("None")){
        return false;
    }
    return true;
}


/*
 * 添加可用的输出端口到ComBoBox output list widget
 */

void UkmediaMainWidget::addComboboxAvailableOutputPort()
{
    QMap<int,QMap<QString,QString>>::iterator at;
    QMap<QString,QString>::iterator it;
    QMap<QString,QString> tempMap;

    for (at = m_pVolumeControl->outputPortMap.begin(); at != m_pVolumeControl->outputPortMap.end();) {
        tempMap = at.value();
        for (it = tempMap.begin(); it != tempMap.end();) {
            if (comboboxOutputPortIsNeedAdd(at.key(), it.value())) {
                qDebug() << "add output combox widget" << at.key()<< it.value() << it.key();
                addComboboxOutputListWidgetItem(it.value(), findCardName(at.key(),m_pVolumeControl->cardMap));
                currentCboxOutputPortLabelMap.insertMulti(at.key(), it.value());
            }
            ++it;
        }
        ++at;
    }
}


/*
 * 添加output port到Combobox output list widget
 */
void UkmediaMainWidget::addComboboxOutputListWidgetItem(QString portLabel, QString cardName)
{
    int i = m_pOutputWidget->m_pDeviceSelectBox->count();
    m_pOutputWidget->m_pDeviceSelectBox->blockSignals(true);
    m_pOutputWidget->m_pDeviceSelectBox->insertItem(i,portLabel,cardName);
    m_pOutputWidget->m_pDeviceSelectBox->blockSignals(false);
    qDebug() << "添加输出设备端口" << portLabel << cardName;
}

//传进来一个portName用来定位他的位置
int UkmediaMainWidget::indexOfOutputPortInOutputCombobox(QString portName)
{
    for (int row=0;row<m_pOutputWidget->m_pDeviceSelectBox->count();row++)
    {
        QString text = m_pOutputWidget->m_pDeviceSelectBox->itemText(row);
        if (text == portName)
            return row;
    }
    return -1;
}


/*
 * 移除combobox output list widget上不可用的输出端口
 */
void UkmediaMainWidget::deleteNotAvailableComboboxOutputPort()
{
    //删除不可用的输出端口
    QMap<int,QString>::iterator it;

    for(it=currentCboxOutputPortLabelMap.begin();it!=currentCboxOutputPortLabelMap.end();) {
        //没找到，需要删除
        if (comboboxOutputPortIsNeedDelete(it.key(),it.value())) {
            int index = indexOfOutputPortInOutputCombobox(it.value());
            if (index == -1)
                return;

            m_pOutputWidget->m_pDeviceSelectBox->blockSignals(true);
            m_pOutputWidget->m_pDeviceSelectBox->removeItem(index);
            m_pOutputWidget->m_pDeviceSelectBox->hidePopup();
            m_pOutputWidget->m_pDeviceSelectBox->blockSignals(false);
            qDebug()<< "deleteNotAvailableComboboxOutputPort" <<index;
            it = currentCboxOutputPortLabelMap.erase(it);
            continue;
        }
        ++it;
    }
}


//==================================输入端============================================
void UkmediaMainWidget::findInputComboboxItem(QString cardName,QString portLabel)
{
    for (int row=0;row<m_pInputWidget->m_pInputDeviceSelectBox->count();row++) {
        QString comboboxcardname = m_pInputWidget->m_pInputDeviceSelectBox->itemData(row).toString();
        QString comboboxportname = m_pInputWidget->m_pInputDeviceSelectBox->itemText(row);
        if (comboboxcardname == cardName && comboboxportname == portLabel) {
            m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(true);
            m_pInputWidget->m_pInputDeviceSelectBox->setCurrentIndex(row);
            m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(false);

            if(!strstr(m_pVolumeControl->sourcePortName.toUtf8().constData(), "internal") \
              && !strstr(m_pVolumeControl->sourcePortName.toUtf8().constData(), "[In] Mic1"))
            {
                Q_EMIT m_pVolumeControl->updateLoopBack(true);
                qDebug() << "findInputComboboxItem -> Q_EMIT updateLoopBack true" << m_pVolumeControl->sourcePortName;
            }else{
                Q_EMIT m_pVolumeControl->updateLoopBack(false);
                qDebug() << "findInputComboboxItem -> Q_EMIT updateLoopBack false" << m_pVolumeControl->sourcePortName;
            }

            if (comboboxcardname.contains("bluez_card")) {
                isCheckBluetoothInput = true;
            }

            break;
        }
        else if (comboboxportname == tr("None")) {
            m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(true);
            m_pInputWidget->m_pInputDeviceSelectBox->setCurrentIndex(row);
            m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(false);
            Q_EMIT m_pVolumeControl->updateLoopBack(false);
            qDebug() << "findInputComboboxItem else -> Q_EMIT updateLoopBack false" << m_pVolumeControl->sourcePortName;
        }
    }
}

/*
 * 在input combobox list widget删除不可用的端口
 */
void UkmediaMainWidget::deleteNotAvailableComboboxInputPort()
{
    //删除不可用的输入端口
    QMap<int,QString>::iterator it;

    for(it=currentCboxInputPortLabelMap.begin();it!=currentCboxInputPortLabelMap.end();) {
        //没找到，需要删除
        if (comboboxInputPortIsNeedDelete(it.key(),it.value())) {
            int index = indexOfInputPortInInputCombobox(it.value());
            if (index == -1)
                return;
            m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(true);
            m_pInputWidget->m_pInputDeviceSelectBox->removeItem(index);
            m_pInputWidget->m_pInputDeviceSelectBox->hidePopup();
            m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(false);
            it = currentCboxInputPortLabelMap.erase(it);
            continue;
        }
        ++it;
    }
}


//传进来input portName用来定位他的位置
int UkmediaMainWidget::indexOfInputPortInInputCombobox(QString portName)
{
    for (int index=0;index<m_pInputWidget->m_pInputDeviceSelectBox->count();index++)
    {
        QString textport = m_pInputWidget->m_pInputDeviceSelectBox->itemText(index);
       // QString textcard = m_pInputWidget->m_pInputDeviceSelectBox->itemData(index).toString();
        if (textport == portName )
        {
            return index;
        }
    }
    return -1;
}


/*
 *  添加可用的输入端口到input list widget
 */
void UkmediaMainWidget::addComboboxAvailableInputPort()
{
    QMap<int,QMap<QString,QString>>::iterator at;
    QMap<QString,QString>::iterator it;
    QMap<QString,QString> tempMap;
    qDebug() << "addComboboxAvailableInputPort"<< m_pVolumeControl->inputPortMap.count() << m_pInputWidget->m_pInputDeviceSelectBox->currentText();

    //增加端口
    for(at=m_pVolumeControl->inputPortMap.begin();at!=m_pVolumeControl->inputPortMap.end();)
    {
        tempMap = at.value();
        for (it=tempMap.begin();it!=tempMap.end();){
            if (comboboxInputPortIsNeedAdd(at.key(),it.value())){
                addComboboxInputListWidgetItem(it.value(), findCardName(at.key(),m_pVolumeControl->cardMap));
                currentCboxInputPortLabelMap.insertMulti(at.key(), it.value());
            }
            ++it;
        }
        ++at;
    }
}

/*
 * 添加输入端口到input combobox widget
 */
void UkmediaMainWidget::addComboboxInputListWidgetItem(QString portLabel, QString cardName)
{
    int i = m_pInputWidget->m_pInputDeviceSelectBox->count();
    m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(true);
    m_pInputWidget->m_pInputDeviceSelectBox->insertItem(i,portLabel,cardName);
    m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(false);
    qDebug() << "添加输入设备端口" << portLabel << cardName;
}


/*
 * 当前的输入端口是否应该添加到Combobox input list widget上
 */
bool UkmediaMainWidget::comboboxInputPortIsNeedAdd(int index, QString name)
{
    QMap<int,QString>::iterator it;
    for(it=currentCboxInputPortLabelMap.begin();it!=currentCboxInputPortLabelMap.end();) {
        if (index == it.key() && name == it.value())
            return false;
        ++it;
    }
    return true;
}


/*
 * 当前的输入端口是否应该在Combobox list上删除
 */
bool UkmediaMainWidget::comboboxInputPortIsNeedDelete(int index, QString name)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for(it = m_pVolumeControl->inputPortMap.begin();it!=m_pVolumeControl->inputPortMap.end();)
    {
        if (it.key() == index) {
            portMap = it.value();
            for (at=portMap.begin();at!=portMap.end();) {
                if (name == at.value())
                    return false;
                ++at;
            }
        }
        ++it;
    }
    if(name == tr("None")){
        return false;
    }
    return true;
}


/*
 * 更新设备端口
*/
void UkmediaMainWidget::updateCboxDevicePort()
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> temp;
    currentCboxInputPortLabelMap.clear();
    currentCboxOutputPortLabelMap.clear();

    qDebug() << __func__ << "output" << m_pVolumeControl->outputPortMap.count() << "input" << m_pVolumeControl->inputPortMap.count();

    //  1.  update sink
    int count = m_pVolumeControl->outputPortMap.count();
    if (count == 0) {
        addNoneItem(SoundType::SINK);
    }
    else {
        removeNoneItem(SoundType::SINK);
    }

    //  2.update input
    count = m_pVolumeControl->inputPortMap.count();
    //  2.1 无可用输入设备，需添加None端口
    if (count == 0) {
        addNoneItem(SoundType::SOURCE);
    }
    //  2.2 存在两个及以上可用输入设备，需移除None端口
    else if (count > 1) {
        removeNoneItem(SoundType::SOURCE);
    }
    //  2.3 存在一个可用输入设备时，判断是否为蓝牙(蓝牙默认a2dp，需保留None端口)
    else if (count == 1) {
        int cardIndex = m_pVolumeControl->inputPortMap.firstKey();
        QString cardName = findCardName(cardIndex, m_pVolumeControl->cardMap);
        if (cardName.contains("bluez") )
            addNoneItem(SoundType::SOURCE);
        else
            removeNoneItem(SoundType::SOURCE);
    }

    //  3.初始化设备端口
    if (cboxfirstEntry) {
        for(it = m_pVolumeControl->outputPortMap.begin(); it != m_pVolumeControl->outputPortMap.end(); ++it) {
            temp = it.value();
            for (at = temp.begin(); at != temp.end(); ++at) {
                QString cardName = findCardName(it.key(), m_pVolumeControl->cardMap);
                addComboboxOutputListWidgetItem(at.value(), cardName);
            }
        }
        for(it = m_pVolumeControl->inputPortMap.begin(); it != m_pVolumeControl->inputPortMap.end(); ++it) {
            temp = it.value();
            for (at = temp.begin(); at != temp.end(); ++at) {
                QString cardName = findCardName(it.key(), m_pVolumeControl->cardMap);
                addComboboxInputListWidgetItem(at.value(), cardName);
            }
        }
        cboxfirstEntry = false;
    } else {//  4.更新设备端口
        for (int i = 0; i < m_pOutputWidget->m_pDeviceSelectBox->count(); i++) {
            int index = -1;
            QMap<int,QString>::iterator at;
            QString portname = m_pOutputWidget->m_pDeviceSelectBox->itemText(i);
            QString cardname = m_pOutputWidget->m_pDeviceSelectBox->itemData(i).toString();
            for (at = m_pVolumeControl->cardMap.begin(); at != m_pVolumeControl->cardMap.end(); ++at) {
                if (cardname == at.value()) {
                    index = at.key();
                    break;
                }
            }
            currentCboxOutputPortLabelMap.insertMulti(index, portname);
        }

        for(int i = 0; i < m_pInputWidget->m_pInputDeviceSelectBox->count(); i++) {
            int index = -1;
            QMap<int,QString>::iterator at;
            QString portname = m_pInputWidget->m_pInputDeviceSelectBox->itemText(i);
            QString cardname = m_pInputWidget->m_pInputDeviceSelectBox->itemData(i).toString();
            for (at = m_pVolumeControl->cardMap.begin(); at != m_pVolumeControl->cardMap.end(); ++at) {
                if (cardname == at.value()) {
                    index = at.key();
                    break;
                }
            }
            currentCboxInputPortLabelMap.insertMulti(index,portname);
        }

        m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(true);
        m_pOutputWidget->m_pDeviceSelectBox->blockSignals(true);
        deleteNotAvailableComboboxOutputPort();
        addComboboxAvailableOutputPort();
        initOutputComboboxItem();
        deleteNotAvailableComboboxInputPort();
        addComboboxAvailableInputPort();
        m_pOutputWidget->m_pDeviceSelectBox->blockSignals(false);
        m_pInputWidget->m_pInputDeviceSelectBox->blockSignals(false);
    }
}

/*
 *  Combobox选项改变，设置对应的输出设备
 */
void UkmediaMainWidget::cboxoutputListWidgetCurrentRowChangedSlot()
{
    //当所有可用的输出设备全部移除，台式机才会出现该情况
    if (m_boxoutputRow == -1)
        return;

    QString outputComboboxPortName = m_pOutputWidget->m_pDeviceSelectBox->itemText(m_boxoutputRow);
    QString outputComboboxCardName = m_pOutputWidget->m_pDeviceSelectBox->itemData(m_boxoutputRow).toString();

    QString inputComboboxPortName = m_pInputWidget->m_pInputDeviceSelectBox->currentText();
    QString inputComboboxCardName = m_pInputWidget->m_pInputDeviceSelectBox->currentData().toString();

    bool isContainBlue = inputComboboxDeviceContainBluetooth();

    //当输出设备从蓝牙切换到其他设备时，需将蓝牙声卡的配置文件切换为a2dp-sink
    if (isContainBlue && (strstr(m_pVolumeControl->defaultSourceName,\
                                "headset_head_unit") || strstr(m_pVolumeControl->defaultSourceName,"bt_sco_source"))){
        QString cardName = blueCardNameInCombobox();
        setCardProfile(cardName,"a2dp_sink");
    }

    QMap<int, QMap<QString,QString>>::iterator outputProfileMap;
    QMap<int ,QMap<QString,QString>>::iterator inputProfileMap;
    QMap<QString,QString> tempMap;
    QMap<QString,QString>::iterator at;
    QString endOutputProfile = "";
    QString endInputProfile = "";
    int count,i;
    int currentCardIndex = findCardIndex(outputComboboxCardName,m_pVolumeControl->cardMap);

    QStringList outputComboboxPortNameList = outputComboboxPortName.split("（");//新增设计combobox需要显示 端口名+（description）;
    QStringList inputComboboxPortNameList = inputComboboxPortName.split("（");

    for (outputProfileMap=m_pVolumeControl->profileNameMap.begin();outputProfileMap!= m_pVolumeControl->profileNameMap.end();) {
        if(currentCardIndex == outputProfileMap.key()){
            tempMap = outputProfileMap.value();
            for(at=tempMap.begin(),i=0;at!= tempMap.end();++i){
                if (at.key() == outputComboboxPortNameList.at(0)) {
                    count = i;
                    endOutputProfile = at.value();
                }
                ++at;
            }
        }
        ++outputProfileMap;
    }

    if (m_pInputWidget->m_pInputDeviceSelectBox->currentText().size()!=0) {
        QMap <QString,QString>::iterator it;
        QMap <QString,QString> temp;
        int index = findCardIndex(inputComboboxCardName,m_pVolumeControl->cardMap);
        for (inputProfileMap=m_pVolumeControl->inputPortProfileNameMap.begin(),count=0;inputProfileMap!= m_pVolumeControl->inputPortProfileNameMap.end();count++) {
            if (inputProfileMap.key() == index) {
                temp = inputProfileMap.value();
                for(it = temp.begin(); it != temp.end();){
                    if(it.key() == inputComboboxPortNameList.at(0)){
                        endInputProfile = it.value();
                    }
                    ++it;
                }
            }
            ++inputProfileMap;
        }
    }
    qDebug() << "outputListWidgetCurrentRowChangedSlot" << m_boxoutputRow << outputComboboxPortName << endOutputProfile <<endInputProfile;
    //如果选择的输入输出设备为同一个声卡，则追加指定输入输出端口属于的配置文件
    if ((m_pInputWidget->m_pInputDeviceSelectBox->currentText().size()!=0 && outputComboboxCardName == inputComboboxCardName ) || \
            outputComboboxCardName == "alsa_card.platform-sound_DA_combine_v5" && inputComboboxCardName == "3a.algo") {

        QString setProfile = endOutputProfile;
        if (!endOutputProfile.contains("input:analog-stereo") || !endOutputProfile.contains("HiFi")) {
            setProfile += "+";
            setProfile +=endInputProfile;
        }

        if (endOutputProfile.contains("a2dp-sink") && endInputProfile.contains("headset-head-unit"))
            setProfile = endOutputProfile;

        setCardProfile(outputComboboxCardName,setProfile);
        setDefaultOutputPortDevice(outputComboboxCardName,outputComboboxPortName);
    }
    //如果选择的输入输出设备不是同一块声卡，需要设置一个优先级高的配置文件
    else {
        int index = findCardIndex(outputComboboxCardName,m_pVolumeControl->cardMap);
        QMap <int,QList<QString>>::iterator it;
        QString profileName;
        for(it=m_pVolumeControl->cardProfileMap.begin();it!=m_pVolumeControl->cardProfileMap.end();) {
            if (it.key() == index) {
                if (strstr(endOutputProfile.toLatin1().data(),"headset_head_unit"))
                    endOutputProfile = "a2dp_sink";
                profileName = findHighPriorityProfile(index,endOutputProfile);
            }
            ++it;
        }

        QString setProfile = profileName;
        setCardProfile(outputComboboxCardName,setProfile);
        setDefaultOutputPortDevice(outputComboboxCardName,outputComboboxPortName);
    }

    m_pOutputWidget->m_pDeviceSelectBox->setObjectName("m_pOutputWidget->m_pDeviceSelectBox");
    ukcc::UkccCommon::buriedSettings("Audio", m_pOutputWidget->m_pDeviceSelectBox->objectName(),"select",m_pOutputWidget->m_pDeviceSelectBox->itemText(m_boxoutputRow));

    qDebug() <<  "active combobox output port:" << outputComboboxPortName << outputComboboxCardName;
}

void UkmediaMainWidget::cboxinputListWidgetCurrentRowChangedSlot()
{
    if (m_boxinputRow == -1)
    {
        return;
    }

    QString inputComboboxPortName = m_pInputWidget->m_pInputDeviceSelectBox->itemText(m_boxinputRow);
    QString inputComboboxCardName = m_pInputWidget->m_pInputDeviceSelectBox->itemData(m_boxinputRow).toString();

    QString outputComboboxPortName = m_pOutputWidget->m_pDeviceSelectBox->currentText();
    QString outputComboboxCardName = m_pOutputWidget->m_pDeviceSelectBox->currentData().toString();

    bool isContainBlue = inputComboboxDeviceContainBluetooth();

    //当输出设备从蓝牙切换到其他设备时，需将蓝牙声卡的配置文件切换为a2dp-sink
    if (isContainBlue && (strstr(m_pVolumeControl->defaultSinkName,"headset_head_unit") || strstr(m_pVolumeControl->defaultSourceName,"bt_sco_source")))
    {
        QString cardName = blueCardNameInCombobox();
        setCardProfile(cardName,"a2dp_sink");
    }

    if(inputComboboxCardName.contains("bluez_card")) {
        isCheckBluetoothInput = true;
    }
    else {
        isCheckBluetoothInput = false;
        if(inputComboboxPortName == tr("None")){
            qDebug() << "come Back Bluetooth A2dp Model";
            return ;
        }
    }

    QMap<int, QMap<QString,QString>>::iterator inputProfileMap;
    QMap<int, QMap<QString,QString>>::iterator outputProfileMap;
    QMap <QString,QString> temp;
    QMap<QString,QString>::iterator at;
    QString endOutputProfile = "";
    QString endInputProfile = "";
    int index = findCardIndex(inputComboboxCardName,m_pVolumeControl->cardMap);
    QStringList outputComboboxPortNameList = outputComboboxPortName.split("（");//新增设计combobox需要显示 端口名+（description）;
    QStringList inputComboboxPortNameList = inputComboboxPortName.split("（");
    for (inputProfileMap=m_pVolumeControl->inputPortProfileNameMap.begin();inputProfileMap!= m_pVolumeControl->inputPortProfileNameMap.end();) {
        if (inputProfileMap.key() == index) {
            temp = inputProfileMap.value();
            for(at=temp.begin();at!=temp.end();){
                if(at.key() == inputComboboxPortNameList.at(0)){
                    endInputProfile = at.value();
                }
                ++at;
            }
        }
        ++inputProfileMap;
    }
    if (m_pOutputWidget->m_pDeviceSelectBox->currentText().size()!=0) {
        QMap <QString,QString>::iterator it;
        QMap <QString,QString> temp;
        int index = findCardIndex(outputComboboxCardName,m_pVolumeControl->cardMap);
        for (outputProfileMap=m_pVolumeControl->profileNameMap.begin();outputProfileMap!= m_pVolumeControl->profileNameMap.end();) {
            if (outputProfileMap.key() == index) {
                temp = outputProfileMap.value();
                for(it=temp.begin();it!=temp.end();){
                    if(it.key() == outputComboboxPortNameList.at(0)){
                        endOutputProfile = it.value();
                    }
                    ++it;
                }
            }
            ++outputProfileMap;
        }
    }
    //如果选择的输入输出设备为同一个声卡，则追加指定输入输出端口属于的配置文件
    if (m_pOutputWidget->m_pDeviceSelectBox->currentText().size()!=0 && inputComboboxCardName == outputComboboxCardName) {
        QString  setProfile;
        //有些声卡的配置文件默认只有输入/输出设备或者配置文件包含了输出输入设备，因此只需要取其中一个配置文件即可
        if (endOutputProfile == "a2dp-sink" || endInputProfile == "headset_head_unit" || endOutputProfile == "HiFi" ) {
            setProfile += endInputProfile;
        }
        else {
            setProfile += endOutputProfile;
            setProfile += "+";
            setProfile +=endInputProfile;
        }
        setCardProfile(inputComboboxCardName,setProfile);
        setDefaultInputPortDevice(inputComboboxCardName,inputComboboxPortName);
    }
    //如果选择的输入输出设备不是同一块声卡，需要设置一个优先级高的配置文件
    else {
        int index = findCardIndex(inputComboboxCardName,m_pVolumeControl->cardMap);
        QMap <int,QList<QString>>::iterator it;
        QString profileName;
        for(it=m_pVolumeControl->cardProfileMap.begin();it!=m_pVolumeControl->cardProfileMap.end();) {

            if (it.key() == index) {
                QStringList list= it.value();
                profileName = findHighPriorityProfile(index,endInputProfile);
                if (list.contains(endOutputProfile)) {

                }
            }
            ++it;
        }
        QString  setProfile = profileName;
        setCardProfile(inputComboboxCardName,setProfile);
        setDefaultInputPortDevice(inputComboboxCardName,inputComboboxPortName);
    }

    m_pInputWidget->m_pInputDeviceSelectBox->setObjectName("m_pInputWidget->m_pInputDeviceSelectBox");
    ukcc::UkccCommon::buriedSettings("Audio",m_pInputWidget->m_pInputDeviceSelectBox->objectName(),"select",m_pInputWidget->m_pInputDeviceSelectBox->itemText(m_boxinputRow));

    qDebug() << "active combobox input port:" << inputComboboxPortName << inputComboboxCardName<< isCheckBluetoothInput;
}


void UkmediaMainWidget::updateComboboxListWidgetItemSlot()
{
    qDebug() << "updateComboboxListWidgetItemSlot";
    initOutputComboboxItem();
    initInputComboboxItem();
    themeChangeIcons();
}


/*****************************************************************
* 函数名称： initAlertSound
* 功能描述： 初始化音效提示音
* 参数说明： 系统音效主题路径 /usr/share/sounds
******************************************************************/
void UkmediaMainWidget::initAlertSound(const char* path)
{
    QDir dir(path);

    if (!dir.exists())
        return;

    //  获取音效主题文件夹
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList nameList = dir.entryList();
    QFileInfoList pathList = dir.entryInfoList();

    //  忽略freeDesktop
    if (nameList.contains("freedesktop")) {
        nameList.removeOne("freedesktop");
        pathList.removeOne(QFileInfo("/usr/share/sounds/freedesktop"));
    }

    for (int i = 0; i < pathList.size(); i++) {
        QFileInfo fileInfo = pathList.at(i);
        if (fileInfo.isDir())
            addSoundFileInCombobox(fileInfo.filePath(), fileInfo.fileName());
    }
}


/*****************************************************************
* 函数名称： addSoundFileInCombobox
* 功能描述： 添加音效文件到自定义音效下拉框
* 参数说明： path: 音效主题路径 dirName: 音效主题名
******************************************************************/
void UkmediaMainWidget::addSoundFileInCombobox(QString path, QString dirName)
{
    QDir dir(path);

    if (!dir.exists())
        return;

    dir.setSorting(QDir::DirsFirst);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsLast);

    QStringList nameList = dir.entryList();
    QFileInfoList pathList = dir.entryInfoList();
    QString xmlName = dirName.append(".xml");

    //  添加lingmo规范音效主题
    if (nameList.contains(xmlName) && nameList.contains("stereo")) {
        for(int i = 0; i < pathList.size(); i++) {
            QFileInfo fileInfo = pathList.at(i);
            if (fileInfo.fileName().contains(xmlName))
                populateModelFromFile(this, fileInfo.filePath().toLatin1().data());
        }
    }
    //  兼容社区音效主题
    else if (! nameList.contains(xmlName) && nameList.contains("stereo")) {
        for(int i = 0; i < pathList.size(); i++) {
            QFileInfo fileInfo = pathList.at(i);
            if (fileInfo.fileName() == "stereo" && fileInfo.isDir()) {
                QDir d(fileInfo.filePath());
                d.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
                QFileInfoList dList = d.entryInfoList();
                for(int i = 0; i < dList.size(); i++) {
                    QFileInfo fileInfo = dList.at(i);
                    QString name = fileInfo.fileName().append("(");
                    name.append(dir.dirName());
                    name.append(")");
                    m_pSoundList.append(fileInfo.filePath());
                    m_pSoundNameList.append(name);
                    m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(true);
                    m_pSoundWidget->m_pVolumeChangeCombobox->addItem(name, fileInfo.filePath());
                    m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(false);
                    m_pSoundWidget->m_pNotificationCombobox->blockSignals(true);
                    m_pSoundWidget->m_pNotificationCombobox->addItem(name, fileInfo.filePath());
                    m_pSoundWidget->m_pNotificationCombobox->blockSignals(false);
                }
            }
        }
    }
}


/*****************************************************************
* 函数名称： customSoundEffectsSlot
* 功能描述： 设置自定义音效并同时播放对应音效
* 参数说明： 对应音效的index
******************************************************************/
void UkmediaMainWidget::customSoundEffectsSlot(int index)
{
    //音效文件完整路径
    QString soundPath = m_pSoundList.at(index);
    playAlretSoundFromPath(this, soundPath);

    QString soundType;
    QComboBox *p = qobject_cast<QComboBox *>(sender());

    if ("volChangeCbox" == p->objectName())
    {
        soundType = AUDIO_VOLUME_CHANGE;
        ukcc::UkccCommon::buriedSettings("Audio", p->objectName(),"select",p->currentText());
    }
    else if ("notifyCbox" == p->objectName())
    {
        soundType = NOTIFICATION_GENGERAL;
        ukcc::UkccCommon::buriedSettings("Audio", p->objectName(),"select",p->currentText());
    }

    m_pSoundSettings->set(soundType, soundPath);
    m_pSoundSettings->set(SOUND_CUSTOM_THEME_KEY, true);
}


/*****************************************************************
* 函数名称： resetCustomSoundEffects
* 功能描述： 切换音效主题时，已设置的自定义音效重置为主题默认音效
* 参数说明： theme: 主题名 soundFile: 自定义音效类型
* 返回值：   重置成功返回true，失败返回false
******************************************************************/
bool UkmediaMainWidget::resetCustomSoundEffects(QString theme, QString soundFile)
{
    int index;
    QString path = SOUND_FILE_PATH;
    path.replace("xxxTheme", theme);
    path.replace("xxxFile", soundFile);


    if (m_pSoundList.contains(path))
        index = m_pSoundList.indexOf(path);
    else if (m_pSoundList.contains(path.replace("ogg", "oga"))) //社区音效主题资源文件为oga格式
        index = m_pSoundList.indexOf(path);
    else
        index = -1;

    if (-1 == index)
        return false;


    //恢复主题默认音效并同步定制音效的gsetting值
    if (soundFile == AUDIO_VOLUME_CHANGE) {
        m_pSoundSettings->set(AUDIO_VOLUME_CHANGE, path);
        m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(true);
        m_pSoundWidget->m_pVolumeChangeCombobox->setCurrentIndex(index);
        m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(false);
    }

    else if (soundFile == NOTIFICATION_GENGERAL) {
        m_pSoundSettings->set(NOTIFICATION_GENGERAL, path);
        m_pSoundWidget->m_pNotificationCombobox->blockSignals(true);
        m_pSoundWidget->m_pNotificationCombobox->setCurrentIndex(index);
        m_pSoundWidget->m_pNotificationCombobox->blockSignals(false);
    }

    return true;
}


/*****************************************************************
* 函数名称： playAlretSoundFromPath
* 功能描述： 切换自定义音效时播放对应音效提示音
* 参数说明： path: 音效资源文件路径
******************************************************************/
void UkmediaMainWidget::playAlretSoundFromPath (UkmediaMainWidget *w,QString path)
{
    caPlayForWidget(w, 0,
                     CA_PROP_APPLICATION_NAME, _("Sound Preferences"),
                     CA_PROP_MEDIA_FILENAME, path.toLatin1().data(),
                     CA_PROP_EVENT_DESCRIPTION, _("Testing event sound"),
                     CA_PROP_CANBERRA_CACHE_CONTROL, "never",
                     CA_PROP_APPLICATION_ID, "org.mate.VolumeControl",
                 #ifdef CA_PROP_CANBERRA_ENABLE
                     CA_PROP_CANBERRA_ENABLE, "1",
                 #endif
                     NULL);
}
