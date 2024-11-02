#include "services/datamanagementprocessorproxy.h"

#include <iostream>
#include <unistd.h>
#include <string>

DataManagementProcessorProxy::DataManagementProcessorProxy(uid_t uid) : uid_(uid)
{
    init();
}

DataManagementProcessorProxy::~DataManagementProcessorProxy()
{
    destroy();
}

void DataManagementProcessorProxy::init()
{
    GError *error = nullptr;

    std::string serverUnixPath = "unix:path=/tmp/org.lingmo.aisdk.aibusiness." + std::to_string(geteuid());
    connection_ = g_dbus_connection_new_for_address_sync(
        serverUnixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
        nullptr, /* GDBusAuthObserver */
        nullptr, /* GCancellable */
        &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n",
                   serverUnixPath.c_str(), error->message);
        g_error_free(error);

        return;
    }

    delegate_ = aisdk_data_management_processor_proxy_new_sync(
        connection_, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_.c_str(),
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating speech processor proxy %s: %s\n",
                   objectPath_.c_str(), error->message);
        g_error_free(error);
        return;
    }
}

void DataManagementProcessorProxy::destroy()
{
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
}

int DataManagementProcessorProxy::similaritySearch(const std::string& searchConditions, std::string& searchResult)
{
    char* output = nullptr;
    int errorCode = -1;
    aisdk_data_management_processor_call_similarity_search_sync(
        delegate_, searchConditions.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, -1,
        &output, &errorCode, nullptr, nullptr);

    if (output != nullptr)
        searchResult = output;

    return errorCode;
}

int DataManagementProcessorProxy::addTextFiles(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_add_text_files_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 3600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::addImageFiles(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_add_image_files_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 3600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::deleteFiles(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_delete_files_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, -1,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::updateFilesName(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_update_files_name_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::updateContentData(const std::string &fileinfos)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_update_files_content_data_sync(
        delegate_, fileinfos.c_str(), uid_,
        G_DBUS_CALL_FLAGS_NONE, 600000,
        &errorCode, nullptr, nullptr);

    return errorCode;
}

int DataManagementProcessorProxy::getAllFileinfos(std::string &result)
{
    char* output = nullptr;
    int errorCode = -1;
    aisdk_data_management_processor_call_get_all_fileinfos_sync(
        delegate_, uid_,
        G_DBUS_CALL_FLAGS_NONE, -1,
        &output, &errorCode, nullptr, nullptr);

    if (output != nullptr)
        result = output;

    return errorCode;
}

int DataManagementProcessorProxy::getFeatureStatus(int &status)
{
    int errorCode = -1;
    aisdk_data_management_processor_call_get_feature_status_sync(
        delegate_,
        G_DBUS_CALL_FLAGS_NONE, -1,
        &status, &errorCode, nullptr, nullptr);

    return errorCode;
}
