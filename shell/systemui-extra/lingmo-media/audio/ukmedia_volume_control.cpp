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
#include "ukmedia_volume_control.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <set>

#include <pulse/introspect.h>
#include <QIcon>
#include <QStyle>
#include <QLabel>
#include <QSettings>
#include <QDebug>
#include <QSlider>
#include <QTimer>
//pa_sink_info *m_pDefaultSink;
/* Used for profile sorting */


struct profile_prio_compare {
    bool operator() (pa_card_profile_info2 const * const lhs, pa_card_profile_info2 const * const rhs) const {

        if (lhs->priority == rhs->priority)
            return strcmp(lhs->name, rhs->name) > 0;

        return lhs->priority > rhs->priority;
    }
};

struct sink_port_prio_compare {
    bool operator() (const pa_sink_port_info& lhs, const pa_sink_port_info& rhs) const {

        if (lhs.priority == rhs.priority)
            return strcmp(lhs.name, rhs.name) > 0;

        return lhs.priority > rhs.priority;
    }
};

struct source_port_prio_compare {
    bool operator() (const pa_source_port_info& lhs, const pa_source_port_info& rhs) const {

        if (lhs.priority == rhs.priority)
            return strcmp(lhs.name, rhs.name) > 0;

        return lhs.priority > rhs.priority;
    }
};

pa_context* UkmediaVolumeControl::context = nullptr;
int UkmediaVolumeControl::reconnectTimeout = 3;
pa_mainloop_api* UkmediaVolumeControl::api = nullptr;
QTimer UkmediaVolumeControl::deviceChangedTimer;

UkmediaVolumeControl::UkmediaVolumeControl():
    canRenameDevices(false),
    m_connected(false),
    m_config_filename(nullptr) {

    profileNameMap.clear();
    connectToPulse(this);
}

/*
 *  设置输出设备静音
 */
void UkmediaVolumeControl::setSinkMute(bool status)
{
    pa_operation* o;
    if (!(o = pa_context_set_sink_mute_by_index(getContext(), sinkIndex, status, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_mute_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);
}

/*
 *  设置输出设备音量
 */
void UkmediaVolumeControl::setSinkVolume(int index, int value)
{
    if (nullptr == m_pDefaultSink)
    {
        qWarning() << "m_pDefaultSink is nullptr, set sink volume failed";
        return;
    }

    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = channel;

    for (int i = 0; i < v.channels; i++)
        v.values[i] = value;

    if (balance != 0)
        pa_cvolume_set_balance(&v, &channelMap, balance);

    /* To set the volume in silent state, unmute the volume first */
    if (sinkMuted)
        setSinkMute(false);

    pa_operation* o;
    if (!(o = pa_context_set_sink_volume_by_index(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);

    qDebug() << __func__ << "index" << value;
}

/*
 *  设置输入设备静音
 */
void UkmediaVolumeControl::setSourceMute(bool status)
{
    pa_operation* o;
    if (!(o = pa_context_set_source_mute_by_index(getContext(), sourceIndex, status, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_mute_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);
}


/*
 * 设置输入设备音量
 */
void UkmediaVolumeControl::setSourceVolume(int index, int value)
{
    pa_cvolume v = m_pDefaultSource->volume;
    v.channels = inputChannel;

    for (int i = 0; i < v.channels; i++)
        v.values[i] = value;

    if (sourceMuted)
        setSourceMute(false);

    pa_operation* o;
    if (!(o = pa_context_set_source_volume_by_index(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_volume_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);

    qDebug() << __func__ << index << value;
}

/*
 * 设置音量平衡值
 */
void UkmediaVolumeControl::setBalanceVolume(int index, int value, float b)
{
    if (nullptr == m_pDefaultSink)
    {
        qWarning() << "m_pDefaultSink is nullptr, set sink balance failed";
        return;
    }

    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = channel;

    for (int i = 0; i < v.channels; i++)
        v.values[i] = value;

    if (b != 0) {
        balance = b;
        pa_cvolume_set_balance(&v, &defaultChannelMap, balance);
    }

    pa_operation* o;
    if (!(o = pa_context_set_sink_volume_by_index(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);

    qDebug() << __func__ << index << value << balance;
}

/*
 * 获取输出设备的静音状态
 */
bool UkmediaVolumeControl::getSinkMute()
{
    return sinkMuted;
}

/*
 * 获取输出设备的音量值
 */
int UkmediaVolumeControl::getSinkVolume()
{
    return sinkVolume;
}

/*
 * 获取平衡音量
 */
float UkmediaVolumeControl::getBalanceVolume()
{
    return balance;
}

/*
 * 获取输入设备的静音状态
 */
bool UkmediaVolumeControl::getSourceMute()
{
    return sourceMuted;
}

/*
 * 获取输入设备的音量值
 */
int UkmediaVolumeControl::getSourceVolume()
{
    return sourceVolume;
}

/*
 * 获取默认的输出设备名字和输入设备的名字还有此时系统的输出的音量
 */
int UkmediaVolumeControl::getDefaultSinkIndex()
{
    pa_operation *o;
    if (!(o = pa_context_get_server_info(getContext(), serverInfoIndexCb, this))) {
        showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
        return -1;
    }
    pa_operation_unref(o);

    qDebug() << "getDefaultSinkIndex" << "defalutSinkName&defaultSourceName" << defaultSinkName << sinkVolume << defaultSourceName << sourceVolume;
    return sinkIndex;
}

/*
 *  滑动条更改设置sink input 音量值
 */
void UkmediaVolumeControl::setSinkInputVolume(int index, int value)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = sinkInputChannel;
    for (int i=0;i<v.channels;i++)
        v.values[i] = value;

    qDebug() << "set sink input volume " << index << v.channels << value;
    pa_operation* o;
    if (!(o = pa_context_set_sink_input_mute(getContext(), index,false, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
    }

    if (!(o = pa_context_set_sink_input_volume(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
    }
}

/*
 *  滑动条更改设置sink input静音状态
 */
void UkmediaVolumeControl::setSinkInputMuted(int index, bool status)
{
    qDebug() << "set sink input muted" << index << status;
    pa_operation* o;
    if (!(o = pa_context_set_sink_input_mute(getContext(), index,status, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
    }
}

/*
 *  滑动条更改设置source output 音量值
 */
void UkmediaVolumeControl::setSourceOutputVolume(int index, int value)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = sourceOutputChannel;
    for (int i=0;i<v.channels;i++)
        v.values[i] = value;

    qDebug() << "set source output volume " << index << v.channels << value;
    pa_operation* o;
    if (!(o = pa_context_set_source_output_mute(getContext(), index,false, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_output_volume() failed").toUtf8().constData());
    }

    if (!(o = pa_context_set_source_output_volume(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_output_volume() failed").toUtf8().constData());
    }
}

/*
 *  滑动条更改设置source output静音状态
 */
void UkmediaVolumeControl::setSourceOutputMuted(int index, bool status)
{
    qDebug() << "set source output muted" << index << status;
    pa_operation* o;
    if (!(o = pa_context_set_source_output_mute(getContext(), index,status, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_output_mute() failed").toUtf8().constData());
    }
}

/*
 * 设置声卡的配置文件
 */
bool UkmediaVolumeControl::setCardProfile(int index, const gchar *name)
{
    qDebug() << "setCardProfile" << index << name;
    pa_operation* o;
    if (!(o = pa_context_set_card_profile_by_index(getContext(), index, name, nullptr, nullptr))) {
        showError(tr("pa_context_set_card_profile_by_index() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 设置默认的输出设备
 */
bool UkmediaVolumeControl::setDefaultSink(const gchar *name)
{
    qDebug() << "setDefaultSink" << name;
    pa_operation* o;
    if (!(o = pa_context_set_default_sink(getContext(), name, nullptr, nullptr))) {
        showError(tr("pa_context_set_default_sink() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 设置默认的输入设备
 */
bool UkmediaVolumeControl::setDefaultSource(const gchar *name)
{
    pa_operation* o;
    if (!(o = pa_context_set_default_source(getContext(), name, nullptr, nullptr))) {
        showError(tr("pa_context_set_default_source() failed").toUtf8().constData());
        return false;
    }
    qDebug() << "setDefaultSource" << name << sourceIndex;

    return true;
}

/*
 * 设置输出设备的端口
 */
bool UkmediaVolumeControl::setSinkPort(const gchar *sinkName, const gchar *portName)
{
    qDebug() << "setSinkPort" << sinkName << portName;
    pa_operation* o;
    if (!(o = pa_context_set_sink_port_by_name(getContext(), sinkName, portName, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_port_by_name() failed").toUtf8().constData());
        return false;
    }
    return true;
}

/*
 * 设置输入设备端口
 */
bool UkmediaVolumeControl::setSourcePort(const gchar *sourceName, const gchar *portName)
{
    qDebug() << "setSourcePort" << sourceName << portName;
    pa_operation* o;
    if (!(o = pa_context_set_source_port_by_name(getContext(), sourceName, portName, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_port_by_name() failed").toUtf8().constData());
        return false;
    }
    return true;
}

UkmediaVolumeControl::~UkmediaVolumeControl() {

    while (!clientNames.empty()) {
        auto i = clientNames.begin();
        g_free(i->second);
        clientNames.erase(i);
    }
}

static void updatePorts(UkmediaVolumeControl *d, std::map<QByteArray, PortInfo> &ports) {
    std::map<QByteArray, PortInfo>::iterator it;
    PortInfo p;
    for (auto & port : d->dPorts) {
        QByteArray desc;
        it = ports.find(port.first);

        if (it == ports.end())
            continue;

        p = it->second;
        desc = p.description;

        if (p.available == PA_PORT_AVAILABLE_YES)
            desc +=  UkmediaVolumeControl::tr(" (plugged in)").toUtf8().constData();
        else if (p.available == PA_PORT_AVAILABLE_NO) {
            if (p.name == "analog-output-speaker" ||
                p.name == "analog-input-microphone-internal")
                desc += UkmediaVolumeControl::tr(" (unavailable)").toUtf8().constData();
            else
                desc += UkmediaVolumeControl::tr(" (unplugged)").toUtf8().constData();
        }

        port.second = desc;
        qDebug() << "updatePorts" << p.name << p.description;
    }

    Q_EMIT d->updatePortSignal();
    it = ports.find(d->activePort);

    if (it != ports.end()) {
        p = it->second;
//        d->setLatencyOffset(p.latency_offset);
    }
}

static void setIconByName(QLabel* label, const char* name) {
    QIcon icon = QIcon::fromTheme(name);
    int size = label->style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    QPixmap pix = icon.pixmap(size, size);
    label->setPixmap(pix);
}

void UkmediaVolumeControl::updateCard(UkmediaVolumeControl *c, const pa_card_info &info) {
    bool insertInputPort = false;
    bool insertOutputPort = false;
    bool is_new = false;
    const char *description;
    QMap<QString,QString> tempInput;
    QMap<QString,QString> tempOutput;
    QList<QString> profileName;
    QMap<QString,QString>portMap;
    QMap<QString,QString> outputPortNameLabelMap;
    QMap<QString,QString>inputPortNameLabelMap;
    QMap<QString,int> profilePriorityMap;
    std::set<pa_card_profile_info2 *, profile_prio_compare> profile_priorities;

    //  记录声卡配置文件优先级
    profile_priorities.clear();
    for (pa_card_profile_info2 **p_profile = info.profiles2; *p_profile != nullptr; ++p_profile)
    {
        profile_priorities.insert(*p_profile);
        profileName.append((*p_profile)->name);
        profilePriorityMap.insert((*p_profile)->name, (*p_profile)->priority);
    }
    cardProfilePriorityMap.insert(info.index, profilePriorityMap);

    description = pa_proplist_gets(info.proplist, PA_PROP_DEVICE_DESCRIPTION);

    c->ports.clear();
    for (uint32_t i = 0; i < info.n_ports; ++i) {
        PortInfo p;

        p.name = info.ports[i]->name;
        p.description = info.ports[i]->description;
        p.priority = info.ports[i]->priority;
        p.available = info.ports[i]->available;
        p.direction = info.ports[i]->direction;
        p.latency_offset = info.ports[i]->latency_offset;
        if (info.ports[i]->profiles2)
            for (pa_card_profile_info2 ** p_profile = info.ports[i]->profiles2; *p_profile != nullptr; ++p_profile) {
                p.profiles.push_back((*p_profile)->name);
            }
#ifdef PA_PORT_AVAILABLE_DISABLE
        if (p.direction == 1 && p.available != PA_PORT_AVAILABLE_NO && p.available != PA_PORT_AVAILABLE_DISABLE) {
            //            portMap.insertMulti(p.name,p.description.data());
            insertOutputPort = true;

            //新增UI设计，combobox显示portname+description
            QString outputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString outputPortName_and_description = outputPortName + "（" + description + "）";

            qDebug() << "add sink port name "<< info.index << p.name << p.description.data() << description ;
            tempOutput.insertMulti(p.name,outputPortName_and_description);

            QList<QString> portProfileName;
            for (auto p_profile : p.profiles) {
                portProfileName.append(p_profile.data());
                outputPortNameLabelMap.insertMulti(p.description.data(),p_profile.data());
                qDebug() << "ctf profilename map insert -----------" << p.description.data() << p_profile.data();
            }
            profileNameMap.insert(info.index,outputPortNameLabelMap);
            cardProfileMap.insert(info.index,portProfileName);
        }
        else if (p.direction == 2 && p.available != PA_PORT_AVAILABLE_NO && p.available != PA_PORT_AVAILABLE_DISABLE) {
            insertInputPort = true;

            //新增UI设计，combobox显示portname+description
            QString inputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString inputPortName_and_description = inputPortName + "（" + description + "）";


            qDebug() << "add source port name "<< info.index << p.name << p.description.data();
            tempInput.insertMulti(p.name,inputPortName_and_description);
            QList<QString> portProfileName;
            for (auto p_profile : p.profiles) {
                portProfileName.append(p_profile.data());
                inputPortNameLabelMap.insertMulti(p.description.data(),p_profile.data());
            }
            inputPortProfileNameMap.insert(info.index,inputPortNameLabelMap);
            cardProfileMap.insert(info.index,portProfileName);
        }
#else
        if (p.direction == 1 && p.available != PA_PORT_AVAILABLE_NO) {
            //            portMap.insertMulti(p.name,p.description.data());
            insertOutputPort = true;

            //新增UI设计，combobox显示portname+description
            QString outputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString outputPortName_and_description = outputPortName + "（" + description + "）";

            qDebug() << "add sink port name "<< info.index << p.name << p.description.data() << description ;
            tempOutput.insertMulti(p.name,outputPortName_and_description);

            QList<QString> portProfileName;
            for (auto p_profile : p.profiles) {
                portProfileName.append(p_profile.data());
                outputPortNameLabelMap.insertMulti(p.description.data(),p_profile.data());
                qDebug() << "ctf profilename map insert -----------" << p.description.data() << p_profile.data();
            }
            profileNameMap.insert(info.index,outputPortNameLabelMap);
            cardProfileMap.insert(info.index,portProfileName);
        }
        else if (p.direction == 2 && p.available != PA_PORT_AVAILABLE_NO) {
            insertInputPort = true;

            //新增UI设计，combobox显示portname+description
            QString inputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString inputPortName_and_description = inputPortName + "（" + description + "）";


            qDebug() << "add source port name "<< info.index << p.name << p.description.data();
            tempInput.insertMulti(p.name,inputPortName_and_description);
            QList<QString> portProfileName;
            for (auto p_profile : p.profiles) {
                portProfileName.append(p_profile.data());
                inputPortNameLabelMap.insertMulti(p.description.data(),p_profile.data());
            }
            inputPortProfileNameMap.insert(info.index,inputPortNameLabelMap);
            cardProfileMap.insert(info.index,portProfileName);
        }
#endif
        c->ports[p.name] = p;
    }
    if (insertInputPort) {
        inputPortMap.insert(info.index,tempInput);
    }
    else {
        inputPortMap.remove(info.index);
    }
    if (insertOutputPort) {
        outputPortMap.insert(info.index,tempOutput);
    }
    else {
        outputPortMap.remove(info.index);
    }
    cardActiveProfileMap.insert(info.index,info.active_profile->name);

    c->profiles.clear();
    for (auto p_profile : profile_priorities) {
        bool hasNo = false, hasOther = false;
        std::map<QByteArray, PortInfo>::iterator portIt;
        QByteArray desc = p_profile->description;

        for (portIt = c->ports.begin(); portIt != c->ports.end(); portIt++) {
            PortInfo port = portIt->second;

            if (std::find(port.profiles.begin(), port.profiles.end(), p_profile->name) == port.profiles.end())
                continue;

            if (port.available == PA_PORT_AVAILABLE_NO)
                hasNo = true;
            else {
                hasOther = true;
                break;
            }
        }
        if (hasNo && !hasOther)
            desc += tr(" (unplugged)").toUtf8().constData();

        if (!p_profile->available)
            desc += tr(" (unavailable)").toUtf8().constData();

        c->profiles.push_back(std::pair<QByteArray,QByteArray>(p_profile->name, desc));
        if (p_profile->n_sinks == 0 && p_profile->n_sources == 0)
            c->noInOutProfile = p_profile->name;
    }
    c->activeProfile = info.active_profile ? info.active_profile->name : "";

    /* Because the port info for sinks and sources is discontinued we need
     * to update the port info for them here. */
    updatePorts(c,c->ports);
    if (is_new) {
        updateDeviceVisibility();
    }

    Q_EMIT checkDeviceSelectionSianal(&info);
    //    c->updating = false;
}

/*
 * Update output device when the default output device or port is updated
 */
bool UkmediaVolumeControl::updateSink(UkmediaVolumeControl *w,const pa_sink_info &info) {
    bool is_new = false;
    m_defaultSinkVolume = info.volume;
    QMap<QString,QString>temp;
    int volume;
    if (info.volume.channels >= 2)
        volume = MAX(info.volume.values[0],info.volume.values[1]);
    else
        volume = info.volume.values[0];

    //默认的输出音量
    if (info.name && strcmp(defaultSinkName.data(),info.name) == 0) {
        //channel和sinkIndex在此处必须更新
        channel = info.volume.channels;
        sinkIndex= info.index;
        balance = pa_cvolume_get_balance(&info.volume,&info.channel_map);
        defaultChannelMap = info.channel_map;
        channelMap = info.channel_map;
        if (info.active_port) {
            sinkActivePortMap.insert(info.name,info.active_port->name);//huawei特殊处理
            if(strcmp(sinkPortName.toLatin1().data(),info.active_port->name) != 0) {
                sinkPortName = info.active_port->name;
                sendDeviceChangedSignal(this);
            }
            else {
                sinkPortName = info.active_port->name;
            }
        }
        defaultOutputCard = info.card;
/*
    bug:96232、95568、95523 解决 hw-panguw 机型，找不到对于sinkPortName导致音量无法同步、静音按钮只能按一次等问题
*/
        QString temp = "";
        if (sinkPortName.contains("histen-algo",Qt::CaseInsensitive)) {
            sinkPortName = findSinkActivePortName(PANGUW_SINK);
            temp = "histen-algo";
        }
        if (sinkVolume != volume || sinkMuted != info.mute)
        {
            if (temp != "") {
                sinkPortName = temp;
            }

            //  同步音量，同步UI
            w->refreshVolume(SoundType::SINK, volume, info.mute);
        }
    }

    if (info.ports) {
        for (pa_sink_port_info ** sinkPort = info.ports; *sinkPort != nullptr; ++sinkPort) {
            temp.insertMulti(info.name,(*sinkPort)->name);
        }
        QList <QMap<QString,QString>> sinkPortMapList;
        if(sinkPortMap.isEmpty())
            sinkPortMap.insertMulti(info.card,temp);
        sinkPortMapList = sinkPortMap.values();

        if(!sinkPortMapList.contains(temp))
            sinkPortMap.insertMulti(info.card,temp);

        qDebug() << "updateSink" << "defauleSinkName:" << defaultSinkName.data() << "sinkport" << sinkPortName << "sinkVolume" << sinkVolume ;

        if(strstr(defaultSinkName.data(),".headset_head_unit") || strstr(defaultSourceName.data(),"bt_sco_source"))
        {
            Q_EMIT updateMonoAudio(false);
            qDebug() << "Q_EMIT updateMonoAudio false" << sinkPortName << info.volume.channels;
        }
        else if(!strstr(defaultSourceName.data(),"bluez_source.") && !strstr(defaultSourceName.data(),"bt_sco_source")){
            Q_EMIT updateMonoAudio(true);
            qDebug() << "Q_EMIT updateMonoAudio true" << sinkPortName << info.volume.channels;
        }


        const char *icon;
        //    std::map<uint32_t, UkmediaCard*>::iterator cw;
        std::set<pa_sink_port_info,sink_port_prio_compare> port_priorities;


        port_priorities.clear();
        for (uint32_t i=0; i<info.n_ports; ++i) {
            port_priorities.insert(*info.ports[i]);
        }

        w->ports.clear();
    }else{
        qDebug() << " no sink port -> updateSink -> Q_EMIT updateMonoAudio true";
        Q_EMIT updateMonoAudio(true);
    }

    if (is_new)
        updateDeviceVisibility();
    return is_new;
}

QString UkmediaVolumeControl::findSinkActivePortName(QString name)
{
    QString portName = "";
    QMap<QString,QString>::iterator at;
    for (at=sinkActivePortMap.begin();at!=sinkActivePortMap.end();++at) {
        if (at.key() == name) {
            portName = at.value();
            break;
        }
    }
    return portName;
}

QString UkmediaVolumeControl::stringRemoveUnrecignizedChar(QString str) {
    str.remove(" ");
    str.remove("/");
    str.remove("(");
    str.remove(")");
    str.remove("[");
    str.remove("]");
    return str;
}


/*
 * stream suspend callback
 */
static void suspended_callback(pa_stream *s, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (pa_stream_is_suspended(s))
        w->updateVolumeMeter(pa_stream_get_device_index(s), PA_INVALID_INDEX, -1);
}

void UkmediaVolumeControl::readCallback(pa_stream *s, size_t length, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    const void *data;
    double v;
    int index;

    index = pa_stream_get_device_index(s);

    /* when the default input device is monitor
     * no input feedback is provided */
    if(strstr(w->defaultSourceName, ".monitor")) {
        Q_EMIT w->peakChangedSignal(0);
        return;
    }

    if (pa_stream_peek(s, &data, &length) < 0) {
        w->showError(UkmediaVolumeControl::tr("Failed to read data from stream").toUtf8().constData());
        return;
    }

    if (!data) {
        /* nullptr data means either a hole or empty buffer.
         * Only drop the stream when there is a hole (length > 0) */
        if (length)
            pa_stream_drop(s);
        return;
    }

    assert(length > 0);
    assert(length % sizeof(float) == 0);

    v = ((const float*) data)[length / sizeof(float) -1];

    pa_stream_drop(s);

    if (v < 0)
        v = 0;
    if (v > 1)
        v = 1;

    w->updateVolumeMeter(index,pa_stream_get_monitor_stream(s),v);
}

pa_stream* UkmediaVolumeControl::createMonitorStreamForSource(uint32_t source_idx, uint32_t stream_idx = -1, bool suspend = false) {
    pa_stream *s;
    char t[16];
    pa_buffer_attr attr;
    pa_sample_spec ss;
    pa_stream_flags_t flags;

    ss.channels = 1;
    ss.format = PA_SAMPLE_FLOAT32;
    ss.rate = 25;

    memset(&attr, 0, sizeof(attr));
    attr.fragsize = sizeof(float);
    attr.maxlength = (uint32_t) -1;

    snprintf(t, sizeof(t), "%u", source_idx);
    if (!(s = pa_stream_new(getContext(), tr("Peak detect").toUtf8().constData(), &ss, nullptr))) {
        showError(tr("Failed to create monitoring stream").toUtf8().constData());
        return nullptr;
    }

    if (stream_idx != (uint32_t) -1)
        pa_stream_set_monitor_stream(s, stream_idx);

    pa_stream_set_read_callback(s, readCallback, this);
    pa_stream_set_suspended_callback(s, suspended_callback, this);

    flags = (pa_stream_flags_t) (PA_STREAM_PEAK_DETECT | PA_STREAM_ADJUST_LATENCY |
                                 (suspend ? PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND : PA_STREAM_NOFLAGS));

    if (pa_stream_connect_record(s, t, &attr, flags) < 0) {
        showError(tr("Failed to connect monitoring stream").toUtf8().constData());
        pa_stream_unref(s);
        return nullptr;
    }
    return s;
}

void UkmediaVolumeControl::updateSource(const pa_source_info &info) {
    int volume = (info.volume.channels >= 2) ? MAX(info.volume.values[0], info.volume.values[1])
            : info.volume.values[0];

    //  Update some variables for the default input
    if (UKMedia_Equal(defaultSourceName.data(), info.name)) {
        sourceIndex = info.index;
        inputChannel = info.volume.channels;

        if (pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE)) {
            masterDevice = pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE);
            sourceInfo s = getSourceInfoByName(masterDevice);
            defaultInputCard = s.card;
            sourcePortName = s.active_port_name;
            qInfo() << "This is a filter source:" << info.name  << "master device:" << masterDevice;
        }
        else {
            defaultInputCard = info.card;
            sourcePortName = (info.active_port) ? info.active_port->name : "";
        }

        //bug#254769/256035 输入设备为内置mic或为空时，需要隐藏侦听功能（控制面板代码实现）并卸载侦听模块
        if(!strstr(sourcePortName.toUtf8().constData(), "internal") \
           && !strstr(sourcePortName.toUtf8().constData(), "[In] Mic1"))
        {
            updateLoopBack(true);
            qDebug() << "updateSource -> Q_EMIT updateLoopBack true" << sourcePortName;
        }else{
            updateLoopBack(false);
            qDebug() << "updateSource -> Q_EMIT updateLoopBack false" << sourcePortName;
        }

        sendDeviceChangedSignal(this);
        refreshVolume(SoundType::SOURCE, volume, info.mute);
        qInfo() << __func__ << "Status1 defaultSource:" << sourceIndex << defaultSourceName << "sourcePort" << sourcePortName;
    }
    /* 开启降噪状态下，如果这个source存在多个端口，且不同端口音量相同时，不会触发 noiseReduceSource 回调，导致无法更新 portName */
    else if (UKMedia_Equal(masterDevice.toLatin1().data(), info.name) && UKMedia_Equal(defaultSourceName.data(), ("noiseReduceSource"))) {
        defaultInputCard = info.card;
        sourcePortName = (info.active_port) ? info.active_port->name : "";
        sendDeviceChangedSignal(this);
        qInfo() << __func__ << "Status2 defaultSource:" << sourceIndex << defaultSourceName << "sourcePort" << sourcePortName;
    }

    if (info.ports) {
        QMap<QString,QString>temp;
        for (pa_source_port_info **sourcePort = info.ports; *sourcePort != nullptr; ++sourcePort)
            temp.insertMulti(info.name, (*sourcePort)->name);

        if(sourcePortMap.isEmpty())
            sourcePortMap.insertMulti(info.card, temp);

        QList<QMap<QString, QString>> sourcePortMapList;
        sourcePortMapList = sourcePortMap.values();

        if(!sourcePortMapList.contains(temp))
            sourcePortMap.insertMulti(info.card, temp);
    }
}


void UkmediaVolumeControl::setIconFromProplist(QLabel *icon, pa_proplist *l, const char *def) {
    const char *t;

    if ((t = pa_proplist_gets(l, PA_PROP_MEDIA_ICON_NAME)))
        goto finish;

    if ((t = pa_proplist_gets(l, PA_PROP_WINDOW_ICON_NAME)))
        goto finish;

    if ((t = pa_proplist_gets(l, PA_PROP_APPLICATION_ICON_NAME)))
        goto finish;

    if ((t = pa_proplist_gets(l, PA_PROP_MEDIA_ROLE))) {

        if (t && strcmp(t, "video") == 0 ||
            strcmp(t, "phone") == 0)
            goto finish;

        if (strcmp(t, "music") == 0) {
            t = "audio";
            goto finish;
        }

        if (strcmp(t, "game") == 0) {
            t = "applications-games";
            goto finish;
        }

        if (strcmp(t, "event") == 0) {
            t = "dialog-information";
            goto finish;
        }
    }

    t = def;

finish:

    setIconByName(icon, t);
}


void UkmediaVolumeControl::updateSinkInput(const pa_sink_input_info &info) {
    sinkInputChannel = info.volume.channels;
    const char *t;
    if ((t = pa_proplist_gets(info.proplist, "module-stream-restore.id"))) {
        if (t && strcmp(t, "sink-input-by-media-role:event") == 0) {
            g_debug("%s", tr("Ignoring sink-input due to it being designated as an event and thus handled by the Event widget").toUtf8().constData());
            return;
        }
    }
    const gchar *description = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_NAME);
    const gchar *appId = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID);
    //没制定应用名称的不加入到应用音量中
    if (description && !strstr(description,"QtPulseAudio")) {
        if (!info.corked) {

            sinkInputMap.insert(description,info.volume.values[0]);
            if (!sinkInputList.contains(description)) {
                sinkInputList.append(description);
                Q_EMIT addSinkInputSignal(description,appId,info.index);
            }
        }
        else {
            Q_EMIT removeSinkInputSignal(description);
            sinkInputList.removeAll(description);
            QMap<QString,int>::iterator it;
            for(it = sinkInputMap.begin();it!=sinkInputMap.end();)
            {
                if(it.key() == description)
                {
                    sinkInputMap.erase(it);
                    break;
                }
                ++it;
            }
        }
    }
}

void UkmediaVolumeControl::updateSourceOutput(const pa_source_output_info &info) {
    sourceOutputChannel = info.volume.channels;
    const char *app;
    //    bool is_new = false;

    if ((app = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID)))
        if (app && strcmp(app, "org.PulseAudio.pavucontrol") == 0
                || strcmp(app, "org.gnome.VolumeControl") == 0
                || strcmp(app, "org.kde.kmixd") == 0)
            return;

    const gchar *description = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_NAME);
    const gchar *appId = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID);

    //没制定应用名称的不加入到应用音量中
    if (description && !strstr(description,"QtPulseAudio")) {
        if (appId && !info.corked) {
            sourceOutputMap.insert(description,info.volume.values[0]);
            Q_EMIT addSourceOutputSignal(description,appId,info.index);
        }
        else {
            Q_EMIT removeSourceOutputSignal(description);
            QMap<QString,int>::iterator it;
            for(it = sourceOutputMap.begin();it!=sourceOutputMap.end();)
            {
                if(it.key() == description)
                {
                    sourceOutputMap.erase(it);
                    break;
                }
                ++it;
            }
        }
    }
}

void UkmediaVolumeControl::updateClient(const pa_client_info &info) {
    g_free(clientNames[info.index]);
    clientNames[info.index] = g_strdup(info.name);
}

void UkmediaVolumeControl::updateServer(const pa_server_info &info) {
    m_pServerInfo = &info;
    defaultSourceName = info.default_source_name ? info.default_source_name : "";
    defaultSinkName = info.default_sink_name ? info.default_sink_name : "";
    qDebug()  << "updateServer" << "default_sink:" << info.default_sink_name << "default_source:" << info.default_source_name;

    if (peak == nullptr && !strstr(defaultSourceName, ".monitor")) {
        QTimer::singleShot(100, this, [=](){
            peak = createMonitorStreamForSource(sourceIndex, -1, !!(sourceFlags & PA_SOURCE_NETWORK));
            qDebug() << "Created peak sourceIndex" << sourceIndex;
        });
    }
    else if (strstr(defaultSourceName, ".monitor")) {
        peak = nullptr;
        pa_operation *o;
        if (!(o = pa_context_kill_source_output(getContext(), peakDetectIndex, nullptr, nullptr)))
            showError(tr("pa_context_kill_source_output() failed").toUtf8().constData());
    }
}

void UkmediaVolumeControl::updateVolumeMeter(uint32_t index, uint32_t sinkInputIdx, double v)
{
    Q_UNUSED(index);
    Q_UNUSED(sinkInputIdx);
    if (lastPeak >= DECAY_STEP)
        if (v < lastPeak - DECAY_STEP)
            v = lastPeak - DECAY_STEP;

    lastPeak = v;

    Q_EMIT peakChangedSignal(v);
}

static guint idleSource = 0;

gboolean idleCb(gpointer data) {
    ((UkmediaVolumeControl*) data)->reallyUpdateDeviceVisibility();
    idleSource = 0;
    return FALSE;
}

void UkmediaVolumeControl::setConnectionState(gboolean connected) {
    if (m_connected != connected) {
        m_connected = connected;
        if (m_connected) {
//            connectingLabel->hide();
//            notebook->show();
        } else {
//            notebook->hide();
//            connectingLabel->show();
        }
    }
}

void UkmediaVolumeControl::updateDeviceVisibility() {

    if (idleSource)
        return;

    idleSource = g_idle_add(idleCb, this);
}

void UkmediaVolumeControl::reallyUpdateDeviceVisibility() {
    bool is_empty = true;

//    for (auto & sinkInputWidget : sinkInputWidgets) {
//        SinkInputWidget* w = sinkInputWidget.second;

//        if (sinkWidgets.size() > 1) {
//            w->directionLabel->show();
//            w->deviceButton->show();
//        } else {
//            w->directionLabel->hide();
//            w->deviceButton->hide();
//        }

//        if (showSinkInputType == SINK_INPUT_ALL || w->type == showSinkInputType) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (eventRoleWidget)
//        is_empty = false;

//    if (is_empty)
//        noStreamsLabel->show();
//    else
//        noStreamsLabel->hide();

    is_empty = true;

//    for (auto & sourceOutputWidget : sourceOutputWidgets) {
//        SourceOutputWidget* w = sourceOutputWidget.second;

//        if (sourceWidgets.size() > 1) {
//            w->directionLabel->show();
//            w->deviceButton->show();
//        } else {
//            w->directionLabel->hide();
//            w->deviceButton->hide();
//        }

//        if (showSourceOutputType == SOURCE_OUTPUT_ALL || w->type == showSourceOutputType) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (is_empty)
//        noRecsLabel->show();
//    else
//        noRecsLabel->hide();

//    is_empty = true;

//    for (auto & sinkWidget : sinkWidgets) {
//        SinkWidget* w = sinkWidget.second;

//        if (showSinkType == SINK_ALL || w->type == showSinkType) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (is_empty)
//        noSinksLabel->show();
//    else
//        noSinksLabel->hide();

//    is_empty = true;

//    for (auto & cardWidget : cardWidgets) {
//        CardWidget* w = cardWidget.second;

//        w->show();
//        is_empty = false;
//    }

//    if (is_empty)
//        noCardsLabel->show();
//    else
//        noCardsLabel->hide();

//    is_empty = true;

//    for (auto & sourceWidget : sourceWidgets) {
//        SourceWidget* w = sourceWidget.second;

//        if (showSourceType == SOURCE_ALL ||
//            w->type == showSourceType ||
//            (showSourceType == SOURCE_NO_MONITOR && w->type != SOURCE_MONITOR)) {
//            w->show();
//            is_empty = false;
//        } else
//            w->hide();
//    }

//    if (is_empty)
//        noSourcesLabel->show();
//    else
//        noSourcesLabel->hide();

//    /* Hmm, if I don't call hide()/show() here some widgets will never
//     * get their proper space allocated */
//    sinksVBox->hide();
//    sinksVBox->show();
//    sourcesVBox->hide();
//    sourcesVBox->show();
//    streamsVBox->hide();
//    streamsVBox->show();
//    recsVBox->hide();
//    recsVBox->show();
//    cardsVBox->hide();
//    cardsVBox->show();
}

void UkmediaVolumeControl::removeCard(uint32_t index) {
//    if (!cardWidgets.count(index))
//        return;

//    delete cardWidgets[index];
//    cardWidgets.erase(index);
    updateDeviceVisibility();
}

//移除对应声卡下的对应sink
void UkmediaVolumeControl::removeCardSink(int cardIndex,QString sinkName)
{
    QMap<int,QMap<QString,QString>>::iterator it1;
    QMap<QString,QString>::iterator it2;

    for(it1=sinkPortMap.begin();it1!=sinkPortMap.end();)
    {
        //1、找到对应声卡
        if(it1.key()==cardIndex)
        {
            for(it2=it1.value().begin();it2!=it1.value().end();)
            {
                //2、找到对应声卡下的对应sink,进行移除
                if(it2.key()==sinkName)
                {
                    it2 = it1->erase(it2);
                    return;
                }
                ++it2;
            }
        }
        ++it1;
    }
}

void UkmediaVolumeControl::removeSink(uint32_t sinkIndex)
{
#if 1
    QMap<int, sinkInfo>::iterator it;

    for(it=sinkMap.begin(); it!=sinkMap.end();)
    {
        if (it.key() == sinkIndex) {
            removeCardSink(it.value().card, it.value().name);
            if(m_pDefaultSink->volume.channels >= 2 && strstr(it.value().name.toLatin1().data(),".a2dp_sink")\
                    && !strstr(defaultSourceName.data(),"bluez_source.") && !strstr(defaultSourceName.data(),"bt_sco_source"))
            {
                Q_EMIT updateMonoAudio(true);
                qDebug() << "Q_EMIT updateMonoAudio true" << sinkPortName << m_pDefaultSink->volume.channels << it.value().name;
            }
            sinkMap.erase(it);
            break;
        }
        ++it;
    }
    updateDeviceVisibility();

#else
    QMap<int,QMap<int, sinkInfo>>::iterator it1;
    QMap<int, sinkInfo>::iterator it2;


    for (it1=sinkMap.begin(); it1!=sinkMap.end();)
    {
        if (it1.key() == index) {
            for(it2=it1.value().begin();it2!=it1.value().end();)
            {
                removeCardSink(it2.key(),it2.value().name);
                if(m_pDefaultSink->volume.channels >= 2 && strstr(it2.value().name.toLatin1().data(),".a2dp_sink")\
                        && !strstr(defaultSourceName.data(),"bluez_source.") && !strstr(defaultSourceName.data(),"bt_sco_source"))
                {
                    Q_EMIT updateMonoAudio(true);
                    qDebug() << "Q_EMIT updateMonoAudio true" << sinkPortName << m_pDefaultSink->volume.channels << it2.value().name;
                }
                ++it2;
            }
            sinkMap.erase(it1);
            break;
        }
        ++it1;
    }
    updateDeviceVisibility();
 #endif
}

//移除对应声卡下的对应source
void UkmediaVolumeControl::removeCardSource(int cardIndex,QString sourceName)
{
    QMap<int,QMap<QString,QString>>::iterator it1;
    QMap<QString,QString>::iterator it2;

    for(it1=sourcePortMap.begin();it1!=sourcePortMap.end();)
    {
        //1、找到对应声卡
        if(it1.key()==cardIndex)
        {
            for(it2=it1.value().begin();it2!=it1.value().end();)
            {
                //2、找到对应声卡下的对应sink,进行移除
                if(it2.key()==sourceName)
                {
                    it2 = it1->erase(it2);
                    return;
                }
                ++it2;
            }
        }
        ++it1;
    }
}

void UkmediaVolumeControl::removeSource(uint32_t sourceIndex)
{
#if 1
    QMap<int, sourceInfo>::iterator it;

    for (it=sourceMap.begin(); it!=sourceMap.end();)
    {
        if (it.key() == sourceIndex) {
            removeCardSource(it.value().card, it.value().name);

            sourceMap.erase(it);
            break;
        }
        ++it;
    }
    updateDeviceVisibility();

#else

    QMap<int,QMap<int, sourceInfo>>::iterator it1;
    QMap<int, sourceInfo>::iterator it2;

    for (it1=sourceMap.begin();it1!=sourceMap.end();)
    {
        if (it1.key() == index) {
            for(it2=it1.value().begin();it2!=it1.value().end();)
            {
                //it2.key()=cardIndex,it2.value()=sourceName,找到对应声卡下的对应source进行删除
                removeCardSource(it2.key(),it2.value().name);
                ++it2;
            }
            sourceMap.erase(it1);
            break;
        }
        ++it1;
    }
    updateDeviceVisibility();
#endif
}

void UkmediaVolumeControl::removeSinkInput(uint32_t index) {
    updateDeviceVisibility();
}

void UkmediaVolumeControl::removeSourceOutput(uint32_t index) {
    updateDeviceVisibility();
}

void UkmediaVolumeControl::removeClient(uint32_t index) {
    g_free(clientNames[index]);
    clientNames.erase(index);
}


void UkmediaVolumeControl::setConnectingMessage(const char *string) {
    QByteArray markup = "<i>";
    if (!string)
        markup += tr("Establishing connection to PulseAudio. Please wait...").toUtf8().constData();
    else
        markup += string;
    markup += "</i>";
//    connectingLabel->setText(QString::fromUtf8(markup));
}

void UkmediaVolumeControl::showError(const char *txt)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s: %s", txt, pa_strerror(pa_context_errno(context)));

    qWarning() << QString::fromUtf8(buf);
}

void UkmediaVolumeControl::decOutstanding(UkmediaVolumeControl *w) {
//    qDebug() << "decOutstanding---------";
    if (n_outstanding <= 0)
        return;

    if (--n_outstanding <= 0) {
        // w->get_window()->set_cursor();
        w->setConnectionState(true);
    }
}

void UkmediaVolumeControl::cardCb(pa_context *c, const pa_card_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Card callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }

    w->cardMap.insert(i->index,i->name);
    w->updateCard(w,*i);
}

void UkmediaVolumeControl::sinkIndexCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Sink callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    int volume;
    if(i->volume.channels >= 2)
        volume = MAX(i->volume.values[0],i->volume.values[1]);
    else
        volume = i->volume.values[0];
    w->channel = i->volume.channels;
    w->defaultOutputCard = i->card;
    w->sinkIndex = i->index;
    w->balance = pa_cvolume_get_balance(&i->volume,&i->channel_map);

    if(i->active_port)
        w->sinkPortName = i->active_port->name;
    else
        w->sinkPortName = "";

    //  同步音量，同步UI
    w->refreshVolume(SoundType::SINK, volume, i->mute);
}

void UkmediaVolumeControl::sourceIndexCb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Source callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    int volume;
    if(i->volume.channels >= 2)
        volume = MAX(i->volume.values[0],i->volume.values[1]);
    else
        volume = i->volume.values[0];

    w->inputChannel = i->volume.channels;
    w->sourceIndex = i->index;

    if (pa_proplist_gets(i->proplist, PA_PROP_DEVICE_MASTER_DEVICE)) {
        w->masterDevice = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_MASTER_DEVICE);
        sourceInfo s = w->getSourceInfoByName(w->masterDevice);
        w->defaultInputCard = s.card;
        w->sourcePortName = s.active_port_name;
    }
    else {
        w->defaultInputCard = i->card;
        w->sourcePortName = (i->active_port) ? i->active_port->name : "";
    }

    w->refreshVolume(SoundType::SOURCE, volume, i->mute);
    qDebug() << __func__ << "defaultInputCard" << w->defaultInputCard << "sourcePort" << w->sourcePortName;
}

void UkmediaVolumeControl::sinkCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Sink callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    w->m_pDefaultSink = i;
    qDebug() << "SinkCb" << i->index << i->name;

#if 1
    w->sinkMap.insert(i->index, w->addSinkInfo(*i));
    w->updateSink(w,*i);
#else
    QMap<int, sinkInfo> temp;
    temp.insert(i->card,w->addSinkInfo(*i));

    w->sinkMap.insert(i->index,temp);
    w->updateSink(w,*i);
#endif
}

void UkmediaVolumeControl::sourceCb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Source callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }

    w->m_pDefaultSource = i;
    qInfo() << "sourceCb" << i->index << i->name;

    w->sourceMap.insert(i->index, w->addSourceInfo(*i));
    w->updateSource(*i);
}

void UkmediaVolumeControl::sinkInputCb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Sink input callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }

    w->updateSinkInput(*i);
}

void UkmediaVolumeControl::sourceOutputCb(pa_context *c, const pa_source_output_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Source output callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0)  {

        if (n_outstanding > 0) {
            /* At this point all notebook pages have been populated, so
             * let's open one that isn't empty */
            //            if (default_tab != -1) {
            //                if (default_tab < 1 || default_tab > w->notebook->count()) {
            //                    if (!w->sinkInputWidgets.empty())
            //                        w->notebook->setCurrentIndex(0);
            //                    else if (!w->sourceOutputWidgets.empty())
            //                        w->notebook->setCurrentIndex(1);
            //                    else if (!w->sourceWidgets.empty() && w->sinkWidgets.empty())
            //                        w->notebook->setCurrentIndex(3);
            //                    else
            //                        w->notebook->setCurrentIndex(2);
            //                } else {
            //                    w->notebook->setCurrentIndex(default_tab - 1);
            //                }
            //                default_tab = -1;
            //            }
        }

        decOutstanding(w);
        return;
    }

    if (i->name && strstr(i->name, "Peak detect"))
        w->peakDetectIndex = i->index;

    qDebug() << __func__ << i->index;

    w->updateSourceOutput(*i);
}

void UkmediaVolumeControl::clientCb(pa_context *c, const pa_client_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Client callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
//    qDebug() << "clientCb" << i->name;
    w->updateClient(*i);
}

void UkmediaVolumeControl::serverInfoIndexCb(pa_context *, const pa_server_info *i, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (!i) {
        w->showError(QObject::tr("Server info callback failure").toUtf8().constData());
        return;
    }
    pa_operation *o;

    qDebug() << "serverInfoIndexCb" << i->default_sink_name << i->default_source_name;
    w->updateServer(*i);
    decOutstanding(w);
}

void UkmediaVolumeControl::serverInfoCb(pa_context *, const pa_server_info *i, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (!i) {
        w->showError(QObject::tr("Server info callback failure").toUtf8().constData());
        return;
    }

    pa_operation *o;
    //默认的输出设备改变时需要获取默认的输出音量
    if(!(o = pa_context_get_sink_info_by_name(w->getContext(),i->default_sink_name,sinkIndexCb,w))) {

    }

    if(!(o = pa_context_get_source_info_by_name(w->getContext(),i->default_source_name,sourceIndexCb,w))) {

    }
    qDebug() << "serverInfoCb" << i->user_name << i->default_sink_name << i->default_source_name;
    w->updateServer(*i);
    sendDeviceChangedSignal(w);
    decOutstanding(w);
}

void UkmediaVolumeControl::extStreamRestoreReadCb(
        pa_context *c,
        const pa_ext_stream_restore_info *i,
        int eol,
        void *userdata) {

    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        decOutstanding(w);
        g_debug(QObject::tr("Failed to initialize stream_restore extension: %s").toUtf8().constData(), pa_strerror(pa_context_errno(c)));
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
//    qDebug() << "extStreamRestoreReadCb" << i->name;
//    w->updateRole(*i);
}

void UkmediaVolumeControl::extStreamRestoreSubscribeCb(pa_context *c, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    pa_operation *o;

    if (!(o = pa_ext_stream_restore_read(c, extStreamRestoreReadCb, w))) {
        w->showError(QObject::tr("pa_ext_stream_restore_read() failed").toUtf8().constData());
        return;
    }
    qDebug() << "extStreamRestoreSubscribeCb" ;
    pa_operation_unref(o);
}

void UkmediaVolumeControl::extDeviceManagerReadCb(
        pa_context *c,
        const pa_ext_device_manager_info *,
        int eol,
        void *userdata) {

    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        decOutstanding(w);
        g_debug(QObject::tr("Failed to initialize device manager extension: %s").toUtf8().constData(), pa_strerror(pa_context_errno(c)));
        return;
    }

    w->canRenameDevices = true;

    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    qDebug() << "extDeviceManagerReadCb";
    /* Do something with a widget when this part is written */
}

void UkmediaVolumeControl::extDeviceManagerSubscribeCb(pa_context *c, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    pa_operation *o;

    if (!(o = pa_ext_device_manager_read(c, extDeviceManagerReadCb, w))) {
        w->showError(QObject::tr("pa_ext_device_manager_read() failed").toUtf8().constData());
        return;
    }
    qDebug() << "extDeviceManagerSubscribeCb";
    pa_operation_unref(o);
}

void UkmediaVolumeControl::subscribeCb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSink(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_info_by_index(c, index, sinkCb, w))) {
                    w->showError(QObject::tr("pa_context_get_sink_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSource(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_source_info_by_index(c, index, sourceCb, w))) {
                    w->showError(QObject::tr("pa_context_get_source_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSinkInput(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_input_info(c, index, sinkInputCb, w))) {
                    w->showError(QObject::tr("pa_context_get_sink_input_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeSourceOutput(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_source_output_info(c, index, sourceOutputCb, w))) {
                    w->showError(QObject::tr("pa_context_get_sink_input_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_CLIENT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
                w->removeClient(index);
            else {
                pa_operation *o;
                if (!(o = pa_context_get_client_info(c, index, clientCb, w))) {
                    w->showError(QObject::tr("pa_context_get_client_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SERVER: {
                pa_operation *o;
                if (!(o = pa_context_get_server_info(c, serverInfoCb, w))) {
                    w->showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_CARD:
            //remove card
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                qDebug() << "remove cards------";
                //移除outputPort
                w->removeSinkPortMap(index);
                w->removeSourcePortMap(index);
                w->removeOutputPortMap(index);
                w->removeInputPortMap(index);

                Q_EMIT w->updatePortSignal();

                w->removeCardMap(index);
                w->removeCardProfileMap(index);

                w->removeProfileMap(index);
                w->removeInputProfile(index);
                w->removeCard(index);

            }
            else {
                pa_operation *o;
                if (!(o = pa_context_get_card_info_by_index(c, index, cardCb, w))) {
                    w->showError(QObject::tr("pa_context_get_card_info_by_index() failed").toUtf8().constData());
                    return;
                }
                pa_operation_unref(o);
            }
            break;

    }
}

void UkmediaVolumeControl::contextStateCallback(pa_context *c, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    g_assert(c);

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:

            break;

        case PA_CONTEXT_READY: {
            pa_operation *o;
            qDebug() << "pa_context_get_state" << "PA_CONTEXT_READY" << pa_context_get_state(c);
            reconnectTimeout = 3;

            pa_context_set_subscribe_callback(c, subscribeCb, w);

            if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                           (PA_SUBSCRIPTION_MASK_SINK|
                                            PA_SUBSCRIPTION_MASK_SOURCE|
                                            PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                            PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                            PA_SUBSCRIPTION_MASK_CLIENT|
                                            PA_SUBSCRIPTION_MASK_SERVER|
                                            PA_SUBSCRIPTION_MASK_CARD), nullptr, nullptr))) {
                w->showError(QObject::tr("pa_context_subscribe() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);

            /* Keep track of the outstanding callbacks for UI tweaks */
            n_outstanding = 0;

            if (!(o = pa_context_get_server_info(c, serverInfoCb, w))) {
                w->showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_client_info_list(c, clientCb, w))) {
                w->showError(QObject::tr("pa_context_client_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_card_info_list(c, cardCb, w))) {
                w->showError(QObject::tr("pa_context_get_card_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_sink_info_list(c, sinkCb, w))) {
                w->showError(QObject::tr("pa_context_get_sink_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_source_info_list(c, sourceCb, w))) {
                w->showError(QObject::tr("pa_context_get_source_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_sink_input_info_list(c, sinkInputCb, w))) {
                w->showError(QObject::tr("pa_context_get_sink_input_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            if (!(o = pa_context_get_source_output_info_list(c, sourceOutputCb, w))) {
                w->showError(QObject::tr("pa_context_get_source_output_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            n_outstanding++;

            Q_EMIT w->paContextReady();
            break;
        }

        case PA_CONTEXT_FAILED:
            w->setConnectionState(false);
            w->updateDeviceVisibility();
            pa_context_unref(w->context);
            w->context = nullptr;

            qWarning("Connection failed, attempting reconnect");
            reconnectTimeout--;
            if (reconnectTimeout <= 0) {
                qWarning("reconnect pulseaudio Three times failed");
                return;
            }

            g_timeout_add_seconds(5, connectToPulse, w);
            return;

        case PA_CONTEXT_TERMINATED:
        default:
            return;
    }
}

pa_context* UkmediaVolumeControl::getContext(void) {
    g_assert(context);
    return context;
}

gboolean UkmediaVolumeControl::connectToPulse(gpointer userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (context) {
        qWarning("pulseAudio is connected");
        return false;
    }

    pa_glib_mainloop *m = pa_glib_mainloop_new(g_main_context_default());
    api = pa_glib_mainloop_get_api(m);
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, QObject::tr("LingmoUI Media Volume Control").toUtf8().constData());
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.PulseAudio.pavucontrol");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "PACKAGE_VERSION");
    context = pa_context_new_with_proplist(api, nullptr, proplist);
    g_assert(context);

    pa_proplist_free(proplist);
    pa_context_set_state_callback(context, contextStateCallback, w);
    if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        if (pa_context_errno(context) == PA_ERR_INVALID) {
            /*w->setConnectingMessage(QObject::tr("Connection to PulseAudio failed. Automatic retry in 5s\n\n"
                "In this case this is likely because PULSE_SERVER in the Environment/X11 Root Window Properties\n"
                "or default-server in client.conf is misconfigured.\n"
                "This situation can also arrise when PulseAudio crashed and left stale details in the X11 Root Window.\n"
                "If this is the case, then PulseAudio should autospawn again, or if this is not configured you should\n"
                "run start-pulseaudio-x11 manually.").toUtf8().constData());*/
            qFatal("connect pulseaudio failed");
        }
        else {
            reconnectTimeout--;
            if (reconnectTimeout <= 0) {
                qWarning("reconnect pulseaudio Three times failed");
                return false;
            }

            g_timeout_add_seconds(5, connectToPulse, w);
        }
    }

    return false;
}


/*
 * 根据名称获取sink input音量
 */
int UkmediaVolumeControl::getSinkInputVolume(const gchar *name)
{
    QMap<QString,int>::iterator it;
    int value = 0;
    for(it = sinkInputMap.begin();it!=sinkInputMap.end();)
    {
        if(it.key() == name)
        {
            qDebug() << "getSinkInputVolume" << "name:" <<name << "it.key" <<it.key() << "it.value" <<it.value();
            value = it.value();
            return value;
        }
        ++it;
    }
    return value;
}

/*
 * 根据名称获取source output音量
 */
int UkmediaVolumeControl::getSourceOutputVolume(const gchar *name)
{
    QMap<QString,int>::iterator it;
    int value = 0;
    for(it = sourceOutputMap.begin();it!=sourceOutputMap.end();)
    {
        if(it.key() == name)
        {
            qDebug() << "getSourceOutputVolume" << "name:" <<name << "it.key" <<it.key() << "it.value" <<it.value();
            value = it.value();
            return value;
        }
        ++it;
    }
    return value;
}


void UkmediaVolumeControl::sinkInputCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("Sink input callback failure").toUtf8().constData());
        return;
    }
    if (eol > 0) {
        decOutstanding(w);
        return;
    }
    w->sinkInputMuted = i->mute;
    if (i->volume.channels >= 2)
        w->sinkInputVolume = MAX(i->volume.values[0],i->volume.values[1]);
    else
        w->sinkInputVolume = i->volume.values[0];
    qDebug() << "sinkInputCallback" <<w->sinkInputVolume ;
}

/*
 * 移除指定索引的output port
 */
void UkmediaVolumeControl::removeOutputPortMap(int index)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    for (it=outputPortMap.begin();it!=outputPortMap.end();) {
        if (it.key() == index) {
            qDebug() << "removeoutputport" <<it.key() << it.value();
            outputPortMap.erase(it);
            break;
        }
        ++it;
    }
}

/*
 * 移除指定索引的input port
 */
void UkmediaVolumeControl::removeInputPortMap(int index)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    for (it=inputPortMap.begin();it!=inputPortMap.end();) {
        if (it.key() == index) {
            inputPortMap.erase(it);
            break;
        }
        ++it;
    }
}

/*
 * 移除指定索引的card
 */
void UkmediaVolumeControl::removeCardMap(int index)
{
    QMap<int, QString>::iterator it;
    for (it=cardMap.begin();it!=cardMap.end();) {
        if (it.key() == index) {
            cardMap.erase(it);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeCardProfileMap(int index)
{
    QMap<int, QList<QString>>::iterator it;
    QMap<int, QMap<QString,int>>::iterator at;
    for (it=cardProfileMap.begin();it!=cardProfileMap.end();) {
        if (it.key() == index) {
            cardProfileMap.erase(it);
            break;
        }
        ++it;
    }

    for (at=cardProfilePriorityMap.begin();at!=cardProfilePriorityMap.cend();) {
        if (at.key() == index) {
            cardProfilePriorityMap.erase(at);
            if(cardProfilePriorityMap.keys().contains(index))
                cardProfilePriorityMap.remove(index);
            break;
        }
        ++at;
    }
}

void UkmediaVolumeControl::removeSinkPortMap(int index)
{
    qDebug() << "removeSinkPortMap///";
    QMap<int,QMap<QString,QString>>::iterator it;
    for(it=sinkPortMap.begin();it!=sinkPortMap.end();){
        if(it.key() == index) {
            sinkPortMap.remove(index);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeSourcePortMap(int index)
{
    qDebug() << "removeSinkPortMap///";
    QMap<int,QMap<QString,QString>>::iterator it;
    for(it=sourcePortMap.begin();it!=sourcePortMap.end();){
        if(it.key() == index) {
            sourcePortMap.erase(it);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeProfileMap(int index)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    qDebug() << "removeProfileMap" << index << profileNameMap;

    for (it=profileNameMap.begin();it!=profileNameMap.end();) {
        if(it.key() == index){
            profileNameMap.erase(it);
            break;
        }
        ++it;
    }
}

bool UkmediaVolumeControl::isExitOutputPort(QString name)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for (it=outputPortMap.begin();it!=outputPortMap.end();) {
        portMap = it.value();
        for (at=portMap.begin();at!=portMap.end();) {
            if (at.value() == name)
                return true;
            ++at;
        }
        ++it;
    }
    return false;
}

void UkmediaVolumeControl::removeInputProfile(int index)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    qDebug() << "removeInputProfile" << index << inputPortProfileNameMap;

    for (it=inputPortProfileNameMap.begin();it!=inputPortProfileNameMap.end();) {
        if(it.key() == index){
            inputPortProfileNameMap.erase(it);
            break;
        }
        ++it;
    }
}

bool UkmediaVolumeControl::isExitInputPort(QString name)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for (it=inputPortMap.begin();it!=inputPortMap.end();) {
        portMap = it.value();
        for (at=portMap.begin();at!=portMap.end();) {
            if (at.value() == name)
                return true;
            ++at;
        }
        ++it;
    }
    return false;
}

sinkInfo UkmediaVolumeControl::addSinkInfo(const pa_sink_info& i)
{
    sinkInfo info;
    info.name = i.name;
    info.index = i.index;
    info.description = i.description;
    info.volume = i.volume;
    info.mute = i.mute;
    info.card = i.card;

    if (i.active_port) {
        info.active_port_name = i.active_port->name;
        info.active_port_description = i.active_port->description;
    }

    if (pa_proplist_gets(i.proplist, PA_PROP_DEVICE_MASTER_DEVICE))
        info.master_device = pa_proplist_gets(i.proplist, PA_PROP_DEVICE_MASTER_DEVICE);

    if (i.ports) {
        for (pa_sink_port_info** sinkPort = i.ports; *sinkPort != nullptr; ++sinkPort) {
            portInfo pInfo;
            pInfo.name = (*sinkPort)->name;
            pInfo.description = (*sinkPort)->description;
            pInfo.priority = (*sinkPort)->priority;
            pInfo.available = (*sinkPort)->available;
            info.sink_port_list.append(pInfo);
        }
    }
    return info;
}

sourceInfo UkmediaVolumeControl::addSourceInfo(const pa_source_info& i)
{
    sourceInfo info;
    info.name = i.name;
    info.index = i.index;
    info.description = i.description;
    info.volume = i.volume;
    info.mute = i.mute;
    info.card = i.card;

    if (i.active_port) {
        info.active_port_name = i.active_port->name;
        info.active_port_description = i.active_port->description;
    }

    if(pa_proplist_gets(i.proplist, PA_PROP_DEVICE_MASTER_DEVICE))
        info.master_device = pa_proplist_gets(i.proplist, PA_PROP_DEVICE_MASTER_DEVICE);

    return info;
}

/**
 * @brief UkmediaVolumeControl::refreshVolume
 * 发送信号刷新音量
 */
void UkmediaVolumeControl::refreshVolume(int soundType, int info_Vol, bool info_Mute)
{
    switch (soundType) {
    case SoundType::SINK: {
        if (sinkMuted != info_Mute) {
            //需求31268待2501再合入
#if 1
            if (outputPortMap.isEmpty() || defaultSinkName.isEmpty()) {
                sinkMuted = true;
            } else {
                sinkMuted = info_Mute;
            }
#elif 0
            sinkMuted = info_Mute;
#endif
            Q_EMIT updateSinkMute(sinkMuted);
        }

        if (sinkVolume != info_Vol) {
            //需求31268待2501再合入
#if 1
            if (outputPortMap.isEmpty() || defaultSinkName.isEmpty()) {
                sinkVolume = 0;
                sinkMuted = true;
            } else {
                sinkVolume = info_Vol;
                sinkMuted = info_Mute;
            }
#elif 0
            sinkVolume = info_Vol;
            sinkMuted = info_Mute;
#endif
            sendVolumeUpdateSignal();
        }
    }
        break;
    case SoundType::SOURCE: {
        if (sourceMuted != info_Mute) {
            //需求31268待2501再合入
#if 1
            if (inputPortMap.isEmpty() || defaultSourceName.isEmpty()) {
                sourceMuted = true;
            } else {
                sourceMuted = info_Mute;
            }
#elif 0
            sourceMuted = info_Mute;
#endif
            Q_EMIT updateSourceMute(sourceMuted);
        }

        if (sourceVolume != info_Vol) {
            //需求31268待2501再合入
#if 1
            if (inputPortMap.isEmpty() || defaultSourceName.isEmpty()) {
                sourceVolume = 0;
                sourceMuted = true;
            } else {
                sourceVolume = info_Vol;
                sourceMuted = info_Mute;
            }
#elif 0
            sourceVolume = info_Vol;
            sourceMuted = info_Mute;
#endif

            sendSourceVolumeUpdateSignal();
        }
    }
        break;
    default:
        break;
    }
}

void UkmediaVolumeControl::sendDeviceChangedSignal(UkmediaVolumeControl* w)
{
    //start 过滤设备端口切换或者设备拔插期间，subscribecb回调反馈的非预期数据
    if (nullptr == w) {
        qDebug() << "w is null";
        return;
    }

    static UkmediaVolumeControl* control = w;
    static bool isConnect = false;
    if (deviceChangedTimer.isActive()) {
        deviceChangedTimer.stop();
    }
    deviceChangedTimer.setInterval(300);
    deviceChangedTimer.setSingleShot(true);
    if (!isConnect) {
        isConnect = QObject::connect(&deviceChangedTimer, &QTimer::timeout, control, [&control](){
            qDebug() << "deviceChangedSignal";
            if (control) {
                Q_EMIT control->deviceChangedSignal();
            }
        });
    }
    deviceChangedTimer.start();
    //end
}

void UkmediaVolumeControl::sendVolumeUpdateSignal()
{
    //start 过滤设备端口切换或者设备拔插期间，subscribecb回调反馈的非预期数据
    static bool isConnect = false;
    if (m_updateVolumeTimer.isActive()) {
        m_updateVolumeTimer.stop();
    }
    m_updateVolumeTimer.setInterval(150);
    m_updateVolumeTimer.setSingleShot(true);
    if (!isConnect) {
        isConnect = QObject::connect(&m_updateVolumeTimer, &QTimer::timeout, this, [&](){
            qDebug() << "refreshVolume sinkVolume:" << sinkVolume;
            Q_EMIT this->updateVolume(sinkVolume, sinkMuted);
        });
    }
    m_updateVolumeTimer.start();
    //end
}

void UkmediaVolumeControl::sendSourceVolumeUpdateSignal()
{
    //start 过滤设备端口切换或者设备拔插期间，subscribecb回调反馈的非预期数据
    static bool isConnect = false;
    if (m_updateSourceVolumeTimer.isActive()) {
        m_updateSourceVolumeTimer.stop();
    }
    m_updateSourceVolumeTimer.setInterval(150);
    m_updateSourceVolumeTimer.setSingleShot(true);
    if (!isConnect) {
        isConnect = QObject::connect(&m_updateSourceVolumeTimer, &QTimer::timeout, this, [&](){
            qDebug() << "refreshVolume sourceVolume:" << sourceVolume;
            Q_EMIT this->updateSourceVolume(sourceVolume, sourceMuted);
        });
    }
    m_updateSourceVolumeTimer.start();
    //end
}

sinkInfo UkmediaVolumeControl::getSinkInfoByIndex(int index)
{
    QMap<int, sinkInfo>::iterator it;

    for (it = sinkMap.begin(); it != sinkMap.end(); ++it)
    {
        if (index == it.key())
            return it.value();
    }
    qInfo() << "Can't find sink info by index" << index;
}

sinkInfo UkmediaVolumeControl::getSinkInfoByName(QString name)
{
    QMap<int, sinkInfo>::iterator it;

    for (it = sinkMap.begin(); it != sinkMap.end(); ++it)
    {
        if (name == it.value().name)
            return it.value();
    }
    qInfo() << "Can't find sink info by name" << name;
}

sourceInfo UkmediaVolumeControl::getSourceInfoByIndex(int index)
{
    QMap<int, sourceInfo>::iterator it;

    for (it = sourceMap.begin(); it != sourceMap.end(); ++it)
    {
        if (index == it.key())
            return it.value();
    }
    qInfo() << "Can't find source info by index" << index;
}

sourceInfo UkmediaVolumeControl::getSourceInfoByName(QString name)
{
    QMap<int, sourceInfo>::iterator it;

    for (it = sourceMap.begin(); it != sourceMap.end(); ++it)
    {
        if (name == it.value().name)
            return it.value();
    }
    qInfo() << "Can't find source info by name" << name;
}
