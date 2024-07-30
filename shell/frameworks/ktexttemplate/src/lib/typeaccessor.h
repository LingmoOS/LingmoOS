/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_TYPEACCESSOR_H
#define KTEXTTEMPLATE_TYPEACCESSOR_H

#include "ktexttemplate_export.h"

#include <QObject>

/// @file

namespace KTextTemplate
{

#ifndef K_DOXYGEN
template<typename T>
struct TypeAccessor {
    static QVariant lookUp(const T object, const QString &property);
};

template<typename T>
struct TypeAccessor<T *> {
    static QVariant lookUp(const T *const object, const QString &property);
};

template<typename T>
struct TypeAccessor<T &> {
    static QVariant lookUp(const T &object, const QString &property);
};
#endif
}

#endif
