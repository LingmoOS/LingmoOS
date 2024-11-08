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

#ifndef SETTINGS_CHILDSETTINGS_H
#define SETTINGS_CHILDSETTINGS_H

#include "lingmo-ai/config.h"
#include "settings/settings.h"

#include <map>
#include <string>

#include <jsoncpp/json/json.h>

namespace aisdk
{

class ChildSettings : public Settings {
public:
    explicit ChildSettings(const char* schema);
    ~ChildSettings() override;

    DeployPolicy getDeployPolicy() const;
    bool setDeployPolicy(DeployPolicy policy);
    void resetDeployPolicy();

    std::string getModel(DeployPolicy policy) const;
    bool setModel(DeployPolicy policy, const std::string& localModel);
    void resetModel(DeployPolicy policy);

    Json::Value getModelConfig(DeployPolicy policy, const std::string& model) const;
    bool setModelConfig(DeployPolicy policy, const std::string& model, const std::string& json);
    void resetModelConfig(DeployPolicy policy, const std::string& model);

private:
    bool loadSettingsJsonValue(const char* key, Json::Value& root) const;
    void saveSettingsJsonValue(const char* key, const Json::Value& root);

    std::string mergeJson(const std::string& oldJsonStr,
                          const std::string& newJsonStr);
};

}

#endif
