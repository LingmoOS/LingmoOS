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

#include <cstdlib>
#include <fstream>

#include "baiduremotemodel.h"
#include "encryption.h"
#include "token.h"

static const std::string kConfigFile = "model_manager.json";
static const std::string kBaiduApiKey = "apiKey";
static const std::string kBaiduSecretKey = "secretKey";
static const std::string kBaiduVersion = "version";

std::string ModelManager::getConfigFilePath() {
    const char* homePath = std::getenv("HOME");
    if (homePath == nullptr) {
        return "";
    }

    return std::string(homePath) + "/.config/" + kConfigFile;
}


ModelManager::ModelManager() {
    loadConfig();

    initRemoteModelList();
}

std::unique_ptr<RemoteModel> ModelManager::createBaiduRemoteModel() {
    std::unique_ptr<BaiduRemoteModel> baiduRemoteModel(new BaiduRemoteModel());
    for (auto modelId : baiduRemoteModel->getModelIdList()) {
        updateBaiduModelKeys(baiduRemoteModel.get(), modelId);
    }

    return baiduRemoteModel;
}

bool ModelManager::updateBaiduModelKeys(RemoteModel* remoteModel, RemoteModelId modelId) {
    if (remoteModel == nullptr
        || remoteModel->getVendor() != BAIDU) {
        return false;
    }

    const auto baiduKey = std::to_string(BAIDU);
    if (!config_.isMember(baiduKey)) {
        return false;
    }

    Json::Value baiduConfig = config_[baiduKey];
    const auto modelKey = std::to_string(modelId);
    if (!baiduConfig.isMember(modelKey)) {
        return false;
    }

    if (!baiduConfig[modelKey].isMember(kBaiduVersion)) {
        baiduConfig[modelKey][kBaiduVersion] = modelId == VISION ? 2 : 1;
    }

    return updateBaiduModelKeys(remoteModel, modelId, baiduConfig[modelKey]);
}

#include <iostream>

bool ModelManager::updateBaiduModelKeys(RemoteModel* remoteModel, RemoteModelId modelId, const Json::Value& modelConfig) {
    if (!modelConfig.isMember(kBaiduApiKey)
        || !modelConfig[kBaiduApiKey].isString()
        || !modelConfig.isMember(kBaiduSecretKey)
        || !modelConfig[kBaiduSecretKey].isString()
        || !modelConfig.isMember(kBaiduVersion)
        || !modelConfig[kBaiduVersion].isInt()) {
            return false;
    }

    auto* baiduRemoteModel = static_cast<BaiduRemoteModel*>(remoteModel);
    const std::string apiKey = 
        Encryption::getInstance().decrypt(modelConfig[kBaiduApiKey].asCString());
    if (apiKey.empty()) {
        return false;
    }

    const std::string secretKey = 
        Encryption::getInstance().decrypt(modelConfig[kBaiduSecretKey].asCString());
    if (secretKey.empty()) {
        return false;
    }

    std::string errorMessage;
    bool hasNetError = false;
    if (apiKey.length() < 12 || secretKey.length() < 12 ||
        token::getBaiduToken(apiKey, secretKey, hasNetError, errorMessage).empty()) {
        fprintf(stderr, "%s", (const char *)errorMessage.c_str());
        return false;
    }

    int version = modelConfig[kBaiduVersion].asInt();
    baiduRemoteModel->setModelInfo(modelId, apiKey, secretKey, version);

    return true;
}

void ModelManager::initRemoteModelList() {
    for (auto vendor : remoteModelVendorList_) {
        if (vendor == BAIDU) {
            remoteModelList_.push_back(createBaiduRemoteModel());    
        }
    }
}

ModelManager& ModelManager::getInstance() {
    static ModelManager modelManager;

    return modelManager;
}

const RemoteModel* ModelManager::getRemoteModelByVendor(RemoteModelVendorId vendor) const {
    for (const auto& model : remoteModelList_) {
        if (model->getVendor() == vendor) {
            return model.get();
        }
    }

    return nullptr;
}

RemoteModel* ModelManager::getRemoteModelByVendor(const std::string& name) const {
    for (const auto& model : remoteModelList_) {
        if (std::to_string(model->getVendor()) == name) {
            return model.get();
        }
    }

    return nullptr;
}

bool ModelManager::setRemoteModelConfig(RemoteModelVendorId vendor, RemoteModelId modelId, 
    const std::string& json){
    if (!isRemoteModelVendorSupported(vendor)) {
        return false;
    }

    if (vendor == BAIDU) {
        return setBaiduRemoteModelConfig(modelId, json);
    }

    return false;
}

std::string ModelManager::getRemoteModelConfig() const {
    return getDecryptedConfig();
}

bool ModelManager::removeRemoteModelConfig(RemoteModelVendorId vendor, RemoteModelId modelId) {
    if (!isRemoteModelVendorSupported(vendor)) {
        return false;
    }

    if (vendor == BAIDU) {
        return removeBaiduRemoteModelConfig(modelId);
    }

    return false;
}

bool ModelManager::setBaiduRemoteModelConfig(RemoteModelId modelId, const std::string& json) {
    Json::Reader reader;
    Json::Value modelConfig;
    if (!reader.parse(json, modelConfig)) {
        return false;
    }

   if (!modelConfig.isMember(kBaiduApiKey)
        || !modelConfig[kBaiduApiKey].isString()
        || !modelConfig.isMember(kBaiduSecretKey)
        || !modelConfig[kBaiduSecretKey].isString()) {
            return false;
    }

    modelConfig[kBaiduApiKey] = 
        Encryption::getInstance().encrypt(modelConfig[kBaiduApiKey].asCString());
    modelConfig[kBaiduSecretKey] = 
        Encryption::getInstance().encrypt(modelConfig[kBaiduSecretKey].asCString());
    if (!modelConfig.isMember(kBaiduVersion)) {
        modelConfig[kBaiduVersion] = modelId == VISION ? 2 : 1;
    }
    if (!updateBaiduModelKeys(getRemoteModelByVendor(std::to_string(BAIDU)), modelId, modelConfig)) {
        return false;
    }

    config_[std::to_string(BAIDU)][std::to_string(modelId)] = modelConfig;

    saveConfig();

    return true;
}

bool ModelManager::removeBaiduRemoteModelConfig(RemoteModelId modelId) {
    auto* remoteModel = getRemoteModelByVendor(std::to_string(BAIDU));
    auto* baiduRemoteModel = static_cast<BaiduRemoteModel*>(remoteModel);
    baiduRemoteModel->removeKeys(modelId);

    config_[std::to_string(BAIDU)].removeMember(std::to_string(modelId));

    saveConfig();

    return true;
}

bool ModelManager::isRemoteModelVendorSupported(RemoteModelVendorId vendor) const {
    for (auto remoteModelVendor : remoteModelVendorList_) {
        if (remoteModelVendor == vendor) {
            return true;
        }
    }

    return false;
}

const RemoteModel* ModelManager::getDefaultRemoteModel() const {
    return getRemoteModelByVendor(defaultRemoteModelVendor_);
}

void ModelManager::loadConfig() {
    const std::string& configFilePath = getConfigFilePath();
    if (configFilePath.empty()) {
        return;
    }

    std::ifstream in(configFilePath);
    if (!in.is_open()) {
        return;
    }

    config_.clear();

    in >> config_;
}

void ModelManager::saveConfig() const {
    const std::string& configFilePath = getConfigFilePath();
    if (configFilePath.empty()) {
        return;
    }

    std::ofstream out(configFilePath);
    if (!out.is_open()) {
        return;
    }

    out << config_;
}

std::string ModelManager::getDecryptedConfig() const {
    Json::Value unencryptedConfig_ = config_;
    std::string str;
    if (config_.isMember(std::to_string(BAIDU))) {
        if (config_[std::to_string(BAIDU)].isMember(std::to_string(NLP))) {
            str = Encryption::getInstance().
                decrypt(config_[std::to_string(BAIDU)][std::to_string(NLP)][kBaiduApiKey].asCString());
            unencryptedConfig_[std::to_string(BAIDU)][std::to_string(NLP)][kBaiduApiKey] =
                str.replace(6, str.size() - 12, "****");
            str = Encryption::getInstance().
                decrypt(config_[std::to_string(BAIDU)][std::to_string(NLP)][kBaiduSecretKey].asCString());
            unencryptedConfig_[std::to_string(BAIDU)][std::to_string(NLP)][kBaiduSecretKey] =
                str.replace(6, str.size() - 12, "****");
        }
        if (config_[std::to_string(BAIDU)].isMember(std::to_string(VISION))) {
            str = Encryption::getInstance().
                decrypt(config_[std::to_string(BAIDU)][std::to_string(VISION)][kBaiduApiKey].asCString());
            unencryptedConfig_[std::to_string(BAIDU)][std::to_string(VISION)][kBaiduApiKey] =
                str.replace(6, str.size() - 12, "****");
            str = Encryption::getInstance().
                decrypt(config_[std::to_string(BAIDU)][std::to_string(VISION)][kBaiduSecretKey].asCString());
            unencryptedConfig_[std::to_string(BAIDU)][std::to_string(VISION)][kBaiduSecretKey] =
                str.replace(6, str.size() - 12, "****");
        }
    }
    return unencryptedConfig_.toStyledString();
}
