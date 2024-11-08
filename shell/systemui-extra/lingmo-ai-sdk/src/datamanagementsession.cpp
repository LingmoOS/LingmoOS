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

#include "datamanagementsession.h"

#include <jsoncpp/json/json.h>

#include <vector>
#include <filesystem>

namespace {

std::string generateFormatCheckResult(bool supported, const std::string &format) {
    Json::Value root;
    root["supported"] = supported;
    root["format"] = format;

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "";
    std::string jsonString = Json::writeString(writerBuilder, root);
    return jsonString;
}

std::string getFileExtension(const std::string &filePath) {
    size_t pos = filePath.find_last_of('.');
    if (pos != std::string::npos) {
        return filePath.substr(pos + 1);
    }
    return std::string();
}

bool isSupportedFormat(const std::string &extension) {
    const std::vector<std::string> supportTextFormat = {"txt", "pdf", "docx", "pptx"};
    const std::vector<std::string> supportImageFormat = {"png", "jpg", "jpeg", "jpe", "bmp", "dib"};

    for (const auto &format : supportTextFormat) {
        if (format == extension)
            return true;
    }
    for (const auto &format : supportImageFormat) {
        if (format == extension)
            return true;
    }
    return false;
}

}

namespace datamanagement {

DataManagementSession::DataManagementSession(DataManagementType type, uid_t uid)
    : proxy_(uid)
    , type_(type)
    , uid_(uid) { }

DataManagementResult DataManagementSession::intToDataManagementResult(int value) const {
    if (value < DATA_MANAGEMENT_SUCCESS || value > DATA_MANAGEMENT_FILE_FORMAT_ERROR) {
        return DATA_MANAGEMENT_UNKNOWN_ERROR;
    }
    return static_cast<DataManagementResult>(value);
}

DataManagementResult DataManagementSession::checkFileFormat(const std::string &filePath, std::string &formatResult)
{
    if (!std::filesystem::exists(filePath)) {
        return DATA_MANAGEMENT_INVAILD_PATH;
    }

    std::string format = getFileExtension(filePath);
    bool isSupported = isSupportedFormat(format);
    formatResult = generateFormatCheckResult(isSupported, format);

    return DATA_MANAGEMENT_SUCCESS;
}

DataManagementResult DataManagementSession::similaritySearch(const std::string &searchConditions, std::string &searchResult)
{
    int result = proxy_.similaritySearch(searchConditions, searchResult);
    return intToDataManagementResult(result);
}

DataManagementResult DataManagementSession::addTextFiles(const std::string &fileinfos)
{
    int result = proxy_.addTextFiles(fileinfos);
    return intToDataManagementResult(result);
}

DataManagementResult DataManagementSession::addImageFiles(const std::string &fileinfos)
{
    int result = proxy_.addImageFiles(fileinfos);
    return intToDataManagementResult(result);
}

DataManagementResult DataManagementSession::deleteFiles(const std::string &fileinfos)
{
    int result = proxy_.deleteFiles(fileinfos);
    return intToDataManagementResult(result);
}

DataManagementResult DataManagementSession::updateFilesName(const std::string &fileinfos)
{
    int result = proxy_.updateFilesName(fileinfos);
    return intToDataManagementResult(result);
}

DataManagementResult DataManagementSession::updateContentData(const std::string &fileinfos)
{
    int result = proxy_.updateContentData(fileinfos);
    return intToDataManagementResult(result);
}

DataManagementResult DataManagementSession::getAllFileinfos(std::string &result)
{
    int res = proxy_.getAllFileinfos(result);
    return intToDataManagementResult(res);
}

DataManagementResult DataManagementSession::getFeatureStatus(int &status)
{
    int res = proxy_.getFeatureStatus(status);
    return intToDataManagementResult(res);
}

} // namespace datamanagement
