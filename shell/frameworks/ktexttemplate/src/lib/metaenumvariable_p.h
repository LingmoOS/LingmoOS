/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_METAENUMVARIABLE_P_H
#define KTEXTTEMPLATE_METAENUMVARIABLE_P_H

#include <QMetaEnum>

struct MetaEnumVariable {
    MetaEnumVariable()
        : value(-1)
    {
    }

    MetaEnumVariable(QMetaEnum _enumerator)
        : enumerator(_enumerator)
        , value(-1)
    {
    }

    MetaEnumVariable(QMetaEnum _enumerator, int _value)
        : enumerator(_enumerator)
        , value(_value)
    {
    }

    bool operator==(const MetaEnumVariable &other) const
    {
        return (enumerator.scope() == other.enumerator.scope() && enumerator.name() == other.enumerator.name()) && value == other.value;
    }

    bool operator==(int otherValue) const
    {
        return value == otherValue;
    }

    bool operator<(const MetaEnumVariable &other) const
    {
        return value < other.value;
    }

    bool operator<(int otherValue) const
    {
        return value < otherValue;
    }

    QMetaEnum enumerator;
    int value;
};

Q_DECLARE_METATYPE(MetaEnumVariable)

#endif
