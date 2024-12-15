// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPENTRY_H
#define DESKTOPENTRY_H

#include <QString>
#include <QMap>
#include <QDebug>
#include <QLocale>
#include <QTextStream>
#include <optional>
#include <QFile>
#include "iniParser.h"
#include "global.h"

enum class EntryContext { Unknown, EntryOuter, Entry, Done };

enum class EntryValueType { String, LocaleString, Boolean, IconString, Raw };

struct DesktopFileGuard;

struct DesktopFile
{
    friend struct DesktopFileGuard;
    DesktopFile(const DesktopFile &) = delete;
    DesktopFile(DesktopFile &&) = default;
    DesktopFile &operator=(const DesktopFile &) = delete;
    DesktopFile &operator=(DesktopFile &&) = default;
    ~DesktopFile() = default;

    [[nodiscard]] QString sourcePath() const noexcept;
    // WARNING: This raw pointer's ownership belong to DesktopFile, DO NOT MODIFY!
    [[nodiscard]] QFile *sourceFile() const noexcept { return &sourceFileRef(); };
    [[nodiscard]] QFile &sourceFileRef() const noexcept { return *m_fileSource; };
    [[nodiscard]] const QString &desktopId() const noexcept { return m_desktopId; }
    [[nodiscard]] bool modified(qint64 time) const noexcept;
    [[nodiscard]] qint64 createTime() const noexcept { return m_ctime; }

    friend bool operator==(const DesktopFile &lhs, const DesktopFile &rhs);
    friend bool operator!=(const DesktopFile &lhs, const DesktopFile &rhs);

    static std::optional<DesktopFile> searchDesktopFileById(const QString &appId, ParserError &err) noexcept;
    static std::optional<DesktopFile> searchDesktopFileByPath(const QString &desktopFilePath, ParserError &err) noexcept;
    static std::optional<DesktopFile> createTemporaryDesktopFile(const QString &temporaryFile) noexcept;
    static std::optional<DesktopFile> createTemporaryDesktopFile(std::unique_ptr<QFile> temporaryFile) noexcept;

private:
    DesktopFile(std::unique_ptr<QFile> source, QString fileId, qint64 mtime, qint64 ctime)
        : m_mtime(mtime)
        , m_ctime(ctime)
        , m_fileSource(std::move(source))
        , m_desktopId(std::move(fileId))
    {
    }

    qint64 m_mtime;
    qint64 m_ctime;
    std::unique_ptr<QFile> m_fileSource{nullptr};
    QString m_desktopId{""};
};

struct DesktopFileGuard
{
    DesktopFileGuard(const DesktopFileGuard &) = delete;
    DesktopFileGuard(DesktopFileGuard &&other) noexcept
        : fileRef(other.fileRef)
    {
    }
    DesktopFileGuard &operator=(const DesktopFileGuard &) = delete;
    DesktopFileGuard &operator=(DesktopFileGuard &&) = delete;

    explicit DesktopFileGuard(const DesktopFile &file)
        : fileRef(file)
    {
    }

    bool try_open()
    {
        if (!fileRef.m_fileSource) {
            return false;
        }

        if (fileRef.m_fileSource->isOpen()) {
            return true;
        }

        auto ret = fileRef.m_fileSource->open(QFile::ExistingOnly | QFile::ReadOnly | QFile::Text);
        if (!ret) {
            qWarning() << "open desktop file failed:" << fileRef.m_fileSource->errorString();
        }

        return ret;
    }

    void close()
    {
        if (!fileRef.m_fileSource) {
            return;
        }

        if (fileRef.m_fileSource->isOpen()) {
            fileRef.m_fileSource->close();
        }
    }

    ~DesktopFileGuard() { close(); }

private:
    const DesktopFile &fileRef;
};

class DesktopEntry
{
public:
    using Value = QVariant;
    DesktopEntry() = default;
    DesktopEntry(const DesktopEntry &) = default;
    DesktopEntry(DesktopEntry &&) = default;
    DesktopEntry &operator=(const DesktopEntry &) = default;
    DesktopEntry &operator=(DesktopEntry &&) = default;

    ~DesktopEntry() = default;
    [[nodiscard]] ParserError parse(const DesktopFile &file) noexcept;
    [[nodiscard]] ParserError parse(QTextStream &stream) noexcept;
    [[nodiscard]] std::optional<QMap<QString, Value>> group(const QString &key) const noexcept;
    [[nodiscard]] std::optional<Value> value(const QString &key, const QString &valueKey) const noexcept;
    void insert(const QString &key, const QString &valueKey, Value &&val) noexcept;
    [[nodiscard]] const auto &data() const noexcept { return m_entryMap; }

    friend bool operator==(const DesktopEntry &lhs, const DesktopEntry &rhs);
    friend bool operator!=(const DesktopEntry &lhs, const DesktopEntry &rhs);

private:
    [[nodiscard]] bool checkMainEntryValidation() const noexcept;
    QMap<QString, QMap<QString, Value>> m_entryMap;
    bool m_parsed{false};

public:
    using container_type = decltype(m_entryMap);
};

bool operator==(const DesktopEntry &lhs, const DesktopEntry &rhs);

bool operator!=(const DesktopEntry &lhs, const DesktopEntry &rhs);

bool operator==(const DesktopFile &lhs, const DesktopFile &rhs);

bool operator!=(const DesktopFile &lhs, const DesktopFile &rhs);

QString unescape(const QString &str, bool shellMode = false) noexcept;

QString toLocaleString(const QStringMap &localeMap, const QLocale &locale) noexcept;

QString toString(const DesktopEntry::Value &value) noexcept;

bool toBoolean(const DesktopEntry::Value &value, bool &ok) noexcept;

QString toIconString(const DesktopEntry::Value &value) noexcept;

float toNumeric(const DesktopEntry::Value &value, bool &ok) noexcept;

#endif
