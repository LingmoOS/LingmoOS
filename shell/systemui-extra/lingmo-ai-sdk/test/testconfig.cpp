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

#include <assert.h>

#include <cstring>
#include <string>

void testGetCapabilityList() {
    const auto length = get_capability_list_length();
    assert(length == 3);
    auto* capabilityList = new Capability[length];

    auto result = get_capability_list(capabilityList);
    assert(result == ErrorCode::CONFIG_SUCCEEDED);

    for (unsigned int index = 0; index < length; index++) {
        assert(capabilityList[index] == index);
    }

    delete[] capabilityList;

}

static void testGlobalSettings() {
    global_settings_reset_enabled();
    assert(global_settings_is_enabled());

    assert(global_settings_set_enabled(false));
    assert(!global_settings_is_enabled());

    global_settings_reset_enabled();
}

static void testCapabilitySettingssEnabled(Capability capability) {
    capability_settings_reset_enabled(capability);
    assert(capability_settings_is_enabled(capability));

    assert(capability_settings_set_enabled(capability, false));
    assert(!capability_settings_is_enabled(capability));

    capability_settings_reset_enabled(capability);
}

static void testCapabilitySettingssDeployPolicy(Capability capability) {
    capability_settings_reset_deploy_policy(capability);
    assert(capability_settings_get_deploy_policy(capability) == PUBLIC_CLOUD);

    assert(capability_settings_set_deploy_policy(capability ,LOCAL));
    assert(capability_settings_get_deploy_policy(capability) == LOCAL);

    assert(capability_settings_set_deploy_policy(capability, PRIVATE_CLOUD));
    assert(capability_settings_get_deploy_policy(capability) == PRIVATE_CLOUD);

    assert(!capability_settings_set_deploy_policy(capability, DEPLOY_POLICY_INVALID));
    assert(capability_settings_get_deploy_policy(capability) == PRIVATE_CLOUD);

    capability_settings_reset_deploy_policy(capability);
}

static void testCapabilitySettingssLocalModel(Capability capability) {
  const auto listLength = capability_settings_get_model_list_length(capability, LOCAL);
    assert(listLength == 0);
    if (listLength != 0) {
        for (unsigned long index = 0; index < listLength; index++) {
            auto length = capability_settings_get_model_length_by_index(capability, LOCAL, index);
            assert(length != 0);
            char* localModel = new char[length];
            assert(capability_settings_get_model_by_index(capability, LOCAL, index, localModel) == CONFIG_SUCCEEDED);
            delete[] localModel;
        }
    }

    capability_settings_reset_model(capability, LOCAL);
    assert(capability_settings_get_model_length(capability, LOCAL) == 0);

    const std::string newLocalModel("newLocalModel");
    assert(capability_settings_set_model(capability, LOCAL, newLocalModel.c_str()) == CONFIG_SUCCEEDED);
    auto newlocalModelLength = capability_settings_get_model_length(capability, LOCAL);
    assert(newlocalModelLength == newLocalModel.length());
    char* localModel = new char[newlocalModelLength];
    auto result = capability_settings_get_model(capability, LOCAL, localModel);
    assert(result == CONFIG_SUCCEEDED);
    assert(std::strncmp(newLocalModel.c_str(), localModel, newlocalModelLength) == 0);

    delete[] localModel;

    capability_settings_reset_model(capability, LOCAL);
}

static void testCapabilitySettingssPublicCloudModel(Capability capability) {
   const auto listLength = capability_settings_get_model_list_length(capability, PUBLIC_CLOUD);
    assert(listLength == 2);
    if (listLength != 0) {
        for (unsigned long index = 0; index < listLength; index++) {
            auto length = capability_settings_get_model_length_by_index(capability, PUBLIC_CLOUD, index);
            assert(length != 0);
            char* publicCloudModel = new char[length];
            assert(capability_settings_get_model_by_index(capability, PUBLIC_CLOUD, index, publicCloudModel) == CONFIG_SUCCEEDED);
            delete[] publicCloudModel;
        }
    }

    capability_settings_reset_model(capability, PUBLIC_CLOUD);
    assert(capability_settings_get_model_length(capability, PUBLIC_CLOUD) == 0);

    const std::string newPublicCloudModel("newPublicCloudModel");
    assert(capability_settings_set_model(capability, PUBLIC_CLOUD, newPublicCloudModel.c_str()) == CONFIG_SUCCEEDED);
    auto newPublicCloudModelLength = capability_settings_get_model_length(capability, PUBLIC_CLOUD);
    assert(newPublicCloudModelLength == newPublicCloudModel.length());
    char* publicCloudModel = new char[newPublicCloudModelLength];
    auto result = capability_settings_get_model(capability, PUBLIC_CLOUD, publicCloudModel);
    assert(result == CONFIG_SUCCEEDED);
    assert(std::strncmp(newPublicCloudModel.c_str(), publicCloudModel, newPublicCloudModelLength) == 0);

    delete[] publicCloudModel;

    capability_settings_reset_model(capability, PUBLIC_CLOUD);
}

static void testCapabilitySettingssPublicCloudModelConfig(Capability capability) {
    capability_settings_reset_model_config(capability, PUBLIC_CLOUD, "baidu");
    assert(capability_settings_get_model_config_length(capability, PUBLIC_CLOUD, "baidu") == 0);

    std::string config = R"(
    {
        "apiKey": "fakeApiKey",
        "secretKey" : "fakeSecretKey"
    }
    )";
    capability_settings_set_model(capability, PUBLIC_CLOUD, "newPublicCloudModel");
    assert(capability_settings_set_model_config(capability, PUBLIC_CLOUD, "newPublicCloudModel", config.c_str()) == CONFIG_SUCCEEDED);
    assert(capability_settings_get_model_config_length(capability, PUBLIC_CLOUD, "newPublicCloudModel") != 0);

    capability_settings_reset_model(capability, PUBLIC_CLOUD);
    capability_settings_reset_model_config(capability, PUBLIC_CLOUD, "baidu");
}

static void testCapabilitySettingssPrivateCloudModel(Capability capability) {
   const auto listLength = capability_settings_get_model_list_length(capability, PRIVATE_CLOUD);
    assert(listLength == 0);
    if (listLength != 0) {
        for (unsigned long index = 0; index < listLength; index++) {
            auto length = capability_settings_get_model_length_by_index(capability, PRIVATE_CLOUD, index);
            assert(length != 0);
            char* privateCloudModel = new char[length];
            assert(capability_settings_get_model_by_index(capability, PRIVATE_CLOUD, index, privateCloudModel) == CONFIG_SUCCEEDED);
            delete[] privateCloudModel;
        }
    }

    capability_settings_reset_model(capability, PRIVATE_CLOUD);
    assert(capability_settings_get_model_length(capability, PRIVATE_CLOUD) == 0);

    const std::string newPrivateCloudModel("newPrivateCloudModel");
    assert(capability_settings_set_model(capability, PRIVATE_CLOUD, newPrivateCloudModel.c_str()) == CONFIG_SUCCEEDED);
    auto newPrivateCloudModelLength = capability_settings_get_model_length(capability, PRIVATE_CLOUD);
    assert(newPrivateCloudModelLength == newPrivateCloudModel.length());
    char* privateCloudModel = new char[newPrivateCloudModelLength];
    auto result = capability_settings_get_model(capability, PRIVATE_CLOUD, privateCloudModel);
    assert(result == CONFIG_SUCCEEDED);
    assert(std::strncmp(newPrivateCloudModel.c_str(), privateCloudModel, newPrivateCloudModelLength) == 0);

    delete[] privateCloudModel;

    capability_settings_reset_model(capability, PRIVATE_CLOUD);
}

static void testCapabilitySettingss(Capability capability) {
    testCapabilitySettingssEnabled(capability);

    testCapabilitySettingssDeployPolicy(capability);

    testCapabilitySettingssLocalModel(capability);

    testCapabilitySettingssPublicCloudModel(capability);

    testCapabilitySettingssPublicCloudModelConfig(capability);

    testCapabilitySettingssPrivateCloudModel(capability);
}

void testCapabilitiesSettings() {
    const auto length = get_capability_list_length();
    auto* capabilityList = new Capability[length];

    auto result = get_capability_list(capabilityList);
    assert(result == ErrorCode::CONFIG_SUCCEEDED);

    for (unsigned int index = 0; index < length; index++) {
        testCapabilitySettingss(capabilityList[index]);
    }

    delete[] capabilityList;
}

int main(int argc, char* argv[]) {
    testGetCapabilityList();

    testGlobalSettings();

    testCapabilitiesSettings();

    return 0;
}
