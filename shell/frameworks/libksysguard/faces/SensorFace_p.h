/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQuickItem>

#include "sensorfaces_export.h"

namespace KSysGuard
{
class SensorFaceController;

/**
 * Base for sensor faces. Use this as root item when creating custom faces.
 * @code
 * import org.kde.ksysguard.faces 1.0
 * SensorFace {
 *    contentItem: Label {
 *        text: "Custom visualization"
 *    }
 * }
 * @endcode
 * @since 5.19
 */
class SENSORFACES_EXPORT SensorFace : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(AbstractSensorFace)
    /**
     * The controller that instantiated this face and is responsible for it. Contains information
     * about the sensors that should be displayed and the configuration of the face
     */
    Q_PROPERTY(KSysGuard::SensorFaceController *controller READ controller CONSTANT)
    /**
     * The current active formFactor. Describes how the face should display itself
     */
    Q_PROPERTY(KSysGuard::SensorFace::FormFactor formFactor READ formFactor WRITE setFormFactor NOTIFY formFactorChanged)
    /**
     * Main item of the face. Assign your custom visualization to it
     */
    Q_PROPERTY(QQuickItem *contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)
#ifdef K_DOXYGEN // Document proprties from SensorFace.qml
    /**
     * Primary actions that this face provides
     */
    Q_PROPERTY(org::kde::kirigami::Action primaryActions)
    /**
     * Secondary actions this face provides
     */
    Q_PROPERTY(org::kde::kirigami::Action secondaryActions)
    /**
     * A color source that can be used  when using charts from KQuickCharts
     * @see SensorFaceController::sensorColors
     */
    Q_PROPERTY(MapProxySource colorSource)
#endif
public:
    /**
     * Describes how the face should display itself.
     */
    enum FormFactor {
        Planar, ///< Default, the face can expand horizontally and vertically
        Vertical, ///< The face is constrained horizontally, but can expand vertically
        Horizontal, ///< The face is constrained vertically, but can expand horizontally
        Constrained ///< The face is fully constrained, for example when in the edit mode of plasma-systemmonitor
    };
    Q_ENUM(FormFactor)

    SensorFace(QQuickItem *parent = nullptr);
    ~SensorFace() override;

    SensorFaceController *controller() const;
    // Not writable from QML
    void setController(SensorFaceController *controller);

    SensorFace::FormFactor formFactor() const;
    void setFormFactor(SensorFace::FormFactor formFactor);

    QQuickItem *contentItem() const;
    void setContentItem(QQuickItem *item);

Q_SIGNALS:
    void formFactorChanged();
    void contentItemChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    class Private;
    const std::unique_ptr<Private> d;
};
}
