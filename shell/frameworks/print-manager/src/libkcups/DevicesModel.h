/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEVICES_MODEL_H
#define DEVICES_MODEL_H

#include <QDBusMessage>
#include <QRegularExpression>
#include <QStandardItemModel>

#include <qqmlregistration.h>

#include <KCupsRequest.h>
#include <kcupslib_export.h>

typedef QMap<QString, QString> MapSS;
typedef QMap<QString, MapSS> MapSMapSS;

class KCupsRequest;

class KCUPSLIB_EXPORT DevicesModel : public QStandardItemModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Role {
        DeviceClass = Qt::UserRole + 2,
        DeviceId,
        DeviceInfo,
        DeviceMakeAndModel,
        DeviceUri,
        DeviceUris,
        DeviceLocation,
        DeviceDescription,
        DeviceCategory
    };
    Q_ENUM(Role)

    enum Kind { Other, Local, Networked, OtherNetworked };
    Q_ENUM(Kind)

    explicit DevicesModel(QObject *parent = nullptr);
    virtual QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void loaded();
    void parentAdded(const QModelIndex &index);
    void errorMessage(const QString &message);

public Q_SLOTS:
    void update();
    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const QStringList &grouped_uris = QStringList());

    void insertDevice(const QString &device_class,
                      const QString &device_id,
                      const QString &device_info,
                      const QString &device_make_and_model,
                      const QString &device_uri,
                      const QString &device_location,
                      const KCupsPrinters &grouped_printers);

    QString uriDevice(const QString &uri) const;

private Q_SLOTS:
    QStandardItem *createItem(const QString &device_class,
                              const QString &device_id,
                              const QString &device_info,
                              const QString &device_make_and_model,
                              const QString &device_uri,
                              const QString &device_location,
                              bool grouped);

    void gotDevice(const QString &device_class,
                   const QString &device_id,
                   const QString &device_info,
                   const QString &device_make_and_model,
                   const QString &device_uri,
                   const QString &device_location);

    void finished();

private:
    QStandardItem *findCreateCategory(const QString &category, Kind kind);
    QString deviceDescription(const QString &uri) const;

    KCupsRequest *m_request = nullptr;
    MapSMapSS m_mappedDevices;
    QRegularExpression m_rx;
    QStringList m_blacklistedURIs;
    QHash<int, QByteArray> m_roles;
};

Q_DECLARE_METATYPE(MapSS)
Q_DECLARE_METATYPE(MapSMapSS)

#endif
