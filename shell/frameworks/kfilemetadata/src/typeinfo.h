/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KFILEMETADATA_TYPEINFO_H
#define KFILEMETADATA_TYPEINFO_H

#include "types.h"
#include "kfilemetadata_export.h"
#include <QString>
#include <QVariant>

namespace KFileMetaData {

class TypeInfoPrivate;
/**
 * \class TypeInfo typeinfo.h <KFileMetaData/TypeInfo>

 * The TypeInfo class can be used to obtain a rough type
 * classification for a file.
 *
 * It is somewhat related to the MIME "<type>/<subtype>"
 * classification, but provides additional types, e.g.
 * MIME does not provide a generic "Archive" type.
 *
 * \sa <KFileMetaData/Types>
 */
class KFILEMETADATA_EXPORT TypeInfo
{
public:
    TypeInfo();
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

    /**
     * Get all supported property names
     */
    static QStringList allNames();

private:
    const TypeInfoPrivate* d;
};

} // namespace
Q_DECLARE_METATYPE(KFileMetaData::TypeInfo)

#endif // KFILEMETADATA_TYPEINFO_H
