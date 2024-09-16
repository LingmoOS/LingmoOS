// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPLICATIONMIMEINFO_H
#define APPLICATIONMIMEINFO_H

#include <QStringList>
#include <QMap>
#include <QFile>
#include <QMimeData>
#include <memory>
#include <QSemaphore>
#include <optional>
#include <QFileInfo>
#include "mimefileparser.h"

using MimeContent = MimeFileParser::Groups;

QStringList getListFiles() noexcept;

class MimeFileBase
{
public:
    static std::optional<MimeFileBase> loadFromFile(const QFileInfo &fileInfo, bool desktopSpec);
    MimeFileBase(MimeFileBase &&) = default;
    MimeFileBase &operator=(MimeFileBase &&) = default;
    MimeFileBase(const MimeFileBase &) = delete;
    MimeFileBase &operator=(const MimeFileBase &) = delete;

    virtual ~MimeFileBase() = default;
    [[nodiscard]] const QFileInfo &fileInfo() const noexcept { return m_info; }
    [[nodiscard]] MimeContent &content() noexcept { return m_content; }
    [[nodiscard]] const MimeContent &content() const noexcept { return m_content; }
    [[nodiscard]] bool isDesktopSpecific() const noexcept { return m_desktopSpec; }
    [[nodiscard]] bool isWritable() const noexcept { return m_writable; }

    void reload() noexcept;

private:
    MimeFileBase(const QFileInfo &info, MimeContent &&content, bool desktopSpec, bool writable);
    bool m_desktopSpec{false};
    bool m_writable{false};
    QFileInfo m_info;
    MimeContent m_content;
};

struct AppList
{
    QStringList added;
    QStringList removed;
};

class MimeApps : public MimeFileBase
{
public:
    static std::optional<MimeApps> createMimeApps(const QString &filePath, bool desktopSpec) noexcept;

    MimeApps(MimeApps &&) = default;
    MimeApps &operator=(MimeApps &&) = default;
    MimeApps(const MimeApps &) = delete;
    MimeApps &operator=(const MimeApps &) = delete;
    ~MimeApps() override = default;

    void addAssociation(const QString &mimeType, const QString &appId) noexcept;
    void removeAssociation(const QString &mimeType, const QString &appId) noexcept;
    void setDefaultApplication(const QString &mimeType, const QString &appId) noexcept;
    void unsetDefaultApplication(const QString &mimeType) noexcept;

    [[nodiscard]] QString queryDefaultApp(const QString &type) const noexcept;
    [[nodiscard]] AppList queryTypes(QString appId) const noexcept;
    [[nodiscard]] bool writeToFile() const noexcept;

private:
    void insertToSection(const QString &section, const QString &mimeType, const QString &appId) noexcept;
    explicit MimeApps(MimeFileBase &&base);
};

class MimeCache : public MimeFileBase
{
public:
    static std::optional<MimeCache> createMimeCache(const QString &filePath) noexcept;

    MimeCache(MimeCache &&) = default;
    MimeCache &operator=(MimeCache &&) = default;
    MimeCache(const MimeCache &) = delete;
    MimeCache &operator=(const MimeCache &) = delete;
    ~MimeCache() override = default;

    [[nodiscard]] QStringList queryApps(const QString &type) const noexcept;
    [[nodiscard]] QStringList queryTypes(QString appId) const noexcept;

private:
    explicit MimeCache(MimeFileBase &&base);
};

class MimeInfo
{
public:
    static std::optional<MimeInfo> createMimeInfo(const QString &directory) noexcept;

    MimeInfo(MimeInfo &&) = default;
    MimeInfo &operator=(MimeInfo &&) = default;
    MimeInfo(const MimeInfo &) = delete;
    MimeInfo &operator=(const MimeInfo &) = delete;
    ~MimeInfo() = default;

    [[nodiscard]] std::vector<MimeApps> &appsList() noexcept { return m_appsList; }
    [[nodiscard]] const std::vector<MimeApps> &appsList() const noexcept { return m_appsList; }
    [[nodiscard]] std::optional<MimeCache> &cacheInfo() noexcept { return m_cache; }
    [[nodiscard]] const std::optional<MimeCache> &cacheInfo() const noexcept { return m_cache; }
    [[nodiscard]] const QString &directory() const noexcept { return m_directory; }

    void reload() noexcept;

private:
    MimeInfo() = default;
    std::vector<MimeApps> m_appsList;
    std::optional<MimeCache> m_cache{std::nullopt};
    QString m_directory;
};

#endif
