/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NLPSESSION_H
#define NLPSESSION_H

#include <string.h>
#include <jsoncpp/json/json.h>
#include "nlp.h"

class NLPSession_
{
public:
    NLPSession_() = default;
    virtual ~NLPSession_() = default;

    void setJsonCallback(NlpCallback callback);
    void setUserData(void* userData);
    NlpCallback getJsonCallback() const;
    void* getUserData() const;

    virtual NlpResult chat(const char* message) = 0;
    virtual std::string lastErrorMessage() const = 0;

    void setContextSize(int size);
    void clearContext();
    Json::Value& getContext();

private:
    NlpCallback jsonCallback_ {nullptr};
    void* userData_ {nullptr};

    int contextSize_ {-1};
    Json::Value context_;
};

#endif
