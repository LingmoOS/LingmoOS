/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

// Self
#include "Features.h"

// Qt
#include <QDBusConnection>

// Utils
#include <utils/d_ptr_implementation.h>

// Local
#include "common/dbus/common.h"
#include "featuresadaptor.h"

class Features::Private
{
};

Features::Features(QObject *parent)
    : Module(QStringLiteral("features"), parent)
    , d()
{
    new FeaturesAdaptor(this);
    QDBusConnection::sessionBus().registerObject(KAMD_DBUS_OBJECT_PATH("Features"), this);
}

Features::~Features()
{
}

// Features object is just a gateway to the other KAMD modules.
// This is a convenience method to pass the request down to the module

template<typename RetType, typename Function>
static RetType passToModule(const QString &key, RetType defaultResult, Function f)
{
    if (key.isEmpty()) {
        return defaultResult;
    }

    const auto params = key.split(QLatin1Char('/'));
    const auto module = Module::get(params.first());

    if (!module) {
        return defaultResult;
    }

    return f(static_cast<Module *>(module), params.mid(1));
}

#define FEATURES_PASS_TO_MODULE(RetType, DefaultResult, What)                                                                                                  \
    passToModule(key, DefaultResult, [=](Module *module, const QStringList &params) -> RetType {                                                               \
        What                                                                                                                                                   \
    });

bool Features::IsFeatureOperational(const QString &key) const
{
    return FEATURES_PASS_TO_MODULE(bool, false, return module->isFeatureOperational(params););
}

QStringList Features::ListFeatures(const QString &key) const
{
    if (key.isEmpty()) {
        return Module::get().keys();
    }

    return FEATURES_PASS_TO_MODULE(QStringList, QStringList(), return module->listFeatures(params););
}

QDBusVariant Features::GetValue(const QString &key) const
{
    return FEATURES_PASS_TO_MODULE(QDBusVariant, QDBusVariant(), return module->featureValue(params););
}

void Features::SetValue(const QString &key, const QDBusVariant &value)
{
    FEATURES_PASS_TO_MODULE(bool, true, module->setFeatureValue(params, value); return true;);
}

#undef FEATURES_PASS_TO_MODULE

#include "moc_Features.cpp"
