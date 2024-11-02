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

#include "settings/childsettings.h"

#include <map>

#include <json-glib/json-glib.h>

namespace aisdk
{

static const char* deployPolicyKey = "deploy-policy";
static const char* policyModelMapKey = "policy-model-map";
static const char* modelConfigKey = "model-config";

static const char* getDeployPolicyName(DeployPolicy policy) {
    static std::map<DeployPolicy, const char*> deployPolicyMap = {
        { LOCAL, "LOCAL"},
        { PUBLIC_CLOUD, "PUBLIC_CLOUD"},
        { PRIVATE_CLOUD, "PRIVATE_CLOUD"}
    };

    return deployPolicyMap.at(policy);
}


ChildSettings::ChildSettings(const char* schema) : Settings(schema) {}

ChildSettings::~ChildSettings() = default;

DeployPolicy ChildSettings::getDeployPolicy() const {
    auto policy = g_settings_get_enum(&settings_, deployPolicyKey);
    if (policy < DEPLOY_POLICY_FIRST || policy >= DEPLOY_POLICY_COUNT) {
        return DEPLOY_POLICY_INVALID;
    }

    return static_cast<DeployPolicy>(policy);
}

bool ChildSettings::setDeployPolicy(DeployPolicy policy) {
    return g_settings_set_enum(&settings_, deployPolicyKey, static_cast<int>(policy));
}

void ChildSettings::resetDeployPolicy() {
    g_settings_reset(&settings_, deployPolicyKey);
}

std::string ChildSettings::getModel(DeployPolicy policy) const {
    Json::Value root;
    if (!loadSettingsJsonValue(policyModelMapKey, root)) {
        return "";
    }

    if (!root.isMember(getDeployPolicyName(policy))) {
        return "";
    }

    return root[getDeployPolicyName(policy)].asString();
}

bool ChildSettings::setModel(DeployPolicy policy, const std::string& model) {
    Json::Value root;
    if (!loadSettingsJsonValue(policyModelMapKey, root)) {
        return false;
    }

    root[getDeployPolicyName(policy)] = model;

    saveSettingsJsonValue(policyModelMapKey, root);

    return true;
}

void ChildSettings::resetModel(DeployPolicy policy) {
    Json::Value root;
    if (!loadSettingsJsonValue(policyModelMapKey, root)) {
        return;
    }

    if (!root.isMember(getDeployPolicyName(policy))) {
        return;
    }

    root[getDeployPolicyName(policy)] = "";

    saveSettingsJsonValue(policyModelMapKey, root);
}

Json::Value ChildSettings::getModelConfig(DeployPolicy policy, const std::string& model) const {
    Json::Value root;
    if (!loadSettingsJsonValue(modelConfigKey, root)) {
        return Json::Value();
    }

    if (!root.isMember(getDeployPolicyName(policy)) || !root[getDeployPolicyName(policy)].isMember(model)) {
        return Json::Value();
    }

    return root[getDeployPolicyName(policy)][model];
}

bool ChildSettings::setModelConfig(DeployPolicy policy, const std::string& model, const std::string& json) {
    Json::Reader reader;
    Json::Value config;

    Json::Value root;
    if (!loadSettingsJsonValue(modelConfigKey, root)) {
        return false;
    }

    std::string oldJsonStr = getModelConfig(policy, model).toStyledString();
    if (!reader.parse(mergeJson(oldJsonStr, json), config)) {
        return false;
    }

    root[getDeployPolicyName(policy)][model] = config;

    saveSettingsJsonValue(modelConfigKey, root);

    return true;
}

void ChildSettings::resetModelConfig(DeployPolicy policy, const std::string& model) {
    Json::Value root;
    if (!loadSettingsJsonValue(modelConfigKey, root)) {
        return;
    }

    if (!root.isMember(getDeployPolicyName(policy)) || !root[getDeployPolicyName(policy)].isMember(model)) {
        return;
    }

    root[getDeployPolicyName(policy)][model].clear();

    saveSettingsJsonValue(modelConfigKey, root);
}

bool ChildSettings::loadSettingsJsonValue(const char* key, Json::Value& root) const {
    GVariant* value = g_settings_get_value(&settings_, key);

    char* data = json_gvariant_serialize_data(value, nullptr);
    g_variant_unref(value);

    std::string json(data);
    g_free(data);

    Json::Reader reader;
    return reader.parse(json, root);
}

void ChildSettings::saveSettingsJsonValue(const char* key, const Json::Value& root) {
    GVariant* value = json_gvariant_deserialize_data(root.toStyledString().c_str(), -1, nullptr, nullptr);

    g_settings_set_value(&settings_, key, value);
}

std::string ChildSettings::mergeJson(
        const std::string& oldJsonStr, const std::string& newJsonStr) {
    Json::Value oldRoot, newRoot, mergedRoot;
    Json::Reader reader;

    bool oldParseSuccess = reader.parse(oldJsonStr, oldRoot);
    if (oldParseSuccess) {
        for (auto it = oldRoot.begin(); it != oldRoot.end(); ++it) {
            mergedRoot[it.key().asString()] = it->asString();
        }
    }

    bool newParseSuccess = reader.parse(newJsonStr, newRoot);
    if (newParseSuccess) {
        for (auto it = newRoot.begin(); it != newRoot.end(); ++it) {
            mergedRoot[it.key().asString()] = it->asString();
        }
    }

    return mergedRoot.toStyledString();
}

}
