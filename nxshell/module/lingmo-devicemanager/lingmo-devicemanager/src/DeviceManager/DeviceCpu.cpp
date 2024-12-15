// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceCpu.h"

#include <math.h>

DeviceCpu::DeviceCpu()
    : DeviceBaseInfo()
    , m_PhysicalID("")
    , m_CoreID("")
    , m_ThreadNum("")
    , m_Frequency("")
    , m_CurFrequency("")
    , m_BogoMIPS("")
    , m_Architecture(""),
      m_Familly("")
    , m_Model("")
    , m_Step("")
    , m_CacheL1Data("")
    , m_CacheL1Order("")
    , m_CacheL2("")
    , m_CacheL3("")
    , m_CacheL4("")
    , m_Extensions("")
    , m_Flags("")
    , m_HardwareVirtual("")
    , m_LogicalCPUNum(0)
    , m_CPUCoreNum(0)
    , m_FrequencyIsRange(false)
    , m_FrequencyIsCur(true)
{
    initFilterKey();
}

void DeviceCpu::setCpuInfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode, int coreNum, int logicalNum)
{
    // 设置CPU信息
    setInfoFromLscpu(mapLscpu);
    setInfoFromLshw(mapLshw);
    setInfoFromDmidecode(mapDmidecode);

    // CPU 名称后面不需要加个数
    m_Name.replace(QRegExp("/[0-9]*$"), "");
    m_Name.replace(QRegExp("x [0-9]*$"), "");

    //  获取逻辑数和core数
    m_LogicalCPUNum = logicalNum;
    m_CPUCoreNum = coreNum;
}

void DeviceCpu::initFilterKey()
{
    // 添加可显示的属性
    addFilterKey(QObject::tr("CPU implementer"));
    addFilterKey(QObject::tr("CPU architecture"));
    addFilterKey(QObject::tr("CPU variant"));
    addFilterKey(QObject::tr("CPU part"));
    addFilterKey(QObject::tr("CPU revision"));
}

void DeviceCpu::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("CPU ID"), m_PhysicalID);
    addBaseDeviceInfo(tr("Core ID"), m_CoreID);
    addBaseDeviceInfo(tr("Threads"), m_ThreadNum);
    if (!m_FrequencyIsCur)
        addBaseDeviceInfo(tr("Max Speed"), m_MaxFrequency);
    addBaseDeviceInfo(tr("BogoMIPS"), m_BogoMIPS);
    addBaseDeviceInfo(tr("Architecture"), m_Architecture);
    addBaseDeviceInfo(tr("CPU Family"), m_Familly);
    addBaseDeviceInfo(tr("Model"), m_Model);
}

const QString &DeviceCpu::vendor() const
{
    return m_Vendor;
}

const QString &DeviceCpu::name() const
{
    return m_Name ;
}

const QString &DeviceCpu::driver() const
{
    return m_Driver;
}

bool DeviceCpu::available()
{
    return true;
}

bool DeviceCpu::frequencyIsRange()const
{
    return m_FrequencyIsRange;
}

QString DeviceCpu::subTitle()
{
    return QString("%1 %2").arg(tr("Processor")).arg(m_PhysicalID);
}

const QString DeviceCpu::getOverviewInfo()
{
    // 获取阿拉伯数字的英文翻译
    getTrNumber();

    QString ov = QString("%1 (%2%3 / %4%5)") \
                 .arg(m_Name) \
                 .arg(m_trNumber[m_CPUCoreNum]) \
                 .arg(tr("Core(s)")) \
                 .arg(m_trNumber[m_LogicalCPUNum]) \
                 .arg(tr("Processor"));

    return ov;
}

void DeviceCpu::setInfoFromLscpu(const QMap<QString, QString> &mapInfo)
{
    // 设置CPU属性
    setAttribute(mapInfo, "model name", m_Name);
    setAttribute(mapInfo, "vendor_id", m_Vendor, false);
    setAttribute(mapInfo, "Thread(s) per core", m_ThreadNum);
    setAttribute(mapInfo, "bogomips", m_BogoMIPS);
    setAttribute(mapInfo, "Architecture", m_Architecture);
    setAttribute(mapInfo, "cpu family", m_Familly);
    setAttribute(mapInfo, "CPU MHz", m_CurFrequency);
    setAttribute(mapInfo, "model", m_Model);
    setAttribute(mapInfo, "stepping", m_Step);
    setAttribute(mapInfo, "L1d cache", m_CacheL1Data);
    setAttribute(mapInfo, "L1i cache", m_CacheL1Order);
    setAttribute(mapInfo, "L2 cache", m_CacheL2);
    setAttribute(mapInfo, "L3 cache", m_CacheL3);
    setAttribute(mapInfo, "L4 cache", m_CacheL4);
    setAttribute(mapInfo, "flags", m_Flags);
    setAttribute(mapInfo, "Virtualization", m_HardwareVirtual);

    setAttribute(mapInfo, "processor", m_PhysicalID);
    setAttribute(mapInfo, "core id", m_CoreID);

    // 计算频率范围
    bool min = mapInfo.find("CPU min MHz") != mapInfo.end();
    bool max = mapInfo.find("CPU max MHz") != mapInfo.end();
    if (min && max) {
        QString minS = mapInfo["CPU min MHz"];
        QString maxS = mapInfo["CPU max MHz"];
        m_MaxFrequency = maxS;
        double minHz = minS.replace("MHz", "").toDouble() / 1000;
        double maxHz = maxS.replace("MHz", "").toDouble() / 1000;
        m_FrequencyIsRange = true;

        // 如果最大最小频率相等则不显示范围
        if (fabs(minHz - maxHz) < 0.001) {
            m_FrequencyIsRange = false;
            m_Frequency = maxHz > 1 ? QString("%1 GHz").arg(maxHz) : QString("%1 MHz").arg(maxHz * 1000);
        } else {
            m_Frequency = QString("%1-%2 GHz").arg(minHz).arg(maxHz);
        }
    } else if (mapInfo.find("CPU MHz") != mapInfo.end()) {
        QString maxS = mapInfo["CPU MHz"];
        m_Frequency = maxS.indexOf("MHz") > -1 ? maxS : maxS + " MHz";
    }
    //获取扩展指令集
    QStringList orders = {"MMX", "SSE", "SSE2", "SSE3", "3D Now", "SSE4", "SSSE3", "SSE4_1", "SSE4_2", "AMD64", "EM64T"};
    foreach (const QString &order, orders) {
        if (mapInfo["Flags"].contains(order, Qt::CaseInsensitive))
            m_Extensions += QString("%1 ").arg(order);
    }

    // 获取其他属性
    getOtherMapInfo(mapInfo);
}

void DeviceCpu::setCurFreq(const QString &curFreq)
{
    if (!curFreq.isEmpty())
        m_CurFrequency = curFreq;
}

void DeviceCpu::setFrequencyIsCur(const bool &flag)
{
    m_FrequencyIsCur = flag;
}

void DeviceCpu::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // longxin CPU型号不从lshw中获取
    // bug39874
    if (m_Name.contains("Loongson", Qt::CaseInsensitive))
        setAttribute(mapInfo, "version", m_Name, false);
    else {
        setAttribute(mapInfo, "product", m_Name, false);//这里覆盖了lscpu中的数据。主要版本中lshw中的version是CPU信息，而1050A中读取到的是版本信息。

        // bug-108166 lshw 中 product 包含NULL信息，version 信息正确
        // bug-112403 lshw 中 product 包含ARMv信息，version 信息正确
        if (m_Name.contains("null", Qt::CaseInsensitive) || m_Name.contains("ARMv", Qt::CaseInsensitive)) {
            setAttribute(mapInfo, "version", m_Name);
        }
    }

    // 获取设备基本信息
    setAttribute(mapInfo, "vendor", m_Vendor);

    // 获取设备其他信息
    getOtherMapInfo(mapInfo);
}

TomlFixMethod DeviceCpu::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "CPU ID", m_PhysicalID);
    ret = setTomlAttribute(mapInfo, "Core ID", m_CoreID);
    ret = setTomlAttribute(mapInfo, "Threads", m_ThreadNum);
    ret = setTomlAttribute(mapInfo, "Frequency", m_Frequency);
    ret = setTomlAttribute(mapInfo, "Current Speed", m_CurFrequency);
    ret = setTomlAttribute(mapInfo, "Max Speed", m_MaxFrequency);
    ret = setTomlAttribute(mapInfo, "BogoMIPS", m_BogoMIPS);
    ret = setTomlAttribute(mapInfo, "Architecture", m_Architecture);
    ret = setTomlAttribute(mapInfo, "CPU Family", m_Familly);
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Virtualization", m_HardwareVirtual);
    ret = setTomlAttribute(mapInfo, "Flags", m_Flags);
    ret = setTomlAttribute(mapInfo, "Extensions", m_Extensions);
    ret = setTomlAttribute(mapInfo, "L4 Cache", m_CacheL4);
    ret = setTomlAttribute(mapInfo, "L3 Cache", m_CacheL3);
    ret = setTomlAttribute(mapInfo, "L2 Cache", m_CacheL2);
    ret = setTomlAttribute(mapInfo, "L1i Cache", m_CacheL1Order);
    ret = setTomlAttribute(mapInfo, "L1d Cache", m_CacheL1Data);
    ret = setTomlAttribute(mapInfo, "Stepping", m_Step);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

void DeviceCpu::setInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    // longxin CPU型号不从dmidecode中获取
    // bug39874
    if (m_Name.contains("Loongson", Qt::CaseInsensitive)) {
        setAttribute(mapInfo, "product", m_Name, false);
    } else {
        setAttribute(mapInfo, "product", m_Name);
    }

    // 获取设备基本信息
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Max Speed", m_Frequency, false);
    setAttribute(mapInfo, "Max Speed", m_MaxFrequency, false);
    // 飞腾架构由于无法通过lscpu获取当前频率，因此需要通过dmidecode获取
    setAttribute(mapInfo, "Current Speed", m_CurFrequency, false);
    setAttribute(mapInfo, "Family", m_Familly, false);

    // 获取其他cpu信息
    getOtherMapInfo(mapInfo);
}

void DeviceCpu::loadOtherDeviceInfo()
{
    // 倒序，头插，保证原来的顺序
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Virtualization"), m_HardwareVirtual);
    addOtherDeviceInfo(tr("Flags"), m_Flags);
    addOtherDeviceInfo(tr("Extensions"), m_Extensions);
    addOtherDeviceInfo(tr("L4 Cache"), m_CacheL4);
    addOtherDeviceInfo(tr("L3 Cache"), m_CacheL3);
    addOtherDeviceInfo(tr("L2 Cache"), m_CacheL2);
    addOtherDeviceInfo(tr("L1i Cache"), m_CacheL1Order);
    addOtherDeviceInfo(tr("L1d Cache"), m_CacheL1Data);
    addOtherDeviceInfo(tr("Stepping"), m_Step);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceCpu::loadTableHeader()
{
    // 加载表头
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(frequencyIsRange() ? tr("Speed") : tr("Max Speed"));
    m_TableHeader.append(tr("Architecture"));
}

void DeviceCpu::loadTableData()
{
    // 加载表格信息
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Frequency);
    m_TableData.append(m_Architecture);
}

void DeviceCpu::getTrNumber()
{
    // 将数字转换为英文翻译
    m_trNumber.insert(1, QObject::tr("One"));
    m_trNumber.insert(2, QObject::tr("Two"));
    m_trNumber.insert(4, QObject::tr("Four"));
    m_trNumber.insert(6, QObject::tr("Six"));
    m_trNumber.insert(8, QObject::tr("Eight"));
    m_trNumber.insert(9, QObject::tr("Nine"));
    m_trNumber.insert(10, QObject::tr("Ten"));
    m_trNumber.insert(12, QObject::tr("Twelve"));
    m_trNumber.insert(14, QObject::tr("Fourteen"));
    m_trNumber.insert(16, QObject::tr("Sixteen"));
    m_trNumber.insert(18, QObject::tr("Eighteen"));

    m_trNumber.insert(20, QObject::tr("Twenty"));
    m_trNumber.insert(22, QObject::tr("Twenty-two"));
    m_trNumber.insert(24, QObject::tr("Twenty-four"));
    m_trNumber.insert(26, QObject::tr("Twenty-six"));
    m_trNumber.insert(28, QObject::tr("Twenty-eight"));

    m_trNumber.insert(30, QObject::tr("Thirty"));
    m_trNumber.insert(32, QObject::tr("Thirty-two"));
    m_trNumber.insert(34, QObject::tr("Thirty-four"));
    m_trNumber.insert(36, QObject::tr("Thirty-six"));
    m_trNumber.insert(38, QObject::tr("Thirty-eight"));

    m_trNumber.insert(40, QObject::tr("Forty"));
    m_trNumber.insert(42, QObject::tr("Forty-two"));
    m_trNumber.insert(44, QObject::tr("Forty-four"));
    m_trNumber.insert(46, QObject::tr("Forty-six"));
    m_trNumber.insert(48, QObject::tr("Forty-eight"));

    m_trNumber.insert(50, QObject::tr("Fifty"));
    m_trNumber.insert(52, QObject::tr("Fifty-two"));
    m_trNumber.insert(54, QObject::tr("Fifty-four"));
    m_trNumber.insert(56, QObject::tr("Fifty-six"));
    m_trNumber.insert(58, QObject::tr("Fifty-eight"));

    m_trNumber.insert(60, QObject::tr("Sixty"));
    m_trNumber.insert(62, QObject::tr("Sixty-two"));
    m_trNumber.insert(64, QObject::tr("Sixty-four"));
    m_trNumber.insert(66, QObject::tr("Sixty-six"));
    m_trNumber.insert(68, QObject::tr("Sixty-eight"));

    m_trNumber.insert(70, QObject::tr("Seventy"));
    m_trNumber.insert(72, QObject::tr("Seventy-two"));
    m_trNumber.insert(74, QObject::tr("Seventy-four"));
    m_trNumber.insert(76, QObject::tr("Seventy-six"));
    m_trNumber.insert(78, QObject::tr("Seventy-eight"));

    m_trNumber.insert(80, QObject::tr("Eighty"));
    m_trNumber.insert(82, QObject::tr("Eighty-two"));
    m_trNumber.insert(84, QObject::tr("Eighty-four"));
    m_trNumber.insert(86, QObject::tr("Eighty-six"));
    m_trNumber.insert(88, QObject::tr("Eighty-eight"));

    m_trNumber.insert(90, QObject::tr("Ninety"));
    m_trNumber.insert(92, QObject::tr("Ninety-two"));
    m_trNumber.insert(94, QObject::tr("Ninety-four"));
    m_trNumber.insert(96, QObject::tr("Ninety-six"));
    m_trNumber.insert(98, QObject::tr("Ninety-eight"));

    m_trNumber.insert(100, QObject::tr("One hundred"));
    m_trNumber.insert(102, QObject::tr("One hundred and Two"));
    m_trNumber.insert(104, QObject::tr("One hundred and four"));
    m_trNumber.insert(106, QObject::tr("One hundred and Six"));
    m_trNumber.insert(108, QObject::tr("One hundred and Eight"));

    m_trNumber.insert(110, QObject::tr("One hundred and Ten"));
    m_trNumber.insert(112, QObject::tr("One hundred and Twelve"));
    m_trNumber.insert(114, QObject::tr("One hundred and Fourteen"));
    m_trNumber.insert(116, QObject::tr("One hundred and Sixteen"));
    m_trNumber.insert(118, QObject::tr("One hundred and Eighteen"));

    m_trNumber.insert(120, QObject::tr("One hundred and Twenty"));
    m_trNumber.insert(122, QObject::tr("One hundred and Twenty-two"));
    m_trNumber.insert(124, QObject::tr("One hundred and Twenty-four"));
    m_trNumber.insert(126, QObject::tr("One hundred and Twenty-six"));
    m_trNumber.insert(128, QObject::tr("One hundred and Twenty-eight"));
    m_trNumber.insert(192, QObject::tr("One hundred and Ninety-two"));
    m_trNumber.insert(256, QObject::tr("Two hundred and fifty-six"));
}


