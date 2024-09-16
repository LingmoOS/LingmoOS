// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "desktopentry.h"
#include "desktopfileparser.h"
#include <QFileInfo>
#include <QDir>
#include <algorithm>
#include <QRegularExpression>
#include <QDirIterator>
#include <QStringView>
#include <QVariant>
#include <iostream>
#include <chrono>
#include <cstdio>

QString DesktopFile::sourcePath() const noexcept
{
    if (!m_fileSource) {
        return "";
    }

    QFileInfo info(*m_fileSource);
    return info.absoluteFilePath();
}

bool DesktopEntry::checkMainEntryValidation() const noexcept
{
    auto it = m_entryMap.find(DesktopFileEntryKey);
    if (it == m_entryMap.end()) {
        return false;
    }

    if (auto name = it->find("Name"); name == it->end()) {
        qWarning() << "No Name.";
        return false;
    }

    auto type = it->find("Type");
    if (type == it->end()) {
        qWarning() << "No Type.";
        for (auto tmp = it->constKeyValueBegin(); tmp != it->constKeyValueEnd(); ++tmp) {
            const auto &[k, v] = *tmp;
            qInfo() << "key:" << k << "value:" << v;
        }
        return false;
    }

    const auto &typeStr = type->toString();
    if (typeStr.isEmpty()) {
        return false;
    }
    if (typeStr == "Link") {
        if (it->find("URL") == it->end()) {
            return false;
        }
    }

    return true;
}

std::optional<DesktopFile> DesktopFile::createTemporaryDesktopFile(std::unique_ptr<QFile> temporaryFile) noexcept
{
    auto [ctime, mtime, _] = getFileTimeInfo(QFileInfo{*temporaryFile});
    if (mtime == 0) {
        qWarning() << "create temporary file failed.";
        return std::nullopt;
    }
    return DesktopFile{std::move(temporaryFile), "", mtime, ctime};
}

std::optional<DesktopFile> DesktopFile::createTemporaryDesktopFile(const QString &temporaryFile) noexcept
{
    const static QString userTmp = QString{"/run/user/%1/"}.arg(getCurrentUID());
    auto tempFile = std::make_unique<QFile>(QString{userTmp + QUuid::createUuid().toString(QUuid::Id128) + ".desktop"});

    if (!tempFile->open(QFile::NewOnly | QFile::WriteOnly | QFile::Text)) {
        qWarning() << "failed to create temporary desktop file:" << QFileInfo{*tempFile}.absoluteFilePath()
                   << tempFile->errorString();
        return std::nullopt;
    }

    auto content = temporaryFile.toLocal8Bit();
    auto writeByte = tempFile->write(content);

    if (writeByte == -1 || writeByte != content.length()) {
        qWarning() << "write to temporary file failed:" << tempFile->errorString();
        return std::nullopt;
    }

    tempFile->close();

    return createTemporaryDesktopFile(std::move(tempFile));
}

std::optional<DesktopFile> DesktopFile::searchDesktopFileByPath(const QString &desktopFile, ParserError &err) noexcept
{
    decltype(auto) desktopSuffix = ".desktop";

    if (!desktopFile.endsWith(desktopSuffix)) {
        qWarning() << "file isn't a desktop file:" << desktopFile;
        err = ParserError::MismatchedFile;
        return std::nullopt;
    }

    QFileInfo fileinfo{desktopFile};
    if (!fileinfo.isAbsolute() or !fileinfo.exists()) {
        qWarning() << "desktop file not found.";
        err = ParserError::NotFound;
        return std::nullopt;
    }

    const auto &XDGDataDirs = getDesktopFileDirs();
    const auto &autostartDirs = getAutoStartDirs();
    auto IsCommonApp = std::any_of(XDGDataDirs.cbegin(), XDGDataDirs.cend(), [&desktopFile](const QString &suffixPath) {
        return desktopFile.startsWith(suffixPath);
    });

    auto IsAutostartApp = std::any_of(autostartDirs.cbegin(), autostartDirs.cend(), [&desktopFile](const QString &suffixPath) {
        return desktopFile.startsWith(suffixPath);
    });

    QString path{desktopFile};
    QString id;

    if (IsCommonApp or IsAutostartApp) {
        auto tmp = path.chopped(sizeof(desktopSuffix) - 1);
        auto components = tmp.split(QDir::separator(), Qt::SkipEmptyParts).toList();
        decltype(components)::const_iterator it;
        if (auto appIt = std::find(components.cbegin(), components.cend(), "applications"); appIt != components.cend()) {
            it = appIt;

        } else if (auto autostartIt = std::find(components.cbegin(), components.cend(), "autostart");
                   autostartIt != components.cend()) {
            it = autostartIt;
        }

        QString FileId;
        ++it;
        while (it != components.cend()) {
            FileId += (*(it++) + "-");
        }
        id = FileId.chopped(1);  // remove extra "-""
    }

    auto filePtr = std::make_unique<QFile>(std::move(path));

    auto [ctime, mtime, _] = getFileTimeInfo(QFileInfo{*filePtr});

    err = ParserError::NoError;
    return DesktopFile{std::move(filePtr), std::move(id), mtime, ctime};
}

std::optional<DesktopFile> DesktopFile::searchDesktopFileById(const QString &appId, ParserError &err) noexcept
{
    auto XDGDataDirs = getDesktopFileDirs();
    constexpr auto desktopSuffix = u8".desktop";

    for (const auto &dir : XDGDataDirs) {
        auto app = QFileInfo{dir + QDir::separator() + appId + desktopSuffix};
        while (!app.exists()) {
            auto filePath = app.absoluteFilePath();
            auto hyphenIndex = filePath.indexOf('-');
            if (hyphenIndex == -1) {
                break;
            }
            filePath.replace(hyphenIndex, 1, QDir::separator());
            app.setFile(filePath);
        }

        if (app.exists()) {
            return searchDesktopFileByPath(app.absoluteFilePath(), err);
        }
    }

    err = ParserError::NotFound;
    return std::nullopt;
}

bool DesktopFile::modified(qint64 time) const noexcept
{
    return time != m_mtime;
}

ParserError DesktopEntry::parse(const DesktopFile &file) noexcept
{
    DesktopFileGuard guard{file};

    if (!guard.try_open()) {
        qWarning() << file.sourcePath() << "can't open.";
        return ParserError::OpenFailed;
    }

    QTextStream stream;
    stream.setDevice(file.sourceFile());
    return parse(stream);
}

ParserError DesktopEntry::parse(QTextStream &stream) noexcept
{
    if (m_parsed) {
        return ParserError::Parsed;
    }

    if (stream.atEnd()) {
        return ParserError::OpenFailed;
    }

    stream.setEncoding(QStringConverter::Utf8);

    ParserError err{ParserError::NoError};
    DesktopFileParser p(stream);
    err = p.parse(m_entryMap);
    m_parsed = true;
    if (err != ParserError::NoError) {
        return err;
    }

    if (!checkMainEntryValidation()) {
        qWarning() << "invalid MainEntry, abort.";
        err = ParserError::MissingInfo;
    }

    return err;
}

std::optional<QMap<QString, DesktopEntry::Value>> DesktopEntry::group(const QString &key) const noexcept
{
    if (auto group = m_entryMap.find(key); group != m_entryMap.cend()) {
        return *group;
    }
    return std::nullopt;
}

std::optional<DesktopEntry::Value> DesktopEntry::value(const QString &groupKey, const QString &valueKey) const noexcept
{
    const auto &destGroup = group(groupKey);
    if (!destGroup) {
        qDebug() << "group " << groupKey << " can't be found.";
        return std::nullopt;
    }

    auto it = destGroup->find(valueKey);
    if (it == destGroup->cend()) {
        qDebug() << "value " << valueKey << " can't be found.";
        return std::nullopt;
    }
    return *it;
}

void DesktopEntry::insert(const QString &key, const QString &valueKey, Value &&val) noexcept
{
    auto outer = m_entryMap.find(key);
    if (outer == m_entryMap.end()) {
        outer = m_entryMap.insert(key, {});
    }

    outer->insert(valueKey, val);
}

QString unescape(const QString &str, bool shellMode) noexcept
{
    QString unescapedStr;
    for (qsizetype i = 0; i < str.size(); ++i) {
        auto c = str.at(i);
        if (c != '\\') {
            unescapedStr.append(c);
            continue;
        }

        switch (str.at(i + 1).toLatin1()) {
        default:
            unescapedStr.append(c);
            break;
        case 'n':
            unescapedStr.append('\n');
            ++i;
            break;
        case 't':
            unescapedStr.append('\t');
            ++i;
            break;
        case 'r':
            unescapedStr.append('\r');
            ++i;
            break;
        case '\\':
            unescapedStr.append('\\');
            ++i;
            break;
        case ';':
            unescapedStr.append(';');
            ++i;
            break;
        case 's': {
            if (shellMode) {  // for wordexp
                unescapedStr.append('\\');
            }
            unescapedStr.append(' ');
            ++i;
        } break;
        }
    }

    return unescapedStr;
}

QString toString(const DesktopEntry::Value &value) noexcept
{
    QString str;

    if (value.canConvert<QStringMap>()) {  // get default locale
        str = value.value<QStringMap>()[DesktopFileDefaultKeyLocale];
    } else {
        str = value.toString();
    }

    if (str.isEmpty()) {
        qWarning() << "value not found.";
        return {};
    }

    auto unescapedStr = unescape(str);

    if (hasNonAsciiAndControlCharacters(unescapedStr)) {
        return {};
    }

    return unescapedStr;
}

QString toLocaleString(const QStringMap &localeMap, const QLocale &locale) noexcept
{
    for (auto it = localeMap.constKeyValueBegin(); it != localeMap.constKeyValueEnd(); ++it) {
        auto [a, b] = *it;
        if (QLocale{a}.name() == locale.name()) {
            return unescape(b);
        }
    }

    return toString(localeMap[DesktopFileDefaultKeyLocale]);
}

QString toIconString(const DesktopEntry::Value &value) noexcept
{
    return toString(value);
}

bool toBoolean(const DesktopEntry::Value &value, bool &ok) noexcept
{
    ok = false;
    const auto &str = toString(value);
    if (str == "true") {
        ok = true;
        return true;
    }

    if (str == "false") {
        ok = true;
        return false;
    }

    return false;
}

float toNumeric(const DesktopEntry::Value &value, bool &ok) noexcept
{
    return value.toFloat(&ok);
}

bool operator==(const DesktopEntry &lhs, const DesktopEntry &rhs)
{
    if (lhs.m_parsed != rhs.m_parsed) {
        return false;
    }

    if (lhs.m_entryMap != rhs.m_entryMap) {
        return false;
    }

    return true;
}

bool operator!=(const DesktopEntry &lhs, const DesktopEntry &rhs)
{
    return !(lhs == rhs);
}

bool operator==(const DesktopFile &lhs, const DesktopFile &rhs)
{
    if (lhs.m_desktopId != rhs.m_desktopId) {
        return false;
    }

    if (lhs.sourcePath() != rhs.sourcePath()) {
        return false;
    }

    if (lhs.m_ctime != rhs.m_ctime or lhs.m_mtime != rhs.m_mtime) {
        return false;
    }

    return true;
}

bool operator!=(const DesktopFile &lhs, const DesktopFile &rhs)
{
    return !(lhs == rhs);
}
