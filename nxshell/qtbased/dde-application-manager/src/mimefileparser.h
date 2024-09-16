// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MIMEAPPFILEPARSER_H
#define MIMEAPPFILEPARSER_H

#include "iniParser.h"
#include <QMimeDatabase>
#include <QMimeType>

constexpr auto defaultApplications = "Default Applications";
constexpr auto addedAssociations = "Added Associations";
constexpr auto removedAssociations = "Removed Associations";
constexpr auto mimeCache = "MIME Cache";

class MimeFileParser : public Parser<QStringList>
{
public:
    explicit MimeFileParser(QTextStream &stream, bool isDesktopSpecific)
        : Parser(stream)
        , m_desktopSpec(isDesktopSpecific)
    {
    }
    ParserError parse(Groups &ret) noexcept override;
    ParserError addGroup(Groups &ret) noexcept override;
    ParserError addEntry(Groups::iterator &group) noexcept override;

private:
    bool m_desktopSpec;
};

#endif
