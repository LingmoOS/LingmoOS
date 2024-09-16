// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDWINDOW_H
#define QWAYLANDWINDOW_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtGui/QIcon>
#include <QtCore/QVariant>
#include <QtCore/QLoggingCategory>

#include <qpa/qplatformwindow.h>

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

struct wl_egl_window;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

Q_DECLARE_LOGGING_CATEGORY(lcWaylandBackingstore)

class QWaylandDisplay;
class QWaylandBuffer;
class QWaylandShellSurface;
class QWaylandSubSurface;
class QWaylandAbstractDecoration;
class QWaylandInputDevice;
class QWaylandScreen;
class QWaylandShmBackingStore;
class QWaylandPointerEvent;

class Q_WAYLAND_CLIENT_EXPORT QWaylandWindow : public QObject, public QPlatformWindow, public QtWayland::wl_surface
{
    Q_OBJECT
public:
    enum WindowType {
        Shm,
        Egl
    };

    QWaylandWindow(QWindow *window);
    ~QWaylandWindow() override;

    virtual WindowType windowType() const = 0;
    virtual void ensureSize();
    WId winId() const override;
    void setVisible(bool visible) override;
    void setParent(const QPlatformWindow *parent) override;

    void setWindowTitle(const QString &title) override;

    inline QIcon windowIcon() const;
    void setWindowIcon(const QIcon &icon) override;

    void setGeometry(const QRect &rect) override;
    void resizeFromApplyConfigure(const QSize &sizeWithMargins, const QPoint &offset = {0, 0});

    void applyConfigureWhenPossible(); //rename to possible?

    using QtWayland::wl_surface::attach;
    void attach(QWaylandBuffer *buffer, int x, int y);
    void attachOffset(QWaylandBuffer *buffer);
    QPoint attachOffset() const;

    using QtWayland::wl_surface::damage;
    void damage(const QRect &rect);

    void commit(QWaylandBuffer *buffer, const QRegion &damage);

    void waitForFrameSync();

    QMargins frameMargins() const override;
    QSize surfaceSize() const;
    QRect windowGeometry() const;

    static QWaylandWindow *fromWlSurface(::wl_surface *surface);

    QWaylandDisplay *display() const { return mDisplay; }
    QWaylandShellSurface *shellSurface() const;
    QWaylandSubSurface *subSurfaceWindow() const;
    QWaylandScreen *waylandScreen() const;

    void handleContentOrientationChange(Qt::ScreenOrientation orientation) override;
    void setOrientationMask(Qt::ScreenOrientations mask);

    void setWindowState(Qt::WindowStates states) override;
    void setWindowFlags(Qt::WindowFlags flags) override;
    void handleWindowStatesChanged(Qt::WindowStates states);

    void raise() override;
    void lower() override;

    void setMask(const QRegion &region) override;

    int scale() const;
    qreal devicePixelRatio() const override;

    void requestActivateWindow() override;
    bool isExposed() const override;
    void unfocus();

    QWaylandAbstractDecoration *decoration() const;

    void handleMouse(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e);
    void handleMouseLeave(QWaylandInputDevice *inputDevice);

    bool touchDragDecoration(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global,
                             Qt::TouchPointState state, Qt::KeyboardModifiers mods);

    bool createDecoration();

#if QT_CONFIG(cursor)
    void setMouseCursor(QWaylandInputDevice *device, const QCursor &cursor);
    void restoreMouseCursor(QWaylandInputDevice *device);
#endif

    QWaylandWindow *transientParent() const;

    QMutex *resizeMutex() { return &mResizeLock; }
    void doApplyConfigure();
    void setCanResize(bool canResize);

    bool setMouseGrabEnabled(bool grab) override;
    static QWaylandWindow *mouseGrab() { return mMouseGrab; }

    void sendProperty(const QString &name, const QVariant &value);
    void setProperty(const QString &name, const QVariant &value);

    QVariantMap properties() const;
    QVariant property(const QString &name);
    QVariant property(const QString &name, const QVariant &defaultValue);

    void setBackingStore(QWaylandShmBackingStore *backingStore) { mBackingStore = backingStore; }
    QWaylandShmBackingStore *backingStore() const { return mBackingStore; }

    bool setKeyboardGrabEnabled(bool) override { return false; }
    void propagateSizeHints() override { }
    void addAttachOffset(const QPoint point);

    bool startSystemMove(const QPoint &pos) override;

    void requestUpdate() override;

public slots:
    void applyConfigure();

protected:
    void surface_enter(struct ::wl_output *output) override;
    void surface_leave(struct ::wl_output *output) override;

    QVector<QWaylandScreen *> mScreens; //As seen by wl_surface.enter/leave events. Chronological order.
    QWaylandDisplay *mDisplay = nullptr;
    QWaylandShellSurface *mShellSurface = nullptr;
    QWaylandSubSurface *mSubSurfaceWindow = nullptr;
    QVector<QWaylandSubSurface *> mChildren;

    QWaylandAbstractDecoration *mWindowDecoration = nullptr;
    bool mMouseEventsInContentArea = false;
    Qt::MouseButtons mMousePressedInContentArea = Qt::NoButton;

    WId mWindowId;
    bool mWaitingForFrameSync = false;
    struct ::wl_callback *mFrameCallback = nullptr;
    QWaitCondition mFrameSyncWait;

    QMutex mResizeLock;
    bool mWaitingToApplyConfigure = false;
    bool mCanResize = true;
    bool mResizeDirty = false;
    bool mResizeAfterSwap;
    QVariantMap m_properties;

    bool mSentInitialResize = false;
    QPoint mOffset;
    int mScale = 1;

    QIcon mWindowIcon;

    Qt::WindowFlags mFlags;
    QRegion mMask;
    Qt::WindowStates mLastReportedWindowStates = Qt::WindowNoState;

    QWaylandShmBackingStore *mBackingStore = nullptr;

private slots:
    void handleScreenRemoved(QScreen *qScreen);

private:
    void setGeometry_helper(const QRect &rect);
    void initWindow();
    void initializeWlSurface();
    bool shouldCreateShellSurface() const;
    bool shouldCreateSubSurface() const;
    void reset(bool sendDestroyEvent = true);
    void sendExposeEvent(const QRect &rect);
    static void closePopups(QWaylandWindow *parent);
    QWaylandScreen *calculateScreenFromSurfaceEvents() const;

    void handleMouseEventWithDecoration(QWaylandInputDevice *inputDevice, const QWaylandPointerEvent &e);
    void handleScreenChanged();

    bool mUpdateRequested = false;

    static const wl_callback_listener callbackListener;
    static void frameCallback(void *data, struct wl_callback *wl_callback, uint32_t time);

    static QMutex mFrameSyncMutex;
    static QWaylandWindow *mMouseGrab;

    friend class QWaylandSubSurface;
};

inline QIcon QWaylandWindow::windowIcon() const
{
    return mWindowIcon;
}

inline QPoint QWaylandWindow::attachOffset() const
{
    return mOffset;
}

}

QT_END_NAMESPACE

#endif // QWAYLANDWINDOW_H
