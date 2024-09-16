// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOKENCACHE_H
#define TOKENCACHE_H

#include "string/string_utils.h"
#include "utility/singleton.h"

#include <iostream>
#include <map>
#include <mutex>

class TokenCache : public CppCommon::Singleton<TokenCache>
{
    friend CppCommon::Singleton<TokenCache>;

public:
    std::string genToken(std::string info);

    bool verifyToken(std::string &token);

    std::vector<std::string> getWebfromToken(const std::string &token);

private:
    std::mutex _cache_lock;
    std::map<std::string, std::string, std::less<>> _cache;
};

#endif // TOKENCACHE_H
