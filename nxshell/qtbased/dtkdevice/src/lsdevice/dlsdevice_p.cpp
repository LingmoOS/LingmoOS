// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dlsdevice.h"
#include "dlsdevice_p.h"

#include "scan.h"
#include "hw.h"

#include <QString>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QDebug>

#include "options.h"
#include "osutils.h"
#include "config.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>

#define PROC_CPU_STAT_PATH "/proc/stat"
#define PROC_CPU_INFO_PATH "/proc/cpuinfo"

using namespace hw;
DDEVICE_BEGIN_NAMESPACE

DlsDevicePrivate::DlsDevicePrivate(DlsDevice *parent)
    : m_hwNode("computer", hw::sys_tem)
    , q_ptr(parent)
{
    // m_classAttrisLstMap.clear();
    m_listDeviceInfo.clear();
    scan_system(m_hwNode);
    addDeviceInfo(m_hwNode, m_listDeviceInfo);

    // mLastCpuStat record the Cpu status
    // init  all  mLastCpuStat
    mLastCpuStat["user"] = 0;
    mLastCpuStat["nice"] = 0;
    mLastCpuStat["sys"] = 0;
    mLastCpuStat["idle"] = 0;
    mLastCpuStat["iowait"] = 0;
    mLastCpuStat["hardqirq"] = 0;
    mLastCpuStat["softirq"] = 0;
    mLastCpuStat["steal"] = 0;
    mLastCpuStat["guest"] = 0;
    mLastCpuStat["guest_nice"] = 0;
    // total is sum of above items
    mLastCpuStat["total"] = 0;

    updateSystemCpuUsage();
}

double DlsDevicePrivate::updateSystemCpuUsage()
{
    double cpuUsage = 0.0;

    QFile file(PROC_CPU_STAT_PATH);
    if (file.exists() && file.open(QFile::ReadOnly)) {

        QByteArray lineData = file.readLine();
        file.close();

        QStringList cpuStatus =  QString(lineData).split(" ", D_SPLIT_BEHAVIOR);

        if (cpuStatus.size() < 11) {
            return cpuUsage;
        }

        QMap<QString, int> curCpuStat;
        {
            curCpuStat["user"] = cpuStatus.at(1).toInt();
            curCpuStat["nice"] = cpuStatus.at(2).toInt();
            curCpuStat["sys"] = cpuStatus.at(3).toInt();
            curCpuStat["idle"] = cpuStatus.at(4).toInt();
            curCpuStat["iowait"] = cpuStatus.at(5).toInt();
            curCpuStat["hardqirq"] = cpuStatus.at(6).toInt();
            curCpuStat["softirq"] = cpuStatus.at(7).toInt();
            curCpuStat["steal"] = cpuStatus.at(8).toInt();
            curCpuStat["guest"] = cpuStatus.at(9).toInt();
            curCpuStat["guest_nice"] = cpuStatus.at(10).toInt();
        }

        int curTotalCpu = 0;
        for (int i = 1; i <= 10; i++) {
            curTotalCpu = curTotalCpu + cpuStatus.at(i).toInt();
        }
        curCpuStat["total"] = curTotalCpu;

        double calcCpuTotal = curCpuStat["total"] - mLastCpuStat["total"];
        double calcCpuIdle =
            (curCpuStat["idle"] + curCpuStat["iowait"]) - (mLastCpuStat["idle"] + mLastCpuStat["iowait"]);

        if (calcCpuTotal == 0.0) {
            qWarning() << " cpu total usage calc result equal 0 ! cpu stat [" << curCpuStat << "]";
            return cpuUsage;
        }

        cpuUsage = (calcCpuTotal - calcCpuIdle) * 100.0 / calcCpuTotal;


        mCpuUsage = cpuUsage;

        mLastCpuStat = curCpuStat;
    } else {
        qWarning() << QString(" file %1 open fail !").arg(PROC_CPU_STAT_PATH);
    }

    return cpuUsage;
}

QMap<QString, int> DlsDevicePrivate::CpuStat()
{
    return mLastCpuStat;
}

double DlsDevicePrivate::getCpuUsage()
{
    return mCpuUsage;
}

void DlsDevicePrivate::addDeviceInfo(hwNode &node, QList<DlsDevice::DDeviceInfo > &ll)
{
    struct DlsDevice::DDeviceInfo entry;

    entry.deviceInfoLstMap.clear();
    entry.devClass = DlsDevice::DtkUnkown;

    if (node.getBusInfo() != "") {
        entry.deviceBaseAttrisLst.append("SysFs_PATH");
        entry.deviceInfoLstMap["SysFs_PATH"] = QString::fromStdString(node.getBusInfo());
    }

    entry.devClass = convertClass(node.getClass());
    entry.subClassName = QString::fromStdString(node.getsubClassName());

    entry.deviceBaseAttrisLst.append("Description");
    entry.deviceInfoLstMap.insert("Description", QString::fromStdString(node.getDescription()));

    entry.deviceBaseAttrisLst.append("sysID");
    entry.deviceInfoLstMap.insert("sysID", QString::fromStdString(node.getId()));

    entry.deviceBaseAttrisLst.append("Vendor");
    entry.deviceInfoLstMap.insert("Vendor", QString::fromStdString(node.getVendor()));


    entry.deviceBaseAttrisLst.append("Name");
    entry.deviceInfoLstMap.insert("Name", QString::fromStdString(node.getProduct()));

    if (! node.getConfig("Vendor_ID").empty()) {
        entry.deviceBaseAttrisLst.append("Vendor_ID");
        entry.deviceInfoLstMap.insert("Vendor_ID", QString::fromStdString(node.getConfig("Vendor_ID")));
    }

    if (! node.getConfig("Product_ID").empty()) {
        entry.deviceBaseAttrisLst.append("Product_ID");
        entry.deviceInfoLstMap.insert("Product_ID", QString::fromStdString(node.getConfig("Product_ID")));
    }

    if (! node.getConfig("VID:PID").empty()) {
        entry.deviceBaseAttrisLst.append("VID:PID");
        entry.deviceInfoLstMap.insert("VID:PID", QString::fromStdString(node.getConfig("VID:PID")));
    }

    if (node.getModalias().length() > 53) {
        entry.deviceBaseAttrisLst.append("Modalias");
        entry.deviceInfoLstMap.insert("Modalias", QString::fromStdString(node.getModalias().substr(0, 53)));
    }

    else {
        entry.deviceBaseAttrisLst.append("Modalias");
        entry.deviceInfoLstMap.insert("Modalias", QString::fromStdString(node.getModalias()));
    }

    entry.deviceBaseAttrisLst.append("ClassName");
    entry.deviceInfoLstMap.insert("ClassName", QString::fromStdString(node.getClassName()));


    if (node.getLogicalName() != "") {
        vector<string> logicalnames = node.getLogicalNames();
        QString  tmps = "";

        for (unsigned int i = 0; i < logicalnames.size(); i++) {
            tmps.append(QString::fromStdString(logicalnames[i]));
            if (i >= logicalnames.size())
                break;
            tmps.append(" ");
        }
        entry.deviceBaseAttrisLst.append("LogicalName");
        entry.deviceInfoLstMap.insert("LogicalName", tmps);
    }

    vector < string > config;
    config = node.getConfigValues("=");

    if (config.size() > 0) {
        QString  tmps = "";
        for (unsigned int i = 0; i < config.size(); i++) {
            tmps.append(QString::fromStdString(config[i]));
            if (i >= config.size())
                break;
            tmps.append(" ");
        }
        entry.deviceBaseAttrisLst.append("Configuration");
        entry.deviceInfoLstMap.insert("Configuration", tmps);
    }

    vector < string > resources;
    resources = node.getResources(":");
    if (resources.size() > 0) {
        QString  tmps = "";
        for (unsigned int i = 0; i < resources.size(); i++) {
            tmps.append(QString::fromStdString(resources[i]));
            if (i >= resources.size())
                break;
            tmps.append(" ");
        }
        entry.deviceBaseAttrisLst.append("Resources");
        entry.deviceInfoLstMap.insert("Resources", tmps);
    }
    if (node.getSize() > 0) {
        string tmpstr = "";

        switch (node.getClass()) {
        case hw::disk:
            tmpstr = kilobytes(node.getSize()) + " (" + decimalkilos(node.getSize()) + "B)";
            break;
        case hw::display:
        case hw::memory:
        case hw::address:
        case hw::storage:
        case hw::volume:
            tmpstr =  kilobytes(node.getSize());
            break;
        case hw::processor:
        case hw::bus:
        case hw::sys_tem:
            tmpstr =  decimalkilos(node.getSize()) + "Hz";
            break;
        case hw::network:
            tmpstr =  decimalkilos(node.getSize()) + "bit/s";
            break;
        case hw::power:
            tmpstr =  node.getSize() + "mWh";
            break;
        default:
            tmpstr =   node.getSize();
        }
        entry.deviceBaseAttrisLst.append("Size");
        entry.deviceInfoLstMap.insert("Size", QString::fromStdString(tmpstr));
    }
//--------------------------------ADD Children---------------------
    ll.append(entry);
    for (int i = 0; i < node.countChildren(); i++) {
        addDeviceInfo(*node.getChild(i), ll);
    }
}

DlsDevice::DevClass DlsDevicePrivate::convertClass(hw::hwClass cc)
{
    switch (cc) {
    case bridge:  return DlsDevice::DtkBridge;
    case sys_tem:  return DlsDevice::DtkProductSystem;
    case memory:  return DlsDevice::DtkMemory;
    case processor: return DlsDevice::DtkCpu;
    case address:   return   DlsDevice::DtkAddress;
    case storage:   return   DlsDevice::DtkStorage;
    case disk:      return   DlsDevice::DtkDisk;
    case tape:      return   DlsDevice::DtkTape;
    case bus:       return   DlsDevice::DtkBus;
    case network:   return   DlsDevice::DtkNetwork;
    case display:   return   DlsDevice::DtkDisplayGPU;
    case input:     return   DlsDevice::DtkInput;
    case printer:   return   DlsDevice::DtkPrinter;
    case multimedia: return   DlsDevice::DtkSoundAudio;
    case communication: return DlsDevice::DtkCommunication;
    case power:       return  DlsDevice::DtkPower;
    case volume:      return  DlsDevice::DtkVolume;
    case generic:     return  DlsDevice::DtkGeneric;
    default:          return  DlsDevice::DtkUnkown;
    }
}

hwClass DlsDevicePrivate::convertClass(DlsDevice::DevClass cc)
{
    switch (cc) {
    case DlsDevice::DtkBridge:  return bridge;
    case DlsDevice::DtkProductSystem:  return sys_tem;
    case DlsDevice::DtkMemory:  return memory;
    case DlsDevice::DtkCpu: return processor;
    case DlsDevice::DtkAddress:   return   address;
    case DlsDevice::DtkStorage:   return   storage;
    case DlsDevice::DtkDisk:      return   disk;
    case DlsDevice::DtkTape:      return   tape;
    case DlsDevice::DtkBus:       return   bus;
    case DlsDevice::DtkNetwork:   return   network;
    case DlsDevice::DtkDisplayGPU:   return   display;
    case DlsDevice::DtkInput:     return   input;
    case DlsDevice::DtkPrinter:   return   printer;
    case DlsDevice::DtkSoundAudio: return   multimedia;
    case DlsDevice::DtkCommunication: return communication;
    case DlsDevice::DtkPower:       return  power;
    case DlsDevice::DtkVolume:      return  volume;
    case DlsDevice::DtkGeneric:     return  generic;
    default:          return  generic;
    }
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceCPU()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if ((m_listDeviceInfo.at(it).devClass == DlsDevice::DtkCpu))
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceStorage()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if ((m_listDeviceInfo.at(it).devClass == DlsDevice::DtkDisk) \
                || (m_listDeviceInfo.at(it).devClass == DlsDevice::DtkStorage))
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceGPU()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if ((m_listDeviceInfo.at(it).devClass == DlsDevice::DtkDisplayGPU))
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceMemory()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if ((m_listDeviceInfo.at(it).devClass == DlsDevice::DtkMemory))
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceMonitor()
{
    return QList<DlsDevice::DDeviceInfo>();
}


QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceAudio()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if ((m_listDeviceInfo.at(it).devClass == DlsDevice::DtkSoundAudio))
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceNetwork()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if ((m_listDeviceInfo.at(it).devClass == DlsDevice::DtkNetwork))
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceCamera()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        //if (m_ListDeviceInfo.at(it).eDevClass == DtkInput)
        if (m_listDeviceInfo.at(it).subClassName == "camera")
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceKeyboard()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if (m_listDeviceInfo.at(it).devClass == DlsDevice::DtkInput)
            if (m_listDeviceInfo.at(it).subClassName == "keyboard")
                infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceMouse()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if (m_listDeviceInfo.at(it).devClass == DlsDevice::DtkInput)
            if (m_listDeviceInfo.at(it).subClassName == "mouse")
                infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

QList<DlsDevice::DDeviceInfo> DlsDevicePrivate::deviceComputer()
{
    QList<DlsDevice::DDeviceInfo >  infoList;
    for (int it = 0; it < m_listDeviceInfo.count(); it++) {
        if ((m_listDeviceInfo.at(it).devClass == DlsDevice::DtkProductSystem))
            infoList.append(m_listDeviceInfo.at(it));
    }
    return infoList;
}

DDEVICE_END_NAMESPACE
