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

#include "modelmanager.h"

#include <cassert>
#include <fstream>

#include <jsoncpp/json/json.h>

#include "baiduremotemodel.h"

static void testEmpty() {
    auto& manager = ModelManager::getInstance();

    const char* nlpConfigJson = u8R"(
        {
            "apiKey": "fakeNlpApiKey",
            "secretKey": "fakeNlpSecretKey"
        }
    )";
    manager.setRemoteModelConfig(BAIDU, NLP, nlpConfigJson);

    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(manager.getRemoteModelByVendor(BAIDU));
    assert(baiduRemoteModel->getApiKey(NLP) == "fakeNlpApiKey");
    assert(baiduRemoteModel->getSecretKey(NLP) == "fakeNlpSecretKey");

    const char* visionConfigJson = u8R"(
        {
            "apiKey": "fakeVisionApiKey",
            "secretKey": "fakeVisionSecretKey"
        }
    )";
    manager.setRemoteModelConfig(BAIDU, VISION, visionConfigJson);
        assert(baiduRemoteModel->getApiKey(VISION) == "fakeVisionApiKey");
    assert(baiduRemoteModel->getSecretKey(VISION) == "fakeVisionSecretKey");

}

static void testNonEmpty() {
    auto& manager = ModelManager::getInstance();

    const char* nlpConfigJson = u8R"(
        {
            "apiKey": "fakeNlpApiKey2",
            "secretKey": "fakeNlpSecretKey2"
        }
    )";
    manager.setRemoteModelConfig(BAIDU, NLP, nlpConfigJson);

    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(manager.getRemoteModelByVendor(BAIDU));
    assert(baiduRemoteModel->getApiKey(NLP) == "fakeNlpApiKey2");
    assert(baiduRemoteModel->getSecretKey(NLP) == "fakeNlpSecretKey2");

    const char* visionConfigJson = u8R"(
        {
            "apiKey": "fakeVisionApiKey2",
            "secretKey": "fakeVisionSecretKey2"
        }
    )";
    manager.setRemoteModelConfig(BAIDU, VISION, visionConfigJson);
        assert(baiduRemoteModel->getApiKey(VISION) == "fakeVisionApiKey2");
    assert(baiduRemoteModel->getSecretKey(VISION) == "fakeVisionSecretKey2");
}

void testGetRemoteModelConfig() {
    std::ifstream in(ModelManager::getConfigFilePath());
    if (!in.is_open()) {
        return;
    }

    Json::Value localConfig;

    in >> localConfig;

    auto& manager = ModelManager::getInstance();
    std::string config = manager.getRemoteModelConfig();

    assert(config == localConfig.toStyledString());
}

void testInvalidKey() {
    auto& manager = ModelManager::getInstance();

    const char* nlpInvalidConfigJson = u8R"(
        {
            "wrongapiKey": "fakeNlpApiKey",
            "secretKey": "fakeNlpSecretKey"
        }
    )";

    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(manager.getRemoteModelByVendor(BAIDU));
    std::string apiKey = baiduRemoteModel->getApiKey(NLP);
    std::string secretkey = baiduRemoteModel->getSecretKey(NLP);

    bool success = manager.setRemoteModelConfig(BAIDU, NLP, nlpInvalidConfigJson);
    assert(!success);
    assert(baiduRemoteModel->getApiKey(NLP) == apiKey);
    assert(baiduRemoteModel->getSecretKey(NLP) == secretkey);
}

void testRemoveRemoteModelConfig() {
    auto& manager = ModelManager::getInstance();
    const char* nlpConfigJson = u8R"(
        {
            "apiKey": "testModelManagerTestRemoveRemoteModelConfigApiKey",
            "secretKey": "testModelManagerTestRemoveRemoteModelConfigSecretKey"
        }
    )";
    manager.setRemoteModelConfig(BAIDU, NLP, nlpConfigJson);

    const auto* baiduRemoteModel =
         dynamic_cast<const BaiduRemoteModel*>(manager.getRemoteModelByVendor(BAIDU));
    assert(baiduRemoteModel->getApiKey(NLP) == "testModelManagerTestRemoveRemoteModelConfigApiKey");
    assert(baiduRemoteModel->getSecretKey(NLP) == "testModelManagerTestRemoveRemoteModelConfigSecretKey");

    manager.removeRemoteModelConfig(BAIDU, NLP);

    assert(baiduRemoteModel->getApiKey(NLP) == "");
    assert(baiduRemoteModel->getSecretKey(NLP) == "");
}

int main(int argc, char* argv[]) {
    auto& manager = ModelManager::getInstance();

    assert(manager.getDefaultRemoteModelVendor() == BAIDU);

    assert(manager.getRemoteModelVendorList().size() == 1);
    assert(manager.getRemoteModelVendorList()[0] == manager.getDefaultRemoteModelVendor());

    assert(manager.getDefaultRemoteModel() != nullptr);
    assert(manager.getDefaultRemoteModel()->getVendor() == BAIDU);

    assert(manager.getRemoteModelList().size() == 1);
    assert(manager.getRemoteModelList()[0]->getVendor() == manager.getDefaultRemoteModel()->getVendor());

    std::ifstream in(ModelManager::getConfigFilePath());
    if (!in.is_open()) {
        testEmpty();
    } else {
        testNonEmpty();
    }

    testGetRemoteModelConfig();

    testInvalidKey();

    testRemoveRemoteModelConfig();

    return 0;
}