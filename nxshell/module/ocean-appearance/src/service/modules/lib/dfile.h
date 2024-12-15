// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILE_H
#define DFILE_H

#include <string>
#include <vector>

#define MAX_FILEPATH_LEN 256
#define MAX_LINE_LEN 256
class DFile
{
public:
    explicit DFile();
    static bool isAbs(std::string file);
    static bool isExisted(std::string file);
    static std::string dir(std::string file);
    static std::string base(std::string file);
    static std::vector<std::string> glob(const std::string& pattern);
};
#endif // DFILE_H
