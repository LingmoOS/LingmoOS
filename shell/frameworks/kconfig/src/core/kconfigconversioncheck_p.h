/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Thomas Braxton <brax108@cox.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIG_CONVERSION_CHECK_P_H
#define KCONFIG_CONVERSION_CHECK_P_H

#include <QDate>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QUuid>
#include <QVariant>

class QColor;
class QFont;

namespace KConfigConversionCheck
{
// used to distinguish between supported/unsupported types
struct supported {
};
struct unsupported {
};

// traits type class to define support for constraints
template<typename T>
struct QVconvertible {
    typedef unsupported toQString;
    typedef unsupported toQVariant;
};

// constraint classes
template<typename T>
struct type_toQString {
    void constraint()
    {
        supported x = y;
        Q_UNUSED(x);
    }
    typename QVconvertible<T>::toQString y;
};

template<typename T>
struct type_toQVariant {
    void constraint()
    {
        supported x = y;
        Q_UNUSED(x);
    }
    typename QVconvertible<T>::toQVariant y;
};

// check if T is convertible to QString thru QVariant
// if not supported can't be used in QList<T> functions
template<typename T>
inline void to_QString()
{
    void (type_toQString<T>::*x)() = &type_toQString<T>::constraint;
    Q_UNUSED(x);
}

// check if T is convertible to QVariant & supported in readEntry/writeEntry
template<typename T>
inline void to_QVariant()
{
    void (type_toQVariant<T>::*x)() = &type_toQVariant<T>::constraint;
    Q_UNUSED(x);
}

// define for all types handled in readEntry/writeEntry
// string_support - is supported by QVariant(type).toString(),
//                  can be used in QList<T> functions
// variant_support - has a QVariant constructor
#define QVConversions(type, string_support, variant_support)                                                                                                   \
    template<>                                                                                                                                                 \
    struct QVconvertible<type> {                                                                                                                               \
        typedef string_support toQString;                                                                                                                      \
        typedef variant_support toQVariant;                                                                                                                    \
    }

// The only types needed here are the types handled in readEntry/writeEntry
// the default QVconvertible will take care of the rest.
QVConversions(bool, supported, supported);
QVConversions(int, supported, supported);
QVConversions(unsigned int, supported, supported);
QVConversions(long long, supported, supported);
QVConversions(unsigned long long, supported, supported);
QVConversions(float, supported, supported);
QVConversions(double, supported, supported);
QVConversions(QString, supported, supported);
QVConversions(QColor, unsupported, supported);
QVConversions(QFont, supported, supported);
QVConversions(QDateTime, unsupported, supported);
QVConversions(QDate, unsupported, supported);
QVConversions(QSize, unsupported, supported);
QVConversions(QRect, unsupported, supported);
QVConversions(QPoint, unsupported, supported);
QVConversions(QSizeF, unsupported, supported);
QVConversions(QRectF, unsupported, supported);
QVConversions(QPointF, unsupported, supported);
QVConversions(QByteArray, supported, supported);
QVConversions(QStringList, unsupported, supported);
QVConversions(QVariantList, unsupported, supported);
QVConversions(QUrl, supported, supported);
QVConversions(QList<QUrl>, unsupported, supported);
QVConversions(QUuid, supported, supported);
}

#endif
