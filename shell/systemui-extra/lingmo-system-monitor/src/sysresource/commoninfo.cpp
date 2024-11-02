#include "commoninfo.h"
#include <QDebug>

// Cpu 信息
QString cpuInfo::getUtilization()
{
    return QString::number(kdk_rti_get_cpu_current_usage()*100, 'f', 2) + "%";
}

QString cpuInfo::getCurrentFrequency()
{
    if (kdk_cpu_get_freq_MHz() != nullptr) {
        float a = atol(kdk_cpu_get_freq_MHz());
        QString currentFreq = QString::number(a/1000, 'f', 2) + "GHz";
        return currentFreq;
    }
    return "";
}

// 需要sudo权限
QString cpuInfo::getFrequency()
{
    QString minFreq = QString::number(kdk_cpu_get_min_freq_MHz()/1000, 'f', 2) + "GHz";
    QString maxFreq = QString::number(kdk_cpu_get_max_freq_MHz()/1000, 'f', 2) + "GHz";

    return minFreq + "~" + maxFreq;
}

QString cpuInfo::getUpTime()
{
    char *ret = kdk_cpu_get_running_time();
    QString runningTime =  QString::fromUtf8(ret);
    free(ret);
    return runningTime;
}

QString cpuInfo::getSockets()
{
    return QString::number(kdk_cpu_get_sockets());
}

QString cpuInfo::getLogicalProcessors()
{
    return QString::number(kdk_cpu_get_process());
}

QString cpuInfo::getVirtualization()
{
    QString virtualization = kdk_cpu_get_virt();
    if (virtualization == nullptr) {
        virtualization = "NULL";
    }
    return virtualization;
}

// 需要sudo权限
QString cpuInfo::getL1iCache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L1i_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getL1dCache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L1d_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getL2Cache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L2_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getL3Cache()
{
    return commom::format::Format_Memory(kdk_cpu_get_L3_cache(), KDK_KILOBYTE);
}

QString cpuInfo::getLoadAverage()
{
    kdk_loadavg loadAvg = kdk_system_get_loadavg();
    QString avg1 = QString::number(loadAvg.loadavg_1m, 'f', 2);
    QString avg5 = QString::number(loadAvg.loadavg_5m, 'f', 2);
    QString avg15 = QString::number(loadAvg.loadavg_15m, 'f', 2);
    return avg1 + " " + avg5 + " " + avg15;
}

QString cpuInfo::getFileDescriptors()
{
    return QString::number(kdk_system_get_file_descriptor());
}

QString cpuInfo::getProcesses()
{
    return QString::number(kdk_system_get_process_nums());
}

QString cpuInfo::getThreads()
{
    return QString::number(kdk_system_get_thread_nums());
}

QString cpuInfo::getHostName()
{
    char *ret = kdk_system_get_hostName();
    QString hostname = QString::fromUtf8(ret);
    free(ret);
    return hostname;
}

QString cpuInfo::getOSType()
{
    char *ret = kdk_system_get_architecture();
    QString architecture = QString::fromUtf8(ret);
    free(ret);
    return architecture;
}

QString cpuInfo::getVersion()
{
    char *ret = kdk_system_get_version(0);
    QString version = QString::fromUtf8(ret);
    free(ret);
    return version;
}

// 内存信息
QString memInfo::getUsed()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_res_usage_KiB(), KDK_KILOBYTE);
}

QString memInfo::getAvailable()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_res_available_KiB(), KDK_KILOBYTE);
}

QString memInfo::getShared()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_shared_KiB(), KDK_KILOBYTE);
}

QString memInfo::getCached()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_cached_KiB(), KDK_KILOBYTE);
}

QString memInfo::getBuffers()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_buffers_KiB(), KDK_KILOBYTE);
}

QString memInfo::getCachedSwap()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_swap_cached_KiB(), KDK_KILOBYTE);
}

QString memInfo::getActive()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_active_KiB(), KDK_KILOBYTE);
}

QString memInfo::getInActive()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_inactive_KiB(), KDK_KILOBYTE);
}

QString memInfo::getDirty()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_dirty_KiB(), KDK_KILOBYTE);
}

QString memInfo::getMapped()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_map_KiB(), KDK_KILOBYTE);
}

QString memInfo::getTotalSwap()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_swap_total_KiB(), KDK_KILOBYTE);
}

QString memInfo::getFreeSwap()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_swap_free_KiB(), KDK_KILOBYTE);
}

QString memInfo::getSlab()
{
    return commom::format::Format_Memory(kdk_rti_get_mem_slab_KiB(), KDK_KILOBYTE);
}

// 网络信息
QString netInfo::getIPv4Address(const char *strKey)
{
    return kdk_nc_get_private_ipv4(strKey);
}

QString netInfo::getIPv4Netmask(const char *strKey, const char *addr)
{
    char *netmask = kdk_nc_get_netmask(strKey, addr);
    QString _netmask = QString::fromUtf8(netmask);
    free(netmask);
    return _netmask;
}

QString netInfo::getIPv4Boardcast(const char *strKey, const char *addr)
{
    char *boardcast = kdk_nc_get_broadAddr(strKey, addr);
    QString _boardcast = QString::fromUtf8(boardcast);
    free(boardcast);
    return _boardcast;
}

QString netInfo::getIPv6Address(const char *strKey)
{
    return kdk_nc_get_private_ipv6(strKey);
}

QString netInfo::getIPv6NetmaskLen(const char *strKey, const char *addr)
{
    return QString::number(kdk_nc_get_mask_length(0, strKey, addr));
}

QString netInfo::getIPv6Scope(const char *strKey, const char *addr)
{
    return QString::number(kdk_nc_get_scope(strKey, addr));
}

QString netInfo::getConnectType(const char *strKey)
{
    return kdk_nc_get_conn_type(strKey);
}

QString netInfo::getNetName(const char *strKey)
{
    char *ret = kdk_nc_get_wifi_name(strKey);
    QString netName = QString::fromUtf8(ret);
    free(ret);
    return netName;
}

QString netInfo::getSignalQuality(const char *strKey)
{
    return QString::number(kdk_nc_get_wifi_signal_qual(strKey));
}

QString netInfo::getSignalStrength(const char *strKey)
{
     return QString::number(kdk_nc_get_wifi_signal_level(strKey));
}

QString netInfo::getNoiseLevel(const char *strKey)
{
    return QString::number(kdk_nc_get_wifi_noise(strKey));
}

QString netInfo::getMacAddress(const char *strKey)
{
    char *ret = kdk_nc_get_phymac(strKey);
    QString mac = QString::fromUtf8(ret);
    free(ret);
    return mac;
}

QString netInfo::getSpeed(const char *strKey)
{
    char *ret = kdk_nc_get_speed(strKey);
    QString speed = QString::fromUtf8(ret);
    free(ret);
    return speed.isEmpty() ? "0 Mb/s" : speed;
}

QString netInfo::getRecvPackets(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_packets(strKey));
}

QString netInfo::getRecvTotal(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_bytes(strKey));
}

QString netInfo::getRecvErrors(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_errors(strKey));
}

QString netInfo::getRecvDropped(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_dropped(strKey));
}

QString netInfo::getRecvFIFO(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_fifo_errors(strKey));
}

QString netInfo::getFrameErrors(const char *strKey)
{
    return QString::number(kdk_nc_get_rx_frame_errors(strKey));
}

QString netInfo::getSendPackets(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_packets(strKey));
}

QString netInfo::getSendTotal(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_bytes(strKey));
}
QString netInfo::getSendErrors(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_errors(strKey));
}

QString netInfo::getSendDropped(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_dropped(strKey));
}

QString netInfo::getSendFIFO(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_fifo_errors(strKey));
}

QString netInfo::getCarrierLoss(const char *strKey)
{
    return QString::number(kdk_nc_get_tx_carrier_errors(strKey));
}

QStringList netInfo::getUpCards()
{
    char **cardlist = kdk_nc_get_upcards();
    QStringList upCardList;
    int index = 0;
    while (cardlist != nullptr && cardlist[index]) {
        if (QString(cardlist[index]) != "lo") {
            upCardList.append(cardlist[index]);
        }
        index++;
    }
    kdk_nc_freeall(cardlist);
    return upCardList;
}

QStringList netInfo::getAllCards()
{
    char **cardlist = kdk_nc_get_cardlist();
    QStringList allCardList;
    int index = 0;
    while (cardlist != nullptr && cardlist[index]) {
        if (QString(cardlist[index]) != "lo") {
            allCardList.append(cardlist[index]);
        }
        index++;
    }
    kdk_nc_freeall(cardlist);
    return allCardList;
}

QList<INet4Addr> netInfo::getIPv4AddressList(const char *strKey)
{
    char **list = kdk_nc_get_ipv4(strKey);
    QList<INet4Addr> IPv4List;
    int index = 0;
    while (list != nullptr && list[index] != nullptr) {
        INet4Addr netAddr;
        netAddr.addr = list[index];
        netAddr.mask = getIPv4Netmask(strKey, list[index]);
        netAddr.bcast = getIPv4Boardcast(strKey, list[index]);
        IPv4List.append(netAddr);
        index++;
    }
    kdk_nc_freeall(list);
    return IPv4List;
}

QList<INet6Addr> netInfo::getIPv6AddressList(const char *strKey)
{
    char **list = kdk_nc_get_ipv6(strKey);
    QList<INet6Addr> IPv6List;
    int index = 0;
    while (list != nullptr && list[index] != nullptr) {
        INet6Addr netAddr;
        netAddr.addr = list[index];
        netAddr.prefixlen = getIPv6NetmaskLen(strKey, list[index]);
        netAddr.scope = getIPv6Scope(strKey, list[index]);
        IPv6List.append(netAddr);
        index++;
    }
    kdk_nc_freeall(list);
    return IPv6List;
}

namespace commom {


const QString format::Format_Memory(long data, KDKVolumeBaseType baseType)
{
    char resultData[50] = {0};

    const char* b = (std::to_string(data)+ UnitSuffixMem[baseType]).c_str();
    kdkVolumeBaseCharacterConvert(b, KDK_GIGABYTE, resultData);

    return data == 0 ? "-" : QString::fromUtf8(resultData);
}

}

