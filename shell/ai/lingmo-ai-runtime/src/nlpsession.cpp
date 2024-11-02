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

#include <nlpsession.h>

void NLPSession_::setJsonCallback(NlpCallback callback)
{
    jsonCallback_ = std::move(callback);
}

void NLPSession_::setUserData(void* userData)
{
    userData_ = std::move(userData);
}

NlpCallback NLPSession_::getJsonCallback() const
{
    return jsonCallback_;
}

void* NLPSession_::getUserData() const
{
    return userData_;
}

Json::Value& NLPSession_::getContext()
{
    return context_;
}

void NLPSession_::setContextSize(int size)
{
    contextSize_ = size;
}
    
void NLPSession_::clearContext()
{
    context_.clear();
}

