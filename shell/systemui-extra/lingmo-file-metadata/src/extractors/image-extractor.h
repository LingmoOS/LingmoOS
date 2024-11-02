/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Modified by: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef LINGMO_FILE_METADATA_IMAGEEXTRACTOR_H
#define LINGMO_FILE_METADATA_IMAGEEXTRACTOR_H

#include "extractor-plugin.h"

namespace LingmoUIFileMetadata {

class ImageExtractor : public ExtractorPlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.lingmo.lingmofilemetadata.ExtractorPlugin"
                      FILE "image-extractor.json")
    Q_INTERFACES(LingmoUIFileMetadata::ExtractorPlugin)
public:
    explicit ImageExtractor(QObject* parent = nullptr);

    void extract(ExtractionResult* result) override;
    [[nodiscard]] QStringList mimetypes() const override;
};
}
#endif //LINGMO_FILE_METADATA_IMAGEEXTRACTOR_H
