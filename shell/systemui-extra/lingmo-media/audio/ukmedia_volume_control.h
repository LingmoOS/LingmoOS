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
#ifndef UKMEDIAVOLUMECONTROL_H
#define UKMEDIAVOLUMECONTROL_H

#include <pulse/ext-stream-restore.h>
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
#include <QDialog>
#include "lingmo_custom_style.h"
#include "../common/ukmedia_common.h"
#include <QTimer>

#define NOISE_REDUCE_SOURCE "noiseReduceSource"
#define PANGUW_SINK "alsa_output.platform-raoliang-sndcard.analog-stereo"

#define DECAY_STEP .04

static int n_outstanding = 0;

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


class UkmediaVolumeControl : public QObject{
    Q_OBJECT
public:
    UkmediaVolumeControl();
    virtual ~UkmediaVolumeControl();
    friend class UkmediaMainWidget;

    void updateCard(UkmediaVolumeControl *c,const pa_card_info &info);
    bool updateSink(UkmediaVolumeControl *c,const pa_sink_info &info);
    void updateSource(const pa_source_info &info);
    void updateSinkInput(const pa_sink_input_info &info);
    void updateSourceOutput(const pa_source_output_info &info);
    void updateClient(const pa_client_info &info);
    void updateServer(const pa_server_info &info);
    void updateVolumeMeter(uint32_t source_index, uint32_t sink_input_index, double v);

    void setConnectionState(gboolean connected);
    void updateDeviceVisibility();
    void reallyUpdateDeviceVisibility();
    pa_stream* createMonitorStreamForSource(uint32_t source_idx, uint32_t stream_idx, bool suspend);
    void setIconFromProplist(QLabel *icon, pa_proplist *l, const char *name);

    //  Sink volume
    bool getSinkMute();
    int  getSinkVolume();
    void setSinkMute(bool status);
    void setSinkVolume(int index, int value);
    void setBalanceVolume(int index, int value, float balance);
    float getBalanceVolume();

    //  Source volume
    bool getSourceMute();
    int  getSourceVolume();
    void setSourceMute(bool status);
    void setSourceVolume(int index,int value);

    //  Sink-input volume
    int  getSinkInputVolume(const gchar *name);
    void setSinkInputMuted(int index,bool status);
    void setSinkInputVolume(int index,int value);

    //  Source-output volume
    int  getSourceOutputVolume(const gchar *name);
    void setSourceOutputMuted(int index, bool status);
    void setSourceOutputVolume(int index, int value);


    int getDefaultSinkIndex();
    bool setCardProfile(int index,const gchar *name); //设置声卡的配置文件
    bool setDefaultSink(const gchar *name); //设置默认的输出设备
    bool setDefaultSource(const gchar *name); //设置默认的输入设备
    bool setSinkPort(const gchar *sinkName ,const gchar *portName); //设置输出设备的端口
    bool setSourcePort(const gchar *sourceName, const gchar *portName); //设置输入设备的端口

    void removeCard(uint32_t index);
    void removeCardSink(int cardIndex,QString sinkName);
    void removeSink(uint32_t sinkIndex);
    void removeCardSource(int cardIndex,QString sourceName);
    void removeSource(uint32_t sourceIndex);
    void removeSinkInput(uint32_t index);
    void removeSourceOutput(uint32_t index);
    void removeClient(uint32_t index);

    void setConnectingMessage(const char *string = NULL);

    void showError(const char *txt);
    static void decOutstanding(UkmediaVolumeControl *w);
    static void readCallback(pa_stream *s, size_t length, void *userdata);
    static void sinkIndexCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void sourceIndexCb(pa_context *c, const pa_source_info *i, int eol, void *userdata);

    static void cardCb(pa_context *, const pa_card_info *i, int eol, void *userdata);
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
    pa_context* getContext(void);
    static gboolean connectToPulse(gpointer userdata);

    void removeOutputPortMap(int index); //移除指定索引的output port
    void removeInputPortMap(int index); //移除指定索引的input port
    void removeCardMap(int index); //移除指定索引的 card
    void removeCardProfileMap(int index); //移除声卡profile map
    void removeSinkPortMap(int index);
    void removeSourcePortMap(int index);
    void removeProfileMap(int index);
    bool isExitOutputPort(QString name);
    QString findSinkActivePortName(QString name);
    void removeInputProfile(int index);
    bool isExitInputPort(QString name);
    QString stringRemoveUnrecignizedChar(QString str);

    sinkInfo addSinkInfo(const pa_sink_info& i);
    sinkInfo getSinkInfoByIndex(int index);
    sinkInfo getSinkInfoByName(QString name);
    sourceInfo addSourceInfo(const pa_source_info& i);
    sourceInfo getSourceInfoByIndex(int index);
    sourceInfo getSourceInfoByName(QString name);
    void refreshVolume(int soundType, int info_Vol, bool info_Mute);

private:
    static void sendDeviceChangedSignal(UkmediaVolumeControl* w=nullptr);
    void sendVolumeUpdateSignal();
    void sendSourceVolumeUpdateSignal();

Q_SIGNALS:
    void paContextReady();
    void updateMonoAudio(bool show);
    void updateLoopBack(bool show);
    void updateSinkMute(bool isMute);
    void updateSourceMute(bool isMute);
    void updateVolume(int value,bool state);
    void updateSourceVolume(int value,bool state);
    void addSinkInputSignal(const gchar* name,const gchar *id,int index);
    void removeSinkInputSignal(const gchar* name);
    void addSourceOutputSignal(const gchar* name,const gchar *id,int index);
    void removeSourceOutputSignal(const gchar* name);
    void checkDeviceSelectionSianal(const pa_card_info *info);
    void peakChangedSignal(double v);
    void updatePortSignal();
    void updateCboxPortSignal();
    void deviceChangedSignal();


protected Q_SLOTS:

public:
    static pa_context *context;
    static int reconnectTimeout;
    static pa_mainloop_api* api;
    QByteArray defaultSinkName, defaultSourceName;

    bool canRenameDevices;
    const pa_server_info *m_pServerInfo;
    int sinkIndex;
    int sourceIndex;
    QStringList sinkInputList;
    QMap<QString, int> sinkInputMap;
    QMap<QString, int> sourceOutputMap;

    std::vector< std::pair<QByteArray,QByteArray> > profiles;
    std::map<QByteArray, PortInfo> ports;
    QByteArray activeProfile;
    QByteArray noInOutProfile;
    QByteArray lastActiveProfile;

    pa_cvolume m_defaultSinkVolume;
    const pa_sink_info *m_pDefaultSink;
    const pa_source_info *m_pDefaultSource;
    std::map<uint32_t, char*> clientNames;

    float balance;
    bool sinkMuted;
    bool sourceMuted;
    bool sinkInputMuted;
    int sinkVolume;
    int sourceVolume;
    int sinkInputVolume;
    int channel;
    int inputChannel;
    int sinkInputChannel;
    int sourceOutputChannel;
    int defaultOutputCard = -1;
    int defaultInputCard = -1;
    QString sinkPortName;
    QString sourcePortName;
    QString masterDevice = "";
    pa_channel_map channelMap;
    pa_channel_map defaultChannelMap;

    double lastPeak = 0;
    QByteArray name;
    QByteArray description;
    QByteArray activePort;
    uint32_t index, card_index;
    int peakDetectIndex = -1;
    pa_stream *peak = nullptr;
    pa_source_flags  sourceFlags;
    std::vector< std::pair<QByteArray,QByteArray> > dPorts;

    QMap<int, QString> cardMap;
    QMap<int,QString> cardActiveProfileMap;
    QMap<int, QList<QString>> cardProfileMap;
    QMap<int, QMap<QString,QString>> profileNameMap; //声卡输出配置文件
    QMap<int, QMap<QString,int>> cardProfilePriorityMap; //记录声卡优先级配置文件

    QMap<int, sinkInfo> sinkMap; //输出设备
    QMap<QString,QString> sinkActivePortMap; //输出设备活跃端口
    QMap<int,QMap<QString,QString>> sinkPortMap;
    QMap<int,QMap<QString,QString>> outputPortMap; //输出端口

    QMap<int, sourceInfo> sourceMap; //输入设备
    QMap<int,QMap<QString,QString>> inputPortMap; //输入端口
    QMap<int,QMap<QString,QString>> sourcePortMap;
    QMap<int, QMap<QString,QString>> inputPortProfileNameMap; //声卡输入配置文件

private:
    gboolean m_connected;
    gchar* m_config_filename;
    //start 过滤设备端口切换或者设备拔插期间，subscribecb回调反馈的非预期数据
    static QTimer deviceChangedTimer;
    QTimer m_updateVolumeTimer;
    QTimer m_updateSourceVolumeTimer;
    //end
};


#endif // UKMEDIAVOLUMECONTROL_H
