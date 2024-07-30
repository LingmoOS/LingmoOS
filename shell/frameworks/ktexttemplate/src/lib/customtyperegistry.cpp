/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Michael Jansen <kde@michael-jansen.biz>
  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "customtyperegistry_p.h"

#include "metaenumvariable_p.h"
#include "safestring.h"

#include <QLoggingCategory>
#include <QQueue>
#include <QStack>
#include <QStringList>

Q_LOGGING_CATEGORY(KTEXTTEMPLATE_CUSTOMTYPE, "kf.texttemplate.customtyperegistry")

using namespace KTextTemplate;

CustomTypeRegistry::CustomTypeRegistry()
{
    // KTextTemplate Types
    registerBuiltInMetatype<SafeString>();
    registerBuiltInMetatype<MetaEnumVariable>();
}

void CustomTypeRegistry::registerLookupOperator(int id, MetaType::LookupFunction f)
{
    CustomTypeInfo &info = types[id];
    info.lookupFunction = f;
}

QVariant CustomTypeRegistry::lookup(const QVariant &object, const QString &property) const
{
    if (!object.isValid())
        return {};
    const auto id = object.userType();
    MetaType::LookupFunction lf;

    {
        auto it = types.constFind(id);
        if (it == types.constEnd()) {
            qCWarning(KTEXTTEMPLATE_CUSTOMTYPE) << "Don't know how to handle metatype" << QMetaType(id).name();
            // :TODO: Print out error message
            return {};
        }

        const CustomTypeInfo &info = it.value();
        if (!info.lookupFunction) {
            qCWarning(KTEXTTEMPLATE_CUSTOMTYPE) << "No lookup function for metatype" << QMetaType(id).name();
            lf = nullptr;
            // :TODO: Print out error message
            return {};
        }

        lf = info.lookupFunction;
    }

    return lf(object, property);
}

bool CustomTypeRegistry::lookupAlreadyRegistered(int id) const
{
    auto it = types.constFind(id);
    if (it != types.constEnd()) {
        return it.value().lookupFunction != nullptr;
    }
    return false;
}
