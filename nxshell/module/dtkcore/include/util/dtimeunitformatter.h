// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTIMEUNITFORMATTER_H
#define DTIMEUNITFORMATTER_H

#include "dtkcore_global.h"
#include "dabstractunitformatter.h"

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DTimeUnitFormatter : public DAbstractUnitFormatter
{
public:
    DTimeUnitFormatter();

    enum TimeUnits
    {
        Seconds,
        Minute,
        Hour,
        Day,
    };

    QString unitStr(int unitId) const override;

protected:
    int unitMax() const override { return Day; }
    int unitMin() const override { return Seconds; }
    uint unitConvertRate(int unitId) const override;
};

DCORE_END_NAMESPACE

#endif // DTIMEUNITFORMATTER_H
