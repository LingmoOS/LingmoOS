/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef LINGMO_FILE_METADATA_THUMBNAILUTILS_H
#define LINGMO_FILE_METADATA_THUMBNAILUTILS_H

#include "lingmo-file-metadata_global.h"
#include "extraction-result.h"
#include <QString>

namespace LingmoUIFileMetadata{

class LINGMOFILEMETADATA_EXPORT ThumbnailUtils {
public:
    static void setThumbnail(ExtractionResult *result, QImage& thumbnail, const QString& pluginName, const QString& version);
    static void handleFailureOfThumbnail(const QString& filePath, const QString& pluginName, const QString& version);
    static bool needGenerateThumbnail(ExtractionResult *result, const QString& pluginName, const QString& version);
private:
    ThumbnailUtils() = default;
};
}




#endif //LINGMO_FILE_METADATA_THUMBNAILUTILS_H
