/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef LINGMOFILEMETADATA_TYPE_INFO_H
#define LINGMOFILEMETADATA_TYPE_INFO_H

#include "types.h"
#include "lingmo-file-metadata_global.h"
#include <QString>
#include <memory>

namespace LingmoUIFileMetadata {

class TypeInfoPrivate;
/**
 * \class TypeInfo typeinfo.h <KFileMetaData/TypeInfo>
 */
class LINGMOFILEMETADATA_EXPORT TypeInfo
{
public:
    TypeInfo(Type::Type type);
    TypeInfo(const TypeInfo& ti);
    ~TypeInfo();

    TypeInfo& operator=(const TypeInfo& rhs);

    /*
     * @since 5.91
     */
    bool operator==(const TypeInfo& rhs) const;

    /**
     * The type identifier
     */
    Type::Type type() const;

    /**
     * An internal unique name for the type
     */
    QString name() const;

    /**
     * A user visible translated name for this type
     */
    QString displayName() const;

    /**
     * Construct a TypeInfo from the internal type name.
     * The internal type name is case insensitive
     */
    static TypeInfo fromName(const QString& name);

private:
    const std::unique_ptr<TypeInfoPrivate> d;
};
}

#endif // LINGMOFILEMETADATA_TYPE_INFO_H
