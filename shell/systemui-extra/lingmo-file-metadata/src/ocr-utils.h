/*
    SPDX-FileCopyrightText: 2023 Junjie Bai <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef LINGMO_FILE_METADATA_OCR_UTILS_H
#define LINGMO_FILE_METADATA_OCR_UTILS_H

#include "lingmo-file-metadata_global.h"
#include <QObject>

namespace LingmoUIFileMetadata {
class LINGMOFILEMETADATA_EXPORT OcrUtils
{
public:
    static QString getTextInPicture(const QString &path);

private:
    explicit OcrUtils() = default;
};
}

#endif //LINGMO_FILE_METADATA_OCR_UTILS_H
