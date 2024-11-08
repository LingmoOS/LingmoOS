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

#ifndef ENGINECONFIGURATION_H
#define ENGINECONFIGURATION_H

#include <string>
#include <jsoncpp/json/json.h>

namespace config {

const std::string promptTemplateConfigFile(const std::string &engineName);
const std::string promptFilePath(const std::string &engineName);

class EngineConfiguration {
public:
    EngineConfiguration();
    EngineConfiguration(const EngineConfiguration &other) = delete;
    EngineConfiguration &operator=(const EngineConfiguration &rhs) = delete;

    /// \note
    /// Lm - large model
    /// Tm - traditional model
    /// Vd - vector database
    std::string currentLmSpeechEngineName() const;
    std::string currentLmNlpEngineName() const;
    std::string currentLmVisionEngineName() const;
    std::string currentTmOcrEngineName() const;
    std::string currentVdEngineName() const;

    std::string promptTemplateConfigFile() const;
    std::string promptFilePath() const;

private:
    void loadAiEngineConfig();
    void loadDbEngineConfig();

private:
    Json::Value aiEngineConfig_;
    Json::Value dbEngineConfig_;
    const std::string aiEngineConfigFile_ { "/etc/lingmo-ai/engines/ai-engine/engine.json" };
    const std::string dbEngineConfigFile_ { "/etc/lingmo-ai/engines/db-engine/engine.json" };
    const std::string promptTemplatesPath { "/etc/lingmo-ai/engines/ai-engine/prompt-templates" };
};

}

#endif // ENGINECONFIGURATION_H
