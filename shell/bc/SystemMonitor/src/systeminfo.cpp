#include "systeminfo.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QVariantMap>
#include <sys/statvfs.h>
#include <pwd.h>
#include <sys/resource.h>
#include <signal.h>
#include <algorithm>
#include <QThread>

SystemInfo::SystemInfo(QObject *parent)
    : QObject(parent)
    , m_cpuUsage(0.0)
    , m_memoryUsage(0.0)
    , m_prevIdleTime(0)
    , m_prevTotalTime(0)
{
    initSystemInfo();

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &SystemInfo::updateCpuUsage);
    connect(m_updateTimer, &QTimer::timeout, this, &SystemInfo::updateMemoryInfo);
    connect(m_updateTimer, &QTimer::timeout, this, &SystemInfo::updateProcessList);
    connect(m_updateTimer, &QTimer::timeout, this, &SystemInfo::updateDiskInfo);
    connect(m_updateTimer, &QTimer::timeout, this, &SystemInfo::updateNetworkInfo);
    m_updateTimer->start(1000);  // Update every second
}

void SystemInfo::initSystemInfo()
{
    // Get CPU Model
    QFile cpuinfo("/proc/cpuinfo");
    if (cpuinfo.open(QIODevice::ReadOnly)) {
        QTextStream stream(&cpuinfo);
        QString line;
        while (stream.readLineInto(&line)) {
            if (line.startsWith("model name")) {
                m_cpuModel = line.section(':', 1).trimmed();
                break;
            }
        }
        cpuinfo.close();
    }

    // Get OS Info
    QFile osRelease("/etc/os-release");
    if (osRelease.open(QIODevice::ReadOnly)) {
        QTextStream stream(&osRelease);
        QString line;
        while (stream.readLineInto(&line)) {
            if (line.startsWith("PRETTY_NAME")) {
                m_osInfo = line.section('=', 1).trimmed();
                m_osInfo.remove('"');
                break;
            }
        }
        osRelease.close();
    }

    // Get kernel version
    QFile procVersion("/proc/version");
    if (procVersion.open(QIODevice::ReadOnly)) {
        QString version = QString::fromUtf8(procVersion.readLine());
        m_kernelVersion = version.section(' ', 2, 2);
        procVersion.close();
    }

    // Get hostname
    QFile hostnameFile("/etc/hostname");
    if (hostnameFile.open(QIODevice::ReadOnly)) {
        m_hostname = QString::fromUtf8(hostnameFile.readAll()).trimmed();
        hostnameFile.close();
    }

    // Get desktop environment
    m_desktopEnvironment = qgetenv("XDG_CURRENT_DESKTOP");

    updateMemoryInfo();

    // 获取 CPU 核心数
    m_cpuCoreCount = QThread::idealThreadCount();
    m_cpuCoreUsage.resize(m_cpuCoreCount);
    m_prevCoreIdleTime.resize(m_cpuCoreCount);
    m_prevCoreTotalTime.resize(m_cpuCoreCount);
}

void SystemInfo::updateCpuUsage()
{
    QFile statFile("/proc/stat");
    if (!statFile.open(QIODevice::ReadOnly)) {
        return;
    }

    // 读取总体 CPU 使用率
    QString line = statFile.readLine();
    QStringList values = line.split(QRegExp("\\s+"));
    if (values.size() >= 5) {
        unsigned long long user = values[1].toLongLong();
        unsigned long long nice = values[2].toLongLong();
        unsigned long long system = values[3].toLongLong();
        unsigned long long idle = values[4].toLongLong();

        unsigned long long totalTime = user + nice + system + idle;
        unsigned long long idleTime = idle;

        if (m_prevTotalTime != 0) {
            double totalDiff = totalTime - m_prevTotalTime;
            double idleDiff = idleTime - m_prevIdleTime;

            m_cpuUsage = (1.0 - idleDiff / totalDiff) * 100.0;
            emit cpuUsageChanged();
        }

        m_prevTotalTime = totalTime;
        m_prevIdleTime = idleTime;
    }

    // 读取每个核心的使用率
    for (int i = 0; i < m_cpuCoreCount; ++i) {
        line = statFile.readLine();
        values = line.split(QRegExp("\\s+"));
        
        if (values.size() >= 5) {
            unsigned long long user = values[1].toLongLong();
            unsigned long long nice = values[2].toLongLong();
            unsigned long long system = values[3].toLongLong();
            unsigned long long idle = values[4].toLongLong();

            unsigned long long totalTime = user + nice + system + idle;
            unsigned long long idleTime = idle;

            if (m_prevCoreTotalTime[i] != 0) {
                double totalDiff = totalTime - m_prevCoreTotalTime[i];
                double idleDiff = idleTime - m_prevCoreIdleTime[i];

                m_cpuCoreUsage[i] = (1.0 - idleDiff / totalDiff) * 100.0;
            }

            m_prevCoreTotalTime[i] = totalTime;
            m_prevCoreIdleTime[i] = idleTime;
        }
    }

    statFile.close();
    emit cpuCoreUsageChanged();
}

void SystemInfo::updateMemoryInfo()
{
    QFile meminfo("/proc/meminfo");
    if (!meminfo.open(QIODevice::ReadOnly)) {
        return;
    }

    unsigned long long total = 0;
    unsigned long long free = 0;
    unsigned long long buffers = 0;
    unsigned long long cached = 0;

    QTextStream stream(&meminfo);
    QString line;
    while (stream.readLineInto(&line)) {
        if (line.startsWith("MemTotal:"))
            total = line.split(QRegExp("\\s+"))[1].toLongLong();
        else if (line.startsWith("MemFree:"))
            free = line.split(QRegExp("\\s+"))[1].toLongLong();
        else if (line.startsWith("Buffers:"))
            buffers = line.split(QRegExp("\\s+"))[1].toLongLong();
        else if (line.startsWith("Cached:"))
            cached = line.split(QRegExp("\\s+"))[1].toLongLong();
    }
    meminfo.close();

    unsigned long long used = total - free - buffers - cached;
    m_memoryUsage = (double)used / total * 100.0;
    m_totalMemory = formatSize(total * 1024);
    m_usedMemory = formatSize(used * 1024);

    emit memoryUsageChanged();
    emit totalMemoryChanged();
    emit usedMemoryChanged();
}

QString SystemInfo::formatSize(unsigned long long size)
{
    double sizeGB = size / (1024.0 * 1024.0 * 1024.0);
    return QString::number(sizeGB, 'f', 1) + " GB";
}

QString SystemInfo::cpuModel() const
{
    return m_cpuModel;
}

QString SystemInfo::osInfo() const
{
    return m_osInfo;
}

double SystemInfo::cpuUsage() const
{
    return m_cpuUsage;
}

double SystemInfo::memoryUsage() const
{
    return m_memoryUsage;
}

QString SystemInfo::totalMemory() const
{
    return m_totalMemory;
}

QString SystemInfo::usedMemory() const
{
    return m_usedMemory;
}

QVariantList SystemInfo::processList() const
{
    return m_processList;
}

QVariantList SystemInfo::diskList() const
{
    return m_diskList;
}

QVariantList SystemInfo::networkList() const
{
    return m_networkList;
}

void SystemInfo::updateProcessList()
{
    QVariantList processes;
    QDir procDir("/proc");
    QStringList pidDirs = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // 获取CPU核心数
    static int numCPUCores = QThread::idealThreadCount();
    
    // 获取系统总CPU时间
    unsigned long long totalCPUTime = 0;
    static unsigned long long prevTotalCPUTime = 0;
    QFile statFile("/proc/stat");
    if (statFile.open(QIODevice::ReadOnly)) {
        QString line = statFile.readLine();
        QStringList values = line.split(QRegExp("\\s+"));
        if (values.size() >= 5) {
            for (int i = 1; i < values.size(); ++i) {
                totalCPUTime += values[i].toLongLong();
            }
        }
        statFile.close();
    }

    // 计算系统CPU时间差
    unsigned long long totalCPUDiff = totalCPUTime - prevTotalCPUTime;
    prevTotalCPUTime = totalCPUTime;

    for (const QString &pid : pidDirs) {
        bool ok;
        pid.toLongLong(&ok);
        if (!ok) continue;

        QString statPath = QString("/proc/%1/stat").arg(pid);
        QString statusPath = QString("/proc/%1/status").arg(pid);
        
        QFile procStatFile(statPath);
        QFile statusFile(statusPath);

        if (procStatFile.open(QIODevice::ReadOnly) && statusFile.open(QIODevice::ReadOnly)) {
            QString statContent = procStatFile.readAll();
            QStringList statParts = statContent.split(" ");

            if (statParts.size() > 22) {
                QVariantMap process;
                process["pid"] = pid;

                // 读取进程名和内存信息
                QTextStream statusStream(&statusFile);
                QString line;
                while (statusStream.readLineInto(&line)) {
                    if (line.startsWith("Name:"))
                        process["name"] = line.section('\t', 1).trimmed();
                    else if (line.startsWith("VmRSS:"))
                        process["memory"] = formatSize(line.section('\t', 1).trimmed().split(' ')[0].toLongLong() * 1024);
                }

                // 计算 CPU 使用率
                unsigned long long utime = statParts[13].toLongLong();
                unsigned long long stime = statParts[14].toLongLong();
                unsigned long long cutime = statParts[15].toLongLong();
                unsigned long long cstime = statParts[16].toLongLong();
                
                // 进程总CPU时间（包括子进程）
                unsigned long long procTime = utime + stime + cutime + cstime;
                
                // 获取上一次的值
                auto prevProcTime = m_prevProcessTimes.value(pid, procTime);
                
                // 计算CPU使用率
                double cpuUsage = 0.0;
                if (totalCPUDiff > 0) {
                    // 计算进程时间差
                    unsigned long long procTimeDiff = procTime - prevProcTime;
                    
                    // 计算使用率：(进程时间差 / 系统时间差) * 100%
                    cpuUsage = (100.0 * procTimeDiff) / totalCPUDiff;
                }
                
                // 保存当前值用于下次计算
                m_prevProcessTimes[pid] = procTime;
                
                // 限制范围并保留一位小数
                cpuUsage = qBound(0.0, cpuUsage, 100.0 * numCPUCores);
                process["cpu"] = QString::number(cpuUsage, 'f', 1);

                // 获取用户信息
                QFile loginuidFile(QString("/proc/%1/loginuid").arg(pid));
                if (loginuidFile.open(QIODevice::ReadOnly)) {
                    QString uid = loginuidFile.readAll().trimmed();
                    struct passwd *pw = getpwuid(uid.toUInt());
                    if (pw) process["user"] = QString::fromLocal8Bit(pw->pw_name);
                }

                processes.append(process);
            }
        }
    }

    // 清理已经不存在的进程的记录
    QStringList currentPids = pidDirs.filter(QRegExp("^\\d+$"));
    QStringList prevPids = m_prevProcessTimes.keys();
    for (const QString &pid : prevPids) {
        if (!currentPids.contains(pid)) {
            m_prevProcessTimes.remove(pid);
        }
    }

    if (m_processList != processes) {
        m_processList = processes;
        emit processListChanged();
    }
}

void SystemInfo::updateDiskInfo()
{
    QVariantList disks;
    QFile mounts("/proc/mounts");
    
    if (mounts.open(QIODevice::ReadOnly)) {
        QTextStream stream(&mounts);
        QString line;
        
        while (stream.readLineInto(&line)) {
            QStringList parts = line.split(" ");
            if (parts.size() >= 4) {
                QString device = parts[0];
                QString mountPoint = parts[1];
                QString filesystem = parts[2];
                
                // 排除一些特殊的文件系统
                if (!filesystem.contains("sysfs") && 
                    !filesystem.contains("proc") && 
                    !filesystem.contains("devpts") && 
                    !filesystem.contains("securityfs") && 
                    !filesystem.contains("cgroup") &&
                    !filesystem.contains("pstore") &&
                    !filesystem.contains("debugfs") &&
                    !filesystem.contains("hugetlbfs") &&
                    !filesystem.contains("mqueue") &&
                    !filesystem.contains("fusectl") &&
                    !filesystem.contains("configfs") &&
                    !filesystem.contains("binfmt_misc") &&
                    !mountPoint.startsWith("/sys") &&
                    !mountPoint.startsWith("/run/user")) {
                    
                    struct statvfs info;
                    if (statvfs(mountPoint.toLocal8Bit().constData(), &info) == 0) {
                        QVariantMap disk;
                        quint64 blockSize = info.f_frsize;
                        quint64 totalSize = blockSize * info.f_blocks;
                        quint64 freeSpace = blockSize * info.f_bfree;
                        quint64 usedSpace = totalSize - freeSpace;
                        
                        // 对于网络文件系统和特殊文件系统，显示设备名而不是设备路径
                        if (device.startsWith("//") || device.contains(":")) {
                            // 网络文件系统，显示共享名
                            disk["device"] = device.split("/").last();
                        } else if (!device.startsWith("/")) {
                            // 特殊文件系统，直接显示类型
                            disk["device"] = filesystem;
                        } else {
                            disk["device"] = device;
                        }

                        disk["mountPoint"] = mountPoint;
                        disk["type"] = filesystem;
                        
                        // 对于某些特殊文件系统，可能没有大小信息
                        if (totalSize > 0) {
                            disk["total"] = formatSize(totalSize);
                            disk["used"] = formatSize(usedSpace);
                            disk["free"] = formatSize(freeSpace);
                            double usedPercentage = (double)usedSpace / totalSize * 100;
                            disk["usedPercentage"] = usedPercentage;
                        } else {
                            disk["total"] = "-";
                            disk["used"] = "-";
                            disk["free"] = "-";
                            disk["usedPercentage"] = 0;
                        }
                        
                        disks.append(disk);
                    }
                }
            }
        }
        mounts.close();
    }

    // 按挂载点排序
    std::sort(disks.begin(), disks.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap()["mountPoint"].toString() < b.toMap()["mountPoint"].toString();
    });

    if (m_diskList != disks) {
        m_diskList = disks;
        emit diskListChanged();
    }
}

void SystemInfo::updateNetworkInfo()
{
    QVariantList networks;
    QFile netdev("/proc/net/dev");
    
    if (netdev.open(QIODevice::ReadOnly)) {
        QTextStream stream(&netdev);
        QString line;
        
        // 跳过前两行头部信息
        stream.readLine();
        stream.readLine();
        
        while (stream.readLineInto(&line)) {
            QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 10) {
                QString interface = parts[0].remove(':');
                if (interface == "lo") continue;  // 跳过回环接口
                
                quint64 rxBytes = parts[1].toULongLong();  // 接收字节数
                quint64 txBytes = parts[9].toULongLong();  // 发送字节数
                
                double rxSpeed = 0;
                double txSpeed = 0;
                
                if (m_prevNetworkBytes.contains(interface)) {
                    auto prev = m_prevNetworkBytes[interface];
                    rxSpeed = (rxBytes - prev.first) / (updateInterval() / 1000.0);  // 转换为每秒
                    txSpeed = (txBytes - prev.second) / (updateInterval() / 1000.0);
                }
                
                m_prevNetworkBytes[interface] = qMakePair(rxBytes, txBytes);
                
                QVariantMap network;
                network["interface"] = interface;
                network["download"] = formatSpeed(rxSpeed);
                network["upload"] = formatSpeed(txSpeed);
                network["total"] = formatSize(rxBytes + txBytes);  // 添加总流量
                networks.append(network);
            }
        }
        netdev.close();
    }

    if (m_networkList != networks) {
        m_networkList = networks;
        emit networkListChanged();
    }
}

QString SystemInfo::formatSpeed(double bytesPerSecond)
{
    const char* units[] = {"B/s", "KB/s", "MB/s", "GB/s"};
    int unit = 0;
    
    while (bytesPerSecond >= 1024 && unit < 3) {
        bytesPerSecond /= 1024;
        unit++;
    }
    
    return QString("%1 %2").arg(bytesPerSecond, 0, 'f', 1).arg(units[unit]);
}

void SystemInfo::killProcess(qint64 pid)
{
    kill(pid, SIGTERM);
}

void SystemInfo::updateProcessPriority(qint64 pid, int priority)
{
    setpriority(PRIO_PROCESS, pid, priority);
}

QVariantList SystemInfo::filteredProcessList(const QString &filter, const QString &sortBy, bool ascending)
{
    QVariantList filtered;
    QString lowerFilter = filter.toLower();

    // 过滤进程
    for (const QVariant &item : m_processList) {
        QVariantMap process = item.toMap();
        QString name = process["name"].toString().toLower();
        QString user = process["user"].toString().toLower();
        QString pid = process["pid"].toString();

        // 匹配进程名、用户名或PID
        if (lowerFilter.isEmpty() ||
            name.contains(lowerFilter) ||
            user.contains(lowerFilter) ||
            pid.contains(lowerFilter))
        {
            filtered.append(process);
        }
    }

    // 排序
    std::sort(filtered.begin(), filtered.end(),
        [sortBy, ascending](const QVariant &a, const QVariant &b) {
            QVariantMap mapA = a.toMap();
            QVariantMap mapB = b.toMap();

            if (sortBy == "cpu") {
                double cpuA = mapA["cpu"].toString().toDouble();
                double cpuB = mapB["cpu"].toString().toDouble();
                return ascending ? cpuA < cpuB : cpuA > cpuB;
            }
            else if (sortBy == "memory") {
                QString memA = mapA["memory"].toString();
                QString memB = mapB["memory"].toString();
                // 移除单位并转换为数值
                double valueA = memA.split(" ")[0].toDouble();
                double valueB = memB.split(" ")[0].toDouble();
                // 考虑单位
                if (memA.contains("GB")) valueA *= 1024;
                if (memB.contains("GB")) valueB *= 1024;
                return ascending ? valueA < valueB : valueA > valueB;
            }
            else if (sortBy == "pid") {
                qint64 pidA = mapA["pid"].toString().toLongLong();
                qint64 pidB = mapB["pid"].toString().toLongLong();
                return ascending ? pidA < pidB : pidA > pidB;
            }
            else {
                QString strA = mapA[sortBy].toString().toLower();
                QString strB = mapB[sortBy].toString().toLower();
                return ascending ? strA < strB : strA > strB;
            }
        });

    return filtered;
}

void SystemInfo::refreshProcesses()
{
    updateProcessList();
}

QString SystemInfo::kernelVersion() const
{
    return m_kernelVersion;
}

QString SystemInfo::hostname() const
{
    return m_hostname;
}

QString SystemInfo::desktopEnvironment() const
{
    return m_desktopEnvironment;
} 