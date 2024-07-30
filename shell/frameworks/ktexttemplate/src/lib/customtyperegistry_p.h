/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Michael Jansen <kde@michael-jansen.biz>
  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef CUSTOMTYPESREGISTRY_P_H
#define CUSTOMTYPESREGISTRY_P_H

#include "metatype.h"

#include <QMutex>

namespace KTextTemplate
{

struct CustomTypeInfo {
public:
    CustomTypeInfo()
        : lookupFunction(nullptr)
    {
    }

    KTextTemplate::MetaType::LookupFunction lookupFunction;
};

struct CustomTypeRegistry {
    CustomTypeRegistry();

    void registerLookupOperator(int id, MetaType::LookupFunction f);

    template<typename RealType, typename HandleAs>
    int registerBuiltInMetatype()
    {
        QVariant (*lf)(const QVariant &, const QString &) = LookupTrait<RealType &, HandleAs &>::doLookUp;

        const int id = qMetaTypeId<RealType>();

        registerLookupOperator(id, reinterpret_cast<MetaType::LookupFunction>(lf));

        return id;
    }

    template<typename Type>
    int registerBuiltInMetatype()
    {
        return registerBuiltInMetatype<Type, Type>();
    }

    QVariant lookup(const QVariant &object, const QString &property) const;
    bool lookupAlreadyRegistered(int id) const;

    QHash<int, CustomTypeInfo> types;
    QMutex mutex;
};
}

#endif
