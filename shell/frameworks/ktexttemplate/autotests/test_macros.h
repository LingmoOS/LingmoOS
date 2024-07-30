/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#include "metatype.h"

#include <map>

#include "qtunorderedmap.h"

#include <array>

template<typename T>
struct ThreeArray : public std::array<T, 3> {
};

#define DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, KeyType, ValueType)                                                                                       \
    typedef ContainerType<KeyType, ValueType> ContainerType##KeyType##ValueType;                                                                               \
    Q_DECLARE_METATYPE(ContainerType##KeyType##ValueType)

#define DECLARE_ASSOCIATIVE_CONTAINER_TYPES_MINIMAL(ContainerType, ValueType) DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, qint32, ValueType)

#define DECLARE_ASSOCIATIVE_CONTAINER_TYPES_ALL(ContainerType, ValueType)                                                                                      \
    DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, qint16, ValueType)                                                                                            \
    DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, qint32, ValueType)                                                                                            \
    DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, qint64, ValueType)                                                                                            \
    DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, quint16, ValueType)                                                                                           \
    DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, quint32, ValueType)                                                                                           \
    DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, quint64, ValueType)                                                                                           \
    DECLARE_ASSOCIATIVE_CONTAINER(ContainerType, QString, ValueType)

#ifndef MINIMAL_CONTAINER_TESTS
#define DECLARE_ASSOCIATIVE_CONTAINER_TYPES DECLARE_ASSOCIATIVE_CONTAINER_TYPES_ALL
#else
#define DECLARE_ASSOCIATIVE_CONTAINER_TYPES DECLARE_ASSOCIATIVE_CONTAINER_TYPES_MINIMAL
#endif

#endif
