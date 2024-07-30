/*
 *   SPDX-FileCopyrightText: 2007-2009 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2014 John Layt <jlayt@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KUNITCONVERSION_UNIT_P_H
#define KUNITCONVERSION_UNIT_P_H

#include "unit.h"
#include "unitcategory.h"
#include "unitcategory_p.h"

#include <KLocalizedString>

namespace KUnitConversion
{
class UnitPrivate : public QSharedData
{
public:
    UnitPrivate();

    UnitPrivate(CategoryId categoryId,
                UnitId id,
                qreal multiplier,
                const QString &symbol,
                const QString &description,
                const QString &matchString,
                const KLocalizedString &symbolString,
                const KLocalizedString &realString,
                const KLocalizedString &integerString);

    virtual ~UnitPrivate();

    UnitPrivate *clone();
    bool operator==(const UnitPrivate &other) const;
    bool operator!=(const UnitPrivate &other) const;

    void setUnitMultiplier(qreal multiplier);
    qreal unitMultiplier() const;

    virtual qreal toDefault(qreal value) const;
    virtual qreal fromDefault(qreal value) const;

    static inline Unit makeUnit(UnitPrivate *dd)
    {
        return Unit(dd);
    }
    static inline Unit makeUnit(CategoryId categoryId,
                         UnitId id,
                        qreal multiplier,
                        const QString &symbol,
                        const QString &description,
                        const QString &matchString,
                        const KLocalizedString &symbolString,
                        const KLocalizedString &realString,
                        const KLocalizedString &integerString)
    {
        return Unit(new UnitPrivate(categoryId, id, multiplier, symbol, description, matchString, symbolString, realString, integerString));
    }

    CategoryId m_categoryId;
    UnitId m_id;
    qreal m_multiplier;
    QString m_symbol;
    QString m_description;
    QString m_matchString;
    KLocalizedString m_symbolString;
    KLocalizedString m_realString;
    KLocalizedString m_integerString;
    UnitCategoryPrivate *m_category = nullptr; // emulating a weak_ptr, as we otherwise have an undeleteable reference cycle
};

} // KUnitConversion namespace

#endif // KUNITCONVERSION_UNIT_P_H
