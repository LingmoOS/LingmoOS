// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPFILEPARSER_H
#define DESKTOPFILEPARSER_H

#include "iniParser.h"
#include "desktopentry.h"

class DesktopFileParser : public Parser<DesktopEntry::Value>
{
public:
    using Parser<DesktopEntry::Value>::Parser;
    ParserError parse(Groups &ret) noexcept override;
    ParserError addGroup(Groups &ret) noexcept override;
    ParserError addEntry(Groups::iterator &group) noexcept override;
};

QString toString(const DesktopFileParser::Groups &map);

#endif
