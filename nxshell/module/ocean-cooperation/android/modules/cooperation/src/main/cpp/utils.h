// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANDROID_UTILS_H
#define ANDROID_UTILS_H

#include <string>
#include <vector>

class Utils {

public:
    static std::string getLocalIp();
    static std::vector<std::string> split(const std::string &s, char delimiter);
    static std::string join(const std::vector<std::string> &v, const std::string &delimiter);

private:
    Utils();

};


#endif //ANDROID_UTILS_H
