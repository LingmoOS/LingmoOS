/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "abstractbackend.h"

#include <QLoggingCategory>

namespace KScreen
{
class QScreenConfig;

class QScreenBackend : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kf6.kscreen.backends.qscreen")

public:
    explicit QScreenBackend();
    ~QScreenBackend() override;

    QString name() const override;
    QString serviceName() const override;
    KScreen::ConfigPtr config() const override;
    void setConfig(const KScreen::ConfigPtr &config) override;
    bool isValid() const override;

private:
    bool m_isValid;
    static KScreen::QScreenConfig *s_internalConfig;
};
} // namespace

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_QSCREEN)
