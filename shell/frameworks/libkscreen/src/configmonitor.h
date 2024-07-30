/*
 *  SPDX-FileCopyrightText: 2012-2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QObject>
#include <QPointer>

#include "config.h"
#include "kscreen_export.h"

namespace KScreen
{
class AbstractBackend;
class BackendManager;

class KSCREEN_EXPORT ConfigMonitor : public QObject
{
    Q_OBJECT

public:
    static ConfigMonitor *instance();

    void addConfig(const KScreen::ConfigPtr &config);
    void removeConfig(const KScreen::ConfigPtr &config);

Q_SIGNALS:
    void configurationChanged();

private:
    explicit ConfigMonitor();
    ~ConfigMonitor() override;

    Q_DISABLE_COPY(ConfigMonitor)

    friend BackendManager;
    void connectInProcessBackend(KScreen::AbstractBackend *backend);

    class Private;
    Private *const d;
};

} /* namespace KScreen */
