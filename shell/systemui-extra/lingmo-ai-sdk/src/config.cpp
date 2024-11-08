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

#include "config.h"

#include <cstring>
#include <vector>

#include <jsoncpp/json/json.h>

#include "settings/globalsettings.h"

using namespace aisdk;

static Capability capabilityList[] = { CAPABILITY_NLP, CAPABILITY_SPEECH, CAPABILITY_VISION };
unsigned long get_capability_list_length() {
    return sizeof(capabilityList) / sizeof(capabilityList[0]);
}

ErrorCode get_capability_list(Capability* list) {
    if (list == nullptr) {
        return CONFIG_FAILED;
    }

    std::memcpy(list, capabilityList, sizeof(capabilityList));

    return CONFIG_SUCCEEDED;
}

void global_settings_reset_enabled() {
    GlobalSettings::getInstance().resetEnabled();
}

int global_settings_is_enabled() {
    return static_cast<int>(GlobalSettings::getInstance().isEnabled());
}

int global_settings_set_enabled(int enabled) {
    return static_cast<int>(GlobalSettings::getInstance().setEnabled(static_cast<bool>(enabled)));
}

void capability_settings_reset_enabled(Capability capability) {
    GlobalSettings::getInstance().getChildSettings(capability).resetEnabled();
}

int capability_settings_is_enabled(Capability capability) {
    return static_cast<int>(GlobalSettings::getInstance().getChildSettings(capability).isEnabled());
}

int capability_settings_set_enabled(Capability capability, int enabled) {
    return static_cast<int>(GlobalSettings::getInstance().getChildSettings(capability).setEnabled(static_cast<bool>(enabled)));
}

void capability_settings_reset_deploy_policy(Capability capability) {
    GlobalSettings::getInstance().getChildSettings(capability).resetDeployPolicy();
}

DeployPolicy capability_settings_get_deploy_policy(Capability capability) {
    return  GlobalSettings::getInstance().getChildSettings(capability).getDeployPolicy();
}

int capability_settings_set_deploy_policy(Capability capability, DeployPolicy policy) {
    return static_cast<bool>(GlobalSettings::getInstance().getChildSettings(capability).setDeployPolicy(policy));
}

static std::vector<std::string> publicCloudModeList{"baidu", "xunfei"};
unsigned long capability_settings_get_model_list_length(Capability capability, DeployPolicy policy) {
    if (policy == PUBLIC_CLOUD) {
        return publicCloudModeList.size();
    }

    return 0;
}

unsigned long capability_settings_get_model_length_by_index(Capability capability, DeployPolicy policy, unsigned long index) {
    if (policy == PUBLIC_CLOUD) {
        return publicCloudModeList[index].length();
    }

    return 0;
}

ErrorCode capability_settings_get_model_by_index(Capability capability, DeployPolicy policy, unsigned long index, char* model) {
    if (model == nullptr) {
        return CONFIG_FAILED;
    }

    if (index >= capability_settings_get_model_list_length(capability, policy)) {
        return CONFIG_FAILED;
    }

    if (policy == PUBLIC_CLOUD) {
        std::memcpy(model, publicCloudModeList[index].c_str(), publicCloudModeList[index].length());
    }

    return CONFIG_SUCCEEDED;
}

void capability_settings_reset_model(Capability capability, DeployPolicy policy) {
    GlobalSettings::getInstance().getChildSettings(capability).resetModel(policy);
}

unsigned long capability_settings_get_model_length(Capability capability, DeployPolicy policy) {
    return GlobalSettings::getInstance().getChildSettings(capability).getModel(policy).length();
}

ErrorCode capability_settings_get_model(Capability capability, DeployPolicy policy, char* model) {
    if (model == nullptr) {
        return CONFIG_FAILED;
    }

    auto modelStr = GlobalSettings::getInstance().getChildSettings(capability).getModel(policy);
    std::memcpy(model, modelStr.c_str(), modelStr.length());

    return CONFIG_SUCCEEDED;
}

ErrorCode capability_settings_set_model(Capability capability, DeployPolicy policy, const char* model) {
    if (model == nullptr) {
        return CONFIG_FAILED;
    }

    return GlobalSettings::getInstance().getChildSettings(capability).setModel(policy, model) ? CONFIG_SUCCEEDED : CONFIG_FAILED;
}

void capability_settings_reset_model_config(Capability capability, DeployPolicy policy, const char* model) {
    GlobalSettings::getInstance().getChildSettings(capability).resetModelConfig(policy, model);
}

unsigned long capability_settings_get_model_config_length(Capability capability, DeployPolicy policy, const char* model) {
    Json::Value jsonConfig = GlobalSettings::getInstance().getChildSettings(capability).getModelConfig(policy, model);
    if (jsonConfig.empty()) {
        return 0;
    }

    return jsonConfig.toStyledString().length();
}

ErrorCode capability_settings_get_model_config(Capability capability, DeployPolicy policy, const char* model, char* json) {
    if (json == nullptr) {
        return CONFIG_FAILED;
    }

    Json::Value jsonConfig =GlobalSettings::getInstance().getChildSettings(capability).getModelConfig(policy, model);
    if (jsonConfig.empty()) {
        return CONFIG_FAILED;
    }

    auto jsonStr = jsonConfig.toStyledString();
    std::memcpy(json, jsonStr.c_str(), jsonStr.length());

    return CONFIG_SUCCEEDED;
}

ErrorCode capability_settings_set_model_config(Capability capability, DeployPolicy policy, const char* model, const char* json) {
    if (json == nullptr) {
        return CONFIG_FAILED;
    }

    return GlobalSettings::getInstance().getChildSettings(capability).setModelConfig(policy, model, json) ? CONFIG_SUCCEEDED : CONFIG_FAILED;
}

unsigned long capability_settings_get_model_key_config_length(Capability capability, DeployPolicy policy, const char* model) {
    return 0;
}

ErrorCode capability_settings_get_model_key_config(Capability capability, DeployPolicy policy, const char* model, char* json) {
    return CONFIG_FAILED;
}
