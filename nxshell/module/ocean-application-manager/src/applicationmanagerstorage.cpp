// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "applicationmanagerstorage.h"
#include "constant.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QDir>
#include <memory>

std::shared_ptr<ApplicationManager1Storage>
ApplicationManager1Storage::createApplicationManager1Storage(const QString &storageDir) noexcept
{
    QDir dir;
    auto dirPath = QDir::cleanPath(storageDir);
    if (!dir.mkpath(dirPath)) {
        qCritical() << "can't create directory";
        return nullptr;
    }

    dir.setPath(dirPath);
    auto storagePath = dir.filePath("storage.json");
    auto obj = std::shared_ptr<ApplicationManager1Storage>(new (std::nothrow) ApplicationManager1Storage{storagePath});

    if (!obj) {
        qCritical() << "new ApplicationManager1Storage failed.";
        return nullptr;
    }

    if (!obj->m_file->open(QFile::ReadWrite)) {
        qCritical() << "can't open file:" << storagePath;
        return nullptr;
    }

    auto content = obj->m_file->readAll();
    if (content.isEmpty()) {  // new file
        obj->setVersion(STORAGE_VERSION);
        obj->setFirstLaunch(true);
        return obj;
    }

    // TODO: support migrate from lower storage version.

    QJsonParseError err;
    auto json = QJsonDocument::fromJson(content, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "parse json failed:" << err.errorString() << "clear this file content.";
        obj->m_file->resize(0);
        obj->setVersion(STORAGE_VERSION);
        obj->setFirstLaunch(true);
    } else {
        obj->m_data = json.object();
        obj->setFirstLaunch(false);
    }

    return obj;
}

ApplicationManager1Storage::ApplicationManager1Storage(const QString &storagePath)
    : m_file(std::make_unique<QFile>(storagePath))
{
}

bool ApplicationManager1Storage::writeToFile() const noexcept
{
    if (!m_file) {
        qCritical() << "file is nullptr";
        return false;
    }

    if (!m_file->resize(0)) {
        qCritical() << "failed to clear file:" << m_file->errorString();
        return false;
    }

    auto content = QJsonDocument{m_data}.toJson(QJsonDocument::Compact);
    auto bytes = m_file->write(content, content.size());
    if (bytes != content.size()) {
        qWarning() << "Incomplete file writes:" << m_file->errorString();
    }

    if (!m_file->flush()) {
        qCritical() << "io error, write failed.";
        return false;
    }

    return true;
}

bool ApplicationManager1Storage::setVersion(uint8_t version) noexcept
{
    m_data["version"] = version;
    return writeToFile();
}

uint8_t ApplicationManager1Storage::version() const noexcept
{
    return m_data["version"].toInt(-1);
}

bool ApplicationManager1Storage::setFirstLaunch(bool first) noexcept
{
    m_data["FirstLaunch"] = first;
    return writeToFile();
}

[[nodiscard]] bool ApplicationManager1Storage::firstLaunch() const noexcept
{
    return m_data["FirstLaunch"].toBool(true);
}

bool ApplicationManager1Storage::createApplicationValue(
    const QString &appId, const QString &groupName, const QString &valueKey, const QVariant &value, bool deferCommit) noexcept
{
    if (appId.isEmpty() or groupName.isEmpty() or valueKey.isEmpty()) {
        qWarning() << "unexpected empty string";
        return false;
    }

    QJsonObject appObj;
    if (m_data.contains(appId)) {
        appObj = m_data[appId].toObject();
    }

    QJsonObject groupObj;
    if (appObj.contains(groupName)) {
        groupObj = appObj[groupName].toObject();
    }

    if (groupObj.contains(valueKey)) {
        qInfo() << "value" << valueKey << value << "is already exists.";
        return true;
    }

    groupObj.insert(valueKey, value.toJsonValue());
    appObj.insert(groupName, groupObj);
    m_data.insert(appId, appObj);

    return deferCommit ? true : writeToFile();
}

bool ApplicationManager1Storage::updateApplicationValue(
    const QString &appId, const QString &groupName, const QString &valueKey, const QVariant &value, bool deferCommit) noexcept
{
    if (appId.isEmpty() or groupName.isEmpty() or valueKey.isEmpty()) {
        qWarning() << "unexpected empty string";
        return false;
    }

    if (!m_data.contains(appId)) {
        qInfo() << "app" << appId << "doesn't exists.";
        return false;
    }
    auto appObj = m_data[appId].toObject();

    if (!appObj.contains(groupName)) {
        qInfo() << "group" << groupName << "doesn't exists.";
        return false;
    }
    auto groupObj = appObj[groupName].toObject();

    if (!groupObj.contains(valueKey)) {
        qInfo() << "value" << valueKey << "doesn't exists.";
        return false;
    }

    groupObj.insert(valueKey, value.toJsonValue());
    appObj.insert(groupName, groupObj);
    m_data.insert(appId, appObj);

    return deferCommit ? true : writeToFile();
}

QVariant ApplicationManager1Storage::readApplicationValue(const QString &appId,
                                                          const QString &groupName,
                                                          const QString &valueKey) const noexcept
{
    if (!m_data.contains(appId)) {
        return {};
    }
    auto app = m_data.constFind(appId)->toObject();
    if (app.isEmpty()) {
        return {};
    }

    if (!app.contains(groupName)) {
        return {};
    }
    auto group = app.constFind(groupName)->toObject();
    if (group.isEmpty()) {
        return {};
    }

    if (!group.contains(valueKey)) {
        return {};
    }
    auto val = group.constFind(valueKey);
    if (val->isNull()) {
        return {};
    }

    return val->toVariant();
}

bool ApplicationManager1Storage::deleteApplicationValue(const QString &appId,
                                                        const QString &groupName,
                                                        const QString &valueKey,
                                                        bool deferCommit) noexcept
{
    if (appId.isEmpty() or groupName.isEmpty() or valueKey.isEmpty()) {
        qWarning() << "unexpected empty string";
        return false;
    }

    auto app = m_data.find(appId).value();
    if (app.isNull()) {
        return true;
    }
    auto appObj = app.toObject();

    auto group = appObj.find(groupName).value();
    if (group.isNull()) {
        return true;
    }
    auto groupObj = group.toObject();

    auto val = groupObj.find(valueKey).value();
    if (val.isNull()) {
        return true;
    }

    groupObj.remove(valueKey);
    appObj.insert(groupName, groupObj);
    m_data.insert(appId, appObj);

    return deferCommit ? true : writeToFile();
}

bool ApplicationManager1Storage::clearData() noexcept
{
    QJsonObject obj;
    m_data.swap(obj);
    return setVersion(STORAGE_VERSION);
}

bool ApplicationManager1Storage::deleteApplication(const QString &appId, bool deferCommit) noexcept
{
    if (appId.isEmpty()) {
        qWarning() << "unexpected empty string.";
        return false;
    }

    m_data.remove(appId);
    return deferCommit ? true : writeToFile();
}

bool ApplicationManager1Storage::deleteGroup(const QString &appId, const QString &groupName, bool deferCommit) noexcept
{
    if (appId.isEmpty() or groupName.isEmpty()) {
        qWarning() << "unexpected empty string.";
        return false;
    }

    auto app = m_data.find(appId).value().toObject();
    if (app.isEmpty()) {
        return true;
    }

    app.remove(groupName);
    m_data.insert(appId, app);
    return deferCommit ? true : writeToFile();
}
