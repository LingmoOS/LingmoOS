/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "abstractbackend.h"

#include <QLoggingCategory>

namespace KScreen
{
class WaylandConfig;

class WaylandBackend : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kf6.kscreen.backends.kwayland")

public:
    explicit WaylandBackend();
    ~WaylandBackend() override = default;

    QString name() const override;
    QString serviceName() const override;
    KScreen::ConfigPtr config() const override;
    void setConfig(const KScreen::ConfigPtr &config) override;
    bool isValid() const override;
    QByteArray edid(int outputId) const override;

private:
    WaylandConfig *m_internalConfig;
};

}

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_WAYLAND)
