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

#ifndef SERVICES_MODELMANAGERSERVICE_H
#define SERVICES_MODELMANAGERSERVICE_H

#include <string>
#include <lingmo-ai/ai-base/modelconfig.h>
#include "services/modelmanagerserviceglue.h"

class ModelManagerService {
public:
    ModelManagerService(GDBusConnection &connection);
    ~ModelManagerService();

    static const std::string &getObjectPath() { return objectPath_; }

private:
    void exportSkeleton();
    void unexportSkeleton();

    static bool
    onHandleGetRemoteModelVendorListLength(AisdkModelManagerService *delegate,
                                           GDBusMethodInvocation *invocation,
                                           gpointer userData);
    static bool
    onHandleGetRemoteModelVendorList(AisdkModelManagerService *delegate,
                                     GDBusMethodInvocation *invocation,
                                     gpointer userData);
    static bool onHandleGetRemoteModelListLength(
        AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
        RemoteModelVendorId vendor, gpointer userData);
    static bool onHandleGetRemoteModelList(AisdkModelManagerService *delegate,
                                           GDBusMethodInvocation *invocation,
                                           RemoteModelVendorId vendor,
                                           gpointer userData);
    static bool onHandleSetRemoteModelConfig(AisdkModelManagerService *delegate,
                                             GDBusMethodInvocation *invocation,
                                             RemoteModelVendorId vendor,
                                             RemoteModelId modelId, gchar *json,
                                             gpointer userData);
    static bool onHandleRemoveRemoteModelConfig(
        AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
        RemoteModelVendorId vendor, RemoteModelId modelId, gpointer userData);
    static bool
    onHandleGetRemoteModelConfigLength(AisdkModelManagerService *delegate,
                                       GDBusMethodInvocation *invocation,
                                       gpointer userData);
    static bool onHandleGetRemoteModelConfig(AisdkModelManagerService *delegate,
                                             GDBusMethodInvocation *invocation,
                                             gpointer userData);
    static bool
    onHandleGetDefaultRemoteModelVendor(AisdkModelManagerService *delegate,
                                        GDBusMethodInvocation *invocation,
                                        gpointer userData);
    static bool onHandleSetDefaultRemoteModelVendor(
        AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
        RemoteModelVendorId vendor, gpointer userData);

private:
    bool isExported_ = false;
    AisdkModelManagerService *delegate_ = nullptr;

    GDBusConnection &connection_;

    static const std::string objectPath_;
};

#endif
