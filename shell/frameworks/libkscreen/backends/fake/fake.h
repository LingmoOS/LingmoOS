/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "abstractbackend.h"
#include "config.h"

#include <QLoggingCategory>
#include <QObject>

class Fake : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kf6.kscreen.backends.fake")

public:
    explicit Fake();
    ~Fake() override;

    void init(const QVariantMap &arguments) override;

    QString name() const override;
    QString serviceName() const override;
    KScreen::ConfigPtr config() const override;
    void setConfig(const KScreen::ConfigPtr &config) override;
    QByteArray edid(int outputId) const override;
    bool isValid() const override;

    void setConnected(int outputId, bool connected);
    void setEnabled(int outputId, bool enabled);
    void setPrimary(int outputId, bool primary);
    void setCurrentModeId(int outputId, const QString &modeId);
    void setRotation(int outputId, int rotation);
    void addOutput(int outputId, const QString &name);
    void removeOutput(int outputId);

private Q_SLOTS:
    void delayedInit();

private:
    QString mConfigFile;
    KScreen::Config::Features mSupportedFeatures = KScreen::Config::Feature::None;
    mutable KScreen::ConfigPtr mConfig;
};
Q_DECLARE_LOGGING_CATEGORY(KSCREEN_FAKE)
