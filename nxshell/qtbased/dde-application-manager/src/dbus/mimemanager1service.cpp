// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/mimemanager1adaptor.h"
#include "applicationmanager1service.h"
#include "applicationservice.h"
#include "constant.h"

MimeManager1Service::MimeManager1Service(ApplicationManager1Service *parent)
    : QObject(parent)
{
    auto *adaptor = new (std::nothrow) MimeManager1Adaptor{this};
    if (adaptor == nullptr or !registerObjectToDBus(this, DDEApplicationManager1MimeManager1ObjectPath, MimeManager1Interface)) {
        std::terminate();
    }
}

MimeManager1Service::~MimeManager1Service() = default;

ObjectMap MimeManager1Service::listApplications(const QString &mimeType) const noexcept
{
    auto type = m_database.mimeTypeForName(mimeType).name();
    if (type.isEmpty()) {
        qInfo() << "try to query raw type:" << mimeType;
        type = mimeType;
    }

    QStringList appIds;

    for (auto it = m_infos.rbegin(); it != m_infos.rend(); ++it) {
        const auto &info = it->cacheInfo();
        if (!info) {
            continue;
        }
        auto apps = info->queryApps(type);
        appIds.append(std::move(apps));
    }
    appIds.removeDuplicates();
    qInfo() << "query" << mimeType << "find:" << appIds;
    const auto &apps = dynamic_cast<ApplicationManager1Service *>(parent())->findApplicationsByIds(appIds);
    return dumpDBusObject(apps);
}

QString MimeManager1Service::queryDefaultApplication(const QString &content, QDBusObjectPath &application) const noexcept
{
    QMimeType mime;
    QFileInfo info{content};
    application = QDBusObjectPath{"/"};

    if (info.isAbsolute() and info.exists()) {
        mime = m_database.mimeTypeForFile(content);
    } else {
        mime = m_database.mimeTypeForName(content);
    }

    auto type = mime.name();

    if (type.isEmpty()) {
        qInfo() << "try to query raw content:" << content;
        type = content;
    }

    QString defaultAppId;
    for (auto it1 = m_infos.rbegin(); it1 != m_infos.rend(); ++it1) {
        const auto &list = it1->appsList();
        for (auto it2 = list.rbegin(); it2 != list.rend(); ++it2) {
            if (auto app = it2->queryDefaultApp(type); !app.isEmpty()) {
                defaultAppId = app;
            }
        }
    }

    if (defaultAppId.isEmpty()) {
        qInfo() << "file's mimeType:" << mime.name() << "but can't find a default application for this type.";
        return type;
    }

    const auto &apps = dynamic_cast<ApplicationManager1Service *>(parent())->findApplicationsByIds({defaultAppId});
    if (apps.isEmpty()) {
        qWarning() << "default application has been found:" << defaultAppId
                   << " but we can't find corresponding application in ApplicationManagerService.";
    } else {
        application = apps.keys().first();
    }

    return type;
}

void MimeManager1Service::setDefaultApplication(const QStringMap &defaultApps) noexcept
{
    auto &app = m_infos.front().appsList();

    if (app.empty()) {
        safe_sendErrorReply(QDBusError::InternalError);
        return;
    }

    auto userConfig = std::find_if(
        app.begin(), app.end(), [](const MimeApps &config) { return !config.isDesktopSpecific(); });  // always find this

    if (userConfig == app.end()) {
        qWarning() << "couldn't find user mimeApps";
        safe_sendErrorReply(QDBusError::InternalError);
        return;
    }

    for (auto it = defaultApps.constKeyValueBegin(); it != defaultApps.constKeyValueEnd(); ++it) {
        userConfig->setDefaultApplication(it->first, it->second);
    }

    if (!userConfig->writeToFile()) {
        safe_sendErrorReply(QDBusError::Failed, "set default app failed, these config will be reset after re-login.");
    }
}

void MimeManager1Service::unsetDefaultApplication(const QStringList &mimeTypes) noexcept
{
    auto &app = m_infos.front().appsList();
    auto userConfig = std::find_if(app.begin(), app.end(), [](const MimeApps &config) { return !config.isDesktopSpecific(); });

    if (userConfig == app.end()) {
        qWarning() << "couldn't find user mimeApps";
        safe_sendErrorReply(QDBusError::InternalError);
        return;
    }

    for (const auto &mime : mimeTypes) {
        userConfig->unsetDefaultApplication(mime);
    }

    if (!userConfig->writeToFile()) {
        safe_sendErrorReply(QDBusError::Failed, "unset default app failed, these config will be reset after re-login.");
    }
}

void MimeManager1Service::appendMimeInfo(MimeInfo &&info)
{
    m_infos.emplace_back(std::move(info));
}

void MimeManager1Service::reset() noexcept
{
    m_infos.clear();
}
