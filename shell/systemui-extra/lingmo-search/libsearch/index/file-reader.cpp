/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */
#include "file-reader.h"
#include <ocr-utils.h>
#include <mime-utils.h>
#include <mutex>
#include "file-extraction-result.h"
#include "common.h"
#include "file-indexer-config.h"

using namespace LingmoUISearch;
FileReader *g_instance = nullptr;
std::once_flag g_instanceFlag;
FileReader *FileReader::getInstance()
{
    std::call_once(g_instanceFlag, [] () {
        g_instance = new FileReader;
    });
    return g_instance;
}

FileReader::FileReader()
= default;
void FileReader::getTextContent(const QString &path, QString &textContent, const QString &suffix)
{
    if(FileIndexerConfig::getInstance()->ocrContentIndexTarget()[suffix]) {
        textContent = LingmoUIFileMetadata::OcrUtils::getTextInPicture(path);
        return;
    }
    QString mimeType = LingmoUIFileMetadata::MimeUtils::strictMimeType(path, {}).name();
    QList<LingmoUIFileMetadata::Extractor*> extractors = m_extractorManager.fetchExtractors(mimeType);
    FileExtractionResult result(path, mimeType, LingmoUIFileMetadata::ExtractionResult::Flag::ExtractPlainText);
    for(auto extractor : extractors) {
        extractor->extract(&result);
        if(!result.text().isEmpty()) {
            textContent = result.text();
            break;
        }
    }
}

QImage FileReader::getThumbnail(const QString &path, const QSize &targetSize, qreal dpr)
{
    QString mimeType = LingmoUIFileMetadata::MimeUtils::strictMimeType(path, {}).name();
    QList<LingmoUIFileMetadata::Extractor*> extractors = m_extractorManager.fetchExtractors(mimeType);
    FileExtractionResult result(path, mimeType, LingmoUIFileMetadata::ExtractionResult::Flag::ExtractThumbnail);
    result.setThumbnailRequest(LingmoUIFileMetadata::ThumbnailRequest(targetSize, dpr));
    for(auto extractor : extractors) {
        extractor->extract(&result);
        if(result.thumbnail().isValid()) {
            return result.thumbnail().image();
        }
    }
    return {};
}
