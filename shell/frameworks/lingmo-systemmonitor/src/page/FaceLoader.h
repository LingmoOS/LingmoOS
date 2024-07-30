/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include "PageDataObject.h"
#include <QObject>
#include <QPointer>
#include <faces/SensorFaceController.h>
#include <qqmlregistration.h>
class QQuickItem;

class FaceLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PageDataObject *dataObject READ dataObject WRITE setDataObject NOTIFY dataObjectChanged)
    Q_PROPERTY(KSysGuard::SensorFaceController *controller READ controller NOTIFY controllerChanged)
    QML_ELEMENT

public:
    explicit FaceLoader(QObject *parent = nullptr);
    ~FaceLoader() override;

    PageDataObject *dataObject() const;
    void setDataObject(PageDataObject *newDataObject);
    Q_SIGNAL void dataObjectChanged();

    KSysGuard::SensorFaceController *controller() const;
    Q_SIGNAL void controllerChanged();

    void reset();

private:
    QPointer<PageDataObject> m_dataObject = nullptr;
    QPointer<KSysGuard::SensorFaceController> m_faceController = nullptr;
    QPointer<KSysGuard::SensorFaceController> m_oldController = nullptr;

    static QHash<QString, QPointer<KSysGuard::SensorFaceController>> s_faceCache;
};
