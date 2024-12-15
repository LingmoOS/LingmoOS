// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dprintermanager.h"
#include "dprinter.h"
#include "dprintclass.h"
#include "cupsattrnames.h"
#include "cupsconnectionfactory.h"
#include "zsettings.h"

#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QRegularExpression>

#include <assert.h>

DPrinterManager *DPrinterManager::m_self = nullptr;

DPrinterManager::DPrinterManager()
{
}

DPrinterManager::~DPrinterManager()
{
    clearDestinationList();
}

DPrinterManager *DPrinterManager::getInstance()
{
    if (nullptr == m_self) {
        m_self = new DPrinterManager;
    }

    return m_self;
}


void DPrinterManager::setAllowedUsers(const QString &strPrinterName, const QVector<QString> strUsers)
{
    vector<string> vecTrans;

    for (int i = 0; i < strUsers.size(); i++) {
        vecTrans.push_back(strUsers[i].toStdString());
    }

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->setPrinterUsersAllowed(strPrinterName.toStdString().c_str(), &vecTrans);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

void DPrinterManager::updateDestinationList()
{
    clearDestinationList();

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return;
        map<string, map<string, string>> mapPrinters = conPtr->getPrinters();

        for (auto iter = mapPrinters.begin(); iter != mapPrinters.end(); iter++) {
            QString strName = QString::fromStdString(iter->first);
            if (m_mapDests.contains(strName))
                continue;
            map<string, string> mapProperty = iter->second;
            string strValue = mapProperty["device-uri"];

            if (strValue.empty()) {
                continue;
            }

            strValue.erase(0, 1);

            DDestination *pDest = nullptr;

            if (0 == strValue.find("file:///dev/null")) {
                pDest = new DPrintClass();
            } else {
                pDest = new DPrinter();
            }

            if (pDest) {
                pDest->setName(strName);
                m_mapDests.insert(strName, pDest);
            } else {
                qCWarning(UTIL) << "not enough memory";
            }

        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QStringList DPrinterManager::getPrintersList()
{
    QStringList printerList;

    for (auto iter = m_mapDests.begin(); iter != m_mapDests.end(); iter++) {
        if (iter.value()->getType() == PRINTER)
            printerList.push_back(iter.key());
    }

    return printerList;
}

QStringList DPrinterManager::getPrinterBaseInfoByName(QString printerName)
{
    QStringList baseInfo;
    DDestination *pDest = m_mapDests[printerName];

    if (pDest != nullptr) {
        baseInfo = pDest->getPrinterBaseInfo();
    }

    return baseInfo;
}

bool DPrinterManager::deletePrinterByName(QString PrinterName)
{
    bool bRet = false;
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            conPtr->deletePrinter(PrinterName.toStdString().data(), "");
            bRet = true;
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    return bRet;
}

void DPrinterManager::setPrinterShared(QString printerName, int shared)
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->setPrinterShared(printerName.toStdString().data(), shared);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
    }
}

bool DPrinterManager::isPrinterShared(QString printerName)
{
    bool isShared = false;
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_ISSHAED);
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrMap = conPtr->getPrinterAttributes(printerName.toStdString().data(), nullptr, &printerAttrs);
            std::string strShared = attrMap.at(CUPS_OP_ISSHAED);
            if (strShared.substr(0, 2) == "b1") {
                isShared = true;
            } else {
                isShared = false;
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
    }
    return isShared;
}

void DPrinterManager::setPrinterEnabled(QString printerName, bool enabled)
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (enabled && conPtr) {
            conPtr->enablePrinter(printerName.toStdString().data(), "");
        } else if (conPtr) {
            conPtr->disablePrinter(printerName.toStdString().data(), "");
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
    }
}

bool DPrinterManager::isPrinterEnabled(QString printerName)
{
    bool isEnable = false;
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_STATE);
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrMap = conPtr->getPrinterAttributes(printerName.toStdString().data(), nullptr, &printerAttrs);
            if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i3")) {
                isEnable = true;
            } else if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i4")) {
                isEnable = true;
            } else {
                isEnable = false;
            }
        }
    } catch (const std::runtime_error &e) {
        qCCritical(UTIL) << e.what();
    }
    return isEnable;
}

void DPrinterManager::setPrinterAcceptJob(QString printerName, bool enabled)
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (enabled && conPtr) {
            conPtr->acceptJobs(printerName.toStdString().data(), "");
        } else if (conPtr) {
            conPtr->rejectJobs(printerName.toStdString().data(), "");
        }
    } catch (const std::runtime_error &e) {
        qCCritical(UTIL) << e.what();
    }
}

bool DPrinterManager::isPrinterAcceptJob(QString printerName)
{
    bool isAcceptJobs = false;
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_TYPE);
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrMap = conPtr->getPrinterAttributes(printerName.toStdString().data(), nullptr, &printerAttrs);
            int typeValue = QString(attrMap.at(CUPS_OP_TYPE).substr(1).data()).toInt();
            int result = typeValue & CUPS_PRINTER_REJECTING;
            if (result == CUPS_PRINTER_REJECTING) {
                isAcceptJobs = false;
            } else {
                isAcceptJobs = true;
            }
        }
    } catch (const std::runtime_error &e) {
        qCCritical(UTIL) << e.what();
    }
    return isAcceptJobs;
}

void DPrinterManager::setPrinterDefault(QString printerName)
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->setDefault(printerName.toStdString().data(), "");
    } catch (const std::runtime_error &e) {
        qCCritical(UTIL) << e.what();
    }
}

bool DPrinterManager::addPrinter(const QString &printer, const QString &info, const QString &location, const QString &device, const QString &ppdfile)
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->addPrinter(printer.toStdString().data(), info.toStdString().data(),
                               location.toStdString().data(), device.toStdString().data(), ppdfile.toStdString().data(), nullptr, nullptr);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
        return false;
    }
    return true;
}

bool DPrinterManager::isDefaultPrinter(QString PrinterName)
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return false;
        string defaultPrinter = conPtr->getDefault();
        if (PrinterName.toStdString().compare(defaultPrinter) == 0) {
            return true;
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
    }
    return false;
}

DDestination *DPrinterManager::getDestinationByName(const QString &strName)
{
    if (m_mapDests.contains(strName))
        return m_mapDests[strName];
    return nullptr;
}

bool DPrinterManager::hasUnfinishedJob()
{
    try {
        vector<string> jobAttrs {"job-id", "job-printer-uri", "job-name"};
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return false;
        map<int, map<string, string>> unfinishedJobs = conPtr->getJobs(nullptr, 1, 1, 0, &jobAttrs);
        if (unfinishedJobs.size() == 0)
            return false;
        return true;
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
        return false;
    }
}

bool DPrinterManager::hasUnfinishedJob(const QString &printer)
{
    //先获取所有的未完成任务，查找指定打印机任务
    try {
        vector<string> jobAttrs {"job-id", "job-printer-uri", "job-name"};
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return false;
        map<int, map<string, string>> unfinishedJobs = conPtr->getJobs(nullptr, 0, 0, 0, &jobAttrs);
        if (unfinishedJobs.size() == 0)
            return false;
        map<int, map<string, string>>::iterator iter;
        for (iter = unfinishedJobs.begin(); iter != unfinishedJobs.end(); iter++) {
            map<string, string> attr = iter->second;
            QString uri = QString::fromStdString(attr.at("job-printer-uri").data()).simplified();
            QString curPrinter = uri.replace("sipp://localhost/printers/", "");
            if (curPrinter == printer) {
                return true;
            }
        }
        return false;
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
        return false;
    }
}

bool DPrinterManager::hasFinishedJob()
{
    try {
        vector<string> jobAttrs {"job-id", "job-printer-uri", "job-state"};
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return false;
        map<int, map<string, string>> finishedJobs = conPtr->getJobs("completed", 1, 1, 0, &jobAttrs);
        if (finishedJobs.size() == 0)
            return false;
        return true;
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
        return false;
    }
}

void DPrinterManager::enableDebugLogging(bool enabled)
{
    m_pServerSettings.enableDebugLogging(enabled);
}

void DPrinterManager::enableRemoteAdmin(bool enabled)
{
    m_pServerSettings.enableRemoteAdmin(enabled);
}

void DPrinterManager::enableRemoteAny(bool enabled)
{
    m_pServerSettings.enableRemoteAny(enabled);
}

void DPrinterManager::enableSharePrinters(bool enabled)
{
    m_pServerSettings.enableSharePrinters(enabled);
}

void DPrinterManager::enableUserCancelAny(bool enabled)
{
    m_pServerSettings.enableUserCancelAny(enabled);
}

bool DPrinterManager::isDebugLoggingEnabled() const
{
    return m_pServerSettings.isDebugLoggingEnabled();
}

bool DPrinterManager::isRemoteAdminEnabled() const
{
    return m_pServerSettings.isRemoteAdminEnabled();
}

bool DPrinterManager::isRemoteAnyEnabled() const
{
    return m_pServerSettings.isRemoteAnyEnabled();
}

bool DPrinterManager::isSharePrintersEnabled() const
{
    return m_pServerSettings.isSharePrintersEnabled();
}

bool DPrinterManager::isUserCancelAnyEnabled() const
{
    return m_pServerSettings.isUserCancelAnyEnabled();
}

bool DPrinterManager::updateServerSetting()
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            m_pServerSettings.updateSettings(conPtr->adminGetServerSettings());
            return true;
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
    }
    return false;

}

void DPrinterManager::commit(const map<string, string> &options)
{
    try {
        m_pServerSettings.commit(g_Settings->getCupsServerHost().toLocal8Bit(), g_Settings->getCupsServerPort(), g_Settings->getCupsServerEncryption(), options);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << e.what();
    }
}

bool DPrinterManager::hasSamePrinter(const QString &printer)
{
    foreach (const QString &str, m_mapDests.keys()) {
        if (str == printer) {
            return true;
        }
    }
    return false;
}

QString DPrinterManager::validataName(const QString &oldPrinterName)
{
    QString newPrinterName = oldPrinterName.trimmed();
    if (newPrinterName.isEmpty())
        return QString();
    newPrinterName = (newPrinterName.length() > 40) ? newPrinterName.left(40) : newPrinterName;
    newPrinterName.replace(QRegularExpression("[/ ?'#\"\\\\<]"), "-");
    return newPrinterName;
}

void DPrinterManager::clearDestinationList()
{
    if (m_mapDests.size() > 0) {
        for (auto iter = m_mapDests.begin(); iter != m_mapDests.end(); iter++) {
            delete iter.value();
        }
        m_mapDests.clear();
    }
}

void DPrinterManager::initLanguageTrans()
{
    m_translator.init();
}

QString DPrinterManager::translateLocal(const QString &strContext, const QString &strKey, const QString &strDefault)
{
    QString strValue = m_translator.translateLocal(strContext, strKey, strDefault);
    return strValue;
}
