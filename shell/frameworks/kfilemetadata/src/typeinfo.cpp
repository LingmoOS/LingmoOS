/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "typeinfo.h"

#include <KLazyLocalizedString>
#include <KLocalizedString>

#include "icnamematch_p.h"

#include <array>

using namespace KFileMetaData;

class KFileMetaData::TypeInfoPrivate
{
public:
    const Type::Type type;
    const QString name;
    const KLazyLocalizedString displayName;

    static constexpr auto fromId(Type::Type type) -> const TypeInfoPrivate*;
    static auto fromName(const QString& name) -> const TypeInfoPrivate*;

    static const TypeInfoPrivate s_Empty;
    static const std::array<TypeInfoPrivate, 9> s_allTypes;
    static const QHash<LcIdentifierName, const TypeInfoPrivate*> s_typeHash;
};

const TypeInfoPrivate TypeInfoPrivate::s_Empty{ Type::Empty, QStringLiteral("empty"), kli18nc("@label", "Empty") };

const std::array<TypeInfoPrivate, 9> TypeInfoPrivate::s_allTypes
{
    TypeInfoPrivate{ Type::Archive,      QStringLiteral("Archive"),      kli18nc("@label", "Archive") },
    TypeInfoPrivate{ Type::Audio,        QStringLiteral("Audio"),        kli18nc("@label", "Audio") },
    TypeInfoPrivate{ Type::Document,     QStringLiteral("Document"),     kli18nc("@label", "Document") },
    TypeInfoPrivate{ Type::Image,        QStringLiteral("Image"),        kli18nc("@label", "Image") },
    TypeInfoPrivate{ Type::Presentation, QStringLiteral("Presentation"), kli18nc("@label", "Presentation") },
    TypeInfoPrivate{ Type::Spreadsheet,  QStringLiteral("Spreadsheet"),  kli18nc("@label", "Spreadsheet") },
    TypeInfoPrivate{ Type::Text,         QStringLiteral("Text"),         kli18nc("@label", "Text") },
    TypeInfoPrivate{ Type::Video,        QStringLiteral("Video"),        kli18nc("@label", "Video") },
    TypeInfoPrivate{ Type::Folder,       QStringLiteral("Folder"),       kli18nc("@label", "Folder") },
};

const QHash<LcIdentifierName, const TypeInfoPrivate*> TypeInfoPrivate::s_typeHash = []()
{
    QHash<LcIdentifierName, const TypeInfoPrivate*> infoHash;
    infoHash.reserve(s_allTypes.size());

    for (const auto& info: s_allTypes) {
        infoHash[QStringView(info.name)] = &info;
    }
    return infoHash;
}();

constexpr auto TypeInfoPrivate::fromId(Type::Type type) -> const TypeInfoPrivate*
{
    for (const auto& t : s_allTypes) {
        if (t.type == type)
            return &t;
    }
    return &s_Empty;
}

auto TypeInfoPrivate::fromName(const QString& name) -> const TypeInfoPrivate*
{
    return s_typeHash.value(LcIdentifierName(name), &s_Empty);
}

TypeInfo::TypeInfo()
    : d(&TypeInfoPrivate::s_Empty) {};

TypeInfo::TypeInfo(Type::Type type)
    : d(TypeInfoPrivate::fromId(type))
{
}

TypeInfo::TypeInfo(const TypeInfo& ti)
    : d(ti.d)
{
}

TypeInfo::~TypeInfo() = default;

TypeInfo& TypeInfo::operator=(const TypeInfo& rhs)
{
    d = rhs.d;
    return *this;
}

bool TypeInfo::operator==(const TypeInfo& rhs) const
{
    return d == rhs.d;
}

QString TypeInfo::displayName() const
{
    return d->displayName.toString();
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
    TypeInfo ti;
    ti.d = TypeInfoPrivate::fromName(name);
    return ti;
}

QStringList TypeInfo::allNames()
{
    static QStringList sNames = []() {
        QStringList names;
        names.reserve(TypeInfoPrivate::s_allTypes.size());
        for (auto info: TypeInfoPrivate::s_allTypes) {
            names.append(info.name);
        }
        return names;
    }();
    return sNames;
}
