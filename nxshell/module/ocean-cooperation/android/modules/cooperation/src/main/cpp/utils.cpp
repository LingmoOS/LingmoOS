// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <sstream>
#include <vector>
#include <ifaddrs.h>
#include <arpa/inet.h>

std::string Utils::getLocalIp()
{
    struct ifaddrs *addresses;
    if (getifaddrs(&addresses) == -1) {
        return ""; // 返回空字符串表示失败
    }

    std::string ipAddress;
    for (struct ifaddrs *addr = addresses; addr != nullptr; addr = addr->ifa_next) {
        if (addr->ifa_addr && addr->ifa_addr->sa_family == AF_INET) { // 确保是 IPv4
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(((struct sockaddr_in *)addr->ifa_addr)->sin_addr), ip, sizeof(ip));
            ipAddress = ip; // 提取 IP 地址
            break; // 找到第一个有效 IP 地址后退出
        }
    }

    freeifaddrs(addresses); // 释放获取的地址链表
    return ipAddress;
}

std::vector<std::string> Utils::split(const std::string &s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// 实现 join 函数
std::string Utils::join(const std::vector<std::string>& elements, const std::string& delimiter)
{
    std::ostringstream oss;
    for (size_t i = 0; i < elements.size(); ++i) {
        oss << elements[i]; // 将元素写入流
        if (i != elements.size() - 1) { // 判断是否为最后一个元素
            oss << delimiter; // 在元素之间添加分隔符
        }
    }
    return oss.str(); // 返回连接后的字符串
}