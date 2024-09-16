// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpuinfo.h"
#include "DDLog.h"

#include <QFile>
#include <QDir>
#include <QLoggingCategory>

#include <sys/utsname.h>

using namespace DDLog;

CpuInfo::CpuInfo()
    : m_Arch("unknow")
{
}
CpuInfo::~CpuInfo()
{
    // clear physicalcpu
    m_MapPhysicalCpu.clear();
}

bool CpuInfo::loadCpuInfo()
{
    // get arch
    readCpuArchitecture();

    // read /sys/devices/system/cpu
    readSysCpu();

    // read the file /proc/cpuinfo
    if (!readProcCpuinfo())
        return false;

    // 重新编号
    QMap<int, PhysicalCpu> curMapPhysicalCpu = m_MapPhysicalCpu;
    m_MapPhysicalCpu.clear();
    foreach (int id, curMapPhysicalCpu.keys()) {
        PhysicalCpu physical = PhysicalCpu(id);

        PhysicalCpu &curPhysical = curMapPhysicalCpu[id];
        QList<int> curCoreNums = curPhysical.coreNums();
        for (int i = 0; i < curCoreNums.size(); ++i) {
            CoreCpu curCoreCpu = curPhysical.coreCpu(curCoreNums[i]);
            curCoreCpu.setCoreId(i);
            physical.addCoreCpu(i, curCoreCpu);
        }
        m_MapPhysicalCpu.insert(id, physical);
    }

    return true;
}

const QString &CpuInfo::arch() const
{
    return m_Arch;
}

void CpuInfo::logicalCpus(QString &info)
{
    foreach (int id, m_MapPhysicalCpu.keys()) {
        PhysicalCpu &physical = m_MapPhysicalCpu[id];
        physical.getInfo(info);
    }
}

int CpuInfo::physicalNum()
{
    if (m_MapPhysicalCpu.find(-1) == m_MapPhysicalCpu.end()) {
        return m_MapPhysicalCpu.size();
    } else {
        return m_MapPhysicalCpu.size() - 1;
    }
}

int CpuInfo::coreNum()
{
    int num = 0;
    foreach (int id, m_MapPhysicalCpu.keys()) {
        if (id < 0)
            continue;
        PhysicalCpu &physical = m_MapPhysicalCpu[id];
        num += physical.coreNum();
    }
    return num;
}

int CpuInfo::logicalNum()
{
    int num = 0;
    foreach (int id, m_MapPhysicalCpu.keys()) {
        if (id < 0)
            continue;
        PhysicalCpu &physical = m_MapPhysicalCpu[id];
        num += physical.logicalNum();
    }
    return num;
}

void CpuInfo::readCpuArchitecture()
{
    struct utsname utsbuf;
    if (-1 == uname(&utsbuf))
        return;
    m_Arch = QString::fromLocal8Bit(utsbuf.machine);
}

bool CpuInfo::readProcCpuinfo()
{
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString cpuInfo = file.readAll();
    QStringList infos = cpuInfo.split("\n\n");
    foreach (const QString &info, infos) {
        if (info.isEmpty())
            continue;
        parseInfo(info);
    }

    file.close();
    return true;
}

bool CpuInfo::parseInfo(const QString &info)
{
    if (info.isEmpty())
        return false;
    // 解析段落信息，获取逻辑id号
    QMap<QString, QString> mapInfo;
    int logical_id = -1;
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        if (line.isEmpty())
            continue;
        QStringList words = line.split(QRegExp("[\\s]*:[\\s]*"));
        if (words.size() != 2)
            continue;
        if ("core" == words[0]) {
            mapInfo.insert("core id", words[1]);
        } else if ("package" == words[0]) {
            mapInfo.insert("physical id", words[1]);
        } else {
            mapInfo.insert(words[0].toLower(), words[1]);
        }
        if (words[0].contains("processor"))
            logical_id = words[1].toInt();
    }

    if (logical_id < 0)
        return false;

    // 找到逻辑cpu
    if (mapInfo.find("physical id") != mapInfo.end()
            && mapInfo.find("core id") != mapInfo.end()) {
        int physical_id = mapInfo["physical id"].toInt();
        if (m_MapPhysicalCpu.find(physical_id) == m_MapPhysicalCpu.end())
            return false;
        PhysicalCpu &physical = m_MapPhysicalCpu[physical_id];
        int core_id = mapInfo["core id"].toInt();
        if (!physical.coreIsExisted(core_id)) {
            if (physical.logicalIsExisted(logical_id)) {
                LogicalCpu &logical = physical.logicalCpu(logical_id);
                if (logical.logicalID() >= 0)
                    setProcCpuinfo(logical, mapInfo);
            }
        } else {
            CoreCpu &core = physical.coreCpu(core_id);
            if (!core.logicalIsExisted(logical_id)) {
                if (physical.logicalIsExisted(logical_id)) {
                    LogicalCpu &logical = physical.logicalCpu(logical_id);
                    if (logical.logicalID() >= 0)
                        setProcCpuinfo(logical, mapInfo);
                }
            } else {
                LogicalCpu &logical = core.logicalCpu(logical_id);
                if (logical.logicalID() >= 0)
                    setProcCpuinfo(logical, mapInfo);
            }
        }
    } else {
        LogicalCpu &logical = logicalCpu(logical_id);
        if (logical.logicalID() >= 0)
            setProcCpuinfo(logical, mapInfo);
    }

    return true;
}

LogicalCpu &CpuInfo::logicalCpu(int logical_id)
{
    foreach (int physical_id, m_MapPhysicalCpu.keys()) {
        if (physical_id < 0)
            continue;
        PhysicalCpu &physical = m_MapPhysicalCpu[physical_id];
        if (physical.logicalIsExisted(logical_id)) {
            return physical.logicalCpu(logical_id);
        }
    }
    return m_MapPhysicalCpu[-1].logicalCpu(-1);
}

void CpuInfo::setProcCpuinfo(LogicalCpu &logical, const QMap<QString, QString> &mapInfo)
{
    logical.setFlags(mapInfo["flags"]);
    logical.setModel(mapInfo["model"]);
    logical.setModelName(mapInfo["model name"]);
    logical.setVendor(mapInfo["vendor_id"]);
    logical.setStepping(mapInfo["stepping"]);
    logical.setcpuFamily(mapInfo["cpu family"]);
    logical.setBogomips(mapInfo["bogomips"]);

    // diff in loognsoon and loongarch
    if ("mips64" == m_Arch || "loongarch64" == m_Arch) {
        logical.setCurFreq(mapInfo["cpu mhz"]);
        logical.setModel(mapInfo["cpu model"]);
        if (logical.flags().isEmpty()) {
            logical.setFlags(mapInfo["features"]);
        }
    }
}

void CpuInfo::readSysCpu()
{
    // /sys/devices/system/cpu/cpu*
    QDir dir("/sys/devices/system/cpu");
    dir.setFilter(QDir::Dirs);
    QRegExp reg("cpu([0-9]{1,4})");
    foreach (const QFileInfo &info, dir.entryInfoList()) {
        const QString &name = info.fileName();
        if (! reg.exactMatch(name))
            continue;
        readSysCpuN(reg.cap(1).toInt(), info.filePath());
    }
}

void CpuInfo::readSysCpuN(int N, const QString &path)
{
    // 第一步先读取物理cpu
    // /sys/devices/system/cpu/cpu0/topology/physical_package_id
    QString physicalPath = path + "/topology/physical_package_id";
    int physical_id = readPhysicalID(physicalPath);
    if (physical_id < 0) {
        return;
    }
    if (m_MapPhysicalCpu.find(physical_id) == m_MapPhysicalCpu.end()) {
        PhysicalCpu physical = PhysicalCpu(physical_id);
        m_MapPhysicalCpu.insert(physical_id, physical);
    }

    // 第二步读取core id
    // /sys/devices/system/cpu/cpu0/topology/core_id
    QString corePath = path + "/topology/core_id";
    QString thread_siblings_list_patch = path + "/topology/thread_siblings_list";
    int tsl = readThreadSiblingsListPath(thread_siblings_list_patch);
    if (tsl < 0) {
        return;
    }
    PhysicalCpu &cpu = m_MapPhysicalCpu[physical_id];
    if (!cpu.coreIsExisted(tsl)) {
        CoreCpu core = CoreCpu(tsl);
        cpu.addCoreCpu(tsl, core);
    }

    // 第三步读取逻辑cpu
    LogicalCpu lcpu;
    lcpu.setLogicalID(N);
    lcpu.setCoreID(tsl);
    lcpu.setPhysicalID(physical_id);
    lcpu.setArch(m_Arch);
    QDir dir(path);
    // get cpu cache
    if (dir.exists("cache"))
        readCpuCache(dir.filePath("cache"), lcpu);
    // get cpu freq
    if (dir.exists("cpufreq"))
        readCpuFreq(dir.filePath("cpufreq"), lcpu);
    CoreCpu &corecpu = cpu.coreCpu(tsl);
    corecpu.addLogicalCpu(N, lcpu);
}

int CpuInfo::readPhysicalID(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return -1;
    QString info = file.readAll();
    file.close();
    if ("sw_64" == m_Arch && -1 == info.toInt()) {
        return 0;
    }
    return info.toInt();
}

int CpuInfo::readCoreID(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    QString info = file.readAll();

    file.close();
    return info.toInt();
}

int CpuInfo::readThreadSiblingsListPath(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    QString info = file.readAll();
    file.close();

    QStringList words = info.split(QRegExp("\\D"));

    return words.isEmpty() ? 0 : words.first().toInt();
}

void CpuInfo::readCpuCache(const QString &path, LogicalCpu &lcpu)
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs);
    foreach (const QFileInfo &fileInfo, dir.entryInfoList()) {
        QString tpath = fileInfo.absoluteFilePath();
        if (!tpath.contains(QRegExp("index[0-9]")))
            continue;
        readCpuCacheIndex(tpath, lcpu);
    }
}

void CpuInfo::readCpuCacheIndex(const QString &path, LogicalCpu &lcpu)
{
    int level = -1;
    QString type, value;

    // get level
    QString levelPath = path + "/level";
    QFile file(levelPath);
    if (file.open(QIODevice::ReadOnly)) {
        QString info = file.readAll();
        level = info.toInt();
    }
    file.close();

    // get type
    QString typePath = path + "/type";
    QFile fileT(typePath);
    if (fileT.open(QIODevice::ReadOnly)) {
        type = fileT.readAll();
    }
    fileT.close();

    // get size
    QString valuePath = path + "/size";
    QFile fileV(valuePath);
    if (fileV.open(QIODevice::ReadOnly)) {
        value = fileV.readAll();
    }
    fileV.close();

    if (level == 2) {
        lcpu.setL2Cache(value);
    } else if (level == 3) {
        lcpu.setL3Cache(value);
    } else if (level == 4) {
        lcpu.setL4Cache(value);
    } else if (level == 1) {
        if (type.contains("Data", Qt::CaseInsensitive))
            lcpu.setL1dCache(value);
        else
            lcpu.setL1iCache(value);
    }
}

void CpuInfo::readCpuFreq(const QString &path, LogicalCpu &lcpu)
{
    // min freq
    QString minFreqPath = path + "/cpuinfo_min_freq";
    QFile minFile(minFreqPath);
    if (minFile.open(QIODevice::ReadOnly)) {
        QString info = minFile.readAll();
        int value = info.toInt() / 1000;
        lcpu.setMinFreq(QString::number(value) + "MHz");
    }
    minFile.close();

    // cur freq
    QString curFreqPath = path + "/scaling_cur_freq";
    QFile curFile(curFreqPath);
    if (curFile.open(QIODevice::ReadOnly)) {
        QString info = curFile.readAll();
        int value = info.toInt() / 1000;
        lcpu.setCurFreq(QString::number(value) + "MHz");
    }
    curFile.close();

    // maxFreq
    QString maxFreqPath = path + "/cpuinfo_max_freq";
    QFile maxFile(maxFreqPath);
    if (maxFile.open(QIODevice::ReadOnly)) {
        QString info = maxFile.readAll();
        int value = info.toInt() / 1000;
        lcpu.setMaxFreq(QString::number(value) + "MHz");
    }
    maxFile.close();
}

void CpuInfo::diagPrintInfo()
{
    foreach (int id, m_MapPhysicalCpu.keys()) {
        qCInfo(appLog) << "m_MapPhysicalCpu id: ***************** " << id;
        PhysicalCpu &pc = m_MapPhysicalCpu[id];
        pc.diagPrintInfo();
    }
}
