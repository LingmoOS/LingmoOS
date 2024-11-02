#ifndef COMMONINFO_H
#define COMMONINFO_H

#include <QObject>
#include <QVariant>
#include <stdlib.h>

#include "lingmosdk/lingmosdk-base/kyutils.h"
#include <lingmosdk/lingmosdk-system/libkyrtinfo.h>
#include <lingmosdk/lingmosdk-system/libkycpu.h>
#include <lingmosdk/lingmosdk-system/libkysysinfo.h>
namespace cpuInfo {

QString getUtilization();       // 获取总利用率
QString getCurrentFrequency();  // 获取当前频率
QString getFrequency();         // 获取频率
QString getUpTime();            // 获取运行时间
QString getSockets();           // 获取插槽
QString getLogicalProcessors(); // 获取逻辑处理器
QString getVirtualization();    // 获取虚拟化
QString getL1iCache();          // 获取 L1 缓存（指令）
QString getL1dCache();          // 获取 L1 缓存（数据）
QString getL2Cache();           // 获取 L2 缓存
QString getL3Cache();           // 获取 L3 缓存
QString getLoadAverage();       // 获取负载均衡
QString getFileDescriptors();   // 获取文件描述符数
QString getProcesses();         // 获取进程数
QString getThreads();           // 获取线程数
QString getHostName();          // 获取计算机名
QString getOSType();            // 获取类型
QString getVersion();           // 获取版本名称
}

namespace memInfo {
QString getUsed();              // 获取已使用
QString getAvailable();         // 获取可使用
QString getShared();            // 获取共享内存
QString getCached();            // 获取高速缓存
QString getBuffers();           // 获取数据缓存
QString getCachedSwap();        // 获取交换缓存区
QString getActive();            // 获取活跃的缓冲文件
QString getInActive();          // 获取不活跃的缓冲文件
QString getDirty();             // 获取脏页
QString getMapped();            // 获取映射大小
QString getTotalSwap();         // 获取交换空间大小
QString getFreeSwap();          // 获取可用交换空间
QString getSlab();              // 获取内核数据结构缓存
}

#include <lingmosdk/lingmosdk-system/libkync.h>
#include <lingmosdk/lingmosdk-system/libkynetinfo.h>

typedef struct Net4Addr
{
    QString addr;
    QString mask;
    QString bcast;
}INet4Addr;

typedef struct Net6Addr
{
    QString addr;
    QString prefixlen;
    QString scope;
}INet6Addr;

namespace netInfo {
QString getIPv4Address(const char *strKey);     // 获取 IPv4 地址
QString getIPv4Netmask(const char *strKey, const char *addr);     // 获取 IPv4 子网掩码
QString getIPv4Boardcast(const char *strKey, const char *addr);   // 获取 IPv4 广播
QString getIPv6Address(const char *strKey);     // 获取 IPv6 地址
QString getIPv6NetmaskLen(const char *strKey, const char *addr);  // 获取 IPv6 掩码长度
QString getIPv6Scope(const char *strKey, const char *addr);       // 获取 IPv6 域
QString getConnectType(const char *strKey);     // 获取连接类型
QString getNetName(const char *strKey);         // 获取网络名称
QString getSignalQuality(const char *strKey);   // 获取信号质量
QString getSignalStrength(const char *strKey);  // 获取信号强度
QString getNoiseLevel(const char *strKey);      // 获取底噪
QString getMacAddress(const char *strKey);      // 获取 MAC 地址
QString getSpeed(const char *strKey);           // 获取速率
QString getRecvPackets(const char *strKey);     // 获取接收包数量
QString getRecvTotal(const char *strKey);       // 获取总计接收
QString getRecvErrors(const char *strKey);      // 获取接收错误包
QString getRecvDropped(const char *strKey);     // 获取接收丢弃包
QString getRecvFIFO(const char *strKey);        // 获取接收 FIFO
QString getFrameErrors(const char *strKey);     // 获取分组帧错误
QString getSendPackets(const char *strKey);     // 获取发送包数量
QString getSendTotal(const char *strKey);       // 获取总计发送
QString getSendErrors(const char *strKey);      // 获取发送错误包
QString getSendDropped(const char *strKey);     // 获取发送丢弃宝
QString getSendFIFO(const char *strKey);        // 获取发送FIFO
QString getCarrierLoss(const char *strKey);     // 获取载波损耗

QStringList getUpCards();                       // 获取up状态网卡列表
QStringList getAllCards();                      // 获取所有网卡列表
QList<INet4Addr> getIPv4AddressList(const char *strKey); // 获取ipv4地址列表
QList<INet6Addr> getIPv6AddressList(const char *strKey); // 获取ipv6地址列表
}

namespace commom {

namespace format {
// 内存使用单位Byte
const char *const UnitSuffixMem[] = {"B","KB", "MB", "GB", "TB", "PB", "EB"};

//const char *const UnitSuffixNet[] = {"b", "KB", ""}

const QString Format_Memory(long data, KDKVolumeBaseType baseType);

}
}


#endif // COMMONINFO_H
