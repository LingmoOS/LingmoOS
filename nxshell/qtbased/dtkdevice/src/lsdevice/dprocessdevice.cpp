// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dprocessdevice.h"
#include "common.h"
#include "osutils.h"
#include "packet.h"
#include <QMap>
#include <QList>
#include <QDebug>

#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <grp.h>

#define PROC_PATH "/proc"
#define PROC_STAT_PATH "/proc/%u/stat"
#define PROC_STATUS_PATH "/proc/%u/status"
#define PROC_STATM_PATH "/proc/%u/statm"
#define PROC_CMDLINE_PATH "/proc/%u/cmdline"
#define PROC_ENVIRON_PATH "/proc/%u/environ"
#define PROC_IO_PATH "/proc/%u/io"
#define PROC_FD_PATH "/proc/%u/fd"
#define PROC_FD_NAME_PATH "/proc/%u/fd/%s"
#define PROC_SCHEDSTAT_PATH "/proc/%u/schedstat"

using namespace common;
using namespace common::error;
using namespace common::alloc;
using namespace common::init;
using namespace core::system;


DDEVICE_BEGIN_NAMESPACE

struct processInfo {
    pid_t pid;
    QHash<QString, QString> environment;
    QString groupName;
    DProcessDevice::DProcessBaseInfo baseInfo;
    DProcessDevice::DProcessMemoryInfo memoryInfo;
    DProcessDevice::DProcessIOInfo ioInfo;
    DProcessDevice::DProcessNetworkInfo networkInfo;  //void NetInfo::resdNetInfo()
    DProcessDevice::DProcessStatus ProcessStatus;
};

/**
 * @brief Network interface socket io stat structure
 */
struct sock_io_stat_t {
    ino_t ino; // socket inode
    qulonglong rx_bytes; // received bytes
    qulonglong rx_packets; // received packets
    qulonglong tx_bytes; // sent bytes
    qulonglong tx_packets; // sent packets
};
// socket io stat typedef
using SockIOStat = QSharedPointer<struct sock_io_stat_t>;
using PacketPayload      = QSharedPointer<struct packet_payload_t>;
using PacketPayloadQueue = QQueue<PacketPayload>;

class DProcessDevicePrivate
{
public:
    explicit DProcessDevicePrivate(DProcessDevice *parent)
        : q_ptr(parent)
    {
        scanAllPids();
        for (int i = 0; i < m_allPids.size(); ++i) {
            pid_t pid = m_allPids.at(i);

            bool bStat = readStat(pid);
            bool bStatus = readStatus(pid);
            bool bEnviron = readEnviron(pid);
            bool bStatm = readStatm(pid);
            bool bIO = readIO(pid);
            bool bCmdline =  readCmdline(pid);
            processInfo info;
            info.pid = pid;
            info.baseInfo.pid = pid;

            if (bStat || bStatus) {
                info.ProcessStatus = m_ProcessStatus;
                info.baseInfo.name = m_baseInfo.name;
                info.baseInfo.mask = m_baseInfo.mask;
            }
            if (bEnviron) {
                info.environment = m_environment;
                m_environmentMap.insert(pid, m_environment);
            }
            if (bStatm) {
                info.memoryInfo = m_memoryInfo;
            }
            if (bIO) {
                info.ioInfo = m_ioInfo;
            }
            if (bCmdline) {
                info.baseInfo.cmdline = m_baseInfo.cmdline;
            }
            if (bStat || bStatus || bCmdline)
                m_baseInfoMap.insert(pid, m_baseInfo);
        }
    }

    void scanAllPids();
    bool groupName(gid_t gid);

    bool  readEnviron(pid_t pid);
    bool  readStat(pid_t pid);
    bool  readStatus(pid_t pid);
    bool  readStatm(pid_t pid);
    bool  readIO(pid_t pid);
    bool  readCmdline(pid_t pid);
    void  readSockInodes(pid_t pid);
    void handleNetData(pid_t pid);

    QList<pid_t> m_allPids;

    QMap< pid_t, QHash<QString, QString> >  m_environmentMap;
    QMap< gid_t, QString > m_groupNameMap;
    QMap< pid_t, DProcessDevice::DProcessBaseInfo > m_baseInfoMap;
    QMap< pid_t, DProcessDevice::DProcessMemoryInfo > m_memoryInfoMap;
    QMap< pid_t, DProcessDevice::DProcessIOInfo > m_ioInfoMap;
    QMap< pid_t, DProcessDevice::DProcessNetworkInfo > m_networkInfoMap;  //void NetInfo::resdNetInfo()
    QMap< pid_t, DProcessDevice::DProcessStatus > m_ProcessStatusMap;

    /**
     * @brief Get socket io stat data with specified inode (thread safe accessor)
     * @param ino Socket inode
     * @param stat Socket io stat data
     * @return Return true if stat data found in stat cache, otherwise return false
     */
    inline bool getSockIOStatByInode(ino_t ino, SockIOStat &stat)
    {
        bool ok = false;

        // check stat data existence
        if (m_sockIOStatMap.contains(ino)) {
            stat = m_sockIOStatMap[ino];
            // remove stat data from cache
            m_sockIOStatMap.remove(ino);
            ok = true;
        }

        return ok;
    }

private:
    DProcessDevice *q_ptr = nullptr;

    QHash<QString, QString> m_environment;
    QString m_groupName;
    DProcessDevice::DProcessBaseInfo m_baseInfo;
    DProcessDevice::DProcessMemoryInfo m_memoryInfo;
    DProcessDevice::DProcessIOInfo m_ioInfo;
    DProcessDevice::DProcessNetworkInfo m_networkInfo;  //void NetInfo::resdNetInfo()
    DProcessDevice::DProcessStatus m_ProcessStatus;
    QList<ino_t> m_sockInodesLst;
    PacketPayloadQueue  m_localPendingPackets   {}; // local cache
    // pending packet queue
    PacketPayloadQueue  m_pendingPackets        {};

    // socket inode to io stat mapping
    QMap<ino_t, SockIOStat> m_sockIOStatMap     {};
};


void DProcessDevicePrivate::scanAllPids()
{
    if (!pushd(PROC_PATH))
        return ;

    struct dirent **namelist;
    int count = scandir(".", &namelist, selectdir, alphasort);
    for (int i = 0; i < count; i ++) {
        if (isdigit(namelist[i]->d_name[0])) {
            auto pid = pid_t(atoi(namelist[i]->d_name));
            m_allPids.append(pid);
        }
        free(namelist[i]);
    }
    if (namelist)
        free(namelist);
    popd();

    return ;
}

bool  DProcessDevicePrivate::groupName(gid_t gid)
{
    bool ok {true};
    auto *grp = getgrgid(gid);
    if (grp) {
        m_groupName = grp->gr_name;
        return ok;
    } else
        m_groupName = QString();

    return !ok;
}

void DProcessDevicePrivate::handleNetData(pid_t pid)
{
    readSockInodes(pid);

    // append pending queue packets to local queue, so we dont lock the mutex for too long
    m_localPendingPackets.append(m_pendingPackets);
    m_pendingPackets.clear();
    // process payload queue
    while (!m_localPendingPackets.isEmpty()) {
        auto payload = m_localPendingPackets.dequeue();

        // sum up sock io stat by inode
        auto ino = payload->ino;
        if (m_sockIOStatMap.contains(ino)) {
            // sum up sock io stat if already exists with same inode stat
            auto &hist = m_sockIOStatMap[ino];
            if (payload->direction == kInboundPacket) {
                hist->rx_bytes += payload->payload;
                hist->rx_packets++;
            } else if (payload->direction == kOutboundPacket) {
                hist->tx_bytes += (payload->payload * 2);
                hist->tx_packets++;
            }
        } else {
            // add new sock io stat if sock ino no exists in cache before
            auto stat = QSharedPointer<struct sock_io_stat_t>::create();
            stat->ino = payload->ino;
            if (payload->direction == kInboundPacket) {
                stat->rx_bytes = payload->payload;
                stat->rx_packets++;
            } else if (payload->direction == kOutboundPacket) {
                stat->tx_bytes = payload->payload;
                stat->tx_packets++;
            }
            m_sockIOStatMap[stat->ino] = stat;
        }
    }

}

// read /proc/[pid]/fd
void DProcessDevicePrivate::readSockInodes(pid_t pid)
{
    struct dirent *dp;
    char path[128], fdp[256 + 32];
    struct stat sbuf;

    sprintf(path, PROC_FD_PATH, pid);

    errno = 0;
    // open /proc/[pid]/fd dir
    uDir dir(opendir(path));
    if (!dir) {
        print_errno(errno, QString("open %1 failed").arg(path));
        return;
    }

    // enumerate each entry
    while ((dp = readdir(dir.get()))) {
        // only if entry name starts with a digit
        if (isdigit(dp->d_name[0])) {
            // open /proc/[pid]/fd/[fd]
            sprintf(fdp, PROC_FD_NAME_PATH, pid, dp->d_name);
            memset(&sbuf, 0, sizeof(struct stat));
            if (!stat(fdp, &sbuf)) {
                // get inode if it's a socket descriptor
                if (S_ISSOCK(sbuf.st_mode)) {
                    m_sockInodesLst << sbuf.st_ino;
                }
            } // ::if(stat)
        } // ::if(isdigit)
    } // ::while(readdir)
    if (errno) {
        print_errno(errno, QString("read %1 failed").arg(path));
    }

    qulonglong sum_recv = 0;
    qulonglong sum_send = 0;

    for (int i = 0; i < m_sockInodesLst.size(); ++i) {
        SockIOStat sockIOStat;
        bool result = getSockIOStatByInode(m_sockInodesLst[i], sockIOStat);
        if (result) {
            sum_recv += sockIOStat->rx_bytes;
            sum_send += sockIOStat->tx_bytes;
        }
    }

    DProcessDevice::DProcessNetworkInfo   NetworkInfo;
    NetworkInfo.recvBytes = sum_recv;
    NetworkInfo.sentBytes = sum_send;
    m_networkInfoMap.insert(pid, NetworkInfo);
}

// read /proc/[pid]/stat
bool DProcessDevicePrivate::readStat(pid_t pid)
{
    bool ok {true};
    char path[256];
    QByteArray buf;
    int fd, rc;
    ssize_t sz;
    char *pos, *begin;

    buf.reserve(1025);

    errno = 0;
    sprintf(path, PROC_STAT_PATH, pid);
    // open /proc/[pid]/stat
    if ((fd = open(path, O_RDONLY)) < 0) {
        print_errno(errno, QString("open %1 failed").arg(path));
        return !ok;
    }
    // read data
    sz = read(fd, buf.data(), 1024);
    close(fd);
    if (sz < 0) {
        print_errno(errno, QString("read %1 failed").arg(path));
        return !ok;
    }
    buf.data()[sz] = '\0';

    // get ProcesInfo name between (...)
    begin = strchr(buf.data(), '(');
    begin += 1;

    pos = strrchr(buf.data(), ')');
    if (!pos) {
        return !ok;
    }

    *pos = '\0';
    // ProcesInfo name (may be truncated by kernel if it's too long)
    m_baseInfo.name = QByteArray(begin);

    pos += 2;

    //****************3**4**5***********************************14***15**
    rc = sscanf(pos, "%c %d %d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu"
                //*16***17******19*20******22************************************
                " %lld %lld %*d %d %u %*u %llu %*u %*u %*u %*u %*u %*u %*u %*u"
                //********************************39*40*41******43***44**********
                " %*u %*u %*u %*u %*u %*u %*u %*u %u %u %u %*u %llu %lld\n",
                &m_ProcessStatus.state, // 3
                &m_ProcessStatus.ppid, // 4
                &m_ProcessStatus.pgid, // 5
                &m_ProcessStatus.utime, // 14
                &m_ProcessStatus.stime, // 15
                &m_ProcessStatus.cutime, // 16
                &m_ProcessStatus.cstime, // 17
                &m_ProcessStatus.nice, // 19
                &m_ProcessStatus.nthreads, // 20
                &m_ProcessStatus.startTime, // 22
                &m_ProcessStatus.processor, // 39
                &m_ProcessStatus.rtPrio, // 40
                &m_ProcessStatus.policy, // 41
                &m_ProcessStatus.guestTime, // 43
                &m_ProcessStatus.cguestTime); // 44
    if (rc < 15) {
        return  !ok;
    }
    // have guest & cguest time
    if (rc < 17) {
        m_ProcessStatus.guestTime = m_ProcessStatus.cguestTime = 0;
    }
    return ok;
}

// read /proc/[pid]/status
bool DProcessDevicePrivate::readStatus(pid_t pid)
{
    bool ok {true};
    const size_t bsiz = 256;
    QByteArray buf;
    char path[128];

    buf.reserve(bsiz);
    sprintf(path, PROC_STATUS_PATH, pid);

    errno = 0;
    uFile fp(fopen(path, "r"));
    // open /proc/[pid]/status
    if (!fp) {
        print_errno(errno, QString("open %1 failed").arg(path));
        return !ok;
    }

    // scan each line
    while (fgets(buf.data(), bsiz, fp.get())) {
        if (!strncmp(buf.data(), "Umask:", 6)) {
            sscanf(buf.data() + 7, "%u", &m_baseInfo.mask);
        } else if (!strncmp(buf.data(), "State:", 6)) {
            sscanf(buf.data() + 7, "%c %*s", &m_ProcessStatus.state);
        } else if (!strncmp(buf.data(), "Uid:", 4)) {
            sscanf(buf.data() + 5, "%u %u %u %u",
                   &m_ProcessStatus.uid,
                   &m_ProcessStatus.euid,
                   &m_ProcessStatus.suid,
                   &m_ProcessStatus.fuid);
        } else if (!strncmp(buf.data(), "Gid:", 4)) {
            sscanf(buf.data() + 5, "%u %u %u %u",
                   &m_ProcessStatus.gid,
                   &m_ProcessStatus.egid,
                   &m_ProcessStatus.sgid,
                   &m_ProcessStatus.fgid);
        }
    } // ::while(fgets)

    if (ferror(fp.get())) {
        print_errno(errno, QString("read %1 failed").arg(path));
        return !ok;
    }
    return ok;
}

//------------readCmdline()
bool DProcessDevicePrivate::readCmdline(pid_t pid)
{
    bool ok {true};
    char cmdpath[128] {};
    const size_t bsiz = 4096;
    QByteArray cmd;
    cmd.reserve(bsiz);
    size_t nb;
    char *cmdbegin, *cmdcur, *cmdend;

    sprintf(cmdpath, PROC_CMDLINE_PATH, pid);

    errno = 0;
    // open /proc/[pid]/cmdline
    uFile fp(fopen(cmdpath, "r"));
    if (!fp) {
        print_errno(errno, QString("open %1 failed").arg(cmdpath));
        return !ok;
    }

    nb = fread(cmd.data(), 1, bsiz - 1, fp.get());
    if (ferror(fp.get())) {
        print_errno(errno, QString("read %1 failed").arg(cmdpath));
        return !ok;
    }

    cmd.data()[nb] = '\0';

    cmdbegin = cmdcur = cmd.data();
    cmdend = cmd.data() + nb;
    while (cmdcur < cmdend) {
        // cmdline may sperarted by null character
        if (*cmdcur == '\0') {
            QByteArray buf(cmdbegin);
            m_baseInfo.cmdline = buf;
            cmdbegin = cmdcur + 1;
        }
        ++cmdcur;
    }
    if (cmdbegin < cmdend) {
        QByteArray buf(cmdbegin);
        m_baseInfo.cmdline = buf;
    }
    return ok;
}

// read /proc/[pid]/io
bool DProcessDevicePrivate::readIO(pid_t pid)
{
    bool ok {true};
    const size_t bsiz = 128;
    char path[128], buf[bsiz];

    sprintf(path, PROC_IO_PATH, pid);

    errno = 0;
    // open /proc/[pid]/io
    uFile fp(fopen(path, "r"));
    if (!fp) {
        print_errno(errno, QString("open %1 failed").arg(path));
        return !ok;
    }

    // scan each line
    while (fgets(buf, bsiz, fp.get())) {
        if (!strncmp(buf, "read_bytes", 10)) {
            sscanf(buf + 12, "%llu", &m_ioInfo.readBytes);
        } else if (!strncmp(buf, "write_bytes", 11)) {
            sscanf(buf + 13, "%llu", &m_ioInfo.writeBytes);
        } else if (!strncmp(buf, "cancelled_write_bytes", 21)) {
            sscanf(buf + 23, "%llu", &m_ioInfo.cancelledWriteBytes);
        }
    } // ::while(fgets)
    if (ferror(fp.get())) {
        print_errno(errno, QString("read %1 failed").arg(path));
        return !ok;
    }
    return ok;
}

// read /proc/[pid]/statm
bool DProcessDevicePrivate::readStatm(pid_t pid)
{
    bool ok {true};
    int fd;
    const size_t bsiz = 1024;
    char path[128] {}, buf[bsiz + 1] {};
    ssize_t nr;

    int shift = 0;
    long size;
    unsigned int kb_shift;

    /* One can also use getpagesize() to get the size of a page */
    if ((size = sysconf(_SC_PAGESIZE)) == -1) {
        perror("sysconf");
    }
    size >>= 10; /* Assume that a page has a minimum size of 1 kB */
    while (size > 1) {
        shift++;
        size >>= 1;
    }
    kb_shift = uint(shift);

    sprintf(path, PROC_STATM_PATH, pid);

    errno = 0;
    // open /proc/[pid]/statm
    if ((fd = open(path, O_RDONLY)) < 0) {
        print_errno(errno, QString("open %1 failed").arg(path));
        return !ok;
    }

    nr = read(fd, buf, bsiz);
    close(fd);
    if (nr < 0) {
        print_errno(errno, QString("read %1 failed").arg(path));
        return !ok;
    }

    buf[nr] = '\0';
    // get resident set size & resident shared size in pages
    nr = sscanf(buf, "%llu %llu %llu", &m_memoryInfo.vmsize, &m_memoryInfo.rss, &m_memoryInfo.shm);
    if (nr != 3) {
        m_memoryInfo.vmsize = 0;
        m_memoryInfo.rss = 0;
        m_memoryInfo.shm = 0;
        print_errno(errno, QString("read %1 failed").arg(path));
    } else {
        // convert to kB
        m_memoryInfo.vmsize <<= kb_shift;
        m_memoryInfo.rss <<= kb_shift;
        m_memoryInfo.shm <<= kb_shift;
    }
    return ok;
}

// read /proc/[pid]/environ
bool DProcessDevicePrivate::readEnviron(pid_t pid)
{
    bool ok {true};
    const size_t sz = 1024;
    char path[128] {};
    ssize_t nb;
    QByteArray sbuf {};
    char buf[sz + 1] {};
    int fd;

    sprintf(path, PROC_ENVIRON_PATH, pid);

    errno = 0;
    // open /proc/[pid]/environ
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        print_errno(errno, QString("open %1 failed").arg(path));
        return !ok;
    }

    while ((nb = read(fd, buf, sz))) {
        buf[nb] = '\0';
        sbuf.append(buf, int(nb));
    }
    close(fd);

    if (nb == 0 && errno != 0) {
        print_errno(errno, QString("read %1 failed").arg(path));
        return !ok;
    }

    if (sbuf.size() > 0) {
        auto elist = sbuf.split('\0');
        for (auto it : elist) {
            // it: name=value pair
            auto kvp = it.split('=');
            if (kvp.size() == 2) {
                m_environment[kvp[0]] = kvp[1];
            }
        }
    } // ::if(sbuf)
    return ok;
}

DProcessDevice::DProcessDevice(QObject *parent)
    : QObject(parent)
    , d_ptr(new DProcessDevicePrivate(this))
{
}

DProcessDevice::~DProcessDevice()
{
}

QList<pid_t> DProcessDevice::allPids()
{
    Q_D(DProcessDevice);
    return (d->m_allPids);
}

QHash<QString, QString> DProcessDevice::environment(pid_t pid)
{
    Q_D(DProcessDevice);
    if (d->m_environmentMap.contains(pid)) {
        return d->m_environmentMap[pid];
    }
    return QHash<QString, QString>();
}

QString DProcessDevice::groupName(const gid_t &gid)
{
    Q_D(DProcessDevice);
    if (d->m_groupNameMap.contains(gid)) {
        return d->m_groupNameMap[gid];
    }
    return QString();
}

DProcessDevice::DProcessBaseInfo DProcessDevice::baseInfo(pid_t pid)
{
    Q_D(DProcessDevice);
    if (d->m_baseInfoMap.contains(pid)) {
        return d->m_baseInfoMap[pid];
    }
    return DProcessBaseInfo();
}

DProcessDevice::DProcessMemoryInfo DProcessDevice::memoryInfo(pid_t pid)
{
    Q_D(DProcessDevice);
    if (d->m_memoryInfoMap.contains(pid)) {
        return d->m_memoryInfoMap[pid];
    }
    return DProcessMemoryInfo();
}

DProcessDevice::DProcessIOInfo DProcessDevice::ioInfo(pid_t pid)
{
    Q_D(DProcessDevice);
    if (d->m_ioInfoMap.contains(pid)) {
        return d->m_ioInfoMap[pid];
    }
    return DProcessIOInfo();
}

DProcessDevice::DProcessNetworkInfo DProcessDevice::networkInfo(pid_t pid)
{
    Q_D(DProcessDevice);
    if (d->m_networkInfoMap.contains(pid)) {
        return d->m_networkInfoMap[pid];
    }
    return DProcessNetworkInfo();
}

DProcessDevice::DProcessStatus DProcessDevice::status(pid_t pid)
{
    Q_D(DProcessDevice);
    if (d->m_ProcessStatusMap.contains(pid)) {
        return d->m_ProcessStatusMap[pid];
    }
    return DProcessStatus();
}

DDEVICE_END_NAMESPACE
