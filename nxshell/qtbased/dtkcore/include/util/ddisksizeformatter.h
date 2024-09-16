// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISKSIZEFORMATTER_H
#define DISKSIZEFORMATTER_H

#include "dabstractunitformatter.h"

DCORE_BEGIN_NAMESPACE

class LIBDTKCORESHARED_EXPORT DDiskSizeFormatter : public DAbstractUnitFormatter
{
public:
    DDiskSizeFormatter();

    enum DiskUnits
    {
        B,
        K,
        M,
        G,
        T,
    };

    QString unitStr(int unitId) const override;

    DDiskSizeFormatter rate(int rate);

protected:
    int unitMin() const override { return B; }
    int unitMax() const override { return T; }
    uint unitConvertRate(int unitId) const override { Q_UNUSED(unitId); return m_rate; }

private:
    int m_rate = 1000;
};

DCORE_END_NAMESPACE

#endif // DISKSIZEFORMATTER_H
