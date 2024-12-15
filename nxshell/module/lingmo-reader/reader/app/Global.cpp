// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Global.h"

#include <QMimeType>
#include <QMimeDatabase>
#include <QDebug>
#include <DGuiApplicationHelper>

namespace Dr {
FileType fileType(const QString &filePath)
{
    FileType fileType = FileType::Unknown;

    //具体的MIME文件类型可以参考https://baike.baidu.com/item/MIME/2900607?fr=aladdin
    const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filePath, QMimeDatabase::MatchContent);

    qInfo() << "当前文件的MIME类型: " << mimeType.name();
    if (mimeType.name() == QLatin1String("application/pdf")) {
        fileType = PDF;
    } else if (mimeType.name() == QLatin1String("application/postscript")) {
        fileType = PS;
    } else if (mimeType.name() == QLatin1String("image/vnd.djvu") || mimeType.name() == QLatin1String("image/vnd.djvu+multipage")) {
        fileType = DJVU;
    } else if (mimeType.name() == QLatin1String("application/zip") && filePath.right(4) == "pptx") {
        fileType = PPTX;
    } else if (mimeType.name() == QLatin1String("application/zip") && filePath.right(4) == "docx") {
        fileType = DOCX;
    } else if (mimeType.name() == QLatin1String("application/x-ole-storage") && filePath.right(4) == "docx") {
        fileType = DOCX;
    }

    return fileType;
}

}
