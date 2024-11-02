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

#include "datamanagement.h"
#include "datamanagementsession.h"

#include <cstring>

namespace {
bool is_valid_datamanagement_session(void* session) {
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    return dataManagementSession != nullptr;
}

// 将int转换为FeatureStatus，输入的int是dbus接口定义的返回值，由后端返回
// 如果dbus getFeatureStatus接口定义更新，这里需要更新
FeatureStatus intToFeatureStatus(int status) {
    switch (status) {
        case 0:
            return FeatureStatus::AVAILABLE;
        case 1:
            return FeatureStatus::NOT_INSTALLED;
        case 2:
            return FeatureStatus::NOT_SUPPORTED;
        default:
            break;
    }
    return FeatureStatus::UNKNOWN;
}

}

DataManagementResult data_management_create_session(DataManagementSession* session, DataManagementType type, int uid)
{
    datamanagement::DataManagementSession* dataManagementSession = new datamanagement::DataManagementSession(type, uid);
    *session = dataManagementSession;
    return DATA_MANAGEMENT_SUCCESS;
}

DataManagementResult data_management_destroy_session(DataManagementSession session)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (dataManagementSession) {
        delete dataManagementSession;
        return DATA_MANAGEMENT_SUCCESS;
    }
    return DATA_MANAGEMENT_SESSION_ERROR;
}

DataManagementResult data_management_get_feature_status(DataManagementSession session, FeatureStatus* status)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    int status_int;
    DataManagementResult getResult = dataManagementSession->getFeatureStatus(status_int);
    *status = intToFeatureStatus(status_int);

    return getResult;
}

DataManagementResult data_management_check_file_format(DataManagementSession session, const char* filepath, char** check_result)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    std::string formatResult;
    DataManagementResult checkResult = dataManagementSession->checkFileFormat(filepath, formatResult);

    if (!formatResult.empty()) {
        size_t length = formatResult.size() + 1;
        *check_result = new char[length];
        std::strcpy(*check_result, formatResult.c_str());
    }

    return checkResult;
}

DataManagementResult data_management_free_check_file_format_result(DataManagementSession session, char* check_result)
{
    if (!is_valid_datamanagement_session(session)) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    delete[] check_result;
    return DATA_MANAGEMENT_SUCCESS;
}

DataManagementResult data_management_similarity_search(DataManagementSession session, const char* search_conditions, char** search_result)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    std::string searchResult;
    DataManagementResult result = dataManagementSession->similaritySearch(search_conditions, searchResult);

    size_t length = searchResult.size() + 1;
    *search_result = new char[length];
    std::strcpy(*search_result, searchResult.c_str());

    return result;
}

DataManagementResult data_management_free_search_result(DataManagementSession session, char* search_result)
{
    if (!is_valid_datamanagement_session(session)) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    delete[] search_result;
    return DATA_MANAGEMENT_SUCCESS;
}

DataManagementResult data_management_add_text_files(DataManagementSession session, const char* fileinfos)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    return dataManagementSession->addTextFiles(fileinfos);
}

DataManagementResult data_management_add_image_files(DataManagementSession session, const char* fileinfos)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    return dataManagementSession->addImageFiles(fileinfos);
}

DataManagementResult data_management_delete_files(DataManagementSession session, const char* fileinfos)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    return dataManagementSession->deleteFiles(fileinfos);
}

DataManagementResult data_management_update_files_name(DataManagementSession session, const char* fileinfos)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    return dataManagementSession->updateFilesName(fileinfos);
}

DataManagementResult data_management_update_files_content(DataManagementSession session, const char* fileinfos)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    return dataManagementSession->updateContentData(fileinfos);
}

DataManagementResult data_management_get_all_fileinfos(DataManagementSession session, char** fileinfos_result)
{
    datamanagement::DataManagementSession* dataManagementSession =
        static_cast<datamanagement::DataManagementSession*>(session);
    if (!dataManagementSession) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    std::string fileinfosResult;
    DataManagementResult result = dataManagementSession->getAllFileinfos(fileinfosResult);

    size_t length = fileinfosResult.size() + 1;
    *fileinfos_result = new char[length];
    std::strcpy(*fileinfos_result, fileinfosResult.c_str());

    return result;
}

DataManagementResult data_management_free_get_all_fileinfos_result(DataManagementSession session, char *fileinfos_result)
{
    if (is_valid_datamanagement_session(session)) {
        return DATA_MANAGEMENT_SESSION_ERROR;
    }

    delete[] fileinfos_result;
    return DATA_MANAGEMENT_SUCCESS;
}
