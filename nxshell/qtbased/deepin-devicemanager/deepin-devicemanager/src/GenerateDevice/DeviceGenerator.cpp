// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceGenerator.h"

// 其它头文件
#include "CmdTool.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceCpu.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceMemory.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceBios.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceImage.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceCdrom.h"
#include "DeviceManager/DevicePrint.h"
#include "DeviceManager/DeviceInput.h"
#include "MacroDefinition.h"

// Dtk头文件
#include <DSysInfo>

// Qt库文件
#include <QLoggingCategory>

DeviceGenerator::DeviceGenerator(QObject *parent)
    : QObject(parent)
{

}

DeviceGenerator::~DeviceGenerator()
{

}



void DeviceGenerator::generatorComputerDevice()
{
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("cat_os_release");
    DeviceComputer *device = new DeviceComputer() ;

    // home url
    if (cmdInfo.size() > 0) {
        QString value = cmdInfo[0]["HOME_URL"];
        device->setHomeUrl(value.replace("\"", ""));
    }

    // name type
    const QList<QMap<QString, QString> >  &sysInfo = DeviceManager::instance()->cmdInfo("lshw_system");
    if (sysInfo.size() > 0) {
        device->setType(sysInfo[0]["description"]);
        device->setVendor(sysInfo[0]["vendor"]);
        device->setName(sysInfo[0]["product"]);
    }

    // set Os Description from /etc/os-version

    QString productName = DeviceGenerator::getProductName();
    device->setOsDescription(productName);

    // os
    const QList<QMap<QString, QString> >  &verInfo = DeviceManager::instance()->cmdInfo("cat_version");
    if (verInfo.size() > 0) {
        QString info = verInfo[0]["OS"].trimmed();
        info = info.trimmed();
        QRegExp reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)", Qt::CaseSensitive);
        int index = reg.indexIn(info);
        if (index != -1) {
            QString tmp = reg.cap(0);
            info.remove(tmp);
            info.insert(index, reg.cap(1));
        }
        device->setOS(info);
    }
    DeviceManager::instance()->addComputerDevice(device);
}

void mergeSortCpuInfoByLogicalID(QList<QMap<QString, QString> > &lsCpu, QList<QMap<QString, QString> > &tmpLst, int begin, int end)
{
    // 合并列表
    int left_length = (end - begin + 1) / 2;
    int left_index = begin;
    int right_index = begin + left_length;
    int result_index = begin;

    // 合并左右区间 左区间未合并结束且右区间未合并结束时
    while (left_index < begin + left_length && right_index < end + 1) {
        // 左右区间,哪个小先排哪个,下标加1
        if (lsCpu[left_index]["processor"].toInt() <= lsCpu[right_index]["processor"].toInt())
            tmpLst[result_index++] = lsCpu[left_index++];
        else
            tmpLst[result_index++] = lsCpu[right_index++];
    }

    // 合并左区间剩余数据
    while (left_index < begin + left_length)
        tmpLst[result_index++] = lsCpu[left_index++];

    // 合并右区间剩余数据
    while (right_index < end + 1)
        tmpLst[result_index++] = lsCpu[right_index++];
}

void sortCpuInfoByLogicalID(QList<QMap<QString, QString> > &lsCpu, QList<QMap<QString, QString> > &tmpLst, int begin, int end)
{
    // 列表个数为1,直接返回
    if (0 == end - begin)
        return;

    // bug 后台获取CPU信息是按照物理CPU,核心,逻辑CPU顺序获取的
    // 界面上展示顺序混乱实际是按照物理CPU,核心,逻辑CPU顺序展示
    // 与产品沟通后,按照用户的使用感修改,CPU信息按照逻辑CPU的id从小到大显示
    // 区间个数为2
    if (1 == end - begin) {
        // 前 processor > 后 processor 时交换位置
        if (lsCpu[begin]["processor"].toInt() > lsCpu[end]["processor"].toInt()) {
            QMap<QString, QString> tmpMap = lsCpu[begin];
            lsCpu[begin] = lsCpu[end];
            lsCpu[end] = tmpMap;
        }
    } else {
        // 区间个数 > 2 递归
        sortCpuInfoByLogicalID(lsCpu, tmpLst, begin, (end - begin) / 2 + begin);
        sortCpuInfoByLogicalID(lsCpu, tmpLst, (end - begin + 1) / 2 + begin, end);
        mergeSortCpuInfoByLogicalID(lsCpu, tmpLst, begin, end);
        lsCpu = tmpLst;
    }
}

void DeviceGenerator::generatorCpuDevice()
{
    // 生成CPU
    // get info from lscpu
    const QList<QMap<QString, QString> >  &lsCpu = DeviceManager::instance()->cmdInfo("lscpu");
    QList<QMap<QString, QString> > tmpLst;
    QList<QMap<QString, QString> > srcLst;
    tmpLst.append(lsCpu);
    srcLst.append(lsCpu);

    // 按照processor id 从小到大排序
    if (lsCpu.size() > 1)
        sortCpuInfoByLogicalID(srcLst, tmpLst, 0, lsCpu.size() - 1);

    // get info from lshw
    const QList<QMap<QString, QString> >  &lshwCpu = DeviceManager::instance()->cmdInfo("lshw_cpu");
    const QMap<QString, QString> &lshw = lshwCpu.size() > 0 ? lshwCpu[0] : QMap<QString, QString>();

    // get info from dmidecode -t 4
    const QList<QMap<QString, QString> >  &dmidecode4 = DeviceManager::instance()->cmdInfo("dmidecode4");
    const QMap<QString, QString> &dmidecode = dmidecode4.size() > 0 ? dmidecode4[0] : QMap<QString, QString>();


    //  获取逻辑数和core数  获取cpu个数 获取logical个数
    int coreNum = 0, coreNum_dmi = 0, logicalNum = 0, logicalNum_dmi = 0;
    const QList<QMap<QString, QString> >  &lsCpu_num = DeviceManager::instance()->cmdInfo("lscpu_num");
    if (lsCpu_num.size() <= 0)
        return;
    const QMap<QString, QString> &map = lsCpu_num[0];
    if (map.find("core") != map.end())
        coreNum = map["core"].toInt();
    if (map.find("logical") != map.end())
        logicalNum = map["logical"].toInt();

    // set cpu number
    QSet<QString> allCPUS;
    for (auto dd4 : dmidecode4) {
        if (dd4.contains("Socket Designation"))
            allCPUS.insert(dd4["Socket Designation"]);
        if (dd4.contains("Core Count"))
            coreNum_dmi += dd4["Core Count"].toInt();
        if (dd4.contains("Thread Count"))
            logicalNum_dmi += dd4["Thread Count"].toInt();
    }
    if(logicalNum_dmi > logicalNum && logicalNum_dmi < 1024)  //due to offline policy
        logicalNum = logicalNum_dmi;
    if(coreNum_dmi > coreNum && coreNum_dmi <= 512) //due to offline policy
        coreNum = coreNum_dmi;

    DeviceManager::instance()->setCpuNum(allCPUS.isEmpty() ? dmidecode4.size() : allCPUS.size());

    // set cpu info
    QList<QMap<QString, QString> >::const_iterator it = srcLst.begin();
    for (; it != srcLst.end(); ++it) {
        DeviceCpu *device = new DeviceCpu;
        device->setCpuInfo(*it, lshw, dmidecode, coreNum, logicalNum);
        DeviceManager::instance()->addCpuDevice(device);
    }
}

void DeviceGenerator::generatorBiosDevice()
{
    // 生成BIOS
    getBiosInfo();
    getSystemInfo();
    getBaseBoardInfo();
    getChassisInfo();
    getBiosMemoryInfo();
}

void DeviceGenerator::generatorMemoryDevice()
{
    // 生成内存
    getMemoryInfoFromLshw();
    getMemoryInfoFromDmidecode();
}

void DeviceGenerator::generatorDiskDevice()
{
    // 生成存储设备
    // 添加从hwinfo中获取的信息
    getDiskInfoFromHwinfo();
    // 添加从lshw中获取的信息
    getDiskInfoFromLshw();

    getDiskInfoFromLsblk();
    getDiskInfoFromSmartCtl();
    DeviceManager::instance()->mergeDisk();
}

void DeviceGenerator::generatorGpuDevice()
{
    // 生成显示适配器
    getGpuInfoFromHwinfo();
    getGpuInfoFromLshw();
    getGpuSizeFromDmesg();
}

void DeviceGenerator::generatorMonitorDevice()
{
    // 生成显示设备
    getMonitorInfoFromHwinfo();
}

void DeviceGenerator::generatorNetworkDevice()
{
    QList<DeviceNetwork *> lstDevice;
    const QList<QMap<QString, QString>> &lstHWInfo = DeviceManager::instance()->cmdInfo("hwinfo_network");
    for (QList<QMap<QString, QString> >::const_iterator it = lstHWInfo.begin(); it != lstHWInfo.end(); ++it) {
        // Hardware Class 类型为 network interface
//        if ("network" == (*it)["Hardware Class"]) {  //去掉， 并与service端同步"hwinfo --network"改为 "hwinfo --netcard"获取网卡信息
//            continue;
//        }

        // 先判断是否是有效网卡信息
        // 符合两种情况中的一种 1. "HW Address" 和 "Permanent HW Address" 都必须有  2. 有 "unique_id"
        if (((*it).find("HW Address") == (*it).end() && (*it).find("Permanent HW Address") == (*it).end()) && ((*it).find("unique_id") == (*it).end())) {
            continue;
        }

        // 如果(*it)中包含unique_id属性，则说明是从数据库里面获取的，否则是从hwinfo中获取的
        if ((*it).find("unique_id") == (*it).end()) {
            DeviceNetwork *device = new DeviceNetwork();
            device->setInfoFromHwinfo(*it);
            lstDevice.append(device);
        } else {
            DeviceNetwork *device = nullptr;
            const QString &unique_id = (*it)["unique_id"];
            for (QList<DeviceNetwork *>::iterator itNet = lstDevice.begin(); itNet != lstDevice.end(); ++itNet) {
                if (!unique_id.isEmpty() && (*itNet)->uniqueID() == unique_id) {
                    device = (*itNet);
                }
            }
            if (device) {
                device->setEnableValue(false);
            }
        }
    }

    // 设置从lshw中获取的信息
    const QList<QMap<QString, QString>> &lstLshw = DeviceManager::instance()->cmdInfo("lshw_network");
    for (QList<QMap<QString, QString> >::const_iterator it = lstLshw.begin(); it != lstLshw.end(); ++it) {
        if ((*it).find("serial") == (*it).end())
            continue;
        const QString &serialNumber = ((*it).find("logical name") != (*it).end()) ? (*it)["serial"] + (*it)["logical name"]  :  (*it)["serial"];
        for (QList<DeviceNetwork *>::iterator itDevice = lstDevice.begin(); itDevice != lstDevice.end(); ++itDevice) {
            const QString &ser2Number =  (*itDevice)->logicalName().isEmpty() ? (*itDevice)->hwAddress() : (*itDevice)->hwAddress() + (*itDevice)->logicalName();
            if (!serialNumber.isEmpty() && ((*itDevice)->uniqueID() == serialNumber ||  ser2Number == serialNumber)) {
                (*itDevice)->setInfoFromLshw(*it);
                break;
            }
        }
    }

    foreach (DeviceNetwork *device, lstDevice) {
        DeviceManager::instance()->addNetworkDevice(device);
    }
}

void DeviceGenerator::generatorAudioDevice()
{
    // 生成音频适配器
    getAudioInfoFromHwinfo();
    // getAudioChipInfoFromDmesg();  //将两个设备芯片显示为一个，该功能代码逻辑错误  被 getAudioInfoFrom_sysFS 取代
    getAudioInfoFromLshw();
    getAudioInfoFrom_sysFS();
}

void DeviceGenerator::generatorBluetoothDevice()
{
    // 生成蓝牙
    getBlueToothInfoFromHwinfo();
    getBluetoothInfoFromLshw();
    getBluetoothInfoFromHciconfig();
}

void DeviceGenerator::generatorKeyboardDevice()
{
    // 生成键盘
    getKeyboardInfoFromHwinfo();
    getKeyboardInfoFromLshw();
}

void DeviceGenerator::generatorMouseDevice()
{
    // 生成鼠标
    getMouseInfoFromHwinfo();
    getMouseInfoFromLshw();
}

void DeviceGenerator::generatorPrinterDevice()
{
    // 生成打印机
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("printer");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DevicePrint *device = new DevicePrint() ;
        device->setInfo(*it);
        device->setHardwareClass("printer");
        DeviceManager::instance()->addPrintDevice(device);
    }
}

void DeviceGenerator::generatorCameraDevice()
{
    // 生成图像设备
    getImageInfoFromHwinfo();
    getImageInfoFromLshw();
}

void DeviceGenerator::generatorCdromDevice()
{
    // 生成光盘
    getCdromInfoFromHwinfo();
    getCdromInfoFromLshw();
}

void DeviceGenerator::generatorOthersDevice()
{
    // 生成其他设备
    getOthersInfoFromHwinfo();
    getOthersInfoFromLshw();
}

void DeviceGenerator::generatorPowerDevice()
{
    // 生成电池
    const QList<QMap<QString, QString> > &daemon = DeviceManager::instance()->cmdInfo("Daemon");
    bool hasDaemon = false;
    // 守护进程信息
    if (daemon.size() > 0)
        hasDaemon = true;

    // 电池信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("upower");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DevicePower *device = new DevicePower();
        if (!device->setInfoFromUpower(*it))
            continue;

        // 设置守护进程信息
        if (hasDaemon)
            device->setDaemonInfo(daemon[0]);

        DeviceManager::instance()->addPowerDevice(device);
    }
}

void DeviceGenerator::getBiosInfo()
{
    // 获取BIOS 信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode0");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DeviceBios *device = new DeviceBios();
        device->setBiosInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }

    const QList<QMap<QString, QString>> &lanInfo = DeviceManager::instance()->cmdInfo("dmidecode13");
    QList<QMap<QString, QString> >::const_iterator iter = lanInfo.begin();
    for (; iter != lanInfo.end(); ++iter) {
        if ((*iter).size() < 2)
            continue;

        DeviceManager::instance()->setLanguageInfo(*iter);
    }

}

void DeviceGenerator::getSystemInfo()
{
    // 获取系统信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode1");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DeviceBios *device = new DeviceBios();
        device->setSystemInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}

void DeviceGenerator::getBaseBoardInfo()
{
    // 获取主板信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode2");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DeviceBios *device = new DeviceBios();
        device->setBaseBoardInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}

void DeviceGenerator::getChassisInfo()
{
    // 获取机箱信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode3");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DeviceBios *device = new DeviceBios();
        device->setChassisInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}

void DeviceGenerator::getBiosMemoryInfo()
{
    // 获取内存插槽信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode16");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DeviceBios *device = new DeviceBios();
        device->setMemoryInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}

void DeviceGenerator::getMemoryInfoFromLshw()
{
    // 从lshw中获取内存信息
    const QList<QMap<QString, QString>> &lstMemory = DeviceManager::instance()->cmdInfo("lshw_memory");
    QList<QMap<QString, QString> >::const_iterator it = lstMemory.begin();

    for (; it != lstMemory.end(); ++it) {
        if ((*it)["description"].contains("System Memory") && (*it).find("vendor") == (*it).end()) {
            continue;
        }
        // bug47194 size属性包含MiB
        // 目前处理内存信息时，bank下一定要显示内存信息，否则无法生成内存
        if (!(*it)["size"].contains("GiB") && !(*it)["size"].contains("MiB"))
            continue;

        DeviceMemory *device = new DeviceMemory();
        device->setInfoFromLshw(*it);
        DeviceManager::instance()->addMemoryDevice(device);
    }
}

void DeviceGenerator::getMemoryInfoFromDmidecode()
{
    // 加载从dmidecode获取的内存信息
    const QList<QMap<QString, QString>> &dmiMemory = DeviceManager::instance()->cmdInfo("dmidecode17");
    QList<QMap<QString, QString> >::const_iterator dIt = dmiMemory.begin();
    for (; dIt != dmiMemory.end(); ++dIt) {
        if ((*dIt).size() < 2 || (*dIt)["size"] == "No Module Installed")
            continue;

        DeviceManager::instance()->setMemoryInfoFromDmidecode(*dIt);
    }
}

void DeviceGenerator::getDiskInfoFromHwinfo()
{
    // 从hwinfo中获取的存储设备信息
    const QList<QMap<QString, QString>> &lstDisk = DeviceManager::instance()->cmdInfo("hwinfo_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2)
            continue;

        DeviceStorage *device = new DeviceStorage();
        if (device->setHwinfoInfo(*dIt) && device->isValid()) {
            DeviceManager::instance()->addStorageDeivce(device);
        } else {
            delete device;
            device = nullptr;
        }

        // Add all disk including no size disk to filter list
        if ((*dIt).find("SysFS BusID") != (*dIt).end())
            addBusIDFromHwinfo((*dIt)["SysFS BusID"]);
    }
}

void DeviceGenerator::getDiskInfoFromLshw()
{
    // 从lshw中获取的存储设备信息
    // lshw -C disk
    const QList<QMap<QString, QString>> &lstDisk = DeviceManager::instance()->cmdInfo("lshw_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2)
            continue;

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(*dIt);
    }

    // lshw -C storage
    const QList<QMap<QString, QString>> &lstStorage = DeviceManager::instance()->cmdInfo("lshw_storage");
    QList<QMap<QString, QString> >::const_iterator sIt = lstStorage.begin();
    for (; sIt != lstStorage.end(); ++sIt) {
        if ((*sIt).size() < 2)
            continue;

        DeviceManager::instance()->addLshwinfoIntoNVMEStorageDevice(*sIt);
    }
}

void DeviceGenerator::getDiskInfoFromLsblk()
{
    // 通过lsblk信息设备存储设备介质类型
    const QList<QMap<QString, QString>> &lstblk = DeviceManager::instance()->cmdInfo("lsblk_d");
    if (lstblk.size() > 0) {
        QMap<QString, QString>::const_iterator it = lstblk[0].begin();
        for (; it != lstblk[0].end(); ++it) {
            DeviceManager::instance()->setStorageDeviceMediaType(it.key(), it.value());
        }
    }
}

void DeviceGenerator::getDiskInfoFromSmartCtl()
{
    // 加载从smartctl中获取的存储设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("smart");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        DeviceManager::instance()->setStorageInfoFromSmartctl((*it)["ln"], *it);
    }
}

void DeviceGenerator::getGpuInfoFromHwinfo()
{
    // 加载从hwinfo获取的显示适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_display");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5)
            continue;

        DeviceGpu *device = new DeviceGpu();
        device->setHwinfoInfo(*it);
        DeviceManager::instance()->addGpuDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}

void DeviceGenerator::getGpuInfoFromLshw()
{
    // 加载从lshw获取的显示适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_display");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5)
            continue;

        DeviceManager::instance()->setGpuInfoFromLshw(*it);
    }
}

void DeviceGenerator::getGpuInfoFromXrandr()
{
    // 加载从xrandr获取的显示适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("xrandr");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setGpuInfoFromXrandr(*it);
    }
}

void DeviceGenerator::getGpuSizeFromDmesg()
{
    // 加载从dmesg获取的显示适配器信息，设置显存大小
    const QList<QMap<QString, QString> > &lstMap = DeviceManager::instance()->cmdInfo("dmesg");
    if (lstMap.size() > 0 && lstMap[0].size() > 0)
        for (QMap<QString, QString> curMap : lstMap) {
            DeviceManager::instance()->setGpuSizeFromDmesg(curMap);
        } else {
        // dmesg无法获取显存信息时从nvidia-settings获取
        const QList<QMap<QString, QString>> &nvidiaMap = DeviceManager::instance()->cmdInfo("nvidia");
        if (nvidiaMap.size() > 0 && nvidiaMap[0].size() > 0) {
            for (QMap<QString, QString> curMap : nvidiaMap) {
                DeviceManager::instance()->setGpuSizeFromDmesg(curMap);
            }
        }
    }
}
void DeviceGenerator::getMonitorInfoFromHwinfo()
{
    // 加载从hwinfo获取的显示设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_monitor");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5)
            continue;

        DeviceMonitor *device = new DeviceMonitor();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addMonitor(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}

void DeviceGenerator::getMonitorInfoFromXrandrVerbose()
{
    // 加载从xrandr --verbose中获取的显示设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("xrandr_verbose");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setMonitorInfoFromXrandr((*it)["mainInfo"], (*it)["edid"]);
    }
}

void DeviceGenerator::getAudioInfoFromHwinfo()
{
    // 加载从hwinfo中获取的音频适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_sound");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5 && (*it).find("path") == (*it).end())
            continue;

        QString path = pciPath(*it);
        DeviceAudio *device = dynamic_cast<DeviceAudio *>(DeviceManager::instance()->getAudioDevice(path));
        if (device) {
            device->setEnableValue(false);
            continue;
        }

        device = new DeviceAudio();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addAudioDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
    DeviceManager::instance()->deleteDisableDuplicate_AudioDevice();
}

void DeviceGenerator::getAudioInfoFrom_sysFS()
{
    // 加载从sysFS中获取的音频适配器信息，设备声卡型号
    // QMap<QString, QString> mapInfo_0;
    // QMap<QString, QString> mapInfo_1;
    // QMap<QString, QString> mapInfo_2;
    QMap<QString, QString> mapInfo;
    QString path;

    for (int i = 0; i < 5; i++) {
        mapInfo.clear();
        DeviceAudio *device = new DeviceAudio();
        bool getcard = device->setInfoFrom_sysFS(mapInfo, i);
        if (getcard) {
            path = mapInfo["SysFS ID"];
            DeviceAudio *tmpdevice = dynamic_cast<DeviceAudio *>(DeviceManager::instance()->getAudioDevice(path));
            if (!tmpdevice) { // 没有该设备 说加新的
                DeviceManager::instance()->addAudioDevice(device);
            } else { //找到有就 加芯片等信息
                QString chip = mapInfo["chip"];
                // DeviceManager::instance()->setAudioChipFromDmesg(chip);
                tmpdevice->setAudioChipFromDmesg(chip);
                delete device;
                device = nullptr;
            }
        }
    }
}

void DeviceGenerator::getAudioInfoFromLshw()
{
    // 加载从lshw中获取的音频适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_multimedia");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5)
            continue;

        DeviceManager::instance()->setAudioInfoFromLshw(*it);
    }
    DeviceManager::instance()->deleteDisableDuplicate_AudioDevice();  //bug 150331  禁用显卡音频，重启后音频信息异常增多
}

void DeviceGenerator::getAudioInfoFromCatInput()
{
    // 加载从 cat /proc/bus/input/devices获取的音频适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("cat_devices");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {

        // 判断输入设备属于音频适配器
        if (true == (*it)["Sysfs"].contains("sound", Qt::CaseInsensitive)) {
            DeviceAudio *device = new DeviceAudio();
            device->setInfoFromCatDevices(*it);
            DeviceManager::instance()->addAudioDevice(device);
        }
    }
}

void DeviceGenerator::getAudioChipInfoFromDmesg()
{
    // 加载从dmesg中获取的音频适配器信息，设备声卡型号
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("audiochip");
    if (lstMap.size() > 0)
        DeviceManager::instance()->setAudioChipFromDmesg(lstMap[0]["chip"]);
}

void DeviceGenerator::getBluetoothInfoFromHciconfig()
{
    //  加载从hciconfig中获取的蓝牙信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hciconfig");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    int index = 0;
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(DeviceManager::instance()->getBluetoothAtIndex(index)) ;
        if (device)
            device->setInfoFromHciconfig(*it);
        index++;
    }
}

void DeviceGenerator::getBlueToothInfoFromHwinfo()
{
    //  加载从hwinfo中获取的蓝牙信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        if ((*it)["Hardware Class"] == "hub" || (*it)["Hardware Class"] == "mouse" || (*it)["Hardware Class"] == "keyboard")
            continue;

        if ((*it)["Model"].contains("bluetooth", Qt::CaseInsensitive) || (*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb" || (*it)["Device"] == "BCM20702A0") {

            // 判断重复设备数据
            QString unique_id = uniqueID(*it);
            DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(DeviceManager::instance()->getBluetoothDevice(unique_id));
            if (device) {
                device->setEnableValue(false);
                device->setInfoFromHwinfo(*it);
                continue;
            }

            device = new DeviceBluetooth();
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addBluetoothDevice(device);
            addBusIDFromHwinfo((*it)["SysFS BusID"]);
        } else if (!it->contains("Driver") && it->contains("Driver Status") && (*it)["Driver Status"].contains("is")) {
            QStringList driverStatus = (*it)["Driver Status"].split("is");
            if (!driverStatus.isEmpty() && driverStatus.first().contains("btusb")) {
                // 判断重复设备数据
                QString unique_id = uniqueID(*it);
                DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(DeviceManager::instance()->getBluetoothDevice(unique_id));
                if (device) {
                    device->setEnableValue(false);
                    device->setInfoFromHwinfo(*it);
                    continue;
                }

                device = new DeviceBluetooth();
                device->setInfoFromHwinfo(*it);
                DeviceManager::instance()->addBluetoothDevice(device);
                addBusIDFromHwinfo((*it)["SysFS BusID"]);
            }
        }
    }
}

void DeviceGenerator::getBluetoothInfoFromLshw()
{
    //  加载从lshw中获取的蓝牙信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setBluetoothInfoFromLshw(*it);
    }
}

void DeviceGenerator::getKeyboardInfoFromHwinfo()
{
    //  加载从hwinfo中获取的键盘信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_keyboard");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceInput *device = new DeviceInput();
        device->setInfoFromHwinfo(*it);
        device->setHardwareClass("keyboard");
        device->setCanEnale(false);
        DeviceManager::instance()->addKeyboardDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}


void DeviceGenerator::getKeyboardInfoFromLshw()
{
    //  加载从lshw中获取的键盘信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->setKeyboardInfoFromLshw(*it);
    }
}

void DeviceGenerator::getMouseInfoFromHwinfo()
{
    //  加载从hwinfo中获取的鼠标信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_mouse");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        // 不让数位板显示在鼠标设备里面(武汉那边的数位板)
        if ((*it)["Device"].contains("PM"))
            continue;

        // 不让数位板显示在鼠标设备里面(nanjing的数位板)
        if ((*it)["Device"].contains("T70"))
            continue;

        //过滤触摸板，如果不是触摸板再检查
        if ((*it)["Hotplug"] != "PS/2") {
            // 先判断是否存在
            QString path = pciPath(*it);
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                continue;
            }
        }

        QString unique_id = uniqueID(*it);
        DeviceInput *device = dynamic_cast<DeviceInput *>(DeviceManager::instance()->getMouseDevice(unique_id));
        if (device) {
            device->setEnableValue(false);
            continue;
        } else {
            if ((*it).find("path") != (*it).end()) {
                continue;
            }
        }

        device = new DeviceInput();
        device->setInfoFromHwinfo(*it);
        device->setHardwareClass("mouse");
        if (device->bluetoothIsConnected()) {
            DeviceManager::instance()->addMouseDevice(device);
            addBusIDFromHwinfo((*it)["SysFS BusID"]);
        } else {
            delete device;
            device = nullptr;
        }
    }

    //  加载从hwinfo --usb中获取的触摸屏信息具有鼠标功能，放到鼠标设备中
    const QList<QMap<QString, QString>> &lstMapUSB = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator iter = lstMapUSB.begin();
    for (; iter != lstMapUSB.end(); ++iter) {
        if ((*iter).size() < 1)
            continue;

        // 指定型号触摸屏，显示在鼠标设备中
        if ((*iter)["Model"].contains("Melfas LGDisplay Incell Touch")) {
            DeviceInput *device = new DeviceInput();
            device->setInfoFromHwinfo(*iter);
            DeviceManager::instance()->addMouseDevice(device);
            addBusIDFromHwinfo((*iter)["SysFS BusID"]);
        }
    }
}

void DeviceGenerator::getMouseInfoFromLshw()
{
    //  加载从lshw中获取的鼠标信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;

        DeviceManager::instance()->addMouseInfoFromLshw(*it);
    }
}

void DeviceGenerator::getMouseInfoFromCatDevices()
{

}

void DeviceGenerator::getImageInfoFromHwinfo()
{
    //  加载从hwinfo中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 3)
            continue;

        // hwinfo中对camera的分类不明确，通过camera等关键字认定图像设备
        if (!(*it)["Model"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Device"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Driver"].contains("uvcvideo", Qt::CaseInsensitive) &&
                !(*it)["Model"].contains("webcam", Qt::CaseInsensitive) &&
                (*it)["Hardware Class"] != "camera") {
            continue;
        }

        // 判断该摄像头是否存在，被禁用的和被拔出
        QString path = pciPath(*it);
        if (path.contains("usb")) {
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                continue;
            }
        }

        // 判断重复设备数据
        QString unique_id = uniqueID(*it);
        DeviceImage *device = dynamic_cast<DeviceImage *>(DeviceManager::instance()->getImageDevice(unique_id));
        if (device) {
            device->setEnableValue(false);
            device->setInfoFromHwinfo(*it);
            continue;
        } else {
            if ((*it).find("path") != (*it).end()) {
                continue;
            }
        }

        device = new DeviceImage();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addImageDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}

void DeviceGenerator::getImageInfoFromLshw()
{
    //  加载从lshw中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DeviceManager::instance()->setCameraInfoFromLshw(*it);
    }
}

void DeviceGenerator::getCdromInfoFromHwinfo()
{
    //  加载从hwinfo中获取的cdrom设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_cdrom");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5)
            continue;

        DeviceCdrom *device = new DeviceCdrom();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addCdromDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}

void DeviceGenerator::getCdromInfoFromLshw()
{
    //  加载从lshw中获取的cdrom设备信息
    const QList<QMap<QString, QString>> &lstDisk = DeviceManager::instance()->cmdInfo("lshw_cdrom");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2)
            continue;

        DeviceManager::instance()->addLshwinfoIntoCdromDevice(*dIt);
    }
}

void DeviceGenerator::getOthersInfoFromHwinfo()
{
    //  加载从hwinfo中获取的其他设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 3)
            continue;
        /*  bug 141439 不可见功能设计需要再次思考*/
        if ((*it).find("Device") != (*it).end() &&
                ((*it)["Device"].contains("fingerprint", Qt::CaseInsensitive)  ||
                 (*it)["Device"].contains("MOH", Qt::CaseInsensitive)
                )) {
            DeviceOthers *device = new DeviceOthers();
            device->setForcedDisplay(true);
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addOthersDevice(device);
            continue;
        }

        bool isOtherDevice = true;
        QString curBus = (*it)["SysFS BusID"];
        curBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
        const QStringList &lstBusId = DeviceManager::instance()->getBusId();
        // 判断该设备是否已经在其他类别中显示
        if ((*it).find("unique_id") != (*it).end() && (*it)["Hardware Class"] != "others") {
            isOtherDevice = false;
        } else if ((*it).find("unique_id") == (*it).end()) {
            if (curBus.isEmpty() || lstBusId.indexOf(curBus) != -1)
                isOtherDevice = false;
        }

        // 添加其他设备
        if (isOtherDevice) {
            // 先判断是否存在
            QString path = pciPath(*it);
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                continue;
            }

            QString unique_id = uniqueID(*it);
            DeviceOthers *device = dynamic_cast<DeviceOthers *>(DeviceManager::instance()->getOthersDevice(unique_id));
            if (device) {
                device->setEnableValue(false);
                continue;
            } else {
                if ((*it).find("path") != (*it).end()) {
                    continue;
                }
            }


            device = new DeviceOthers();
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addOthersDevice(device);
        }
    }
}

void DeviceGenerator::getOthersInfoFromLshw()
{
    // 加载从lshw中获取的其他设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        DeviceManager::instance()->setOthersDeviceInfoFromLshw(*it);
    }
}

void DeviceGenerator::addBusIDFromHwinfo(const QString &sysfsBusID)
{
    // 添加hwinfo中唯一标识设备的字符串
    if (sysfsBusID.isEmpty())
        return;

    QString busID = sysfsBusID;
    busID.replace(QRegExp("\\.[0-9]+$"), "");

    m_ListBusID.append(busID);
}

const QStringList &DeviceGenerator::getBusIDFromHwinfo()
{
    return m_ListBusID;
}

const QString DeviceGenerator::getProductName()
{
    // 由DTK接口获取系统名称
    QString name = DSysInfo::uosSystemName(QLocale(QLocale::English));
    QString productType = DSysInfo::uosProductTypeName(QLocale(QLocale::English));

    if (!productType.contains("Server", Qt::CaseInsensitive)) {
        // 非服务器版 “产品名称”+“大版本号”+“版本名称”
        name += " " + DSysInfo::majorVersion();
        name += " " + DSysInfo::uosEditionName(QLocale(QLocale::English));
    } else {
        // 服务器版 产品名称”+“大版本号”+“（完整版本号识别码版本识别码）”
        name += " " + DSysInfo::majorVersion();
        name += " (";
        name += DSysInfo::minorVersion();
        name += DSysInfo::uosEditionName(QLocale(QLocale::English));
        name += ")";
    }

    return name;
}

QString DeviceGenerator::pciPath(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("path") != mapInfo.end()) { // SysFS Device Link
        return mapInfo["path"];
    } else if (mapInfo.find("SysFS Device Link") != mapInfo.end()) {
        return mapInfo["SysFS Device Link"];
    } else if (mapInfo.find("SysFS ID") != mapInfo.end()) {
        return mapInfo["SysFS ID"];
    } else {
        if (mapInfo["Device Files"].contains("platform")) {
            return "platform";
        }
        return "";
    }
}

QString DeviceGenerator::uniqueID(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("unique_id") != mapInfo.end()) {
        return mapInfo["unique_id"];
    } else if (mapInfo.find("Unique ID") != mapInfo.end()) {
        return mapInfo["Unique ID"];
    } else if (mapInfo.find("Permanent HW Address") != mapInfo.end()) {
        return mapInfo["Permanent HW Address"];
    } else if (mapInfo.find("Serial ID") != mapInfo.end()) {
        return mapInfo["Serial ID"];
    }
    return "";
}

void DeviceGenerator::generatorInfoFromToml(DeviceType deviceType)
{
    DeviceManager::instance()->tomlDeviceSet(deviceType);
}

