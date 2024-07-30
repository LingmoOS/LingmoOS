/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "FaceLoader.h"

using namespace Qt::StringLiterals;
using namespace KSysGuard;

QHash<QString, QPointer<KSysGuard::SensorFaceController>> FaceLoader::s_faceCache;

FaceLoader::FaceLoader(QObject *parent)
    : QObject(parent)
{
}

FaceLoader::~FaceLoader()
{
    if (m_dataObject) {
        m_dataObject->setFaceLoader(nullptr);
    }

    if (m_oldController) {
        m_oldController->deleteLater();
    }
}

PageDataObject *FaceLoader::dataObject() const
{
    return m_dataObject;
}

void FaceLoader::setDataObject(PageDataObject *newDataObject)
{
    if (newDataObject == m_dataObject) {
        return;
    }

    if (m_faceController) {
        m_faceController->disconnect(m_dataObject);
    }

    if (m_dataObject) {
        m_dataObject->setFaceLoader(nullptr);
    }

    m_dataObject = newDataObject;

    if (m_dataObject) {
        m_dataObject->setFaceLoader(this);

        auto faceConfig = m_dataObject->value(QStringLiteral("face")).toString();
        if (faceConfig.isEmpty()) {
            faceConfig = QStringLiteral("Face-%1").arg(reinterpret_cast<quintptr>(this));
            m_dataObject->insert(QStringLiteral("face"), faceConfig);
            Q_EMIT m_dataObject->valueChanged(QStringLiteral("face"), faceConfig);
        }

        auto cacheName = m_dataObject->fileName() + u"_"_s + faceConfig;
        if (s_faceCache.contains(cacheName)) {
            m_faceController = s_faceCache.value(cacheName);

            if (!m_faceController) {
                s_faceCache.remove(cacheName);
            }
        }

        if (!m_faceController) {
            auto configGroup = m_dataObject->config()->group(faceConfig);
            m_faceController = new SensorFaceController(configGroup, qmlEngine(this), qmlEngine(this));
            m_faceController->setShouldSync(false);
            s_faceCache.insert(cacheName, m_faceController);
        }

        connect(m_faceController, &SensorFaceController::faceIdChanged, m_dataObject, &PageDataObject::markDirty);
        connect(m_faceController, &SensorFaceController::titleChanged, m_dataObject, &PageDataObject::markDirty);
        connect(m_faceController, &SensorFaceController::totalSensorsChanged, m_dataObject, &PageDataObject::markDirty);
        connect(m_faceController, &SensorFaceController::highPrioritySensorIdsChanged, m_dataObject, &PageDataObject::markDirty);
        connect(m_faceController, &SensorFaceController::lowPrioritySensorIdsChanged, m_dataObject, &PageDataObject::markDirty);
        connect(m_faceController, &SensorFaceController::sensorColorsChanged, m_dataObject, &PageDataObject::markDirty);
        connect(m_faceController, &SensorFaceController::updateRateLimitChanged, m_dataObject, &PageDataObject::markDirty);

        Q_EMIT controllerChanged();
    }

    if (m_oldController) {
        delete m_oldController;
        m_oldController = nullptr;
    }

    Q_EMIT dataObjectChanged();
}

SensorFaceController *FaceLoader::controller() const
{
    return m_faceController;
}

void FaceLoader::reset()
{
    auto faceConfig = m_dataObject->value(QStringLiteral("face")).toString();
    auto cacheName = m_dataObject->fileName() + u"_"_s + faceConfig;
    if (s_faceCache.contains(cacheName)) {
        s_faceCache.remove(cacheName);
    }

    // Deleting the controller here, even when using deleteLater will trigger a
    // crash in the QML runtime because it still has references to the object
    // that do not get cleared. So instead, only delete it once we have a new
    // face controller.
    m_oldController = m_faceController;
    m_faceController = nullptr;
}

#include "moc_FaceLoader.cpp"
