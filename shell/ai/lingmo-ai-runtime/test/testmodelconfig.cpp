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

#include "modelconfig.h"

#include <cassert>
#include <fstream>
#include <string>

#include "baiduremotemodel.h"
#include "modelmanager.h"

void testGetRemoteModelConfigLength() {
    std::ifstream in(ModelManager::getConfigFilePath());
    if (!in.is_open()) {
        return;
    }

    Json::Value localConfig;

    in >> localConfig;

    assert(get_remote_model_config_length() == localConfig.toStyledString().length());
}

void testGetRemoteModelConfig() {
    std::ifstream in(ModelManager::getConfigFilePath());
    if (!in.is_open()) {
        return;
    }

    Json::Value localConfig;

    in >> localConfig;

    std::vector<char> buffer;
    buffer.reserve(get_remote_model_config_length());
    auto result = get_remote_model_config(buffer.data());
    assert(result == SUCCEEDED);
    std::string config(buffer.data(), buffer.data() + get_remote_model_config_length());

    assert(config == localConfig.toStyledString());
}

void testRemoveRemoteModelConfig() {
    auto& manager = ModelManager::getInstance();
    const char* nlpConfigJson = u8R"(
        {
            "apiKey": "testModelConfigTestRemoveRemoteModelConfigApiKey",
            "secretKey": "testModelConfigTestRemoveRemoteModelConfigSecretKey"
        }
    )";
    manager.setRemoteModelConfig(BAIDU, NLP, nlpConfigJson);

    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(manager.getRemoteModelByVendor(BAIDU));
    assert(baiduRemoteModel->getApiKey(NLP) == "testModelConfigTestRemoveRemoteModelConfigApiKey");
    assert(baiduRemoteModel->getSecretKey(NLP) == "testModelConfigTestRemoveRemoteModelConfigSecretKey");

    remove_remote_model_config(BAIDU, NLP);

    assert(baiduRemoteModel->getApiKey(NLP) == "");
    assert(baiduRemoteModel->getSecretKey(NLP) == "");
}

int main(int argc, char* argv[]) {
    assert(get_remote_model_vendor_list_length() == 1);

    assert(get_remote_model_vendor_list() != nullptr);

    assert(get_remote_model_vendor_list()[0] == BAIDU);

    assert(get_remote_model_list_length(BAIDU) == 2);

    assert(get_remote_model_list(BAIDU) != nullptr);

    assert(get_remote_model_list(BAIDU)[0] == NLP);

    assert(get_remote_model_list(BAIDU)[1] == VISION);

    assert(get_default_remote_model_vendor() == BAIDU);

    const char* baiduConfigJson = u8R"(
        {
            "apiKey": "fakeApiKey3",
            "secretKey": "fakeSecretKey3"
        }
    )";
    auto result = set_remote_model_config(BAIDU, NLP, baiduConfigJson);
    assert(result == SUCCEEDED);

    testGetRemoteModelConfigLength();

    testGetRemoteModelConfig();

    testRemoveRemoteModelConfig();

    return 0;
}