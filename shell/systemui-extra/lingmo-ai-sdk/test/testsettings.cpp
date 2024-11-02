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

#include <assert.h>

#include "settings/globalsettings.h"

using namespace aisdk;

static void testGlobalSettings() {
    GlobalSettings::getInstance().resetEnabled();
    assert(GlobalSettings::getInstance().isEnabled());

    assert(GlobalSettings::getInstance().setEnabled(false));
    assert(!GlobalSettings::getInstance().isEnabled());

    GlobalSettings::getInstance().resetEnabled();
}

static void testChildSettings(ChildSettings& childSettings) {
    childSettings.resetEnabled();
    assert(childSettings.isEnabled());

    assert(childSettings.setEnabled(false));
    assert(!childSettings.isEnabled());

    childSettings.resetEnabled();

    childSettings.resetDeployPolicy();
    assert(childSettings.getDeployPolicy() == PUBLIC_CLOUD);

    assert(childSettings.setDeployPolicy(LOCAL));
    assert(childSettings.getDeployPolicy() == LOCAL);

    assert(childSettings.setDeployPolicy(PRIVATE_CLOUD));
    assert(childSettings.getDeployPolicy() == PRIVATE_CLOUD);

    assert(!childSettings.setDeployPolicy(DEPLOY_POLICY_INVALID));
    assert(childSettings.getDeployPolicy() == PRIVATE_CLOUD);

    childSettings.resetDeployPolicy();

    childSettings.resetModel(LOCAL);
    assert(childSettings.getModel(LOCAL).empty());

    assert(childSettings.setModel(LOCAL, "newLocalModel"));
    assert(childSettings.getModel(LOCAL) == "newLocalModel");

    childSettings.resetModel(LOCAL);

    childSettings.resetModel(PUBLIC_CLOUD);
    assert(childSettings.getModel(PUBLIC_CLOUD).empty());

    assert(childSettings.setModel(PUBLIC_CLOUD, "newPublicCloudModel"));
    assert(childSettings.getModel(PUBLIC_CLOUD) == "newPublicCloudModel");

    childSettings.resetModel(PUBLIC_CLOUD);

    childSettings.resetModelConfig(PUBLIC_CLOUD, "newPublicCloudModel");
    assert(childSettings.getModelConfig(PUBLIC_CLOUD, "newPublicCloudModel").empty());

    std::string config = R"(
    {
        "apiKey": "fakeApiKey",
        "secretKey" : "fakeSecretKey"
    }
    )";
    childSettings.setModel(PUBLIC_CLOUD, "newPublicCloudModel");
    assert(childSettings.setModelConfig(PUBLIC_CLOUD, "newPublicCloudModel", config));
    assert(!childSettings.getModelConfig(PUBLIC_CLOUD, "newPublicCloudModel").empty());

    childSettings.resetModel(PUBLIC_CLOUD);
    childSettings.resetModelConfig(PUBLIC_CLOUD, "newPublicCloudModel");

    childSettings.resetModel(PRIVATE_CLOUD);
    assert(childSettings.getModel(PRIVATE_CLOUD).empty());

    assert(childSettings.setModel(PRIVATE_CLOUD, "newPrivateCloudModel"));
    assert(childSettings.getModel(PRIVATE_CLOUD) == "newPrivateCloudModel");

    childSettings.resetModel(PRIVATE_CLOUD);
}

static void testTextSettings() {
    testChildSettings(GlobalSettings::getInstance().getChildSettings(CAPABILITY_NLP));
}

static void testSpeechSettings() {
    testChildSettings(GlobalSettings::getInstance().getChildSettings(CAPABILITY_SPEECH));
}

static void testVisionSettings() {
    testChildSettings(GlobalSettings::getInstance().getChildSettings(CAPABILITY_VISION));
}

int main(int argc, char* argv[]) {
    testGlobalSettings();

    testTextSettings();

    testSpeechSettings();

    testVisionSettings();

    return 0;
}
