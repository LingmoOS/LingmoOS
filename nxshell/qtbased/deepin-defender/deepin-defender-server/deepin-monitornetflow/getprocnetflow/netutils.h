// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdio.h>
#include <arpa/inet.h>

#include <QString>

/*// network utilities
*/
//

// 从字符串中获取ipv4数据
void getIpv4Addr(const QString &addrStr, in_addr_t &ip4Addr);
void getNetPort(const QString &portStr, unsigned short &port);
void getIpv6Addr(const QString &addrStr, uint8_t (&localIp6Addr)[16]);

// 比较ipv6地址，相同返回真
bool cmpIpv6Addr(const uint8_t addrA[16], const uint8_t addrB[16]);
// 判断字符是否为纯数字
bool is_number_t(const char *string);

// 检查dbus服务名称是否规范
bool checkDbusServiceName(const QString &service);
// 检查dbus服务路径是否规范
bool checkDbusServicePath(const QString &path);
