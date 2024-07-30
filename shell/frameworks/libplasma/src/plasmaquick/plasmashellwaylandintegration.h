/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLASMASHELLWAYLANDINTEGRATION_P_H
#define PLASMASHELLWAYLANDINTEGRATION_P_H

#include <QObject>
#include <QPoint>
#include <memory>

#include "qwayland-plasma-shell.h"

#include <plasmaquick/plasmaquick_export.h>

class QWindow;

class PlasmaShellSurface;
class PlasmaShellWaylandIntegrationPrivate;

/**
 * @brief The PlasmaWaylandShellIntegration class exposes Plasma specific
 * specific wayland extensions for
 *
 * The class abstracts the wayland protocol tasks, automatically sending
 * cached metadata when the underlying platform surfaces are created.
 */

class PLASMAQUICK_EXPORT PlasmaShellWaylandIntegration : public QObject
{
    Q_OBJECT
public:
    /**
     * Returns the relevant PlasmaWaylandShellIntegration instance for this window
     * creating one if needed.
     *
     * A valid instance will always returned, it will no-op on unsupported platforms
     */
    static PlasmaShellWaylandIntegration *get(QWindow *window);
    ~PlasmaShellWaylandIntegration();

    void setPosition(const QPoint &position);
    void setPanelBehavior(QtWayland::org_kde_plasma_surface::panel_behavior panelBehavior);
    void setRole(QtWayland::org_kde_plasma_surface::role role);
    void setTakesFocus(bool takesFocus);
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    PlasmaShellWaylandIntegration(QWindow *window);

    Q_PRIVATE_SLOT(d, void platformSurfaceCreated(QWindow *window))
    Q_PRIVATE_SLOT(d, void surfaceCreated())
    Q_PRIVATE_SLOT(d, void surfaceDestroyed())

    const std::unique_ptr<PlasmaShellWaylandIntegrationPrivate> d;
};

#endif // PLASMASHELLWAYLANDINTEGRATION_P_H
