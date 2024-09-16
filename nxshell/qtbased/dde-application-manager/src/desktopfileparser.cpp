// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QRegularExpression>
#include <QStringBuilder>

#include "desktopfileparser.h"
#include "constant.h"
#include "global.h"

namespace {
bool isInvalidLocaleString(const QString &str) noexcept
{
    constexpr auto Language = R"((?:[a-z]+))";        // language of locale postfix. eg.(en, zh)
    constexpr auto Country = R"((?:_[A-Z]+))";        // country of locale postfix. eg.(US, CN)
    constexpr auto Encoding = R"((?:\.[0-9A-Z-]+))";  // encoding of locale postfix. eg.(UFT-8)
    constexpr auto Modifier = R"((?:@[a-zA-Z=;]+))";  // modifier of locale postfix. eg.(euro;collation=traditional)
    const static auto validKey = QString(R"(^%1%2?%3?%4?$)").arg(Language, Country, Encoding, Modifier);
    // example: https://regex101.com/r/hylOay/2
    static const QRegularExpression _re = []() -> QRegularExpression {
        QRegularExpression tmp{validKey};
        tmp.optimize();
        return tmp;
    }();
    thread_local const auto re = _re;

    return re.match(str).hasMatch();
}

bool isLocaleString(const QString &key) noexcept
{
    static QSet<QString> localeSet{"Name", "GenericName", "Comment", "Keywords"};
    return localeSet.contains(key);
}

}  // namespace

ParserError DesktopFileParser::parse(Groups &ret) noexcept
{
    std::remove_reference_t<decltype(ret)> groups;
    while (!m_stream.atEnd()) {
        auto err = addGroup(groups);
        if (err != ParserError::NoError) {
            return err;
        }

        if (groups.size() != 1) {
            continue;
        }

        if (groups.keys().first() != DesktopFileEntryKey) {
            qWarning() << "There should be nothing preceding "
                          "'Desktop Entry' group in the desktop entry file "
                          "but possibly one or more comments.";
            return ParserError::InvalidFormat;
        }
    }

    if (!m_line.isEmpty()) {
        qCritical() << "Something is wrong in Desktop file parser, check logic.";
        return ParserError::InternalError;
    }

    ret = std::move(groups);
    return ParserError::NoError;
}

ParserError DesktopFileParser::addGroup(Groups &ret) noexcept
{
    skip();
    if (!m_line.startsWith('[')) {
        qWarning() << "Invalid desktop file format: unexpected line:" << m_line;
        return ParserError::InvalidFormat;
    }

    // Parsing group header.
    // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#group-header

    auto groupHeader = m_line.sliced(1, m_line.size() - 2).trimmed();

    if (groupHeader.contains('[') || groupHeader.contains(']') || hasNonAsciiAndControlCharacters(groupHeader)) {
        qWarning() << "group header invalid:" << m_line;
        return ParserError::InvalidFormat;
    }

    if (ret.find(groupHeader) != ret.end()) {
        qWarning() << "duplicated group header detected:" << groupHeader;
        return ParserError::InvalidFormat;
    }

    auto group = ret.insert(groupHeader, {});

    m_line.clear();
    while (!m_stream.atEnd() && !m_line.startsWith('[')) {
        skip();
        if (m_line.startsWith('[')) {
            break;
        }
        auto err = addEntry(group);
        if (err != ParserError::NoError) {
            return err;
        }
    }
    return ParserError::NoError;
}

ParserError DesktopFileParser::addEntry(typename Groups::iterator &group) noexcept
{
    auto line = m_line;
    m_line.clear();
    auto splitCharIndex = line.indexOf('=');
    if (splitCharIndex == -1) {
        qWarning() << "invalid line in desktop file, skip it:" << line;
        return ParserError::NoError;
    }
    auto keyStr = line.first(splitCharIndex).trimmed();
    auto valueStr = line.sliced(splitCharIndex + 1).trimmed();

    QString key{""};
    QString localeStr{DesktopFileDefaultKeyLocale};
    // NOTE:
    // We are process "localized keys" here, for usage check:
    // https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#localized-keys
    qsizetype localeBegin = keyStr.indexOf('[');
    qsizetype localeEnd = keyStr.lastIndexOf(']');
    if ((localeBegin == -1 && localeEnd != -1) || (localeBegin != -1 && localeEnd == -1)) {
        qWarning() << "unmatched [] detected in desktop file, skip this line: " << line;
        return ParserError::NoError;
    }

    if (localeBegin == -1 && localeEnd == -1) {
        key = keyStr;
    } else {
        key = keyStr.sliced(0, localeBegin);
        localeStr = keyStr.sliced(localeBegin + 1, localeEnd - localeBegin - 1);  // strip '[' and ']'
    }

    static const QRegularExpression _re = []() {
        QRegularExpression tmp{"R([^A-Za-z0-9-])"};
        tmp.optimize();
        return tmp;
    }();
    // NOTE: https://stackoverflow.com/a/25583104
    thread_local const QRegularExpression re = _re;
    if (re.match(key).hasMatch()) {
        qWarning() << "invalid key name:" << key << ", skip this line:" << line;
        return ParserError::NoError;
    }

    if (localeStr != DesktopFileDefaultKeyLocale and !isInvalidLocaleString(localeStr)) {
        qWarning().noquote() << QString("invalid LOCALE (%2) for key \"%1\"").arg(key, localeStr);
        return ParserError::NoError;
    }

    if (auto keyIt = group->find(key); keyIt != group->end()) {
        if (!isLocaleString(key)) {
            qWarning() << "duplicate key:" << key << "skip.";
            return ParserError::NoError;
        }

        if (!keyIt->canConvert<QStringMap>()) {
            qWarning() << "underlying type of value is invalid, raw value:" << *keyIt << "skip";
            return ParserError::NoError;
        }

        auto localeMap = keyIt->value<QStringMap>();
        if (localeMap.find(localeStr) != localeMap.end()) {
            qWarning() << "duplicate locale key:" << key << "skip.";
            return ParserError::NoError;
        }

        localeMap.insert(localeStr, valueStr);
        group->insert(key, QVariant::fromValue(localeMap));
    } else {
        if (isLocaleString(key)) {
            group->insert(key, QVariant::fromValue(QStringMap{{localeStr, valueStr}}));
        } else {
            group->insert(key, QVariant::fromValue(valueStr));
        }
    }

    return ParserError::NoError;
}

QString toString(const DesktopFileParser::Groups &map)
{
    QString ret;
    auto groupToString = [&ret, map](const QString &group) {
        const auto &groupEntry = map[group];
        ret.append('[' % group % "]\n");
        for (auto entryIt = groupEntry.constKeyValueBegin(); entryIt != groupEntry.constKeyValueEnd(); ++entryIt) {
            const auto &key = entryIt->first;
            const auto &value = entryIt->second;
            if (value.canConvert<QStringMap>()) {
                const auto &rawMap = value.value<QStringMap>();
                std::for_each(rawMap.constKeyValueBegin(), rawMap.constKeyValueEnd(), [key, &ret](const auto &inner) {
                    const auto &[locale, rawVal] = inner;
                    ret.append(key);
                    if (locale != DesktopFileDefaultKeyLocale) {
                        ret.append('[' % locale % ']');
                    }
                    ret.append('=' % rawVal % '\n');
                });
            } else if (value.canConvert<QStringList>()) {
                const auto &rawVal = value.value<QStringList>();
                auto str = rawVal.join(';');
                ret.append(key % '=' % str % '\n');
            } else if (value.canConvert<QString>()) {
                const auto &rawVal = value.value<QString>();
                ret.append(key % '=' % rawVal % '\n');
            } else {
                qWarning() << "value type mismatch:" << value;
            }
        }
        ret.append('\n');
    };

    groupToString(DesktopFileEntryKey);
    for (const auto &groupName : map.keys()) {
        if (groupName == DesktopFileEntryKey) {
            continue;
        }
        groupToString(groupName);
    }

    return ret;
}
