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

#ifndef AIENGINE_TM_H
#define AIENGINE_TM_H

#include <string>
#include <vector>
#include <memory>

namespace ai_engine {
namespace tm {
enum class AiCapability
{
    Ocr,
};

class AbstractAiEnginePluginFactory
{
public:
    virtual ~AbstractAiEnginePluginFactory() {}
    virtual bool isCloud() = 0;
    virtual std::string engineName() const = 0;
    virtual std::vector<AiCapability> aiCapabilities() const = 0;
};

}
}

#define AI_TRADITIONAL_MODEL_ENGINE_FACTORY(ClassName)                                              \
extern "C" {                                                                                        \
    ::ai_engine::tm::AbstractAiEnginePluginFactory *ai_traditional_model_engine_factory_instance() {\
        static ClassName factory;                                                                   \
        return &factory;                                                                            \
}                                                                                                   \
}

#endif // AIENGINE_H
