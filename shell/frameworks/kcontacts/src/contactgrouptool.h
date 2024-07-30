/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2008 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_CONTACTGROUPTOOL_H
#define KCONTACTS_CONTACTGROUPTOOL_H

#include "kcontacts_export.h"
#include <QString>
class QIODevice;

template<class T>
class QList;

namespace KContacts
{
class ContactGroup;

/**
 * Static methods for converting ContactGroup to XML format and vice versa.
 *
 * @author Kevin Krammer <kevin.krammer@gmx.at>
 * @since 4.3
 */
namespace ContactGroupTool
{
/**
 * Converts XML data coming from a @p device into a contact @p group.
 * If an error occurs, @c false is returned and @p errorMessage is set.
 */
Q_REQUIRED_RESULT KCONTACTS_EXPORT bool convertFromXml(QIODevice *device, ContactGroup &group, QString *errorMessage = nullptr);

/**
 * Converts a contact @p group into XML data and writes them to a @p device.
 * If an error occurs, @c false is returned and @p errorMessage is set.
 */
Q_REQUIRED_RESULT KCONTACTS_EXPORT bool convertToXml(const ContactGroup &group, QIODevice *device, QString *errorMessage = nullptr);

/**
 * Converts XML data coming from a @p device into a @p list of contact groups.
 * If an error occurs, @c false is returned and @p errorMessage is set.
 */
Q_REQUIRED_RESULT KCONTACTS_EXPORT bool convertFromXml(QIODevice *device, QList<ContactGroup> &list, QString *errorMessage = nullptr);

/**
 * Converts a @p list of contact groups into XML data and writes them to a @p device.
 * If an error occurs, @c false is returned and @p errorMessage is set.
 */
Q_REQUIRED_RESULT KCONTACTS_EXPORT bool convertToXml(const QList<ContactGroup> &list, QIODevice *device, QString *errorMessage = nullptr);
}
}

#endif
