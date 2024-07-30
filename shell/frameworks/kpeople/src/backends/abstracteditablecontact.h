/*
    SPDX-FileCopyrightText: 2019 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KPEOPLE_ABSTRACT_EDITABLE_CONTACT_H
#define KPEOPLE_ABSTRACT_EDITABLE_CONTACT_H

#include "abstractcontact.h"
#include <kpeoplebackend/kpeoplebackend_export.h>

namespace KPeople
{
/**
 * @brief Additionally to @class AbstractContact features, it will allow us
 * to suggest the backend property values for a said key.
 *
 * @since 5.62
 * @internal
 */

class KPEOPLEBACKEND_EXPORT AbstractEditableContact : public AbstractContact
{
public:
    typedef QExplicitlySharedDataPointer<AbstractEditableContact> Ptr;
    typedef QList<AbstractEditableContact::Ptr> List;
    AbstractEditableContact();
    ~AbstractEditableContact() override;

    /**
     * @p key @see AbstractContact for key identifiers
     * @p value suggested value
     *
     * @returns whether the change was successful
     */
    virtual bool setCustomProperty(const QString &key, const QVariant &value) = 0;

private:
    Q_DISABLE_COPY(AbstractEditableContact)
};

}

Q_DECLARE_METATYPE(KPeople::AbstractEditableContact::List)
Q_DECLARE_METATYPE(KPeople::AbstractEditableContact::Ptr)

#endif
