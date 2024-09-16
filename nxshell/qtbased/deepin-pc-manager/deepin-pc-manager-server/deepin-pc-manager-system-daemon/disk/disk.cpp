// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "disk.h"

#include "parted/parted.h"

#include <QDebug>
#include <QList>

// 检查是否为可用设备
bool useableDevice(const PedDevice *lpDevice)
{
    Q_ASSERT(nullptr != lpDevice);
    char *buf = static_cast<char *>(malloc(size_t(lpDevice->sector_size)));
    if (!buf)
        return false;

    // Must be able to read from the first sector before the disk device is considered
    // useable in GParted.
    bool success = false;
    int fd = open(lpDevice->path, O_RDONLY | O_NONBLOCK);
    if (fd >= 0) {
        ssize_t bytesRead = read(fd, buf, size_t(lpDevice->sector_size));
        success = (bytesRead == lpDevice->sector_size);
        close(fd);
    }
    free(buf);
    return success;
}

// 获取磁盘设备路径列表
QList<QString> getDiskDevicePathList()
{
    QList<QString> devicepaths{};

    ped_device_probe_all();
    PedDevice *lpDevice = ped_device_get_next(nullptr);
    while (lpDevice) {
        if (useableDevice(lpDevice)) {
            devicepaths.append(lpDevice->path);
        }
        lpDevice = ped_device_get_next(lpDevice);
    }

    return devicepaths;
}
