// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_SUMMARY_VIEW_WIDGET_H
#define CPU_SUMMARY_VIEW_WIDGET_H

#include <DTableView>
#include <QHeaderView>
#include <DApplicationHelper>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <DApplication>
#include <DStyle>
#include <QScroller>

#include "common/common.h"
#include "model/cpu_info_model.h"
#include "system/cpu_set.h"
#include "base/base_detail_item_delegate.h"



using namespace core::system;
using namespace common::format;
/**
 * @brief Memory summary view widget
 */
DWIDGET_USE_NAMESPACE
class CPUInfoModel;
class CPUSummaryTableModel;
class CPUDetailSummaryTable : public DTableView
{
    Q_OBJECT
public:
    explicit CPUDetailSummaryTable(CPUInfoModel *dataModel, QWidget *parent = nullptr);

public slots:
    void fontChanged(const QFont &font);

    void onModelUpdate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QFont m_font;
    CPUSummaryTableModel *m_model;
};

class CPUSummaryTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CPUSummaryTableModel(CPUInfoModel *m_model, QObject *parent = nullptr);

    virtual ~CPUSummaryTableModel();

private:
    CPUInfoModel *m_model;

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
                    return QApplication::translate("CPUSummaryTableModel", "Utilization"); //百分比显示，为CPU的总体利用率，显示精度为1%，2秒刷新1次；
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "Current frequency");//显示当前CPU的实际运行速度，单位说明：如果当前CPU速度大于1GHz，单位为GHz；如果当前CPU速度小于1GHz，显示单位为MHz；
                break;

            case 1:
                if (column == 0)    //m_model->cpuSet()->modelName()); //CPU属于的名字及其编号、标称主频；
                    return QApplication::translate("CPUSummaryTableModel", "Frequency");//最小频率  ~ 最大频率；
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "Up time");//最近一次开机到目前的运行时间。格式 天（DDDD）：小时（HH）：分钟（MM），60分自动进位到1小时；24小时自动进位为1天；最大支持 9999天；
                break;
            case 2:
                if (column == 0)    //m_model->cpuSet()->coreId(0));   //处理器ID
                    return QApplication::translate("CPUSummaryTableModel", "Sockets"); //插槽
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "Logical processors"); //逻辑处理器数量；（格式：数字）
                break;
            case 3:
                if (column == 0)
                    return QApplication::translate("CPUSummaryTableModel", "Virtualization"); //虚拟化机制；（Intel 的VT-x，AMD的AMD-v），如果当前CPU不支持虚拟化，显示“不支持”；
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "L1i cache");//L1缓存（指令）：1级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                break;
            case 4:
                if (column == 0)
                    return QApplication::translate("CPUSummaryTableModel", "L1d cache");//L1缓存（数据）：1级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "L2 cache");//L2缓存：2级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                break;
            case 5:
                if (column == 0)
                    return QApplication::translate("CPUSummaryTableModel", "L3 cache");//L3缓存：3级缓存大小；（单位：小于1K，用B；小于1M，用KB；大于1M：用MB；）
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "Load average"); //负载均衡：Load Average 就是一段时间 (1 分钟、5分钟、15分钟) 内平均 Load；
                break;
            case 6:
                if (column == 0)
                    return QApplication::translate("CPUSummaryTableModel", "File descriptors");//文件描述符数
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "Processes"); //进程数量（格式：数字）
                break;
            case 7:
                if (column == 0)
                    return QApplication::translate("CPUSummaryTableModel", "Threads");//线程数量（格式：数字）
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "Host name");//显示主机名称。（格式：字符串）
                break;
            case 8:
                if (column == 0)
                    return QApplication::translate("CPUSummaryTableModel", "OS type");//系统类型
                else if (column == 1)
                    return QApplication::translate("CPUSummaryTableModel", "Version"); //版本号
                break;
            default:
                break;
            }
        } else if (role == Qt::UserRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return  QString::number(m_model->cpuAllPercent(), 'f', 0) + "%";
                else if (column == 1)
                    return m_model->cpuSet()->curFreq();
                break;
            case 1:
                if (column == 0)
                    return m_model->cpuSet()->minFreq() + " ~ " + m_model->cpuSet()->maxFreq();
                else if (column == 1)
                    return m_model->uptime();
                break;
            case 2:
                if (column == 0)
                    return m_model->cpuSet()->socketCount() == 0 ? "-" : QString::number(m_model->cpuSet()->socketCount());
                else if (column == 1)
                    return QString::number(m_model->cpuSet()->cpuCount());
                break;
            case 3:
                if (column == 0)
                    return m_model->cpuSet()->virtualization();
                else if (column == 1)
                    return m_model->cpuSet()->l1iCache();
                break;
            case 4:
                if (column == 0)
                    return m_model->cpuSet()->l1dCache();
                else if (column == 1)
                    return m_model->cpuSet()->l2Cache();
                break;
            case 5:
                if (column == 0)
                    return m_model->cpuSet()->l3Cache();
                else if (column == 1)
                    return m_model->loadavg();
                break;
            case 6:
                if (column == 0)
                    return QString::number(m_model->nFileDescriptors());
                else if (column == 1)
                    return  QString::number(m_model->nProcesses());
                break;
            case 7:
                if (column == 0)
                    return QString::number(m_model->nThreads());
                else if (column == 1)
                    return  m_model->hostname();
                break;
            case 8:
                if (column == 0)
                    return m_model->osType();
                else if (column == 1)
                    return m_model->osVersion();
                break;
            default:
                break;
            }
        } else if (role == Qt::TextColorRole) {
            const auto &palette = DApplicationHelper::instance()->applicationPalette();
            return palette.color(DPalette::Text);
        }

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::NoItemFlags;
    }

private slots:
    void onModelUpdated();

};

#endif // CPU_SUMMARY_VIEW_WIDGET_H
