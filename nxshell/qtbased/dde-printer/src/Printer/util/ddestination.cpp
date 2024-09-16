// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddestination.h"
#include "cupsattrnames.h"
#include "cupsconnectionfactory.h"

#include <QDebug>

#include <map>
#include <assert.h>

using namespace std;

Q_LOGGING_CATEGORY(UTIL, "org.deepin.dde-printer.util")

DDestination::DDestination()
{

}

QString DDestination::getName()
{
    return m_strName;
}

void DDestination::setName(const QString &strName)
{
    m_strName = strName;
    initPrinterAttr();
}

bool DDestination::isShared()
{
    return m_isShared;
}

void DDestination::setShared(bool shared)
{
    m_isShared = shared;
}

bool DDestination::isEnabled()
{
    return m_isEnabled;
}

void DDestination::setEnabled(bool enabled)
{
    m_isEnabled = enabled;
    //设置启用和禁用之后会影响当前的打印机状态字符串信息，需要更新
    initPrinterAttr();
}

QStringList DDestination::getPrinterBaseInfo()
{
    initPrinterAttr();
    QStringList baseInfo;
    baseInfo.append(m_printerLocation);
    baseInfo.append(m_printerModel.isEmpty() ? m_printerInfo : m_printerModel);
    baseInfo.append(m_printerStatus);
    return baseInfo;
}

QString DDestination::printerInfo()
{
    QString strPrintInfo;
    vector<string> requestAttr;
    requestAttr.push_back(CUPS_OP_INFO);

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attr = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(), nullptr, &requestAttr);
            strPrintInfo = QString::fromStdString(attr[CUPS_OP_INFO].data());
            strPrintInfo = strPrintInfo.remove(0, 1);
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return strPrintInfo;
}

void DDestination::setPrinterInfo(const QString &info)
{
    m_printerInfo = info;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->setPrinterInfo(m_strName.toStdString().c_str(), m_printerInfo.toStdString().c_str());
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QString DDestination::printerLocation()
{
    QString strLocation;
    vector<string> requestAttr;
    requestAttr.push_back(CUPS_OP_LOCATION);

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attr = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(), nullptr, &requestAttr);
            strLocation = QString::fromStdString(attr[CUPS_OP_LOCATION].data());
            strLocation = strLocation.remove(0, 1);
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return strLocation;
}

void DDestination::setPrinterLocation(const QString &location)
{
    m_printerLocation = location;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->setPrinterLocation(m_strName.toStdString().c_str(), m_printerLocation.toStdString().c_str());
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}


bool DDestination::initPrinterPPD()
{
    return false;
}

bool DDestination::isPpdFileBroken()
{
    return true;
}

void DDestination::getPrinterInfo(QString &info, QString &location, QString &deviceURI, QString &ppdfile) const
{
    info = m_printerInfo;
    location = m_printerLocation;
    deviceURI = m_printerURI;
    ppdfile = m_ppdFile;
}

void DDestination::initPrinterAttr()
{
    vector<string> printerAttrs;
    printerAttrs.push_back(CUPS_OP_ISSHAED);
    printerAttrs.push_back(CUPS_OP_ISACCEPT);
    printerAttrs.push_back(CUPS_OP_LOCATION);
    printerAttrs.push_back(CUPS_OP_MAKE_MODEL);
    printerAttrs.push_back(CUPS_OP_STATE);
    printerAttrs.push_back(CUPS_OP_INFO);
    printerAttrs.push_back(CUPS_DEV_URI);

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return;
        map<string, string> attrMap = conPtr->getPrinterAttributes(m_strName.toStdString().data(), nullptr, &printerAttrs);
        string strShared = attrMap.at(CUPS_OP_ISSHAED);
        if (strShared.substr(0, 2) == "b1") {
            m_isShared = true;
        } else {
            m_isShared = false;
        }

        m_printerLocation = attrMap.at(CUPS_OP_LOCATION).substr(1).data();
        m_printerInfo = attrMap.at(CUPS_OP_INFO).substr(1).data();

        if (attrMap.find(CUPS_OP_MAKE_MODEL) != attrMap.end()) {
            m_printerModel = attrMap.at(CUPS_OP_MAKE_MODEL).substr(1).data();
        } else {
            m_printerModel = "";
        }

        m_printerURI = attrMap.at(CUPS_DEV_URI).substr(1).data();
        m_ppdFile = conPtr->getPPD(m_strName.toStdString().data()).data();
        // 此处i3表示数据类型为int 值为3，其实是enumeration类型转换
        if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i3")) {
            m_printerStatus = QObject::tr("Idle");
            m_isEnabled = true;
        } else if (attrMap.at(CUPS_OP_STATE).substr(0, 2) == string("i4")) {
            m_printerStatus = QObject::tr("Printing");
            m_isEnabled = true;
        } else {
            m_printerStatus = QObject::tr("Disabled");
            m_isEnabled = false;
        }
    } catch (const std::runtime_error &e) {
        qCCritical(UTIL) << e.what();
    }
}

DESTTYPE DDestination::getType() const
{
    return m_type;
}
