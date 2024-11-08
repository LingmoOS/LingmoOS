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

#ifndef DATAMANAGEMENTSESSION_H
#define DATAMANAGEMENTSESSION_H

#include "datamanagement.h"
#include "services/datamanagementprocessorproxy.h"

namespace datamanagement {

class DataManagementSession
{
public:
    DataManagementSession(DataManagementType type, uid_t uid);

    DataManagementResult checkFileFormat(const std::string& filePath, std::string& formatResult);
    DataManagementResult similaritySearch(const std::string& searchConditions, std::string& searchResult);
    DataManagementResult addTextFiles(const std::string& fileinfos);
    DataManagementResult addImageFiles(const std::string& fileinfos);
    DataManagementResult deleteFiles(const std::string& fileinfos);
    DataManagementResult updateFilesName(const std::string& fileinfos);
    DataManagementResult updateContentData(const std::string& fileinfos);
    DataManagementResult getAllFileinfos(std::string& result);
    DataManagementResult getFeatureStatus(int& status);

private:
    DataManagementProcessorProxy proxy_;
    DataManagementType type_;
    uid_t uid_;

    DataManagementResult intToDataManagementResult(int value) const;
};

} // namespace datamanagement

#endif // DATAMANAGEMENTSESSION_H
