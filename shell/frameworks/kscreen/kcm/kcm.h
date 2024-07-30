/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QTimer>

#include <KQuickManagedConfigModule>

#include "output_model.h"

namespace KScreen
{
class ConfigOperation;
}

class ConfigHandler;
class OrientationSensor;
class OutputIdentifier;

class QSortFilterProxyModel;

class KCMKScreen : public KQuickManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(QAbstractItemModel *outputModel READ outputModel CONSTANT)
    Q_PROPERTY(bool backendReady READ backendReady NOTIFY backendReadyChanged)
    Q_PROPERTY(bool screenNormalized READ screenNormalized NOTIFY screenNormalizedChanged)
    Q_PROPERTY(bool perOutputScaling READ perOutputScaling NOTIFY perOutputScalingChanged)
    Q_PROPERTY(bool xwaylandClientsScaleSupported READ xwaylandClientsScaleSupported NOTIFY xwaylandClientsScaleSupportedChanged)
    Q_PROPERTY(bool primaryOutputSupported READ primaryOutputSupported NOTIFY primaryOutputSupportedChanged)
    Q_PROPERTY(bool outputReplicationSupported READ outputReplicationSupported NOTIFY outputReplicationSupportedChanged)
    Q_PROPERTY(bool tearingSupported READ tearingSupported NOTIFY tearingSupportedChanged)
    Q_PROPERTY(qreal globalScale READ globalScale WRITE setGlobalScale NOTIFY globalScaleChanged)
    Q_PROPERTY(bool autoRotationSupported READ autoRotationSupported NOTIFY autoRotationSupportedChanged)
    Q_PROPERTY(bool orientationSensorAvailable READ orientationSensorAvailable NOTIFY orientationSensorAvailableChanged)
    Q_PROPERTY(bool tabletModeAvailable READ tabletModeAvailable NOTIFY tabletModeAvailableChanged)
    Q_PROPERTY(bool xwaylandClientsScale READ xwaylandClientsScale WRITE setXwaylandClientsScale NOTIFY xwaylandClientsScaleChanged)
    Q_PROPERTY(bool tearingAllowed READ allowTearing WRITE setAllowTearing NOTIFY tearingAllowedChanged)
    Q_PROPERTY(bool multipleScreensAvailable READ multipleScreensAvailable NOTIFY multipleScreensAvailableChanged)

public:
    enum InvalidConfigReason {
        NoEnabledOutputs,
        ConfigHasGaps,
    };
    Q_ENUM(InvalidConfigReason)

    explicit KCMKScreen(QObject *parent, const KPluginMetaData &data);

    void load() override;
    void save() override;
    void defaults() override;

    bool isSaveNeeded() const override;

    QAbstractItemModel *outputModel() const;

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

    void setAllowTearing(bool allow);
    bool allowTearing() const;
    bool tearingSupported() const;

    bool autoRotationSupported() const;
    bool orientationSensorAvailable() const;
    bool tabletModeAvailable() const;

    bool multipleScreensAvailable() const;

    void doSave();
    Q_INVOKABLE void revertSettings();
    Q_INVOKABLE void requestReboot();

    Q_INVOKABLE void setStopUpdatesFromBackend(bool value);
    Q_INVOKABLE void updateFromBackend();

Q_SIGNALS:
    void backendReadyChanged();
    void backendError();
    void changed();
    void screenNormalizedChanged();
    void perOutputScalingChanged();
    void primaryOutputSupportedChanged();
    void outputReplicationSupportedChanged();
    void globalScaleChanged();
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
    void tearingSupportedChanged();
    void tearingAllowedChanged();
    void multipleScreensAvailableChanged();

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

    QSortFilterProxyModel *m_outputProxyModel;

    QTimer *m_loadCompressor;
};
