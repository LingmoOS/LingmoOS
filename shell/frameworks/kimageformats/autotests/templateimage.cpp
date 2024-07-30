/*
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "templateimage.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVersionNumber>

TemplateImage::TemplateImage(const QFileInfo &fi) :
    m_fi(fi)
{

}

bool TemplateImage::isTemplate() const
{
    auto list = suffixes();
    for (auto&& suffix : list) {
        if (!m_fi.suffix().compare(suffix, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

QFileInfo TemplateImage::compareImage(bool &skipTest) const
{
    auto fi = jsonImage(skipTest);
    if (skipTest) {
        return {};
    }
    if (fi.exists()) {
        return fi;
    }
    return legacyImage();
}


QStringList TemplateImage::suffixes()
{
    return QStringList({"png", "tif", "tiff", "json"});
}

QFileInfo TemplateImage::legacyImage() const
{
    auto list = suffixes();
    for (auto&& suffix : list) {
        auto fi = QFileInfo(QStringLiteral("%1/%2.%3").arg(m_fi.path(), m_fi.completeBaseName(), suffix));
        if (fi.exists()) {
            return fi;
        }
    }
    return {};
}

QFileInfo TemplateImage::jsonImage(bool &skipTest) const
{
    auto fi = QFileInfo(QStringLiteral("%1.json").arg(m_fi.filePath()));
    if (!fi.exists()) {
        return {};
    }

    QFile f(fi.filePath());
    if (!f.open(QFile::ReadOnly)) {
        return {};
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        return {};
    }

    auto currentQt = QVersionNumber::fromString(qVersion());
    auto arr = doc.array();
    for (auto val : arr) {
        if (!val.isObject())
            continue;
        auto obj = val.toObject();
        auto minQt = QVersionNumber::fromString(obj.value("minQtVersion").toString());
        auto maxQt = QVersionNumber::fromString(obj.value("maxQtVersion").toString());
        auto name = obj.value("fileName").toString();
        auto unsupportedFormat = obj.value("unsupportedFormat").toBool();

        // filter
        if (name.isEmpty() && !unsupportedFormat)
            continue;
        if (!minQt.isNull() && currentQt < minQt)
            continue;
        if (!maxQt.isNull() && currentQt > maxQt)
            continue;
        if (unsupportedFormat) {
            skipTest = true;
            break;
        }
        return QFileInfo(QStringLiteral("%1/%2").arg(fi.path(), name));
    }

    return {};
}

