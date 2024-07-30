/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINDOWSHADOW_H
#define WINDOWSHADOW_H

#include <private/kwindowshadow_p.h>

#include <memory>

class Shadow;
class ShmBuffer;
class Shm;

class WindowShadowTile final : public QObject, public KWindowShadowTilePrivate
{
public:
    WindowShadowTile();
    ~WindowShadowTile();

    bool create() override;
    void destroy() override;

    static WindowShadowTile *get(const KWindowShadowTile *tile);

    std::unique_ptr<ShmBuffer> buffer;
};

class WindowShadow final : public QObject, public KWindowShadowPrivate
{
public:
    WindowShadow();
    ~WindowShadow() override;
    bool create() override;
    void destroy() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool internalCreate();
    void internalDestroy();

    std::unique_ptr<Shadow> shadow;
};

#endif // WINDOWSHADOW_H
