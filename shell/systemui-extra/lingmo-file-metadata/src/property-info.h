/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef _LINGMOFILEMETADATA_PROPERTYINFO_H
#define _LINGMOFILEMETADATA_PROPERTYINFO_H

#include <QString>
#include <QVariant>
#include <QObject>
#include "properties.h"

#include <memory>

namespace LingmoUIFileMetadata {

class PropertyInfoPrivate;
/**
 * \class PropertyInfo property-info.h
 *
 * The PropertyInfo class can be used to obtain extra information
 * about any property. It is commonly used be indexers in order
 * to obtain a translatable name of the property along with
 * additional information such as if the property should be indexed.
 */
class LINGMOFILEMETADATA_EXPORT PropertyInfo : public QObject
{
    Q_OBJECT
public:
    PropertyInfo();
    explicit PropertyInfo(Property::Property property);
    PropertyInfo(const PropertyInfo& pi);
    ~PropertyInfo();

    PropertyInfo& operator=(const PropertyInfo& rhs);
    bool operator==(const PropertyInfo& rhs) const;

    /**
     * The enumeration which represents this property
     */
    Property::Property property() const;

    /**
     * The internal unique name used to refer to the property
     */
    QString name() const;

    /**
     * A user visible name of the property
     */
    QString displayName() const;

    /**
     * The type the value of this property should be.
     * Eg - Property::Height should be an integer
     */
    QVariant::Type valueType() const;

    /**
     * Indicates if this property requires indexing or should just be stored.
     * Eg - Property::Height does not need to be part of the global index.
     *      When a user searches for 600, they should not get images with
     *      that height
     *
     * This is just a recommendation.
     */
    bool shouldBeIndexed() const;

    /**
     * Construct a PropertyInfo from the internal property name.
     * The internal property name is case insensitive
     */
    static PropertyInfo fromName(const QString& name);

    /**
     * Returns the value of the property as a QString with added formatting,
     * added units if needed, and translated enums.
     * @since 5.56
     */
    QString formatAsDisplayString(const QVariant& value) const;

private:
    const std::unique_ptr<PropertyInfoPrivate> d;
};

} // namespace
Q_DECLARE_METATYPE(LingmoUIFileMetadata::PropertyInfo)


#endif // _LINGMOFILEMETADATA_PROPERTYINFO_H
