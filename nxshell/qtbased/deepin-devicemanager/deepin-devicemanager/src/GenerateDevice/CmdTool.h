// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMDTOOL_H
#define CMDTOOL_H

#include <QObject>
#include <QMap>
#include <QProcess>
#include <QFile>
#include <cups.h>

#include <DWidget>
#include <DSysInfo>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

/**
 * @brief The CmdTool class
 * 用于获取设备信息的类，主要执行命令获取信息，然后解析生成对应的map
 */

class CmdTool
{
public:
    CmdTool();
    /**
     * @brief loadCmdInfo:通过命令获取设备信息,[loadCmdInfo]:一般的处理方式
     * @param key:与命令对应的关键字
     * @param debugFile:调试时所需文件名
     */
    void loadCmdInfo(const QString &key, const QString &debugFile);

    /**
     * @brief getCurNetworkLinkStatus:lshw -C network获取当前连接状态
     * @return
     */
    QString getCurNetworkLinkStatus(QString driverName);

    /**
     * @brief getCurPowerInfo:upower --dump获取电池信息
     * @return
     */
    QMap<QString, QMap<QString, QString>> getCurPowerInfo();

    /**
     * @brief cmdInfo:获取命令的解析结果
     * @return 解析结果以map形式返回
     */
    QMap<QString, QList<QMap<QString, QString> > > &cmdInfo();


    /**
     * @brief loadOemTomlFileName: 加载厂商适配信息
     */
    QString loadOemTomlFileName(const QMap<QString, QString> &mapInfo);

        /**
     * @brief parseOemTomlInfo: 解析并加载厂商适配信息
     */
    bool parseOemTomlInfo(const QString filename);

private:

    /**
     * @brief loadLshwInfo:文件加载lshw获取的信息
     * @param debugFile:调试文件名
     */
    void loadLshwInfo(const QString &debugFile);

    /**
     * @brief loadLsblkInfo:加载lsblk -d -o name,rota获取的信息
     * @param debugfile:调试文件名
     */
    void loadLsblkInfo(const QString &debugfile);

    /**
     * @brief loadLssgInfo:加载ls /dev/sg*获取的信息
     * @param debugfile:调试文件名
     */
    void loadLssgInfo(const QString &debugfile);


    /**
     * @brief loadSmartCtlInfo:加载smartctl获取的信息
     * @param logicalName:逻辑名称
     * @param debugfile:调试文件名
     */
    void loadSmartCtlInfo(const QString &logicalName, const QString &debugfile);

    /**
     * @brief loadDmesgInfo:加载dmesg获取的信息
     * @param debugfile:调试文件名
     */
    void loadDmesgInfo(const QString &debugfile);

    /**
     * @brief loadHciconfigInfo:加载hciconfig -a获取的信息
     * @param debugfile:调试文件名
     */
    void loadHciconfigInfo(const QString &debugfile);

    /**
     * @brief loadBluetoothCtlInfo:这个函数是对LoadHciconfigInfo的扩展
     * @param mapInfo:hciconfig -a获取的信息map
     */
    void loadBluetoothCtlInfo(QMap<QString, QString> &mapInfo); // 这个函数是对LoadHciconfigInfo的扩展

    /**
     * @brief loadPrinterInfo:加载打印机信息
     */
    void loadPrinterInfo();

    /**
     * @brief loadHwinfoInfo:加载hwinfo信息
     * @param key:与cmd对应的关键字
     * @param debugfile:调试文件名
     */
    void loadHwinfoInfo(const QString &key, const QString &debugfile);

    /**
     * @brief getMulHwinfoInfo : 解析 hwinfo --sound --network --keyboard --cdrom --disk --display --mouse --usb
     * @param mapLstMap
     */
    void getMulHwinfoInfo(const QString &info);

    /**
     * @brief addWidthToMap
     * @param mapInfo
     */
    void addWidthToMap(QMap<QString, QString> &mapInfo);

    /**
     * @brief getRemoveInfo
     * @param lstMap
     */
    void getRemoveAuthInfo(const QString &info, QList<QMap<QString, QString>> &lstMap);

    /**
     * @brief getAuthorizedInfo
     * @param lstAuth
     */
    void getAuthorizedInfo(QStringList &lstAuth);

    /**
     * @brief updateMapInfo
     * @param removeLstMap
     * @param mapInfo
     * @return
     */
    void updateMapInfo(QList<QMap<QString, QString>> &removeLstMap, QMap<QString, QString> &mapInfo);

    /**
     * @brief clearUsbDevice
     * @param removeLstMap
     */
    void clearUsbDevice(QList<QMap<QString, QString>> &removeLstMap);

    /**
     * @brief loadDmidecodeInfo:加载dmidecode信息
     * @param key:与cmd对应的关键字
     * @param debugfile:调试文件名
     */
    void loadDmidecodeInfo(const QString &key, const QString &debugfile);

    /**
     * @brief loadDmidecode2Info:加载dmidecode -t 2信息
     * @param key:dmidecode2
     * @param debugfile:调试文件名
     */
    void loadDmidecode2Info(const QString &key, const QString &debugfile);

    /**
     * @brief loadLscpuInfo load lscpu info
     * @param key
     * @param debugfile
     */
    void loadLscpuInfo(const QString &key, const QString &debugfile);

    /**
     * @brief loadCatInfo:加载cat xxx信息
     * @param key:与cmd对应的关键字
     * @param debugfile:调试文件名
     */
    void loadCatInfo(const QString &key, const QString &debugfile);

    /**
     * @brief loadUpowerInfo:加载upower --dump
     * @param key:upower
     * @param debugfile:调试文件名
     */
    void loadUpowerInfo(const QString &key, const QString &debugfile);

    /**
     * @brief loadBiosInfoFromLspci:从lspci加载BIOS芯片组信息
     * @param chipsetFamliy:【out】芯片组信息
     */
    void loadBiosInfoFromLspci(QString &chipsetFamliy);

    /**
     * @brief loadCatInputDeviceInfo:加载cat /proc/bus/input/devices信息
     * @param key:cat_devices
     * @param debugfile:调试文件名
     */
    void loadCatInputDeviceInfo(const QString &key, const QString &debugfile);

    /**
     * @brief loadCatAudioInfo:加载cat /proc/asound/card0/codec#0信息
     * @param key:cat_audio
     * @param debugfile:调试文件名
     */
    void loadCatAudioInfo(const QString &key, const QString &debugfile);

    /**
     * @brief loadCatConfigInfo:cat /boot/config* | grep '=y'信息
     * @param key:dr_config
     * @param debugfile:调试文件名
     */
    void loadCatConfigInfo(const QString &key, const QString &debugfile);
    /**
     * @brief loadBootDeviceManfid:加载本机自带硬盘
     * @param key:bootdevice
     * @param debugfile:调试文件名
     */
    void loadBootDeviceManfid(const QString &key, const QString &debugfile);

    /**
     * @brief getSMBIOSVersion:获取SMBIOS版本号
     * @param info:dmidecode获取的片断信息
     * @param version:SMBIOS版本号
     */
    void getSMBIOSVersion(const QString &info, QString &version);

    /**
         * @brief loadNvidiaSettingInfo : 加载nvidia-settings  -q  VideoRam  信息
         * @param key   nvidia
         * @param debugfile  nvidia.txt
         */
    void loadNvidiaSettingInfo(const QString &key, const QString &debugfile);

    /**
     * @brief getMapInfoFromCmd:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromInput:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromInput(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromLshw:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromHwinfo:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromDmidecode:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromSmartctl:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromSmartctl(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromHciconfig:将通过命令获取的信息字符串，转化为map形式
     * @param mapInfo:解析字符串保存为map形式
     * @param info:命令获取的信息字符串
     */
    void getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info);

    /**
     * @brief getMapInfoFromBluetoothCtl:将通过命令获取的信息字符串，转化为map形式
     * @param mapInfo:解析字符串保存为map形式
     * @param info:命令获取的信息字符串
     */
    void getMapInfoFromBluetoothCtl(QMap<QString, QString> &mapInfo, const QString &info);

    /**
     * @brief addMapInfo:添加map信息
     * @param key:与命令对应的关键字
     * @param mapInfo:解析得到的设备信息map
     */
    void addMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);

    /**
     * @brief addMouseKeyboardInfoMapInfo : 添加map信息
     * @param key : 与命令对应的关键字
     * @param mapInfo : 解析得到的设备信息map
     */
    void addMouseKeyboardInfoMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);

    /**
     * @brief addUsbMapInfo
     * @param key
     * @param mapInfo
     */
    void addUsbMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);

    /**
     * @brief containsInfoInTheMap 判断一个map里面有一个str
     * @param info
     * @param mapInfo
     * @return
     */
    bool containsInfoInTheMap(const QString &info, const QMap<QString, QString> &mapInfo);

    /**
     * @brief getMapInfo:解析打印机cups第三方库获取的信息
     * @param mapInfo:解析得到的map信息
     * @param src:cups获取的原始信息
     */
    void getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src);

    /**
     * @brief getDeviceInfo:通过文件获取设备信息字符
     * @param deviceInfo:设备信息
     * @param debugFile:调试文件名称
     * @return true:获取信息成功;false:获取信息失败
     */
    bool getDeviceInfo(QString &deviceInfo, const QString &debugFile);

    /**
     * @brief getDeviceInfoFromCmd:通过文件获取设备信息字符
     * @param deviceInfo:设备信息
     * @param cmd:调试文件名称
     * @return true:获取信息成功;false:获取信息失败
     */
    bool getDeviceInfoFromCmd(QString &deviceInfo, const QString &cmd);

    /**
     * @brief getDeviceInfo:通过文件获取设备信息字符
     * @param deviceInfo:设备信息
     * @param debugFile:调试文件名称
     * @return true:获取信息成功;false:获取信息失败
     */
    bool getCatDeviceInfo(QString &deviceInfo, const QString &debugFile);

private:
    QMap<QString, QList<QMap<QString, QString> > > m_cmdInfo;
};

#endif // CMDTOOL_H
