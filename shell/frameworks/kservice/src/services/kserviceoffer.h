/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICEOFFER_H
#define KSERVICEOFFER_H

#include <kservice.h>

#include <memory>

class KServiceOfferPrivate;

/**
 * @internal
 *
 * This class holds the user-specific preferences of a service
 * (whether it can be a default offer or not, how big is the preference
 * for this offer, ...). Basically it is a reference to a
 * KService, a number that represents the user's preference (bigger
 * is better) and a flag whether the KService can be used as default.
 *
 * @see KService
 * @short Holds the user's preference of a service.
 */
class KSERVICE_EXPORT KServiceOffer // exported for kbuildsycoca
{
public:
    /**
     * Create an invalid service offer.
     */
    KServiceOffer();

    /**
     * Copy constructor.
     * Shallow copy (the KService will not be copied).
     */
    KServiceOffer(const KServiceOffer &);

    /**
     * Creates a new KServiceOffer.
     * @param service a pointer to the KService
     * @param pref the user's preference value, must be positive,
     *              bigger is better
     * @param mimeTypeInheritanceLevel level of MIME type inheritance
     *       which allows this service to handling the MIME type.
     *       0 if no inheritance involved, 1 for parent MIME type, etc.
     * @since 5.71
     */
    KServiceOffer(const KService::Ptr &service, int pref, int mimeTypeInheritanceLevel);

    ~KServiceOffer();

    /**
     * A service is bigger that the other when it can be default
     * (and the other is not) and its preference value it higher.
     */
    bool operator<(const KServiceOffer &) const;

    /**
     * Assignment operator
     */
    KServiceOffer &operator=(const KServiceOffer &other);

    /**
     * The bigger this number is, the better is this service.
     * @return the preference number (negative numbers will be
     *         returned by invalid service offers)
     */
    int preference() const;

    /**
     * The bigger this number is, the better is this service.
     * Set the preference number
     * @internal - only for KMimeTypeTrader
     */
    void setPreference(int p);

    /**
     * The service which this offer is about.
     * @return the service this offer is about, can be @c nullptr
     *         in valid offers or when not set
     */
    KService::Ptr service() const;

    /**
     * Check whether the entry is valid. A service is valid if
     * its preference value is positive.
     * @return true if the service offer is valid
     */
    bool isValid() const;

    /**
     * When copying an offer from a parent MIME type, remember that it's an inherited capability
     * (for sorting purposes; we prefer a handler for postscript over any text/plain handler)
     */
    void setMimeTypeInheritanceLevel(int level);

    /**
     * Mimetype inheritance level
     * @internal
     */
    int mimeTypeInheritanceLevel() const;

private:
    std::unique_ptr<KServiceOfferPrivate> const d;
};

/**
 * A list of weighted offers.
 */
typedef QList<KServiceOffer> KServiceOfferList;

QDebug operator<<(QDebug dbg, const KServiceOffer &offer);

#endif /* KSERVICEOFFER_H */
