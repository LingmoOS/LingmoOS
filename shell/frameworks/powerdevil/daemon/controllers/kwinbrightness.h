/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "displaybrightness.h"

#include <KScreen/Config>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>
#include <QTimer>
#include <unordered_map>

class KWinDisplayDetector;

class KWinDisplayBrightness : public DisplayBrightness
{
public:
    explicit KWinDisplayBrightness(const KScreen::OutputPtr &output, KWinDisplayDetector *detector);

    QString id() const override;
    QString label() const override;
    int knownSafeMinBrightness() const override;
    int maxBrightness() const override;
    int brightness() const override;
    void setBrightness(int brightness) override;

    void applyPendingBrightness();
    void setConfigOperationDone();

private:
    void handleBrightnessChanged();

    const KScreen::OutputPtr m_output;
    KWinDisplayDetector *const m_detector;
    double m_desiredBrightness;
    bool m_inhibitChangeSignal = false;
};

class KWinDisplayDetector : public DisplayBrightnessDetector
{
    Q_OBJECT
public:
    explicit KWinDisplayDetector(QObject *parent = nullptr);
    ~KWinDisplayDetector();

    void detect() override;
    QList<DisplayBrightness *> displays() const override;

    void scheduleSetConfig();

private:
    void checkOutputs();
    void setConfig();
    void setConfigDone();

    KScreen::ConfigPtr m_config;
    KScreen::SetConfigOperation *m_setConfigOp = nullptr;
    std::unordered_map<KScreen::Output *, std::unique_ptr<KWinDisplayBrightness>> m_displays;
    QList<DisplayBrightness *> m_displayList;
    QTimer m_setConfigTimer;
    bool m_setConfigOutOfDate = false;
};
