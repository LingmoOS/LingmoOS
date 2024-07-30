/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VCARDLINE_H
#define VCARDLINE_H

#include <QString>
#include <QStringList>
#include <QVariant>

#include "kcontacts_export.h"

#include "../parametermap_p.h"

namespace KContacts
{
class KCONTACTS_EXPORT VCardLine
{
public:
    typedef QList<VCardLine> List;

    VCardLine();
    VCardLine(const QString &identifier);
    VCardLine(const QString &identifier, const QVariant &value);
    VCardLine(const VCardLine &line);

    ~VCardLine();

    VCardLine &operator=(const VCardLine &line);

    /**
     * Equality operator.
     *
     */
    Q_REQUIRED_RESULT bool operator==(const VCardLine &other) const;

    /**
     * Sets the identifier of this line e.g. UID, FN, CLASS
     *
     * @param identifier The VCard identifier of this line
     */
    void setIdentifier(const QString &identifier);

    /**
     * Returns the identifier of this line.
     */
    Q_REQUIRED_RESULT QString identifier() const;

    /**
     * Sets the value of this line.
     */
    void setValue(const QVariant &value);

    /**
     * Returns the value of this line.
     */
    Q_REQUIRED_RESULT QVariant value() const;

    /**
     * Sets the group the line belongs to.
     */
    void setGroup(const QString &group);

    /**
     * Returns the group the line belongs to.
     */
    Q_REQUIRED_RESULT QString group() const;

    /**
     * Returns whether the line belongs to a group.
     */
    Q_REQUIRED_RESULT bool hasGroup() const;

    /**
     * Returns all parameters.
     */
    Q_REQUIRED_RESULT QStringList parameterList() const;

    /**
     * Add a new parameter to the line.
     *
     * @param param Name of the parameter to insert
     * @param value Value of the parameter to insert
     */
    void addParameter(const QString &param, const QString &value);

    void addParameters(const ParameterMap &params);

    /**
     * Returns the values of a special parameter.
     * You can get a list of all parameters with paramList().
     *
     * @param param Name of the parameter to look for
     */
    Q_REQUIRED_RESULT QStringList parameters(const QString &param) const;

    /**
     * Returns only the first value of a special parameter.
     * You can get a list of all parameters with paramList().
     *
     * @param param Name of the parameter to look for
     */
    Q_REQUIRED_RESULT QString parameter(const QString &param) const;

    /**
     * Returns all parameters
     */
    Q_REQUIRED_RESULT ParameterMap parameterMap() const;

private:
    ParameterMap mParamMap;
    QString mIdentifier;
    QString mGroup;
    QVariant mValue;
};
}

#endif
