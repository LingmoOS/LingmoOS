// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRAPHICSDEVICEINFO_H
#define GRAPHICSDEVICEINFO_H

#include <QSet>
enum DeviceFlag
{
    NoDevice    = 0,
    AMD         = 1,
    NVIDIA      = 1 << 1,
    INTEL       = 1 << 2,
};

class Device
{
public:
    Device(const QString &name, const QString &info, const QString &driver,  bool vga, DeviceFlag flag)
    {
        m_name = name;
        m_info = info;
        m_driver = driver;
        boot_vga = vga;
        m_flag = flag;
    }

    Device(){}
    QString m_name;
    QString m_info;
    QString m_driver;
    bool boot_vga;
    DeviceFlag m_flag; 
};

class GraphicsDeviceInfo
{
public:
    
    Q_DECLARE_FLAGS(DeviceFlags, DeviceFlag)

    explicit GraphicsDeviceInfo();

    int deviceFlag() const { return m_sysDevFlag; }
    int curDeviceFlag() const { return m_curDevFlag; }
    int deviceNums() const;
    Device curDevice() const { return m_curDevice; };
    const QList<Device> devices() const { return m_devices; }
    bool isNotebook();
    static DeviceFlag deviceType(const QString &devInfo);
private:
    void init();

private:
    DeviceFlags m_sysDevFlag;
    DeviceFlags m_curDevFlag;
    QList<Device> m_devices;
    Device m_curDevice;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(GraphicsDeviceInfo::DeviceFlags)

#endif
