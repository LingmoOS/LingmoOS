// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audioutils.h"

#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusError>
#include <QDBusMessage>


AudioUtils::AudioUtils(QObject *parent)
{
    Q_UNUSED(parent);
    initAudioDBusInterface();
}

//初始化音频dbus服务的接口
void AudioUtils::initAudioDBusInterface()
{
    // 初始化音频服务Dus接口
    m_audioDBusInterface = new QDBusInterface(AudioService,
                                              AudioPath,
                                              AudioInterface,
                                              QDBusConnection::sessionBus());
    //检查是否存在音频服务
    if (m_audioDBusInterface->isValid()) {
        qInfo() << "音频服务初始化成功！音频服务： " << AudioService << " 地址：" << AudioPath << " 接口：" << AudioInterface;
        m_defaultSourcePath = m_audioDBusInterface->property("DefaultSource").value<QDBusObjectPath>().path();
        initDefaultSourceDBusInterface();
        m_defaultSinkPath = m_audioDBusInterface->property("DefaultSink").value<QDBusObjectPath>().path();
        initDefaultSinkDBusInterface();
        //qDebug() << "sinks: " << m_audioDBusInterface->property("Sinks").value<QList<QDBusObjectPath>>();

        initConnections();
    } else {
        qWarning() << "初始化失败！音频服务 (" << AudioService << ") 不存在";
    }
}

//初始化音频dbus服务默认输入源的接口
void AudioUtils::initDefaultSourceDBusInterface()
{
    if (m_defaultSourceDBusInterface) {
        delete m_defaultSourceDBusInterface;
        m_defaultSourceDBusInterface = nullptr;
    }
    // 初始化Dus接口
    m_defaultSourceDBusInterface = new QDBusInterface(AudioService,
                                                      m_defaultSourcePath,
                                                      SourceInterface,
                                                      QDBusConnection::sessionBus());
    if (m_defaultSourceDBusInterface->isValid()) {
        qInfo() << "默认音频输入源初始化成功！音频服务： " << AudioService << " 默认输入源地址" << m_defaultSourcePath << " 默认输入源接口：" << SourceInterface;
    } else {
        qWarning() << "默认音频输入源初始化失败！默认输入源地址 (" << m_defaultSourcePath << ") 不存在";
    }
}

//初始化音频dbus服务默认输出源的接口
void AudioUtils::initDefaultSinkDBusInterface()
{
    if (m_defaultSinkDBusInterface) {
        delete m_defaultSinkDBusInterface;
        m_defaultSinkDBusInterface = nullptr;
    }
    // 初始化Dus接口
    m_defaultSinkDBusInterface = new QDBusInterface(AudioService,
                                                    m_defaultSinkPath,
                                                    SinkInterface,
                                                    QDBusConnection::sessionBus());
    if (m_defaultSinkDBusInterface->isValid()) {
        qInfo() << "默认音频输出源初始化成功！音频服务： " << AudioService << " 默认输出源地址" << m_defaultSinkPath << " 默认输出源接口：" << SinkInterface;
    } else {
        qWarning() << "默认音频输出源初始化失败！默认输出源地址 (" << m_defaultSinkPath << ") 不存在";
    }
}

//初始化音频dbus服务属性改变链接
void AudioUtils::initConnections()
{
    //监听音频服务的属性改变
    QDBusConnection::sessionBus().connect(AudioService,
                                          AudioPath,
                                          PropertiesInterface,
                                          "PropertiesChanged",
                                          "sa{sv}as",
                                          this,
                                          SLOT(onDBusAudioPropertyChanged(QDBusMessage))
                                         );
}

//获取当前系统音频通道
QString AudioUtils::currentAudioChannel()
{
    QString str_output = "-1";
    if (m_defaultSinkDBusInterface && m_defaultSinkDBusInterface->isValid()) {
        QString sinkName = m_defaultSinkDBusInterface->property("Name").toString();
        qDebug() << "系统声卡名称: " << sinkName;
        QStringList options;
        options << "-c";
        options << QString("pacmd list-sources | grep -PB 1 %1 | head -n 1 | perl -pe 's/.* //g'").arg(sinkName);
        QProcess process;
        process.start("bash", options);
        process.waitForFinished();
        process.waitForReadyRead();
        str_output = process.readAllStandardOutput();
        qDebug() << "pacmd命令: " << options;
        qDebug() << "通过pacmd命令获取的系统音频通道号: " << str_output;
        if (str_output.isEmpty()) {
            QStringList options1;
            options1 << "-c";
            options1 << QString("pactl list sources | grep -PB 2 %1 | head -n 1 | perl -pe 's/.* #//g'").arg(sinkName);
            process.start("bash", options1);
            process.waitForFinished();
            process.waitForReadyRead();
            str_output = process.readAllStandardOutput();
            qDebug() << "pactl命令: " << options;
            qDebug() << "通过pactl命令获取的系统音频通道号: " << str_output;
            if (str_output.isEmpty()) {
                if (!m_defaultSinkPath.isEmpty()) {
                    str_output = m_defaultSinkPath.right(1);
                    qInfo() << "通过pacmd命令获取的系统音频通道号失败！自动分配通道号:" << str_output;
                } else {
                    str_output = "-1";
                    qWarning() << "自动分配通道号失败！默认音频输出源服务地址为空！" << m_defaultSinkPath;
                }
            }
        }
        return str_output;
    } else {
        str_output = "-1";
        qWarning() << __FUNCTION__ << __LINE__ << "获取系统音频通道号失败！m_defaultSinkDBusInterface is nullptr or invalid ";
    }
    return str_output;
}

//获取默认输出或输入设备名称
QString AudioUtils::getDefaultDeviceName(DefaultAudioType mode)
{
    QString device = "";
    if (mode == DefaultAudioType::Sink) {
        //1.首先取出默认系统声卡
        if (m_defaultSinkDBusInterface && m_defaultSinkDBusInterface->isValid()) {
            QString device = m_defaultSinkDBusInterface->property("Name").toString();
            qInfo() << "default sink name is : " << device;
            if (!device.isEmpty() && !device.endsWith(".monitor")) {
                device += ".monitor";
            }
        } else {
            qWarning() << __FUNCTION__ << __LINE__ << "m_defaultSinkDBusInterface is nullptr or invalid ";
        }
        //2.如果默认系统声卡不是物理声卡和蓝牙声卡，需找出真实的物理声卡
        if (!device.startsWith("alsa", Qt::CaseInsensitive) && !device.startsWith("blue", Qt::CaseInsensitive)) {
            if (m_audioDBusInterface && m_audioDBusInterface->isValid()) {
                QList<QDBusObjectPath> sinks = m_audioDBusInterface->property("Sinks").value<QList<QDBusObjectPath>>();
                for (int i = 0; i < sinks.size(); i++) {
                    QDBusInterface *realSink = new QDBusInterface(AudioService,
                                                                  sinks[i].path(),
                                                                  SinkInterface,
                                                                  QDBusConnection::sessionBus());
                    if (realSink->isValid()) {
                        device = realSink->property("Name").toString();
                        qInfo() << "realSink name is : " << device;
                        if (device.startsWith("alsa", Qt::CaseInsensitive)) {
                            device += ".monitor";
                            break;
                        } else {
                            device = "";
                        }
                    }
                }
            } else {
                qDebug() << __FUNCTION__ << __LINE__ << "m_audioDBusInterface is nullptr or invalid";
            }
        }
    } else if (mode == DefaultAudioType::Source) {
        if (m_defaultSourceDBusInterface && m_defaultSourceDBusInterface->isValid()) {
            device = m_defaultSourceDBusInterface->property("Name").toString();
            qInfo() << "default source name is : " << device;
            if (device.endsWith(".monitor")) {
                device.clear();
            }
        } else {
            qDebug() << __FUNCTION__ << __LINE__ << "m_defaultSourceDBusInterface is nullptr or invalid";
        }
    } else {
        qCritical() << "The passed parameter is incorrect! Please pass in 1 or 2!";
    }
    return device;
}

//音频dbus服务的接口
QDBusInterface *AudioUtils::audioDBusInterface()
{
    if (m_audioDBusInterface && m_audioDBusInterface->isValid()) {
        return m_audioDBusInterface;
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << "m_audioDBusInterface is nullptr or invalid";
        return nullptr;
    }
}

//音频dbus服务默认输入源的接口
QDBusInterface *AudioUtils::defaultSourceDBusInterface()
{
    if (m_defaultSourceDBusInterface && m_defaultSourceDBusInterface->isValid()) {
        return m_defaultSourceDBusInterface;
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << "m_defaultSourceDBusInterface is nullptr or invalid";
        return nullptr;
    }
}

//音频dbus服务默认输出源的接口
QDBusInterface *AudioUtils::defaultSinkDBusInterface()
{
    if (m_defaultSinkDBusInterface && m_defaultSinkDBusInterface->isValid()) {
        return m_defaultSinkDBusInterface;
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << "m_defaultSinkDBusInterface is nullptr or invalid";
        return nullptr;
    }
}

//音频dbus服务默认输入源的活跃端口
AudioPort AudioUtils::defaultSourceActivePort()
{
    AudioPort port;
    auto inter = new QDBusInterface(AudioService,
                                    m_defaultSourcePath,
                                    PropertiesInterface,
                                    QDBusConnection::sessionBus());

    if (inter->isValid()) {
        //qInfo() << "音频服务： "<< AudioService <<" 默认输入源地址"<< m_defaultSourcePath << " 属性接口："<< PropertiesInterface;
        QDBusReply<QDBusVariant> reply = inter->call("Get", SourceInterface, "ActivePort");
        reply.value().variant().value<QDBusArgument>() >> port;
        //qInfo() << "ActivePort:" << port;
    } else {
        qWarning() << "默认输入源地址 (" << m_defaultSourcePath << ") 不存在";
    }
    delete inter;
    inter = nullptr;
    return port;
}

//音频dbus服务默认输入源的音量
double AudioUtils::defaultSourceVolume()
{
    if (m_defaultSourceDBusInterface && m_defaultSourceDBusInterface->isValid()) {
        return m_defaultSourceDBusInterface->property("Volume").value<double>();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << "m_defaultSourceDBusInterface is nullptr or invalid";
        return 0.0;
    }
}

//音频dbus服务的声卡信息
QString AudioUtils::cards()
{
    if (m_audioDBusInterface && m_audioDBusInterface->isValid()) {
        return m_audioDBusInterface->property("Cards").toString();
    } else {
        qDebug() << __FUNCTION__ << __LINE__ << "m_audioDBusInterface is nullptr or invalid";
        return "";
    }
}

//音频dbus服务属性改变时触发
void AudioUtils::onDBusAudioPropertyChanged(QDBusMessage msg)
{
    QList<QVariant> arguments = msg.arguments();
    //qDebug() << "arguments" << arguments;
    //参数固定长度
    if (3 != arguments.count()) {
        qWarning() << "参数长度不为3！ 参数: " << arguments;
        return;
    }
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName == AudioInterface) {
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys =  changedProps.keys();
        foreach (const QString &prop, keys) {
            //qDebug() << "property: " << prop << changedProps[prop];
            if (prop == QStringLiteral("DefaultSource")) {
                //默认输入源地址改变
                const QDBusObjectPath &defaultSourcePath = qvariant_cast<QDBusObjectPath>(changedProps[prop]);
                if (m_defaultSourcePath != defaultSourcePath.path()) {
                    qInfo() << "默认输入源地址改变:" <<  m_defaultSourcePath << " To " << defaultSourcePath.path();
                    m_defaultSourcePath = defaultSourcePath.path();
                    //发射默认输入源信号
                    emit defaultSourceChanaged();
                    //重新初始化默认输入源接口
                    initDefaultSourceDBusInterface();
                }
            } else if (prop == QStringLiteral("Cards")) {
                //声卡信息改变
                const QString &Cards = qvariant_cast<QString>(changedProps[prop]);
                if (m_audioCards != Cards) {
                    qInfo() << "声卡信息改变:" <<  m_audioCards << " To " << Cards;
                    m_audioCards = Cards;
                    emit cardsChanged(m_audioCards);
                }
            }
        }
    }
}
