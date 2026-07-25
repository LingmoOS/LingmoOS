/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */
#pragma once

#include <QtQml/QQmlExtensionPlugin>
#include <qobject.h>

class LingmoUI : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    void registerTypes(const char *uri) override;
};
