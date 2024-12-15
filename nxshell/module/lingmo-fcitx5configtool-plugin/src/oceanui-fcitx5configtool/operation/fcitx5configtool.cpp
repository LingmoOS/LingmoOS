// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fcitx5configtool.h"
#include "private/fcitx5configtool_p.h"
#include "fcitx5configproxy.h"
#include "fcitx5addonsproxy.h"

#include <dbusprovider.h>
#include <imconfig.h>
#include <configwidget.h>

#include <ocean-control-center/oceanuifactory.h>

#include <fcitxqtdbustypes.h>

#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QtQml/QQmlEngine>

#include <DDBusSender>
#include <DUtil>
#include <QFileInfo>

namespace lingmo {
namespace fcitx5configtool {

static QString kFcitxConfigGlobalPath = "fcitx://config/global";
static const QString kSogouAddonUniqueName = "com.sogou.ime.ng.fcitx5.lingmo-addon";
static const QString kSogouIMUniqueName = "com.sogou.ime.ng.fcitx5.lingmo";
static const QString kSogouConfigureAppId = "com.sogou.ime.ng.fcitx5.lingmo.configurer";

Fcitx5ConfigToolWorkerPrivate::Fcitx5ConfigToolWorkerPrivate(Fcitx5ConfigToolWorker *parent)
    : QObject(parent), q(parent)
{
}

void Fcitx5ConfigToolWorkerPrivate::init()
{
    dbusProvider = new fcitx::kcm::DBusProvider(this);
    imConfig = new fcitx::kcm::IMConfig(dbusProvider, fcitx::kcm::IMConfig::Flatten, this);
    configProxy = new Fcitx5ConfigProxy(dbusProvider, kFcitxConfigGlobalPath, this);
    addonsProxy = new Fcitx5AddonsProxy(dbusProvider, this);
    imListModel = new IMListModel(this);
    imListModel->resetData(imConfig->currentIMModel());

    initConnect();
}

void Fcitx5ConfigToolWorkerPrivate::initConnect()
{
    connect(dbusProvider, &fcitx::kcm::DBusProvider::availabilityChanged, this, [this](bool avail) {
        qInfo() << "Availability changed:" << avail;
        if (avail) {
            configProxy->requestConfig(false);
            addonsProxy->load();
        } else {
            configProxy->clear();
            addonsProxy->clear();
        }
    });
    connect(imConfig, &fcitx::kcm::IMConfig::imListChanged, this, [=]() {
        qInfo() << "list changed:" << imConfig->currentIMModel()->rowCount();
        imListModel->resetData(imConfig->currentIMModel());
    });

    connect(imListModel, &IMListModel::requestRemove, this, [this](int index) {
        qInfo() << "Remove IM:" << index;
        imConfig->removeIM(index);
        imConfig->save();
    });

    connect(imListModel, &IMListModel::requestMove, this, [this](int from, int to) {
        qInfo() << "Move IM from" << from << to;
        imConfig->move(from, to);
        imConfig->save();
    });

    connect(configProxy, &Fcitx5ConfigProxy::requestConfigFinished, q, &Fcitx5ConfigToolWorker::requestConfigFinished);
    connect(addonsProxy, &Fcitx5AddonsProxy::requestAddonsFinished, q, &Fcitx5ConfigToolWorker::requestAddonsFinished);
    configProxy->requestConfig(false);
    addonsProxy->load();
}

Fcitx5ConfigToolWorker::Fcitx5ConfigToolWorker(QObject *parent)
    : QObject(parent), d(new Fcitx5ConfigToolWorkerPrivate(this))
{
    qmlRegisterType<Fcitx5ConfigProxy>("org.lingmo.oceanui.fcitx5configtool", 1, 0, "Fcitx5ConfigProxy");
    qmlRegisterType<Fcitx5AddonsProxy>("org.lingmo.oceanui.fcitx5configtool", 1, 0, "Fcitx5AddonsProxy");
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void Fcitx5ConfigToolWorker::init()
{
    d->init();
}

Fcitx5ConfigProxy *Fcitx5ConfigToolWorker::fcitx5ConfigProxy() const
{
    return d->configProxy;
}

Fcitx5AddonsProxy *Fcitx5ConfigToolWorker::fcitx5AddonsProxy() const
{
    return d->addonsProxy;
}

void Fcitx5ConfigToolWorker::showAddonSettingsDialog(const QString &addonStr, const QString &title) const
{
    // If addonStr is empty or addonsProxy is null, don't show the dialog
    if (addonStr.isEmpty() || !d->addonsProxy) {
        return;
    }

    if (addonStr == kSogouAddonUniqueName) {
        DDBusSender()
                .service("org.desktopspec.ApplicationManager1")
                .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath(kSogouConfigureAppId))
                .interface("org.desktopspec.ApplicationManager1.Application")
                .method("Launch")
                .arg(QString(""))
                .arg(QStringList())
                .arg(QVariantMap())
                .call();
        return;
    }

    launchConfigDialog(QString("fcitx://config/addon/%1").arg(addonStr),
                       title);
}

void Fcitx5ConfigToolWorker::openLingmoAppStore() const
{
    DDBusSender()
            .service("org.desktopspec.ApplicationManager1")
            .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath("lingmo-app-store"))
            .interface("org.desktopspec.ApplicationManager1.Application")
            .method("Launch")
            .arg(QString(""))
            .arg(QStringList())
            .arg(QVariantMap())
            .call();
}

void Fcitx5ConfigToolWorker::showIMSettingsDialog(int index) const
{
    if (!d->imConfig)
        return;

    auto item = d->imConfig->currentIMModel()->index(index);
    QString uniqueName = item.data(fcitx::kcm::FcitxIMUniqueNameRole).toString();

    if (uniqueName == kSogouIMUniqueName) {
        DDBusSender()
                .service("org.desktopspec.ApplicationManager1")
                .path(QStringLiteral("/org/desktopspec/ApplicationManager1/") + DUtil::escapeToObjectPath(kSogouConfigureAppId))
                .interface("org.desktopspec.ApplicationManager1.Application")
                .method("Launch")
                .arg(QString(""))
                .arg(QStringList())
                .arg(QVariantMap())
                .call();
        return;
    }

    launchConfigDialog(QString("fcitx://config/inputmethod/%1").arg(uniqueName),
                       item.data(Qt::DisplayRole).toString());
}

void Fcitx5ConfigToolWorker::addIM(int index)
{
    d->imConfig->addIM(index);
    d->imConfig->save();
}

void Fcitx5ConfigToolWorker::launchConfigDialog(const QString &uri, const QString &title) const
{
    QString execPath = QString::fromLocal8Bit(DCC_CONFIGTOOL_EXEC_PATH) + "/oceanui-fcitx5configtool-exec";
    QFileInfo fileInfo(execPath);
    if (!fileInfo.exists())
        execPath = "/usr/libexec/oceanui-fcitx5configtool-exec";

    QStringList args;
    args << "-u" << uri
         << "-t" << title;
    qInfo() << "exec: " << execPath << "args:" << args;
    QProcess::startDetached(execPath, args);
}

IMListModel *Fcitx5ConfigToolWorker::imlistModel() const
{
    Q_ASSERT(d->imListModel);
    return d->imListModel;
}

fcitx::kcm::IMProxyModel *Fcitx5ConfigToolWorker::imProxyModel() const
{
    return d->imConfig->availIMModel();
}

DCC_FACTORY_CLASS(Fcitx5ConfigToolWorker)
}   // namespace fcitx5configtool
}   // namespace lingmo

#include "fcitx5configtool.moc"
