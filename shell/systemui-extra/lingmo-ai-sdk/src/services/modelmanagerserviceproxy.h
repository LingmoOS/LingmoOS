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

#ifndef SERVICES_MODELMANAGERSERVICEPROXY_H
#define SERVICES_MODELMANAGERSERVICEPROXY_H

#include <string>
#include <vector>

#include "services/modelmanagerserviceglue.h"

#include "modelconfig.h"

class ModelManagerServiceProxy {
public:
    static ModelManagerServiceProxy &getInstance();

    ~ModelManagerServiceProxy();

    unsigned int getRemoteModelVendorListLength() const;
    const std::vector<RemoteModelVendorId> &getRemoteModelVendorList() const;

    unsigned int getRemoteModelListLength(RemoteModelVendorId vendor) const;
    const std::vector<RemoteModelId> &
    getRemoteModelList(RemoteModelVendorId vendor) const;

    bool setRemoteModelConfig(RemoteModelVendorId vendor, RemoteModelId modelId,
                              const char *json);
    bool removeRemoteModelConfig(RemoteModelVendorId vendor,
                                 RemoteModelId modelId);
    unsigned int getRemoteModelConfigLength() const;
    std::string getRemoteModelConfig() const;

    RemoteModelVendorId getDefaultRemoteModelVendor() const;
    bool setDefaultRemoteModelVendor(RemoteModelVendorId vendor);

private:
    ModelManagerServiceProxy();

    void init();
    void destroy();

private:
    AisdkModelManagerService *delegate_ = nullptr;

    mutable std::vector<RemoteModelVendorId> remoteModelVendorIdList_;

    mutable std::vector<RemoteModelId> remoteModelIdList_;
};

#endif