// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transferhelper.h"
#include "transferhelper_p.h"

#include "common/constant.h"
#include "common/commonutils.h"
#include "discover/deviceinfo.h"
#include "gui/mainwindow.h"

#include <CuteIPCInterface.h>

#include <QDesktopServices>
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTime>
#include <QProcess>
#include <QJsonDocument>

using ButtonStateCallback = std::function<bool(const QString &, const DeviceInfoPointer)>;
using ClickedCallback = std::function<void(const QString &, const DeviceInfoPointer)>;
Q_DECLARE_METATYPE(ButtonStateCallback)
Q_DECLARE_METATYPE(ClickedCallback)

inline constexpr char TransferButtonId[] { "transfer-button" };

inline constexpr char NotifyCancelAction[] { "cancel" };
inline constexpr char NotifyRejectAction[] { "reject" };
inline constexpr char NotifyAcceptAction[] { "accept" };
inline constexpr char NotifyCloseAction[] { "close" };
inline constexpr char NotifyViewAction[] { "view" };

#ifdef linux
inline constexpr char Ksend[] { "send" };
#else
inline constexpr char Ksend[] { ":/icons/deepin/builtin/texts/send_18px.svg" };
#endif

using namespace deepin_cross;
using namespace cooperation_transfer;
using namespace cooperation_core;

TransferHelperPrivate::TransferHelperPrivate(TransferHelper *qq)
    : QObject(qq),
      q(qq)
{
    ipcInterface = new CuteIPCInterface();

    backendOk = ipcInterface->connectToServer("dde-cooperation");
    if (backendOk) {
        DLOG << "success connect to: dde-cooperation";
    } else {
        WLOG << "can not connect to: dde-cooperation";
    }
}

TransferHelperPrivate::~TransferHelperPrivate()
{
}


TransferHelper::TransferHelper(QObject *parent)
    : QObject(parent),
      d(new TransferHelperPrivate(this))
{
    if (d->backendOk) {
        // bind SIGNAL to SLOT
        d->ipcInterface->remoteConnect(SIGNAL(searched(QString)), this, SLOT(searchResultSlot(QString)));
        d->ipcInterface->remoteConnect(SIGNAL(refreshed(QStringList)), this, SLOT(refreshResultSlot(QStringList)));
        d->ipcInterface->remoteConnect(SIGNAL(deviceChanged(bool, QString)), this, SLOT(deviceChangedSlot(bool, QString)));

        d->ipcInterface->remoteConnect(this, SIGNAL(refresh()), SLOT(onRefreshDevice()));
        d->ipcInterface->remoteConnect(this, SIGNAL(search(QString)), SLOT(onSearchDevice(QString)));
        d->ipcInterface->remoteConnect(this, SIGNAL(sendFiles(QString, QString, QStringList)), SLOT(onSendFiles(QString, QString, QStringList)));

        // frist, refresh & get device list
        Q_EMIT refresh();
    }
}

TransferHelper::~TransferHelper()
{
}

TransferHelper *TransferHelper::instance()
{
    static TransferHelper ins;
    return &ins;
}

void TransferHelper::registBtn(cooperation_core::MainWindow *window)
{
    ClickedCallback clickedCb = TransferHelper::buttonClicked;
    ButtonStateCallback visibleCb = TransferHelper::buttonVisible;
    ButtonStateCallback clickableCb = TransferHelper::buttonClickable;

    QVariantMap transferInfo { { "id", TransferButtonId },
                               { "description", tr("Send files") },
                               { "icon-name", Ksend },
                               { "location", 3 },
                               { "button-style", 1 },
                               { "clicked-callback", QVariant::fromValue(clickedCb) },
                               { "visible-callback", QVariant::fromValue(visibleCb) },
                               { "clickable-callback", QVariant::fromValue(clickableCb) } };

    window->onRegistOperations(transferInfo);
}

void TransferHelper::buttonClicked(const QString &id, const DeviceInfoPointer info)
{
    QString ip = info->ipAddress();
    QString name = info->deviceName();
    LOG << "button clicked, button id: " << id.toStdString()
        << " ip: " << ip.toStdString()
        << " device name: " << name.toStdString();

    if (id == TransferButtonId) {
        QStringList selectedFiles = qApp->property("sendFiles").toStringList();
        if (selectedFiles.isEmpty())
            selectedFiles = QFileDialog::getOpenFileNames(qApp->activeWindow());

        if (selectedFiles.isEmpty())
            return;

        // send command to local socket.
        Q_EMIT TransferHelper::instance()->sendFiles(ip, name, selectedFiles);
        qApp->exit(0);
    }
}

bool TransferHelper::buttonVisible(const QString &id, const DeviceInfoPointer info)
{
    if (id == TransferButtonId) {
        switch (info->transMode()) {
        case DeviceInfo::TransMode::Everyone:
            return info->connectStatus() != DeviceInfo::Offline;
        case DeviceInfo::TransMode::OnlyConnected:
            return info->connectStatus() == DeviceInfo::Connected;
        default:
            return false;
        }
    }

    return true;
}

bool TransferHelper::buttonClickable(const QString &id, const DeviceInfoPointer info)
{
    Q_UNUSED(id)
    Q_UNUSED(info)

    return true;
}

void TransferHelper::searchResultSlot(const QString& info)
{
    if (info.isEmpty()) {
        // From remove onFinishedDiscovery(false)
        Q_EMIT finishDiscovery(false);
        return;
    }

    auto devInfo = parseFromJson(info);
    if (transable(devInfo)) {
        Q_EMIT onlineDevices({ devInfo });
    } else if (devInfo) {
        // filter the invisible device
        auto ip = devInfo->ipAddress();
        Q_EMIT offlineDevice(ip);
    }
}

void TransferHelper::refreshResultSlot(const QStringList& infoList)
{
    QList<DeviceInfoPointer> devList;
    for (auto info : infoList) {
        auto devInfo = parseFromJson(info);
        if (transable(devInfo)) {
            devList << devInfo;
        }
    }

    bool found = !devList.isEmpty();
    if (found)
        Q_EMIT onlineDevices(devList);

    Q_EMIT finishDiscovery(found);
}

void TransferHelper::deviceChangedSlot(bool found, const QString& info)
{
    if (found) {
        searchResultSlot(info);
    } else {
        Q_EMIT offlineDevice(info);
    }
}

DeviceInfoPointer TransferHelper::parseFromJson(const QString &info)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(info.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        WLOG << "parse search info error, info:" << info.toStdString();
        return nullptr;
    }

    auto map = doc.toVariant().toMap();
    auto devInfo = DeviceInfo::fromVariantMap(map);
    devInfo->setConnectStatus(DeviceInfo::Connectable);

    return devInfo;
}

bool TransferHelper::transable(const DeviceInfoPointer devInfo)
{
    if (!devInfo || !devInfo->isValid()) {
        return false;
    }

    if (DeviceInfo::TransMode::Everyone == devInfo->transMode()) {
        return true;
    }

    if (DeviceInfo::TransMode::OnlyConnected == devInfo->transMode() &&
        DeviceInfo::ConnectStatus::Connected == devInfo->connectStatus()) {
        return true;
    }

    return false;
}
