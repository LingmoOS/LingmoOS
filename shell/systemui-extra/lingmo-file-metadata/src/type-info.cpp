/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "type-info.h"
#include <QObject>

using namespace LingmoUIFileMetadata;

class LingmoUIFileMetadata::TypeInfoPrivate
{
public:
    Type::Type type;
    QString name;
    QString displayName;
};

TypeInfo::TypeInfo(Type::Type type)
    : d(new TypeInfoPrivate)
{
    d->type = type;

    switch (type) {
    case Type::Empty:
        d->name = QStringLiteral("empty");
        d->displayName = QString();
        break;

    case Type::Archive:
        d->name = QStringLiteral("Archive");
        d->displayName = QObject::tr("Archive");
        break;

    case Type::Audio:
        d->name = QStringLiteral("Audio");
        d->displayName = QObject::tr("Audio");
        break;

    case Type::Document:
        d->name = QStringLiteral("Document");
        d->displayName = QObject::tr("Document");
        break;

    case Type::Image:
        d->name = QStringLiteral("Image");
        d->displayName = QObject::tr("Image");
        break;

    case Type::Presentation:
        d->name = QStringLiteral("Presentation");
        d->displayName = QObject::tr("Presentation");
        break;

    case Type::Spreadsheet:
        d->name = QStringLiteral("Spreadsheet");
        d->displayName = QObject::tr("Spreadsheet");
        break;

    case Type::Text:
        d->name = QStringLiteral("Text");
        d->displayName = QObject::tr("Text");
        break;

    case Type::Video:
        d->name = QStringLiteral("Video");
        d->displayName = QObject::tr("Video");
        break;

    case Type::Folder:
        d->name = QStringLiteral("Folder");
        d->displayName = QObject::tr("Folder");
        break;
    }
}

TypeInfo::TypeInfo(const TypeInfo& ti)
    : d(new TypeInfoPrivate(*ti.d))
{
}

TypeInfo::~TypeInfo() = default;

TypeInfo& TypeInfo::operator=(const TypeInfo& rhs)
{
    *d = *rhs.d;
    return *this;
}

bool TypeInfo::operator==(const TypeInfo& rhs) const
{
    return std::tie(d->type, d->name, d->displayName) == std::tie(rhs.d->type, rhs.d->name, rhs.d->displayName);
}

QString TypeInfo::displayName() const
{
    return d->displayName;
}

QString TypeInfo::name() const
{
    return d->name;
}

Type::Type TypeInfo::type() const
{
    return d->type;
}

TypeInfo TypeInfo::fromName(const QString& name)
{
    for (int t = static_cast<int>(Type::FirstType); t <= static_cast<int>(Type::LastType); t++) {
        TypeInfo ti(static_cast<Type::Type>(t));
        if (ti.name().compare(name, Qt::CaseInsensitive) == 0) {
            return ti;
        }
    }

    return TypeInfo(Type::Empty);
}
