#ifndef CPUDETAILSMODEL_H
#define CPUDETAILSMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QPalette>

#include "commoninfo.h"

class CpuDetailsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CpuDetailsModel(QObject *parent = nullptr);
    ~CpuDetailsModel();

protected:
    int rowCount(const QModelIndex &) const
    {
        return 9;
    }

    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        int row = index.row();
        int column = index.column();

        if (role == Qt::DisplayRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return tr("Utilization"); //百分比显示，为CPU的总体利用率，显示精度为1%，2秒刷新1次；
                else if (column == 1)
                    return tr("Current frequency");//显示当前CPU的实际运行速度，单位说明：如果当前CPU速度大于1GHz，单位为GHz；如果当前CPU速度小于1GHz，显示单位为MHz；
                break;

            case 1:
                if (column == 0)    //m_cpuInfo->cpuSet()->modelName()); //CPU属于的名字及其编号、标称主频；
                    return tr("Frequency");//最小频率  ~ 最大频率；
                else if (column == 1)
                    return tr("Up time");//最近一次开机到目前的运行时间。格式 天（DDDD）：小时（HH）：分钟（MM），60分自动进位到1小时；24小时自动进位为1天；最大支持 9999天；
                break;
            case 2:
                if (column == 0)    //m_cpuInfo->cpuSet()->coreId(0));   //处理器ID
                    return tr("Sockets"); //插槽
                else if (column == 1)
                    return tr("Logical processors"); //逻辑处理器数量；（格式：数字）
                break;
            case 3:
                if (column == 0)
                    return tr("Virtualization"); //虚拟化机制；（Intel 的VT-x，AMD的AMD-v），如果当前CPU不支持虚拟化，显示“不支持”；
                else if (column == 1)
                    return tr("L1i cache");//L1缓存（指令）：1级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                break;
            case 4:
                if (column == 0)
                    return tr("L1d cache");//L1缓存（数据）：1级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                else if (column == 1)
                    return tr("L2 cache");//L2缓存：2级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                break;
            case 5:
                if (column == 0)
                    return tr("L3 cache");//L3缓存：3级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                else if (column == 1)
                    return tr("Load average"); //负载均衡：Load Average 就是一段时间 (1 分钟、5分钟、15分钟) 内平均 Load；
                break;
            case 6:
                if (column == 0)
                    return tr("File descriptors");//文件描述符数
                else if (column == 1)
                    return tr("Processes"); //进程数量（格式：数字）
                break;
            case 7:
                if (column == 0)
                    return tr("Threads");//线程数量（格式：数字）
                else if (column == 1)
                    return tr("Host name");//显示主机名称。（格式：字符串）
                break;
            case 8:
                if (column == 0)
                    return tr("OS type");//系统类型
                else if (column == 1)
                    return tr("Version"); //版本号
                break;
            default:
                break;
            }
        } else if (role == Qt::UserRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return cpuInfo::getUtilization();
                else if (column == 1);
                    return cpuInfo::getCurrentFrequency();
                break;
            case 1:
                if (column == 0)
                    return cpuInfo::getFrequency();
                else if (column == 1)
                    return cpuInfo::getUpTime();
                break;
            case 2:
                if (column == 0)
                    return cpuInfo::getSockets();
                else if (column == 1)
                    return cpuInfo::getLogicalProcessors();
                break;
            case 3:
                if (column == 0)
                    return cpuInfo::getVirtualization();
                else if (column == 1)
                    return cpuInfo::getL1iCache();
                break;
            case 4:
                if (column == 0)
                    return cpuInfo::getL1dCache();
                else if (column == 1)
                    return cpuInfo::getL2Cache();
                break;
            case 5:
                if (column == 0)
                    return cpuInfo::getL3Cache();
                else if (column == 1)
                    return cpuInfo::getLoadAverage();
                break;
            case 6:
                if (column == 0)
                    return cpuInfo::getFileDescriptors();
                else if (column == 1)
                    return  cpuInfo::getProcesses();
                break;
            case 7:
                if (column == 0)
                    return cpuInfo::getThreads();
                else if (column == 1)
                    return  cpuInfo::getHostName();
                break;
            case 8:
                if (column == 0)
                    return cpuInfo::getOSType();
                else if (column == 1)
                    return cpuInfo::getVersion();
                break;
            default:
                break;
            }
        } else if (role == Qt::TextColorRole) {
            const auto &palette = QPalette();
            return palette.color(QPalette::Text);
        }

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::NoItemFlags;
    }

public slots:
    void onModelUpdated();
};

#endif // CPUDETAILSMODEL_H
