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

#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include <jsoncpp/json/json.h>

#include "remotemodel.h"

class ModelManager {
public:
    static ModelManager& getInstance();

    static std::string getConfigFilePath();

    ~ModelManager() = default;

    const std::vector<std::unique_ptr<RemoteModel>>& getRemoteModelList() const { return remoteModelList_; }
    const std::vector<RemoteModelVendorId>& getRemoteModelVendorList() const { return remoteModelVendorList_; }

    bool setRemoteModelConfig(RemoteModelVendorId vendor, RemoteModelId modelId, 
        const std::string& json);
    std::string getRemoteModelConfig() const;
    bool removeRemoteModelConfig(RemoteModelVendorId vendor, RemoteModelId modelId);

    RemoteModelVendorId getDefaultRemoteModelVendor() const { return defaultRemoteModelVendor_; }
    void setDefaultRemoteModelVendor(RemoteModelVendorId vendor) { defaultRemoteModelVendor_ = vendor; }
    
    const RemoteModel* getDefaultRemoteModel() const;
    const RemoteModel* getRemoteModelByVendor(RemoteModelVendorId vendor) const;
    RemoteModel* getRemoteModelByVendor(const std::string& name) const;

private:
    ModelManager();

    void loadConfig();
    void saveConfig() const;
    std::string getDecryptedConfig() const;

    bool isRemoteModelVendorSupported(RemoteModelVendorId vendor) const;
    bool setBaiduRemoteModelConfig(RemoteModelId modelId, const std::string& json);
    bool removeBaiduRemoteModelConfig(RemoteModelId modelId);

    bool updateBaiduModelKeys(RemoteModel* remoteModel, RemoteModelId modelId);
    bool updateBaiduModelKeys(RemoteModel* remoteModel, RemoteModelId modelId, const Json::Value& modelConfig);
    void initRemoteModelList();
    std::unique_ptr<RemoteModel> createBaiduRemoteModel();

private:
    std::vector<std::unique_ptr<RemoteModel>> remoteModelList_;
    std::vector<RemoteModelVendorId> remoteModelVendorList_ = { BAIDU};

    RemoteModelVendorId defaultRemoteModelVendor_ = BAIDU;

    Json::Value config_;
};

#endif