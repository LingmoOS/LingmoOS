// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkgui_global.h"
#include "dplatformnativeinterface.h"
#include "dummysettings.h"
#include "ddynamicmetaobject.h"

#include <QColor>
#include <QDebug>
#include <QGuiApplication>

DGUI_USE_NAMESPACE

static bool buildNativeSettings(QObject *object, quint32 /*settingWindow*/)
{
    if (!qApp)
        return false;

    static QPair<QByteArray, QVariant> kvs[] = {
        {"Net/ThemeName", "deepin"},
        {"Qt/ActiveColor", QColor(Qt::red)},
        {"Net/IconThemeName", "bloom"},
        {"DTK/WindowRadius", 18}
    };

    QString domain = object->property("_d_domain").toByteArray();
    domain.replace("/", "_");
    auto settings = new DummySettings(domain);

    for (auto pair : kvs) {
        settings->setSetting(pair.first, pair.second);
    }

     // set object.metaobject to this, destroyed with object destroyed...
    DDynamicMetaObject *mj = new DDynamicMetaObject(object, settings, false);

    if (!mj->isValid()) {
        delete mj;
        return false;
    }

    return true;
}

DPlatformNativeInterface::DPlatformNativeInterface()
    :QPlatformNativeInterface()
{

}

QFunctionPointer DPlatformNativeInterface::platformFunction(const QByteArray &function) const
{
    if (!function.compare("_d_buildNativeSettings"))
        return reinterpret_cast<QFunctionPointer>(buildNativeSettings);

    return QPlatformNativeInterface::platformFunction(function);
}
