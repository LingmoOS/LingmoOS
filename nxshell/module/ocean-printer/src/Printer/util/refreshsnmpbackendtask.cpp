// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "refreshsnmpbackendtask.h"
#include "dprintermanager.h"
#include "dprinter.h"
#include "common.h"

#include <QDebug>

RefreshSnmpBackendTask::RefreshSnmpBackendTask(QObject *parent) : QThread(parent)
{
    m_bExit = true;
    initColorTable();
}

RefreshSnmpBackendTask::~RefreshSnmpBackendTask()
{
    if (isTaskRunning()) {
        stopTask();
        wait();
    }
}

void RefreshSnmpBackendTask::setPrinters(const QStringList &strPrinters)
{
    m_strPrinterNames = strPrinters;
}

void RefreshSnmpBackendTask::beginTask()
{
    if (!m_bExit) {
        return;
    }

    m_vecFreshNode.clear();
    m_vecFreshNode.reserve(m_strPrinterNames.size());
    DPrinterManager *pManager = DPrinterManager::getInstance();

    for (int i = 0; i < m_strPrinterNames.size(); i++) {
        DDestination *pDest = pManager->getDestinationByName(m_strPrinterNames.at(i));

        if (pDest != nullptr) {
            if (DESTTYPE::PRINTER == pDest->getType()) {
                DPrinter *pPrinter = static_cast<DPrinter *>(pDest);

                if (pPrinter != nullptr) {
                    SNMPFRESHNODE node;
                    QString strInfo;
                    pPrinter->getPrinterInfo(strInfo, node.strLoc, node.strUrl, node.strPpdName);
                    node.strName = pPrinter->getName();
                    m_vecFreshNode.push_back(node);
                }
            }
        }
    }

    start();
}

void RefreshSnmpBackendTask::stopTask()
{
    m_bExit = true;
    terminate();
}

bool RefreshSnmpBackendTask::isTaskRunning()
{
    return !m_bExit;
}

QVector<SUPPLYSDATA> RefreshSnmpBackendTask::getSupplyData(const QString &strName)
{
    QVector<SUPPLYSDATA> retData;

    if (m_mapSupplyInfo.contains(strName)) {
        retData = m_mapSupplyInfo[strName];
    }

    return retData;
}

void RefreshSnmpBackendTask::run()
{
    m_bExit = false;

    for (int i = 0; i < m_vecFreshNode.size(); i++) {

        if (m_bExit)
            break;

        bool bRet = canGetSupplyMsg(m_vecFreshNode[i]);
        QString strName = m_vecFreshNode[i].strName;
        emit refreshsnmpfinished(strName, bRet);
    }

    m_bExit = true;
}

bool RefreshSnmpBackendTask::canGetSupplyMsg(const SNMPFRESHNODE &node)
{
    QVector<SUPPLYSDATA> vecMarkInfo;
    QString strIp = node.strLoc;
    if (!isIpv4Address(strIp)) {
        strIp = getHostFromUri(node.strUrl);
    }

    if (node.strUrl.startsWith("socket://")) {
        cupssnmp snmp;
        snmp.setIP(strIp.toStdString());
        snmp.setPPDName(node.strPpdName.toStdString().c_str());
        snmp.SNMPReadSupplies();
        vector<SUPPLYSDATA> tempVec = snmp.getMarkInfo();

        for (unsigned int i = 0; i < tempVec.size(); i++) {
            vecMarkInfo.push_back(tempVec[i]);
        }
    } else if (node.strUrl.startsWith("ipp://")) {
        try {
            Connection c;
            QStringList strLevelList;
            QStringList strNameList;
            QStringList strHighLevels;
            QStringList strTypes;
            QStringList strColors;
            c.init(strIp.toStdString().c_str(), ippPort(), 0);
            vector<string> requestAttrs;
            requestAttrs.push_back("marker-names");
            requestAttrs.push_back("marker-levels");
            requestAttrs.push_back("marker-high-levels");
            requestAttrs.push_back("marker-types");
            requestAttrs.push_back("marker-colors");
            /*获取属性的接口，如果传入名称，会默认构造localhost的uri，导致多次共享的ipp无法获取正确的属性，这里改为传入uri*/
            map<string, string> attrs = c.getPrinterAttributes(nullptr,
                                                               node.strUrl.toLocal8Bit().data(), &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                if (iter->first == "marker-names") {
                    QString strVal = QString::fromStdString(iter->second);
                    strNameList = strVal.split('\0');
                } else if (iter->first == "marker-levels") {
                    QString strVal = QString::fromStdString(iter->second);
                    strLevelList = strVal.split('\0');
                } else if (iter->first == "marker-high-levels") {
                    QString strVal = QString::fromStdString(iter->second);
                    strHighLevels = strVal.split('\0');
                } else if (iter->first == "marker-types") {
                    QString strVal = QString::fromStdString(iter->second);
                    strTypes = strVal.split('\0');
                } else if (iter->first == "marker-colors") {
                    QString strVal = QString::fromStdString(iter->second);
                    strColors = strVal.split('\0');
                }
            }

            for (int i = 0; i < strNameList.size(); i++) {
                QString strName = strNameList.at(i).trimmed();
                QString strLevel = strLevelList.at(i).trimmed();

                if (strName.isEmpty() && strLevel.isEmpty()) {
                    continue;
                }

                SUPPLYSDATA info;
                strName = strName.remove(0, 2);
                strLevel = strLevel.remove(0, 2);
                strcpy(info.name, strName.toStdString().c_str());
                info.level = strLevel.toInt();

                if (strHighLevels.isEmpty()) {
                    info.max_capacity = 100;
                } else {
                    QString strMaxCapacity = strHighLevels.at(i).trimmed();

                    if (strMaxCapacity.isEmpty()) {
                        info.max_capacity = 100;
                    } else {
                        strMaxCapacity = strMaxCapacity.remove(0, 2);
                        info.max_capacity = strMaxCapacity.toInt();
                    }
                }

                QString strType = strTypes.at(i).trimmed();
                strType = strType.remove(0, 2);
                /*only support toner,ink not support*/
                bool valid = false;
                if (strType == "toner") {
                    info.type = 3;
                    QString strColor = strColors.at(i).trimmed();
                    strColor = strColor.remove(0, 2);
                    QString strColorName = getColorName(strColor);
                    strncpy(info.color, strColor.toStdString().c_str(), sizeof(info.color) - 1);
                    info.color[sizeof(info.color) - 1] = '\0';
                    strncpy(info.colorName, strColorName.toStdString().c_str(), sizeof(info.colorName) -1);
                    info.colorName[sizeof(info.colorName) - 1] = '\0';
                    valid = true;
                } else if (strType == "waste-toner") {
                    info.type = 4;
                    valid = true;
                }
                /*过滤其他无法识别的耗材信息*/
                if (valid)
                    vecMarkInfo.push_back(info);
            }
        } catch (const std::runtime_error &e) {
            qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(e.what());
        }
    }

    m_mapSupplyInfo.insert(node.strName, vecMarkInfo);
    return (vecMarkInfo.size() > 0);
}

void RefreshSnmpBackendTask::initColorTable()
{
    m_colorTable.insert("#000000", "Black");
    m_colorTable.insert("#0000FF", "Blue");
    m_colorTable.insert("#A52A2A", "Brown");
    m_colorTable.insert("#00FFFF", "Cyan");
    m_colorTable.insert("#404040", "Dark-gray");
    m_colorTable.insert("#404040", "Dark gray");
    m_colorTable.insert("#FFCC00", "Dark-yellow");
    m_colorTable.insert("#FFCC00", "Dark yellow");
    m_colorTable.insert("#FFD700", "Gold");
    m_colorTable.insert("#808080", "Gray");
    m_colorTable.insert("#00FF00", "Green");
    m_colorTable.insert("#606060", "Light-black");
    m_colorTable.insert("#606060", "Light black");
    m_colorTable.insert("#E0FFFF", "Light-cyan");
    m_colorTable.insert("#E0FFFF", "Light cyan");
    m_colorTable.insert("#D3D3D3", "Light-gray");
    m_colorTable.insert("#D3D3D3", "Light gray");
    m_colorTable.insert("#FF77FF", "Light-magenta");
    m_colorTable.insert("#FF77FF", "Light magenta");
    m_colorTable.insert("#FF00FF", "Magenta");
    m_colorTable.insert("#FFA500", "Orange");
    m_colorTable.insert("#FF0000", "Red");
    m_colorTable.insert("#C0C0C0", "Silver");
    m_colorTable.insert("#FFFFFF", "White");
    m_colorTable.insert("#FFFF00", "Yellow");
}

QString RefreshSnmpBackendTask::getColorName(const QString &strColor)
{
    QString strTmp = strColor.toUpper();
    return m_colorTable.value(strTmp);
}
