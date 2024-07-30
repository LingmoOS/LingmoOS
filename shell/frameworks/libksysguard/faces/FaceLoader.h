/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QJsonArray>
#include <QObject>
#include <QQmlParserStatus>

#include "SensorFaceController.h"
#include "sensorfaces_export.h"

namespace KSysGuard
{

/**
 * A helper class to make it easier to load faces when used inside a face.
 *
 * This is primarily intended to support use cases where there is a sensor face
 * that wants to load a different face as a child, for example the Grid face
 * uses this to creates individual faces for each grid cell.
 *
 * This will create a new SensorFaceController that makes use of the
 * parentController's config group for reading settings. The child group name
 * is set by the groupName property. By default, the new controller is read
 * only and does not write to the config group.
 */
class SENSORFACES_EXPORT FaceLoader : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)
    /**
     * The parent SensorFaceController that will be used for configuration storage.
     */
    Q_PROPERTY(KSysGuard::SensorFaceController *parentController READ parentController WRITE setParentController NOTIFY parentControllerChanged)
    /**
     * The name of the config group to read configuration from.
     */
    Q_PROPERTY(QString groupName READ groupName WRITE setGroupName NOTIFY groupNameChanged)
    /**
     * The sensors to use for this face.
     *
     * This will set `highPrioritySensorIds` on the internal SensorFaceController.
     */
    Q_PROPERTY(QJsonArray sensors READ sensors WRITE setSensors NOTIFY sensorsChanged)
    /**
     * The face to use.
     *
     * This sets the faceId of the internal SensorFaceController.
     */
    Q_PROPERTY(QString faceId READ faceId WRITE setFaceId NOTIFY faceIdChanged)
    /**
     * A map of sensor colors to be used by the face.
     *
     * This forwards to the internal SensorFaceController.
     */
    Q_PROPERTY(QVariantMap colors READ colors WRITE setColors NOTIFY colorsChanged)
    /**
     * A map of sensor labels to be used by the face.
     *
     * This forwards to the internal SensorFaceController.
     */
    Q_PROPERTY(QVariantMap labels READ labels WRITE setLabels NOTIFY labelsChanged)
    /**
     * Whether to allow modifying the face configuration.
     *
     * If false (the default), any changes to configuration will be ignored. If
     * true, changes will be written and stored in the config group.
     *
     * \note If multiple FaceLoaders share the same configuration, the face will
     * need to be recreated after configuration has changed, as there is
     * currently no way to properly reload the configuration.
     */
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly NOTIFY readOnlyChanged)
    /**
     * The face controller that provides the loaded face.
     */
    Q_PROPERTY(KSysGuard::SensorFaceController *controller READ controller NOTIFY controllerChanged)

public:
    FaceLoader(QObject *parent = nullptr);
    ~FaceLoader() override;

    SensorFaceController *parentController() const;
    void setParentController(SensorFaceController *newParentController);
    Q_SIGNAL void parentControllerChanged();

    QString groupName() const;
    void setGroupName(const QString &newGroupName);
    Q_SIGNAL void groupNameChanged();

    QJsonArray sensors() const;
    void setSensors(const QJsonArray &newSensors);
    Q_SIGNAL void sensorsChanged();

    QString faceId() const;
    void setFaceId(const QString &newFaceId);
    Q_SIGNAL void faceIdChanged();

    QVariantMap colors() const;
    void setColors(const QVariantMap &newColors);
    Q_SIGNAL void colorsChanged();

    QVariantMap labels() const;
    void setLabels(const QVariantMap &newLabels);
    Q_SIGNAL void labelsChanged();

    bool readOnly() const;
    void setReadOnly(bool newReadOnly);
    Q_SIGNAL void readOnlyChanged();

    SensorFaceController *controller() const;
    Q_SIGNAL void controllerChanged();

    Q_INVOKABLE void reload();

    void classBegin() override;
    void componentComplete() override;

private:
    class Private;
    const std::unique_ptr<Private> d;
};

}
