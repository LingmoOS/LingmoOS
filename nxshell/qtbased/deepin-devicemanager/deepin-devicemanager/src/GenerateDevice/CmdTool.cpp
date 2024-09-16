// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "CmdTool.h"
#include <QDir>
#include <cmath>
#include "DDLog.h"

// Qt库文件
#include<QLoggingCategory>
#include<QDateTime>
#include<QMutex>
#include <QCryptographicHash>

// 其它头文件
#include "../commondefine.h"
#include "EDIDParser.h"
#include "DeviceManager.h"
#include "DBusInterface.h"
#include "DBusEnableInterface.h"
#include "MacroDefinition.h"

using namespace DDLog;

CmdTool::CmdTool()
{

}

void CmdTool::addMapInfo(const QString &key, const QMap<QString, QString> &mapInfo)
{
    // 设备分类，与设备信息对照表
    if (m_cmdInfo.find(key) != m_cmdInfo.end()) {
        m_cmdInfo[key].append(mapInfo);
    } else {
        QList<QMap<QString, QString> > lstMap;
        lstMap.append(mapInfo);
        m_cmdInfo.insert(key, lstMap);
    }
}

void CmdTool::addMouseKeyboardInfoMapInfo(const QString &key, const QMap<QString, QString> &mapInfo)
{
    if (containsInfoInTheMap("Linux Foundation", mapInfo) // 在服务器版本中发现，hwinfo --mouse 和 hwinfo --keyboard获取的信息里面有多余的无用信息，需要过滤
            || containsInfoInTheMap("Elite Remote Control Driver", mapInfo) // 在笔记本中发现了一个多余信息，做特殊处理 Elite Remote Control Driver
            || containsInfoInTheMap("serial console", mapInfo) // 鲲鹏台式机子上发现一条多余信息  Model: "serial console"
            || containsInfoInTheMap("Wacom", mapInfo)) { // 数位板信息被显示成了mouse信息,这里需要做特殊处理(搞不懂数位板为什么不能显示成鼠标)
        return;
    }
    addMapInfo(key, mapInfo);
}

void CmdTool::addUsbMapInfo(const QString &key, const QMap<QString, QString> &mapInfo)
{
    QList<QMap<QString, QString>>::iterator it = m_cmdInfo["hwinfo_usb"].begin();
    // 有的是有同一个设备有两段信息，我们只需要一个
    // 比如 SysFS BusID: 1-3:1.2   和  SysFS BusID: 1-3:1.0 这个是同一个设备
    // 我们只需要一个
    for (; it != m_cmdInfo["hwinfo_usb"].end(); ++it) {
        QString curBus = (*it)["SysFS BusID"];
        QString newBus = mapInfo["SysFS BusID"];
        curBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
        newBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
        if (!curBus.isEmpty() && curBus == newBus) {
            return;
        }
    }

    // 这个是用来过滤，没有接入任何设备的usb接口
    if (mapInfo["Model"].contains("Linux Foundation")
            || mapInfo["Model"].contains("Wireless Network Adapter")
            || mapInfo["Model"].contains("Hub Controller"))
        return;

    if (mapInfo["Hardware Class"].contains("hub", Qt::CaseInsensitive))
        return;

    // 打印机几信息不从hwinfo --usb里面获取，需要过滤
    if (containsInfoInTheMap("Printer", mapInfo) || containsInfoInTheMap("LaserJet", mapInfo))
        return;

    // 提前过滤掉键盘鼠标
    if (containsInfoInTheMap("mouse", mapInfo) || containsInfoInTheMap("keyboard", mapInfo))
        return;

    // 这里特殊处理数位板信息，通过hwinfo --mouse可以获取到数位板信息，但是根据需求数位板应该在其它设备里面(虽然这很不合理)
    // 所以这里需要做特殊处理 即 item 里面包含了 Wacom 的 就说明是数位板设备，那就应该添加到其它设备里面
    if (containsInfoInTheMap("Wacom", mapInfo))
        return;
    addMapInfo(key, mapInfo);
}

bool CmdTool::containsInfoInTheMap(const QString &info, const QMap<QString, QString> &mapInfo)
{
    foreach (const QString &key, mapInfo.keys()) {
        if (mapInfo[key].contains(info, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

void CmdTool::getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src)
{
    // 获取打印机信息
    mapInfo.insert("Name", src->name);
    for (int i = 0; i < src->num_options; i++)
        mapInfo.insert(src->options[i].name, src->options[i].value);
}

void CmdTool::loadCmdInfo(const QString &key, const QString &debugFile)
{
    // 根据命令获取设备文件信息
    if ("lshw" == key)
        loadLshwInfo(debugFile);
    else if ("lsblk_d" == key)
        loadLsblkInfo(debugFile);
    else if ("ls_sg" == key)
        loadLssgInfo(debugFile);
    else if ("dmesg" == key)
        loadDmesgInfo(debugFile);
    else if ("hciconfig" == key)
        loadHciconfigInfo(debugFile);
    else if ("printer" == key)
        loadPrinterInfo();
    else if ("upower" == key)
        loadUpowerInfo(key, debugFile);
    else if (key.startsWith("hwinfo"))
        loadHwinfoInfo(key, debugFile);
    else if (key.startsWith("dmidecode"))
        loadDmidecodeInfo(key, debugFile);
    else if ("cat_devices" == key)
        loadCatInputDeviceInfo(key, debugFile);
    else if ("cat_audio" == key)
        loadCatAudioInfo(key, debugFile);
    else if ("bootdevice" == key)
        loadBootDeviceManfid(key, debugFile);    // 加载蓝牙设备配对信息
    else if ("lscpu" == key)
        loadLscpuInfo(key, debugFile);
    else if ("dr_config" == key)
        loadCatConfigInfo(key, debugFile);
    else if ("nvidia" == key)
        loadNvidiaSettingInfo(key, debugFile);
    else
        loadCatInfo(key, debugFile);
}

QMap<QString, QList<QMap<QString, QString> > > &CmdTool::cmdInfo()
{
    return m_cmdInfo;
}

QString CmdTool::loadOemTomlFileName(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() <= 0)
        return QString();

    QString tomlFileName;
    //获取文件名字oeminfo_manufacturer_productname_version.toml，
    if (mapInfo.contains("Manufacturer")
            && mapInfo.contains("Product Name")
            && mapInfo.contains("Version")) {
        QString Manufacturer = mapInfo["Manufacturer"];
        QString productname = mapInfo["Product Name"];
        QString version = mapInfo["Version"];

        if (Manufacturer.contains("N/A"))  Manufacturer = "";
        if (productname.contains("N/A"))  productname = "";
        if (version.contains("N/A"))  version = "";

        QRegExp regExp("[-/'~!@#$%^&*(){}:;,.\"\\|~`]");
        QString replace = "";         // 这里是将特殊字符换为空字符串,""代表直接去掉
        Manufacturer = Manufacturer.replace(regExp, replace);
        productname = productname.replace(regExp, replace);
        version = version.replace(regExp, replace);

        tomlFileName = "oeminfo_" + Manufacturer + "_" + productname + "_" + version;
        tomlFileName = tomlFileName.trimmed().toLower();
        tomlFileName = tomlFileName.remove(" ") + ".toml";
        return tomlFileName;
    }
    return QString();
}

bool CmdTool::parseOemTomlInfo(const QString filename)
{
    bool tomlFileRead = false;
    bool tomlPars = false;
    QString info = QString();

    if (filename.isEmpty()) {
        qCInfo(appLog) << "Toml File name  is null " ;
        return tomlFileRead;
    }
    qCInfo(appLog) << "Toml File name  is: /etc/deepin/hardware/" << filename;

    QString curPathFile =  "/etc/deepin/hardware/" + filename;
    if (QFile::exists(curPathFile)) {
        QFile file(curPathFile);
        if (file.open(QIODevice::ReadOnly)) {
            info = file.readAll().data();
            tomlFileRead = true;
            file.close();
        }
    }
    if (!tomlFileRead) {
        curPathFile = "/etc/deepin/hardware/oeminfodebug.toml"; //方便调试，并不开放给用户用
        QFile file(curPathFile);
        if (file.open(QIODevice::ReadOnly)) {
            info = file.readAll().data();
            tomlFileRead = true;
            file.close();
        }
    }

    if (!tomlFileRead)
        return tomlFileRead;
    if (info.isEmpty())
        return false;
    QStringList lines = info.split("\n");
    QRegExp regClass("\\s*(\\[)([.\\w\\s]*)(\\])\\s*#*.*");//正则表达式提取“[XXX]”,XXX为任意字符串。  并去掉[[xxxx]]
//    QRegExp regKeyValue("\\s*([a-zA-Z0-9_-]+)=\"(.*)\".*");       //裸键只能存在字母、数字、下划线和破折号（a-zA-Z0-9_-）
//    QRegExp regValue("\\s*\"(.*)\".*");
    QString classkey;
    QStringList deviceClassesList;
    QStringList ValueKeyList;
    QStringList wordlst;
    QMap<QString, QString> itemMap;
    foreach (const QString &line, lines) {
        if (line.trimmed().startsWith("#"))  // # 开头为注释  不取
            continue;
        else if (regClass.exactMatch(line)) {  //[xxx]
            if (itemMap.count() > 0) {
                addMapInfo("toml" + classkey, itemMap);
                tomlPars = true;
            }

            classkey = regClass.cap(2).split('.').first();
            itemMap.clear();
            ValueKeyList.clear();
            deviceClassesList.append(regClass.cap(2));
//        } else if (regKeyValue.exactMatch(line)) {  //键值对=
        } else if (line.contains("=")) {
            wordlst = line.split("=");
            if (2 == wordlst.size()) {

                QString key = wordlst[0].remove('"').trimmed();
                QString valuetmp = wordlst[1];
                QStringList valuelst;
                QString value;
                if (valuetmp.contains("#"))         {
                    valuelst = valuetmp.split("#");
                    value = valuelst[0].remove('\"').trimmed();
                } else
                    value = valuetmp.remove('\"').trimmed();

                itemMap.insert(key, value);
                ValueKeyList.append(key);
            }
        }
    }

    if (!classkey.isEmpty() &&  itemMap.count()) {
        addMapInfo("toml" + classkey, itemMap);
    }

    return tomlPars;
}

void CmdTool::loadLshwInfo(const QString &debugFile)
{
    // 加载lshw信息
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugFile);

    // 根据设备类型解析lshw信息
    QStringList items = deviceInfo.split("*-");
    bool isFirst = true;
    foreach (const QString &item, items) {
        QMap<QString, QString> mapInfo;
        if (isFirst) {
            // 系统信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_system", mapInfo);
            isFirst = false;
            continue;
        }

        // CPU 信息
        if (item.startsWith("cpu")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_cpu", mapInfo);
        } else if (item.startsWith("disk")) {         // 存储设备信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_disk", mapInfo);
        } else if (item.startsWith("storage")) {
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_storage", mapInfo);
#ifdef __sw_64__
        } else if ((item.startsWith("memory") && !item.startsWith("memory UNCLAIMED")) || item.startsWith("bank")) {      // 内存信息
#else
        } else if (item.startsWith("bank")) {      // 内存信息
#endif
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_memory", mapInfo);
        } else if (item.startsWith("display")) {      // 显卡信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_display", mapInfo);
        } else if (item.startsWith("multimedia")) {   // 音频信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_multimedia", mapInfo);
        } else if (item.startsWith("network")) {      // 网卡信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_network", mapInfo);
        } else if (item.startsWith("usb")) {          // USB 设备信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_usb", mapInfo);
        } else if (item.startsWith("cdrom")) {        // 光盘信息
            getMapInfoFromLshw(item, mapInfo);
            addMapInfo("lshw_cdrom", mapInfo);
        }
    }
    if (!m_cmdInfo.contains("lshw_memory")) {     // 内存信息
        foreach (const QString &item, items) {
            if (item.startsWith("memory")) {
                QMap<QString, QString> mapInfo;
                getMapInfoFromLshw(item, mapInfo);
                addMapInfo("lshw_memory", mapInfo);
            }
        }
    }
}

void CmdTool::loadLsblkInfo(const QString &debugfile)
{
    // 加载lsblk信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile))
        return;

    QStringList lines = deviceInfo.split("\n");
    QMap<QString, QString> mapInfo;

    // 获取存储设备逻辑名称以及ROTA信息
    foreach (QString line, lines) {
        QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
        if (words.size() != 2 || "NAME" == words[0])
            continue;

        mapInfo.insert(words[0].trimmed(), words[1].trimmed());

        //sudo smartctl --all /dev/%1   文件信息
        loadSmartCtlInfo(words[0].trimmed(), "smartctl_" + words[0].trimmed() + ".txt");
    }
    addMapInfo("lsblk_d", mapInfo);
}

void CmdTool::loadLssgInfo(const QString &debugfile)
{
    // 加载ls /dev/sg*信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile))
        return;

    QStringList lines = deviceInfo.split("\n");

    // 获取存储设备逻辑名称以及ROTA信息
    foreach (QString line, lines) {
        if (line.isEmpty())
            continue;

        QStringList words = line.split("/");

        //sudo smartctl --all /dev/%1   文件信息
        if (words.count() > 2)
            loadSmartCtlInfo(words[2].trimmed(), "smartctl_" + words[2].trimmed() + ".txt");
    }
}

void CmdTool::loadSmartCtlInfo(const QString &logicalName, const QString &debugfile)
{
    QString deviceInfo;

    // 获取硬盘smartctl信息
    if (!getDeviceInfo(deviceInfo, debugfile))
        return;

    QMap<QString, QString> mapInfo;

    // 硬盘逻辑名称
    mapInfo["ln"] = logicalName;

    getMapInfoFromSmartctl(mapInfo, deviceInfo);
    addMapInfo("smart", mapInfo);
}

void CmdTool::loadDmesgInfo(const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile))
        return;

    // 获取显存大小信息
    QMap<QString, QString> mapInfo;
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        // DeviceCdrom m_HwinfoToLshw 值为0000:01:00.0 此处同步修改,否则显存大小无法显示
        QRegExp reg(".*([0-9a-z]{4}:[0-9a-z]{2}:[0-9a-z]{2}.[0-9]{1}):.*VRAM([=:]{1}) ([0-9]*)[\\s]{0,1}M.*");
        if (reg.exactMatch(line)) {
            double size = reg.cap(3).toDouble();
            QString sizeS = QString("%1GB").arg(size / 1024);
            mapInfo["Size"] = reg.cap(1) + "=" + sizeS;
        }

        // Bug-85049 JJW 显存特殊处理
        QRegExp regJJW(".*VRAM Size ([0-9]*)M.*");
        if (regJJW.exactMatch(line)) {
            double size = regJJW.cap(1).toDouble();
            QString sizeS = QString("%1GB").arg(size / 1024);
            mapInfo["Size"] = "null=" + sizeS;
        }
    }
    addMapInfo("dmesg", mapInfo);

    mapInfo.clear();

    // 声卡芯片型号
    /* 正则表达式匹配的字符串实例：
     * ALC662 rev3:
     * ALC887-VD:
     * ALC887:
    */
    foreach (const QString &line, lines) {
        QRegExp reg(".*autoconfig for ([A-Za-z0-9]{6}( [A-Za-z0-9]+|-[A-Za-z0-9]+|)):.*");
        if (reg.exactMatch(line)) {
            QString chip = reg.cap(1);
            mapInfo["chip"] = chip;
        }
    }
    addMapInfo("audiochip", mapInfo);
}

void CmdTool::loadHciconfigInfo(const QString &debugfile)
{
    // 获取hciconfig文件信息
    QString deviceInfo;

    // 判断是否是系统是否是个人版
    DSysInfo::UosEdition type = DSysInfo::uosEditionType();
    if (DSysInfo::UosHome == type) {
        // 如果是个人版则直接执行命令获取设备信息
        // bug 目前服务端与直接执行命令获取结果不一致
        QProcess process;
        int msecs = 10000;
        QString cmd = "hciconfig --all";
        process.start(cmd);

        // 获取命令执行结果
        bool res = process.waitForFinished(msecs);
        if (res)
            deviceInfo = process.readAllStandardOutput();

    } else {
        // 获取文件信息
        if (!getDeviceInfo(deviceInfo, debugfile))
            return;
    }

    QStringList paragraphs = deviceInfo.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty())
            continue;
        QMap<QString, QString> mapInfo;
        getMapInfoFromHciconfig(mapInfo, paragraph);
        loadBluetoothCtlInfo(mapInfo);
    }
}

void CmdTool::loadBluetoothCtlInfo(QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("BD Address") == mapInfo.end()) {
        addMapInfo("hciconfig", mapInfo);
        return;
    }
    QProcess process;
    process.start("bluetoothctl show " + mapInfo["BD Address"]);
    process.waitForFinished(2000);
    QString deviceInfo = process.readAllStandardOutput();

    // 读取文件信息
    if (deviceInfo.isEmpty()) {
        addMapInfo("hciconfig", mapInfo);
        return;
    }

    getMapInfoFromBluetoothCtl(mapInfo, deviceInfo);

    addMapInfo("hciconfig", mapInfo);
}

void CmdTool::loadPrinterInfo()
{
    // 通过cups获取打印机信息
    //cups会识别打印机信息，之前通过文件判断概率性出现文件无信息的情况

    cups_dest_t *dests = nullptr;
    http_t *http = nullptr;
    int num_dests;
    num_dests = cupsGetDests2(http, &dests);
    if (nullptr == dests) {
        cupsFreeDests(num_dests, dests);
        return;
    }
    for (int i = 0; i < num_dests; i++) {
        cups_dest_t *dest = nullptr;
        QMap<QString, QString> mapInfo;
        dest = dests + i;
        getMapInfo(mapInfo, dest);
        // 这里为了和打印机管理保持一致，做出限制
        if (mapInfo.size() > 10)
            addMapInfo("printer", mapInfo);
    }
    cupsFreeDests(num_dests, dests);
}

void CmdTool::loadHwinfoInfo(const QString &key, const QString &debugfile)
{
    // 显示屏信息从前台直接获取
    QString deviceInfo;
    if ("hwinfo_monitor" == key) {
//        getDeviceInfoFromCmd(deviceInfo, "hwinfo --monitor");
        getDeviceInfo(deviceInfo, debugfile);
        QStringList items = deviceInfo.split("\n\n");
        foreach (const QString &item, items) {
            if (item.isEmpty())
                continue;
            QMap<QString, QString> mapInfo;
            getMapInfoFromHwinfo(item, mapInfo);
            if ("monitor" == mapInfo["Hardware Class"])
                addMapInfo(key, mapInfo);
        }
    } else { // 处理其它信息 mouse sound keyboard usb display cdrom disk
        getDeviceInfo(deviceInfo, debugfile);
        getMulHwinfoInfo(deviceInfo);
    }
}

void CmdTool::getMulHwinfoInfo(const QString &info)
{
//    // 获取已经禁用的设备的信息
    QString sAinfo, sRinfo;
    DBusEnableInterface::getInstance()->getRemoveInfo(sRinfo);
    DBusEnableInterface::getInstance()->getAuthorizedInfo(sAinfo);

    // 获取信息
    QStringList items = info.split("\n\n");
    QStringList auths = sAinfo.split("\n\n", QString::SkipEmptyParts);
    QStringList remos = sRinfo.split("\n\n", QString::SkipEmptyParts);
    QStringList resItems = items + auths + remos;

    foreach (const QString &item, resItems) {
        if (item.isEmpty())
            continue;
        QMap<QString, QString> mapInfo;
        getMapInfoFromHwinfo(item, mapInfo);
        if (mapInfo["Hardware Class"] == "sound" || mapInfo["Device"].contains("USB Audio")) {
            // mapInfo["Device"].contains("USB Audio") 是为了处理未识别的USB声卡 Bug-118773
            addMapInfo("hwinfo_sound", mapInfo);
        } else if (mapInfo["Hardware Class"].contains("network")) {
            //if (mapInfo.find("SysFS Device Link") != mapInfo.end() && mapInfo["SysFS Device Link"].contains("/devices/platform"))
            bool hasAddress = mapInfo.find("HW Address") != mapInfo.end() || mapInfo.find("Permanent HW Address") != mapInfo.end();
            bool hasPath = mapInfo.find("path") != mapInfo.end();
            if (hasPath || hasAddress) {
                addMapInfo("hwinfo_network", mapInfo);
            }
        } else if ("keyboard" == mapInfo["Hardware Class"]) {
            addMouseKeyboardInfoMapInfo("hwinfo_keyboard", mapInfo);
        } else if ("mouse" == mapInfo["Hardware Class"]) {
            addMouseKeyboardInfoMapInfo("hwinfo_mouse", mapInfo);
        } else if ("cdrom" == mapInfo["Hardware Class"]) {
            addMapInfo("hwinfo_cdrom", mapInfo);
        } else if ("disk" == mapInfo["Hardware Class"]) {
            addMapInfo("hwinfo_disk", mapInfo);
        } else if ("graphics card" == mapInfo["Hardware Class"]) {
            if (mapInfo["Device"].contains("Graphics Processing Unit"))
                continue;
            addWidthToMap(mapInfo);
            addMapInfo("hwinfo_display", mapInfo);
        } else {
            addUsbMapInfo("hwinfo_usb", mapInfo);
        }
    }
}

void CmdTool::addWidthToMap(QMap<QString, QString> &mapInfo)
{
    QString vendor = mapInfo["Vendor"];
    if (!vendor.contains("NVIDIA Corporation")) {
        return;
    }

    QString cmd = QString("nvidia-settings -q GPUMemoryInterface");
    QString sInfo;
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    sInfo = process.readAllStandardOutput();
    QStringList lines = sInfo.split("\n");
    foreach (const QString &line, lines) {
        QRegExp reg("\\s\\sAttribute\\s'GPUMemoryInterface' \\(.*\\):\\s([0-9]{2}).*");
        if (reg.exactMatch(line)) {
            mapInfo.insert("Width", reg.cap(1) + " bits");
        }
    }
}

void CmdTool::loadDmidecodeInfo(const QString &key, const QString &debugfile)
{
    // dmidecode -t 2信息单独处理
    if ("dmidecode2" == key) {
        loadDmidecode2Info(key, debugfile);
        return;
    }
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugfile);

    // According to the latest demand , The notebook should not have chassis information
    if ("dmidecode3" == key) {
        if (deviceInfo.contains("laptop", Qt::CaseInsensitive) ||
                deviceInfo.contains("notebook", Qt::CaseInsensitive)) {
            return;
        }
    }

    QStringList items = deviceInfo.split("\n\n");

    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;
        QMap<QString, QString> mapInfo;
        getMapInfoFromDmidecode(item, mapInfo);
        if (("dmidecode1" == key) && (mapInfo.size() > 0)) {
            QString filename = loadOemTomlFileName(mapInfo);
            parseOemTomlInfo(filename);
        }

        // 过滤空cpu卡槽信息
        if ("dmidecode4" == key && mapInfo.find("ID") == mapInfo.end())
            continue;

        if (mapInfo.size() > MIN_NUM)
            addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadDmidecode2Info(const QString &key, const QString &debugfile)
{
    // 通过命令获取设备信息
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugfile);

    QStringList items = deviceInfo.split("\n\n");
    QMap<QString, QString> mapInfo;
    bool isGetInfo = false;
    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;

        getMapInfoFromDmidecode(item, mapInfo);

        // 芯片信息,SMBIOS版本信息
        if (false == isGetInfo) {
            QString chipset;
            loadBiosInfoFromLspci(chipset);
            mapInfo.insert("chipset", chipset);
            QString version;
            getSMBIOSVersion(item, version);
            mapInfo.insert("SMBIOS Version", version);
            isGetInfo = true;
        }
    }

    addMapInfo(key, mapInfo);
}

void CmdTool::loadLscpuInfo(const QString &key, const QString &debugfile)
{
    // 通过命令获取设备信息
    QString deviceInfo;
    getDeviceInfo(deviceInfo, debugfile);

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;

        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo, " : ");
        addMapInfo(key, mapInfo);
    }

    QString info;
    if (!DBusInterface::getInstance()->getInfo("lscpu_num", info))
        return;
    QMap<QString, QString> mapInfo;
    getMapInfoFromCmd(info, mapInfo, " : ");
    addMapInfo("lscpu_num", mapInfo);
}

void CmdTool::loadCatInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getCatDeviceInfo(deviceInfo, debugfile))
        return;

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;

        QMap<QString, QString> mapInfo;
        if ("cat_version" == key)
            mapInfo["OS"] = item;
        else
            // 根据文件内容调整分隔符
            getMapInfoFromCmd(item, mapInfo, key.startsWith("cat_os") ? "=" : ": ");
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadUpowerInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile))
        return;

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty() || item.contains("DisplayDevice")
                || item.contains("mouse", Qt::CaseInsensitive)
                || item.contains("keyboard", Qt::CaseInsensitive)) // 98003远程后发现无线鼠标电量干扰了计算机电池电量的显示，排除无线鼠标，无线键盘
            continue;

        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo);
        if (item.contains("Daemon:"))
            // 守护进程
            addMapInfo("Daemon", mapInfo);
        else
            //电池信息
            addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadBiosInfoFromLspci(QString &chipsetFamliy)
{
    QString chipset;
    // 通过文件获取 lspci -v -s %1 信息
    if (!getDeviceInfo(chipset, "lspci_vs.txt"))
        return;

    QStringList lines = chipset.split("\n");
    foreach (const QString &line, lines) {
        if (line.contains("Subsystem", Qt::CaseInsensitive)) {
            QStringList words = line.split(": ");
            if (2 == words.size())
                chipsetFamliy = words[1].trimmed();
            break;
        }
    }
}

void CmdTool::loadCatInputDeviceInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getCatDeviceInfo(deviceInfo, debugfile))
        return;

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;

        QMap<QString, QString> mapInfo;
        getMapInfoFromInput(item, mapInfo, "=");

        // 获取与正则表达式匹配的输入设备
        QRegExp rem = QRegExp(".*(event[0-9]{1,2}).*");
        if (rem.exactMatch(mapInfo["Handlers"])) {
            QString name = rem.cap(1);
            DeviceManager::instance()->addInputInfo(name, mapInfo);
        } else {
            QRegExp re = QRegExp(".*(mouse[0-9]{1,2}).*");
            if (re.exactMatch(mapInfo["Handlers"])) {
                QString name = re.cap(1);
                DeviceManager::instance()->addInputInfo(name, mapInfo);
            }
        }

        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadCatAudioInfo(const QString &key, const QString &debugfile)
{
    // 获取设备信息
    QString deviceInfo;
    if (!getCatDeviceInfo(deviceInfo, debugfile))
        return;

    QStringList items = deviceInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;

        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo, ":");
        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadCatConfigInfo(const QString &key, const QString &debugfile)
{
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile))
        return;
    QStringList items = deviceInfo.split("\n");
    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;

        QMap<QString, QString> mapInfo;
        QStringList lines = item.split("\n");
        mapInfo.insert("drivers", lines[0].trimmed());

        addMapInfo(key, mapInfo);
    }
}

void CmdTool::loadBootDeviceManfid(const QString &key, const QString &debugfile)
{
    // 加载硬盘信息
    QString deviceInfo;
    if (!getDeviceInfo(deviceInfo, debugfile))
        return;

    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", deviceInfo.trimmed());
    addMapInfo(key, mapInfo);
}

void CmdTool::getSMBIOSVersion(const QString &info, QString &version)
{
    QStringList lineList = info.split("\n");

    foreach (auto line, lineList) {
        //  SMBIOS 3.0.0 present.
        QRegExp rx("^SMBIOS ([\\d]*.[\\d]*.[\\d])+ present.$");
        if (rx.indexIn(line) > -1) {
            version = rx.cap(1);
            break;
        }
    }
}

void CmdTool::loadNvidiaSettingInfo(const QString &key, const QString &debugfile)
{
    Q_UNUSED(debugfile);
    // 加载nvidia-settings  -q  VideoRam 信息
    // 命令与xrandr命令一样无法在后台运行,该从前台命令直接获取信息
    QString deviceInfo;
    if (getDeviceInfoFromCmd(deviceInfo, "nvidia-smi -L")) {
        QStringList gpuList = deviceInfo.split("\n");
        for (QString item : gpuList) {
            int index = item.indexOf(":");
            if (item.isEmpty() || index == -1)
                continue;
            QString firstStr = item.left(index).trimmed();
            QString lastStr = item.right(item.size() - index - 1).trimmed();
            if (firstStr.isEmpty() || lastStr.isEmpty())
                continue;
            QStringList gpuNumList = firstStr.split(' ', QString::SkipEmptyParts);
            if (gpuNumList.size() != 2)
                continue;
            QString deviceStr;
            int devIndex = lastStr.indexOf("(");
            if (devIndex != -1) {
                deviceStr = lastStr.left(devIndex).trimmed();
            }

            QString memoryInfo;
            if (!getDeviceInfoFromCmd(memoryInfo, QString("nvidia-smi -i %1 -q -d MEMORY").arg(gpuNumList[1])))
                continue;

            // 读取Bus Id
            QString sizeStr;
            QStringList memoryList = memoryInfo.split("\n");
            bool memoryUsageFlag = false;
            QString curBusIdStr;
            foreach (QString memoryItem, memoryList) {
                // 读取ID
                if (curBusIdStr.isEmpty() && memoryItem.trimmed().startsWith("GPU")) {
                    QStringList strList = memoryItem.split(' ', QString::SkipEmptyParts);
                    curBusIdStr = strList.size() == 2 ? strList[1] : "null";
                }

                // 获取显存大小
                if (!memoryUsageFlag && memoryItem.trimmed() == "FB Memory Usage") {
                    memoryUsageFlag = true;
                } else if (memoryUsageFlag) {
                    QStringList strList = memoryItem.split(':', QString::SkipEmptyParts);
                    if (strList.size() != 2) {
                        break;
                    }
                    if (strList[0].trimmed() == "Total") {
                        if (strList[1].trimmed() != "N/A") {
                            QStringList memorySizeList = strList[1].split(' ', QString::SkipEmptyParts);
                            if (memorySizeList.size() == 2) {
                                bool isOk = false;
                                int mSize = memorySizeList[0].toInt(&isOk);
                                if (isOk && memorySizeList[1].trimmed() == "MiB" && mSize >= 1000) {
                                    int curSize =  static_cast<int>((floor(mSize / 1000.0)));
                                    if ((mSize / 1000.0 - curSize) < 0.6 && (mSize / 1000.0 - curSize) > 0.4) {
                                        sizeStr = "null=" + QString::number(curSize) + ".5GB";
                                    } else {
                                        curSize = static_cast<int>((floor(mSize / 1000.0 + 0.5)));
                                        sizeStr = "null=" + QString::number(curSize) + "GB";
                                    }
                                } else if (isOk) {
                                    if (memorySizeList[1].trimmed() == "MiB" && mSize < 1000)
                                        sizeStr = "null=" + QString::number(mSize) + "MB";
                                }
                            }
                        }
                        break;
                    }
                }
            }


            QMap<QString, QString> mapInfo;
            if (getDeviceInfoFromCmd(deviceInfo, "nvidia-settings  -q  VideoRam")) {
                QRegExp reg("[\\s\\S]*VideoRam[\\s\\S]*([0-9]{4,})[\\s\\S]*");
                QStringList list = deviceInfo.split("\n");

                foreach (QString item, list) {
                    // Attribute 'VideoRam' (jixiaomei-PC:0.0): 2097152.  正则表达式获取2097152
                    if (reg.exactMatch(item)) {
                        QString gpuSize = reg.cap(1);
                        int numSize = gpuSize.toInt();
                        numSize /= 1024;
                        if (numSize >= 1024) {   // Bug109782 1024MB -> 1G
                            numSize /= 1024;
                            gpuSize = "null=" + QString::number(numSize) + "GB";   // 从nvidi-setting中获取显存信息没有Unique id ,格式与dmesg中获取信息保持一致,故添加"null="
                        } else {
                            gpuSize = "null=" + QString::number(numSize) + "MB";
                        }
                        mapInfo.insert("Size", gpuSize);
                        mapInfo.insert("Device", deviceStr);
                        break;
                    }
                }
            }

            // 从nvidia-smi读取显存大小
            if (!mapInfo.contains("Size") && !sizeStr.isEmpty()) {
                mapInfo.insert("Size", sizeStr);
                mapInfo.insert("Device", deviceStr);
            }
            if (!curBusIdStr.isEmpty() && curBusIdStr != "null") {
                mapInfo.insert("BusID", curBusIdStr);
            }

            if (mapInfo.contains("Size"))
                addMapInfo(key, mapInfo);
        }
    }

}

void CmdTool::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (2 == words.size())
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
    }
}

void CmdTool::getMapInfoFromInput(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        *it = (*it).replace(QRegExp("^[A-Z]: "), "");
        *it = (*it).trimmed();
        if ((*it).count(ch) > 2) {
            QStringList words = (*it).split(" ");
            foreach (auto attri, words) {
                QStringList attriList = attri.split(ch);

                if (2 == attriList.size())
                    mapInfo.insert(attriList[0].trimmed(), attriList[1].trimmed().replace("\"", ""));
            }
        } else {
            QStringList attriList = (*it).split(ch);
            if (2 == attriList.size())
                mapInfo.insert(attriList[0].trimmed(), attriList[1].trimmed().replace("\"", ""));
            else if (3 == attriList.size())
                mapInfo.insert(attriList[0].trimmed(), attriList[1].trimmed() + attriList[2].trimmed());
        }
    }
}

void CmdTool::getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2)
            continue;

        // && words[0].contains("configuration") == false && words[0].contains("resources") == false
        // 将configuration的内容进行拆分
        QString keyStr = words[0].trimmed();
        QString valueStr = words[1].trimmed();
        if (keyStr.contains("configuration")) {
            QStringList keyValues = valueStr.split(" ");

            for (QStringList::iterator itKV = keyValues.begin(); itKV != keyValues.end(); ++itKV) {
                QStringList attr = (*itKV).split("=");
                if (attr.size() != 2)
                    continue;

                mapInfo.insert(attr[0].trimmed(), attr[1].trimmed());
            }
        } else if (keyStr.contains("resources")) {
            QStringList keyValues = valueStr.split(" ");

            for (QStringList::iterator itKV = keyValues.begin(); itKV != keyValues.end(); ++itKV) {
                QStringList attr = (*itKV).split(":");
                if (attr.size() != 2)
                    continue;

                if (mapInfo.find(attr[0].trimmed()) != mapInfo.end())
                    mapInfo[attr[0].trimmed()] += QString("  ");

                mapInfo[attr[0].trimmed()] += attr[1].trimmed();
            }
        } else {
            // 过滤重复网卡逻辑名称数据
            if (info.startsWith("network")) {
                if (mapInfo.contains(keyStr) && keyStr == "logical name") {
                    if (!valueStr.startsWith("/dev") && mapInfo["logical name"].startsWith("/dev")){
                        mapInfo.remove(keyStr);
                        mapInfo.insert(keyStr, valueStr);
                    }
                } else {
                    mapInfo.insert(keyStr, valueStr);
                }
            } else {
                mapInfo.insert(keyStr, valueStr);
            }
        }
    }
}

QString CmdTool::getCurNetworkLinkStatus(QString driverName)
{
    //通过ifconfig 判断网络是否连接
    QProcess process;
    QString cmd = "ifconfig";
    process.start(cmd);
    QString ifconfigInfo;
    QString link;
    bool re = process.waitForFinished(-1);
    if (!re)
        return "";
    ifconfigInfo = process.readAllStandardOutput();
    //截取查询到的各个网卡连接信息
    QStringList list = ifconfigInfo.split("\n\n");
    for (int i = 0; i < list.size(); i++) {
        //判断具体的网卡
        if (!list.at(i).contains(driverName))
            continue;
        if (list.at(i).contains("broadcast")) //若网络连接，会出现子网信息
            link = "yes";
        else
            link = "no";
    }
    return link;
}

QMap<QString, QMap<QString, QString>> CmdTool::getCurPowerInfo()
{
    QString powerInfo;
    QMap<QString, QMap<QString, QString>> map;
    QProcess process;

    //执行"upower --dump"命令获取电池相关信息
    QString cmd = "upower --dump";
    process.start(cmd);

    // 获取命令执行结果
    process.waitForFinished(-1);
    powerInfo = process.readAllStandardOutput();
    QStringList items = powerInfo.split("\n\n");
    foreach (const QString &item, items) {
        if (item.isEmpty() || item.contains("DisplayDevice")
                || item.contains("line_power", Qt::CaseInsensitive)
                || item.contains("mouse", Qt::CaseInsensitive)
                || item.contains("keyboard", Qt::CaseInsensitive)) // 98003远程后发现无线鼠标电量干扰了计算机电池电量的显示，排除无线鼠标，无线键盘
            continue;

        QMap<QString, QString> mapInfo;
        getMapInfoFromCmd(item, mapInfo);
        if (!item.contains("Daemon:"))
            map.insert("upower", mapInfo);
        else
            map.insert("Daemon", mapInfo);
    }
    return map;
}

void CmdTool::getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QString tmpkey;
    QString tmpvalue;
    QString tmpvid;
    tmpvid.clear();
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if ((*it).contains("PS/2 Mouse")) {
            words.clear();
            words << "Hotplug" << "PS/2";
        }
        if ((*it).contains("SubDevice:")) {
            tmpkey = "PsubID";
        }
        if (words.size() != 2)
            continue;

        if (mapInfo.find(words[0].trimmed()) != mapInfo.end())
            mapInfo[words[0].trimmed()] += QString(" ");

        /*pick PID VID*/
        if (
            ("SubDevice" ==  words[0].trimmed() || "SubVendor" == words[0].trimmed() ||
             "Vendor" ==  words[0].trimmed() || "Device" == words[0].trimmed())
            && !(words[1].trimmed().isEmpty() ||  words[1].trimmed().contains("unknown"))
            && words[1].trimmed().contains("0x")
        ) {
            if ("SubDevice" ==  words[0].trimmed()) {
                tmpkey = "PsubID";
                tmpvalue = words[1].trimmed(); //re.cap(0);
            } else if ("SubVendor" ==  words[0].trimmed()) {
                tmpkey = "VsubID";
                tmpvalue = words[1].trimmed();
            } else if ("Vendor" ==  words[0].trimmed()) {
                tmpkey = "VID";
                tmpvalue = words[1].trimmed();
            } else if ("Device" ==  words[0].trimmed()) {
                tmpkey = "PID";
                tmpvalue = words[1].trimmed();
            }

            QStringList tmpword = tmpvalue.split(" ");
            if (tmpword.size() > 1) {
                mapInfo[tmpkey] += tmpword[1].trimmed();

                if (tmpkey == "VID") {
                    tmpvid = tmpword[1].trimmed();
                } else if (tmpkey == "PID") {
                    if (!tmpvid.isEmpty()) {
                        tmpkey = "VID_PID";
                        tmpvalue.clear();
                        tmpvalue = tmpvid + tmpword[1].remove("0x", Qt::CaseSensitive).trimmed();
                        tmpvid.clear();
                        mapInfo[tmpkey] += tmpvalue;
                    }
                }
            }
        }

        QRegExp re(".*\"(.*)\".*");
        if (re.exactMatch(words[1].trimmed())) {
            QString key = words[0].trimmed();
            QString value = re.cap(1);

            //这里是为了防止  "usb-storage", "sr"  -》 usb-storage", "sr
            // bug112311 驱动模块显示异常
            if ("Driver" ==  key || "Driver Modules" ==  key)
                value.replace("\"", "");

            // 如果信息中有unknown 则过滤
            if (!value.contains("unknown"))
                mapInfo[key] += value;

        } else {
            // 此处如果subDevice,subVendor,Device没有值，则过滤
//            if ("SubDevice" ==  words[0].trimmed() ||
//                    "SubVendor" == words[0].trimmed() ||
//                    "Device" == words[0].trimmed()) {
//                continue;
//            }
            if ("Resolution" == words[0].trimmed()) {
                mapInfo[words[0].trimmed()] += words[1].trimmed();
            } else {
                // 如果信息中有unknown 则过滤
                if (!words[1].trimmed().contains("unknown"))
                    mapInfo[words[0].trimmed()] = words[1].trimmed();
            }
        }
        if ((*it).contains("Config Status")) {
            //qCInfo(appLog) << "  Config Status"<< words[0]<<words[1];
            if(words[1].contains("avail=yes"))
                mapInfo["cfg_avail"] = "yes";
        }
    }

    if (mapInfo.contains("VID_PID") && !mapInfo["VID_PID"].isEmpty() && (mapInfo.contains("SysFS ID") || mapInfo.contains("SysFS Device Link"))) {
        QCryptographicHash Hash(QCryptographicHash::Md5);
        QByteArray buf;
        buf.append(mapInfo[tmpkey]);
        if (mapInfo.contains("SysFS Device Link") && !mapInfo["SysFS Device Link"].isEmpty()) {
            buf.append(mapInfo["SysFS Device Link"]);
        } else {
            buf.append(mapInfo["SysFS ID"]);
        }
        Hash.addData(buf);
        mapInfo["Unique ID"] = QString::fromStdString(Hash.result().toBase64().toStdString());
    }

    if (mapInfo.find("Module Alias") != mapInfo.end())
        mapInfo["Module Alias"].replace(QRegExp("[0-9a-zA-Z]{10}$"), "");

}

void CmdTool::getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList lines = info.split("\n");
    QString lasKey;
    foreach (const QString &line, lines) {
        if (line.isEmpty())
            continue;

        QStringList words = line.split(ch);
        if (1 ==  words.size() && words[0].endsWith(":")) {
            lasKey = words[0].replace(QRegExp(":$"), "");
            mapInfo.insert(lasKey.trimmed(), " ");
        } else if (1 ==  words.size() && !lasKey.isEmpty()) {
            mapInfo[lasKey.trimmed()] += words[0];
            mapInfo[lasKey.trimmed()] += "  /  ";
        } else if (2 ==  words.size()) {
            lasKey = "";
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}

void CmdTool::getMapInfoFromSmartctl(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch)
{
    QString indexName;
    int startIndex = 0;

    QRegExp reg("^[\\s\\S]*[\\d]:[\\d][\\s\\S]*$");//time 08:00

    for (int i = 0; i < info.size(); ++i) {
        if (info[i] != '\n' && i != info.size() - 1)
            continue;

        QString line = info.mid(startIndex, i - startIndex);
        startIndex = i + 1;


        int index = line.indexOf(ch);
        if (index > 0 && false == reg.exactMatch(line) && false == line.contains("Error") && false == line.contains("hh:mm:SS")) {
            if (line.indexOf("(") < index && line.indexOf(")") > index)
                continue;

            if (line.indexOf("[") < index && line.indexOf("]") > index)
                continue;

            indexName = line.mid(0, index).trimmed().remove(" is");
            if (mapInfo.contains(indexName)) {
                mapInfo[indexName] += ", ";
                mapInfo[indexName] += line.mid(index + 1).trimmed();
            } else {
                mapInfo[indexName] = line.mid(index + 1).trimmed();
            }
            continue;
        }

        if (false == indexName.isEmpty() && (line.startsWith("\t\t") || line.startsWith("    ")) && false == line.contains(":")) {
            if (mapInfo.contains(indexName)) {
                mapInfo[indexName] += ", ";
                mapInfo[indexName] += line.trimmed();
            } else {
                mapInfo[indexName] = line.trimmed();
            }

            continue;
        }

        indexName = "";

        QRegExp rx("^[ ]*[0-9]+[ ]+([\\w-_]+)[ ]+0x[0-9a-fA-F-]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+([0-9\\/w-]+[ ]*[ 0-9\\/w-()]*)$");
        if (rx.indexIn(line) > -1) {
            mapInfo[rx.cap(1)] = rx.cap(2);
            continue;
        }

        QStringList strList;

        if (line.endsWith(")")) {
            int leftBracket = line.indexOf("(");
            if (leftBracket > 0) {
                QString str = line.left(leftBracket).trimmed();
                strList = str.trimmed().split(" ");
                if (strList.size() > 2)
                    strList.last() += line.mid(leftBracket);
            }
        } else if (0 == strList.size()) {
            strList = line.trimmed().split(" ");
        }

        if (strList.size() < 5)
            continue;

        if (line.contains("Power_On_Hours")) {
            mapInfo["Power_On_Hours"] = strList.last();
            continue;
        }

        if (line.contains("Power_Cycle_Count")) {
            mapInfo["Power_Cycle_Count"] = strList.last();
            continue;
        }

        if (line.contains("Raw_Read_Error_Rate")) {
            mapInfo["Raw_Read_Error_Rate"] = strList.last();
            continue;
        }

        if (line.contains("Spin_Up_Time")) {
            mapInfo["Spin_Up_Time"] = strList.last();
            continue;
        }

        if (line.contains("Start_Stop_Count")) {
            mapInfo["Start_Stop_Count"] = strList.last();
            continue;
        }

        if (line.contains("Reallocated_Sector_Ct")) {
            mapInfo["Reallocated_Sector_Ct"] = strList.last();
            continue;
        }

        if (line.contains("Seek_Error_Rate")) {
            mapInfo["Seek_Error_Rate"] = strList.last();
            continue;
        }

        if (line.contains("Spin_Retry_Count")) {
            mapInfo["Spin_Retry_Count"] = strList.last();
            continue;
        }
        if (line.contains("Calibration_Retry_Count")) {
            mapInfo["Calibration_Retry_Count"] = strList.last();
            continue;
        }
        if (line.contains("G-Sense_Error_Rate")) {
            mapInfo["G-Sense_Error_Rate"] = strList.last();
            continue;
        }
        if (line.contains("Power-Off_Retract_Count")) {
            mapInfo["Power-Off_Retract_Count"] = strList.last();
            continue;
        }
        if (line.contains("Load_Cycle_Count")) {
            mapInfo["Load_Cycle_Count"] = strList.last();
            continue;
        }
        if (line.contains("Temperature_Celsius")) {
            mapInfo["Temperature_Celsius"] = strList.last();
            continue;
        }
        if (line.contains("Reallocated_Event_Count")) {
            mapInfo["Reallocated_Event_Count"] = strList.last();
            continue;
        }
        if (line.contains("Current_Pending_Sector")) {
            mapInfo["Current_Pending_Sector"] = strList.last();
            continue;
        }
        if (line.contains("Offline_Uncorrectable")) {
            mapInfo["Offline_Uncorrectable"] = strList.last();
            continue;
        }
        if (line.contains("UDMA_CRC_Error_Count")) {
            mapInfo["UDMA_CRC_Error_Count"] = strList.last();
            continue;
        }
        if (line.contains("Multi_Zone_Error_Rate")) {
            mapInfo["Multi_Zone_Error_Rate"] = strList.last();
            continue;
        }
    }
}

void CmdTool::getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info)
{
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        QStringList pairs = line.trimmed().split("  ");
        if (pairs.size() < 1)
            continue;

        foreach (const QString &pair, pairs) {
            QStringList keyValue = pair.trimmed().split(": ");
            if (2 == keyValue.size())
                mapInfo[keyValue[0].trimmed()] = keyValue[1].trimmed();
        }
    }
}

void CmdTool::getMapInfoFromBluetoothCtl(QMap<QString, QString> &mapInfo, const QString &info)
{
    QStringList lines = info.split("\n");
    QString uuid = "";
    foreach (const QString &line, lines) {
        QStringList keyValue = line.trimmed().split(": ");
        if (2 == keyValue.size()) {
            if ("UUID" == keyValue[0]) {
                QString valueStr = keyValue[1].trimmed();
                QStringList valueStrList = valueStr.split("(", QString::SkipEmptyParts);
                if (valueStrList.size() == 2)
                    valueStr = valueStrList[0].trimmed() + ":(" + valueStrList[1];
                uuid.append(valueStr);
                uuid.append("\n");
            } else {
                mapInfo[keyValue[0].trimmed()] = keyValue[1].trimmed();
            }
        }
    }
    if (uuid != "")
        mapInfo["UUID"] = uuid;
}

bool CmdTool::getDeviceInfo(QString &deviceInfo, const QString &debugFile)
{
    QString key = debugFile;
    key.replace(".txt", "");
    if (DBusInterface::getInstance()->getInfo(key, deviceInfo))
        return true;

    // deviceInfo 不为空时信息已读取
    if (!deviceInfo.isEmpty())
        return true;

    // 从文件中获取设备信息
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly))
        return false;

    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();

    return true;
}

bool CmdTool::getDeviceInfoFromCmd(QString &deviceInfo, const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    deviceInfo = process.readAllStandardOutput();
    return true;
}

bool CmdTool::getCatDeviceInfo(QString &deviceInfo, const QString &debugFile)
{
    // deviceInfo 不为空时信息已读取
    if (!deviceInfo.isEmpty())
        return true;

    // 从文件中获取设备信息
    QFile inputDeviceFile(debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly))
        return false;

    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();
    return true;
}
