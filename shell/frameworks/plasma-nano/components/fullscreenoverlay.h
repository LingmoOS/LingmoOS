/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef FULLSCREENOVERLAY_H
#define FULLSCREENOVERLAY_H

#include <QQuickWindow>

namespace KWayland
{
namespace Client
{
class PlasmaShell;
class PlasmaShellSurface;
class Surface;
}
}

class FullScreenOverlay : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(bool acceptsFocus MEMBER m_acceptsFocus NOTIFY acceptsFocusChanged)

public:
    explicit FullScreenOverlay(QQuickWindow *parent = nullptr);
    ~FullScreenOverlay() override;

Q_SIGNALS:
    void activeChanged(); // clazy:exclude=overridden-signal
    void acceptsFocusChanged();

protected:
    bool event(QEvent *event) override;

private:
    void initWayland();
    KWayland::Client::PlasmaShellSurface *m_plasmaShellSurface = nullptr;
    KWayland::Client::Surface *m_surface = nullptr;
    KWayland::Client::PlasmaShell *m_plasmaShellInterface = nullptr;
    bool m_acceptsFocus = true;
};

#endif
