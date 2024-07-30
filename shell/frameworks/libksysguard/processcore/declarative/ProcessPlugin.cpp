/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ProcessPlugin.h"

#include <QQmlEngine>

#include "application_data_model.h"
#include "process_attribute_model.h"
#include "process_controller.h"
#include "process_data_model.h"

#include "ProcessEnums.h"

using namespace KSysGuard;

void ProcessPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.ksysguard.process"));

    qRegisterMetaType<KSysGuard::ProcessController::Signal>();
    qRegisterMetaType<KSysGuard::ProcessController::Result>();
    qRegisterMetaType<KSysGuardProcess::ProcessStatus>();
    qRegisterMetaType<KSysGuardProcess::IoPriorityClass>();
    qRegisterMetaType<KSysGuardProcess::Scheduler>();

    qmlRegisterType<ProcessController>(uri, 1, 0, "ProcessController");
    qmlRegisterUncreatableMetaObject(KSysGuardProcess::staticMetaObject, uri, 1, 0, "Process", QStringLiteral("Contains process enums"));
    qmlRegisterType<ProcessDataModel>(uri, 1, 0, "ProcessDataModel");
    qmlRegisterUncreatableType<ProcessAttributeModel>(uri, 1, 0, "ProcessAttributeModel", QStringLiteral("Available through ProcessDataModel"));
    qmlRegisterType<ApplicationDataModel>(uri, 1, 0, "ApplicationDataModel");
}
