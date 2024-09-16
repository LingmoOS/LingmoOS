// SPDX-FileCopyrightText: 2021 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOCPARSER_H
#define DOCPARSER_H

#include <string>

class DocParser
{
public:
    static std::string convertFile(const std::string &filename);
};

#endif // DOCPARSER_H
