/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../decorationdefines.h"
#include <kdecoration2/private/kdecoration2_private_export.h>

#include <QIcon>
#include <QString>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDecoration2 API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace KDecoration2
{
class Decoration;
class DecoratedClient;

class KDECORATIONS_PRIVATE_EXPORT DecoratedClientPrivate
{
public:
    virtual ~DecoratedClientPrivate();
    virtual bool isActive() const = 0;
    virtual QString caption() const = 0;
    virtual bool isOnAllDesktops() const = 0;
    virtual bool isShaded() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool isMaximized() const = 0;
    virtual bool isMaximizedHorizontally() const = 0;
    virtual bool isMaximizedVertically() const = 0;
    virtual bool isKeepAbove() const = 0;
    virtual bool isKeepBelow() const = 0;

    virtual bool isCloseable() const = 0;
    virtual bool isMaximizeable() const = 0;
    virtual bool isMinimizeable() const = 0;
    virtual bool providesContextHelp() const = 0;
    virtual bool isModal() const = 0;
    virtual bool isShadeable() const = 0;
    virtual bool isMoveable() const = 0;
    virtual bool isResizeable() const = 0;

    virtual WId windowId() const = 0;
    virtual WId decorationId() const = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual QSize size() const = 0;
    virtual QPalette palette() const = 0;
    virtual Qt::Edges adjacentScreenEdges() const = 0;

    virtual void requestShowToolTip(const QString &text) = 0;
    virtual void requestHideToolTip() = 0;
    virtual void requestClose() = 0;
    virtual void requestToggleMaximization(Qt::MouseButtons buttons) = 0;
    virtual void requestMinimize() = 0;
    virtual void requestContextHelp() = 0;
    virtual void requestToggleOnAllDesktops() = 0;
    virtual void requestToggleShade() = 0;
    virtual void requestToggleKeepAbove() = 0;
    virtual void requestToggleKeepBelow() = 0;
    virtual void requestShowWindowMenu(const QRect &rect) = 0;

    Decoration *decoration();
    Decoration *decoration() const;

    virtual QColor color(ColorGroup group, ColorRole role) const;
    virtual QString windowClass() const = 0;

protected:
    explicit DecoratedClientPrivate(DecoratedClient *client, Decoration *decoration);
    DecoratedClient *client();

private:
    class Private;
    const std::unique_ptr<Private> d;
};

class KDECORATIONS_PRIVATE_EXPORT ApplicationMenuEnabledDecoratedClientPrivate : public DecoratedClientPrivate
{
public:
    ~ApplicationMenuEnabledDecoratedClientPrivate() override;

    virtual bool hasApplicationMenu() const = 0;
    virtual bool isApplicationMenuActive() const = 0;

    virtual void showApplicationMenu(int actionId) = 0;
    virtual void requestShowApplicationMenu(const QRect &rect, int actionId) = 0;

protected:
    explicit ApplicationMenuEnabledDecoratedClientPrivate(DecoratedClient *client, Decoration *decoration);
};

} // namespace
