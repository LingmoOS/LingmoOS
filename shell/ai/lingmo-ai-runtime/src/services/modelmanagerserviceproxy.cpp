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

#include "services/modelmanagerserviceproxy.h"

#include "services/modelmanagerservice.h"
#include "services/serverproxy.h"

ModelManagerServiceProxy &ModelManagerServiceProxy::getInstance() {
    static ModelManagerServiceProxy instance{};

    return instance;
}

ModelManagerServiceProxy::ModelManagerServiceProxy() { init(); }

ModelManagerServiceProxy::~ModelManagerServiceProxy() { destroy(); }

void ModelManagerServiceProxy::init() {
    const auto &unixPath = ServerProxy::getUnixPath();
    GError *error = nullptr;
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating model manager service proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }

    auto *connection = ServerProxy::getInstance().getConnection();
    const auto &objectPath = ModelManagerService::getObjectPath();
    delegate_ = aisdk_model_manager_service_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath.c_str(),
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating model manager service proxy %s: %s\n",
                   objectPath.c_str(), error->message);
        g_error_free(error);
    }
}

void ModelManagerServiceProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}

unsigned int ModelManagerServiceProxy::getRemoteModelVendorListLength() const {
    guint length = 0;
    GError *error = nullptr;
    bool success =
        aisdk_model_manager_service_call_get_remote_model_vendor_list_length_sync(
            delegate_, &length, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model vendor list length: %s\n",
                   error->message);

        g_error_free(error);
    }

    return length;
}

const std::vector<RemoteModelVendorId> &
ModelManagerServiceProxy::getRemoteModelVendorList() const {
    remoteModelVendorIdList_.clear();

    GVariant *list = nullptr;
    GError *error = nullptr;
    bool success =
        aisdk_model_manager_service_call_get_remote_model_vendor_list_sync(
            delegate_, &list, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model vendor list: %s\n",
                   error->message);

        g_error_free(error);
    } else {
        GVariantIter iter;
        g_variant_iter_init(&iter, list);

        RemoteModelVendorId id;
        while (g_variant_iter_next(&iter, "i", &id)) {
            remoteModelVendorIdList_.push_back(id);
        }

        g_variant_unref(list);
    }

    return remoteModelVendorIdList_;
}

unsigned int ModelManagerServiceProxy::getRemoteModelListLength(
    RemoteModelVendorId vendor) const {
    guint length = 0;
    GError *error = nullptr;
    bool success =
        aisdk_model_manager_service_call_get_remote_model_list_length_sync(
            delegate_, static_cast<gint>(vendor), &length, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model list length: %s\n",
                   error->message);

        g_error_free(error);
    }

    return length;
}

const std::vector<RemoteModelId> &
ModelManagerServiceProxy::getRemoteModelList(RemoteModelVendorId vendor) const {
    remoteModelIdList_.clear();

    GVariant *list = nullptr;
    GError *error = nullptr;
    bool success = aisdk_model_manager_service_call_get_remote_model_list_sync(
        delegate_, static_cast<gint>(vendor), &list, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model list: %s\n", error->message);

        g_error_free(error);
    } else {
        GVariantIter iter;
        g_variant_iter_init(&iter, list);

        RemoteModelId id;
        while (g_variant_iter_next(&iter, "i", &id)) {
            remoteModelIdList_.push_back(id);
        }

        g_variant_unref(list);
    }

    return remoteModelIdList_;
}

bool ModelManagerServiceProxy::setRemoteModelConfig(RemoteModelVendorId vendor,
                                                    RemoteModelId modelId,
                                                    const char *json) {
    gboolean serverResult = false;
    GError *error = nullptr;

    bool success =
        aisdk_model_manager_service_call_set_remote_model_config_sync(
            delegate_, static_cast<gint>(vendor), static_cast<gint>(modelId),
            json, &serverResult, nullptr, &error);
    if (!success) {
        g_printerr("Error setting remote model config: %s\n", error->message);

        g_error_free(error);

        return false;
    }

    return static_cast<bool>(serverResult);
}

bool ModelManagerServiceProxy::removeRemoteModelConfig(
    RemoteModelVendorId vendor, RemoteModelId modelId) {
    gboolean serverResult = false;
    GError *error = nullptr;

    bool success =
        aisdk_model_manager_service_call_remove_remote_model_config_sync(
            delegate_, static_cast<gint>(vendor), static_cast<gint>(modelId),
            &serverResult, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model vendor list: %s\n",
                   error->message);

        g_error_free(error);

        return false;
    }

    return static_cast<bool>(serverResult);
}

unsigned int ModelManagerServiceProxy::getRemoteModelConfigLength() const {
    guint length = 0;
    GError *error = nullptr;
    bool success =
        aisdk_model_manager_service_call_get_remote_model_config_length_sync(
            delegate_, &length, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model config length: %s\n",
                   error->message);

        g_error_free(error);
    }

    return length;
}

std::string ModelManagerServiceProxy::getRemoteModelConfig() const {
    gchar *config = nullptr;
    GError *error = nullptr;
    bool success =
        aisdk_model_manager_service_call_get_remote_model_config_sync(
            delegate_, &config, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model config length: %s\n",
                   error->message);

        g_error_free(error);
        return "";
    }

    std::string result = config;
    g_free(config);

    return result;
}

RemoteModelVendorId
ModelManagerServiceProxy::getDefaultRemoteModelVendor() const {
    gint vendor = static_cast<gint>(INVALID);
    GError *error = nullptr;
    bool success =
        aisdk_model_manager_service_call_get_default_remote_model_vendor_sync(
            delegate_, &vendor, nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model vendor: %s\n", error->message);

        g_error_free(error);
    }

    return static_cast<RemoteModelVendorId>(vendor);
}

bool ModelManagerServiceProxy::setDefaultRemoteModelVendor(
    RemoteModelVendorId vendor) {
    GError *error = nullptr;
    bool success =
        aisdk_model_manager_service_call_set_default_remote_model_vendor_sync(
            delegate_, static_cast<gint>(vendor), nullptr, &error);
    if (!success) {
        g_printerr("Error getting remote model vendor: %s\n", error->message);

        g_error_free(error);
    }

    return success;
}