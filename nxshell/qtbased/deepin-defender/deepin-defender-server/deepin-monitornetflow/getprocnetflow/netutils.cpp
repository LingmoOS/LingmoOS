// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netutils.h"

// 从字符串中获取ipv4数据
void getIpv4Addr(const QString &addrStr, in_addr_t &ip4Addr)
{
    bool ok;
    ip4Addr = addrStr.mid(0, 8).toUInt(&ok, 16);
}

void getNetPort(const QString &portStr, unsigned short &port)
{
    bool ok;
    port = portStr.mid(0, 4).toUShort(&ok, 16);
}

void getIpv6Addr(const QString &addrStr, uint8_t (&localIp6Addr)[16])
{
    bool ok;
    for (int i = 0; i < 16; i++) {
        int addr6By8Bit = addrStr.mid(i * 2, 2).toInt(&ok, 16);
        localIp6Addr[i] = uint8_t(addr6By8Bit);
    }
}

// 比较ipv6地址，相同返回真
bool cmpIpv6Addr(const uint8_t addrA[16], const uint8_t addrB[16])
{
    bool ret = true;
    int size = 16;
    int count = 0;

    //确定元素是否包含相同的数据
    while (count < size) {
        if (addrA[count] != addrB[count]) {
            ret = false;
            break;
        }
        count++;
    }

    return ret;
}

// 判断字符是否为纯数字
bool is_number_t(const char *string)
{
    while (*string) {
        if (!isdigit(*string))
            return false;
        string++;
    }
    return true;
}

bool checkDbusServiceName(const QString &service)
{
    bool retIsValid = false;
    // 不可空
    if (service.isEmpty()) {
        retIsValid = false;
        return retIsValid;
    }

    // 不可包含除‘.’之外的符号
    for (const QChar &charUnit : service) {
        if (charUnit.isSymbol() && ('.' != charUnit)) {
            retIsValid = false;
            return retIsValid;
        }
    }

    // 不可以符号开头
    QChar firstChar = service.front();
    if (firstChar.isSymbol()) {
        retIsValid = false;
        return retIsValid;
    }
    // 服务名称中必须包含‘.’
    if (!service.contains(".")) {
        retIsValid = false;
        return retIsValid;
    }
    // 不可以符号结尾
    QChar lastChar = service.back();
    if (lastChar.isSymbol()) {
        retIsValid = false;
        return retIsValid;
    }

    retIsValid = true;
    return retIsValid;
}

bool checkDbusServicePath(const QString &path)
{
    bool retIsValid = false;
    // 不可空
    if (path.isEmpty()) {
        retIsValid = false;
        return retIsValid;
    }

    // 不可包含除‘/’之外的符号
    for (const QChar &charUnit : path) {
        if (charUnit.isSymbol() && ('/' != charUnit)) {
            retIsValid = false;
            return retIsValid;
        }
    }

    // 必须以‘/’开头
    QChar firstChar = path.front();
    if ('/' != firstChar) {
        retIsValid = false;
        return retIsValid;
    }

    // 服务名称中必须包含‘/’
    if (!path.contains("/")) {
        retIsValid = false;
        return retIsValid;
    }

    // 不可以符号结尾
    QChar lastChar = path.back();
    if (lastChar.isSymbol()) {
        retIsValid = false;
        return retIsValid;
    }

    retIsValid = true;
    return retIsValid;
}
