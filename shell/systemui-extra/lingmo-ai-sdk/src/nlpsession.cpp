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

#include "nlpsession.h"
#include <jsoncpp/json/value.h>


NlpResult nlp::NlpSession::init(const std::string &engineName, const Json::Value &config)
{
    return textProcessor_.initEngine(engineName, config);
}

void nlp::NlpSession::setResultCallback(NlpResultCallback callback, std::any userData)
{
    textProcessor_.setChatCallback(
                [callback, userData](NlpTextData result) {
        callback(result, std::any_cast<void *>(userData));
    });
}

void nlp::NlpSession::setActor(int actorId)
{
    textProcessor_.setActor(actorId);
}

NlpResult nlp::NlpSession::chat(const std::string &message)
{
    if (message.empty()) {
        return NlpResult::NLP_PARAM_ERROR;
    }
    return textProcessor_.chat(message);
}

void nlp::NlpSession::stopChat()
{
    textProcessor_.stopChat();
}

void nlp::NlpSession::setContext(int size)
{
    textProcessor_.setContext(size);
}

void nlp::NlpSession::clearContext()
{
    textProcessor_.clearContext();
}
