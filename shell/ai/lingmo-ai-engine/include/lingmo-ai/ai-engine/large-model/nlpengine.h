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

#ifndef NLPENGINE_H
#define NLPENGINE_H

#include <functional>
#include <string>
#include "aiengineerror.h"

namespace ai_engine {
namespace lm {
namespace nlp {

struct ChatResult {
    std::string text;
    EngineError error;
};

using ChatResultCallback = std::function<void(ChatResult result)>;

class AbstractNlpEngine {
public:
    virtual ~AbstractNlpEngine() {}
    virtual std::string engineName() = 0;
    virtual bool isCloud() = 0;

    virtual void setChatResultCallback(ChatResultCallback callback) = 0;

    virtual void setContextSize(int size) = 0;
    virtual void clearContext() = 0;

    virtual bool initChatModule(ai_engine::lm::EngineError &error) = 0;
    virtual bool chat(const std::string &message, ai_engine::lm::EngineError &error) = 0;
    virtual void stopChat() = 0;
    virtual bool destroyChatModule(ai_engine::lm::EngineError &error) = 0;
};

} // namespace nlp
} // namespace lm
} // namespace ai_engine

#endif // NLPENGINE_H
