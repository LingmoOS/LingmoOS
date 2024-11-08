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

#include <cstring>

#include "services/modelmanagerserviceproxy.h"

unsigned int get_remote_model_vendor_list_length() {
    return ModelManagerServiceProxy::getInstance()
        .getRemoteModelVendorListLength();
}

const RemoteModelVendorId *get_remote_model_vendor_list() {
    return ModelManagerServiceProxy::getInstance()
        .getRemoteModelVendorList()
        .data();
}

unsigned int get_remote_model_list_length(RemoteModelVendorId vendor) {
    return ModelManagerServiceProxy::getInstance().getRemoteModelListLength(
        vendor);
}

const RemoteModelId *get_remote_model_list(RemoteModelVendorId vendor) {
    return ModelManagerServiceProxy::getInstance()
        .getRemoteModelList(vendor)
        .data();
}

ModelConfigResult set_remote_model_config(RemoteModelVendorId vendor,
                                          RemoteModelId modelId,
                                          const char *json) {
    if (json == nullptr) {
        return FAILED;
    }

    return ModelManagerServiceProxy::getInstance().setRemoteModelConfig(
               vendor, modelId, json)
               ? SUCCEEDED
               : FAILED;
}

ModelConfigResult remove_remote_model_config(RemoteModelVendorId vendor,
                                             RemoteModelId modelId) {
    return ModelManagerServiceProxy::getInstance().removeRemoteModelConfig(
               vendor, modelId)
               ? SUCCEEDED
               : FAILED;
}

unsigned int get_remote_model_config_length() {
    return ModelManagerServiceProxy::getInstance().getRemoteModelConfigLength();
}

ModelConfigResult get_remote_model_config(char *json) {
    if (json == nullptr) {
        return FAILED;
    }

    std::string config =
        ModelManagerServiceProxy::getInstance().getRemoteModelConfig();
    std::memcpy(json, config.c_str(), config.length());

    return SUCCEEDED;
}

RemoteModelVendorId get_default_remote_model_vendor() {
    return ModelManagerServiceProxy::getInstance()
        .getDefaultRemoteModelVendor();
}

void set_default_remote_model_vendor(RemoteModelVendorId vendor) {
    ModelManagerServiceProxy::getInstance().setDefaultRemoteModelVendor(vendor);
}