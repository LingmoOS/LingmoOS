/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

#include <KConfigGroup>

#include "sensorfaces_export.h"

class KConfigPropertyMap;
class QQmlEngine;
class KDesktopFile;
class KConfigLoader;

namespace KSysGuard
{
class SensorFace;
class SensorFaceControllerPrivate;

/**
 * The SensorFaceController links sensor faces and applications in which these faces are shown. It
 * abstracts the configuration and properties of faces.
 *
 * For faces it offers information about which sensors should be displayed and hints set by the
 * application about how the information should be displayed. It can be accessed by faces  using
 * the `SensorFace::controller` property.
 *
 * Applications can use this class to instantiate faces from a given config and for querying the
 * capabilities of faces.
 *
 * @since 5.19
 */
class SENSORFACES_EXPORT SensorFaceController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("It's not possible to create objects of type SensorFaceController")
    /**
     * A title for the face.
     * @see showTitle
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    /** Whether the title should be displayed or if it should be hidden instead
     * @see title
     */
    Q_PROPERTY(bool showTitle READ showTitle WRITE setShowTitle NOTIFY showTitleChanged)
    /**
     * The id of the current face. For example `org.kde.ksysguard.textonly`
     */
    Q_PROPERTY(QString faceId READ faceId WRITE setFaceId NOTIFY faceIdChanged)
    /**
     * Sensors that are typically used to display a total in some way or form. For example in the pie
     * char face they are not drawn as part of the chart but appear in the centre of it.
     */
    Q_PROPERTY(QJsonArray totalSensors READ totalSensors WRITE setTotalSensors NOTIFY totalSensorsChanged)
    /**
     * Sensors that should always be shown in the face. This is the main list of sensors that are of
     * the most interest.
     */
    Q_PROPERTY(QJsonArray highPrioritySensorIds READ highPrioritySensorIds WRITE setHighPrioritySensorIds NOTIFY highPrioritySensorIdsChanged)
    /**
     * Maps sensorIds to colors that can be used when a color for something relating to a
     * specific sensor is needed.
     */
    Q_PROPERTY(QVariantMap sensorColors READ sensorColors WRITE setSensorColors NOTIFY sensorColorsChanged)

    /**
     * Maps sensorIds to user configurable labels than should be displayed instead of the name of the sensor.
     */
    Q_PROPERTY(QVariantMap sensorLabels READ sensorLabels WRITE setSensorLabels NOTIFY sensorLabelsChanged)

    /**
     * Secondary list of sensors. These sensors do not necessarily appear in main part of the face.
     * For example in most faces they are just added to the legend.
     */
    Q_PROPERTY(QJsonArray lowPrioritySensorIds READ lowPrioritySensorIds WRITE setLowPrioritySensorIds NOTIFY lowPrioritySensorIdsChanged)

    /**
     * The name of the current face
     */
    Q_PROPERTY(QString name READ name NOTIFY faceIdChanged)
    /**
     * The icon of the current face
     */
    Q_PROPERTY(QString icon READ icon NOTIFY faceIdChanged)
    /**
     * Whether the current face supports sensor colors
     */
    Q_PROPERTY(bool supportsSensorsColors READ supportsSensorsColors NOTIFY faceIdChanged)
    /**
     * Whether the current face can display total sensors
     */
    Q_PROPERTY(bool supportsTotalSensors READ supportsTotalSensors NOTIFY faceIdChanged)
    /**
     * Whether the current face can display low priority sensors
     */
    Q_PROPERTY(bool supportsLowPrioritySensors READ supportsLowPrioritySensors NOTIFY faceIdChanged)
    /**
     * The amount of total sensors the current face supports
     */
    Q_PROPERTY(int maxTotalSensors READ maxTotalSensors NOTIFY faceIdChanged)
    /**
     * A map of config options and values that are specific to the current face as defined by the
     * `main.xml` of the face.
     * @see faceConfigUi
     */
    Q_PROPERTY(KConfigPropertyMap *faceConfiguration READ faceConfiguration NOTIFY faceConfigurationChanged)

    /**
     * The full representation of the current face. Typically includes additional elements like
     * a legend or title
     */
    Q_PROPERTY(QQuickItem *fullRepresentation READ fullRepresentation NOTIFY faceIdChanged)
    /**
     * The compact representation of the current face. Typically only includes the main visualization
     * of the data without legend, title, etc.
     */
    Q_PROPERTY(QQuickItem *compactRepresentation READ compactRepresentation NOTIFY faceIdChanged)
    /**
     * A user interface that is suited for configuring the face specific options.
     * Emits `configurationChanged` if a config value changed. To apply the changes call `saveConfig`
     * on it.
     * @see faceConfiguration
     */
    Q_PROPERTY(QQuickItem *faceConfigUi READ faceConfigUi NOTIFY faceIdChanged)
    /**
     * A user interface for configuring the general appearance of a face like the title and the used
     * face.
     * Emits `configurationChanged` if a config value changed. To apply the changes call `saveConfig`
     * on it.
     */
    Q_PROPERTY(QQuickItem *appearanceConfigUi READ appearanceConfigUi NOTIFY faceIdChanged)
    /**
     * A user interface for configuring which sensors are displayed in a face
     * Emits `configurationChanged` if a config value changed. To apply the changes call `saveConfig`
     * on it.
     */
    Q_PROPERTY(QQuickItem *sensorsConfigUi READ sensorsConfigUi NOTIFY faceIdChanged)

    /**
     * A list of all available faces. The name is available as the display role and the id as `pluginId`
     */
    Q_PROPERTY(QAbstractItemModel *availableFacesModel READ availableFacesModel CONSTANT)
    /**
     * A list of available face presets. The name is available as the display role, the id as `pluginId`.
     * The properties of the preset can be accessed via the `config` role.
     */
    Q_PROPERTY(QAbstractItemModel *availablePresetsModel READ availablePresetsModel CONSTANT)
    /**
     * The minimum time that needs to elapse, in milliseconds, between updates of the face.
     */
    Q_PROPERTY(int updateRateLimit READ updateRateLimit WRITE setUpdateRateLimit NOTIFY updateRateLimitChanged)
    /**
     * Contains the paths of missing sensors, if there are any.
     */
    Q_PROPERTY(QJsonArray missingSensors READ missingSensors NOTIFY missingSensorsChanged)

public:
    /**
     * Creates a new SensorFaceController.
     * This is only useful for applications that want display SensorFaces.
     *
     * SensorFaces  can access the controller that created them using their `SensorFace::controller`
     * property.
     * @param config The controller uses this config group to read and save the face configuration
     * @param engine This engine will be used for creating the Qml components
     */
    SensorFaceController(KConfigGroup &config, QQmlEngine *engine, QQmlEngine *configEngine);
    ~SensorFaceController() override;

    /**
     * Retrieve the KConfigGroup this controller is using to store configuration.
     *
     * This is primarily intended to allow adding child groups to the face
     * configuration.
     */
    KConfigGroup configGroup() const;

    void setFaceId(const QString &face);
    QString faceId() const;

    QQuickItem *fullRepresentation();
    QQuickItem *compactRepresentation();
    QQuickItem *faceConfigUi();
    QQuickItem *appearanceConfigUi();
    QQuickItem *sensorsConfigUi();

    KConfigPropertyMap *faceConfiguration() const;

    QString title() const;
    void setTitle(const QString &title);

    bool showTitle() const;
    void setShowTitle(bool show);

    QJsonArray totalSensors() const;
    void setTotalSensors(const QJsonArray &sensor);

    QJsonArray highPrioritySensorIds() const;
    void setHighPrioritySensorIds(const QJsonArray &ids);

    QJsonArray sensors() const;

    QJsonArray lowPrioritySensorIds() const;
    void setLowPrioritySensorIds(const QJsonArray &ids);

    QJsonArray missingSensors() const;

    QVariantMap sensorColors() const;
    void setSensorColors(const QVariantMap &colors);

    QVariantMap sensorLabels() const;
    void setSensorLabels(const QVariantMap &labels);

    int updateRateLimit() const;
    void setUpdateRateLimit(int limit);

    // from face config, immutable by the user
    QString name() const;
    const QString icon() const;

    bool supportsSensorsColors() const;
    bool supportsTotalSensors() const;
    bool supportsLowPrioritySensors() const;

    int maxTotalSensors() const;

    QAbstractItemModel *availableFacesModel();
    QAbstractItemModel *availablePresetsModel();

    /**
     * Reload the configuration.
     */
    Q_INVOKABLE void reloadConfig();
    /**
     * Loads a specific preset
     * @see availablePresetsModel
     */
    Q_INVOKABLE void loadPreset(const QString &preset);
    /**
     * Save the current configuration as a preset
     */
    Q_INVOKABLE void savePreset();
    /**
     * Uninstall a specific preset
     */
    Q_INVOKABLE void uninstallPreset(const QString &pluginId);

    /**
     * Whether the controller should sync configuration changes
     * @see setShouldSync
     */
    bool shouldSync() const;
    /**
     * Specifies if the controller should automatically sync configuration changes.
     * @param sync If `true` applied config changes are written to the KConfigGroup that was
     *   specified in the @ref SensorFaceController::SensorFaceController "constructor". If `false`
     *   config changes are applied after calling `saveConfig` on a configuration ui  but not written
     *   to the KConfigGroup.
     */
    void setShouldSync(bool sync);

    /**
     * Reload only the face configuration.
     *
     * This does not touch sensors, colors or anything else, only the config
     * loaded from the face package is reloaded.
     */
    Q_INVOKABLE void reloadFaceConfiguration();

    /**
     * Replace one sensor with another.
     *
     * This replaces a configured sensor with a new one. This replacement happens
     * inside the configuration, bypassing thing like the sensor properties which
     * are populated with resolved sensor ids rather than the configured entries.
     *
     * You should call @ref reloadConfig once you have made all replacements.
     */
    Q_INVOKABLE void replaceSensors(const QString &from, const QString &to);

Q_SIGNALS:
    void faceIdChanged();
    void titleChanged();
    void showTitleChanged();
    void totalSensorsChanged();
    void highPrioritySensorIdsChanged();
    void lowPrioritySensorIdsChanged();
    void sensorsChanged();
    void sensorColorsChanged();
    void sensorLabelsChanged();
    void updateRateLimitChanged();
    void faceConfigurationChanged();
    void missingSensorsChanged();

private:
    const std::unique_ptr<SensorFaceControllerPrivate> d;
};
}
