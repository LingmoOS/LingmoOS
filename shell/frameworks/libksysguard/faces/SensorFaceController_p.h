/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KConfigGroup>
#include <KConfigLoader>
#include <KConfigPropertyMap>
#include <KLocalizedContext>
#include <KPackage/PackageLoader>
#include <QJsonArray>
#include <QQuickItem>
#include <QStandardItemModel>

#include <functional>

#include "sensorfaces_export.h"

class QQmlEngine;

namespace KSysGuard
{
class SensorFaceController;
class SensorFace;
class SensorQuery;

class FacesModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum AdditionalRoles {
        PluginIdRole = Qt::UserRole + 1,
    };
    Q_ENUM(AdditionalRoles)

    FacesModel(QObject *parent = nullptr);
    ~FacesModel() override = default;

    Q_INVOKABLE void reload();
    Q_INVOKABLE QString pluginId(int row);

    QHash<int, QByteArray> roleNames() const override;
};

class PresetsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum AdditionalRoles { PluginIdRole = Qt::UserRole + 1, ConfigRole, WritableRole };
    Q_ENUM(AdditionalRoles)

    PresetsModel(QObject *parent = nullptr);
    ~PresetsModel() override = default;

    Q_INVOKABLE void reload();

    QHash<int, QByteArray> roleNames() const override;
};

struct SensorResolver {
    SensorResolver(SensorFaceController *_controller, const QJsonArray &_expected);
    void execute();

    SensorFaceController *controller = nullptr;

    QJsonArray expected;
    QJsonArray found;
    QJsonArray missing;

    QList<SensorQuery *> queries;

    std::function<void(SensorResolver *)> callback;
};

// This is exported so we can use it in autotests
class SENSORFACES_EXPORT SensorFaceControllerPrivate
{
public:
    SensorFaceControllerPrivate();

    QJsonArray readSensors(const KConfigGroup &config, const QString &entryName);
    QJsonArray readAndUpdateSensors(KConfigGroup &config, const QString &entryName);
    QString replaceDiskId(const QString &entryname) const;
    QString replacePartitionId(const QString &entryname) const;
    void resolveSensors(const QJsonArray &partialEntries, std::function<void(SensorResolver *)>);
    SensorFace *createGui(const QString &qmlPath);
    QQuickItem *createConfigUi(const QString &file, const QVariantMap &initialProperties);

    SensorFaceController *q;
    QString title;
    QQmlEngine *engine;
    QQmlEngine *configEngine;

    KConfigGroup faceProperties;
    KConfigPropertyMap *faceConfiguration = nullptr;
    KConfigLoader *faceConfigLoader = nullptr;

    bool configNeedsSave = false;
    KPackage::Package facePackage;
    QString faceId;
    KLocalizedContext *contextObj = nullptr;
    KConfigGroup configGroup;
    KConfigGroup appearanceGroup;
    KConfigGroup sensorsGroup;
    KConfigGroup colorsGroup;
    KConfigGroup labelsGroup;
    QPointer<SensorFace> fullRepresentation;
    QPointer<SensorFace> compactRepresentation;
    QPointer<QQuickItem> faceConfigUi;
    QPointer<QQuickItem> appearanceConfigUi;
    QPointer<QQuickItem> sensorsConfigUi;

    QJsonArray totalSensors;
    QJsonArray highPrioritySensorIds;
    QJsonArray lowPrioritySensorIds;
    QJsonArray missingSensors;

    QTimer *syncTimer;
    bool shouldSync = true;
    FacesModel *availableFacesModel = nullptr;
    PresetsModel *availablePresetsModel = nullptr;

    static QList<QPair<QRegularExpression, QString>> sensorIdReplacements;
    static QRegularExpression oldDiskSensor;
    static QRegularExpression oldPartitionSensor;
};

}
