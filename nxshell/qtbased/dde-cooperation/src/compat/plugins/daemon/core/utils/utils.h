// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CORE_UTIL_H
#define CORE_UTIL_H

#include <stdint.h>
#include <string.h>
#include <regex>
#include "co/rand.h"
#include "co/hash.h"
#include "co/str.h"
#include "common/constant.h"
#include "common/commonutils.h"

#include <QDir>
#include <QUuid>
#include <QStandardPaths>
#include <QHostInfo>
#include <QNetworkInterface>

class Util
{
public:
    static std::string genRandPin()
    {
        int pin_len = 6;
        return std::string(co::randstr("0123456789", pin_len).c_str());
    }

    static std::string genAuthToken(const char *uuid, const char *pin)
    {
        std::string all = uuid;   // + pin;
        fastring encodes = base64_encode(all);
        return std::string(encodes.c_str());
    }

    static bool checkToken(const char *token) { return true; }

    static std::string decodeBase64(const char *str)
    {
        fastring decodes = base64_decode(str);
        return std::string(decodes.c_str());
    }

    static std::string encodeBase64(const char *str)
    {
        fastring encodes = base64_encode(str);
        return std::string(encodes.c_str());
    }

    static std::string getFirstIp()
    {
        return deepin_cross::CommonUitls::getFirstIp();
    }

    static std::string getHostname(void)
    {
        QString hostName = QHostInfo::localHostName();
        return hostName.toStdString();
    }

    static std::string getUsername()
    {
        QString userName = QStandardPaths::displayName(QStandardPaths::HomeLocation);
        return userName.toStdString();
    }

    static int getOSType()
    {
#ifdef _WIN32
        return WINDOWS;
#elif __linux__
        return LINUX;
#endif
    }

    static QString configPath()
    {
        QDir winInfoPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        if (!winInfoPath.exists()) {
            winInfoPath.mkpath(winInfoPath.absolutePath());
        }

        QString winInfoFilePath(winInfoPath.filePath("cooperation-config.conf"));
        return winInfoFilePath;
    }

    static QString barrierConfig()
    {
        QDir winInfoPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        if (!winInfoPath.exists()) {
            winInfoPath.mkpath(winInfoPath.absolutePath());
        }

        QString winInfoFilePath(winInfoPath.filePath("cooperation-barrier.conf"));
        return winInfoFilePath;
    }

    static std::string genUUID()
    {
        QString hostid = QUuid::createUuid().toString(QUuid::Id128);
        return hostid.toStdString();
    }

    static bool isValidUUID(const std::string &uuid)
    {
        // UUID正则表达式模式
        std::regex pattern(
                R"([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");

        // 使用正则表达式匹配UUID
        return std::regex_match(uuid, pattern);
    }

    static std::string parseFileName(const char *path)
    {
        fastring file_name;
        co::vector<fastring> path_slips = str::split(path, '/');
        file_name = path_slips.pop_back();

        return std::string(file_name.c_str());
    }
};

#endif
