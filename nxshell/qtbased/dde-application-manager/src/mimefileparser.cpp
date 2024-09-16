// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mimefileparser.h"
#include <QDebug>

ParserError MimeFileParser::parse(Groups &ret) noexcept
{
    std::remove_reference_t<decltype(ret)> groups;
    while (!m_stream.atEnd()) {
        auto err = addGroup(groups);
        if (err != ParserError::NoError) {
            ret.clear();
            return err;
        }
    }

    if (!m_line.isEmpty()) {
        qCritical() << "Something is wrong in mimeapp.list parser, check logic.";
        ret.clear();
        return ParserError::InternalError;
    }

    ret = std::move(groups);
    return ParserError::NoError;
}

ParserError MimeFileParser::addGroup(Groups &ret) noexcept
{
    skip();
    if (!m_line.startsWith('[')) {
        qWarning() << "Invalid mimeapp.list format: unexpected line:" << m_line;
        return ParserError::InvalidFormat;
    }

    // Parsing group header, this format is same as desktop file's group

    auto groupHeader = m_line.sliced(1, m_line.size() - 2).trimmed();

    if (groupHeader.contains('[') || groupHeader.contains(']') || hasNonAsciiAndControlCharacters(groupHeader)) {
        qWarning() << "group header invalid:" << m_line;
        return ParserError::InvalidFormat;
    }

    if (m_desktopSpec and (groupHeader == addedAssociations or groupHeader == removedAssociations)) {
        qWarning()
            << "desktop-specific mimeapp.list is not possible to add or remove associations from these files, skip this group.";
        while (!m_stream.atEnd() && !m_line.startsWith('[')) {
            skip();
            if (m_line.startsWith('[')) {
                break;
            }
        }
        return ParserError::NoError;
    }

    Groups::iterator group;
    if (group = ret.find(groupHeader); group == ret.end()) {
        group = ret.insert(groupHeader, {});
    }

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

ParserError MimeFileParser::addEntry(Groups::iterator &group) noexcept
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

    if (valueStr.isEmpty()) {
        return ParserError::InvalidFormat;
    }

    auto newValues = valueStr.split(';', Qt::SkipEmptyParts);
    auto value = group->value(keyStr);
    value.append(newValues);
    group->insert(keyStr, newValues);

    return ParserError::NoError;
}
