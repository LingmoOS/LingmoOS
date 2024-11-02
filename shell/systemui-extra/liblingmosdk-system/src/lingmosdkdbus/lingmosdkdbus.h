/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef LINGMOSDKDBUS_H
#define LINGMOSDKDBUS_H

/**
 * @file lingmosdkdbus.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief sdk系统层信息生成dbus信号
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-system-dbus
 * @{
 * 
 */

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QtDebug>
#include <string.h>

#include <memory>

#include "../systeminfo/libkysysinfo.h"     // 分辨率
#include "../hardware/libkync.h"         // 网卡
#include "../hardware/libkybios.h"         // BIOS
#include "../hardware/libkydisplay.h"         // display
#include "../disk/libkydiskinfo.h"      // 磁盘
#include "../proc/libkyrtinfo.h"        // 进程
#include "../proc/libkyprocess.h"      // 进程
#include "../hardware/libkyboard.h"        // 主板
#include "../net/libkynetinfo.h"       // 网络
#include "../realtime/libkyrealtimeinfo.h"  // 运行时信息
#include "../hardware/libkyusb.h"           // USB外设
#include "../hardware/libkycpu.h"           //CPU
#include "../hardware/libkyprinter.h"       //打印
#include "../location/libkylocation.h"      //地理位置
#include "../packages/libkypackages.h"        //包
#include "../accounts/libkyaccounts.h"        //包

/**
 * @brief 生成磁盘信息dbus信号类
 * 主要用来生成磁盘的列表，磁盘的扇区数量，每个扇区的字节数，容量，型号、序列号、子分区数量，类型和固件版本信息的dbus信号
 */
class KySdkDisk : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.disk")
public:
    explicit KySdkDisk();
    ~KySdkDisk();
public slots:
    /**
     * @brief 获取所有磁盘的列表
     *
     * @return QStringList 磁盘的列表，每个字符串表示一个磁盘的绝对路径
     */
    QStringList getDiskList() const;

    /**
     * @brief 获取磁盘的每个扇区的字节数
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return int 每个扇区的字节数
     */
    unsigned int getDiskSectorSize(const QString diskname) const;

    /**
     * @brief 获取磁盘容量
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return double 磁盘容量，MiB为单位
     */
    double getDiskTotalSizeMiB(const QString diskname) const;

    /**
     * @brief 获取磁盘型号
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 磁盘型号
     */
    QString getDiskModel(const QString diskname) const;

    /**
     * @brief 获取磁盘序列号
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 磁盘序列号
     */
    QString getDiskSerial(const QString diskname) const;

    /**
     * @brief 获取磁盘/分区下的子分区数量
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return unsigned int 该磁盘/分区下的子分区数量
     */
    unsigned int getDiskPartitionNums(const QString diskname) const;

    /**
     * @brief 获取磁盘类型
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 磁盘类型，固态 or 机械 or 混合
     */
    QString getDiskType(const QString diskname) const;

    /**
     * @brief 获取磁盘固件版本信息
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 固件版本信息
     */
    QString getDiskVersion(const QString diskname) const;

    /**
     * @brief 获取磁盘扇区数量
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return unsigned long long 扇区数量
     */
    unsigned long long getDiskSectorNum(const QString diskname) const;

    /**
     * @brief 获取硬盘大小
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 硬盘大小
     */
    QString getHardDiskSize(const QString diskname) const;

    /**
     * @brief 获取硬盘固件版本
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 硬盘固件版本
     */
    QString getHardFwrev(const QString diskname) const;

    /**
     * @brief 获取硬盘类型
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 硬盘类型
     */
    QString getHardType(const QString diskname) const;

    /**
     * @brief 获取硬盘型号
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 硬盘型号
     */
    QString getHardModel(const QString diskname) const;

    /**
     * @brief 获取硬盘序列号
     *
     * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return QString 硬盘序列号
     */
    QString getHardSerial(const QString diskname) const;
};

/**
 * @brief 生成cpu信息dbus信号类
 * 主要用来生成cpu的架构、制造厂商、型号、额定主频、核心数量、虚拟化支持和线程数信息dbus信号
 */
class KySdkCpuInfo : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.cpuinfo")
public:
    explicit KySdkCpuInfo();
    ~KySdkCpuInfo();
public slots:
    /**
     * @brief 获取cpu架构
     *
     * @return QString 架构信息，如"x86_64"
     */
    QString getCpuArch() const;

    /**
     * @brief 获取cpu制造厂商
     *
     * @return QString 制造厂商，如“GenuineIntel”
     */
    QString getCpuVendor() const;

    /**
     * @brief 获取cpu型号
     *
     * @return QString CPU型号名称，如“Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz”
     */
    QString getCpuModel() const;

    /**
     * @brief 获取cpu额定主频
     *
     * @return QString 额定主频赫兹数，如“1794.742”
     */
    QString getCpuFreqMHz() const;

    /**
     * @brief 获取cpu核心数量
     *
     * @return unsigned int 所有可用的CPU核心数量
     */
    unsigned int getCpuCorenums() const;

    /**
     * @brief 获取cpu对虚拟化的支持
     *
     * @return QString 若CPU支持虚拟化，则返回虚拟化技术，如“vmx”；若不支持，返回NULL
     */
    QString getCpuVirt() const;

    /**
     * @brief 获取cpu线程数
     *
     * @return unsigned int cpu支持的线程数
     */
    unsigned int getCpuProcess() const;
};

/**
 * @brief 生成网卡信息dbus信号类
 * 主要用来生成网卡列表、检测指定网卡是否处于UP状态、获取系统中当前处于 link up 状态的网卡列表、获取指定网卡的物理MAC地址、
 * 获取指定网卡的第一个IPv4地址、获取指定网卡的所有IPv4地址、获取指定网卡的第一个IPv6地址、获取指定网卡的所有IPv6地址、
 * 获取指定网卡的有线/无线类型、获取指定网卡的厂商名称和设备型号的dbus信号
 */
class KySdkNetCard : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.netcard")
public:
    explicit KySdkNetCard();
    ~KySdkNetCard();
public slots:
    /**
     * @brief 获取系统中所有的网卡
     *
     * @return QStringList 网卡名称列表
     */
    QStringList getNetCardName() const;

    /**
     * @brief 检测指定网卡是否处于UP状态
     *
     * @param netCardName 网卡名称，如eno1
     * @return int Up返回1，Down返回0
     */
    int getNetCardUp(const QString netCardName) const;

    /**
     * @brief 获取系统中当前处于 link up 状态的网卡列表
     *
     * @return QStringList 处于link up状态的网卡列表
     */
    QStringList getNetCardUpcards() const;

    /**
     * @brief 获取指定网卡的物理MAC地址
     *
     * @param netCardName 网卡名称，如eno1
     * @return QString 物理MAC地址
     */
    QString getNetCardPhymac(const QString netCardName) const;

    /**
     * @brief 获取指定网卡的第一个IPv4地址
     *
     * @param netCardName 网卡名称，如eno1
     * @return QString IPv4地址
     */
    QString getNetCardPrivateIPv4(const QString netCardName) const;

    /**
     * @brief 获取指定网卡的所有IPv4地址
     *
     * @param netCardName 网卡名称，如eno1
     * @return QStringList IPv4地址列表
     */
    QStringList getNetCardIPv4(const QString netCardName) const;

    /**
     * @brief 获取指定网卡的第一个IPv6地址
     *
     * @param netCardName 网卡名称，如eno1
     * @return QString IPv6地址
     */
    QString getNetCardPrivateIPv6(const QString netCardName) const;

    /**
     * @brief 获取指定网卡的有线/无线类型
     *
     * @param netCardName 网卡名称，如eno1
     * @return int 0 有线 1 无线
     */
    int getNetCardType(const QString netCardName) const;

    /**
     * @brief 获取指定网卡的厂商名称和设备型号
     * 
     * @param netCardName 网卡名称，如eno1
     * @return QStringList 网卡的厂商名称和设备型号
     */
    QStringList getNetCardProduct(const QString netCardName) const;

    /**
     * @brief 获取指定网卡的所有IPv6地址
     *
     * @param netCardName 网卡名称，如eno1
     * @return QStringList IPv6地址列表
     */
    QStringList getNetCardIPv6(const QString netCardName) const;
};

/**
 * @brief 生成bios信息dbus信号类
 * 主要用来生成bios的厂商和版本信息dbus信号类
 */
class KySdkBios : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.bios")
public:
    explicit KySdkBios();
    ~KySdkBios();
public slots:
    /**
     * @brief 获取bios厂商
     *
     * @return QString 厂商名
     */
    QString getBiosVendor() const;

    /**
     * @brief 获取bios版本
     *
     * @return QString 版本号
     */
    QString getBiosVersion() const;
};

/**
 * @brief 生成主板信息dbus信号类
 * 主要用来生成主板型号、生产日期、序列号、厂商信息dbus信号类
 */
class KySdkMainBoard : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.mainboard")
public:
    explicit KySdkMainBoard();
    ~KySdkMainBoard();
public slots:
    /**
     * @brief 获取主板型号
     *
     * @return QString 主板型号
     */
    QString getMainboardName() const;

    /**
     * @brief 获取主板生产日期
     *
     * @return QString 主板生产日期
     */
    QString getMainboardDate() const;

    /**
     * @brief 获取主板序列号
     *
     * @return QString 主板序列号
     */
    QString getMainboardSerial() const;

    /**
     * @brief 获取主板厂商
     *
     * @return QString 主板厂商名
     */
    QString getMainboardVendor() const;
};

/**
 * @brief 生成usb设备信息dbus信号类
 * 主要用来生成usb设备的名称、类型、pid、vid、序列号、路径信息dbus信号
 */
class KySdkPeripheralsEnum : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.peripheralsenum")
public:
    explicit KySdkPeripheralsEnum();
public slots:
    /**
     * @brief 获取外接usb设备信息
     * 
     * @return QStringList usb设备名称、类型、pid、vid、序列号、路径信息列表
     */
    QStringList getAllUsbInfo() const;

    /**
     * @brief 获取usb设备的产品信息
     * 
     * @return QString  usb设备的产品信息
     */
    QString getUsbProductName(int busNum, int devAddress) const;

    /**
     * @brief 获取usb设备厂商信息
     * 
     * @return QString usb设备的厂商信息
     */
    QString getUsbManufacturerName(int busNum, int devAddress) const;
};

class KySdkDisplay : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.display")
public:
    explicit KySdkDisplay();
    ~KySdkDisplay();
public slots:
    /**
     * @brief 获取显卡的制造商
     *
     * @return QString 显卡的制造商
     */
    QString getDisplayVendor() const;

    /**
     * @brief 获取显卡的型号
     *
     * @return QString 显卡的型号
     */
    QString getDisplayProduct() const;

    /**
     * @brief 获取显卡的说明
     *
     * @return QString 显卡的说明
     */
    QString getDisplayDescription() const;

    /**
     * @brief 获取显卡的物理 id
     *
     * @return QString 显卡的物理 id
     */
    QString getDisplayPhysicalId() const;

    /**
     * @brief 获取显卡的总线地址
     *
     * @return QString 显卡的总线地址
     */
    QString getDisplayBusInfo() const;

    /**
     * @brief 获取显卡的设备版本
     *
     * @return QString 显卡的设备版本
     */
    QString getDisplayVersion() const;

    /**
     * @brief 获取显卡的数据宽度
     *
     * @return QString 显卡的数据宽度
     */
    QString getDisplayWidth() const;

    /**
     * @brief 获取显卡的频率
     *
     * @return QString 显卡的频率
     */
    QString getDisplayClock() const;

    /**
     * @brief 获取显卡的功能
     *
     * @return QString 显卡的功能
     */
    QString getDisplayCapabilities() const;

    /**
     * @brief 获取显卡的配置
     *
     * @return QString 显卡的配置
     */
    QString getDisplayConfiguration() const;

    /**
     * @brief 获取显卡的资源
     *
     * @return QString 显卡的资源
     */
    QString getDisplayResources() const;
};

/**
 * @brief 获取包列表信息类
 * 主要用来获取系统中所有包列表、获取系统中指定包的版本号、检测指定包名的软件包是否正确安装
 */
class KySdkPackageInfo : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.packageinfo")
public:
    explicit KySdkPackageInfo();
public slots:
    /**
     * @brief 获取系统中所有包列表
     * 
     * @return QStringList 系统中所有包列表
     */
    QStringList getPackageList() const;

    /**
     * @brief 获取系统中指定包的版本号
     * 
     * @param packageName 软件包名
     * @return QString 版本号
     */
    QString getPackageVersion(const QString packageName) const;

    /**
     * @brief 检测指定包名的软件包是否正确安装
     * 
     * @param packageName 包名
     * @param version 版本号，大部分情况下为NULL，预留为snap做准备
     * @return int 成功返回0，失败返回错误码
     */
    int getPackageInstalled(const QString packageName, const QString version) const;
};

/**
 * @brief 获取当前系统资源占用信息类
 * 主要用来获取系统中物理内存总大小、物理内存使用率、物理内存使用大小、实际可用的物理内存大小、实际空闲的物理内存大小、
 * 所有应用申请的虚拟内存总量、系统中Swap分区总大小、Swap分区使用率、Swap分区使用量、Swap分区空闲大小、CPU瞬时使用率、
 * 操作系统开机时长dbus信息
 */
class KySdkResource : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.resource")
public:
    explicit KySdkResource();
public slots:
    /**
     * @brief 获取系统中物理内存总大小
     * 
     * @return double 物理内存大小，KiB为单位
     */
    double getMemTotalKiB() const;

    /**
     * @brief 获取物理内存使用率
     * 
     * @return double 物理内存使用率
     */
    double getMemUsagePercent() const;

    /**
     * @brief 获取物理内存使用大小，注意Buffer/Cache被计算为已使用内存
     * 
     * @return double 物理内存使用大小，KiB为单位
     */
    double getMemUsageKiB() const;

    /**
     * @brief 获取实际可用的物理内存大小，该数值约等于Free + Buffer + Cache
     * 
     * @return double 可用物理内存大小，KiB为单位
     */
    double getMemAvailableKiB() const;

    /**
     * @brief 获取实际空闲的物理内存大小，注意Buffer/Cache被计算为已使用内存
     * 
     * @return double 空闲的物理内存大小，KiB为单位
     */
    double getMemFreeKiB() const;

    /**
     * @brief 获取所有应用申请的虚拟内存总量
     * 
     * @return double 虚拟内存总申请量，KiB为单位
     */
    double getMemVirtAllocKiB() const;

    /**
     * @brief 获取系统中Swap分区总大小
     * 
     * @return double Swap分区大小，KiB为单位
     */
    double getMemSwapTotalKiB() const;

    /**
     * @brief 获取Swap分区使用率
     * 
     * @return double Swap分区使用率
     */
    double getMemSwapUsagePercent() const;

    /**
     * @brief 获取Swap分区使用量
     * 
     * @return double Swap分区使用量，KiB为单位
     */
    double getMemSwapUsageKiB() const;

    /**
     * @brief 获取Swap分区空闲大小
     * 
     * @return double Swap分区空闲大小，KiB为单位
     */
    double getMemSwapFreeKiB() const;

    /**
     * @brief 获取CPU瞬时使用率
     * 
     * @return double CPU瞬时使用率，该值 < 1.00
     */
    double getCpuCurrentUsage() const;

    /**
     * @brief 获取操作系统开机时长
     * 
     * @return QString 开机时长
     */
    QString getUpTime() const;
};

/**
 * @brief 生成某一进程的瞬时详细信息dbus信号类
 * 主要用来生成某一进程的CPU使用率、获取某一进程的内存占用率、获取某一进程的进程状态、获取某一进程的端口号占用、
 * 获取某一进程的启动时间、获取某一进程的运行时间、获取某一进程的cpu时间、获取某一进程的Command、获取某一进程的属主、
 * 获取某一进程的信息、获取进程所有信息dbus信号类
 */
class KySdkProcess : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.process")
public:
    explicit KySdkProcess();
public slots:
    /**
     * @brief 获取某一进程的CPU使用率
     * 
     * @param pid 进程号
     * @return double 进程的CPU使用率
     */
    double getProcInfoCpuUsage(int pid) const;

    /**
     * @brief 获取某一进程的内存占用率
     * 
     * @param pid 进程号
     * @return double 进程的内存占用率
     */
    double getProcInfoMemUsage(int pid) const;

    /**
     * @brief 获取某一进程的进程状态
     * 
     * @param pid 进程号
     * @return QString 进程状态
     */
    QString getProcInfoStatus(int pid) const;

    /**
     * @brief 获取某一进程的端口号占用
     * 
     * @param pid 进程号
     * @return int 进程使用的端口号
     */
    int getProcInfoPort(int pid) const;

    /**
     * @brief 获取某一进程的启动时间
     * 
     * @param pid 进程号
     * @return QString 进程的启动时间
     */
    QString getProcInfoStartTime(int pid) const;

    /**
     * @brief 获取某一进程的运行时间
     * 
     * @param pid 进程号
     * @return QString 进程的运行时间
     */
    QString getProcInfoRunningTime(int pid) const;

    /**
     * @brief 获取某一进程的cpu时间
     * 
     * @param pid 进程号
     * @return QString 进程的cpu时间
     */    
    QString getProcInfoCpuTime(int pid) const;

    /**
     * @brief 获取某一进程的Command
     * 
     * @param pid 进程号
     * @return QString 进程的Command
     */
    QString getProcInfoCmd(int pid) const;

    /**
     * @brief 获取某一进程的属主
     * 
     * @param pid 进程号
     * @return QString 进程的属主
     */
    QString getProcInfoUser(int pid) const;

    /**
     * @brief 获取某一进程的信息
     * 
     * @param procName 进程名
     * @return QStringList 某一进程CPU使用率、内存占用率、状态、端口号占用、启动时间、运行时间、cpu时间、Command、属主信息列表
     */
    QStringList getProcInfo(const QString procName) const;

    /**
     * @brief 获取进程所有信息
     * 
     * @return QStringList 所有进程的CPU使用率、内存占用率、状态、端口号占用、启动时间、运行时间、cpu时间、Command、属主信息列表
     */
    QStringList getAllProcInfo() const;
};

/**
 * @brief 获取操作系统基础信息类
 * 主要用来获取操作系统架构信息、操作系统名称、操作系统版本号、操作系统激活状态、操作系统服务序列号、内核版本号、
 * 当前登录用户的用户名（Effect User）、操作系统项目编号名、操作系统项目子编号名、操作系统产品标识码、操作系统宿主机的虚拟机类型、
 * 操作系统宿主机的云平台类型、判断当前镜像系统是否为 专用机 系统、系统版本号/补丁版本号、系统当前显示屏幕,分辨率,系统支持分辨率
 */
class KySdkResolution : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.sysinfo")
public:
    explicit KySdkResolution();
public slots:
    /**
     * @brief 获取操作系统架构信息
     * 
     * @return QString 操作系统架构信息
     */
    QString getSystemArchitecture() const;

    /**
     * @brief 获取操作系统名称
     * 
     * @return QString 操作系统名称
     */
    QString getSystemName() const;

    /**
     * @brief 获取操作系统版本号
     * 
     * @param verbose 0获取简略版本号，1获取详细版本号
     * @return QString 操作系统版本号
     */
    QString getSystemVersion(bool verbose) const;

    /**
     * @brief 获取操作系统激活状态
     * 
     * @return int 2表示已过期；0表示未激活，处于试用期；1表示已激活；-1表示接口内部错误；
     */
    int getSystemActivationStatus() const;

    /**
     * @brief 获取操作系统服务序列号
     * 
     * @return  QString 操作系统服务序列号
     */
    QString getSystemSerialNumber() const;

    /**
     * @brief 获取内核版本号
     * 
     * @return QString 内核版本号
     */
    QString getSystemKernelVersion() const;

    /**
     * @brief 获取当前登录用户的用户名（Effect User）
     * 
     * @return QString 当前登录用户的用户名（Effect User）
     */
    QString getSystemEffectUser() const;

    /**
     * @brief 获取当前登录用户的登录时间
     * 
     * @return QString 成功返回字符串，失败返回NULL。返回的字符串需要被 free 释放
     */
    QString getSystemEffectUserLoginTime() const;

    /**
     * @brief 获取操作系统项目编号名
     * 
     * @return QString 操作系统项目编号名
     */
    QString getSystemProjectName() const;

    /**
     * @brief 获取操作系统项目子编号名
     * 
     * @return QString 操作系统项目子编号名
     */
    QString getSystemProjectSubName() const;

    /**
     * @brief 获取操作系统产品标识码
     * 
     * @return unsigned int 返回标志码
     *         0000 信息异常
     *         0001 仅PC特性
     *         0010 仅平板特性
     *         0011 支持平板与PC特性
     */
    unsigned int getSystemProductFeatures() const;

    /**
     * @brief 获取操作系统宿主机的虚拟机类型
     * 
     * @return QString 操作系统宿主机的虚拟机类型，类型如下：
     *         [none, qemu, kvm, zvm, vmware, hyper-v, orcale virtualbox, xen, bochs, \
     *          uml, parallels, bhyve, qnx, arcn, openvz, lxc, lxc-libvirt, systemd-nspawn,\
     *          docker, podman, rkt, wsl]
     *         其中 none 表示运行在物理机环境中；其他字符串代表具体的虚拟环境类型
     */
    QString getSystemHostVirtType() const;

    /**
     * @brief 获取操作系统宿主机的云平台类型
     * 
     * @return QString 获取失败返回NULL，获取成功返回一个字符串，字符串内容如下：
     *               [none, huawei]
     *               其中 none 表示运行在物理机或未知的云平台环境中；其他字符串代表不同的云平台
     */
    QString getSystemHostCloudPlatform() const;

    /**
     * @brief  获取系统版本号
     * 
     * @return QString 系统版本号
     */
    QString getSystemOSVersion() const;

    /**
     * @brief  获取补丁版本号
     * 
     * @return QString 补丁版本号
     */
    QString getSystemUpdateVersion() const;

    /**
     * @brief  判断当前镜像系统是否为 专用机 系统
     * 
     * @return bool true代表是 false代表不是
     */
    bool getSystemIsZYJ() const;

    /**
     * @brief  获取整机序列号
     * 
     * @return QString 整机序列号
     */
    QString getHostSerial() const;
};

/**
 * @brief 生成网络信息dbus信号类
 * 主要用来生成指定网络端口的状态、区间端口状态、网关信息、防火墙信息dbus信号
 */
class KySdkNet : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.net")
public:
    explicit KySdkNet();

public slots:
    /**
     * @brief 获取指定网络端口的状态
     * 
     * @param port 端口号
     * @return int 端口状态 TCP_ESTABLISHED:0x1   TCP_SYN_SENT:0x2    TCP_SYN_RECV:0x3    TCP_FIN_WAIT1:0x4    TCP_FIN_WAIT2:0x5
     * TCP_TIME_WAIT:0x6    TCP_CLOSE:0x7    TCP_CLOSE_WAIT:0x8    TCP_LAST_ACL:0x9    TCP_LISTEN:0xa    TCP_CLOSING:0xb
     */
    int getPortState(int port) const;

    /**
     * @brief 获取区间端口状态
     * 
     * @param start 开始端口
     * @param end 结束端口
     * @return QStringList 区间端口状态列表
     */
    QStringList getMultiplePortStat(int start, int end) const;

    /**
     * @brief 获取网关信息
     *
     * @return QStringList 网关信息网卡名和地址列表
     */
    QStringList getGatewayInfo() const;

    /**
     * @brief 获取防火墙信息
     * 
     * @return QStringList  防火墙信息列表
     */
    QStringList getFirewallState() const;

    /**
     * @brief 获取进程对应端口
     * 
     * @return QStringList  进程对应端口
     */
    QStringList getProcPort() const;
};

/**
 * @brief 生成瞬时信息dbus信号类
 * 主要用来生成瞬时网速，cpu温度、磁盘温度、磁盘转速dbus信号
 */
class KySdkRunInfo : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.runinfo")
public:
    explicit KySdkRunInfo();
public slots:
    /**
     * @brief 获取瞬时网速信息
     * 
     * @param nc 网卡名称，如eno1
     * @return double 瞬时网速
     */
    double getNetSpeed(const QString nc) const;

    /**
     * @brief 获取磁盘转速
     * 
     * @param diskpath 磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return int 磁盘转速
     */
    int getDiskRate(const QString diskpath) const;

    /**
     * @brief 获取CPU温度
     * 
     * @return double CPU温度
     */
    double getCpuTemperature() const;

    /**
     * @brief 获取磁盘温度
     * 
     * @param diskpath 磁盘名称，应当是例如/dev/sda这种绝对路径
     * @return double 磁盘温度
     */
    double getDiskTemperature(const QString diskpath) const;
};

/**
 * @brief 生成地址信息dbus信号类
 * 主要用来生成本地地址信息dbus信号
 */
class KySdkGps : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.gps")
public:
    explicit KySdkGps();
public slots:
    /**
     * @brief 获取本地地址信息
     *
     * @return QString json格式的地址信息
     */
    QString getGPSInfo() const;
private:
};

/**
 * @brief 生成打印信息dbus信息类
 * 主要用来生成本地的所有打印机(包含可用打印机和不可用打印机)、获取本地的所有可用打印机(打印机处于idle状态)、
 * 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印、
 * 下载网络文件、打印本地文件、清除某个打印机的所有打印队列、获取当前打印机状态(状态不是实时更新)、
 * 获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）、获取url内的文件名dbus信息
 */
class KySdkPrint : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.print")
public:
    explicit KySdkPrint();
public slots:
    /**
     * @brief 获取本地的所有打印机(包含可用打印机和不可用打印机)
     * 
     * @return QStringList 地的所有打印机名称列表
     */
    QStringList getPrinterList() const;

    /**
     * @brief 获取本地的所有可用打印机(打印机处于idle状态)
     * 
     * @return QStringList 本地的所有可用打印机名称列表
     */
    QStringList getPrinterAvailableList() const;

    /**
     * @brief 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印
     * 
     * @param number_up cups属性，一张纸打印几页，如2 4等
     * @param media cups属性，纸张类型，如A4
     * @param number_up_layout cups属性，如lrtb
     * @param sides cups属性，单面如one-sided，双面如two-sided-long-edge(长边翻转),two-sided-short-edge(短边翻转)
     */
    void setPrinterOptions(int number_up, const QString media, const QString number_up_layout, const QString sides) const;
    
    /**
     * @brief 打印本地文件
     * 
     * @param printername 打印机名
     * @param filepath 文件绝对路径
     * 
     * @return int 打印作业的id
     * 
     * @notice 虽然支持多种格式的打印，但是打印除pdf之外的格式打印的效果都不够好，建议打印pdf格式的文件
     *         打印机处于stop状态函数返回0
     *         格式不对的文件可以成功创建打印任务，但是打印不出来。什么叫格式不对，举个例子，比如当前要打印日志文件a.log,
     *         然后把a.log改名叫a.pdf,这个时候a.pdf就变成了格式不对的文件，打印不出来了
     * 
     */
    int getPrinterPrintLocalFile (const QString printername, const QString filepath) const;

    /**
     * @brief 清除某个打印机的所有打印队列
     * 
     * @param printername 打印机名
     * @return int 成功返回0,失败返回-1
     */
    int getPrinterCancelAllJobs(const QString printername) const;

    /**
     * @brief 获取当前打印机状态(状态不是实时更新)
     * 
     * @param printername 打印机名
     * @return int 状态码
     */
    int getPrinterStatus(const QString printername) const;

    /**
     * @brief  获取url内的文件名
     * 
     * @param  url 路径
     * @return String 解析出来的函数名
     */
    QString getPrinterFilename(const QString url) const;

    /**
     * @brief 获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）
     * 
     * @param printername 打印机名
     * @param jobid 打印作业的id
     * @return int 状态码
     */
    int getPrinterJobStatus(const QString printername, int jobid) const;

    /**
     * @brief 下载网络文件
     * 
     * @param url 要打印的url
     * @param filepath 保存的文件路径
     * 
     * @return int 打印作业的id
     */
    int getPrinterDownloadRemoteFile(const QString url, const QString filepath) const;
};

// class KySdkRest : public QObject
// {
//     Q_OBJECT
//     Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.resolution")
// public:
//     explicit KySdkRest();
//     ~KySdkRest();
// public slots:
//     QStringList getSysLegalResolution() const;
// };

class KySdkAccounts : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.accounts")
public:
    explicit KySdkAccounts();
public slots:
    bool changePassword(const QString userName, const QString passWord) const;
};

#endif // LINGMOSDKDBUS_H
