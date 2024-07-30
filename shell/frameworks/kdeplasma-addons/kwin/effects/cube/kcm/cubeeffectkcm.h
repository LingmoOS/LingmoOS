/*
    SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KCModule>

#include "ui_cubeeffectkcm.h"

class KConfigLoader;

class CubeEffectConfig : public KCModule
{
    Q_OBJECT

public:
    CubeEffectConfig(QObject *parent, const KPluginMetaData &data);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    void updateUiFromConfig();
    void updateUiFromDefaultConfig();
    void updateUnmanagedState();

    int uiBackground() const;
    void setUiBackground(int mode);
    void updateBackgroundFromUi();

    qreal distanceFactor() const;
    void setDistanceFactor(qreal factor);
    void updateDistanceFactorFromUi();

    ::Ui::CubeEffectConfig ui;
    KConfigLoader *m_configLoader = nullptr;
};
