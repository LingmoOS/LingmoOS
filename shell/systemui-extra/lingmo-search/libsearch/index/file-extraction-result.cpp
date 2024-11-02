/*
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "file-extraction-result.h"
namespace LingmoUISearch {
class FileExtractionResultPrivate
{
public:
    LingmoUIFileMetadata::PropertyMultiMap m_properties;
    QString m_text;
    QVector<LingmoUIFileMetadata::Type::Type> m_types;
};

LingmoUISearch::FileExtractionResult::FileExtractionResult(const QString &url, const QString &mimetype,
                                                       const LingmoUIFileMetadata::ExtractionResult::Flags &flags)
        : ExtractionResult(url, mimetype, flags)
        , d(new FileExtractionResultPrivate)
{
}

FileExtractionResult::~FileExtractionResult() = default;


FileExtractionResult::FileExtractionResult(const FileExtractionResult &rhs): ExtractionResult(*this)
        , d(new FileExtractionResultPrivate(*rhs.d))
{
}

FileExtractionResult &FileExtractionResult::operator=(const FileExtractionResult &rhs)
{
    *d = *rhs.d;
    return *this;
}

void FileExtractionResult::add(LingmoUIFileMetadata::Property::Property property, const QVariant &value)
{
    d->m_properties.insert(property, value);
}

void FileExtractionResult::addType(LingmoUIFileMetadata::Type::Type type)
{
    d->m_types << type;
}

void FileExtractionResult::append(const QString &text)
{
    QString tmp = text;
    d->m_text.append(tmp.replace("\n", "").replace("\r", " "));
}

LingmoUIFileMetadata::PropertyMultiMap FileExtractionResult::properties() const
{
    return d->m_properties;
}

QString FileExtractionResult::text() const
{
    return d->m_text;
}

QVector<LingmoUIFileMetadata::Type::Type> FileExtractionResult::types() const
{
    return d->m_types;
}
}