/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <KQuickAddons/ManagedConfigModule>

#include "output_model.h"

namespace KScreen
{
class ConfigOperation;
}

class ConfigHandler;
class OrientationSensor;
class OutputIdentifier;

class KCMKScreen : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(OutputModel *outputModel READ outputModel NOTIFY outputModelChanged)
    Q_PROPERTY(bool backendReady READ backendReady NOTIFY backendReadyChanged)
    Q_PROPERTY(bool screenNormalized READ screenNormalized NOTIFY screenNormalizedChanged)
    Q_PROPERTY(bool perOutputScaling READ perOutputScaling NOTIFY perOutputScalingChanged)
    Q_PROPERTY(bool xwaylandClientsScaleSupported READ xwaylandClientsScaleSupported NOTIFY xwaylandClientsScaleSupportedChanged)
    Q_PROPERTY(bool primaryOutputSupported READ primaryOutputSupported NOTIFY primaryOutputSupportedChanged)
    Q_PROPERTY(bool outputReplicationSupported READ outputReplicationSupported NOTIFY outputReplicationSupportedChanged)
    Q_PROPERTY(qreal globalScale READ globalScale WRITE setGlobalScale NOTIFY globalScaleChanged)
    Q_PROPERTY(int outputRetention READ outputRetention WRITE setOutputRetention NOTIFY outputRetentionChanged)
    Q_PROPERTY(bool autoRotationSupported READ autoRotationSupported NOTIFY autoRotationSupportedChanged)
    Q_PROPERTY(bool orientationSensorAvailable READ orientationSensorAvailable NOTIFY orientationSensorAvailableChanged)
    Q_PROPERTY(bool tabletModeAvailable READ tabletModeAvailable NOTIFY tabletModeAvailableChanged)
    Q_PROPERTY(bool xwaylandClientsScale READ xwaylandClientsScale WRITE setXwaylandClientsScale NOTIFY xwaylandClientsScaleChanged)

public:
    enum InvalidConfigReason {
        NoEnabledOutputs,
        ConfigHasGaps,
    };
    Q_ENUM(InvalidConfigReason)

    explicit KCMKScreen(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~KCMKScreen() override = default;

    void load() override;
    void save() override;
    void defaults() override;

    bool isSaveNeeded() const override;

    OutputModel *outputModel() const;

    Q_INVOKABLE void identifyOutputs();

    bool backendReady() const;

    Q_INVOKABLE QSize normalizeScreen() const;
    bool screenNormalized() const;

    bool perOutputScaling() const;
    bool primaryOutputSupported() const;
    bool outputReplicationSupported() const;

    qreal globalScale() const;
    void setGlobalScale(qreal scale);

    bool xwaylandClientsScale() const;
    void setXwaylandClientsScale(bool scale);
    bool xwaylandClientsScaleSupported() const;

    int outputRetention() const;
    void setOutputRetention(int retention);

    bool autoRotationSupported() const;
    bool orientationSensorAvailable() const;
    bool tabletModeAvailable() const;

    void doSave();
    Q_INVOKABLE void revertSettings();
    Q_INVOKABLE void requestReboot();

    Q_INVOKABLE void setStopUpdatesFromBackend(bool value);
    Q_INVOKABLE void updateFromBackend();

Q_SIGNALS:
    void backendReadyChanged();
    void backendError();
    void outputModelChanged();
    void changed();
    void screenNormalizedChanged();
    void perOutputScalingChanged();
    void primaryOutputSupportedChanged();
    void outputReplicationSupportedChanged();
    void globalScaleChanged();
    void outputRetentionChanged();
    void autoRotationSupportedChanged();
    void orientationSensorAvailableChanged();
    void tabletModeAvailableChanged();
    void invalidConfig(InvalidConfigReason reason);
    void errorOnSave();
    void globalScaleWritten();
    void outputConnect(bool connected);
    void settingsReverted();
    void showRevertWarning();
    void xwaylandClientsScaleChanged();
    void xwaylandClientsScaleSupportedChanged();

private:
    void setBackendReady(bool error);
    void setScreenNormalized(bool normalized);

    void exportGlobalScale();

    void configReady(KScreen::ConfigOperation *op);
    void continueNeedsSaveCheck(bool needs);
    void checkConfig();

    std::unique_ptr<ConfigHandler> m_configHandler;
    OrientationSensor *m_orientationSensor;
    bool m_backendReady = false;
    bool m_screenNormalized = true;
    bool m_settingsReverted = false;
    bool m_stopUpdatesFromBackend = false;
    bool m_configNeedsSave = false;

    QTimer *m_loadCompressor;
};
