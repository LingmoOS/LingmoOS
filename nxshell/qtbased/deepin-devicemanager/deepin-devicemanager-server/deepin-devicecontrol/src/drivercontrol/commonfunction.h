// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONFUNCTION_H
#define COMMONFUNCTION_H

// 其它头文件
#include <QString>
#include <QMap>

#include <sys/utsname.h>

static QMap<QString, QString> mapArch = {   {"aarch64", "arm64"}
    , {"x86_64", "amd64"}
    , {"mips64", "mips64el"}
    , {"i386", "i386"}
    , {"sw_64", "sw_64"}
    , {"loongarch", "loongarch"}
    , {"loongarch64", "loongarch64"}
};

/**
 * @brief The DDeviceType enum 可更新驱动类型
 */
enum DriverType {
    DR_Null = 0,
    DR_Bluetooth = 1,
    DR_Camera = 2,
    DR_Gpu = 3,
    DR_Keyboard = 4,
    DR_Sound = 5,
    DR_Mouse = 6,
    DR_Network = 7,
    DR_Printer = 8,
    DR_Scaner = 9,
    DR_Tablet = 10,
    DR_WiFi = 11
};

struct DriverInfo {
    DriverType type;
    QString    vendorId;
    QString    vendorName;
    QString    modelId;
    QString    modelName;
    QString    driverName;
    QString    version;

    QString    debVersion;//包版本
    QString    packages;//包名
};

struct RepoDriverInfo {
    QString strDebManufacturer;//包厂商
    QString strDebVersion;//包版本
    QString strPackages;//包名
    int iLevel;
};


class Common
{
public:
    Common();
    ~Common();
    static QString getArch()
    {
        QString arch;
        struct utsname utsbuf;
        if (-1 != uname(&utsbuf)) {
            arch = QString::fromLocal8Bit(utsbuf.machine);
        }
        return arch;
    }

    static QString getArchStore()
    {
        return mapArch[getArch()];
    }

};

#define EC_NULL 0
#define EC_NETWORK 1
#define EC_CANCEL 2
#define EC_NOTFOUND 3
#define EC_NOTIFY_NETWORK 4  // 仅仅做通知处理，通知前台异常
#define EC_REINSTALL 5 // 通知前台重新安装
#define EC_6 6

#endif // COMMONFUNCTION_H
