
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
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "ukmedia_volume_control.h"
#include "ukmedia_output_widget.h"
#include "ukmedia_input_widget.h"
#include "ukmedia_sound_effects_widget.h"
#include "lingmo_list_widget_item.h"
#include <QMediaPlayer>
#include <gio/gio.h>
#include <libxml/tree.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gobject/gparamspecs.h>
#include <glib/gstdio.h>
extern "C" {
#include <dconf/dconf.h>
#include <canberra.h>
#include <glib/gmain.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/glib-mainloop.h>
#include <pulse/error.h>
#include <pulse/subscribe.h>
#include <pulse/introspect.h>
}
#include <utime.h>
#include <a.out.h>
#include <QScreen>
#include <QApplication>
#include <QDomDocument>
#include <QGSettings>
#include <QAudioInput>
#include <set>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QSettings>
#include <QTime>

#include "ukcc/interface/ukcccommon.h"
#include "ukmedia_device_control_widget.h"
#include "ukmedia_settings_widget.h"
#include "ukmedia_app_device_ctrl.h"

#define DEFAULT_ALERT_ID "__default"
#define CUSTOM_THEME_NAME "__custom"
#define NO_SOUNDS_THEME_NAME "__no_sounds"

#ifdef __GNUC__
#define CA_CLAMP(x, low, high)                                          \
        __extension__ ({ typeof(x) _x = (x);                            \
                        typeof(low) _low = (low);                       \
                        typeof(high) _high = (high);                    \
                        ((_x > _high) ? _high : ((_x < _low) ? _low : _x)); \
                })
#else
#define CA_CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

typedef enum
{
    GVC_LEVEL_SCALE_LINEAR,
    GVC_LEVEL_SCALE_LOG
} LevelScale;


class UkmediaMainWidget : public QWidget
{
    Q_OBJECT

public:
    UkmediaMainWidget(QWidget *parent = nullptr);
    ~UkmediaMainWidget();

private:
    void initWidget(); //初始化界面
    void initGsettings(); //初始化gsetting值
    void initLabelAlignment();  //初始化label对齐方式
    void initButtonSliderStatus(QString key);
    void dealSlot(); //处理槽函数
    int valueToPaVolume(int value); //滑动条值转换成音量
    int paVolumeToValue(int value); //音量值转换成滑动条值
    void handleBalanceSlider(float balanceVolume);
    void themeChangeIcons();
    int caProplistMergeAp(ca_proplist *p, va_list ap);
    int caPlayForWidget(UkmediaMainWidget *w, uint32_t id, ...);
    int caProplistSetForWidget(ca_proplist *p, UkmediaMainWidget *widget);

    QPixmap drawDarkColoredPixmap(const QPixmap &source);
    QPixmap drawLightColoredPixmap(const QPixmap &source);

    void inputVolumeDarkThemeImage(int value,bool status);
    void outputVolumeDarkThemeImage(int value,bool status);
    int getInputVolume();
    int getOutputVolume();
//    void comboboxCurrentTextInit();
    QList<char *> listExistsPath();
    QString findFreePath();
    void addValue(QString name,QString filename);

    static void updateTheme (UkmediaMainWidget *w);

    static void setupThemeSelector (UkmediaMainWidget *w);
    static void soundThemeInDir (UkmediaMainWidget *w,GHashTable *hash,const char *dir);
    static char *loadIndexThemeName (const char *index,char **parent);

    static void setComboxForThemeName (UkmediaMainWidget *w,const char *name);
    static void updateAlertsFromThemeName (UkmediaMainWidget *w,const gchar *name);
    static void updateAlert (UkmediaMainWidget *w,const char *alert_id);
    static int getFileType (const char *sound_name,char **linked_name);
    static char *customThemeDirPath (const char *child);

    void initAlertSound(const char *path);
    void addSoundFileInCombobox(QString path, QString dirName);
    bool resetCustomSoundEffects(QString theme, QString soundFile);
    static void populateModelFromDir (UkmediaMainWidget *w,const char *dirname);
    static void populateModelFromFile (UkmediaMainWidget *w,const char *filename);
    static void populateModelFromNode (UkmediaMainWidget *w,xmlNodePtr node);
    static xmlChar *xmlGetAndTrimNames (xmlNodePtr node);

    void playAlretSoundFromPath (UkmediaMainWidget *w,QString path);

    static gboolean saveAlertSounds (QComboBox *combox,const char *id);
    static void deleteOldFiles (const char **sounds);
    static void deleteOneFile (const char *sound_name, const char *pattern);
    static void deleteDisabledFiles (const char **sounds);
    static void addCustomFile (const char **sounds, const char *filename);
    static gboolean cappletFileDeleteRecursive (GFile *file, GError **error);
    static gboolean directoryDeleteRecursive (GFile *directory, GError **error);
    static void createCustomTheme (const char *parent);
    static void customThemeUpdateTime (void);
    static gboolean customThemeDirIsEmpty (void);

    void addNoneItem(int soundType);
    void removeNoneItem(int soundType);
    void initOutputComboboxItem();//初始化输出的Combobox选项框
    void initInputComboboxItem();//初始化输入的Combobox选项框
    void findOutputComboboxItem(QString cardName,QString portLabel); //初始化Combobox output/input list widget的选项

    void addComboboxAvailableOutputPort();
    void addComboboxOutputListWidgetItem(QString portLabel, QString cardName);
    void deleteNotAvailableComboboxOutputPort();//删除不可用的端口
    int indexOfOutputPortInOutputCombobox(QString portName);
    bool comboboxOutputPortIsNeedAdd(int index,QString name);//port是否需要在Combobox list中添加
    bool comboboxOutputPortIsNeedDelete(int index,QString name);//port是否需要在Combobox list删除

    void findInputComboboxItem(QString cardName,QString portLabel); //初始化Combobox output/input list widget的选项
    void addComboboxAvailableInputPort();
    void addComboboxInputListWidgetItem(QString portLabel, QString cardName); //添加input listwidget item
    void deleteNotAvailableComboboxInputPort();
    int indexOfInputPortInInputCombobox(QString portName);//获取输入combobox当前的选项框的index
    bool comboboxInputPortIsNeedAdd(int index,QString name);//port是否需要在Combobox list中添加
    bool comboboxInputPortIsNeedDelete(int index,QString name);//port是否需要在Combobox list删除
    int findCardIndex(QString cardName, QMap<int,QString> cardMap);//查找声卡指定的索引

    QString findCardName(int index,QMap<int,QString> cardMap);
    QString findHighPriorityProfile(int index,QString profile);
    QString findPortSink(int cardIndex,QString portName);
    QString findPortSource(int cardIndex,QString portName);

    bool inputComboboxDeviceContainBluetooth();
    QString blueCardNameInCombobox();//记录蓝牙声卡名称

    void inputStreamMapCardName(QString streamName,QString cardName);
    void outputStreamMapCardName(QString streamName,QString cardName);
    QString findInputStreamCardName(QString streamName);
    QString findOutputStreamCardName(QString streamName);

    bool exitBluetoochDevice();

    QString findOutputPortName(int index,QString portLabel); //找到outputPortLabel对应的portName
    QString findInputPortName(int index,QString portLabel); //找到inputPortLabel对应的portName
    QString findOutputPortLabel(int index,QString portName); //查找名为portName对应的portLabel
    QString findInputPortLabel(int index,QString portName); //查找名为portName对应的portLabel
    void setCardProfile(QString name,QString profile); //设置声卡的配置文件
    void setDefaultOutputPortDevice(QString devName,QString portName); //设置默认的输出端口
    void setDefaultInputPortDevice(QString devName,QString portName); //设置默认的输入端口
    QString findCardActiveProfile(int index); //查找声卡的active profile

private Q_SLOTS:

    void initVoulmeSlider(); //初始化音量滑动条的值
    void themeComboxIndexChangedSlot(int index); //主题下拉框改变
    void customSoundEffectsSlot(int index); //自定义音效改变
    void outputWidgetSliderChangedSlot(int v); //输出音量改变
    void outputWidgetSliderChangedSlotInBlue(int value); //蓝牙模式下调节音量特殊处理
    void inputWidgetSliderChangedSlot(int v); //输入滑动条更改
    void inputMuteButtonSlot(); //输入音量静音控制
    void outputMuteButtonSlot(); //输出音量静音控制
    void balanceSliderChangedMono(int balanceSliderValue,int sinkVolume ,int sinkIndex); //开启单声道
    void balanceSliderChangedSlot(int balanceSliderValue); //平衡值改变
    void peakVolumeChangedSlot(double v); //输入等级
    void updateCboxDevicePort(); //更新combobox设备端口
    void updateComboboxListWidgetItemSlot();

    void timeSliderSlot();
    void lingmoThemeChangedSlot(const QString &);
    void onKeyChanged (const QString &);
    void globalThemeChangedSlot(const QString &);

    void startupButtonSwitchChangedSlot(bool status); //开机音乐开关
    void poweroffButtonSwitchChangedSlot(bool status); //关机音乐开关
    void logoutMusicButtonSwitchChangedSlot(bool status); //注销音乐开关
    void wakeButtonSwitchChangedSlot(bool status); //唤醒音乐开关
    void alertSoundButtonSwitchChangedSlot(bool status);
    void noiseReductionButtonSwitchChangedSlot(bool status); //降噪开关
    void loopbackButtonSwitchChangedSlot(bool status);//侦听开关
    void setAllSinkMono();  //开启单声道，遍历sinkMap，将所有双声道sink设置为单声道输出
    void monoAudioBtuuonSwitchChangedSlot(bool status); //单声道开关
    void volumeIncreaseBtuuonSwitchChangedSlot(bool status);
    void bootMusicSettingsChanged(const QString &);
    void cboxoutputListWidgetCurrentRowChangedSlot();//combobox output list widget选项改变
    void cboxinputListWidgetCurrentRowChangedSlot();//combobox input list widget选项改变

private:
    UkmediaInputWidget *m_pInputWidget;
    UkmediaOutputWidget *m_pOutputWidget;
    UkmediaSoundEffectsWidget *m_pSoundWidget;
    UkmediaVolumeControl *m_pVolumeControl;
    UkmediaSettingsWidget *m_pSettingsWidget;

    QStringList m_pSoundList;
    QStringList m_pSoundNameList;
    QStringList m_pThemeNameList;

    QStringList m_soundThemeList;
    QStringList m_soundThemeDirList;
    QStringList m_soundThemeXmlNameList;

    QGSettings *m_pSoundSettings;
    QGSettings *m_pBootSetting;
    QGSettings *m_pThemeSetting;
    QGSettings *m_pGlobalThemeSetting;

    QString mThemeName;
    QString m_languageEnvStr;

    bool m_hasMusic;
    bool firstEnterSystem = true;

    const gchar* m_privOutputPortLabel = "";

    int callBackCount = 0;
    bool firstEntry = true;

    bool cboxfirstEntry = true;
    QMap<int, QString> currentOutputPortLabelMap;
    QMap<int, QString> currentInputPortLabelMap;
    QMap<QString,QString> inputCardStreamMap;
    QMap<QString,QString> outputCardStreamMap;
    QMap<int, QString> currentCboxOutputPortLabelMap;
    QMap<int, QString> currentCboxInputPortLabelMap;


    bool updatePort = true;
    bool setDefaultstream = true;
    int reconnectTime;
    QTimer *time;

    QTimer *timeSlider;
    bool mousePress = false;
    bool mouseReleaseState = false;

    QTimer *timeSliderBlance;
    bool mousePressBlance = false;
    bool mouseReleaseStateBlance = false;
    ca_context* m_caContext = nullptr;

    QTime m_boxRepeatTime;
    QTimer m_boxoutputRepeatTimer;
    int m_boxoutputRow = 0;
    QTimer m_boxinputRepeatTimer;
    int m_boxinputRow = 0;
    SystemVersion m_version;
};

#endif // WIDGET_H
