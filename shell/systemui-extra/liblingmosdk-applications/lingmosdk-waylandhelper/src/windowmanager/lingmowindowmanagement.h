/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef LINGMOWINDOWMANAGEMENT_H
#define LINGMOWINDOWMANAGEMENT_H

#include <QIcon>
#include <QObject>
#include <QSize>
#include <QVector>
#include <KWayland/Client/output.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/event_queue.h>

struct lingmo_activation;
struct lingmo_activation_feedback;
struct lingmo_window;
struct lingmo_window_management;

class LingmoUIWindow;

using namespace KWayland::Client;


class LingmoUIWindowManagement : public QObject
{
    Q_OBJECT
public:
    explicit LingmoUIWindowManagement(QObject *parent = nullptr);
    ~LingmoUIWindowManagement() override;

    bool isValid() const;

    void release();

    void destroy();

    void setup(lingmo_window_management *wm);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    operator lingmo_window_management *();
    operator lingmo_window_management *() const;

    bool isShowingDesktop() const;

    void setShowingDesktop(bool show);

    void showDesktop();

    void hideDesktop();

    QList<LingmoUIWindow *> windows() const;

    LingmoUIWindow *activeWindow() const;

    QVector<quint32> stackingOrder() const;

    QVector<QByteArray> stackingOrderUuids() const;

Q_SIGNALS:

    void interfaceAboutToBeReleased();

    void interfaceAboutToBeDestroyed();

    void showingDesktopChanged(bool);

    void windowCreated(LingmoUIWindow *window);

    void activeWindowChanged();

    void removed();

    void stackingOrderChanged();

    void stackingOrderUuidsChanged();

public:
    class Private;

private:
    QScopedPointer<Private> d;
};

class LingmoUIWindow : public QObject
{
    Q_OBJECT
public:
    ~LingmoUIWindow() override;


    void release();

    void destroy();

    bool isValid() const;

    operator lingmo_window *();
    operator lingmo_window *() const;

    QString title() const;

    QString appId() const;

    quint32 virtualDesktop() const;

    bool isActive() const;

    bool isFullscreen() const;

    bool isKeepAbove() const;

    bool isKeepBelow() const;

    bool isMinimized() const;

    bool isMaximized() const;

    bool isOnAllDesktops() const;

    bool isDemandingAttention() const;

    bool isCloseable() const;

    bool isMaximizeable() const;

    bool isMinimizeable() const;

    bool isFullscreenable() const;

    bool skipTaskbar() const;

    bool skipSwitcher() const;

    QIcon icon() const;

    bool isShadeable() const;

    bool isShaded() const;

    bool isMovable() const;

    bool isResizable() const;

    bool isVirtualDesktopChangeable() const;

    quint32 pid() const;

    void requestActivate();

    void requestClose();

    void requestMove();

    void requestResize();

    void requestVirtualDesktop(quint32 desktop);

    void requestToggleKeepAbove();

    void requestDemandAttention();

    void requestToggleKeepBelow();

    void requestToggleMinimized();

    void requestToggleMaximized();

    void setStartupGeometry(Surface *surface, const QRect &geometry);

    void setMinimizedGeometry(Surface *panel, const QRect &geom);

    void unsetMinimizedGeometry(Surface *panel);

    void requestToggleShaded();

    quint32 internalId() const;

    QByteArray uuid() const;

    QPointer<LingmoUIWindow> parentWindow() const;

    QRect geometry() const;

    void requestEnterVirtualDesktop(const QString &id);

    void requestEnterNewVirtualDesktop();

    void requestLeaveVirtualDesktop(const QString &id);

    QStringList lingmoVirtualDesktops() const;

    void requestEnterActivity(const QString &id);

    void requestLeaveActivity(const QString &id);

    QStringList lingmoActivities() const;

    QString applicationMenuServiceName() const;

    QString applicationMenuObjectPath() const;

    void sendToOutput(Output *output) const;

    void setHighlight();

    void unsetHightlight();

    bool isHighlight();



Q_SIGNALS:

    void titleChanged();

    void appIdChanged();

    void virtualDesktopChanged();

    void activeChanged();

    void fullscreenChanged();

    void keepAboveChanged();

    void keepBelowChanged();

    void minimizedChanged();

    void maximizedChanged();

    void onAllDesktopsChanged();

    void demandsAttentionChanged();

    void closeableChanged();

    void minimizeableChanged();

    void maximizeableChanged();

    void fullscreenableChanged();

    void skipTaskbarChanged();

    void skipSwitcherChanged();

    void iconChanged();

    void shadeableChanged();

    void shadedChanged();

    void movableChanged();

    void resizableChanged();

    void modalityChanged();

    void acceptFocusChanged();

    void virtualDesktopChangeableChanged();

    void unmapped();

    void parentWindowChanged();

    void geometryChanged();


    void lingmoVirtualDesktopEntered(const QString &id);


    void lingmoVirtualDesktopLeft(const QString &id);


    void lingmoActivityEntered(const QString &id);


    void lingmoActivityLeft(const QString &id);

    void applicationMenuChanged();

private:
    friend class LingmoUIWindowManagement;
    explicit LingmoUIWindow(LingmoUIWindowManagement *parent, lingmo_window *activation, quint32 internalId, const char *uuid);
    class Private;
    QScopedPointer<Private> d;
};


Q_DECLARE_METATYPE(LingmoUIWindow *)



#endif // LINGMOWINDOWMANAGEMENT_H
