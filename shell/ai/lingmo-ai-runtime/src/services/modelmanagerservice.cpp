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

#include "services/modelmanagerservice.h"

const std::string ModelManagerService::objectPath_ =
    "/org/lingmo/aisdk/modelmanagerservice";

ModelManagerService::ModelManagerService(GDBusConnection &connection)
    : connection_(connection) {
    exportSkeleton();
}

ModelManagerService::~ModelManagerService() { unexportSkeleton(); }

void ModelManagerService::exportSkeleton() {
    delegate_ = aisdk_model_manager_service_skeleton_new();
    g_signal_connect(
        delegate_, "handle-get-remote-model-vendor-list-length",
        G_CALLBACK(ModelManagerService::onHandleGetRemoteModelVendorListLength),
        nullptr);
    g_signal_connect(
        delegate_, "handle-get-remote-model-vendor-list",
        G_CALLBACK(ModelManagerService::onHandleGetRemoteModelVendorList),
        nullptr);
    g_signal_connect(
        delegate_, "handle-get-remote-model-list-length",
        G_CALLBACK(ModelManagerService::onHandleGetRemoteModelListLength),
        nullptr);
    g_signal_connect(
        delegate_, "handle-get-remote-model-list",
        G_CALLBACK(ModelManagerService::onHandleGetRemoteModelList), nullptr);
    g_signal_connect(
        delegate_, "handle-set-remote-model-config",
        G_CALLBACK(ModelManagerService::onHandleSetRemoteModelConfig), nullptr);
    g_signal_connect(
        delegate_, "handle-remove-remote-model-config",
        G_CALLBACK(ModelManagerService::onHandleRemoveRemoteModelConfig),
        nullptr);
    g_signal_connect(
        delegate_, "handle-get-remote-model-config-length",
        G_CALLBACK(ModelManagerService::onHandleGetRemoteModelConfigLength),
        nullptr);
    g_signal_connect(
        delegate_, "handle-get-remote-model-config",
        G_CALLBACK(ModelManagerService::onHandleGetRemoteModelConfig), nullptr);
    g_signal_connect(
        delegate_, "handle-get-default-remote-model-vendor",
        G_CALLBACK(ModelManagerService::onHandleGetDefaultRemoteModelVendor),
        nullptr);
    g_signal_connect(
        delegate_, "handle-set-default-remote-model-vendor",
        G_CALLBACK(ModelManagerService::onHandleSetDefaultRemoteModelVendor),
        nullptr);
    GError *error = nullptr;

    isExported_ = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath_.c_str(),
        &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n",
                   objectPath_.c_str(), error->message);
        g_error_free(error);
    }
}

void ModelManagerService::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(
            G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }

    g_object_unref(delegate_);
}

bool ModelManagerService::onHandleGetRemoteModelVendorListLength(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    gpointer userData) {
    aisdk_model_manager_service_complete_get_remote_model_vendor_list_length(
        delegate, invocation, 1);

    return true;
}

bool ModelManagerService::onHandleGetRemoteModelVendorList(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    gpointer userData) {
    gint elements[] = {static_cast<gint>(BAIDU)};
    GVariant *list = g_variant_new_fixed_array(
        G_VARIANT_TYPE_INT32, elements, G_N_ELEMENTS(elements), sizeof(gint));

    aisdk_model_manager_service_complete_get_remote_model_vendor_list(
        delegate, invocation, list);

    return true;
}

bool ModelManagerService::onHandleGetRemoteModelListLength(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    RemoteModelVendorId vendor, gpointer userData) {
    aisdk_model_manager_service_complete_get_remote_model_list_length(
        delegate, invocation, 1);

    return true;
}

bool ModelManagerService::onHandleGetRemoteModelList(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    RemoteModelVendorId vendor, gpointer userData) {
    gint elements[] = {static_cast<gint>(NLP)};
    GVariant *list = g_variant_new_fixed_array(
        G_VARIANT_TYPE_INT32, elements, G_N_ELEMENTS(elements), sizeof(gint));

    aisdk_model_manager_service_complete_get_remote_model_list(
        delegate, invocation, list);

    return true;
}

bool ModelManagerService::onHandleSetRemoteModelConfig(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    RemoteModelVendorId vendor, RemoteModelId modelId, gchar *json,
    gpointer userData) {
    aisdk_model_manager_service_complete_set_remote_model_config(
        delegate, invocation, true);

    return true;
}

bool ModelManagerService::onHandleRemoveRemoteModelConfig(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    RemoteModelVendorId vendor, RemoteModelId modelId, gpointer userData) {
    aisdk_model_manager_service_complete_remove_remote_model_config(
        delegate, invocation, true);
    return true;
}

bool ModelManagerService::onHandleGetRemoteModelConfigLength(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    gpointer userData) {
    aisdk_model_manager_service_complete_get_remote_model_config_length(
        delegate, invocation, 1);

    return true;
}

bool ModelManagerService::onHandleGetRemoteModelConfig(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    gpointer userData) {
    aisdk_model_manager_service_complete_get_remote_model_config(
        delegate, invocation, "{}");

    return true;
}

bool ModelManagerService::onHandleGetDefaultRemoteModelVendor(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    gpointer userData) {
    aisdk_model_manager_service_complete_get_default_remote_model_vendor(
        delegate, invocation, static_cast<gint>(BAIDU));

    return true;
}

bool ModelManagerService::onHandleSetDefaultRemoteModelVendor(
    AisdkModelManagerService *delegate, GDBusMethodInvocation *invocation,
    RemoteModelVendorId vendor, gpointer userData) {
    aisdk_model_manager_service_complete_set_default_remote_model_vendor(
        delegate, invocation);

    return true;
}