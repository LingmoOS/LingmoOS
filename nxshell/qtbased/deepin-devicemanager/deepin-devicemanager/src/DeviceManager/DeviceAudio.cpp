// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceAudio.h"
#include "DBusEnableInterface.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
// 其它头文件
#include <QLoggingCategory>
DeviceAudio::DeviceAudio()
    : DeviceBaseInfo()
    , m_Model("")
    , m_BusInfo("")
    , m_Irq("")
    , m_Memory("")
    , m_Width("")
    , m_Clock("")
    , m_Capabilities("")
    , m_Chip("")
    , m_DriverModules("")
{
    // 初始化可显示属性
    initFilterKey();

    // 设置可禁用
    m_CanEnable = true;
    m_CanUninstall = true;
    m_IsCatDevice = false;
}

bool DeviceAudio::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("path") != mapInfo.end()) {
        setAttribute(mapInfo, "name", m_Name);
        setAttribute(mapInfo, "driver", m_Driver);
        m_SysPath = mapInfo["path"];
        m_HardwareClass = mapInfo["Hardware Class"];
        m_Enable = false;
        //设备禁用的情况，没必要再继续向下执行(可能会引起不必要的问题)，直接return
        m_CanUninstall = !driverIsKernelIn(m_Driver);
        return false;
    }
    //1. 获取设备的基本信息

    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "IRQ", m_Irq);
    setAttribute(mapInfo, "Memory Range", m_Memory);
    setAttribute(mapInfo, "Driver Modules", m_DriverModules); // 驱动模块

    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    setAttribute(mapInfo, "Module Alias", m_UniqueID);
    setAttribute(mapInfo, "SysFS ID", m_BusInfo);

    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);

    m_PhysID = m_VID_PID;

    // 此处不能用 && 因为 m_DriverModules 可能为空
    if (driverIsKernelIn(m_DriverModules) || driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }

    //2. 获取设备的唯一标识
    setHwinfoLshwKey(mapInfo);
    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);

    // 获取设备的唯一标识
    setPhysIDMapKey(mapInfo);
    return true;
}

bool DeviceAudio::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    //1. 先判断传入的设备信息是否是该设备信息，根据总线信息来判断
    if (!matchToLshw(mapInfo))
        return false;

    //2. 确定了是该设备信息，则获取设备的基本信息
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    if (m_Vendor == "0000")
        m_Vendor = "";

    setAttribute(mapInfo, "", m_Model);

    /*
     * 再过去设备的版本号的时候，会出现版本号为00的情况，
     * 这不符合常理，所以当版本号为00时，默认版本号获取不到
    */
    setAttribute(mapInfo, "version", m_Version);
    if (m_Version == "00")
        m_Version = "";

    // 获取设备的基本信息
//    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "irq", m_Irq);
    setAttribute(mapInfo, "", m_Memory);
    setAttribute(mapInfo, "width", m_Width);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "SysFS ID", m_BusInfo);

    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);

    return true;
}

TomlFixMethod DeviceAudio::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
//  must cover the  loadOtherDeviceInfo
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "SysFS_Path", m_SysPath);
    ret = setTomlAttribute(mapInfo, "KernelModeDriver", m_Driver);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Chip", m_Chip);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    ret = setTomlAttribute(mapInfo, "Memory Address", m_Memory);
    ret = setTomlAttribute(mapInfo, "IRQ", m_Irq);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}
bool DeviceAudio::setInfoFrom_sysFS(QMap<QString, QString> &mapInfo, int ii)
{
    //4. get from cat /sys/class/sound
    QString hwCxDx;
    QString hwCard_path;

    QString Cardx = QString("card%1").arg(ii);
    QString Cardx_path =   "/sys/class/sound/"  + Cardx;

    QFileInfo info1(Cardx_path);
    if (info1.isSymLink()) {       // returns true
        m_BusInfo =  info1.symLinkTarget();
        m_SysPath =  info1.symLinkTarget();
    }
    QString tmps = "/sound/" + Cardx;
    m_SysPath = m_SysPath.remove(tmps);
    m_SysPath = m_SysPath.remove("/sys");

    QString vid = DeviceBaseInfo::get_string(Cardx_path + "/device/vendor");
    QString pid = DeviceBaseInfo::get_string(Cardx_path + "/device/device");

    for (int n = 0; n < 11; n++) { //这里最多假定为device编号最大为 0-9

        hwCxDx = QString("hwC%1D%2").arg(ii).arg(n);
        hwCard_path =   "/sys/class/sound/"  + hwCxDx;
        QDir dir(hwCard_path);
        if (dir.exists()) {
            QFile file(hwCard_path + "/vendor_id");
            if (file.open(QIODevice::ReadOnly)) {
                file.close();
                break;
            }
        }
        if (9 == n)
            return false;
    }

    if (vid.isEmpty() || pid.isEmpty()) {                    // 如果/cardx/device/device 先取作数据
        m_VID_PID = DeviceBaseInfo::get_string(hwCard_path + "/vendor_id");  //没有 则取 chip vendor_id
    } else
        m_VID_PID = vid.trimmed() + pid.remove("0x", Qt::CaseSensitive).trimmed();

    m_Chip = DeviceBaseInfo::get_string(hwCard_path + "/vendor_name") + " " + DeviceBaseInfo::get_string(hwCard_path + "/chip_name");
    m_Vendor = DeviceBaseInfo::get_string(hwCard_path + "/vendor_name");
    m_Name =  DeviceBaseInfo::get_string(Cardx_path + "/id") + " " + QString("/card%1").arg(ii);
    m_Model = m_VID_PID;
    m_Driver = "driveby " + DeviceBaseInfo::get_string(hwCard_path + "/vendor_name");  //debug

    // setAttribute(mapInfo, "SysFS ID", m_SysPath);

    m_BusInfo = m_SysPath;
    mapInfo.insert("VID_PID", m_VID_PID);
    mapInfo.insert("SysFS ID", m_SysPath);
    mapInfo.insert("chip", m_Chip);

    //2. 获取设备的其它信息
    getOtherMapInfo(mapInfo);

    // 设置不可禁用
    m_CanEnable = false;
    m_CanUninstall = false;
    return true;
}

bool DeviceAudio::setInfoFromCatDevices(const QMap<QString, QString> &mapInfo)
{
    //1. 获取设备的基本信息
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "Version", m_Version);
    setAttribute(mapInfo, "Bus", m_BusInfo);
    setAttribute(mapInfo, "", m_Irq);
    setAttribute(mapInfo, "", m_Memory);
    setAttribute(mapInfo, "", m_Width);
    setAttribute(mapInfo, "", m_Clock);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "", m_Description);

    //2. 获取设备的其它信息
    getOtherMapInfo(mapInfo);

    //3. get from cat /input/devices
    m_IsCatDevice = true;

    return true;
}

void DeviceAudio::setInfoFromCatAudio(const QMap<QString, QString> &mapInfo)
{
    //1. 获取设备的基本信息
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);

    //2. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
}

bool DeviceAudio::setAudioChipFromDmesg(const QString &info)
{
    // 设置声卡芯片型号
    m_Chip = info;
    return true;
}

const QString &DeviceAudio::chip_name()const
{
    return m_Chip;
}


const QString &DeviceAudio::name()const
{
    return m_Name;
}

const QString &DeviceAudio::vendor() const
{
    return m_Vendor;
}

const QString &DeviceAudio::driver() const
{
    if (! m_DriverModules.isEmpty())
        return m_DriverModules;
    return m_Driver;
}
const QString &DeviceAudio::uniqueID() const
{
    return m_SysPath;
}
EnableDeviceStatus DeviceAudio::setEnable(bool e)
{
    if (!m_SysPath.contains("usb")) {
        m_UniqueID = m_Name;
    }
    m_HardwareClass = "sound";
    // 设置设备状态
    if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable(m_HardwareClass, m_Name, m_SysPath, m_UniqueID, e, m_Driver);
    if (res) {
        m_Enable = e;
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

bool DeviceAudio::enable()
{
    // 获取设备状态
    return m_Enable;
}

QString DeviceAudio::subTitle()
{
    // 设备信息子标题
    return m_Name;
}

const QString DeviceAudio::getOverviewInfo()
{
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceAudio::initFilterKey()
{
    // 添加可显示的属性
    addFilterKey(tr("Device Name"));
    addFilterKey(QObject::tr("SubVendor"));
    addFilterKey(QObject::tr("SubDevice"));
    addFilterKey(QObject::tr("Driver Status"));
    addFilterKey(QObject::tr("Driver Activation Cmd"));
    addFilterKey(QObject::tr("Config Status"));
    addFilterKey(QObject::tr("latency"));
    addFilterKey(QObject::tr("Phys"));
    addFilterKey(QObject::tr("Sysfs"));
    addFilterKey(QObject::tr("Handlers"));
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
    addFilterKey(QObject::tr("Bus"));
    addFilterKey(QObject::tr("Version"));
    addFilterKey(QObject::tr("Driver"));
}

void DeviceAudio::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("SysFS_Path"), m_SysPath);
    addBaseDeviceInfo(tr("Description"), m_Description);
    addBaseDeviceInfo(tr("Revision"), m_Version);
    addBaseDeviceInfo(tr("KernelModeDriver"), m_Driver);
}

void DeviceAudio::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Module Alias"), m_Modalias);
    addOtherDeviceInfo(tr("Physical ID"), m_PhysID);
    addOtherDeviceInfo(tr("Chip"), m_Chip);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Memory Address"), m_Memory);   // 1050需求 内存改为内存地址
    addOtherDeviceInfo(tr("IRQ"), m_Irq);
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceAudio::loadTableHeader()
{
    // 表头信息
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
}

void DeviceAudio::loadTableData()
{
    // 记载表格内容
    QString tName = m_Name;

    if (!available()) {
        tName = "(" + tr("Unavailable") + ") " + m_Name;
    }

    if (!enable()) {
        tName = "(" + tr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
}

// sysfs sound 数据获取说明如下：
// /sys/class/sound$ tree         
// ├── card0 -> ../../devices/pci0000:00/0000:00:1f.3/sound/card0
// ├── hwC0D0 -> ../../devices/pci0000:00/0000:00:1f.3/sound/card0/hwC0D0
// ├── card1 -> ../../devices/pci0000:00/0000:00:01.0/0000:01:00.1/sound/card1
// ├── hwC1D0 -> ../../devices/pci0000:00/0000:00:01.0/0000:01:00.1/sound/card1/hwC1D0
//    deep@nuc8:/sys/class/sound/card0$ cat device/vendor
//    0x8086
//    deep@nuc8:/sys/class/sound/card0$ cat device/device
//    0x9dc8
//    deep@nuc8:/sys/class/sound/card0$ cat hwC0D0/vendor_id
//    0x10ec0235
