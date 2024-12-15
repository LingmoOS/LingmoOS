// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DABSTRACTUNITFORMATTER_H
#define DABSTRACTUNITFORMATTER_H

#include "dtkcore_global.h"

#include <QPair>
#include <QList>

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DAbstractUnitFormatter
{
public:
    DAbstractUnitFormatter();
    ~DAbstractUnitFormatter();

protected:
    virtual int unitMax() const = 0;
    virtual int unitMin() const = 0;
    virtual uint unitConvertRate(int unitId) const = 0;
    virtual qreal unitValueMax(int unitId) const { return unitConvertRate(unitId) - 1; }
    virtual qreal unitValueMin(int unitId) const { Q_UNUSED(unitId); return 1; }
    virtual QString unitStr(int unitId) const = 0;

public:
    qreal formatAs(qreal value, int currentUnit, const int targetUnit) const;
    QPair<qreal, int> format(const qreal value, const int unit) const;
    QList<QPair<qreal, int>> formatAsUnitList(const qreal value, int unit) const;
};

DCORE_END_NAMESPACE

#endif // DABSTRACTUNITFORMATTER_H
