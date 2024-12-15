// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "webbinder.h"

#include <regex>

WebBinder::WebBinder()
{

}

int WebBinder::bind(std::string webDir, std::string diskDir)
{
    // std::regex validateWeb(R"([^\0<>:"/\\|?*]*$)");
    // std::regex validateDisk(R"((\/)?(?![.]{1,2}($|\/))[\w.-]+(\/[\w.-]+)*(\\.[\w]+)?)");
    std::regex isFile(R"(\S*\/$)");

    // std::regex validateWeb(R"([^\0<>:"/\\|?*]*$)"), validateDisk("(\\/)?((\\w)+(\\/)?)*(\\.(\\w)+)?"), isFile("\\S*\\/");
    // std::regex validateWeb("(\\/((\\w)*|\\*))*"), validateDisk("(\\/)?((\\w)+(\\/)?)*(\\.(\\w)+)?"), isFile("\\S*\\/");

    // if (!std::regex_match(webDir, validateWeb)) return -2;
    // if (!std::regex_match(diskDir, validateDisk)) return -3;
    if (std::regex_match(webDir, isFile) != std::regex_match(diskDir, isFile)) return -4;

    for (auto &pair : _binds) if (pair.first.compare(webDir) == 0) return -1;

    _binds.insert(_binds.begin(), std::make_pair(webDir, diskDir));

    return 0;
}

int WebBinder::unbind(std::string webDir)
{
    for (size_t i = 0; i < _binds.size(); i++) {
        if (_binds[i].first.compare(webDir) == 0) {
            _binds.erase(_binds.begin() + i);
            return 0;
        }
    }
    return -1;
}

void WebBinder::clear()
{
    _binds.clear();
}

std::string WebBinder::getPath(std::string path)
{
    if (path.empty()) {
        return "";
    }
    for (auto &pair : _binds) {
        const std::string& virtualBind = pair.first;
        if (path.find(virtualBind) == 0) {
            std::string rest = path.substr(virtualBind.length()); // 获取虚拟路径中与绑定路径匹配部分之外的部分
            return pair.second + rest; // 返回对应的物理路径
        }
    }

    return "";
}

bool WebBinder::containWeb(const std::string &name)
{
    for (auto &pair : _binds) {
        if (pair.first.find(name) != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool WebBinder::lastWeb(const std::string &name)
{
    if (!_binds.empty()) {
        auto lastElement = _binds.back();
        if (name.find(lastElement.first) != std::string::npos) {
            return true;
        }
    }

    return false;
}

//source: https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
void WebBinder::replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

//source: https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
bool WebBinder::replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}
