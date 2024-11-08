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

#include "aienginemanager.h"

namespace ai_engine {

std::unique_ptr<ai_engine::lm::nlp::AbstractNlpEngine> AiEngineManager::createLmNlpEngine() {
    return enginePluginManager_.createNlpEngine(engineConfig_.currentLmNlpEngineName());
}

std::unique_ptr<lm::speech::AbstractSpeechEngine> AiEngineManager::createLmSpeechEngine() {
    return enginePluginManager_.createSpeechEngine(engineConfig_.currentLmSpeechEngineName());
}

std::unique_ptr<lm::vision::AbstractVisionEngine> AiEngineManager::createLmVisionEngine() {
    return enginePluginManager_.createVisionEngine(engineConfig_.currentLmVisionEngineName());
}

std::unique_ptr<config::PromptTemplate> AiEngineManager::promptTemplate() {
    return std::unique_ptr<config::PromptTemplate>(
        new config::PromptTemplate(
            engineConfig_.promptTemplateConfigFile(),
            engineConfig_.promptFilePath()));
}

} // namespace ai_engine
