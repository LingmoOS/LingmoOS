// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPLICATIONMANAGERSTORAGE_H
#define APPLICATIONMANAGERSTORAGE_H

#include <QString>
#include <QJsonObject>
#include <QFile>

enum class ModifyMode { Create, Update };

class ApplicationManager1Storage
{
public:
    ApplicationManager1Storage(const ApplicationManager1Storage &) = delete;
    ApplicationManager1Storage(ApplicationManager1Storage &&) = default;
    ApplicationManager1Storage &operator=(const ApplicationManager1Storage &) = delete;
    ApplicationManager1Storage &operator=(ApplicationManager1Storage &&) = default;
    ~ApplicationManager1Storage() = default;

    [[nodiscard]] bool createApplicationValue(const QString &appId,
                                              const QString &groupName,
                                              const QString &valueKey,
                                              const QVariant &value,
                                              bool deferCommit = false) noexcept;
    [[nodiscard]] bool updateApplicationValue(const QString &appId,
                                              const QString &groupName,
                                              const QString &valueKey,
                                              const QVariant &value,
                                              bool deferCommit = false) noexcept;
    [[nodiscard]] QVariant
    readApplicationValue(const QString &appId, const QString &groupName, const QString &valueKey) const noexcept;
    [[nodiscard]] bool deleteApplicationValue(const QString &appId,
                                              const QString &groupName,
                                              const QString &valueKey,
                                              bool deferCommit = false) noexcept;
    [[nodiscard]] bool clearData() noexcept;
    [[nodiscard]] bool deleteApplication(const QString &appId, bool deferCommit = false) noexcept;
    [[nodiscard]] bool deleteGroup(const QString &appId, const QString &groupName, bool deferCommit = false) noexcept;

    bool setVersion(uint8_t version) noexcept;
    [[nodiscard]] uint8_t version() const noexcept;

    bool setFirstLaunch(bool first) noexcept;
    [[nodiscard]] bool firstLaunch() const noexcept;

    [[nodiscard]] static std::shared_ptr<ApplicationManager1Storage>
    createApplicationManager1Storage(const QString &storageDir) noexcept;

private:
    [[nodiscard]] bool writeToFile() const noexcept;
    explicit ApplicationManager1Storage(const QString &storagePath);
    std::unique_ptr<QFile> m_file;
    QJsonObject m_data;
};

#endif
