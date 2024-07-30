/*
 *  SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QCommandLineParser>
#include <QObject>

#include "output.h"
#include "types.h"

namespace KScreen
{
class ConfigOperation;
class Dpms;

class Doctor : public QObject
{
    Q_OBJECT

public:
    explicit Doctor(QObject *parent = nullptr);
    ~Doctor() override;

    void setOptionList(const QStringList &positionalArgs);
    void start(QCommandLineParser *m_parser);
    void configReceived(KScreen::ConfigOperation *op);
    OutputPtr findOutput(const QString &query);
    KScreen::ModePtr findMode(OutputPtr output, const QString &query);

    void showBackends() const;
    void showOutputs() const;
    void showJson() const;

    void setEnabled(OutputPtr output, bool enable = true);
    void setPosition(OutputPtr output, const QPoint &pos);
    bool setMode(OutputPtr output, const QString &query);
    void setScale(OutputPtr output, qreal scale);
    void setRotation(OutputPtr output, KScreen::Output::Rotation rot);
    void setOverscan(OutputPtr output, uint32_t overscan);
    void setVrrPolicy(OutputPtr output, KScreen::Output::VrrPolicy policy);
    void setRgbRange(OutputPtr output, KScreen::Output::RgbRange rgbRange);
    void setPrimary(OutputPtr output);
    void setPriority(OutputPtr output, uint32_t priority);
    void setHdrEnabled(OutputPtr output, bool enable);
    void setSdrBrightness(OutputPtr output, uint32_t brightness);
    void setWcgEnabled(OutputPtr output, bool enable);

Q_SIGNALS:
    void outputsChanged();
    void started();
    void configChanged();

private:
    // static QString modeString(KWayland::Server::OutputDeviceInterface* outputdevice, int mid);
    void applyConfig();
    void parseOutputArgs();
    KScreen::ConfigPtr m_config;
    QCommandLineParser *m_parser;
    bool m_changed;
    QStringList m_outputArgs;
    Dpms *m_dpmsClient;
};

} // namespace
