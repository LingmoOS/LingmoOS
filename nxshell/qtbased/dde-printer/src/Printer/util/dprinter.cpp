// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dprinter.h"
#include "dprintermanager.h"
#include "cupsconnectionfactory.h"

#include <assert.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QDir>

DPrinter::DPrinter()
{
    m_type = DESTTYPE::PRINTER;
    m_bNeedSavePpd = false;
    m_bNeedUpdateInk = true;
}

DPrinter::~DPrinter()
{

}

bool DPrinter::initPrinterPPD()
{
    bool bRet = false;
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            time_t tm = 0;
            string strPPDName = conPtr->getPPD3(m_strName.toStdString().c_str(), &tm, nullptr);
            m_ppd.load(strPPDName.c_str());
            m_ppd.localize();
            bRet = true;
        }
    } catch (const std::runtime_error &e) {
        bRet = false;
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return bRet;
}

bool DPrinter::isPpdFileBroken()
{
    bool bRet = initPrinterPPD();
    return !bRet;
}

void DPrinter::setPageSize(const QString &strValue)
{
    setOptionValue("PageSize", strValue);
}

QString DPrinter::getPageSize()
{
    return getOptionValue("PageSize");
}

QVector<QMap<QString, QString>> DPrinter::getPageSizeChooses()
{
    return getOptionChooses("PageSize");
}

void DPrinter::setPageRegion(const QString &strValue)
{
    setOptionValue("PageRegion", strValue);
}

QString DPrinter::getPageRegion()
{
    return getOptionValue("PageRegion");
}

QVector<QMap<QString, QString>> DPrinter::getPageRegionChooses()
{
    return getOptionChooses("PageRegion");
}

void DPrinter::setMediaType(const QString &strValue)
{
    setOptionValue("MediaType", strValue);
}

QString DPrinter::getMediaType()
{
    return getOptionValue("MediaType");
}

QVector<QMap<QString, QString>> DPrinter::getMediaTypeChooses()
{
    return getOptionChooses("MediaType");
}

bool DPrinter::canSetColorModel()
{
    bool bRet = false;

    try {
        Attribute attr = m_ppd.findAttr("ColorDevice", nullptr);
        QString strValue = QString::fromStdString(attr.getValue());
        strValue = strValue.trimmed();

        if (strValue == "False") {
            bRet = false;
        } else {
            bRet = true;
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    return bRet;
}

void DPrinter::setColorModel(const QString &strColorMode)
{
    /*
    if (bColor) {
        setOptionValue("ColorModel", "RGB");
    } else {
        setOptionValue("ColorModel", "Gray");
    }
    */

    if (getOptionValue("ColorModel").isEmpty()) {
        setOptionValue("ColorMode", strColorMode);
        return;
    }

    setOptionValue("ColorModel", strColorMode);
}

QString DPrinter::getColorModel()
{
    QString strColorModel = getOptionValue("ColorModel");

    if (!strColorModel.isEmpty()) {
        return strColorModel;
    }

    strColorModel = getOptionValue("ColorMode");
    return strColorModel.isEmpty() ? getColorAttr() : strColorModel;
}

QVector<QMap<QString, QString>> DPrinter::getColorModelChooses()
{
    QVector<QMap<QString, QString>> vecChoose = getOptionChooses("ColorModel");

    if (!vecChoose.isEmpty()) {
        return vecChoose;
    }

    if (!(vecChoose = getOptionChooses("ColorMode")).isEmpty()) {
        return vecChoose;
    }

    QString strVal = getColorAttr();

    if (!strVal.isEmpty()) {
        QMap<QString, QString> choose;

        if (strVal == "Gray") {
            choose["choice"] = "Gray";
            choose["text"] = "Grayscale";
        } else {
            choose["choice"] = "RGB";
            choose["text"] = "Color";
        }

        vecChoose.push_back(choose);
    }

    return vecChoose;
}

void DPrinter::setPrintQuality(const QString &strValue)
{
    setOptionValue("OutputMode", strValue);
}

QString DPrinter::getPrintQuality()
{
    return getOptionValue("OutputMode");
}

QVector<QMap<QString, QString>> DPrinter::getPrintQualityChooses()
{
    return getOptionChooses("OutputMode");
}

void DPrinter::setInputSlot(const QString &strValue)
{
    setOptionValue("InputSlot", strValue);
}

QString DPrinter::getInputSlot()
{
    return getOptionValue("InputSlot");
}

QVector<QMap<QString, QString>> DPrinter::getInputSlotChooses()
{
    return getOptionChooses("InputSlot");
}

bool DPrinter::canSetDuplex()
{
    bool bRet = true;
    return bRet;
}

void DPrinter::setDuplex(const QString &strValue)
{
    setOptionValue("Duplex", strValue);
}

QString DPrinter::getDuplex()
{
    return getOptionValue("Duplex");
}

QVector<QMap<QString, QString>> DPrinter::getDuplexChooses()
{
    return getOptionChooses("Duplex");
}

QString DPrinter::getDriverName()
{
    std::string strLinkPath;
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            strLinkPath = conPtr->getPPD(m_strName.toStdString().c_str());
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
    return QFile::symLinkTarget(QString::fromStdString(strLinkPath));
}

QString DPrinter::getPrinterMakeAndModel()
{
    QString strMakeAndModel = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("printer-make-and-model");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return strMakeAndModel;
        map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
            strMakeAndModel = QString::fromStdString(iter->second.data());
            strMakeAndModel = strMakeAndModel.remove(0, 1);

            if (strMakeAndModel.contains("(recommended)")) {
                strMakeAndModel.remove("(recommended)");
                strMakeAndModel.append(QObject::tr("(recommended)"));
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strMakeAndModel.clear();
    }

    return strMakeAndModel;
}

QString DPrinter::getPrinterUri()
{
    QString strUri = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("device-uri");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                     nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                strUri = QString::fromStdString(iter->second.data());
                strUri = strUri.remove(0, 1);
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strUri.clear();
    }

    return strUri;
}

void DPrinter::setPrinterUri(const QString &strValue)
{
    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->setPrinterDevice(m_strName.toStdString().c_str(), strValue.toStdString().c_str());
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QString DPrinter::getPageOrientation()
{
    QString strOritentation = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("orientation-requested-default");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                     nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                if (0 == iter->first.compare("orientation-requested-default")) {
                    strOritentation = QString::fromStdString(iter->second.data());

                    if (strOritentation.isEmpty()) {
                        strOritentation = QString::fromStdString("3");
                    } else {
                        strOritentation = strOritentation.remove('i');
                        strOritentation = strOritentation.trimmed();
                    }
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strOritentation.clear();
    }

    return strOritentation;
}

void DPrinter::setPageOrientationChoose(const QString &strValue)
{
    try {
        string strDefault = strValue.toStdString();
        vector<string> Attrs;
        Attrs.push_back(strDefault);
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->addPrinterOptionDefault(m_strName.toStdString().c_str(),
                                            "orientation-requested", &Attrs);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QVector<QMap<QString, QString>> DPrinter::getPageOrientationChooses()
{
    QVector<QMap<QString, QString>> vecChoose;

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("orientation-requested-supported");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(), nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                QMap<QString, QString> choose;

                if (0 == iter->first.compare("orientation-requested-supported")) {
                    QString strChoose = QString::fromStdString(iter->second);
                    //QStringList strChooses = strChoose.split('`');
                    QStringList strChooses = strChoose.split('\0');
                    DPrinterManager *pManger = DPrinterManager::getInstance();

                    for (int i = 0; i < strChooses.size(); i++) {
                        QString strTemp = strChooses[i].remove('`');
                        strTemp = strChooses[i].remove('i');
                        strTemp = strTemp.trimmed();

                        if (0 == strTemp.compare("3")) {
                            choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Portrait (no rotation)", "Portrait (no rotation)");
                            choose[QString::fromStdString("choice")] = strTemp;
                        } else if (0 == strTemp.compare("4")) {
                            choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Landscape (90 degrees)", "Landscape (90 degrees)");
                            choose[QString::fromStdString("choice")] = strTemp;
                        } else if (0 == strTemp.compare("5")) {
                            choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Reverse landscape (270 degrees)", "Reverse landscape (270 degrees)");
                            choose[QString::fromStdString("choice")] = strTemp;
                        } else if (0 == strTemp.compare("6")) {
                            choose[QString::fromStdString("text")] = pManger->translateLocal("Orientation_Combo", "Reverse portrait (180 degrees)", "Reverse portrait (180 degrees)");
                            choose[QString::fromStdString("choice")] = strTemp;
                        } else {
                            continue;
                        }

                        vecChoose.push_back(choose);
                    }
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        vecChoose.clear();
    }

    return vecChoose;
}

QString DPrinter::getPageOutputOrder()
{
    QString strOrder = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("outputorder-default");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return strOrder;
        map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        if (0 == attrs.size()) {
            setPageOutputOrder(QString::fromStdString("normal"));
            strOrder = QString::fromStdString("normal");
        } else {
            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                if (0 == iter->first.compare("outputorder-default")) {
                    strOrder = QString::fromStdString(iter->second.data());

                    if (strOrder.isEmpty()) {
                        strOrder = " ";
                    } else {
                        strOrder = strOrder.remove(0, 1);
                        strOrder = strOrder.trimmed();
                    }
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strOrder.clear();
    }

    return strOrder;
}

void DPrinter::setPageOutputOrder(const QString &strValue)
{
    try {
        string strDefault = strValue.toStdString();
        vector<string> Attrs;
        Attrs.push_back(strDefault);
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->addPrinterOptionDefault(m_strName.toStdString().c_str(),
                                            "outputorder", &Attrs);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QVector<QMap<QString, QString>> DPrinter::getPageOutputOrderChooses()
{
    DPrinterManager *pManger = DPrinterManager::getInstance();
    QVector<QMap<QString, QString>> vecChoose;
    //normal,reverse
    QMap<QString, QString> choose;
    choose[QString::fromStdString("text")] = pManger->translateLocal("PrintOrder_Combo", "Normal", "Normal");
    choose[QString::fromStdString("choice")] = QString::fromStdString("normal");
    vecChoose.push_back(choose);
    choose.clear();
    choose[QString::fromStdString("text")] = pManger->translateLocal("PrintOrder_Combo", "Reverse", "Reverse");
    choose[QString::fromStdString("choice")] = QString::fromStdString("reverse");
    vecChoose.push_back(choose);
    return vecChoose;
}

QString DPrinter::getBindEdgeOption()
{
    return getOptionValue("BindEdge");
}

void DPrinter::setBindEdgetOption(const QString &strValue)
{
    setOptionValue("BindEdge", strValue);
}

QVector<QMap<QString, QString>> DPrinter::getBindEdgeChooses()
{
    return getOptionChooses("BindEdge");
}

QString DPrinter::getFinishings()
{
    QString strFinishings = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("finishings-default");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return strFinishings;
        map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
            if (0 == iter->first.compare("finishings-default")) {
                strFinishings = QString::fromStdString(iter->second.data());

                if (strFinishings.isEmpty()) {
                    strFinishings = " ";
                } else {
                    strFinishings = strFinishings.remove('i');
                    strFinishings = strFinishings.trimmed();

                    if (strFinishings == "3") {
                        strFinishings = "none";
                    } else if (strFinishings == "51") {
                        strFinishings = "left";
                    } else if (strFinishings == "50") {
                        strFinishings = "right";
                    }
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strFinishings.clear();
    }

    return strFinishings;
}

void DPrinter::setFinishings(const QString &strValue)
{
    try {
        string strDefault;

        if (strValue == "left") {
            strDefault = "51";
        } else if (strValue == "top") {
            strDefault = "50";
        } else {
            strDefault = "3";
        }

        vector<string> Attrs;
        Attrs.push_back(strDefault);
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->addPrinterOptionDefault(m_strName.toStdString().c_str(),
                                            "finishings", &Attrs);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QVector<QMap<QString, QString>> DPrinter::getFinishingsChooses()
{
    QVector<QMap<QString, QString>> vecChoose;

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("finishings-supported");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (!conPtr)
            return vecChoose;
        map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                 nullptr, &requestAttrs);

        for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
            QMap<QString, QString> choose;
            QString strChoose = QString::fromStdString(iter->second);
            QStringList strChooses = strChoose.split('\0');
            DPrinterManager *pManger = DPrinterManager::getInstance();

            for (int i = 0; i < strChooses.size(); i++) {
                QString strTemp = strChooses[i].remove('`');
                strTemp = strChooses[i].remove('i');
                strTemp = strTemp.trimmed();

                if (0 == strTemp.compare("3")) {
                    choose[QString::fromStdString("text")] = pManger->translateLocal("StapleLocation_Combo", "Bind (none)", "Bind (none)");
                    choose[QString::fromStdString("choice")] = "none";
                } else if (0 == strTemp.compare("50")) {
                    choose[QString::fromStdString("text")] = pManger->translateLocal("StapleLocation_Combo", "Bind (top)", "Bind (top)");
                    choose[QString::fromStdString("choice")] = "top";
                } else if (0 == strTemp.compare("51")) {
                    choose[QString::fromStdString("text")] = pManger->translateLocal("StapleLocation_Combo", "Bind (left)", "Bind (left)");
                    choose[QString::fromStdString("choice")] = "left";
                } else {
                    continue;
                }

                vecChoose.push_back(choose);
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        vecChoose.clear();
    }

    return vecChoose;
}

QString DPrinter::getStapleLocation()
{
    return getOptionValue("StapleLocation");
}

void DPrinter::setStapleLoaction(const QString &strVal)
{
    setOptionValue("StapleLocation", strVal);
}

QVector<QMap<QString, QString>> DPrinter::getStapLocationChooses()
{
    return getOptionChooses("StapleLocation");
}

QString DPrinter::getResolution()
{
    return getOptionValue("Resolution");
}

void DPrinter::setResolution(const QString &strValue)
{
    setOptionValue("Resolution", strValue);
}

QVector<QMap<QString, QString>> DPrinter::getResolutionChooses()
{
    return getOptionChooses("Resolution");
}

QVector<SUPPLYSDATA> DPrinter::getSupplys()
{
    return m_vecMarkInfo;
}

void DPrinter::updateSupplys()
{
    if (!m_bNeedUpdateInk) {
        return;
    }

    m_vecMarkInfo.clear();
    QString strURI = getPrinterUri();
    QString strLoc = printerLocation();

    if (strURI.startsWith("socket://")) {
        time_t tm = 0;
        std::string strPPDName ;
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            try {
                strPPDName = conPtr->getPPD3(m_strName.toStdString().c_str(), &tm, nullptr);
            } catch (std::invalid_argument &e) {
                qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
                return;
            }
        } else {
            return;
        }

        cupssnmp snmp;
        snmp.setIP(strLoc.toStdString());
        snmp.setPPDName(strPPDName);
        snmp.SNMPReadSupplies();
        vector<SUPPLYSDATA> tempVec = snmp.getMarkInfo();

        for (unsigned int i = 0; i < tempVec.size(); i++) {
            m_vecMarkInfo.push_back(tempVec[i]);
        }
    } else if (strURI.startsWith("ipp://")) {
        try {

            QString strMarkerLevel;
            QString strMarkerName;
            QStringList strLevelList;
            QStringList strNameList;

            vector<string> requestAttrs;
            requestAttrs.push_back("marker-names");
            requestAttrs.push_back("marker-levels");
            requestAttrs.push_back("marker-high-levels");

            auto conPtr = CupsConnectionFactory::createConnection(strLoc, ippPort(), 0);
            if (!conPtr)
                return;
            map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                     nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                if (iter->first == "marker-names") {
                    strMarkerName = QString::fromStdString(iter->second);
                    strNameList = strMarkerName.split('\0');
                } else {
                    strMarkerLevel = QString::fromStdString(iter->second);
                    strLevelList = strMarkerLevel.split('\0');
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
                m_vecMarkInfo.push_back(info);
            }
        } catch (const std::runtime_error &e) {
            qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        }
    }

    m_bNeedUpdateInk = false;
}

void DPrinter::disableSupplys()
{
    m_bNeedUpdateInk = true;
}

int DPrinter::getMinMarkerLevel()
{
    int iMinValue = 100;

    if (m_vecMarkInfo.size() > 0) {
        for (int i = 0; i < m_vecMarkInfo.size(); i++) {
            if ((iMinValue > m_vecMarkInfo[i].level) && (m_vecMarkInfo[i].level >= 0)) {
                iMinValue = m_vecMarkInfo[i].level;
            }
        }
    } else {
        iMinValue = 1000;
    }

    return iMinValue;
}

void DPrinter::SetSupplys(const QVector<SUPPLYSDATA> &vecMarkInfo)
{
    m_bNeedUpdateInk = false;
    m_vecMarkInfo = vecMarkInfo;
}

QStringList DPrinter::getDefaultPpdOpts()
{
    QStringList strDefaultOptions;
    QString strValue = getPageSize();
    strDefaultOptions.push_back(strValue);
    strValue = getMediaType();
    strDefaultOptions.push_back(strValue);
    strValue = getColorModel();
    strDefaultOptions.push_back(strValue);
    strValue = getPrintQuality();
    strDefaultOptions.push_back(strValue);
    strValue = getInputSlot();
    strDefaultOptions.push_back(strValue);
    strValue = getDuplex();
    strDefaultOptions.push_back(strValue);
    return strDefaultOptions;
}

bool DPrinter::saveModify()
{
    bool bRet = false;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            conPtr->addPrinter(getName().toUtf8().data(), nullptr, nullptr, nullptr, nullptr, nullptr, &m_ppd);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        bRet = false;
    }

    m_bNeedSavePpd = false;
    return bRet;
}

bool DPrinter::needSavePpd()
{
    return m_bNeedSavePpd;
}

bool DPrinter::isConflict(const QString &strModule, const QString &strValue, QVector<CONFLICTNODE> &vecConflicts)
{
    bool bRet = false;
    std::vector<Constraint> vecCons = m_ppd.getConstraints();

    for (unsigned int i = 0; i < vecCons.size(); i++) {
        Constraint tempCons = vecCons[i];
        QString strOption1 = QString::fromStdString(tempCons.getOption1());

        if (strOption1 != strModule) {
            continue;
        }

        QString strChoice1 = QString::fromStdString(tempCons.getChoice1());

        if (strChoice1 != strValue) {
            continue;
        }

        CONFLICTNODE node;
        node.strOption = QString::fromStdString(tempCons.getOption2());
        node.strValue = QString::fromStdString(tempCons.getChoice2());
        vecConflicts.push_back(node);
        bRet = true;
    }

    return bRet;
}

QVector<INSTALLABLEOPTNODE> DPrinter::getInstallableNodes()
{
    QVector<INSTALLABLEOPTNODE> nodes;

    try {
        vector<Group> groups = m_ppd.getOptionGroups();

        for (unsigned int k = 0; k < groups.size(); k++) {
            Group grp = groups[k];
            QString strName = QString(grp.getName().data());

            if (strName == QString::fromStdString("InstallableOptions")) {
                vector<Option>  opts = grp.getOptions();

                for (unsigned int i = 0; i < opts.size(); i++) {
                    Option opt = opts[i];
                    INSTALLABLEOPTNODE node;
                    node.strOptName = QString(opt.getKeyword().data());
                    node.strOptText = QString(opt.getText().data());
                    node.strDefaultValue = getOptionValue(node.strOptName);
                    node.vecChooseableValues = getOptionChooses(node.strOptName);
                    nodes.push_back(node);
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
    return nodes;
}

void DPrinter::setInstallableNodeValue(const QString &strOpt, const QString &strValue)
{
    try {
        setOptionValue(strOpt, strValue);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

QVector<GENERALOPTNODE> DPrinter::getGeneralNodes()
{
    QVector<GENERALOPTNODE> nodes;

    try {
        vector<Group> groups = m_ppd.getOptionGroups();

        for (unsigned int k = 0; k < groups.size(); k++) {
            Group grp = groups[k];
            QString strName = QString(grp.getName().data());

            if (strName != QString::fromStdString("InstallableOptions")) {
                vector<Option>  opts = grp.getOptions();

                for (unsigned int i = 0; i < opts.size(); i++) {
                    Option opt = opts[i];
                    GENERALOPTNODE node;
                    node.strOptName = QString::fromStdString(opt.getKeyword());

                    if (node.strOptName == QString("ColorModel") || /* add ppd file's ColorMode */
                       (node.strOptName == QString("PageRegion")) ||
                       !(node.strOptName.compare("ColorMode", Qt::CaseInsensitive))) {
                        continue;
                    }

                    node.strOptText = QString::fromStdString(opt.getText());
                    node.strOptText = node.strOptText.trimmed();
                    node.strDefaultValue = getOptionValue(node.strOptName);
                    node.vecChooseableValues = getOptionChooses(node.strOptName);
                    nodes.push_back(node);
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
    return nodes;
}

void DPrinter::setGeneralNodeValue(const QString &strOpt, const QString &strValue)
{
    try {
        setOptionValue(strOpt, strValue);
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
}

OPTNODE DPrinter::getOptionNodeByKeyword(const QString &strKey)
{
    OPTNODE node;

    try {
        vector<Group> groups = m_ppd.getOptionGroups();

        for (unsigned int k = 0; k < groups.size(); k++) {
            Group grp = groups[k];
            vector<Option>  opts = grp.getOptions();

            for (unsigned int i = 0; i < opts.size(); i++) {
                Option opt = opts[i];
                QString strTempKey = QString(opt.getKeyword().data());

                if (strTempKey == strKey) {
                    node.strOptName = strKey;
                    node.strOptText = QString(opt.getText().data());
                    node.strDefaultValue = getOptionValue(node.strOptName);
                    node.vecChooseableValues = getOptionChooses(node.strOptName);
                }
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }
    return node;
}

QString DPrinter::getPPDName()
{
    auto conPtr = CupsConnectionFactory::createConnectionBySettings();
    if (!conPtr)
        return "";
    QString strPpdName;
    try {
        time_t tm = 0;
        string strVal = conPtr->getPPD3(m_strName.toStdString().c_str(), &tm, nullptr);
        strPpdName = QString::fromStdString(strVal);
    } catch (const std::invalid_argument &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strPpdName.clear();
    }

    return strPpdName;
}

QString DPrinter::getOptionValue(const QString &strOptName)
{
    QString strDefault;

    try {
        Option opt = m_ppd.findOption(strOptName.toStdString().c_str());
        strDefault = QString::fromStdString(opt.getDefchoice());
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strDefault.clear();
    }

    return strDefault;
}

void DPrinter::setOptionValue(const QString &strOptName, const QString &strValue)
{
#ifdef _DEBUG
    cout << strOptName.c_str() << endl;
    cout << strValue.c_str() << endl;
#endif

    try {
        int iConflicts = m_ppd.markOption(strOptName.toStdString().c_str(), strValue.toStdString().c_str());

        if (iConflicts != 0) {
            qCDebug(UTIL) << "conflict numbers:" << iConflicts;
            m_bNeedSavePpd = false;
        } else {
            m_bNeedSavePpd = true;
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        m_bNeedSavePpd = false;
    }
}

QVector<QMap<QString, QString>> DPrinter::getOptionChooses(const QString &strOptName)
{
    QVector<QMap<QString, QString>> retMap;

    try {
        Option opt = m_ppd.findOption(strOptName.toStdString().c_str());
        vector<map<string, string>> vecChoices = opt.getChoices();

        for (unsigned int i = 0; i < vecChoices.size(); i++) {
            map<string, string> mapChoices = vecChoices[i];
            QMap<QString, QString> mapTrans;

            for (auto iter = mapChoices.begin(); iter != mapChoices.end(); iter++) {
                mapTrans.insert(QString::fromStdString(iter->first), QString::fromStdString(iter->second));
            }

            retMap.push_back(mapTrans);
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        retMap.clear();
    }

    return retMap;
}

QString DPrinter::getColorAttr()
{
    QString strDefault;

    try {
        Attribute attr = m_ppd.findAttr("DefaultColorSpace", nullptr);
        strDefault = QString::fromStdString(attr.getValue());
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
    }

    return strDefault;
}

QString DPrinter::getMarkerType()
{
    QString strMarkerType = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("marker-types");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                     nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                strMarkerType = QString::fromStdString(iter->second.data());
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strMarkerType.clear();
    }

    return strMarkerType;
}

QString DPrinter::getMarkerName()
{
    QString strMarkerName = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("marker-names");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                     nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                strMarkerName = QString::fromStdString(iter->second.data());
                strMarkerName = strMarkerName.trimmed();
                strMarkerName = strMarkerName.remove(0, 2);
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strMarkerName.clear();
    }

    return strMarkerName;
}

QString DPrinter::getMarkerLevel()
{
    QString strMarkerLevel = "";

    try {
        vector<string> requestAttrs;
        requestAttrs.push_back("marker-levels");
        //requestAttrs.push_back("marker-low-levels");
        //requestAttrs.push_back("marker-high-levels");
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr) {
            map<string, string> attrs = conPtr->getPrinterAttributes(m_strName.toStdString().c_str(),
                                                                     nullptr, &requestAttrs);

            for (auto iter = attrs.begin(); iter != attrs.end(); iter++) {
                strMarkerLevel = QString::fromStdString(iter->second.data());
            }
        }
    } catch (const std::runtime_error &e) {
        qCWarning(UTIL) << "Got execpt: " << QString::fromUtf8(e.what());
        strMarkerLevel.clear();
    }

    return strMarkerLevel;
}
