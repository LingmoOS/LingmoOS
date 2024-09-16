// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "voicevolumewatcher.h"

#include <QThread>

voiceVolumeWatcher::voiceVolumeWatcher(QObject *parent)
    : QObject(parent)
    , m_coulduse(false)
    , m_isExistSystemAudio(false)
{
    m_audioUtils = new AudioUtils();
    //检查是否存在音频服务
    if (m_audioUtils != nullptr) {
        QString cards = m_audioUtils->cards();
        if (!cards.isEmpty()) {
            //初始化所有的活动端口
            initAvailInputPorts(cards);
        }
        connect(m_audioUtils, &AudioUtils::cardsChanged, this, &voiceVolumeWatcher::onCardsChanged);
        m_isExistSystemAudio = true;
        m_watchTimer = new QTimer(this);
        //定时器检测麦克风是否存在
        connect(m_watchTimer, &QTimer::timeout, this, &voiceVolumeWatcher::slotvoiceVolumeWatcher);
    } else {
        qWarning() << "启动音频监听服务失败！";
    }
}

//停止log循环读取
void voiceVolumeWatcher::setWatch(const bool isWatcher)
{
    if (isWatcher) {
        m_watchTimer->start(1000);
    } else {
        m_watchTimer->stop();
    }
}

// 将原有的run方法替换为slotvoiceVolumeWatcher，解决截图录屏退出时缓慢的问题
void voiceVolumeWatcher::slotvoiceVolumeWatcher()
{
    static const double DBL_EPSILON = 0.000001;
    bool couldUse = false;
    double currentMicrophoneVolume = 0.0;
    if (nullptr != m_audioUtils) {
        //https://pms.uniontech.com/zentao/bug-view-52019.html
        //qDebug() << "=======================================";
        couldUse = false;
        if (isMicrophoneAvail(m_audioUtils->defaultSourceActivePort().name)) {
            currentMicrophoneVolume = m_audioUtils->defaultSourceVolume();
            if (currentMicrophoneVolume > DBL_EPSILON) {
                couldUse = true;
            }
        }
        if (couldUse != m_coulduse) {
            //发送log信息到UI
            m_coulduse = couldUse;
            emit sigRecodeState(couldUse);
        }
    } else {
        if (couldUse != m_coulduse) {
            //发送log信息到UI
            m_coulduse = couldUse;
            emit sigRecodeState(couldUse);
        }
    }
}

//返回当前是否有系统声卡
bool voiceVolumeWatcher::getystemAudioState()
{
    return m_isExistSystemAudio;
}

// 麦克风声音检测
bool voiceVolumeWatcher::isMicrophoneAvail(const QString &activePort) const
{
    bool available = false;
    QMap<QString, Port>::const_iterator iter = m_availableInputPorts.find(activePort);
    if (iter != m_availableInputPorts.end()) {
        //qDebug() <<"麦克风是否可用： " <<  iter.key() << iter.value();
        if (!iter->isLoopback()) {
            available = true;
        }
    }
    return available;
}

void voiceVolumeWatcher::onCardsChanged(const QString &value)
{
    //qDebug() << "Cards changed:" << value;
    if (value.isEmpty()) {
        return;
    }
    initAvailInputPorts(value);
}

void voiceVolumeWatcher::initAvailInputPorts(const QString &cards)
{
    m_availableInputPorts.clear();
    QJsonDocument doc = QJsonDocument::fromJson(cards.toUtf8());
    //qDebug() << "cards: " << doc;
    QJsonArray jCards = doc.array();

    for (QJsonValue cardVar : jCards) {
        QJsonObject jCard = cardVar.toObject();
        const QString cardName = jCard["Name"].toString();
        const int     cardId   = jCard["Id"].toInt();

        QJsonArray jPorts = jCard["Ports"].toArray();

        for (QJsonValue portVar : jPorts) {
            Port port;

            QJsonObject jPort = portVar.toObject();
            port.available = jPort["Available"].toInt();
            port.isActive = false;

            // 0 Unknow 1 Not available 2 Available
            if (port.available == 2 || port.available == 0) {
                port.portId   = jPort["Name"].toString();
                port.portName = jPort["Description"].toString();
                port.cardId   = cardId;
                port.cardName = cardName;
                // 只添加输入port
                if (port.isInputPort()) {
                    m_availableInputPorts.insert(port.portId, port);
                    //qDebug() << " " << port;
                }
            }
        }
    }
    //qDebug() << "m_availableInputPorts: " << m_availableInputPorts;
}
//只要端口名不含有output字段，那该端口都归于输入端口
bool voiceVolumeWatcher::Port::isInputPort() const
{
//    const QString inputPortFingerprint("input");
//    return portId.contains(inputPortFingerprint, Qt::CaseInsensitive);
    const QString inputPortFingerprint("output");
    return !portId.contains(inputPortFingerprint, Qt::CaseInsensitive);
}

bool voiceVolumeWatcher::Port::isLoopback() const
{
    const QString loopbackFingerprint("Loopback");

    return cardName.contains(loopbackFingerprint,  Qt::CaseInsensitive);
}


QDebug &operator <<(QDebug &out, const voiceVolumeWatcher::Port &port)
{
    out << "\n Port { "
        << "portId=" << port.portId << ","
        << "portName=" << port.portName << ","
        << "available=" << port.available << ","
        << "isActive=" << port.isActive << ","
        << "cardName=" << port.cardName << ","
        << "cardId=" << port.cardId << ","
        << " }\n";

    return out;
}

voiceVolumeWatcher::~voiceVolumeWatcher()
{

}

