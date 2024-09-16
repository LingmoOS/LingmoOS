// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncworker.h"
#include "cryptor.h"

#include <DSysInfo>
#include <DDBusSender>
#include <QJsonDocument>
#include <QJsonObject>

#include <QProcess>
#include <QDBusConnection>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <unistd.h>

DCORE_USE_NAMESPACE

static const QString RebindError = QStringLiteral("7519");
static const QString SurPlusError = QStringLiteral("7520");
static const QString DeepinClientSv = QStringLiteral("com.deepin.deepinid.Client");
static const QString DeepinClientPath = QStringLiteral("/com/deepin/deepinid/Client");
static const QString DeepinClientIf = QStringLiteral("com.deepin.deepinid.Client");

static const QString SyncService = QStringLiteral("com.deepin.sync.Daemon");
static const QString SyncInterface = QStringLiteral("com.deepin.sync.Daemon");
static const QString SyncPath = QStringLiteral("/com/deepin/sync/Daemon");

static const QString DeepinIDService = QStringLiteral("com.deepin.deepinid");
static const QString DeepinIDInterface = QStringLiteral("com.deepin.deepinid");
static const QString DeepinIDPath = QStringLiteral("/com/deepin/deepinid");

static const QString UtcloudPath = QStringLiteral("/com/deepin/utcloud/Daemon");
static const QString UtcloudInterface = QStringLiteral("com.deepin.utcloud.Daemon");

// 根据deepin-deepinid-daemon决定deepinid是否显示
static const QString DeepinidDaemonPath = QStringLiteral("/usr/lib/deepin-deepinid-daemon/deepin-deepinid-daemon");
static const QString DaemonFildPath = QStringLiteral("/usr/lib/deepin-deepinid-daemon");


SyncWorker::SyncWorker(SyncModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_syncInter(new SyncDaemon(this))
    , m_deepinId_inter(new DeepinIdProxy(this))
    , m_syncHelperInter(new QDBusInterface("com.deepin.sync.Helper", "/com/deepin/sync/Helper", "com.deepin.sync.Helper", QDBusConnection::systemBus(), this))
    , m_utcloudInter(new QDBusInterface(SyncService, UtcloudPath, UtcloudInterface, QDBusConnection::sessionBus(), this))
    , m_watcher(new QFileSystemWatcher(this))
{

    registerIntStringMetaType();

    QDBusConnection::systemBus().connect("com.deepin.license", "/com/deepin/license/Info",
                                         "com.deepin.license.Info", "LicenseStateChange",
                                         this, SLOT(licenseStateChangeSlot()));

    connect(m_syncInter, &SyncDaemon::StateChanged, this, &SyncWorker::onStateChanged, Qt::QueuedConnection);
    connect(m_syncInter, &SyncDaemon::LastSyncTimeChanged, this, &SyncWorker::onLastSyncTimeChanged, Qt::QueuedConnection);
    connect(m_syncInter, &SyncDaemon::SwitcherChange, this, &SyncWorker::onSyncModuleStateChanged, Qt::QueuedConnection);
    connect(m_deepinId_inter, &DeepinIdProxy::UserInfoChanged, [this](const QVariantMap &userinfo){
        m_model->setUserinfo(userinfo);

        QTimer::singleShot(500, this, [this] {
            qDebug() << "On UserInfo Changed";
            refreshSyncState();   // sync.Daemon
            refreshSwitcherDump();  // utcloud.Daemon
        });
    });
    QDBusConnection::sessionBus().connect(SyncService,UtcloudPath,UtcloudInterface,"SwitcherChange","av",this, SLOT(utcloudSwitcherChange(QVariantList)));
    QDBusConnection::sessionBus().connect(SyncService,UtcloudPath,UtcloudInterface,"LoginStatus","av",this, SLOT(utcloudLoginStatus(QVariantList)));
    connect(this, &SyncWorker::pendingCallWatcherFinished, this, &SyncWorker::callWatcherResult);

    QFileInfo file(DeepinidDaemonPath);
    auto fileValid = (file.exists());
    m_watcher->addPath(DaemonFildPath);
    m_model->setSyncIsValid(fileValid);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged,this, [=](QString str){
        qDebug() << "FileSystemWatcher::fileChanged ==> " << str;
        if (str == DaemonFildPath) {
            QFileInfo file(DeepinidDaemonPath);
            m_model->setSyncIsValid(file.exists());
        }
    });
}

void SyncWorker::initData()
{
    getUserDeepinidInfo();
    licenseStateChangeSlot();
    refreshSwitcherDump();
    m_model->setUserinfo(m_deepinId_inter->userInfo());
    //
    getRSAPubKey();
}

void SyncWorker::activate()
{
    m_syncInter->setDBusBlockSignals(false);
    m_deepinId_inter->setDBusBlockSignals(false);

    onStateChanged(m_syncInter->state());

    onLastSyncTimeChanged(m_syncInter->lastSyncTime());
}

void SyncWorker::deactivate()
{
    m_syncInter->setDBusBlockSignals(true);
    m_deepinId_inter->setDBusBlockSignals(true);
}

void SyncWorker::refreshSwitcherDump()
{
    QList<QVariant> argumentList;
    m_utcloudInter->callWithCallback(QStringLiteral("SwitcherDump"), argumentList, this, SIGNAL(pendingCallWatcherFinished(QString)));
}

void SyncWorker::setSync(std::pair<SyncType, bool> state)
{
    // TODO(justforlxz): Maybe will add screensaver in the future
    // you don't need a multimap.
    const std::list<std::pair<SyncType, QStringList>> map { m_model->moduleMap() };
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (it->first == state.first) {
            for (const QString &value : it->second) {
                m_syncInter->SwitcherSet(value, state.second);
            }
        }
    }
}

void SyncWorker::setSyncState(const QString &syncType, bool state)
{
    qDebug() << " worker_sync Info : " << syncType << state;
    m_syncInter->SwitcherSet(syncType, state);
}

void SyncWorker::setUtcloudState(const QString &utcloudType, bool state)
{
    qDebug() << "setUtcloudState(const QString &utcloudType, bool state) " << utcloudType << state;
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(utcloudType) << QVariant::fromValue(state);
    m_utcloudInter->asyncCallWithArgumentList(QStringLiteral("SwitcherSet"), argumentList);
}

void SyncWorker::loginUser()
{
    m_deepinId_inter->Login();
}

void SyncWorker::logoutUser()
{
    m_deepinId_inter->Logout();
}

void SyncWorker::setAutoSync(bool autoSync)
{
    m_syncInter->SwitcherSet("enabled", autoSync);
}

void SyncWorker::onSyncModuleStateChanged(const QString &module, bool enable)
{
    if (module == "enabled") {
        return m_model->setEnableSync(enable);
    }

    const std::list<std::pair<SyncType, QStringList>> list = m_model->moduleMap();
    for (auto it = list.cbegin(); it != list.cend(); ++it) {
        if (it->second.contains(module)) {
            m_model->setModuleSyncState(it->first, enable);
            break;
        }
    }
}

void SyncWorker::onStateChanged(const IntString &state)
{
    std::pair<qint32, QString> value(state.state, state.description);
    if (SyncModel::isSyncStateValid(value)) {
        qDebug() << "activate: " << state.description;
        m_model->setSyncState(value);
    }
}

void SyncWorker::onLastSyncTimeChanged(qlonglong lastSyncTime)
{
    qDebug() << "lastSyncTime: " << lastSyncTime;
    m_model->setLastSyncTime(lastSyncTime);
}

void SyncWorker::licenseStateChangeSlot()
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, watcher,
            &QFutureWatcher<void>::deleteLater);

    QFuture<void> future = QtConcurrent::run(this, &SyncWorker::getLicenseState);
    watcher->setFuture(future);
}

void SyncWorker::getUOSID()
{
    if (!m_syncHelperInter->isValid()) {
        qWarning() << "syncHelper interface invalid: (getUOSID)" << m_syncHelperInter->lastError().message();
        return;
    }
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this, [=] {
        qDebug() << " getUOSID: " << watcher->result();
        m_model->setUOSID(watcher->result());
        watcher->deleteLater();
    });
    QFuture<QString> future = QtConcurrent::run( [=]() -> QString{
        QDBusReply<QString> retUOSID = m_syncHelperInter->call("UOSID");
        if (retUOSID.value().isEmpty()) {
            qWarning() << "UOSID failed:" << retUOSID.error().message();
        }
        return retUOSID.value();
    });
    watcher->setFuture(future);
}

void SyncWorker::getUUID()
{
    QDBusInterface accountsInter("org.deepin.dde.Accounts1",
                                 QString("/org/deepin/dde/Accounts1/User%1").arg(getuid()),
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());
    if (!accountsInter.isValid()) {
        qWarning() << "accounts interface invalid: (getUUID)" << accountsInter.lastError().message();
        return;
    }

    QDBusPendingCall call = accountsInter.asyncCall("Get", "org.deepin.dde.Accounts.User" ,"UUID");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, call, watcher] {
        if (!call.isError()) {
            QDBusReply<QDBusVariant> reply = call.reply();
            qDebug() << " getUUID: " << reply.value().variant().toString();
            m_model->setUUID(reply.value().variant().toString());

        } else {
            qWarning() << "Failed to get driver info: " << call.error().message();
        }
        watcher->deleteLater();
    });
}

void SyncWorker::getHostName()
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, [=]{
        watcher->deleteLater();
    });

    QFuture<void> future = QtConcurrent::run([=]{
        QDBusInterface hostnameInter("org.freedesktop.hostname1",
                                     "/org/freedesktop/hostname1",
                                     "org.freedesktop.hostname1",
                                     QDBusConnection::systemBus());
        m_model->setHostName(hostnameInter.property("StaticHostname").toString());
    });
    watcher->setFuture(future);
}

void SyncWorker::getRSAPubKey()
{
    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusPendingCall call = deepinIf.asyncCall("GetRSAKey");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [=](QDBusPendingCallWatcher *self){
        QDBusPendingReply<QString> reply = *self;
        if(reply.isError()) {
            qDebug() << "get rsa key error:" << reply.error();
        }
        else {
            m_RSApubkey = reply.value().toStdString();
        }
        self->deleteLater();
    });
}

void SyncWorker::getDeviceList()
{
    getTrustDeviceList();
}

void SyncWorker::clearData()
{
    QDBusInterface deepinIf(SyncService, UtcloudPath, UtcloudInterface, QDBusConnection::sessionBus());
    QDBusReply<void> reply = deepinIf.asyncCall("Empty");
    if(!reply.isValid())
    {
        qWarning() << "clear cloud data error:" << reply.error();
    }
    else {
        qDebug() << "clear cloud data success";
    }
}

void SyncWorker::unBindPlatform()
{
    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<void> reply = deepinIf.asyncCall("UnBindPlatform", "wechat");
    if(!reply.isValid())
    {
        qWarning() << "unbind platform error:" << reply.error();
    }
}

void SyncWorker::refreshUserInfo()
{
    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<void> reply = deepinIf.asyncCall("FlushUseInfo");
    if(!reply.isValid())
    {
        qWarning() << "refresh user info error:" << reply.error();
    }
}

void SyncWorker::refreshSyncInfo()
{
    refreshSyncState();   // sync.Daemon
    refreshSwitcherDump();  // utcloud.Daemon
}

void SyncWorker::openForgetPasswd(const QString &url)
{
    QDBusInterface clientIf(DeepinClientSv, DeepinClientPath, DeepinClientIf, QDBusConnection::sessionBus());
    QDBusReply<void> reply = clientIf.asyncCall("LoadPage", url);
    if(!reply.isValid())
    {
        qWarning() << "load page error:" << reply.error();
    }
}

void SyncWorker::registerPasswd(const QString &strpwd)
{
    qDebug() << Q_FUNC_INFO << strpwd;
    QByteArray encryptPwd;
    if(Cryptor::RSAPublicEncryptData(m_RSApubkey, strpwd, encryptPwd))
    {
        QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
        QDBusReply<void> reply = deepinIf.call("SetPassword", QString::fromLocal8Bit(encryptPwd.toBase64()));
        if(!reply.isValid())
        {
            qWarning() << "set password error:" << QDBusError::errorString(reply.error().type()) << reply.error();
        }
    }
    else
    {
        qWarning() << "encrypt password failed";
    }
}

bool SyncWorker::checkPasswdEmpty(bool &isEmpty)
{
    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<QString> reply = deepinIf.call("MeteInfo");
    if(reply.isValid())
    {
        QString data = reply.value();
        //qInfo() << "mete info:" << data;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        isEmpty = jsonObj["passwordEmpty"].toBool();
        return true;
    }
    else
    {
        qWarning() << "get mete info error:" << reply.error();
        return false;
    }
}

QString SyncWorker::getSessionID()
{
    QString strsession;
    QDBusInterface deepinIdIf(DeepinIDService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<QByteArray> reply = deepinIdIf.call("Get");
    if(reply.isValid()) {
        auto sessionJson = reply.value();
        auto doc = QJsonDocument::fromJson(sessionJson);
        auto session = doc.object();
        strsession = session.value("SessionID").toString();
    }
    else {
        qWarning() << "get session id error:" << reply.error();
    }

    return strsession;
}

void SyncWorker::getTrustDeviceList(int pageIndex, int pageSize)
{
    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusPendingCall pendCall = deepinIf.asyncCall("DeviceList", pageIndex, pageSize);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [=](QDBusPendingCallWatcher *watcher){
        QDBusPendingReply<QString> reply = *watcher;
        if(reply.isValid()) {
            QString replyData = reply.value();
            qDebug() << "get device list:" << replyData;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData.toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            int pageSize = jsonObj["page_size"].toInt();
            int total = jsonObj["total"].toInt();
            QJsonArray devArray = jsonObj["list"].toArray();
            this->m_model->addTrustDeviceList(devArray);
            if(total > pageSize) {
                qDebug() << "get more device list";
                this->getTrustDeviceList(pageIndex + 1, pageSize);
            }
        }
        else {
            qWarning() << "get device list error:" << reply.error();
        }

        watcher->deleteLater();
    });
}

void SyncWorker::removeDevice(const QString &devid)
{
    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    deepinIf.asyncCall("DeviceRemove", devid);
}

bool SyncWorker::checkPassword(const QString &passwd, QString &encryptPwd, int &count)
{
    QByteArray encryptData;
    if(!Cryptor::RSAPublicEncryptData(m_RSApubkey, passwd, encryptData))
    {
        qInfo() << "encrypt password failed";
        return false;
    }

    encryptPwd = encryptData.toBase64();
    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<QString> reply = deepinIf.call("Checkpwd", QString::fromLocal8Bit(encryptData.toBase64()));
    if(reply.isValid())
    {
        m_pwdToken = reply.value();
        //qDebug() << "password token:" << m_pwdToken;
        return true;
    }
    else
    {
        QString errmsg = reply.error().message();
        qDebug() << "check password error:" << errmsg;
        if(errmsg.contains(SurPlusError)) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(errmsg.toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            QJsonObject dataObj = jsonObj["data"].toObject();
            count = dataObj["surplus_count"].toInt();
        }
        return false;
    }
}

bool SyncWorker::resetPassword(const QString &oldpwd, const QString &newpwd)
{
    QByteArray newEncrypt;
    if(!Cryptor::RSAPublicEncryptData(m_RSApubkey, newpwd, newEncrypt))
    {
        qInfo() << "encrypt password failed";
        return false;
    }

    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<void> reply = deepinIf.call("ResetPassword", oldpwd, QString::fromLocal8Bit(newEncrypt.toBase64()));
    if(reply.isValid())
    {
        return true;
    }
    else
    {
        qWarning() << "reset password error:" << reply.error();
        return false;
    }
}

int SyncWorker::sendVerifyCode(const QString &straddr)
{
    QByteArray encryptAddr;
    if(!Cryptor::RSAPublicEncryptData(m_RSApubkey, straddr, encryptAddr))
    {
        qInfo() << "encrypt password failed";
        return -1;
    }

    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<QString> reply = deepinIf.call("SendCode", QString::fromLocal8Bit(encryptAddr.toBase64()));
    if(reply.isValid())
    {
        QString codeStr = reply.value();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(codeStr.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        qDebug() << "get code object:" << codeStr;
        return jsonObj["cd"].toInt();
    }
    else
    {
        qWarning() << "send verify code, " << straddr << " error:" << reply.error();
        return -1;
    }
}

bool SyncWorker::updatePhoneEmail(const QString &phonemail, const QString &verifyCode, QString &rebindAccount, QString &rebindKey)
{
    QByteArray encryptPM;
    QByteArray encryptVC;
    if(!Cryptor::RSAPublicEncryptData(m_RSApubkey, phonemail, encryptPM))
    {
        qWarning() << "rsa encrypt phonemail failed";
        return -2;
    }

    if(!Cryptor::RSAPublicEncryptData(m_RSApubkey, verifyCode, encryptVC))
    {
        qWarning() << "rsa encrypt verify code failed";
        return -2;
    }

    QDBusInterface deepinIf(SyncService, DeepinIDPath, DeepinIDInterface, QDBusConnection::sessionBus());
    QDBusReply<QString> reply = deepinIf.call("UpdatePhoneEmail", QString::fromLocal8Bit(encryptPM.toBase64()),
                                           QString::fromLocal8Bit(encryptVC.toBase64()), m_pwdToken, rebindKey);
    if(reply.isValid())
    {
        return true;
    }
    else
    {
        QString errMsg = reply.error().message();
        qWarning() << "update phoneemail error:" << errMsg;
        if(errMsg.contains(RebindError)) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(errMsg.toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            QJsonObject dataObj = jsonObj["data"].toObject();
            rebindKey = dataObj["bind_key"].toString();
            rebindAccount = dataObj["show_user_name"].toString();
            qInfo() << rebindKey << rebindAccount;
            return false;
        }

        rebindKey.clear();
        rebindAccount.clear();
        return false;
    }
}

void SyncWorker::asyncLocalBindCheck(const QString &uuid)
{
    qDebug() << "user Bind uuid" << uuid;
    QFutureWatcher<BindCheckResult> *watcher = new QFutureWatcher<BindCheckResult>(this);
    connect(watcher, &QFutureWatcher<BindCheckResult>::finished, [this, watcher] {
        BindCheckResult result = watcher->result();
        if (result.error.isEmpty()) {
            qDebug() << "user Bind " << result.ubid;
            m_model->setBindLocalUBid(result.ubid);
        }
        else {
            m_model->setResetPasswdError(result.error);
        }
        watcher->deleteLater();
    });
    QFuture<BindCheckResult> future = QtConcurrent::run(this, &SyncWorker::checkLocalBind, uuid);
    watcher->setFuture(future);
}


void SyncWorker::asyncBindAccount(const QString &uuid, const QString &hostName)
{
    qDebug() << "Start Bind! uuid: " << uuid << " host name: " << hostName;
    QFutureWatcher<BindCheckResult> *watcher = new QFutureWatcher<BindCheckResult>(this);
    connect(watcher, &QFutureWatcher<BindCheckResult>::finished, [this, watcher] {
        BindCheckResult result = watcher->result();
        m_model->setBindLocalUBid(result.ubid);
        watcher->deleteLater();
    });
    QFuture<BindCheckResult> future = QtConcurrent::run(this, &SyncWorker::bindAccount, uuid, hostName);
    watcher->setFuture(future);
}

void SyncWorker::asyncUnbindAccount(const QString &ubid)
{
    qDebug() << "Start UnBind!";
    QFutureWatcher<BindCheckResult> *watcher = new QFutureWatcher<BindCheckResult>(this);
    connect(watcher, &QFutureWatcher<BindCheckResult>::finished, [this, watcher] {
        BindCheckResult result = watcher->result();
        if (result.error.isEmpty()) {
            m_model->setBindLocalUBid(QString());
        }
        else {
            m_model->setResetPasswdError(result.error);
        }
        watcher->deleteLater();
    });
    QFuture<BindCheckResult> future = QtConcurrent::run(this, &SyncWorker::unBindAccount, ubid);
    watcher->setFuture(future);
}

void SyncWorker::asyncSetFullname(const QString &fullname)
{
    QDBusInterface utInterface("com.deepin.sync.Daemon",
                               "/com/deepin/utcloud/Daemon",
                               "com.deepin.utcloud.Daemon"
                               ,QDBusConnection::sessionBus());
    QDBusReply<bool> retFullName = utInterface.call("SetNickname", fullname);
    if (retFullName.isValid()) {
        qDebug() << "SetNicknameSuccess";
    } else {
        qWarning() << "Bind fullname failed:" << retFullName.error().message();
        m_model->setResetUserNameError(retFullName.error().message());
    }
}

void SyncWorker::onSetFullname(const QString &fullname)
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
    QFuture<void> future = QtConcurrent::run(this, &SyncWorker::asyncSetFullname, fullname);
    watcher->setFuture(future);
}

void SyncWorker::onPullMessage()
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    watcher->setFuture(QtConcurrent::run(this, &SyncWorker::futurePullMessage));
}

void SyncWorker::futurePullMessage()
{
    QDBusPendingReply<QString> retTime = DDBusSender()
                                         .service("com.deepin.sync.Daemon")
                                         .interface("com.deepin.utcloud.Daemon")
                                         .path("/com/deepin/utcloud/Daemon")
                                         .method("PullMessage")
                                         .call();
    if (!retTime.error().message().isEmpty())
        qDebug() << " message value: " << retTime.value();
}

// 解析
void SyncWorker::callWatcherResult(const QString &result)
{
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    QJsonObject jobj = doc.object();
    m_model->setEnableSync(jobj["enabled"].toBool());

    QJsonObject apps = jobj["apps"].toObject();

    QList<Apps*> applist;
    for (auto key : apps.keys()) {
        QJsonObject jAppsDate = apps[key].toObject();

        const QString name =  jAppsDate["name"].toString();
        const QString displayName = jAppsDate["display_name"].toString();
        bool enable = jAppsDate["enable"].toBool();
        const QString icon = jAppsDate["icon"].toString();

        qDebug() << " date ==> " << name << enable << icon;
        Apps *apps = new Apps;
        apps->setKey(key);
        apps->setName(displayName.isEmpty() ? name : displayName);
        apps->setEnable(enable);
        apps->setIconPath(icon);
        m_model->setUtcloudSwitcherState(key, enable);
        applist.append(apps);
    }

    m_model->addSyncItem(applist);
}

void SyncWorker::refreshSyncState()
{
    QFutureWatcher<QJsonObject> *watcher = new QFutureWatcher<QJsonObject>(this);
    connect(watcher, &QFutureWatcher<QJsonObject>::finished, this, [=] {
        QJsonObject obj = watcher->result();
        qDebug() << "OBJ: " << obj;
        if (obj.isEmpty() || !obj["enabled"].toBool()) {
            qDebug() << "Sync Info is Wrong!";
            return;
        }

        const std::list<std::pair<SyncType, QStringList>> moduleMap{
            m_model->moduleMap()
        };
        for (auto it = moduleMap.cbegin(); it != moduleMap.cend(); ++it) {
            m_model->setModuleSyncState(it->first, obj[it->second.first()].toBool());
        }
        watcher->deleteLater();
    });

    QFuture<QJsonObject> future = QtConcurrent::run([=]() -> QJsonObject {
        QDBusPendingReply<QString> reply = m_syncInter->SwitcherDump();
        reply.waitForFinished();
        return QJsonDocument::fromJson(reply.value().toUtf8()).object();
    });

    watcher->setFuture(future);
}

void SyncWorker::utcloudSwitcherChange(QVariantList data)
{
    QString utcloudPath = data.at(0).toString();
    bool state = data.at(1).toBool();
    m_model->setUtcloudSwitcherState(utcloudPath, state);
}

void SyncWorker::utcloudLoginStatus(QVariantList data)
{
    int32_t state = data.at(0).toInt();
    // TODO: 目前只处理一中登录情况， 后续增加使用enum定义  4: 登录完成数据同步
    if (state == 4)
        refreshSwitcherDump();
}

void SyncWorker::getUserDeepinidInfo()
{
    getUOSID();
    getUUID();
    getHostName();
}

void SyncWorker::getLicenseState()
{
    if (DSysInfo::uosEditionType() == DSysInfo::UosCommunity) {
        m_model->setActivation(true);
        return;
    }

    QDBusInterface licenseInfo("com.deepin.license",
                               "/com/deepin/license/Info",
                               "com.deepin.license.Info",
                               QDBusConnection::systemBus());

    if (!licenseInfo.isValid()) {
        qWarning()<< "com.deepin.license error ,"<< licenseInfo.lastError().name();
        return;
    }

    quint32 reply = licenseInfo.property("AuthorizationState").toUInt();
    qDebug() << "authorize result:" << reply;
    m_model->setActivation(reply >= 1 && reply <= 3);
}

BindCheckResult SyncWorker::logout(const QString &ubid)
{
    BindCheckResult result = unBindAccount(ubid);
    m_deepinId_inter->Logout();
    return result;
}

BindCheckResult SyncWorker::checkLocalBind(const QString &uuid)
{
    BindCheckResult result;
    QDBusReply<QString> retLocalBindCheck= m_deepinId_inter->LocalBindCheck(uuid);
    if (!m_syncHelperInter->isValid()) {
        qWarning() << "syncHelper interface invalid: (localBindCheck)" << m_syncHelperInter->lastError().message();
        return result;
    }
    if (retLocalBindCheck.error().message().isEmpty()) {
        result.ubid = retLocalBindCheck.value();
    } else {
        qWarning() << "localBindCheck failed:" << retLocalBindCheck.error().message();
        result.error = retLocalBindCheck.error().message();
    }
    return result;
}

BindCheckResult SyncWorker::bindAccount(const QString &uuid, const QString &hostName)
{
    BindCheckResult result;
    QDBusPendingReply<QString> retUBID = DDBusSender()
                                         .service("com.deepin.deepinid")
                                         .interface("com.deepin.deepinid")
                                         .path("/com/deepin/deepinid")
                                         .method("BindLocalUUid").arg(uuid).arg(hostName)
                                         .call();
    retUBID.waitForFinished();
    if (retUBID.error().message().isEmpty()) {
        qDebug() << "Bind success!";
        result.ubid = retUBID.value();
    } else {
        qWarning() << "Bind failed:" << retUBID.error().message();
        result.error = retUBID.error().message();
    }
    return result;
}

BindCheckResult SyncWorker::unBindAccount(const QString &ubid)
{
    BindCheckResult result;
    QDBusPendingReply<QString> retUnBoundle = DDBusSender()
                                              .service("com.deepin.deepinid")
                                              .interface("com.deepin.deepinid")
                                              .path("/com/deepin/deepinid")
                                              .method("UnBindLocalUUid").arg(ubid)
                                              .call();
    retUnBoundle.waitForFinished();
    if (retUnBoundle.error().message().isEmpty()) {
        qDebug() << "unBind success!";
        result.ret = true;
    } else {
        qWarning() << "unBind failed:" << retUnBoundle.error().message();
        result.error = retUnBoundle.error().message();
        result.ret = false;
    }
    return result;
}
