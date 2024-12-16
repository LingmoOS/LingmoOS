// Copyright (C) 2023 JiDe Zhang <zhangjide@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "wglobal.h"

#include <WCursor>
#include <WLayerSurface>
#include <WOutput>
#include <WSeat>
#include <WSurfaceItem>
#include <wquickoutputlayout.h>
#include <wtoplevelsurface.h>
#include <wxdgdecorationmanager.h>

#include <QList>
#include <QQmlApplicationEngine>

Q_MOC_INCLUDE(<woutputrenderwindow.h>)
Q_MOC_INCLUDE(<wqmlcreator.h>)
Q_MOC_INCLUDE(<qwcompositor.h>)

Q_DECLARE_OPAQUE_POINTER(QWindow *)

WAYLIB_SERVER_BEGIN_NAMESPACE
class WQuickCursor;
class WQmlCreator;
class WOutputRenderWindow;
class WInputMethodHelper;
WAYLIB_SERVER_END_NAMESPACE

QW_BEGIN_NAMESPACE
class QWCompositor;
QW_END_NAMESPACE

struct wlr_output_event_request_state;
QW_USE_NAMESPACE
WAYLIB_SERVER_USE_NAMESPACE

struct OutputInfo;

struct Margins
{
    Q_GADGET
    Q_PROPERTY(uint32_t left MEMBER left)
    Q_PROPERTY(uint32_t top MEMBER top)
    Q_PROPERTY(uint32_t right MEMBER right)
    Q_PROPERTY(uint32_t bottom MEMBER bottom)
public:
    uint32_t left = 0, top = 0, right = 0, bottom = 0;
};

class Helper : public WSeatEventFilter
{
    Q_OBJECT
    Q_PROPERTY(WToplevelSurface* activatedSurface READ activatedSurface WRITE setActivateSurface NOTIFY activatedSurfaceChanged FINAL)
    Q_PROPERTY(WSurfaceItem* resizingItem READ resizingItem NOTIFY resizingItemChanged FINAL)
    Q_PROPERTY(WSurfaceItem *movingItem READ movingItem WRITE setMovingItem NOTIFY movingItemChanged FINAL)
    Q_PROPERTY(QString waylandSocket READ waylandSocket WRITE setWaylandSocket NOTIFY socketFileChanged FINAL)
    Q_PROPERTY(QString xwaylandSocket READ xwaylandSocket WRITE setXWaylandSocket NOTIFY socketFileChanged FINAL)
    Q_PROPERTY(QString currentUser READ currentUser WRITE setCurrentUser NOTIFY currentUserChanged FINAL)
    Q_PROPERTY(bool switcherOn MEMBER m_switcherOn NOTIFY switcherOnChanged FINAL)
    Q_PROPERTY(bool switcherEnabled MEMBER m_switcherEnabled FINAL)
    Q_PROPERTY(WQuickOutputLayout* outputLayout READ outputLayout CONSTANT)
    Q_PROPERTY(WSeat* seat READ seat CONSTANT)
    Q_PROPERTY(WCursor* cursor READ cursor CONSTANT)
    Q_PROPERTY(QW_NAMESPACE::QWCompositor* compositor READ compositor NOTIFY compositorChanged FINAL)
    Q_PROPERTY(WXdgDecorationManager *xdgDecorationManager READ xdgDecorationManager NOTIFY xdgDecorationManagerChanged)
    Q_PROPERTY(WQmlCreator* outputCreator READ outputCreator CONSTANT)
    Q_PROPERTY(WQmlCreator* xdgShellCreator READ xdgShellCreator CONSTANT)
    Q_PROPERTY(WQmlCreator* xwaylandCreator READ xwaylandCreator CONSTANT)
    Q_PROPERTY(WQmlCreator* layerShellCreator READ layerShellCreator CONSTANT)
    Q_PROPERTY(WQmlCreator* inputPopupCreator READ inputPopupCreator CONSTANT)

    // TODO: move to workspace
    Q_PROPERTY(int currentWorkspaceId READ currentWorkspaceId WRITE setCurrentWorkspaceId NOTIFY currentWorkspaceIdChanged FINAL)

public:
    explicit Helper(WServer *server);

    enum Switcher {
        Next,
        Previous,
    };
    Q_ENUM(Switcher)

    void initProtocols(WOutputRenderWindow *window);
    WQuickOutputLayout *outputLayout() const;
    WSeat *seat() const;
    QWCompositor *compositor() const;

    WCursor *cursor() const;

    WXdgDecorationManager *xdgDecorationManager() const;

    WQmlCreator *outputCreator() const;
    WQmlCreator *xdgShellCreator() const;
    WQmlCreator *xwaylandCreator() const;
    WQmlCreator *layerShellCreator() const;
    WQmlCreator *inputPopupCreator() const;

    void setCurrentUser(const QString &currentUser);

    inline QString currentUser() const { return m_currentUser; }

    inline int currentWorkspaceId() const { return m_currentWorkspaceId; }

    void setCurrentWorkspaceId(int currentWorkspaceId);

    QString waylandSocket() const;
    QString xwaylandSocket() const;

    Q_INVOKABLE QString clientName(Waylib::Server::WSurface *surface) const;

    bool addAction(const QString &user, QAction *action);
    void removeAction(const QString &user, QAction *action);

    Q_INVOKABLE void closeSurface(Waylib::Server::WSurface *surface);

    void stopMoveResize();

    WToplevelSurface *activatedSurface() const;
    WSurfaceItem *resizingItem() const;
    WSurfaceItem *movingItem() const;

    Q_INVOKABLE bool registerExclusiveZone(WLayerSurface *layerSurface);
    Q_INVOKABLE bool unregisterExclusiveZone(WLayerSurface *layerSurface);
    Q_INVOKABLE Margins getExclusiveMargins(WLayerSurface *layerSurface);
    Q_INVOKABLE quint32 getTopExclusiveMargin(WToplevelSurface *layerSurface);
    Q_INVOKABLE quint32 getBottomExclusiveMargin(WToplevelSurface *layerSurface);
    Q_INVOKABLE quint32 getLeftExclusiveMargin(WToplevelSurface *layerSurface);
    Q_INVOKABLE quint32 getRightExclusiveMargin(WToplevelSurface *layerSurface);

    // Output
    Q_INVOKABLE void onSurfaceEnterOutput(WToplevelSurface *surface,
                                          WSurfaceItem *surfaceItem,
                                          WOutput *output);
    Q_INVOKABLE void onSurfaceLeaveOutput(WToplevelSurface *surface,
                                          WSurfaceItem *surfaceItem,
                                          WOutput *output);
    Q_INVOKABLE Margins getOutputExclusiveMargins(WOutput *output);
    std::pair<WOutput *, OutputInfo *> getFirstOutputOfSurface(WToplevelSurface *surface);

    Q_INVOKABLE bool selectSurfaceToActivate(WToplevelSurface *surface) const;

public Q_SLOTS:
    void startMove(WToplevelSurface *surface, WSurfaceItem *shell, WSeat *seat, int serial);
    void startResize(
        WToplevelSurface *surface, WSurfaceItem *shell, WSeat *seat, Qt::Edges edge, int serial);
    void cancelMoveResize(WSurfaceItem *shell);
    WSurface *getFocusSurfaceFrom(QObject *object);

    void allowNonDrmOutputAutoChangeMode(WOutput *output);
    void enableOutput(WOutput *output);

Q_SIGNALS:
    void activatedSurfaceChanged();
    void resizingItemChanged();
    void movingItemChanged();
    void backToNormal();
    void reboot();
    void greeterVisibleChanged();
    void topExclusiveMarginChanged();
    void bottomExclusiveMarginChanged();
    void leftExclusiveMarginChanged();
    void rightExclusiveMarginChanged();
    void socketFileChanged();
    void switcherChanged(Switcher action);
    void switcherOnChanged(bool on);
    void compositorChanged();
    void xdgDecorationManagerChanged();
    void currentWorkspaceIdChanged();
    void currentUserChanged(const QString &user);


    // 键盘信号
    void resizePane(int size, int direction);
    void choosePane(int id);
    void swapPane();
    void removePane(int flag);
    void switchLayout();
    void createWs();
    void destoryWs();
    void switchNextWs();
    // void switchBackWs();
    void moveWs(int wsId);

protected:
    bool beforeDisposeEvent(WSeat *seat, QWindow *watched, QInputEvent *event) override;
    bool afterHandleEvent(WSeat *seat,
                          WSurface *watched,
                          QObject *surfaceItem,
                          QObject *,
                          QInputEvent *event) override;
    bool unacceptedEvent(WSeat *seat, QWindow *watched, QInputEvent *event) override;

    void setActivateSurface(WToplevelSurface *newActivate);
    void setResizingItem(WSurfaceItem *newResizingItem);
    void setMovingItem(WSurfaceItem *newMovingItem);
    void onOutputRequeseState(wlr_output_event_request_state *newState);
    OutputInfo *getOutputInfo(WOutput *output);

    WServer *m_server = nullptr;
    QWRenderer *m_renderer = nullptr;
    QWAllocator *m_allocator = nullptr;
    QWCompositor *m_compositor = nullptr;
    WQuickOutputLayout *m_outputLayout = nullptr;
    WQuickCursor *m_cursor = nullptr;
    QPointer<WSeat> m_seat;

    WXdgDecorationManager *m_xdgDecorationManager;

    WQmlCreator *m_outputCreator = nullptr;
    WQmlCreator *m_xdgShellCreator = nullptr;
    WQmlCreator *m_xwaylandCreator = nullptr;
    WQmlCreator *m_layerShellCreator = nullptr;
    WQmlCreator *m_inputPopupCreator = nullptr;

    QPointer<WSocket> m_socket;

    QPointer<WToplevelSurface> m_activateSurface;

    QList<std::pair<WOutput *, OutputInfo *>> m_outputExclusiveZoneInfo;

    // for move resize
    struct
    {
        QPointer<WToplevelSurface> surface;
        QPointer<WSurfaceItem> surfaceItem;
        WSeat *seat = nullptr;
        QPointF surfacePosOfStartMoveResize;
        QSizeF surfaceSizeOfStartMoveResize;
        Qt::Edges resizeEdgets;
        WSurfaceItem *resizingItem = nullptr;
        WSurfaceItem *movingItem = nullptr;
    } moveReiszeState;

private:
    void setWaylandSocket(const QString &socketFile);
    void setXWaylandSocket(const QString &socketFile);

private:
    QString m_waylandSocket;
    QString m_xwaylandSocket;
    QString m_currentUser;
    std::map<QString, std::vector<QAction *>> m_actions;
    bool m_switcherOn = false;
    bool m_switcherEnabled = true;

    int m_currentWorkspaceId{ 0 };
};

struct OutputInfo
{
    QList<WToplevelSurface *> surfaceList;
    QList<WSurfaceItem *> surfaceItemList;

    // for Exclusive Zone
    Margins exclusiveMargins = { 0, 0, 0, 0 };
    QList<std::tuple<WLayerSurface *, uint32_t, WLayerSurface::AnchorType>> registeredSurfaceList;
};
