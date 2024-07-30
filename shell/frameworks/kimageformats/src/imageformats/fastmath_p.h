/*
    Approximated math functions used into conversions.

    SPDX-FileCopyrightText: Edward Kmett
    SPDX-FileCopyrightText: 2023 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef FASTMATH_P_H
#define FASTMATH_P_H

#include <QtGlobal>

/*!
 * \brief fastPow
 * Based on Edward Kmett code released into the public domain.
 * See also: https://github.com/ekmett/approximate
 */
inline double fastPow(double x, double y)
{
    union {
        double d;
        qint32 i[2];
    } u = {x};
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    u.i[1] = qint32(y * (u.i[1] - 1072632447) + 1072632447);
    u.i[0] = 0;
#else // never tested
    u.i[0] = qint32(y * (u.i[0] - 1072632447) + 1072632447);
    u.i[1] = 0;
#endif
    return u.d;
}

#endif // FASTMATH_P_H
