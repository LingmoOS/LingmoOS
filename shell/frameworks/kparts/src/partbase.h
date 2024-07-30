/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_PARTBASE_H
#define _KPARTS_PARTBASE_H

#include <kparts/kparts_export.h>

#include <QObject>

#include <KXMLGUIClient>
#include <memory>

// Internal:
// As KParts::PartBase is inherited by KParts::Part which also inheriting from QObject,
// which already has a protected d_ptr member, the macro Q_DECLARE_PRIVATE cannot be used
// as it references d_ptr without any class qualifier, which is ambiguous then.
#define KPARTS_DECLARE_PRIVATE(Class) Q_DECLARE_PRIVATE_D(PartBase::d_ptr, Class)

namespace KParts
{
class PartBasePrivate;

/**
 * @class PartBase partbase.h <KParts/PartBase>
 *
 * @short Base class for all parts.
 */
class KPARTS_EXPORT PartBase : virtual public KXMLGUIClient
{
    KPARTS_DECLARE_PRIVATE(PartBase)

public:
    /**
     *  Constructor.
     */
    PartBase();

    /**
     *  Destructor.
     */
    ~PartBase() override;

    /**
     *  Internal method. Called by KParts::Part to specify the parent object for plugin objects.
     *
     * @internal
     */
    void setPartObject(QObject *object);
    QObject *partObject() const;

protected:
    KPARTS_NO_EXPORT explicit PartBase(PartBasePrivate &dd);

    std::unique_ptr<PartBasePrivate> const d_ptr;

private:
    Q_DISABLE_COPY(PartBase)
};

} // namespace

#endif
