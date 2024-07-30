/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../src/private/decoratedclientprivate.h"

#include <QObject>

class MockClient : public QObject, public KDecoration2::ApplicationMenuEnabledDecoratedClientPrivate
{
    Q_OBJECT
public:
    explicit MockClient(KDecoration2::DecoratedClient *client, KDecoration2::Decoration *decoration);

    Qt::Edges adjacentScreenEdges() const override;
    QString caption() const override;
    WId decorationId() const override;
    int height() const override;
    QIcon icon() const override;
    bool isActive() const override;
    bool isCloseable() const override;
    bool isKeepAbove() const override;
    bool isKeepBelow() const override;
    bool isMaximizeable() const override;
    bool isMaximized() const override;
    bool isMaximizedHorizontally() const override;
    bool isMaximizedVertically() const override;
    bool isMinimizeable() const override;
    bool isModal() const override;
    bool isMoveable() const override;
    bool isOnAllDesktops() const override;
    bool isResizeable() const override;
    bool isShadeable() const override;
    bool isShaded() const override;
    QPalette palette() const override;
    bool hasApplicationMenu() const override;
    bool isApplicationMenuActive() const override;
    bool providesContextHelp() const override;
    void requestClose() override;
    void requestContextHelp() override;
    void requestToggleMaximization(Qt::MouseButtons buttons) override;
    void requestMinimize() override;
    void requestShowWindowMenu(const QRect &rect) override;
    void requestShowApplicationMenu(const QRect &rect, int actionId) override;
    void requestToggleKeepAbove() override;
    void requestToggleKeepBelow() override;
    void requestToggleOnAllDesktops() override;
    void requestToggleShade() override;
    void requestShowToolTip(const QString &text) override;
    void requestHideToolTip() override;
    QSize size() const override;
    int width() const override;
    WId windowId() const override;
    QString windowClass() const override;

    void showApplicationMenu(int actionId) override;

    void setCloseable(bool set);
    void setMinimizable(bool set);
    void setProvidesContextHelp(bool set);
    void setShadeable(bool set);
    void setMaximizable(bool set);

    void setWidth(int w);
    void setHeight(int h);

Q_SIGNALS:
    void closeRequested();
    void minimizeRequested();
    void quickHelpRequested();
    void menuRequested();
    void applicationMenuRequested();

private:
    bool m_closeable = false;
    bool m_minimizable = false;
    bool m_contextHelp = false;
    bool m_keepAbove = false;
    bool m_keepBelow = false;
    bool m_shadeable = false;
    bool m_shaded = false;
    bool m_maximizable = false;
    bool m_maximizedVertically = false;
    bool m_maximizedHorizontally = false;
    bool m_onAllDesktops = false;
    int m_width = 0;
    int m_height = 0;
};
