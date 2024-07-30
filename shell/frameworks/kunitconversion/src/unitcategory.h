/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KUNITCONVERSION_UNITCATEGORY_H
#define KUNITCONVERSION_UNITCATEGORY_H

#include "kunitconversion/kunitconversion_export.h"
#include "unit.h"
#include "value.h"

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QString>
#include <QStringList>

#include <chrono>

class QNetworkReply;

namespace KUnitConversion
{
class UnitCategoryPrivate;
class UpdateJob;

/**
 * @short Class to define a category of units of measurement
 *
 * This is a class to define a category of units of measurement.
 *
 * @see Converter, Unit, Value
 *
 * @author Petri Damstén <damu@iki.fi>
 * @author John Layt <jlayt@kde.org>
 */

class KUNITCONVERSION_EXPORT UnitCategory
{
public:
    /**
     * Null constructor
     **/
    UnitCategory();

    /**
     * Copy constructor, copy @p other to this.
     **/
    UnitCategory(const UnitCategory &other);

    ~UnitCategory();

    /**
     * Assignment operator, assign @p other to this.
     **/
    UnitCategory &operator=(const UnitCategory &other);

    /**
     * Move-assigns @p other to this UnitCategory instance, transferring the
     * ownership of the managed pointer to this instance.
     **/
    UnitCategory &operator=(UnitCategory &&other);

    /**
     * @return @c true if this UnitCategory is equal to the @p other UnitCategory.
     **/
    bool operator==(const UnitCategory &other) const;

    /**
     * @return @c true if this UnitCategory is not equal to the @p other UnitCategory.
     **/
    bool operator!=(const UnitCategory &other) const;

    /**
     * @return returns true if this UnitCategory is null
     **/
    bool isNull() const;

    /**
     * @return category id.
     **/
    CategoryId id() const;

    /**
     * Returns name for the unit category.
     *
     * @return Translated name for category.
     **/
    QString name() const;

    /**
     * @return unit category description
     **/
    QString description() const;

    /**
     * Returns default unit.
     *
     * @return default unit.
     **/
    Unit defaultUnit() const;

    /**
     * Check if unit category has a unit.
     *
     * @return True if unit is found
     **/
    bool hasUnit(const QString &unit) const;

    /**
     * Return unit for string.
     *
     * @return Pointer to unit class.
     **/
    Unit unit(const QString &s) const;

    /**
     * Return unit for unit enum.
     *
     * @return Pointer to unit class.
     **/
    Unit unit(UnitId unitId) const;

    /**
     * Return units in this category.
     *
     * @return list of units.
     **/
    QList<Unit> units() const;

    /**
     * Return most common units in this category.
     *
     * @return list of units.
     **/
    QList<Unit> mostCommonUnits() const;

    /**
     * Return all unit names, short names and unit synonyms in this category.
     *
     * @return list of units.
     **/
    QStringList allUnits() const;

    /**
     * Convert value to another unit selected by string.
     *
     * @param value value to convert
     * @param toUnit unit to convert to. If empty default unit is used.
     * @return converted value
     **/
    Value convert(const Value &value, const QString &toUnit = QString()) const;

    /**
     * Convert value to another unit selected by enum.
     *
     * @param value value to convert
     * @param toUnit unit to convert to.
     * @return converted value
     **/
    Value convert(const Value &value, UnitId toUnit) const;

    /**
     * Convert value to another unit.
     *
     * @param value value to convert
     * @param toUnit unit to be used for conversion
     * @return converted value
     **/
    Value convert(const Value &value, const Unit &toUnit) const;

    /**
     * @return true if category has conversion table that needs to be updated via online access, otherwise false
     * @see syncConversionTable()
     */
    bool hasOnlineConversionTable() const;

    /**
     * Request an update of the online conversion table when it is older than @p updateSkipPeriod.
     *
     * Returned jobs are automatically deleted, ie. it is safe to ignore the return value if you
     * do not care about being notified about the completion (or failure) of the update process.
     * Calling this method while another update is already in progress will not trigger another update
     * but instead allows you to watch the already ongoing update.
     * Performing conversions before the update has completed will return results based on the old
     * conversion table, if available.
     *
     * @note This method must be called from the main thread!
     *
     * @returns an UpdateJob if an update is necessary or already running, @c nullptr otherwise.
     * @see UpdateJob
     * @since 6.0
     */
    UpdateJob* syncConversionTable(std::chrono::seconds updateSkipPeriod = std::chrono::hours(24));

private:
    friend class Unit;
    friend class UnitCategoryPrivate;

    KUNITCONVERSION_NO_EXPORT explicit UnitCategory(UnitCategoryPrivate *dd);

protected:
    QExplicitlySharedDataPointer<UnitCategoryPrivate> d;
};


/**
 * Dynamic conversion data update job.
 * Created via the factory methods in KUnitConversion::Updater, useful for
 * getting notified about an update having completed.
 * Update jobs are automatically deleted on completion, but it is also save to delete
 * instances manually.
 *
 * @see UnitCategory
 * @since 6.0
 */
class KUNITCONVERSION_EXPORT UpdateJob : public QObject
{
    Q_OBJECT
public:
    ~UpdateJob();

Q_SIGNALS:
    void finished();

private:
    friend class UnitCategoryPrivate;
    explicit UpdateJob(QNetworkReply *dd);
    QNetworkReply *d;
};

} // KUnitConversion namespace

Q_DECLARE_TYPEINFO(KUnitConversion::UnitCategory, Q_RELOCATABLE_TYPE);

#endif
