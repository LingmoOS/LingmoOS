// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEBBINDER_H
#define WEBBINDER_H

#include "utility/singleton.h"

#include <string>
#include <vector>
#include <utility>

class WebBinder : public CppCommon::Singleton<WebBinder>
{
    friend CppCommon::Singleton<WebBinder>;
public:
    WebBinder();

    int bind(std::string webDir, std::string diskDir);
    int unbind(std::string webDir);
    void clear();
    std::string getPath(std::string path);
    bool containWeb(const std::string &name);
    bool lastWeb(const std::string &name);

private:
    void replaceAll(std::string &str, const std::string &from, const std::string &to);
    bool replace(std::string &str, const std::string &from, const std::string &to);

    std::vector<std::pair<std::string, std::string>> _binds;
};

#endif // WEBBINDER_H
