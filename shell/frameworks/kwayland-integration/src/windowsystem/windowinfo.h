/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINDOWINFO_H
#define WINDOWINFO_H

#include <QtGlobal>

#include <kwindowinfo.h>
#include <private/kwindowinfo_p.h>

namespace KWayland
{
namespace Client
{
class Surface;
class PlasmaShellSurface;
}
}

class WindowInfo : public KWindowInfoPrivate
{
public:
    WindowInfo(WId window, NET::Properties properties, NET::Properties2 properties2);
    ~WindowInfo() override;

    bool valid(bool withdrawn_is_valid) const override;
    NET::States state() const override;
    bool isMinimized() const override;
    NET::MappingState mappingState() const override;
    NETExtendedStrut extendedStrut() const override;
    NET::WindowType windowType(NET::WindowTypes supported_types) const override;
    QString visibleName() const override;
    QString visibleNameWithState() const override;
    QString name() const override;
    QString visibleIconName() const override;
    QString visibleIconNameWithState() const override;
    QString iconName() const override;
    bool onAllDesktops() const override;
    bool isOnDesktop(int desktop) const override;
    int desktop() const override;
    QStringList activities() const override;
    QRect geometry() const override;
    QRect frameGeometry() const override;
    WId transientFor() const override;
    WId groupLeader() const override;
    QByteArray windowClassClass() const override;
    QByteArray windowClassName() const override;
    QByteArray windowRole() const override;
    QByteArray clientMachine() const override;
    bool actionSupported(NET::Action action) const override;

private:
    bool m_valid;
    NET::Properties m_properties;
    NET::Properties2 m_properties2;
    KWayland::Client::Surface *m_surface;
    KWayland::Client::PlasmaShellSurface *m_plasmaShellSurface;
};

#endif
