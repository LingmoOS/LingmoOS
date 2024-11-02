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
#ifndef UKMEDIAMAINWIDGET_H
#define UKMEDIAMAINWIDGET_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <QMenu>
#include <QCheckBox>
#include <QWidgetAction>
#include <QMessageBox>
#include <QFrame>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QGSettings>
#include <QMediaPlayer>
#include <QTimer>
#include <QTime>
#include <QFrame>
#include <QDesktopWidget>
#include <QMap>
#include <QLayout>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <windowmanager/windowmanager.h>
#include <lingmostylehelper/lingmostylehelper.h>

#include "dbus-adaptor/dbus-adaptor.h"
#include "dbus-adaptor/bluez-adaptor.h"
#include "custom_sound.h"
#include "ukmedia_application_volume_widget.h"
#include "ukmedia_osd_display_widget.h"
#include "lingmo_media_set_headset_widget.h"
#include "ukmedia_volume_control.h"
#include "lingmo_list_widget_item.h"
#include "ukmedia_system_volume_widget.h"

extern "C" {
#include <gio/gio.h>
#include <dconf/dconf.h>
#include <canberra.h>
#include <glib/gmain.h>
}
#include <alsa/asoundlib.h>
#include <set>

//  为平板模式提供设置音量值
#define LINGMO_VOLUME_KEY   "volumesize" //音量大小
#define LINGMO_VOLUME_STATE "soundstate" //音量状态
#define LINGMO_VOLUME_BRIGHTNESS_GSETTING_ID "org.lingmo.quick-operation.panel"

// 初始化音量
#define HEADPHONE_VOLUME 11141
#define HDMI_VOLUME 65536
#define OUTPUT_VOLUME 43909
#define MIC_VOLUME 65536

//  任务栏多屏显示声音应用获取屏幕可用区域接口
#define PANEL_SETTINGS      "org.lingmo.panel.settings"
#define PANEL_SIZE_KEY      "panelsize"
#define PANEL_POSITION_KEY  "panelposition"

const QByteArray TRANSPARENCY_GSETTINGS = "org.lingmo.control-center.personalise";

static const gchar *iconNameOutputs[] = {
        "audio-volume-muted-symbolic",
        "audio-volume-low-symbolic",
        "audio-volume-medium-symbolic",
        "audio-volume-high-symbolic",
        NULL
};

static const gchar *iconNameInputs[] = {
        "microphone-sensitivity-muted-symbolic",
        "microphone-sensitivity-low-symbolic",
        "microphone-sensitivity-medium-symbolic",
        "microphone-sensitivity-high-symbolic",
        NULL
};

typedef struct {
        const pa_card_port_info *headphones;
        const pa_card_port_info *headsetmic;
        const pa_card_port_info *headphonemic;
        const pa_card_port_info *internalmic;
        const pa_card_port_info *internalspk;
} headsetPorts;

class QDBusServiceWatcher;


class UkmediaMainWidget : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.media")//lingmo-media的dbus接口
public:
    explicit UkmediaMainWidget(QMainWindow *parent = nullptr);
    ~UkmediaMainWidget();

    enum PanelPosition{
            Bottom = 0, //!< The bottom side of the screen.
            Top,    //!< The top side of the screen.
            Left,   //!< The left side of the screen.
            Right   //!< The right side of the screen.
        };

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);

Q_SIGNALS:
    void updateVolume(int value);
    void updateSourceVolume(int value);
    void updateMute(bool isMute);
    void updateSourceMute(bool isMute);
    void updateDevSignal();

public Q_SLOTS:
    QString getDefaultOutputDevice();
    QString getDefaultInputDevice();
    QStringList getAllOutputDevices();
    QStringList getAllInputDevices();
    int getDefaultOutputVolume();
    int getDefaultInputVolume();
    bool getDefaultOutputMuteState();
    bool getDefaultInputMuteState();
    bool setDefaultOutputVolume(int value);
    bool setDefaultInputVolume(int value);
    bool setDefaultOutputMuteState(bool mute);
    bool setDefaultInputMuteState(bool mute);
    bool setDefaultOutputDevice(QString deviceName);
    bool setDefaultInputDevice(QString deviceName);
    void initPanelGSettings();
    QRect caculatePosition(); //任务栏多屏显示声音应用获取屏幕可用区域接口
    void advancedWidgetShow(); //显示advance widget

    QStringList getAppList();
    QList<QVariant> getRecordAppInfo();
    QList<QVariant> getPlaybackAppInfo();
    QString getSystemInputDevice();
    QString getSystemOutputDevice();
    QString getAppInputDevice(QString appName);
    QString getAppOutputDevice(QString appName);
    int  getAppVolume(QString appName);
    bool getAppMuteState(QString appName);
    bool setAppVolume(QString appName, int value);
    bool setAppMuteState(QString appName, bool state);
    bool setSystemInputDevice(QString portLabel);
    bool setSystemOutputDevice(QString portLabel);
    bool setAppOutputDevice(QString appName, int cardIndex, QString sinkPortName);
    bool setAppInputDevice(QString appName, int cardIndex, QString sourcePortName);
    bool isPortHidingNeeded(int soundType, int cardIndex, QString portLabel);

private Q_SLOTS:
    void switchMonoAudio();
    void switchModuleLoopBack();
    void activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason reason); //托盘图标
    void jumpControlPanelSlot(); //跳转到控制面板

    void setHeadsetPort(QString str); //根据弹出框选择的切换不同的端口
    void handleTimeout();
    void advancedSystemSliderChangedSlot(int value); //advancedSystemSlider值改变
    void systemVolumeSliderChangedSlot(int value);

    void systemVolumeSliderChangedSlotInBlue(int value); //蓝牙模式下调节音量特殊处理

    void systemVolumeButtonClickedSlot();

    void mouseMeddleClickedTraySlot(); //鼠标中间贱点击托盘图标
    void trayWheelRollEventSlot(bool step); //鼠标滚轮在托盘图标上滚动

    void appWidgetMuteButtonCLickedSlot();//应用音量静音按钮点击

    void lingmoThemeChangedSlot(const QString &themeStr); //主题改变
    void soundThemeChangedSlot(const QString &soundThemeStr);
    void fontSizeChangedSlot(const QString &themeStr); //字体大小改变
    void volumeSettingChangedSlot(const QString &key); //volume gseting值改变
    void initVolumeSlider(); //初始化滑动条

    void addAppToAppwidget(QString name, QString iconName, int index, int value, int channel); //添加应用
    void removeSubAppWidget(QString m_pAppName); //移除子应用窗口
    void sinkInputVolumeChangedSlot(QString name, QString iconName, int value); //sinkInput音量更新
    void soundSettingChanged(const QString &);

    //添加拔插headset提示
    void checkAudioDeviceSelectionNeeded (const pa_card_info *info);
    void updateDevicePort();
    void updateListWidgetItemSlot(); //更新输出设备列表
    void outputListWidgetCurrentRowChangedSlot(int row); //output list widget选项改变
    void inputListWidgetCurrentRowChangedSlot(int row); //output list widget选项改变
    void updateAppVolume(QString str, int value, bool state);
    void deviceChangedShowSlot(QString dev_name);//osd提示弹窗

    void onTransChanged();
    void paintWithTrans();

    void onPrepareForSleep(bool sleep);
    void getSessionActive(); //获取session的活跃状态
    void batteryLevelChanged(QString dev_macAddr, int battery);
    void sendUpdateVolumeSignal(int soundType, int value);
    void sendUpdateMuteSignal(int soundType, bool isMute);
    void initBlueDeviceVolume(int index, QString name);

private:
    void UkmediaDbusRegister(); //注册Dbus信号
    void initSystemTrayIcon(); //初始化系统托盘
    void initWidget(); //初始化弹出框
    void initVolumeLed();//初始化静音灯
    void initGsettings(); //初始化gsetting
    void initStrings();
    void initLabelAlignment();  //初始化label对齐方式
    void initSystemVolume(); //初始化系统音量
    void myLine(QFrame *volumeLine,QFrame *volumeSettingLine);//添加分隔线
    void dealSlot(); //处理槽函数
    void systemTrayMenuInit(); //初始化右键菜单

    void initOutputListWidgetItem();//初始化默认选择的输出设备
    void initInputListWidgetItem();//初始化默认选择的输输入设备

    QPixmap drawLightColoredPixmap(const QPixmap &source); //绘制高亮颜色图标
    QPixmap drawDarkColoredPixmap(const QPixmap &source); //绘制深色颜色图标
    void themeChangeIcons(); //主题更改时更新图标颜色

    void hideWindow(); //隐藏窗口
    void inputVolumeDarkThemeImage(int value,bool status); //绘制input button颜色
    void outputVolumeDarkThemeImage(int value,bool status); //绘制output button颜色
    void drawImagColorFromTheme(LingmoUIButtonDrawSvg *button, QString iconStr); //绘制iamge颜色
    void setVolumeSettingValue(int value); //设置volume gsetting的值
    void setVolumeSettingMute(bool state); //设置volume gsetting的静音

    int valueToPaVolume(int value); //滑动条值转换成音量
    int paVolumeToValue(int value); //音量值转换成滑动条值
    QString outputVolumeToIconStr(bool status,int volume);

    QString getAppName(QString desktopfp); //从desktop中获取应用名称
    QString getAppIcon(QString desktopfp); //从desktop中获取应用图标
    QString appNameToIconName(QString appName, QString appIconName);

    void initSubApplicationWidget(QString str); //子应用窗口布局

    static gboolean verifyAlsaCard (int cardindex,gboolean *headsetmic,gboolean *headphonemic);
    static headsetPorts *getHeadsetPorts (const pa_card_info *c);
    void freePrivPortNames();

    void setDeviceButtonState(int row);
    void addOutputListWidgetItem(QString portName, QString portLabel,QString cardName); //添加output listWidget item
    void addInputListWidgetItem(QString portName, QString cardName); //添加input listwidget item

    int findCardIndex(QString cardName, QMap<int,QString> cardMap);//查找声卡指定的索引
    QString findCardName(int index,QMap<int,QString> cardMap);
    QString findHighPriorityProfile(int index,QString profile);
    QString findPortSink(int index,QString portName);
    QString findPortSource(int index,QString portName);

    bool outputPortIsNeedDelete(int index,QString name);//port是否需要在outputListWidget删除
    bool outputPortIsNeedAdd(int index,QString name);//port是否需要在outputListWidget删除
    bool inputPortIsNeedDelete(int index,QString name);//port是否需要在inputListWidget删除
    bool inputPortIsNeedAdd(int index,QString name);//port是否需要在inputListWidget删除

    int indexOfOutputPortInOutputListWidget(QString portName);
    int indexOfInputPortInInputListWidget(QString portName);

    void deleteNotAvailableOutputPort(); //删除不可用的输出端口
    void addAvailableOutputPort(); //添加可用输出端口
    void deleteNotAvailableInputPort(); //删除不可用的输入端口
    void addAvailableInputPort(); //添加可用输入端口

    QString findOutputPortName(int index,QString portLabel); //找到outputPortLabel对应的portName
    QString findInputPortName(int index,QString portLabel); //找到inputPortLabel对应的portName
    QString findOutputPortLabel(int index,QString portName); //查找名为portName对应的portLabel
    QString findInputPortLabel(int index,QString portName); //查找名为portName对应的portLabel
    void setCardProfile(QString name,QString profile); //设置声卡的配置文件
    void setDefaultOutputPortDevice(QString devName,QString portName); //设置默认的输出端口
    void setDefaultInputPortDevice(int cardIndex, QString portLabel); //设置默认的输入端口
    QString findCardActiveProfile(int index); //查找声卡的active profile
    QString stringRemoveUnrecignizedChar(QString str);//移除xml文件中不能识别的字符

    void findOutputListWidgetItem(QString cardName,QString portLabel);
    void findInputListWidgetItem(QString cardName,QString portLabel);
    QString blueCardName(); //记录蓝牙声卡名称
    bool inputDeviceContainBluetooth();
    QString findFile(const QString path,QString str);

    void inhibit();
    void uninhibit();
    void switchModuleEchoCancel(); //加载或卸载降噪模块
    void resetVolumeSliderRange(); //重新设置滑动条的范围
    void switchStartupPlayMusic(); //接收到开关按钮的gsetting信号
    void monitorSessionStatus();
    void playAlertSound(QString soundEvent);    //播放提示音

private:
    FixLabel *systemWidgetLabel;
    FixLabel *appWidgetLabel;
    QHBoxLayout * m_tabBarLayout = nullptr;

    UkmediaVolumeControl *m_pVolumeControl;
    Bluetooth_Dbus *m_pBluetoothDbus;
    UkmediaSystemVolumeWidget *systemWidget;
    QTabWidget *m_pTabWidget;
    ApplicationVolumeWidget *appWidget; //应用界面窗口
    UkmediaOsdDisplayWidget *osdWidget;
    LingmoUIMediaSetHeadsetWidget *headsetWidget; //当声卡中有headsetmic 端口时插入4段式耳机的弹窗

    UkmediaOsdDisplayWidget *headset;   //需求:输出设备插拔提示弹出

    Divider *volumeLine;
    Divider *volumeSettingLine;

    MyTimer *timer; //定时器
    QMenu *menu; //右键菜单
    QGSettings *m_pSoundSettings;
    QGSettings *m_pStartUpSetting;
    QGSettings *m_pThemeSetting; //主题gsettting
    QGSettings *m_pTransparencySetting; //透明度gestting
    QGSettings *m_pVolumeSetting; //
    QGSettings *m_pFontSetting; //字体gsetting
    QGSettings *m_pSoundThemeSetting;

    QGSettings * m_pTransGsettings;   //透明度配置文件
    double m_pTransparency=0.0;  //透明度

    UkmediaTrayIcon *soundSystemTrayIcon; //系统托盘
    QAction *m_pSoundPreferenceAction; //声音首选项菜单项

    QStringList *eventList; //提示声音类型列表
    QStringList *eventIdNameList; //提示声音名称列表

    QMap<int, QString> currentOutputPortLabelMap;
    QMap<int, QString> currentInputPortLabelMap;
    int      headsetCard; //headset 声卡
    gboolean hasHeadsetmic; //是否包含headset mic端口
    gboolean hasHeadphonemic; //是否包含headphone mic端口
    gboolean headsetPluggedIn; //headpset 是否插入
    char *m_pHeadphonesName; //headphone名称
    char *m_pHeadsetmicName; //
    char *m_pHeadphonemicName;
    char *m_pInternalspkName;
    char *m_pInternalmicName;

    QTimer *m_pTimer = nullptr;
    bool shortcutMute = false;
    bool firstEntry = true;

    QProcess *m_process;
    QString mThemeName = LINGMO_THEME_DEFAULT;
    bool firstEnterSystem = true;
    ca_context *caContext;
    bool firstLoad = true;

    bool isPlay = false; //用于避免运行托盘时第一次响起的提示音
    bool sinkInputMute = false;
    bool lingmoVideoMuteSignal = false;
    bool lingmoVideoVolumeSignal = false;
    bool isAppMuteBtnPress = false;
    bool isLoadEchoCancel = false;
    int outputListWidgetRow = -1;

    QString m_languageEnvStr;
    QString m_sessionControllerName;
    QString m_sessionPath;
    QString m_sessionControllerService ;
    QString m_sessionControllerPath;
    QString m_sessionControllerManagerInterface;
    QString m_sessionControllerSeatInterface ;
    QString m_sessionControllerSessionInterface ;
    QString m_sessionControllerActiveProperty ;
    QString m_sessionControllerPropertiesInterface;

    QDBusUnixFileDescriptor m_inhibitFileDescriptor;

    QGSettings *m_panelGSettings = nullptr;
    int m_panelPosition;
    int m_panelSize;

    SystemVersion m_version;
};

#endif // UKMEDIAMAINWIDGET_H
