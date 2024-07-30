/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULELOADER_H
#define KCMODULELOADER_H

#include <KPluginMetaData>
#include <kcmodule.h>

#include <memory>

class QWidget;
class QQmlEngine;

/**
 * @short Loads a KCModule
 * In case the provided metadata points to a KQuickConfigModule, it is wrapped in a KCModule
 */
namespace KCModuleLoader
{
/**
 * Loads a @ref KCModule. If loading fails a KCM which displays an error message is returned.
 *
 * @param metaData KPluginMetaData for loading the plugin
 * @param engine QQmlEngine that will be used for KQuickConfigModule classes.
 * If none is set, a internal engine will be created and reused for further modules.
 * In case your app already has an engine, you should pass it in explicitly
 *
 * @return a pointer to the loaded @ref KCModule
 */
KCMUTILS_EXPORT KCModule *
loadModule(const KPluginMetaData &metaData, QWidget *parent = nullptr, const QVariantList &args = {}, const std::shared_ptr<QQmlEngine> &engine = {});
}

#endif // KCMODULELOADER_H
