/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Carsten Pfeiffer <pfeiffer@kde.org>

    This SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_ADDRESSEEHELPER_P_H
#define KCONTACTS_ADDRESSEEHELPER_P_H

#include "kcontacts_export.h"

#include <QSet>

namespace KContacts
{

// TODO KF6: unexport and turn into an implementation detail
// this is unused externally, both as code as well as via the config file
// so we only need this internally and can probably also drop the config
// file access

/**
 * This singleton class stores static data, which is shared
 * by all Addressee objects. It maintains three lists of
 * strings, which can be queried using this class:
 *
 * - a list of honoric prefixes, like "Mrs.", "Prof." etc,
 *   see containsTitle()
 * - a list of inclusions, such as "van" or "de", see
 *   containsPrefix()
 * - a list of honoric suffixes, such as "I" or "Jr.", see
 *   containsSuffix()
 *
 * All of these lists have a hardcoded and a configurable
 * part. The configurable part is found in @c kabcrc, group
 * @c General, fields @c Prefixes, @c Inclusions, and
 * @c Suffixes.
 *
 * In addition to the above, this class stores one conveniece
 * setting: it stores whether or not a single name component
 * should be interpreted as a family name (see
 * treatAsFamilyName()). The corresponding configuration
 * field is @c TreatAsFamilyName.
 */
class AddresseeHelper
{
public:
    /**
     * Singleton interface to this class
     *
     * @return a pointer to the unique instance of this class.
     */
    static AddresseeHelper *self();

    /**
     * Queries the list of honoric prefixes.
     *
     * @param title the honoric prefix to search for
     * @return @c true, if @p title was found in the list,
     *         @c false otherwise
     */
    Q_REQUIRED_RESULT bool containsTitle(const QString &title) const;

    /**
     * Queries the list of inclusions.
     *
     * @param prefix the inclusion to search for
     * @return @c true, if @p prefix was found in the list,
     *         @c false otherwise
     */
    Q_REQUIRED_RESULT bool containsPrefix(const QString &prefix) const;

    /**
     * Queries the list of honoric suffixes.
     *
     * @param suffix the honoric suffix to search for
     * @return @c true, if @p suffix was found in the list,
     *         @c false otherwise
     */
    Q_REQUIRED_RESULT bool containsSuffix(const QString &suffix) const;

    /**
     * Returns whether or not a single name component should
     * be interpreted as a family name.
     *
     * @return @c true if single name component is a family name,
     *         @c false otherwise.
     */
    Q_REQUIRED_RESULT bool treatAsFamilyName() const;

    /** @internal */
    AddresseeHelper();

    ~AddresseeHelper();

private:
    /**
     * Recreates the static data and reparses the configuration.
     */
    void initSettings();

    QSet<QString> mTitles;
    QSet<QString> mPrefixes;
    QSet<QString> mSuffixes;
    bool mTreatAsFamilyName;
};
}

#endif
