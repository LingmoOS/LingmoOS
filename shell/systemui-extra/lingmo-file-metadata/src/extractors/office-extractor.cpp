/*
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
#include "office-extractor.h"
#include "binary-parser.h"

using namespace LingmoUIFileMetadata;

const QStringList supportedMimeTypes = {
    QStringLiteral("application/wps-office.doc"),    //2003word
    QStringLiteral("application/wps-office.dot"),
    QStringLiteral("application/wps-office.wps"),
    QStringLiteral("application/wps-office.et"),     //2003excel
    QStringLiteral("application/wps-office.xls"),
    QStringLiteral("application/wps-office.dps"),    //2003powerpoint
    QStringLiteral("application/x-ole-storage"),     //OLE file,
    //file ends with pps&ppt is identifier as ole file, but ole file has too many sub types, need a filter
};

OfficeExtractor::OfficeExtractor(QObject *parent) : ExtractorPlugin(parent)
{

}

void OfficeExtractor::extract(ExtractionResult *result)
{
    QFileInfo info(result->inputUrl());
    QString suffix = info.suffix();
    if (suffix == "doc" || suffix == "dot" || suffix == "wps" || suffix == "ppt" ||
            suffix == "pps" || suffix == "dps" || suffix == "et" || suffix == "xls") {
        result->addType(Type::Document);
        if (result->inputFlags() & ExtractionResult::ExtractPlainText) {
            QString contents;
            KBinaryParser parser;
            parser.RunParser(result->inputUrl(), contents);
            result->append(contents);
        }
    } else {
        qDebug() << "The parsing of the type:" << result->inputMimetype() << " is not fully supported currently";
    }
}

QStringList OfficeExtractor::mimetypes() const
{
    return supportedMimeTypes;
}
