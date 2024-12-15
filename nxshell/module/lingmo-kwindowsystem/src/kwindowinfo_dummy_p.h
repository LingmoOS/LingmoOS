/*
 *   Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KWINDOWINFO_DUMMY_P_H
#define KWINDOWINFO_DUMMY_P_H
#include "kwindowinfo_p.h"

/**
 * Dummy implementation for KWindowInfoPrivate. This is used as a fallback if there is no
 * implementation for the currently used windowing system platform.
 */
class KWindowInfoPrivateDummy : public KWindowInfoPrivate
{
public:
    KWindowInfoPrivateDummy(WId window, NET::Properties properties, NET::Properties2 properties2);
    ~KWindowInfoPrivateDummy();

    bool valid(bool withdrawn_is_valid) const Q_DECL_OVERRIDE;
    NET::States state() const Q_DECL_OVERRIDE;
    bool isMinimized() const Q_DECL_OVERRIDE;
    NET::MappingState mappingState() const Q_DECL_OVERRIDE;
    NETExtendedStrut extendedStrut() const Q_DECL_OVERRIDE;
    NET::WindowType windowType(NET::WindowTypes supported_types) const Q_DECL_OVERRIDE;
    QString visibleName() const Q_DECL_OVERRIDE;
    QString visibleNameWithState() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString visibleIconName() const Q_DECL_OVERRIDE;
    QString visibleIconNameWithState() const Q_DECL_OVERRIDE;
    QString iconName() const Q_DECL_OVERRIDE;
    bool onAllDesktops() const Q_DECL_OVERRIDE;
    bool isOnDesktop(int desktop) const Q_DECL_OVERRIDE;
    int desktop() const Q_DECL_OVERRIDE;
    QStringList activities() const Q_DECL_OVERRIDE;
    QRect geometry() const Q_DECL_OVERRIDE;
    QRect frameGeometry() const Q_DECL_OVERRIDE;
    WId transientFor() const Q_DECL_OVERRIDE;
    WId groupLeader() const Q_DECL_OVERRIDE;
    QByteArray windowClassClass() const Q_DECL_OVERRIDE;
    QByteArray windowClassName() const Q_DECL_OVERRIDE;
    QByteArray windowRole() const Q_DECL_OVERRIDE;
    QByteArray clientMachine() const Q_DECL_OVERRIDE;
    bool actionSupported(NET::Action action) const Q_DECL_OVERRIDE;
};

#endif
