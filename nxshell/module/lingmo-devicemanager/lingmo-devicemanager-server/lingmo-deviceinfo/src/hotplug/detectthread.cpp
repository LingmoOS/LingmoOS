// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectthread.h"
#include "monitorusb.h"
#include "DDLog.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QCryptographicHash>

#define LEAST_NUM 10

using namespace DDLog;

DetectThread::DetectThread(QObject *parent)
    : QThread(parent)
    , mp_MonitorUsb(new MonitorUsb())
{
    // 连接槽函数
    connect(mp_MonitorUsb, SIGNAL(usbChanged()), this, SLOT(slotUsbChanged()), Qt::QueuedConnection);

    QMap<QString, QMap<QString, QString>> usbInfo;
    curHwinfoUsbInfo(usbInfo);
    updateMemUsbInfo(usbInfo);
}

void DetectThread::run()
{
    if (mp_MonitorUsb) {
        mp_MonitorUsb->monitor();
    }
}

void DetectThread::setWorkingFlag(bool flag)
{
    mp_MonitorUsb->setWorkingFlag(flag);
    if (flag && !isRunning()) {
        start();
    }
}

void DetectThread::slotUsbChanged()
{
    // 当监听到新的usb时，内核需要加载usb信息，而上层应用需要在内核处理之后获取信息
    // 为了确保缓存信息之前，内核已经处理完毕，先判断内核是否处理完信息，且判断时间不能多于10s
    qint64 begin = QDateTime::currentMSecsSinceEpoch();
    qint64 end = begin;
    while ((end - begin) <= 10000) {
        if (isUsbDevicesChanged())
            break;
        sleep(1);
        end = QDateTime::currentMSecsSinceEpoch();
    }
    qCInfo(appLog) << " 此次判断插拔是否完成的时间为 ************ " << QDateTime::currentMSecsSinceEpoch() - begin;
    emit usbChanged();
}

bool DetectThread::isUsbDevicesChanged()
{
    QMap<QString, QMap<QString, QString>> curUsbInfo;
    curHwinfoUsbInfo(curUsbInfo);

    // 拔出的时候，如果当前的usb设备个数小于m_MapUsbInfo的个数则返回true
    if (curUsbInfo.size() < m_MapUsbInfo.size()) {
        updateMemUsbInfo(curUsbInfo);
        return true;
    }

    // 数量一样或curUsbInfo的大小大于m_MapUsbInfo的大小，则一个一个的比较
    // 如果curUsbInfo里面的在m_MapUsbInfo里面找不到则说明内核信息还没有处理完
    foreach (const QString &key, curUsbInfo.keys()) {
        if (m_MapUsbInfo.find(key) != m_MapUsbInfo.end())
            continue;
        if ("disk" == curUsbInfo[key]["Hardware Class"]
                && curUsbInfo[key].find("Capacity") == curUsbInfo[key].end())
            continue;
        updateMemUsbInfo(curUsbInfo);
        return true;
    }
    return false;
}

void DetectThread::updateMemUsbInfo(const QMap<QString, QMap<QString, QString>> &usbInfo)
{
    m_MapUsbInfo.clear();
    m_MapUsbInfo = usbInfo;
}

void DetectThread::curHwinfoUsbInfo(QMap<QString, QMap<QString, QString>> &usbInfo)
{
    QProcess process;
    process.start("hwinfo --usb --keyboard --mouse");
    process.waitForFinished(-1);
    QString info = process.readAllStandardOutput();

    QStringList items = info.split("\n\n");
    foreach (const QString &item, items) {
        QMap<QString, QString> mapItem;
        if (!getMapInfo(item, mapItem))
            continue;

        QString uniqueID;
        if (mapItem.contains("Unique ID"))
            uniqueID = mapItem["Unique ID"];
        if (mapItem.contains("Vendor") && mapItem.contains("Device") && (mapItem.contains("SysFS ID") || mapItem.contains("SysFS Device Link"))
                && mapItem["Vendor"].contains("0x") && mapItem["Device"].contains("0x")) {
            QStringList vendorlist = mapItem["Vendor"].split(" ");
            QStringList devicelist = mapItem["Device"].split(" ");
            if (vendorlist.size() > 1 && devicelist.size() > 1  && ((mapItem.contains("SysFS ID") && !mapItem["SysFS ID"].isEmpty())
                                                                    || (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()))) {
                QString valueStr = vendorlist[1].trimmed() + devicelist[1].remove("0x", Qt::CaseSensitive).trimmed();
                QCryptographicHash Hash(QCryptographicHash::Md5);
                QByteArray buf;
                buf.append(valueStr);
                if (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()) {
                    buf.append(mapItem["SysFS Device Link"].trimmed());
                } else {
                    buf.append(mapItem["SysFS ID"].trimmed());
                }
                Hash.addData(buf);
                uniqueID = QString::fromStdString(Hash.result().toBase64().toStdString());
            }
        }

        // 使用 Unique ID 作为唯一标识
        usbInfo.insert(uniqueID, mapItem);
    }
}

bool DetectThread::getMapInfo(const QString &item, QMap<QString, QString> &mapInfo)
{
    QStringList lines = item.split("\n");
    // 行数太少则为无用信息
    if (lines.size() <= LEAST_NUM) {
        return false;
    }

    foreach (const QString &line, lines) {
        QStringList words = line.split(": ");
        if (words.size() != 2)
            continue;
        mapInfo.insert(words[0].trimmed(), words[1].trimmed());
    }

    // hub为usb接口，可以直接过滤
    if ("hub" == mapInfo["Hardware Class"]) {
        return false;
    }

    return true;
}
