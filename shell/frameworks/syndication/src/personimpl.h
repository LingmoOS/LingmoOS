/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_PERSONIMPL_H
#define SYNDICATION_PERSONIMPL_H

#include "syndication_export.h"
#include <syndication/person.h>

#include <QString>

namespace Syndication
{
class PersonImpl;

//@cond PRIVATE
typedef QSharedPointer<PersonImpl> PersonImplPtr;

/**
 * @internal
 */
class SYNDICATION_EXPORT PersonImpl : public Person
{
public:
    PersonImpl();
    PersonImpl(const QString &name, const QString &uri, const QString &email);

    Q_REQUIRED_RESULT bool isNull() const override
    {
        return m_null;
    }
    Q_REQUIRED_RESULT QString name() const override
    {
        return m_name;
    }
    Q_REQUIRED_RESULT QString uri() const override
    {
        return m_uri;
    }
    Q_REQUIRED_RESULT QString email() const override
    {
        return m_email;
    }

private:
    bool m_null;
    QString m_name;
    QString m_uri;
    QString m_email;
};
//@endcond

} // namespace Syndication

#endif // SYNDICATION_PERSONIMPL_H
