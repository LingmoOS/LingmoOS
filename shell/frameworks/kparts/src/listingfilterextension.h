/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2012 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPARTS_LISTINGFILTEREXTENSION_H
#define KPARTS_LISTINGFILTEREXTENSION_H

#include <kparts/kparts_export.h>

#include <QObject>
#include <memory>

class KFileItemList;

namespace KParts
{
class ReadOnlyPart;
class ListingFilterExtensionPrivate;

/**
 * @class ListingFilterExtension listingfilterextension.h <KParts/ListingFilterExtension>
 *
 * @short An extension for filtering listings.
 *
 * This extension is intended to be implemented by parts that provide listing
 * services, e.g. file management parts and is intended to provide a generic
 * API for filtering any listing through keywords, wildcard characters and/or
 * content-type.
 *
 * Examples:
 *
 * To show items that only match the term "kde"
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext && (ext->supportedFilterModes() & KParts::ListingFilterExtension::SubString)) {
 *       ext->setFilter(KParts::ListingFilterExtension::SubString, QLatin1String("kde"));
 *    }
 * \endcode
 *
 * To show items that only match "text/html"
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext && (ext->supportedFilterModes() & KParts::ListingFilterExtension::MimeType)) {
 *        ext->setFilter(KParts::ListingFilterExtension::MimeType, QLatin1String("text/html"));
 *    }
 * \endcode
 *
 * To show items that only match the wildcard string "*.txt"
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext && (ext->supportedFilterModes() & KParts::ListingFilterExtension::WildCard)) {
 *        ext->setFilter(KParts::ListingFilterExtension::WildCard, QLatin1String("*.txt"));
 *    }
 * \endcode
 *
 * To show items that match multiple mime types, e.g. text/html & application/xml:
 *
 * \code
 *    KParts::ListingFilterExtension* ext = KParts::ListingFilterExtension::childObject(part);
 *    if (ext &&
 *        (ext->supportedFilterModes() & KParts::ListingFilterExtension::MimeType) &&
 *        ext->supportsMultipleFilters(KParts::ListingFilterExtension::MimeType)) {
 *        QStringList mimeTypes = ext->filter(KParts::ListingFilterExtension::MimeType).toStringList();
 *        mimeTypes << QLatin1String("text/html") << QLatin1String("application/xml");
 *        ext->setFilter(KParts::ListingFilterExtension::MimeType, mimeTypes);
 *    }
 * \endcode
 *
 * @since 4.9.2
 */
class KPARTS_EXPORT ListingFilterExtension : public QObject
{
    Q_OBJECT

public:
    /**
     * Supported file filtering modes modes.
     * @FilterModes
     */
    enum FilterMode {
        None = 0x00,
        MimeType = 0x01, /*!< Filter by mime type, e.g. "text/plain". */
        SubString = 0x02, /*!< Filter by matching any part of a file or directory name, e.g. "Documents" */
        WildCard = 0x04, /*!< Filter by using wildcard matches, e.g. "*.txt" */
    };

    /**
     * Stores a combination of #FilterMode values.
     */
    Q_DECLARE_FLAGS(FilterModes, FilterMode)

    /*! Constructor */
    explicit ListingFilterExtension(KParts::ReadOnlyPart *parent);

    /*! Destructor */
    ~ListingFilterExtension() override;

    /**
     * Queries @p obj for a child object which inherits from this class.
     */
    static ListingFilterExtension *childObject(QObject *obj);

    /**
     * Returns the OR'ed value of the file filter modes supported by the part
     * that implements this extension.
     *
     * By default this function returns None.
     */
    virtual FilterModes supportedFilterModes() const;

    /**
     * Returns true if the part that implements this extension allows
     * the use of multiple filters for the given filtering @p mode.
     *
     * By default this function returns false.
     */
    virtual bool supportsMultipleFilters(FilterMode mode) const;

    /**
     * Returns the currently set filters for the given @p mode.
     *
     * @param mode the desired filter mode as specified in @ref FilterMode.
     */
    virtual QVariant filter(FilterMode mode) const = 0;

    /**
     * Sets the file @p filter that should be applied by the part that
     * implements this extension for the given filtering @p mode.
     *
     * To remove a filter for a given filter mode, simply call this function with
     * the desired mode and the @p filter parameter set to a NULL variant.
     *
     * The second parameter can be
     *
     * @param mode the desired filter mode as specified in @ref FilterMode.
     * @param filter a list of filter texts based on the selected mode.
     */
    virtual void setFilter(FilterMode mode, const QVariant &filter) = 0;

private:
    std::unique_ptr<ListingFilterExtension> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ListingFilterExtension::FilterModes)

}

#endif /* KPARTS_LISTINGFILTEREXTENSION_H */
