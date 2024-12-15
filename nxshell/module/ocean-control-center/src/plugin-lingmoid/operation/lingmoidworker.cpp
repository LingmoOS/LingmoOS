// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmoidworker.h"
#include "utils.h"

#include <DDBusSender>
#include <lsysinfo.h>

#include <QUrl>
#include <QDesktopServices>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

DCORE_USE_NAMESPACE

LingmoWorker::LingmoWorker(LingmoidModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_lingmoIDProxy(new LingmoidDBusProxy(this))
    , m_syncProxy(new SyncDBusProxy(this))
    , m_utcloudInter(new QDBusInterface(SYNC_SERVICE, UTCLOUD_PATH, UTCLOUD_INTERFACE, QDBusConnection::sessionBus(), this))
{

    QDBusConnection::systemBus().connect(LICENSE_SERVICE, LICENSE_PATH, LICENSE_INTERFACE, "LicenseStateChange", this, SLOT(licenseStateChangeSlot()));

    connect(m_lingmoIDProxy, &LingmoidDBusProxy::UserInfoChanged, [this](const QVariantMap &userinfo){
        m_model->setUserinfo(userinfo);
    });
}

void LingmoWorker::initData()
{
    licenseStateChangeSlot();
    m_model->setUserinfo(m_lingmoIDProxy->userInfo());
}

void LingmoWorker::loginUser()
{
    m_lingmoIDProxy->login();
}

void LingmoWorker::logoutUser()
{
    m_lingmoIDProxy->logout();
}

void LingmoWorker::openWeb()
{
    QString url = loadCodeURL();
    QUrl::toPercentEncoding(url);
    QDesktopServices::openUrl(QUrl(url));
}

void LingmoWorker::licenseStateChangeSlot()
{
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    QFuture<void> future = QtConcurrent::run(&LingmoWorker::getLicenseState, this);
    watcher->setFuture(future);
}

QString LingmoWorker::loadCodeURL()
{
    auto func_getToken = [] {
        QDBusPendingReply<QString> retToken = DDBusSender()
                .service("com.lingmo.sync.Daemon")
                .interface("com.lingmo.utcloud.Daemon")
                .path("/com/lingmo/utcloud/Daemon")
                .method("UnionLoginToken")
                .call();
        retToken.waitForFinished();
        QString token = retToken.value();
        if (token.isEmpty())
            qDebug() << retToken.error().message();
        return token;
    };

    QString oauthURI = "https://login.lingmo.org";

    if (!qEnvironmentVariableIsEmpty("LINGMOID_OAUTH_URI")) {
        oauthURI = qgetenv("LINGMOID_OAUTH_URI");
    }

    QString url = oauthURI += QString("/oauth2/authorize/registerlogin?autoLoginKey=%1").arg(func_getToken());
    return url;
}

void LingmoWorker::getLicenseState()
{
    if (IsCommunitySystem) {
        m_model->setActivation(true);
        return;
    }

    QDBusInterface licenseInfo(LICENSE_SERVICE, LICENSE_PATH, LICENSE_INTERFACE, QDBusConnection::systemBus());
    if (!licenseInfo.isValid()) {
        qWarning()<< "com.lingmo.license error ,"<< licenseInfo.lastError().name();
        return;
    }

    quint32 reply = licenseInfo.property("AuthorizationState").toUInt();
    qDebug() << "authorize result:" << reply;
    m_model->setActivation(reply >= 1 && reply <= 3);
}
