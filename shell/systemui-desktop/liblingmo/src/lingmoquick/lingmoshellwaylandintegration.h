/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINGMOSHELLWAYLANDINTEGRATION_P_H
#define LINGMOSHELLWAYLANDINTEGRATION_P_H

#include <QObject>
#include <QPoint>
#include <memory>

#include "qwayland-lingmo-shell.h"

#include <lingmoquick/lingmoquick_export.h>

class QWindow;

class LingmoShellSurface;
class LingmoShellWaylandIntegrationPrivate;

/**
 * @brief The LingmoWaylandShellIntegration class exposes Lingmo specific
 * specific wayland extensions for
 *
 * The class abstracts the wayland protocol tasks, automatically sending
 * cached metadata when the underlying platform surfaces are created.
 */

class LINGMOQUICK_EXPORT LingmoShellWaylandIntegration : public QObject
{
    Q_OBJECT
public:
    /**
     * Returns the relevant LingmoWaylandShellIntegration instance for this window
     * creating one if needed.
     *
     * A valid instance will always returned, it will no-op on unsupported platforms
     */
    static LingmoShellWaylandIntegration *get(QWindow *window);
    ~LingmoShellWaylandIntegration();

    void setPosition(const QPoint &position);
    void setPanelBehavior(QtWayland::org_kde_lingmo_surface::panel_behavior panelBehavior);
    void setRole(QtWayland::org_kde_lingmo_surface::role role);
    void setTakesFocus(bool takesFocus);
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    LingmoShellWaylandIntegration(QWindow *window);

    Q_PRIVATE_SLOT(d, void platformSurfaceCreated(QWindow *window))
    Q_PRIVATE_SLOT(d, void surfaceCreated())
    Q_PRIVATE_SLOT(d, void surfaceDestroyed())

    const std::unique_ptr<LingmoShellWaylandIntegrationPrivate> d;
};

#endif // LINGMOSHELLWAYLANDINTEGRATION_P_H
