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

#include "ukmedia_volume_control.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <QEventLoop>
#include <set>
#include <pulse/introspect.h>
#include <QIcon>
#include <QStyle>
#include <QLabel>
#include <QSettings>
#include <QDebug>
#include <QSlider>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <QSettings>
#include <alsa/asoundlib.h>
#include <QGSettings>

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

UkmediaVolumeControl::UkmediaVolumeControl() :
    showSinkInputType(SINK_INPUT_CLIENT),
    showSinkType(SINK_ALL),
    showSourceOutputType(SOURCE_OUTPUT_CLIENT),
    showSourceType(SOURCE_NO_MONITOR),
    canRenameDevices(false),
    m_connected(false),
    m_config_filename(nullptr)
{
    profileNameMap.clear();
    //创建声音初始化记录文件
    customSoundFile = new CustomSound();
    customSoundFile->createAudioFile();
    initUserConfig();
    UkmediaDbusRegister();

    //start pulseaudio和lingmo-volume进程启动时间差小于3s时，会捕获到pulseaudio初始化设备时的非预期数据导致出现设备切换动作
    QTimer::singleShot(3*1000, [this]() {
        connectToPulse(this);
    });
    //end
}

void UkmediaVolumeControl::initUserConfig()
{
    m_pUsrConf = std::make_shared<LingmoUIVolumeControlUserConfig>();
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
 *  设置输出设备音量通过info
 */
void UkmediaVolumeControl::setSinkVolumeByInfo(const pa_sink_info &info, int value)
{
    pa_cvolume v = info.volume;
    for (int i = 0; i < v.channels; i++)
        v.values[i] = value;

    if (balance != 0)
        pa_cvolume_set_balance(&v,&channelMap,balance);

    pa_operation* o;
    if (!(o = pa_context_set_sink_mute_by_index(getContext(), info.index, false, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_mute_by_index() failed").toUtf8().constData());
        return;
    }

    if (!(o = pa_context_set_sink_volume_by_index(getContext(), info.index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);

    qDebug() << __func__ << info.index << value;
}

void UkmediaVolumeControl::setSourceVolumeByInfo(const pa_source_info &info,int value)
{
    pa_cvolume v = info.volume;
    for (int i = 0; i < v.channels; i++)
        v.values[i] = value;

    if (balance != 0)
        pa_cvolume_set_balance(&v, &channelMap, balance);

    pa_operation* o;
    if (!(o = pa_context_set_source_mute_by_index(getContext(), info.index,false, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_mute_by_index() failed").toUtf8().constData());
        return;
    }

    if (!(o = pa_context_set_source_volume_by_index(getContext(), info.index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_volume_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);

    qDebug() << __func__ << info.index << value;
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
    if (nullptr == m_pDefaultSource)
    {
        qWarning() << "m_pDefaultSource is nullptr, set source volume failed";
        return;
    }

    pa_cvolume v = m_pDefaultSource->volume;
    v.channels = inputChannel;

    for (int i = 0; i < v.channels; i++)
        v.values[i] = value;

    /* To set the volume in silent state, unmute the volume first */
    if (sourceMuted)
        setSourceMute(false);

    pa_operation* o;
    if (!(o = pa_context_set_source_volume_by_index(getContext(), index, &v, nullptr, nullptr))) {
        showError(tr("pa_context_set_source_volume_by_index() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);

    qDebug() << __func__ << "index" << value;
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

int UkmediaVolumeControl::getDefaultSinkIndex()
{
    pa_operation *o;
    if (!(o = pa_context_get_server_info(getContext(), serverInfoIndexCb, this))) {
        showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
        return -1;
    }
    pa_operation_unref(o);

    return sinkIndex;
}

/*
 * 根据名称获取sink input静音状态
 */
bool UkmediaVolumeControl::getSinkInputMuted(QString description)
{
    int muteStatus = 1;

    QMap<QString,int>::iterator it;
    for (it=sinkInputMuteMap.begin();it!=sinkInputMuteMap.end();) {
        qDebug() << "getSinkInputMuted" << it.key() << description << it.value();
        if (it.key() == description) {
            muteStatus = it.value();
            break;
        }
        ++it;
    }

    return muteStatus==1?true:false;
}

/*
 * 根据模块名称获取对应模块索引
 */
void UkmediaVolumeControl::getModuleIndexByName(QString name)
{
    findModuleStr = name;
    pa_context_get_module_info_list(getContext(), moduleInfoCb,this);
}

/*
 *  滑动条更改设置sink input 音量值
 */
void UkmediaVolumeControl::setSinkInputVolume(int index, int value,int channel)
{
    pa_cvolume v = m_pDefaultSink->volume;

    v.channels = channel;
    for (int i=0;i<v.channels;i++)
        v.values[i] = value;

    qDebug() << "set sink input volume" << index << v.channels << value;

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
bool UkmediaVolumeControl::setSinkInputMuted(QString appName, bool status)
{
    QList<int> appIndexList = findAppIndex(SoundType::SINK, appName);

    if (appIndexList.isEmpty())
        return false;

    for (int index : appIndexList) {
        pa_operation* o;
        if (!(o = pa_context_set_sink_input_mute(getContext(), index, status, nullptr, nullptr))) {
            showError(tr("pa_context_set_sink_volume_by_index() failed").toUtf8().constData());
        }
        pa_operation_unref(o);
        qDebug() << "set sink input muted" << index << status;
    }

    return true;
}

void UkmediaVolumeControl::moveSinkInput(QString appName, const char* sinkName)
{
    QList<int> appIndexList = findAppIndex(SoundType::SINK, appName);

    if (appIndexList.isEmpty())
        return;

    for (int index : appIndexList) {
        pa_operation* o;

        if (!(o = pa_context_move_sink_input_by_name(getContext(), index, sinkName, simple_callback, nullptr))) {
            showError(tr("pa_context_move_sink_input_by_index() failed").toUtf8().constData());
        }

        qDebug() << __func__ << "app" << index << appName << "sink:" << sinkName;
        pa_operation_unref(o);
    }
}

void UkmediaVolumeControl::moveSoureOutput(QString appName, const char* sourceName)
{
    QList<int> appIndexList = findAppIndex(SoundType::SOURCE, appName);

    if (appIndexList.isEmpty())
        return;

    for (int index : appIndexList) {
        pa_operation* o;

        if (!(o = pa_context_move_source_output_by_name(getContext(), index, sourceName, simple_callback, nullptr)))
            showError(tr("pa_context_move_source_output_by_name() failed").toUtf8().constData());

        pa_operation_unref(o);
        qDebug() << __func__ << "app" << index << appName << "source:" << sourceName;
    }
}

/*
 *  滑动条更改设置source output 音量值
 */
void UkmediaVolumeControl::setSourceOutputVolume(int index, int value, int channel)
{
    pa_cvolume v = m_pDefaultSink->volume;
    v.channels = channel;
    for (int i=0;i<v.channels;i++)
        v.values[i] = value;

    qDebug() << "set source output volume" << index << v.channels << value;

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
bool UkmediaVolumeControl::setSourceOutputMuted(QString appName, bool status)
{
    QList<int> appIndexList = findAppIndex(SoundType::SOURCE, appName);

    if (appIndexList.isEmpty())
        return false;

    for (int index : appIndexList) {
        pa_operation* o;

        if (!(o = pa_context_set_source_output_mute(getContext(), index, status, nullptr, nullptr)))
            showError(tr("pa_context_set_source_output_mute() failed").toUtf8().constData());

        pa_operation_unref(o);
        qDebug() << "set source output muted" << index << status;
    }

    return true;
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
    bool monoAudioState = false;

    QGSettings *m_pSoundSettings = new QGSettings(KEY_SOUNDS_SCHEMA);
    monoAudioState = m_pSoundSettings->get(MONO_AUDIO).toBool();

    pa_operation* o;
    if (!(o = pa_context_set_default_sink(getContext(), name, nullptr, nullptr))) {
        showError(tr("pa_context_set_default_sink() failed").toUtf8().constData());
        return false;
    }
#if 0
    if(monoAudioState)
    {
        if (!(o = pa_context_set_default_sink(getContext(), "mono", nullptr, nullptr))) {
            showError(tr("pa_context_set_default_sink() failed").toUtf8().constData());
            return false;
        }
    }
#endif
    qDebug() << " setDefaultSink  " << name << defaultSinkName << monoAudioState;
    return true;
}

/*
 * 设置默认的输入设备
 */
bool UkmediaVolumeControl::setDefaultSource(const gchar *name)
{
    qDebug() << "setDefaultSource" << name;

    pa_operation* o;
    if (!(o = pa_context_set_default_source(getContext(), name, nullptr, nullptr))) {
        showError(tr("pa_context_set_default_source() failed").toUtf8().constData());
        return false;
    }

    return true;
}

/*
 * 设置输出设备的端口
 */
bool UkmediaVolumeControl::setSinkPort(const gchar *sinkName, const gchar *portName)
{
    qDebug() << "setSinkPort" << "sinkname:" << sinkName << "portname:" << portName;

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

/*
 * kill 索引为index 的sink input
 */
bool UkmediaVolumeControl::killSinkInput(int index)
{
    pa_operation* o;
    if (!(o = pa_context_kill_sink_input(getContext(), index, nullptr, nullptr))) {
        showError(tr("pa_context_kill_sink_input() failed").toUtf8().constData());
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

        if (it == ports.end()) {
            continue;
        }

        p = it->second;
        desc = p.description;

        if (p.available == PA_PORT_AVAILABLE_YES) {
            desc +=  UkmediaVolumeControl::tr(" (plugged in)").toUtf8().constData();
        } else if (p.available == PA_PORT_AVAILABLE_NO) {
            if (p.name == "analog-output-speaker" ||
                    p.name == "analog-input-microphone-internal") {
                desc += UkmediaVolumeControl::tr(" (unavailable)").toUtf8().constData();
            } else {
                desc += UkmediaVolumeControl::tr(" (unplugged)").toUtf8().constData();
            }
        }
        port.second = desc;
    }
    qDebug() << "updatePorts" << p.name << p.description;

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

int UkmediaVolumeControl::boolToInt(bool b)
{
    if (b == true)
        return 1;
    else
        return 0;
}

void UkmediaVolumeControl::updateCard(UkmediaVolumeControl *c, const pa_card_info &info) {
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
        if (info.ports[i]->profiles2)//bug#92426 该蓝牙音响存在输入端口无输入配置文件会导致段错误
            for (pa_card_profile_info2 ** p_profile = info.ports[i]->profiles2; *p_profile != nullptr; ++p_profile) {
                p.profiles.push_back((*p_profile)->name);
            }
#ifdef PA_PORT_AVAILABLE_DISABLE
        if (p.direction == 1 && p.available != PA_PORT_AVAILABLE_NO && p.available != PA_PORT_AVAILABLE_DISABLE) {
            //portMap.insertMulti(p.name,p.description.data());
            //新增UI设计，combobox显示portname+description
            QString outputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString outputPortName_and_description = outputPortName + "（" + description + "）";

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
            qDebug() << " add source port name "<< info.index << p.name << p.description.data();
            QString inputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString inputPortName_and_description = inputPortName + "（" + description + "）";

            tempInput.insertMulti(p.name,inputPortName_and_description);

            for (auto p_profile : p.profiles) {
                inputPortNameLabelMap.insertMulti(p.description.data(),p_profile.data());
            }
            inputPortProfileNameMap.insert(info.index,inputPortNameLabelMap);
        }
#else
        if (p.direction == 1 && p.available != PA_PORT_AVAILABLE_NO) {
            //portMap.insertMulti(p.name,p.description.data());
            //新增UI设计，combobox显示portname+description
            QString outputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString outputPortName_and_description = outputPortName + "（" + description + "）";

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
            qDebug() << " add source port name "<< info.index << p.name << p.description.data();
            QString inputPortName = p.description.data();//端口名(如：扬声器,模拟耳机..)
            QString inputPortName_and_description = inputPortName + "（" + description + "）";

            tempInput.insertMulti(p.name,inputPortName_and_description);

            for (auto p_profile : p.profiles) {
                inputPortNameLabelMap.insertMulti(p.description.data(),p_profile.data());
            }
            inputPortProfileNameMap.insert(info.index,inputPortNameLabelMap);
        }
#endif
        c->ports[p.name] = p;
    }
    inputPortMap.insert(info.index,tempInput);
    outputPortMap.insert(info.index,tempOutput);
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

            if (port.available == PA_PORT_AVAILABLE_NO) {
                hasNo = true;
            } else {
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

    if (strstr(info.name, "bluez_card")) {
        getBatteryLevel(info.name);
    }

    /* Because the port info for sinks and sources is discontinued we need
     * to update the port info for them here. */
    updatePorts(c,c->ports);

    Q_EMIT checkDeviceSelectionSianal(&info);
}

/*
    获取声卡名，参考aplay -L
*/

QString card_get(int card)
{
    snd_ctl_t *handle;
    int  err, dev, idx;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    char name[32];
    sprintf(name, "hw:%d", card);
    if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
        qInfo("control open (%i): %s", card, snd_strerror(err));
        return "Error";
    }
    if ((err = snd_ctl_card_info(handle, info)) < 0) {
        qInfo("control hardware info (%i): %s", card, snd_strerror(err));
        snd_ctl_close(handle);
        return "Error";
    }
    dev = -1;
    while (1) {
        unsigned int count;
        if (dev < 0)
            break;
        snd_pcm_info_set_device(pcminfo, dev);
        snd_pcm_info_set_subdevice(pcminfo, 0);
        snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
        if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
//                    if (err != -ENOENT)
//                        error("control digital audio info (%i): %s", card, snd_strerror(err));
            continue;
        }
        qDebug() <<"card_get"<< card << snd_ctl_card_info_get_id(info);

        count = snd_pcm_info_get_subdevices_count(pcminfo);
        printf(("  Subdevices: %i/%i\n"),
            snd_pcm_info_get_subdevices_avail(pcminfo), count);
        for (idx = 0; idx < (int)count; idx++) {
            snd_pcm_info_set_subdevice(pcminfo, idx);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                qInfo("control digital audio playback info (%i): %s", card, snd_strerror(err));
            } else {
                printf(("  Subdevice #%i: %s\n"),
                    idx, snd_pcm_info_get_subdevice_name(pcminfo));
            }
        }
    }
    snd_ctl_close(handle);

    return snd_ctl_card_info_get_id(info);
}

void UkmediaVolumeControl::insertJson(const JsonType &type, const QString &key, const QJsonValue &value)
{
    switch (type) {
    case JsonType::JSON_TYPE_SOUNDEFFECT:
//        m_pUsrConf->insert(m_pUsrConf->getJsonByType(std::dynamic_pointer_cast<LingmoUILoginSoundUserConfig>(m_pUsrConf)->getJsonMap(), type),
//                           key, value);
        break;
    case JsonType::JSON_TYPE_USERINFO:
        m_pUsrConf->insert(m_pUsrConf->getJsonByType(std::dynamic_pointer_cast<LingmoUIVolumeControlUserConfig>(m_pUsrConf)->getJsonMap(), type),
                           key, value);
        break;
    default:
        break;
    }

}

/*
 * 滑动条值转换成音量值
 */
int UkmediaVolumeControl::valueToPaVolume(int value)
{
    if (value > UKMEDIA_VOLUME_NORMAL) {
        value = UKMEDIA_VOLUME_NORMAL;
    }
    else if (value < UKMEDIA_VOLUME_MIN) {
        value = UKMEDIA_VOLUME_MIN;
    }

    return value / UKMEDIA_VOLUME_NORMAL * PA_VOLUME_NORMAL;
}

/*
 * 音量值转换成滑动条值
 */
int UkmediaVolumeControl::paVolumeToValue(int value)
{
    if (value > PA_VOLUME_NORMAL) {
        value = PA_VOLUME_NORMAL;
    }
    else if (value < PA_VOLUME_MIN) {
        value = PA_VOLUME_MIN;
    }

    return (value / PA_VOLUME_NORMAL * UKMEDIA_VOLUME_NORMAL) + 0.5;
}

/*
 * 根据声卡索引查找声卡名
 */
QString UkmediaVolumeControl::findCardName(int index,QMap<int,QString> cardMap)
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
 * 初始化默认输出音量
 */
void UkmediaVolumeControl::initDefaultSinkVolume(const pa_sink_info &info)
{
    QString active_port_name = info.active_port ? info.active_port->name : "";
    if (strcmp(info.name, "auto_null") == 0 && strcmp(defaultSinkName, "auto_null") == 0)
        active_port_name = "auto_null_sink";

    if (active_port_name.isEmpty() || active_port_name.contains("histen")
            || customSoundFile->isExist(stringRemoveUnrecignizedChar(active_port_name)))
        return;

    if (strstr(info.name, "bluez")) {
        if (defaultSinkName.contains("bluez")) {
            Q_EMIT initBlueDeviceVolumeSig(info.index, active_port_name);
        }
        return;
    }

    int volume;
    QString outputCardName = findCardName(defaultOutputCard, cardMap);
    if (active_port_name.contains("headphone", Qt::CaseInsensitive)) {
        volume = (outputCardName.contains("bluez", Qt::CaseSensitive)) ? OUTPUT_VOLUME : HEADPHONE_VOLUME;
    }
    else if (active_port_name.contains("hdmi", Qt::CaseInsensitive)) {
        volume = HDMI_VOLUME;
    }
    else if (active_port_name.contains("speaker", Qt::CaseInsensitive)) {
        volume = OUTPUT_VOLUME;
    }
    else if (active_port_name.contains("lineout", Qt::CaseInsensitive)) {
        volume = HEADPHONE_VOLUME;
    }
    else {
        volume = OUTPUT_VOLUME;
    }

    setSinkVolumeByInfo(info, valueToPaVolume(volume));
    customSoundFile->addXmlNode(active_port_name, false);
    qDebug() << "initDefaultSinkVolume" << active_port_name << volume;
}

/*
 * Update output device when the default output device or port is updated
 */
bool UkmediaVolumeControl::updateSink(UkmediaVolumeControl *w,const pa_sink_info &info) {
    bool is_new = false;
    QString cardStr ="hw:CARD=";

    int volume = (info.volume.channels >= 2) ? MAX(info.volume.values[0], info.volume.values[1])
            : info.volume.values[0];

    if (info.active_port) {
        sinkActivePortMap.insert(info.name, info.active_port->name);
    }

    if (pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE)) {
        masterSinkDev = pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE);
    }

    //  Initialize the output volume
    initDefaultSinkVolume(info);

    if (strcmp(defaultSinkName.data(), info.name) == 0) {
        channel = info.volume.channels;
        channelMap = info.channel_map;
        sinkIndex = info.index;
        QString portName = "";
        int cardId = 0;

        if (info.active_port) {
            portName = info.active_port->name;

            //获取当前声卡名
            if(pa_proplist_gets(info.proplist, "alsa.card") && pa_proplist_gets(info.proplist, "alsa.device"))
            {
                cardId = atoi(pa_proplist_gets(info.proplist, "alsa.card"));
                cardStr += card_get(atoi(pa_proplist_gets(info.proplist, "alsa.card")));
                cardStr += ",DEV=";
                cardStr += pa_proplist_gets(info.proplist, "alsa.device");
            }
        } else {
            portName = "";
        }

        //  是否发送音乐暂停信号
        if (isNeedSendPortChangedSignal(portName, sinkPortName, info.name)) {
            sendPortChangedSignal();
        }

        //  是否发送声音输出设备切换弹窗信号
        sendOsdWidgetSignal(portName, info.description);

        //  同步sinkPortName
        if (sinkPortName != portName) {
            sinkPortName = portName;
            sendDeviceChangedSignal(this);
        }

        // 更新输出设备、音量、静音状态
        insertJson(JsonType::JSON_TYPE_USERINFO, AUDIO_CARD_KEY, cardStr);
        insertJson(JsonType::JSON_TYPE_USERINFO, AUDIO_CARDID_KEY, cardId);
        insertJson(JsonType::JSON_TYPE_USERINFO, AUDIO_VOLUME_KEY, paVolumeToValue(volume));
        insertJson(JsonType::JSON_TYPE_USERINFO, AUDIO_MUTE_KEY, info.mute ? true : false);

        //  同步默认声卡index
        defaultOutputCard = info.card;

        //  同步音量，同步UI
        balance = pa_cvolume_get_balance(&info.volume,&info.channel_map);
        refreshVolume(SoundType::SINK, volume, info.mute);
    }
    qDebug() << "UpdateSink--->" << "sinkIndex:" << sinkIndex << "sinkPortName:" << sinkPortName << "sinkVolume:" << sinkVolume << "channel:" << channel;

    if (info.ports) {
        QMap<QString,QString> temp;
        for (pa_sink_port_info ** sinkPort = info.ports; *sinkPort != nullptr; ++sinkPort) {
            temp.insertMulti(info.name,(*sinkPort)->name);
        }
        QList <QMap<QString,QString>> sinkPortMapList;
        if (sinkPortMap.isEmpty()) {
            sinkPortMap.insertMulti(info.card,temp);
        }
        sinkPortMapList = sinkPortMap.values();

        if (!sinkPortMapList.contains(temp)) {
            sinkPortMap.insertMulti(info.card,temp);
        }

        w->ports.clear();
    }
    return is_new;
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

    if (pa_stream_is_suspended(s)) {
//        w->updateVolumeMeter(pa_stream_get_device_index(s), PA_INVALID_INDEX, -1);
    }
}

static void read_callback(pa_stream *s, size_t length, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    const void *data;
    double v;

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

    pa_stream_set_read_callback(s, read_callback, this);
    pa_stream_set_suspended_callback(s, suspended_callback, this);

    flags = (pa_stream_flags_t) (PA_STREAM_DONT_MOVE | PA_STREAM_PEAK_DETECT | PA_STREAM_ADJUST_LATENCY |
                                 (suspend ? PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND : PA_STREAM_NOFLAGS) /*|
                                 (!showVolumeMetersCheckButton->isChecked() ? PA_STREAM_START_CORKED : PA_STREAM_NOFLAGS)*/);

    if (pa_stream_connect_record(s, t, &attr, flags) < 0) {
        showError(tr("Failed to connect monitoring stream").toUtf8().constData());
        pa_stream_unref(s);
        return nullptr;
    }
    return s;
}

void UkmediaVolumeControl::initDefaultSourceVolume(const pa_source_info &info)
{
    QString active_port_name = info.active_port ? info.active_port->name : "";
    if (strcmp(info.name, "auto_null") == 0 && strcmp(defaultSourceName, "auto_null") == 0)
        active_port_name = "auto_null_source";

    if (active_port_name.isEmpty() || active_port_name.contains("3a-algo")
            || customSoundFile->isExist(stringRemoveUnrecignizedChar(active_port_name)))
        return;

    if (active_port_name.contains("hda")) {
        QString preSourcePortName = active_port_name;
        int index = preSourcePortName.indexOf("hda-");
        preSourcePortName.remove(index, 4);
        if (customSoundFile->isExist(stringRemoveUnrecignizedChar(preSourcePortName))) {
            qDebug() << "因PA配置导致端口变化，不做初始化音量处理";
            return;
        }
    }
    else if(active_port_name.contains("alc269vc")) {
        QString preSourcePortName = active_port_name;
        int index = preSourcePortName.indexOf("alc269vc-");
        preSourcePortName.remove(index, 9);
        if (customSoundFile->isExist(stringRemoveUnrecignizedChar(preSourcePortName))) {
            qDebug() << "因PA配置导致端口变化，不做初始化音量处理";
            return;
        }
    }

    setSourceVolumeByInfo(info,valueToPaVolume(MIC_VOLUME));
    customSoundFile->addXmlNode(active_port_name, false);
    qDebug() << __func__ << active_port_name;
}

void UkmediaVolumeControl::updateSource(const pa_source_info &info) {
    int volume = (info.volume.channels >= 2) ? MAX(info.volume.values[0], info.volume.values[1])
            : info.volume.values[0];

    // Record the masterDevice of noiseReduceSource
    if (pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE)) {
        masterDevice = pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE);
    }

    //  Initialize the input volume
    initDefaultSourceVolume(info);

    //  Update some variables for the default input
    if (UKMedia_Equal(defaultSourceName.data(), info.name))
    {
        sourceIndex = info.index;
        inputChannel = info.volume.channels;
        QString portName = "";
        int inputCard = -1;


        // Status: this is a filter soource
        if (pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE)) {
            masterDevice = pa_proplist_gets(info.proplist, PA_PROP_DEVICE_MASTER_DEVICE);
            sourceInfo s = getSourceInfoByName(masterDevice);
            inputCard = s.card;
            portName = s.active_port_name;
            qInfo() << "This is a filter source:" << info.name << "master device:" << masterDevice;
        }
        // Status: generally
        else {
            inputCard = info.card;
            portName = (info.active_port) ? info.active_port->name : "";
        }

        //  判断是否发送端口改变信号
        if (sourcePortName != portName || defaultInputCard != inputCard) {
            sendSourcePortChangedSignal();
            qInfo() << "Send SIGNAL: sourcePortChanged." << sourcePortName << portName << defaultInputCard << inputCard;
        }

        // 如果当前端口为无 or 内置Mic，需要关闭侦听GSettings
        if (portName.isEmpty() || portName.contains("internal") || portName.contains("Mic1")) {
            if (QGSettings::isSchemaInstalled(KEY_SOUNDS_SCHEMA)) {
                QGSettings soundSettings(KEY_SOUNDS_SCHEMA);

                if(soundSettings.get(LOOP_BACK).toBool()) {
                    soundSettings.set(LOOP_BACK, "false");
                    qInfo() << "Disable GSettings of the Loopback module";
                }
            }
        }

        //  当信号发送完，更新defaultInputCard和sourcePortName
        sourcePortName = portName;
        defaultInputCard = inputCard;
        refreshVolume(SoundType::SOURCE, volume, info.mute);
        qInfo() << __func__ << "Status1 defaultSource:" << sourceIndex << defaultSourceName << "sourcePort" << sourcePortName;
    }
    /* 开启降噪状态下，如果这个source存在多个端口，且不同端口音量相同时，不会触发 noiseReduceSource 回调，导致无法更新 portName */
    else if (UKMedia_Equal(masterDevice.toLatin1().data(), info.name) && UKMedia_Equal(defaultSourceName, "noiseReduceSource"))
    {
        if (!UKMedia_Equal(info.active_port->name, sourcePortName.toLatin1().data()) || info.card != defaultInputCard) {
            sendSourcePortChangedSignal();
            qInfo() << "Send SIGNAL: sourcePortChanged. Status 2";
        }

        defaultInputCard = info.card;
        sourcePortName = (info.active_port) ? info.active_port->name : "";
        qInfo() << __func__ << "Status2 defaultSource:" << sourceIndex << defaultSourceName << "sourcePort" << sourcePortName;
    }

    if (info.ports) {
        QMap<QString,QString> temp;
        for (pa_source_port_info **sourcePort = info.ports; *sourcePort != nullptr; ++sourcePort) {
            temp.insertMulti(info.name, (*sourcePort)->name);
        }

        if (sourcePortMap.isEmpty()) {
            sourcePortMap.insertMulti(info.card, temp);
        }

        if (!sourcePortMap.values().contains(temp)) {
            sourcePortMap.insertMulti(info.card, temp);
        }
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

        if (strcmp(t, "video") == 0 ||
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
    QString appId = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID);
    QString appIconName = pa_proplist_gets(info.proplist,PA_PROP_APPLICATION_ICON_NAME);\
    const gchar *appBinary = pa_proplist_gets(info.proplist,PA_PROP_APPLICATION_PROCESS_BINARY);
    qDebug() << "description:" << description << "appId:" << appId << "appIconName:" << appIconName << "appBinary:" << appBinary;
    appBinary = AppDesktopFileAdaption(appBinary);

    if (appBinary) {
        appId = appBinary;
        appIconName = appBinary;
        description = appBinary;
    }
    else if (appBinary && UKMedia_Equal(appBinary, "lingmo-recorder") &&
             UKMedia_Equal(pa_proplist_gets(info.proplist, PA_PROP_MEDIA_NAME), "pulsesink probe")) {
        return;
    }
    else {
        return;
    }

    qDebug() << "updateSinkInput:" << "appIconName:"<< appIconName << "appId:" << appId << "desc:" << description << "appBinary:" << appBinary << info.volume.values[0];

    //没制定应用名称的不加入到应用音量中
    if (description && !strstr(description,"QtPulseAudio") && !strstr(description,"ALSA") && !strstr(description,"aplay"))
    {
        sinkInputIndexMap.insert(info.index,description);    //记录每个sinkinput的index
        sinkInputValueMap.insert(description,info.volume.values[0]);     //记录相同description的最新音量
        sinkInputMuteMap.insert(description,info.mute);

        if (!sinkInputList.contains(description)) {
            sinkInputList.append(description);  //记录应用的个数 来增删应用widget
            Q_EMIT addSinkInputSignal(description, appId, info.index, info.volume.values[0], info.volume.channels);
        }
        Q_EMIT sinkInputVolumeChangedSignal(description, appId, info.volume.values[0]); //应用音量发生改变时发送信号更新滑动条的值

        // ukcc应用音量需求
        sinkInputMap.insert(info.index, addSinkInputInfo(info));
        Q_EMIT updateApp(appBinary);
     }
}

void UkmediaVolumeControl::updateSourceOutput(const pa_source_output_info &info) {
    sourceOutputChannel = info.volume.channels;
    const char *app;
    bool is_new = false;

    if ((app = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID))) {
        if (strcmp(app, "org.PulseAudio.pavucontrol") == 0
                || strcmp(app, "org.gnome.VolumeControl") == 0
                || strcmp(app, "org.kde.kmixd") == 0) {
            return;
        }
    }

    const gchar *description = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_NAME);
    QString appId = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ID);
    QString appIconName = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_ICON_NAME);
    const gchar *appBinary = pa_proplist_gets(info.proplist,PA_PROP_APPLICATION_PROCESS_BINARY);
    qDebug() << "description:" << description << "appId:" << appId << "appIconName:" << appIconName << "appBinary:" << appBinary;
    appBinary = AppDesktopFileAdaption(appBinary);

    if (appBinary) {
        appId = appBinary;
        description = appBinary;
        appIconName = appBinary;
    }
    else {
        qDebug() << "updateSourceOutput appBinary is null";
        return;
    }

    qDebug() << "updateSourceOutput:" << "appIconName:"<< appIconName << "appId:" << appId << "desc:" << description << "appBinary:" << appBinary << info.volume.values[0];

    if(description && !strstr(description,"QtPulseAudio") && !strstr(description,"ALSA") && !strstr(description,"Volume Control") && !strstr(description,"aplay"))
    {
        sourceOutputIndexMap.insert(info.index,description); //记录所有source-output index
        sourceOutputValueMap.insert(description,info.volume.values[0]); //记录相同description的最新音量
        sinkInputMuteMap.insert(description,info.mute);

        if (!sinkInputList.contains(description)) {
            sinkInputList.append(description);
            Q_EMIT addSinkInputSignal(description, appId, info.index, info.volume.values[0], info.volume.channels);
        }
        if (!sinkInputValueMap.keys().contains(description))
            Q_EMIT sinkInputVolumeChangedSignal(description, appId, info.volume.values[0]);

        // ukcc应用音量需求
        sourceOutputMap.insert(info.index, addSourceOutputInfo(info));
        Q_EMIT updateApp(description);
    }
}


void UkmediaVolumeControl::updateClient(const pa_client_info &info) {
    g_free(clientNames[info.index]);
    clientNames[info.index] = g_strdup(info.name);
}

void UkmediaVolumeControl::updateServer(const pa_server_info &info) {
    defaultSourceName = info.default_source_name ? info.default_source_name : "";
    defaultSinkName = info.default_sink_name ? info.default_sink_name : "";
    qDebug() << "updateServer" << "defaultSinkName " << defaultSinkName << "defaultSourceName" << defaultSourceName;
}

#if HAVE_EXT_DEVICE_RESTORE_API
void UkmediaVolumeControl::updateDeviceInfo(const pa_ext_device_restore_info &info) {
//    if (sinkWidgets.count(info.index)) {
//        SinkWidget *w;
//        pa_format_info *format;

//        w = sinkWidgets[info.index];

//        w->updating = true;

//        /* Unselect everything */
//        for (int j = 1; j < PAVU_NUM_ENCODINGS; ++j)
//            w->encodings[j].widget->setChecked(false);


//        for (uint8_t i = 0; i < info.n_formats; ++i) {
//            format = info.formats[i];
//            for (int j = 1; j < PAVU_NUM_ENCODINGS; ++j) {
//                if (format->encoding == w->encodings[j].encoding) {
//                    w->encodings[j].widget->setChecked(true);
//                    break;
//                }
//            }
//        }

//        w->updating = false;
//    }
}
#endif


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


void UkmediaVolumeControl::removeCard(uint32_t index) {

}

void UkmediaVolumeControl::removeSink(uint32_t index) {
    QMap<int, sinkInfo>::iterator it;
    Q_EMIT removeSinkSignal();

    for (it = sinkMap.begin(); it != sinkMap.end();) {
        if (it.key() == index) {
            qDebug() << "removeSink" << index;
            sinkMap.erase(it);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeSource(uint32_t index) {
    QMap<int, sourceInfo>::iterator it;
    for (it=sourceMap.begin();it!=sourceMap.end();)  {
        if (it.key() == index) {
            qDebug() << "removeSource" << index;
            sourceMap.erase(it);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeSinkInput(uint32_t index)
{
    QString description = "";
    qDebug() << "removeSinkInput" << index;

    sinkInputMap.remove(index);
    description = sinkInputIndexMap.value(index); //先获取要移除sinkinput的description
    sinkInputIndexMap.remove(index);//移除对应index的sinkinput

    if(!sinkInputIndexMap.values().contains(description) || sinkInputIndexMap.isEmpty())
    {
        sinkInputValueMap.remove(description);//如果所有相同description的index都被移除，需要移除valueMap对应的description
        sinkInputMuteMap.remove(description);
        if(description != "")
            Q_EMIT removeSinkInputSignal(description);//并移除对应app widget
    }

}

void UkmediaVolumeControl::removeSourceOutput(uint32_t index)
{
    QString description = "";
    sourceOutputMap.remove(index);
    description = sourceOutputIndexMap.value(index);
    sourceOutputIndexMap.remove(index);

    qDebug() << __func__ << index << description;

    if(description == "Loopback")
        isLoadLoopback = false;

    if(!sourceOutputIndexMap.values().contains(description) || sourceOutputIndexMap.isEmpty())
    {
        sourceOutputValueMap.remove(description);
        sinkInputMuteMap.remove(description);
        //sinkinput包含同名应用时，不发送remove信号，只更新应用类型为仅输出可用
        if(description != "" && !sinkInputIndexMap.values().contains(description)){
            Q_EMIT removeSinkInputSignal(description);
        }
        else if (description != "")
            Q_EMIT updateApp(description);
    }
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
}

void UkmediaVolumeControl::showError(const char *txt)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s: %s", txt, pa_strerror(pa_context_errno(context)));

    qWarning() << QString::fromUtf8(buf);
}

void UkmediaVolumeControl::decOutstanding(UkmediaVolumeControl *w) {
    if (w->n_outstanding <= 0)
        return;

    if (--w->n_outstanding <= 0) {
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

void UkmediaVolumeControl::batteryLevelCb(pa_context *c, const pa_card_info *i, int eol, void *userdata) {
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

    qDebug() << "batteryLevelCb" << i->index << eol;
    const char *battery = pa_proplist_gets(i->proplist,"bluetooth.battery");
    QString macAddr = pa_proplist_gets(i->proplist, "device.string");

    if (battery) {

        qDebug() << "get bluetooth battery is:" << battery << atoi(battery);
        w->batteryLevel = atoi(battery);
    } else {
        w->batteryLevel = -1;
    }

    Q_EMIT w->bluetoothBatteryChanged(macAddr, w->batteryLevel);
}

void UkmediaVolumeControl::bluetoothCardCb(pa_context *c, const pa_card_info *i, int eol, void *userdata) {
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

    qDebug() << "bluetooth card cb" << i->index << eol;
    const char *battery = pa_proplist_gets(i->proplist,"bluetooth.battery");
    QString macAddr = pa_proplist_gets(i->proplist, "device.string");
    if (battery) {

        qDebug() << "bluetooth battery is ===" << battery << atoi(battery);
//        char *str = strtok(battery,".");
        w->batteryLevel = atoi(battery);
    } else {
        w->batteryLevel = -1;
    }

    Q_EMIT w->bluetoothBatteryChanged(macAddr, w->batteryLevel);
}


#if HAVE_EXT_DEVICE_RESTORE_API
static void ext_device_restore_subscribeCb(pa_context *c, pa_device_type_t type, uint32_t idx, void *userdata);
#endif

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
    QString portName = "";
    if (i->volume.channels >= 2) {
        volume = MAX(i->volume.values[0],i->volume.values[1]);
    } else {
        volume = i->volume.values[0];
    }

    w->channel = i->volume.channels;
    w->sinkIndex = i->index;
    w->balance = pa_cvolume_get_balance(&i->volume,&i->channel_map);

    /*  同步sinkPortName三种情况
     *  case1: 默认声卡端口可用，直接同步当前可用端口
     *  case2：默认声卡端口不可用，端口设置为空
     *  case3: 默认声卡为虚拟声卡，查找master.device声卡信息并同步
     */
    if (i->active_port) {
        portName = i->active_port->name;
    } else {
        portName = "";
    }

    if (pa_proplist_gets(i->proplist, PA_PROP_DEVICE_MASTER_DEVICE)) {
        portName = w->findSinkMasterDeviceInfo(i->proplist, i->name);
    }

    //  是否发送音乐暂停信号
    if (w->isNeedSendPortChangedSignal(portName, w->sinkPortName, i->name)) {
        w->sendPortChangedSignal();
    }

    //  是否发送声音输出设备切换弹窗信号
    w->sendOsdWidgetSignal(portName, i->description);

    //  同步sinkPortName
    w->sinkPortName = (w->sinkPortName != portName) ? portName : w->sinkPortName;

    //  同步默认声卡index
    w->defaultOutputCard = i->card;

    //  同步音量，同步UI
    w->refreshVolume(SoundType::SINK, volume, i->mute);

    qDebug() << "sinkIndexCb" << w->defaultOutputCard << w->sinkPortName << w->sinkIndex;
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
    if (i->volume.channels >= 2) {
        volume = MAX(i->volume.values[0],i->volume.values[1]);
    } else {
        volume = i->volume.values[0];
    }

    w->inputChannel = i->volume.channels;
    w->sourceIndex = i->index;

    QString portName = "";
    int inputCard = -1;

    if (pa_proplist_gets(i->proplist, PA_PROP_DEVICE_MASTER_DEVICE)) {
        w->masterDevice = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_MASTER_DEVICE);
        sourceInfo s = w->getSourceInfoByName(w->masterDevice);
        inputCard = s.card;
        portName = s.active_port_name;
    }
    else {
        inputCard = i->card;
        portName = (i->active_port) ? i->active_port->name : "";
    }

    // 判断是否发送端口改变信号
    if (w->sourcePortName != portName || w->defaultInputCard != inputCard) {
        w->sendSourcePortChangedSignal();
        qInfo() << "Send SIGNAL: sourcePortChanged." << w->sourcePortName << portName << w->defaultInputCard << inputCard;
    }

    //  当信号发送完，更新defaultInputCard和sourcePortName
    w->sourcePortName = portName;
    w->defaultInputCard = inputCard;

    w->refreshVolume(SoundType::SOURCE, volume, i->mute);
    qDebug() << __func__ << "defaultInputCard" << w->defaultInputCard << "sourcePort" << w->sourcePortName;
}

void UkmediaVolumeControl::sinkCb(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY) {
            return;
        }

        w->showError(QObject::tr("Sink callback failure").toUtf8().constData());
        return;
    }

    if (eol > 0) {
        decOutstanding(w);
        return;
    }

    w->m_pDefaultSink = i;
    w->sinkMap.insert(i->index,  w->addSinkInfo(*i));

    qDebug() << __func__ << "Volume:" << i->volume.values[0] << "isMute:" << i->mute << i->name;
    w->updateSink(w,*i);
}

void UkmediaVolumeControl::sourceCb(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY) {
            return;
        }

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
        decOutstanding(w);
        return;
    }

    if (i->source == PA_ID_INVALID)
        return;

    if (strstr(i->name,"Loopback")) {
        w->sourceOutputIndexMap.insert(i->index,"Loopback");
        qDebug() << "The module-loopback has been loaded." << w->sourceOutputIndexMap;
    }

    qDebug() << "sourceOutputCb" << i->name;
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

void UkmediaVolumeControl::serverInfoCb(pa_context *, const pa_server_info *i, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (!i) {
        w->showError(QObject::tr("Server info callback failure").toUtf8().constData());
        return;
    }

    pa_operation *o;
    //默认的输出设备改变时需要获取默认的输出音量
    if (strcmp(w->defaultSinkName.data(),i->default_sink_name) != 0) {
      //默认输出改变时才走sinkindexcb
        if(!(o = pa_context_get_sink_info_by_name(w->getContext(),i->default_sink_name,sinkIndexCb,w))) {
            w->showError(tr("pa_context_get_sink_info_by_name() failed").toUtf8().constData());
        }
    }
    if(!(o = pa_context_get_source_info_by_name(w->getContext(),i->default_source_name,sourceIndexCb,w))){
        w->showError(tr("pa_context_get_source_info_by_name() failed").toUtf8().constData());
    }

    qDebug() << "serverInfoCb" << i->default_sink_name << i->default_source_name;
    w->updateServer(*i);
    sendDeviceChangedSignal(w);
    decOutstanding(w);
}

void UkmediaVolumeControl::serverInfoIndexCb(pa_context *, const pa_server_info *i, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (!i) {
        w->showError(QObject::tr("Server info callback failure").toUtf8().constData());
        return;
    }
    pa_operation *o;

//    qDebug() << "serverInfoIndexCb" << i->default_sink_name << i->default_source_name;
    w->updateServer(*i);
    decOutstanding(w);
}

void UkmediaVolumeControl::getBatteryLevel(QString dev)
{
    pa_operation *o;
    if (!(o = pa_context_get_card_info_by_name(getContext(), dev.toLatin1().data(), batteryLevelCb, this))) {
        showError(QObject::tr("pa_context_get_card_info_by_index() failed").toUtf8().constData());
        return ;
    }
    pa_operation_unref(o);
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

#if HAVE_EXT_DEVICE_RESTORE_API
void ext_device_restore_read_cb(
        pa_context *,
        const pa_ext_device_restore_info *i,
        int eol,
        void *userdata) {

    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        w->decOutstanding(w);
        g_debug(QObject::tr("Failed to initialize device restore extension: %s").toUtf8().constData(), pa_strerror(pa_context_errno(context)));
        return;
    }

    if (eol > 0) {
        w->decOutstanding(w);
        return;
    }

    /* Do something with a widget when this part is written */
    w->updateDeviceInfo(*i);
}

static void ext_device_restore_subscribeCb(pa_context *c, pa_device_type_t type, uint32_t idx, void *userdata) {
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);
    pa_operation *o;

    if (type != PA_DEVICE_TYPE_SINK)
        return;

    if (!(o = pa_ext_device_restore_read_formats(c, type, idx, ext_device_restore_read_cb, w))) {
        w->showError(QObject::tr("pa_ext_device_restore_read_sink_formats() failed").toUtf8().constData());
        return;
    }

    pa_operation_unref(o);
}
#endif

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
#ifdef PA_SUBSCRIPTION_EVENT_BLUETOOTH_BATTERY
        case PA_SUBSCRIPTION_EVENT_BLUETOOTH_BATTERY:

            qDebug() << "PA_SUBSCRIPTION_EVENT_BLUETOOTH_BATTERY" ;
            pa_operation *o;
            if (!(o = pa_context_get_card_info_by_index(c, index, bluetoothCardCb, w))) {
                w->showError(QObject::tr("pa_context_get_card_info_by_index() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            break;
#endif
        case PA_SUBSCRIPTION_EVENT_CARD:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                qDebug() << "remove cards------";
                //移除outputPort
                w->removeSinkPortMap(index);
                w->removeOutputPortMap(index);
                w->removeSourcePortMap(index);
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
            w->n_outstanding = 0;

            if (!(o = pa_context_get_server_info(c, serverInfoCb, w))) {
                w->showError(QObject::tr("pa_context_get_server_info() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            w->n_outstanding++;

            if (!(o = pa_context_get_client_info_list(c, clientCb, w))) {
                w->showError(QObject::tr("pa_context_client_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            w->n_outstanding++;

            if (!(o = pa_context_get_card_info_list(c, cardCb, w))) {
                w->showError(QObject::tr("pa_context_get_card_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            w->n_outstanding++;

            if (!(o = pa_context_get_sink_info_list(c, sinkCb, w))) {
                w->showError(QObject::tr("pa_context_get_sink_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            w->n_outstanding++;

            if (!(o = pa_context_get_source_info_list(c, sourceCb, w))) {
                w->showError(QObject::tr("pa_context_get_source_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            w->n_outstanding++;

            if (!(o = pa_context_get_sink_input_info_list(c, sinkInputCb, w))) {
                w->showError(QObject::tr("pa_context_get_sink_input_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            w->n_outstanding++;

            if (!(o = pa_context_get_source_output_info_list(c, sourceOutputCb, w))) {
                w->showError(QObject::tr("pa_context_get_source_output_info_list() failed").toUtf8().constData());
                return;
            }
            pa_operation_unref(o);
            w->n_outstanding++;

            Q_EMIT w->paContextReady();
            break;
        }

        case PA_CONTEXT_FAILED:
            w->setConnectionState(false);
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

void UkmediaVolumeControl::moduleInfoCb(pa_context *c, const pa_module_info *i, int eol, void *userdata)
{
    UkmediaVolumeControl *w = static_cast<UkmediaVolumeControl*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        w->showError(QObject::tr("moduleInfoCb callback failure").toUtf8().constData());
        return;
    }
    if(i && strcmp(i->name,w->findModuleStr.toLatin1().data()) == 0) {
        w->findModuleIndex = i->index;
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
    return sinkInputValueMap.value(name);
}

/*
 * 根据名称获取source output音量
 */
int UkmediaVolumeControl::getSourceOutputVolume(const gchar *name)
{
   return sourceOutputValueMap.value(name);
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
    qDebug() << "sinkInputCallback" <<w->sinkInputVolume <<i->name;
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
    qDebug() << "removeCardProfileMap";
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
            //usb耳机一个声卡存在两个端口，使用erase时只会擦除一个key时，导致拔出耳机时没有完全移除key值，需要使用remove将存入的两个相同的key值全部删除
            //sinkPortMap.erase(it);
            sinkPortMap.remove(index);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeSourcePortMap(int index)
{
    qDebug() << "removeSourcePortMap///";
    QMap<int,QMap<QString,QString>>::iterator it;
    for(it=sourcePortMap.begin();it!=sourcePortMap.end();){
        if(it.key() == index) {
            sourcePortMap.remove(index);
            break;
        }
        ++it;
    }
}

void UkmediaVolumeControl::removeProfileMap(int index)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    qDebug() << "removeProfileMap"  << index;

    for (it=profileNameMap.begin();it!=profileNameMap.end();) {
        if(it.key() == index)
        {
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

int UkmediaVolumeControl::findOutputPort(QString name){
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    int count = 0;
    for (it=outputPortMap.begin();it!=outputPortMap.end();) {
        portMap = it.value();
        for (at=portMap.begin();at!=portMap.end();) {
            if (at.key() == name) {
                ++count;
            }
            ++at;
        }
        ++it;
    }
    return count;
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

void UkmediaVolumeControl::removeInputProfile(int index)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    qDebug() << "removeInputProfile"  << index;

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

bool UkmediaVolumeControl::isNeedSendPortChangedSignal(QString newPort, QString prePort, QString cardName)
{
    //  端口不变时不发送信号
    if (newPort == prePort)
        return false;

    //  切换到空端口发送信号
    if (newPort.isEmpty() && !prePort.isEmpty() && cardName != "mono") {
        qDebug() << "Send SIGNAL: sinkPortChanged. Case: Switch to a null port" << newPort << prePort;
        return true;
    }

    //  拔插设备自动切换到扬声器端口时发送信号，用户主动切换输出时不做信号发送
    if (newPort.contains(SPEAKER) && !cardName.contains("bluez") && !cardName.contains("usb")) {
        int newPortCount = -1;
        int prePortCount = -1;
        newPortCount = findOutputPort(newPort);
        prePortCount = findOutputPort(prePort);
        //  保证outputPortMap中只含有一个扬声器端口，并且之前的端口已不在map中
        if (newPortCount == 1 && prePortCount == 0) {
            qDebug() << "Send SIGNAL: sinkPortChanged. Case: Switch to speaker port" << newPort << prePort;
            return true;
        }
    }

    return false;
}


void UkmediaVolumeControl::sendPortChangedSignal()
{
    QDBusMessage message = QDBusMessage::createSignal("/","org.lingmo.media","sinkPortChanged");
    message<<"portChanged";
    QDBusConnection::sessionBus().send(message);
    qDebug() << "sendPortChangedSignal sinkPortChanged ";
}

void UkmediaVolumeControl::sendSourcePortChangedSignal()
{
    QDBusMessage message = QDBusMessage::createSignal("/","org.lingmo.media","sourcePortChanged");
    message<<"portChanged";
    QDBusConnection::sessionBus().send(message);
    qDebug() << "sendSourcePortChangedSignal sourcePortChanged ";
}


int UkmediaVolumeControl::findPortSinkIndex(QString name)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    int cardIndex = -1;
    for (it=sinkPortMap.begin();it!=sinkPortMap.end();) {
        portNameMap = it.value();
        for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
            if (tempMap.key() == name) {
                cardIndex = it.key();
                break;
            }
            ++tempMap;
        }
        ++it;
    }
    return cardIndex;
}


int UkmediaVolumeControl::findPortSourceIndex(QString name)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    int cardIndex = -1;
    for (it=sourcePortMap.begin();it!=sourcePortMap.end();) {
        portNameMap = it.value();
        for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
            if (tempMap.key() == name) {
                cardIndex = it.key();
                break;
            }
            ++tempMap;
        }
        ++it;
    }
    return cardIndex;
}

QString UkmediaVolumeControl::findSinkPortName(int cardIndex)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    QString portName = "";
    for (it=outputPortMap.begin();it!=outputPortMap.end();) {
        if(it.key() == cardIndex){
            portNameMap = it.value();
            for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
                portName = tempMap.key();
                break;
            }
            ++tempMap;
        }
        ++it;
    }
    return portName;
}

QString UkmediaVolumeControl::findSourcePortName(int cardIndex)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    QString portName = "";
    for (it=inputPortMap.begin();it!=inputPortMap.end();) {
        if(it.key() == cardIndex){
            portNameMap = it.value();
            for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
                portName = tempMap.key();
                break;
            }
            ++tempMap;
        }
        ++it;
    }
    return portName;
}

QString UkmediaVolumeControl::findSinkMasterDeviceInfo(pa_proplist *proplist, const char *info_name)
{
    masterSinkDev = pa_proplist_gets(proplist,PA_PROP_DEVICE_MASTER_DEVICE);
    if (!masterSinkDev.isEmpty()) {
        int cardIndex = findPortSinkIndex(masterSinkDev);
        if (cardIndex != -1) {
            qDebug() << "findSinkMasterDeviceInfo" << sinkPortName << findSinkPortName(cardIndex);
            defaultOutputCard = findPortSinkIndex(masterSinkDev);
            return findSinkPortName(cardIndex);
        }
        else {
            qDebug() << "can't find masterDevice info in findSinkMasterDeviceInfo()";
            return "";
        }
    }
}

int UkmediaVolumeControl::findMasterDeviceCardIndex(QString masterDev)
{
    return findPortSinkIndex(masterDev);
}

QString UkmediaVolumeControl::findMasterDeviceInfo(pa_proplist *proplist, const char *info_name)
{
    masterDevice = pa_proplist_gets(proplist,PA_PROP_DEVICE_MASTER_DEVICE);
    if (!masterDevice.isEmpty()) {
        int cardIndex = findPortSourceIndex(masterDevice);
        if (cardIndex != -1) {
            qDebug() << "findMasterDeviceInfo" << sourcePortName << findSourcePortName(cardIndex);
            defaultInputCard = findPortSourceIndex(masterDevice);
            return findSourcePortName(cardIndex);
        }
        else {
            qDebug() << "can't find masterDevice info in findMasterDeviceInfo()";
            return "";
        }
    }
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
            sinkMuted = info_Mute;
            Q_EMIT updateMute(sinkMuted);
        }

        if (sinkVolume != info_Vol) {
            sinkVolume = info_Vol;
            sinkMuted = info_Mute;
            sendVolumeUpdateSignal();
        }
    }
        break;
    case SoundType::SOURCE: {
        if (sourceMuted != info_Mute) {
            sourceMuted = info_Mute;
            Q_EMIT updateSourceMute(sourceMuted);
        }

        if (sourceVolume != info_Vol) {
            sourceVolume = info_Vol;
            sourceMuted = info_Mute;
            sendSourceVolumeUpdateSignal();
        }
    }
        break;
    default:
        break;
    }
}

void UkmediaVolumeControl::sendOsdWidgetSignal(QString portName, QString description)
{
    if (portName == "histen-algo" || defaultSinkName.contains("auto_null")) {
        return;
    }

    if (osdFirstFlag) {
        m_description = description;
        osdFirstFlag = false;
        return;
    }

    if (portName != sinkPortName || (sinkPortName == portName && !m_description.contains(description) && !description.contains(m_description))) {
        QString iconStr = "";
        if (portName.contains("headphone", Qt::CaseInsensitive)) {
            iconStr = "audio-headphones-symbolic";
        }
        else if (portName.contains("headset", Qt::CaseInsensitive)) {
            iconStr = "audio-headset-symbolic";
        }
        else {
            iconStr = "audio-volume-high-symbolic";
        }
        sendOSDDeviceChangedSignal(iconStr);
        qDebug() << "Send SIGNAL: osdWidgetSignal. Case: sink port has changed" << portName << sinkPortName << description << m_description;
    }

    m_description = description;
}

void UkmediaVolumeControl::UkmediaDbusRegister()
{
    QDBusConnection::sessionBus().unregisterService("org.lingmo.media");

    QDBusConnection con=QDBusConnection::sessionBus();

    if (!con.registerService("org.lingmo.media"))
        qDebug() << "registerService failed" << con.lastError().message();

    if (!con.registerObject("/org/lingmo/media/control", this,
                            QDBusConnection::ExportAllSlots|
                            QDBusConnection::ExportAllSignals))
        qDebug() << "registerObject failed" << con.lastError().message();

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

    if (pa_proplist_gets(i.proplist, PA_PROP_DEVICE_MASTER_DEVICE))
        info.master_device = pa_proplist_gets(i.proplist, PA_PROP_DEVICE_MASTER_DEVICE);

    if (i.ports) {
        for (pa_source_port_info** sourcePort = i.ports; *sourcePort != nullptr; ++sourcePort) {
            portInfo pInfo;
            pInfo.name = (*sourcePort)->name;
            pInfo.description = (*sourcePort)->description;
            pInfo.priority = (*sourcePort)->priority;
            pInfo.available = (*sourcePort)->available;
            info.source_port_list.append(pInfo);
        }
    }

    return info;
}

appInfo UkmediaVolumeControl::addSinkInputInfo(const pa_sink_input_info& info)
{
    appInfo app;
    app.name = AppDesktopFileAdaption(pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_PROCESS_BINARY));
#ifdef PA_PROP_APPLICATION_MOVE
    app.move = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_MOVE);
#endif
    app.index = info.index;
    app.mute = info.mute;
    app.volume = info.volume.values[0];
    app.channel = info.volume.channels;
    app.direction = PA_STREAM_PLAYBACK;
    app.masterIndex = info.sink;
    app.masterDevice = findMasterDevice(SoundType::SINK, info.sink);
    return app;
}

appInfo UkmediaVolumeControl::addSourceOutputInfo(const pa_source_output_info& info)
{
    appInfo app;
    app.name = AppDesktopFileAdaption(pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_PROCESS_BINARY));
#ifdef PA_PROP_APPLICATION_MOVE
    app.move = pa_proplist_gets(info.proplist, PA_PROP_APPLICATION_MOVE);
#endif
    app.index = info.index;
    app.mute = info.mute;
    app.volume = info.volume.values[0];
    app.channel = info.volume.channels;
    app.direction = PA_STREAM_RECORD;
    app.masterIndex = info.source;
    app.masterDevice = findMasterDevice(SoundType::SOURCE, info.source);
    return app;
}

/*
 *  通过sink index寻找对应sink name
 *  通过source index寻找对应source name
 */
QString UkmediaVolumeControl::findMasterDevice(int soundType, int index)
{
    QString name = "";

    if (SoundType::SINK == soundType) {
        QMap<int, sinkInfo>::iterator it;
        for (it = sinkMap.begin(); it != sinkMap.end(); it++) {
            if (index == it.key()) {
                name = it.value().name;
                break;
            }
        }
    }
    else if (SoundType::SOURCE == soundType) {
        QMap<int, sourceInfo>::iterator it;
        for (it = sourceMap.begin(); it != sourceMap.end(); it++) {
            if (index == it.key()) {
                name = it.value().name;
                break;
            }
        }
    }

    return name;
}

/*
 *  通过 sink-input name 寻找对应 index
 */
QList<int> UkmediaVolumeControl::findAppIndex(int soundType, QString appName)
{
    QList<int> appIndexList;
    QMap<int, QString> appMap;
    QMap<int, QString>::iterator it;

    if (SoundType::SINK == soundType)
        appMap = sinkInputIndexMap;
    else if (SoundType::SOURCE == soundType)
        appMap = sourceOutputIndexMap;

    for (it = appMap.begin(); it != appMap.end(); it++) {
        if (appName == it.value())
            appIndexList.append(it.key());
    }

    return appIndexList;
}

void UkmediaVolumeControl::simple_callback(pa_context *c, int success, void *userdata)
{
    if (!success) {
        qDebug() << "move failed" << pa_strerror(pa_context_errno(c));
        return;
    }
}

const gchar* UkmediaVolumeControl::AppDesktopFileAdaption(const gchar* appName)
{
    if (appName == nullptr)
        return nullptr;

    //某些第三方应用获取到appName与它的desktop文件名不一致，需手动适配
    if (UKMedia_Equal(appName, "aplay"))
        appName = nullptr;
    else if (UKMedia_Equal(appName, "lingmo-kmre-audio"))
        appName = nullptr;
    else if (UKMedia_Equal(appName, "lingmo-powermanagement"))
        appName = nullptr;
    else if (UKMedia_Equal(appName, "wpp"))
        appName = "wps-office-wpp";
    else if (UKMedia_Equal(appName, "wine-preloader"))
        appName = "lingmo-kwre-wechat";
    else if (UKMedia_Equal(appName, "wechat"))
        appName = "wechat-beta";
    else if (strstr(appName, "qaxbrowser"))
        appName = "qaxbrowser-safe";
    else if (strstr(appName, "browser360"))
        appName = "browser360-cn";

    return appName;
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
            Q_EMIT this->updateVolume(sinkVolume);
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
            Q_EMIT this->updateSourceVolume(sourceVolume);
        });
    }
    m_updateSourceVolumeTimer.start();
    //end
}

void UkmediaVolumeControl::sendOSDDeviceChangedSignal(QString icon)
{
    //start 过滤设备端口切换或者设备拔插期间，subscribecb回调反馈的非预期数据
    static QString iconStr = "";
    iconStr = icon;
    static bool isConnect = false;
    if (m_osdDeviceChangedTimer.isActive()) {
        m_osdDeviceChangedTimer.stop();
    }
    m_osdDeviceChangedTimer.setInterval(300);
    m_osdDeviceChangedTimer.setSingleShot(true);
    if (!isConnect) {
        isConnect = QObject::connect(&m_osdDeviceChangedTimer, &QTimer::timeout, this, [&](){
            qDebug() << "device_changed_signal iconStr:" << iconStr;
            Q_EMIT this->device_changed_signal(iconStr);
        });
    }
    m_osdDeviceChangedTimer.start();
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
