// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H

#include <QThread>
#include <QMap>
#include <QDateTime>

class MonitorUsb;

/**
 * @brief The DetectThread class
 */
class DetectThread : public QThread
{
    Q_OBJECT
public:
    explicit DetectThread(QObject *parent);

    /**
     * @brief run
     */
    void run() override;

    /**
     * @brief setWorkingFlag 设置工作状态
     */
    void setWorkingFlag(bool flag);

signals:
    /**
     * @brief usbChanged
     */
    void usbChanged();

private slots:
    /**
     * @brief slotUsbChanged usb发生变化时的曹函数处理
     */
    void slotUsbChanged();

private:
    /**
     * @brief isUsbDevicesChanged 判断hwinfo --usb信息是否发生变化
     * @return
     */
    bool isUsbDevicesChanged();

    /**
     * @brief updateMemUsbInfo 更新
     * @param usbInfo
     */
    void updateMemUsbInfo(const QMap<QString, QMap<QString, QString>> &usbInfo);

    /**
     * @brief curHwinfoUsbInfo 获取当前的usb信息
     * @param usbInfo
     */
    void curHwinfoUsbInfo(QMap<QString, QMap<QString, QString>> &usbInfo);

    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    bool getMapInfo(const QString &item, QMap<QString, QString> &mapInfo);

private:
    MonitorUsb *mp_MonitorUsb; //<! udev检测任务
    QMap<QString, QMap<QString, QString>> m_MapUsbInfo; //<! usb信息
};

#endif // DETECTTHREAD_H
