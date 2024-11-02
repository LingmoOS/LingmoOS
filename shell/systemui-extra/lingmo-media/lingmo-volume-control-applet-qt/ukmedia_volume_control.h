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
#ifndef UKMEDIAVOLUMECONTROL_H
#define UKMEDIAVOLUMECONTROL_H

#include <pulse/ext-stream-restore.h>
#if HAVE_EXT_DEVICE_RESTORE_API
#  include <pulse/ext-device-restore.h>
#endif

#include <signal.h>
#include <string.h>
#include <glib.h>
#include <QObject>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-restore.h>
#include <pulse/ext-device-manager.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QLabel>
#include <QTimer>
#include <QDialog>

#include "config.h"
#include "custom_sound.h"
#include "../lingmo-login-sound/lingmo_login_sound_user_config.h"
#include "../common/ukmedia_common.h"
#include "lingmo_volume_control_user_config.h"

class PortInfo {
public:
      QByteArray name;
      QByteArray description;
      uint32_t priority;
      int available;
      int direction;
      int64_t latency_offset;
      std::vector<QByteArray> profiles;
};

class UkmediaVolumeControl : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.media")
public:
    UkmediaVolumeControl();
    virtual ~UkmediaVolumeControl();

public:
    void UkmediaDbusRegister();
    void insertJson(const JsonType& type, const QString& key, const QJsonValue& value);
    int boolToInt(bool b);
    void updateCard(UkmediaVolumeControl *c, const pa_card_info &info);

    QString findCardName(int index, QMap<int, QString> cardMap);
    int valueToPaVolume(int value);
    int paVolumeToValue(int volume);
    void initDefaultSinkVolume(const pa_sink_info &info);
    void initDefaultSourceVolume(const pa_source_info &info);

    bool updateSink(UkmediaVolumeControl *c, const pa_sink_info &info);
    void updateSource(const pa_source_info &info);
    void updateSinkInput(const pa_sink_input_info &info);
    void updateSourceOutput(const pa_source_output_info &info);
    void updateClient(const pa_client_info &info);
    void updateServer(const pa_server_info &info);

    void setConnectionState(gboolean connected);
    pa_stream* createMonitorStreamForSource(uint32_t source_idx, uint32_t stream_idx, bool suspend);
    void setIconFromProplist(QLabel *icon, pa_proplist *l, const char *name);

    //  Sink volume
    bool getSinkMute();
    int  getSinkVolume();
    void setSinkMute(bool status);
    void setSinkVolume(int index, int value);
    void setSinkVolumeByInfo(const pa_sink_info &info, int value);

    //  Source volume
    bool getSourceMute();
    int  getSourceVolume();
    void setSourceMute(bool status);
    void setSourceVolume(int index,int value);
    void setSourceVolumeByInfo(const pa_source_info &info, int value);

    //  Sink-input volume
    bool getSinkInputMuted(QString description);
    int  getSinkInputVolume(const gchar* name);
    bool setSinkInputMuted(QString name,bool status);
    void setSinkInputVolume(int index, int value, int channel);

    //  Source-output volume
    int  getSourceOutputVolume(const gchar *name);
    bool setSourceOutputMuted(QString appName, bool status);
    void setSourceOutputVolume(int index, int value, int channel);

    // sinkInfo
    sinkInfo addSinkInfo(const pa_sink_info& i);
    sinkInfo getSinkInfoByIndex(int index);
    sinkInfo getSinkInfoByName(QString name);

    // sourceInfo
    sourceInfo addSourceInfo(const pa_source_info& i);
    sourceInfo getSourceInfoByIndex(int index);
    sourceInfo getSourceInfoByName(QString name);

    void refreshVolume(int soundType, int info_Vol, bool info_Mute);

    void getModuleIndexByName(QString name);
    int getDefaultSinkIndex();
    bool setCardProfile(int index,const gchar *name); //设置声卡的配置文件
    bool setDefaultSink(const gchar *name); //设置默认的输出设备
    bool setDefaultSource(const gchar *name); //设置默认的输入设备
    bool setSinkPort(const gchar *sinkName,const gchar *name); //设置输出设备的端口
    QString stringRemoveUnrecignizedChar(QString str);//移除xml中不能识别的字符
    bool setSourcePort(const gchar *portName, const gchar *name); //设置输入设备的端口

    bool killSinkInput(int index); //kill 索引为index的sink input
//#if HAVE_EXT_DEVICE_RESTORE_API
////    void updateDeviceInfo(const pa_ext_device_restore_info &info);
//#endif

    void removeCard(uint32_t index);
    void removeSink(uint32_t index);
    void removeSource(uint32_t index);
    void removeSinkInput(uint32_t index);
    void removeSourceOutput(uint32_t index);
    void removeClient(uint32_t index);

    void setConnectingMessage(const char *string = NULL);

    void showError(const char *txt);
    static void decOutstanding(UkmediaVolumeControl *w);
    static void cardCb(pa_context *, const pa_card_info *i, int eol, void *userdata);
    static void bluetoothCardCb(pa_context *c, const pa_card_info *i, int eol, void *userdata);
    static void batteryLevelCb(pa_context *c, const pa_card_info *i, int eol, void *userdata);
    static void sinkIndexCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void sourceIndexCb(pa_context *c, const pa_source_info *i, int eol, void *userdata);

    static void simple_callback(pa_context *c, int success, void *userdata);
    static void sinkCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void sourceCb(pa_context *, const pa_source_info *i, int eol, void *userdata);
    static void sinkInputCb(pa_context *, const pa_sink_input_info *i, int eol, void *userdata);
    static void sinkInputCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata); //不更新sink input
    static void sourceOutputCb(pa_context *, const pa_source_output_info *i, int eol, void *userdata);
    static void clientCb(pa_context *, const pa_client_info *i, int eol, void *userdata);
    static void serverInfoCb(pa_context *, const pa_server_info *i, void *userdata);
    static void serverInfoIndexCb(pa_context *, const pa_server_info *i, void *userdata);
    static void extStreamRestoreReadCb(pa_context *,const pa_ext_stream_restore_info *i,int eol,void *userdata);
    static void extStreamRestoreSubscribeCb(pa_context *c, void *userdata);
//    void ext_device_restore_read_cb(pa_context *,const pa_ext_device_restore_info *i,int eol,void *userdata);
//    static void ext_device_restore_subscribe_cb(pa_context *c, pa_device_type_t type, uint32_t idx, void *userdata);
    static void extDeviceManagerReadCb(pa_context *,const pa_ext_device_manager_info *,int eol,void *userdata);
    static void extDeviceManagerSubscribeCb(pa_context *c, void *userdata);
    static void subscribeCb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata);
    static void contextStateCallback(pa_context *c, void *userdata);
    static void moduleInfoCb(pa_context *c, const pa_module_info *i, int eol, void *userdata);
    pa_context* getContext(void);
    static gboolean connectToPulse(gpointer userdata);

    void removeOutputPortMap(int index); //移除指定索引的output port
    void removeInputPortMap(int index); //移除指定索引的input port
    void removeCardMap(int index); //移除指定索引的 card
    void removeCardProfileMap(int index); //移除声卡profile map
    void removeSinkPortMap(int index);
    void removeSourcePortMap(int index);
    void removeProfileMap(int index);
    void removeInputProfile(int index);
    bool isExitOutputPort(QString name);
    int findOutputPort(QString name);
    QString findSinkActivePortName(QString name);
    bool isExitInputPort(QString name);
    void sendPortChangedSignal();
    void sendSourcePortChangedSignal();
    int findPortSourceIndex(QString name);
    int findPortSinkIndex(QString name);
    QString findSourcePortName(int cardIndex);
    QString findSinkPortName(int cardIndex);

    int findMasterDeviceCardIndex(QString masterDev);
    QString findMasterDeviceInfo(pa_proplist *proplist, const char *info_name); //开启降噪时，设置对应的sourcePortName与defaultinputcard
    QString findSinkMasterDeviceInfo(pa_proplist *proplist, const char *info_name);
    bool isNeedSendPortChangedSignal(QString newPort, QString prePort, QString cardName);

    void sendOsdWidgetSignal(QString portName, QString description);

    appInfo addSinkInputInfo(const pa_sink_input_info & info);
    appInfo addSourceOutputInfo(const pa_source_output_info& info);
    void moveSinkInput(QString appName, const char* sinkName);
    void moveSoureOutput(QString appName, const char* sourceName);
    QList<int> findAppIndex(int soundType, QString appName);
    QString findMasterDevice(int soundType, int index);
    const gchar* AppDesktopFileAdaption(const gchar* appName);

private:
    static void sendDeviceChangedSignal(UkmediaVolumeControl* w=nullptr);
    void sendVolumeUpdateSignal();
    void sendSourceVolumeUpdateSignal();
    void sendOSDDeviceChangedSignal(QString icon="");

Q_SIGNALS:
    void paContextReady();
    void updateSourceVolume(int value);
    void updateSourceMute(bool state);
    void checkDeviceSelectionSianal(const pa_card_info *info);
    void deviceChangedSignal();
    void removeSinkSignal();
    void device_changed_signal(QString str);
    void bluetoothBatteryChanged(QString dev, int battery);
    void initBlueDeviceVolumeSig(int index, QString name);

    void updatePortSignal();
    void updateMute(bool state);
    void updateVolume(int value);
    void removeSinkInputSignal(QString name);
    void sinkInputVolumeChangedSignal(QString name, QString id, int value);
    void updateApp(QString name);
    void addSinkInputSignal(QString name, QString id, int index, int value, int channel);

protected Q_SLOTS:
    void getBatteryLevel(QString dev);

public:
    static pa_context *context;
    static int reconnectTimeout;
    static pa_mainloop_api* api;
    QByteArray defaultSinkName, defaultSourceName;
    int n_outstanding = 0;
    bool canRenameDevices;
    int sinkIndex; //输出设备索引
    int sourceIndex; //输入设备索引
//    int sinkInputVolume;
    bool isLoadLoopback = false;
    int batteryLevel;

    //针对一个应用产生多个sinkinput，indexMap记录应用产生sinkinput的每个index, valueMap刷新应用的最新音量
    QMap<int, QString> sinkInputIndexMap;
    QMap<QString, int> sinkInputValueMap;
    QMap<int, QString> sourceOutputIndexMap;
    QMap<QString, int> sourceOutputValueMap;
    QMap<QString, int> sinkInputMuteMap;
    QStringList sinkInputList;

    QMap<int, appInfo> sinkInputMap;
    QMap<int, appInfo> sourceOutputMap;

    std::vector< std::pair<QByteArray,QByteArray> > profiles;
    std::map<QByteArray, PortInfo> ports;

    std::vector< std::pair<QByteArray,QByteArray> > dPorts;
    QByteArray activePort;
    QByteArray activeProfile;
    QByteArray noInOutProfile;
    QByteArray lastActiveProfile;

    const pa_sink_info *m_pDefaultSink = nullptr;
    const pa_source_info *m_pDefaultSource = nullptr;
    std::map<uint32_t, char*> clientNames;
    SinkInputType showSinkInputType;
    SinkType showSinkType;
    SourceOutputType showSourceOutputType;
    SourceType showSourceType;
    int sinkVolume = 0; //输出音量
    int sourceVolume = 0; //输入音量
    bool sinkMuted; //输出静音状态
    bool sourceMuted; //输入静音状态
    int sinkInputVolume; //sink input 音量
    bool sinkInputMuted; //sink input 静音状态

    float balance; //平衡音量值
    int channel = 0; //通道数
    int inputChannel = 0;
    int sinkInputChannel = 0;
    int sourceOutputChannel = 0;
    QString sourcePortName = ""; //输入设备端口名
    QString sinkPortName = ""; //输出设备端口名
    const gchar *sinkPortLabel; //输出设备端口名

    int defaultOutputCard;
    int defaultInputCard;
    QString findModuleStr = ""; //需要查找的module name
    int findModuleIndex = -1; //查找到模块的index
    QString masterDevice = "";
    QString masterSinkDev = "";

    pa_channel_map channelMap;
    friend class UkmediaMainWidget;

    QMap<int, sinkInfo> sinkMap; //输出设备
    QMap<int, sourceInfo> sourceMap; //输入设备

    QMap<int, QString> cardMap;
    QMap<int, QList<QString>> cardProfileMap;
    QMap<int,QString> cardActiveProfileMap;

    QMap<int,QMap<QString,QString>> sinkPortMap;
    QMap<QString,QString> sinkActivePortMap; //输出设备活跃端口
    QMap<int,QMap<QString,QString>> outputPortMap; //输出端口
    QMap<int,QMap<QString,QString>> inputPortMap; //输入端口
    QMap<int,QMap<QString,QString>> profileNameMap; //声卡输出配置文件
    QMap<int, QMap<QString,QString>>inputPortProfileNameMap; //声卡输入配置文件
    QMap<int, QMap<QString,int>> cardProfilePriorityMap; //记录声卡优先级配置文件
    QMap<int,QMap<QString,QString>> sourcePortMap;

private:
    void initUserConfig();

private:
    CustomSound *customSoundFile;
    gboolean m_connected;
    gchar* m_config_filename;
    int firstgo = true;
    int firstGoWithNoSink = true;
    QString m_description; // sink_description
    bool osdFirstFlag = true;

    //start 过滤设备端口切换或者设备拔插期间，subscribecb回调反馈的非预期数据
    static QTimer deviceChangedTimer;
    QTimer m_osdDeviceChangedTimer;
    QTimer m_updateVolumeTimer;
    QTimer m_updateSourceVolumeTimer;
    //end

    std::shared_ptr<UserConfig> m_pUsrConf;
};


#endif // UKMEDIAVOLUMECONTROL_H
