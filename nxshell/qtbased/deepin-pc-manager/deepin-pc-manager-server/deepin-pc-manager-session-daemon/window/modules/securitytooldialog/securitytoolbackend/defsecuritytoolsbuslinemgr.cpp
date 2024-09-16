// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defsecuritytoolsbuslinemgr.h"

#include "../../deepin-pc-manager/src/window/modules/common/gsettingkey.h"
#include "../../deepin-pc-manager/src/window/modules/common/invokers/invokerfactory.h"
#include "window/modules/common/aes/aesencrpyobj.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QPair>

#define DEFENDER_DATA_DIR_PATH "/usr/share/deepin-pc-manager/" // 数据库路径

DefToolAuthorityCheckObj::DefToolAuthorityCheckObj(QObject *pParent)
    : QObject(pParent)
{
}

DefToolAuthorityCheckObj::~DefToolAuthorityCheckObj() { }

DefSecurityToolsBusLineMgr::DefSecurityToolsBusLineMgr(QObject *pParent)
    : QObject(pParent)
    , m_pAuthorityCheckObj(nullptr)
    , m_pManagerInter(nullptr)
    , m_bLoad(false)
    , m_gSettingsInvokerInter(nullptr)
    , m_isSecEnhanceSrvValid(false)
    , m_isResourceManagerValid(false)
{
    m_pManagerInter =
        InvokerFactory::GetInstance().CreateInvoker("com.home.appstore.daemon",
                                                    "/appstore",
                                                    "com.home.appstore.daemon.interface",
                                                    ConnectType::SYSTEM,
                                                    this);
    m_gSettingsInvokerInter =
        InvokerFactory::GetInstance().CreateSettings(DEEPIN_PC_MANAGER_GSETTING_PATH, "", this);
}

DefSecurityToolsBusLineMgr::~DefSecurityToolsBusLineMgr()
{
    if (!m_pAuthorityCheckObj) {
        m_pAuthorityCheckObj->deleteLater();
        m_pAuthorityCheckObj = nullptr;
    }
}

void DefSecurityToolsBusLineMgr::setAutorityCheckObj(DefToolAuthorityCheckObj *pObj)
{
    m_pAuthorityCheckObj = pObj;
}

void DefSecurityToolsBusLineMgr::installPackage(const QString &strPackageKey)
{
    if (!hasAutority()) {
        return;
    }

    if (!isInit()) {
        init();
    }

    if (!isInit()) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::INSTALLFAILED);
        return;
    }

    if (!hasInstaller(strPackageKey)) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::INSTALLFAILED);
        return;
    }

    DefSecurityToolsBaseInstaller *pInstaller = getInstaller(strPackageKey);

    if (pInstaller) {
        QObject::connect(pInstaller,
                         &DefSecurityToolsBaseInstaller::sAppStatusChanged,
                         this,
                         &DefSecurityToolsBusLineMgr::onAppStatusChanged);
        pInstaller->installPackage();
    } else {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::INSTALLFAILED);
    }
}

void DefSecurityToolsBusLineMgr::unInstallPackage(const QString &strPackageKey)
{
    if (!hasAutority()) {
        return;
    }

    if (!isInit()) {
        init();
    }

    if (!isInit()) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UNINSTALLFAILED);
        return;
    }

    if (!hasInstaller(strPackageKey)) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UNINSTALLFAILED);
        return;
    }

    DefSecurityToolsBaseInstaller *pInstaller = getInstaller(strPackageKey);

    if (pInstaller) {
        QObject::connect(pInstaller,
                         &DefSecurityToolsBaseInstaller::sAppStatusChanged,
                         this,
                         &DefSecurityToolsBusLineMgr::onAppStatusChanged);
        pInstaller->unInstallPackage();
    } else {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UNINSTALLFAILED);
    }
}

void DefSecurityToolsBusLineMgr::updatePackage(const QString &strPackageKey)
{
    if (!hasAutority()) {
        return;
    }

    if (!isInit()) {
        init();
    }

    if (!isInit()) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UPDATEFAIL);
        return;
    }

    if (!hasInstaller(strPackageKey)) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UPDATEFAIL);
        return;
    }

    DefSecurityToolsBaseInstaller *pInstaller = getInstaller(strPackageKey);

    if (pInstaller) {
        QObject::connect(pInstaller,
                         &DefSecurityToolsBaseInstaller::sAppStatusChanged,
                         this,
                         &DefSecurityToolsBusLineMgr::onAppStatusChanged);
        pInstaller->updatePackage();
    } else {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UPDATEFAIL);
    }
}

void DefSecurityToolsBusLineMgr::showTool(const QString &strPackageKey,
                                          const QStringList &strParams)
{
    if (!hasAutority()) {
        return;
    }

    if (!isInit()) {
        init();
    }

    if (!isInit()) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UPDATEFAIL);
        return;
    }

    if (!hasInstaller(strPackageKey)) {
        Q_EMIT notifyAppStatusChanged(strPackageKey, DEFSECURITYTOOLSTATUS::UPDATEFAIL);
        return;
    }

    DefSecurityToolsBaseInstaller *pInstaller = getInstaller(strPackageKey);

    if (pInstaller) {
        pInstaller->showtool(strParams);
    }
}

void DefSecurityToolsBusLineMgr::loadAlltools()
{
    if (!hasAutority()) {
        return;
    }

    DEFSECURITYTOOLINFOLIST infolist;
    QDir defenderDataDir(DEFENDER_DATA_DIR_PATH);

    if (!defenderDataDir.exists(DEFENDER_DATA_DIR_PATH)) {
        m_bLoad = false;
        Q_EMIT notifyToolsInfoUpdate(infolist);
        return;
    }

    QString strFileName = QString("%1toolinfo.json").arg(DEFENDER_DATA_DIR_PATH);
    infolist = readFrom(strFileName);

    // 不可以直接对其进行卸载,需要判断其信息是否存在。存在则更新信息，否则挂载一个安装器。
    DEFSECURITYTOOLINFOLIST RTInfolist;

    foreach (DEFSECURITYTOOLINFO info, infolist) {
        DefSecurityToolsBaseInstaller *pInstaller = nullptr;
        QString strPackageKey = info.getPackageKey();

        if (hasInstaller(strPackageKey)) {
            pInstaller = getInstaller(strPackageKey);

            if (!pInstaller) {
                unregistInstaller(strPackageKey);
                qCritical() << "Installer can't be empty!";
                continue;
            }

            pInstaller->setInfo(info);
        } else {
            pInstaller = DefSecurityToolsInstallerFactory::createInstaller(info);

            if (pInstaller) {
                pInstaller->setInfo(info);
                registerInstaller(strPackageKey, pInstaller);
            }
        }

        Q_ASSERT(pInstaller != nullptr);

        // 设置本地最新包信息
        if (m_latestLocalPackages.contains(info.strPackageName)) {
            pInstaller->setLatestPackageInfo(m_latestLocalPackages.value(info.strPackageName));
        }

        // 系统工具默认总是安装，不用查。
        if (!info.bSystemTool) {
            info.bHasInstalled = pInstaller->isPackageExist();
        }

        if (!info.bSystemTool) {
            info.bCanUpdate = pInstaller->isUpdatable();
        } else {
            info.bCanUpdate = false;
        }

        // 当包不可安装时不挂载
        if (!info.bSystemTool) {
            if (pInstaller->isPackageInstallable()) {
                RTInfolist.push_back(info);
            } else {
                unregistInstaller(info.getPackageKey());
            }
        } else {
            // 判断工具是否可用，如不可用则隐藏
            if ("Execution Prevention" == info.strAppName && !isExecPreventValid()) {
                continue;
            }

            if ("Superior Protection" == info.strAppName && !isSuperiorProtectionValid()) {
                continue;
            }

            if ("Block Removal of Kernel Modules" == info.strAppName
                && !isKernelModulesProtectionValid()) {
                continue;
            }

            if ("Internet Control" == info.strAppName && !isResourceManagerValid()) {
                continue;
            }

            RTInfolist.push_back(info);
        }
    }

    m_bLoad = true;
    Q_EMIT notifyToolsInfoUpdate(RTInfolist);
}

bool DefSecurityToolsBusLineMgr::isInit() const
{
    return m_bLoad;
}

void DefSecurityToolsBusLineMgr::init()
{
    initLatestLocalPackages();
    loadAlltools();
}

bool DefSecurityToolsBusLineMgr::hasAutority()
{
    return true;
}

void DefSecurityToolsBusLineMgr::registerInstaller(const QString &strPackageKey,
                                                   DefSecurityToolsBaseInstaller *pInstaller)
{
    Q_ASSERT(pInstaller != nullptr);
    QString strKey = strPackageKey.trimmed();
    pInstaller->setStoreManagerInter(m_pManagerInter);
    m_pToolInstallers[strKey] = pInstaller;
}

void DefSecurityToolsBusLineMgr::unregistInstaller(const QString &strPackageKey)
{
    QString strKey = strPackageKey.trimmed();

    if (m_pToolInstallers.contains(strKey)) {
        DefSecurityToolsBaseInstaller *pInstaller = m_pToolInstallers[strKey];

        if (pInstaller) {
            pInstaller->deleteLater();
            pInstaller = nullptr;
        }

        m_pToolInstallers.remove(strKey);
    }
}

bool DefSecurityToolsBusLineMgr::hasInstaller(const QString &strPackageKey)
{
    QString strKey = strPackageKey.trimmed();
    return m_pToolInstallers.contains(strKey);
}

DEFSECURITYTOOLINFOLIST DefSecurityToolsBusLineMgr::defaultSecurityInfos()
{
    // 在此添加系统黙认工具信息
    DEFSECURITYTOOLINFOLIST infolist;
    return infolist;
}

DEFSECURITYTOOLINFOLIST DefSecurityToolsBusLineMgr::readFrom(const QString &strFileName)
{
    DEFSECURITYTOOLINFOLIST infolist;
    infolist += defaultSecurityInfos();
    QFile rf(strFileName);

    if (!rf.open(QIODevice::ReadOnly)) {
        return infolist;
    }

    QByteArray byData = rf.readAll();
    rf.close();
    AesEncrpyObj aesObj;
    aesObj.initKey();
    QByteArray byDecrypt = aesObj.decryptData(byData);
    QJsonParseError jsonErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(byDecrypt, &jsonErr);

    if (QJsonParseError::NoError != jsonErr.error) {
        qCritical() << jsonErr.errorString();
        return infolist;
    }

    QJsonObject jsonAllInfos = jsonDoc.object();
    QJsonArray jsonClassifyTools = jsonAllInfos.value("Data").toArray();

    for (int i = 0; i < jsonClassifyTools.size(); i++) {
        QJsonValue value = jsonClassifyTools.at(i);
        QJsonObject jsonInfoObj = value.toObject();
        DEFSECURITYTOOLINFO info;
        info.strPackageName = jsonInfoObj.value("PackageName").toString();
        info.strAppName = jsonInfoObj.value("AppName").toString();
        info.strImagePath = jsonInfoObj.value("ImagePath").toString();
        info.strCompany = jsonInfoObj.value("CompanyName").toString();
        info.strDescription = jsonInfoObj.value("Description").toString();
        QString strInt = jsonInfoObj.value("Classify").toString();
        info.iClassify = strInt.toInt();
        info.strInallModel = jsonInfoObj.value("InstallModel").toString();
        info.strCallMethodName = jsonInfoObj.value("CallMethodName").toString();
        info.bSystemTool = jsonInfoObj.value("SystemTool").toBool();
        info.bHasInstalled = jsonInfoObj.value("HasInstalled").toBool();
        info.strReserve = jsonInfoObj.value("Reserve").toString();
        info.strReserve2 = jsonInfoObj.value("Reserve2").toString();
        info.strReserve3 = jsonInfoObj.value("Reserve3").toString();
        info.strReserve4 = jsonInfoObj.value("Reserve4").toString();
        infolist.push_back(info);
    }

    return infolist;
}

void DefSecurityToolsBusLineMgr::writeTo(const DEFSECURITYTOOLINFOLIST &infolist,
                                         const QString &strFileName)
{
    QJsonArray jsonClassifyTools;

    foreach (DEFSECURITYTOOLINFO info, infolist) {
        QJsonObject jsonToolInfo;
        jsonToolInfo.insert("PackageName", info.strPackageName);
        jsonToolInfo.insert("AppName", info.strAppName);
        jsonToolInfo.insert("ImagePath", info.strImagePath);
        jsonToolInfo.insert("CompanyName", info.strCompany);
        jsonToolInfo.insert("Description", info.strDescription);
        jsonToolInfo.insert("Classify", QString::number(info.iClassify));
        jsonToolInfo.insert("InstallModel", info.strInallModel);
        jsonToolInfo.insert("CallMethodName", info.strCallMethodName);
        jsonToolInfo.insert("SystemTool", info.bSystemTool);
        jsonToolInfo.insert("HasInstalled", info.bHasInstalled);
        jsonToolInfo.insert("Reserve", info.strReserve);
        jsonToolInfo.insert("Reserve2", info.strReserve2);
        jsonToolInfo.insert("Reserve3", info.strReserve3);
        jsonToolInfo.insert("Reserve4", info.strReserve4);
        jsonClassifyTools.push_back(jsonToolInfo);
    }

    QJsonObject jsonAllInfos;
    jsonAllInfos.insert("Data", jsonClassifyTools);
    QJsonDocument document;
    document.setObject(jsonAllInfos);
    QByteArray byData = document.toJson(QJsonDocument::Indented);

    QFile wf(strFileName);

    if (wf.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        AesEncrpyObj aesObj;
        aesObj.initKey();
        QByteArray byEncrypt = aesObj.encryptData(byData);
        wf.write(byEncrypt);
        wf.close();
    }
}

DefSecurityToolsBaseInstaller *
DefSecurityToolsBusLineMgr::getInstaller(const QString strPackageKey) const
{
    QString strKey = strPackageKey.trimmed();
    DefSecurityToolsBaseInstaller *pInstaller = nullptr;

    if (m_pToolInstallers.contains(strKey)) {
        pInstaller = m_pToolInstallers.value(strKey);
    }

    return pInstaller;
}

void DefSecurityToolsBusLineMgr::initLatestLocalPackages()
{
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_pManagerInter, "GetLocalOnTheShelvesAppInfo");
    QStringList strReplys = msg.arguments().first().toStringList();

    if (!strReplys.isEmpty()) {
        foreach (QString strVal, strReplys) {
            QByteArray replayData = QByteArray::fromStdString(strVal.toStdString());
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(replayData, &error);

            if (QJsonParseError::NoError == error.error) {
                PACKAGEMSG packageMsg;
                QJsonObject jsonObj = doc.object();

                if (jsonObj.contains("package_name")) {
                    packageMsg.strPackageName = jsonObj.value("package_name").toString();
                }

                if (jsonObj.contains("package_version")) {
                    packageMsg.strVersion = jsonObj.value("package_version").toString();
                }

                m_latestLocalPackages.insert(packageMsg.strPackageName, packageMsg);
            } else {
                qWarning() << "get local package information failed!";
            }
        }
    }
}

bool DefSecurityToolsBusLineMgr::isHideTools(const DEFSECURITYTOOLINFO &info,
                                             const QString &strAppName,
                                             const QString &strGsetting)
{
    bool m_isShow = true;
    if (strAppName == info.strAppName) {
        m_isShow = m_gSettingsInvokerInter->GetValue(strGsetting).toBool();
    }

    return m_isShow;
}

bool DefSecurityToolsBusLineMgr::isExecPreventValid()
{
    // 目前尚未开发
    return false;
}

bool DefSecurityToolsBusLineMgr::isSuperiorProtectionValid()
{
    return m_isSecEnhanceSrvValid;
}

bool DefSecurityToolsBusLineMgr::isKernelModulesProtectionValid()
{
    // 目前尚未开发
    return false;
}

bool DefSecurityToolsBusLineMgr::isResourceManagerValid()
{
    return m_isResourceManagerValid;
}

void DefSecurityToolsBusLineMgr::onAppStatusChanged(const QString &strPackageKey,
                                                    DEFSECURITYTOOLSTATUS status)
{
    if (DEFSECURITYTOOLSTATUS::INSTALLING != status && DEFSECURITYTOOLSTATUS::UNINSTALLING != status
        && DEFSECURITYTOOLSTATUS::UPDATING != status) {
        DefSecurityToolsBaseInstaller *pInstaller = getInstaller(strPackageKey);
        QObject::disconnect(pInstaller,
                            &DefSecurityToolsBaseInstaller::sAppStatusChanged,
                            this,
                            &DefSecurityToolsBusLineMgr::onAppStatusChanged);
    }

    Q_EMIT notifyAppStatusChanged(strPackageKey, status);
}
