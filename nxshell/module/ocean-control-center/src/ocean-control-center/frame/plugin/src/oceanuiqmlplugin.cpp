// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "oceanuiqmlplugin.h"

#include "oceanuiapp.h"
#include "oceanuimodel.h"
#include "oceanuiobject.h"
#include "oceanuirepeater.h"
#include "oceanuiquickdbusinterface.h"

#include <qqml.h>

namespace oceanuiV25 {

void OceanUIQmlPlugin::registerTypes(const char *uri)
{
    // @uri org.lingmo.oceanui
    qmlRegisterModule(uri, 1, 0);
    qmlRegisterType<oceanuiV25::OceanUIObject>(uri, 1, 0, "OceanUIObject");
    qmlRegisterType<oceanuiV25::OceanUIRepeater>(uri, 1, 0, "OceanUIRepeater");
    qmlRegisterType<oceanuiV25::OceanUIModel>(uri, 1, 0, "OceanUIModel");
    qmlRegisterType<oceanuiV25::OceanUIQuickDBusInterface>(uri, 1, 0, "OceanUIDBusInterface");

    qmlRegisterSingletonInstance(uri, 1, 0, "OceanUIApp", oceanuiV25::OceanUIApp::instance());
}

void OceanUIQmlPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);
}

} // namespace oceanuiV25
