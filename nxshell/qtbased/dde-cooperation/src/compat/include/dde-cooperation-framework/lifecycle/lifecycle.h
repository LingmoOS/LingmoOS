// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LIFECYCLE_H
#define LIFECYCLE_H

#include <dde-cooperation-framework/lifecycle/pluginmetaobject.h>
#include <dde-cooperation-framework/lifecycle/plugin.h>
#include <dde-cooperation-framework/lifecycle/plugincreator.h>
#include <dde-cooperation-framework/dde_cooperation_framework_global.h>

#include <QString>
#include <QObject>

DPF_BEGIN_NAMESPACE

namespace LifeCycle {
DPF_EXPORT void initialize(const QStringList &IIDs, const QStringList &paths);
DPF_EXPORT void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames);
DPF_EXPORT void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames,
                const QStringList &lazyNames);

DPF_EXPORT bool isAllPluginsInitialized();
DPF_EXPORT bool isAllPluginsStarted();
DPF_EXPORT QStringList pluginIIDs();
DPF_EXPORT QStringList pluginPaths();
DPF_EXPORT QStringList blackList();
DPF_EXPORT QStringList lazyLoadList();
DPF_EXPORT PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                      const QString version = "");

DPF_EXPORT bool readPlugins();
DPF_EXPORT bool loadPlugins();
DPF_EXPORT void shutdownPlugins();

DPF_EXPORT bool loadPlugin(PluginMetaObjectPointer &pointer);
DPF_EXPORT void shutdownPlugin(PluginMetaObjectPointer &pointer);
}   // namepsace LifeCycle

DPF_END_NAMESPACE

#endif   // LIFECYCLE_H
