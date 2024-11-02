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

#include "ofd-extractor.h"
#include "quazip5/quazip.h"
#include "quazip5/quazipfile.h"
#include <QStringList>
#include <QFileInfo>
#include <QXmlStreamReader>

#define MAX_CONTENT_LENGTH 20480000

using namespace LingmoUIFileMetadata;

const QStringList supportedMimeTypes = {
        QStringLiteral("application/ofd"),
        QStringLiteral("application/zip")
};

OfdExtractor::OfdExtractor(QObject *parent) : ExtractorPlugin(parent) {

}

/**
 * 参考标准 GB/T33190-2016
 * 参考文档 https://www.doc88.com/p-5354924116081.html
 */
void OfdExtractor::extract(ExtractionResult *result) {
    QFileInfo info(result->inputUrl());
    if (!info.exists() || info.isDir()) {
        return;
    }

    if (info.suffix() != "ofd") {
        return;
    }

    QuaZip zipfile(result->inputUrl());
    if (!zipfile.open(QuaZip::mdUnzip)) {
        return;
    }

    result->addType(Type::Document);


    QXmlStreamReader reader;
    if (result->inputFlags() & ExtractionResult::Flag::ExtractMetaData) {
        if (zipfile.setCurrentFile("OFD.xml")) {
            QuaZipFile fileR(&zipfile);
            if (fileR.open(QIODevice::ReadOnly)) {
                reader.setDevice(&fileR);
                QStringList keywords;
                while (!reader.atEnd()) {
                    if (reader.readNextStartElement()) {
                        if (reader.name().toString() == "Title") {
                            result->add(Property::Title, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "Author") {
                            result->add(Property::Author, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "Subject") {
                            result->add(Property::Subject, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "Abstract") {
                            result->add(Property::Description, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "CreationDate") {
                            result->add(Property::CreationDate, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "Creator") {
                            result->add(Property::Generator, reader.readElementText());
                            continue;
                        }

                        if (reader.name().toString() == "Keyword") {
                            keywords.append(reader.readElementText());
                        }
                    }
                }
                fileR.close();

                if (!keywords.isEmpty()) {
                    result->add(Property::Keywords, keywords);
                }
            }
        }
    }

    if (!(result->inputFlags() & ExtractionResult::Flag::ExtractPlainText)) {
        return;
    }
    // GB/T 33190-2016规范定义可以存在多个Doc_x目录，暂时只取第一个目录的内容
    QString prefix("Doc_0/Pages/");
    QStringList fileList;
    for (const auto &file: zipfile.getFileNameList()) {
        if (file.startsWith(prefix)) {
            fileList << file;
        }
    }

    QString textContent;
    for (int i = 0; i < fileList.count(); ++i) {
        QString filename = prefix + "Page_" + QString::number(i) + "/Content.xml";
        if (!zipfile.setCurrentFile(filename)) {
            continue;
        }
        QuaZipFile fileR(&zipfile);
        if (!fileR.open(QIODevice::ReadOnly)) {
            continue;
        }
        reader.setDevice(&fileR);

        while (!reader.atEnd()) {
            if (reader.readNextStartElement() && reader.name().toString() == "TextCode") {
                textContent.append(reader.readElementText());
                if (textContent.length() >= MAX_CONTENT_LENGTH / 3) {
                    fileR.close();
                    zipfile.close();
                    result->append(textContent);
                    return;
                }
            }
        }
        fileR.close();
    }
    zipfile.close();
    result->append(textContent);
}

QStringList OfdExtractor::mimetypes() const {
    return supportedMimeTypes;
}
