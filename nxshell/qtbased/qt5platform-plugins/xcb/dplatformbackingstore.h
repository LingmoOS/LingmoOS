// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DXCBBACKINGSTORE_H
#define DXCBBACKINGSTORE_H

#define private public
#include <qpa/qplatformbackingstore.h>
#undef private
#include <qpa/qplatformwindow.h>

#include <QBasicTimer>

#include "global.h"
#include "utility.h"

QT_BEGIN_NAMESPACE
class QXcbBackingStore;
class QWidgetWindow;
class QOpenGLTextureBlitter;
QT_END_NAMESPACE

struct xcb_property_notify_event_t;

DPP_BEGIN_NAMESPACE

class DXcbShmGraphicsBuffer;
class WindowEventListener;
class DPlatformWindowHook;

class DPlatformBackingStore : public QPlatformBackingStore
{
public:
    DPlatformBackingStore(QWindow *window, QXcbBackingStore *proxy);
    ~DPlatformBackingStore();

    QPaintDevice *paintDevice() Q_DECL_OVERRIDE;

    // 'window' can be a child window, in which case 'region' is in child window coordinates and
    // offset is the (child) window's offset in relation to the window surface.
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) Q_DECL_OVERRIDE;
#ifndef QT_NO_OPENGL
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context) Q_DECL_OVERRIDE;
#else
    void composeAndFlushHelper(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context,
                         bool translucentBackground);
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                         QPlatformTextureList *textures, QOpenGLContext *context,
                         bool translucentBackground) Q_DECL_OVERRIDE;
#endif
    QImage toImage() const Q_DECL_OVERRIDE;

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
    GLuint toTexture(const QRegion &dirtyRegion, QSize *textureSize) const Q_DECL_OVERRIDE;
#elif QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    GLuint toTexture(const QRegion &dirtyRegion, QSize *textureSize, bool *needsSwizzle) const Q_DECL_OVERRIDE;
#else
    GLuint toTexture(const QRegion &dirtyRegion, QSize *textureSize, TextureFlags *flags) const Q_DECL_OVERRIDE;
#endif
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    QPlatformGraphicsBuffer *graphicsBuffer() const Q_DECL_OVERRIDE;
#endif

    void resize(const QSize &size, const QRegion &staticContents) Q_DECL_OVERRIDE;

    void beginPaint(const QRegion &region) Q_DECL_OVERRIDE;
    void endPaint() Q_DECL_OVERRIDE;

    static bool isWidgetWindow(const QWindow *window);

private:
    void initUserProperties();

    bool updateWindowMargins(bool repaintShadow = true);
    void updateFrameExtents();
    void updateInputShapeRegion();
    void updateClipPath();
    void updateWindowShadow();
    bool updateWindowBlurAreasForWM();
    void doDelayedUpdateWindowShadow(int delaye = 30);

    /// update of user properties
    void updateWindowRadius();
    void updateBorderWidth();
    void updateBorderColor();
    void updateUserClipPath();
    void updateFrameMask();
    void updateShadowRadius();
    void updateShadowOffset();
    void updateShadowColor();
    void updateTranslucentBackground();
    void updateEnableSystemResize();
    void updateEnableSystemMove();
    void updateEnableBlurWindow();
    void updateWindowBlurAreas();
    void updateWindowBlurPaths();
    void updateAutoInputMaskByClipPath();

    void setWindowMargins(const QMargins &margins);
    void setClipPah(const QPainterPath &path);
    void setWindowBlurArea(const QVector<Utility::BlurArea> &area);

    void paintWindowShadow(QRegion region = QRegion());
    void repaintWindowShadow();

    inline bool isWidgetWindow() const
    { return isWidgetWindow(window());}
    QWidgetWindow *widgetWindow() const;

    inline QPoint windowOffset() const
    { return QPoint(windowMargins.left(), windowMargins.top());}
    inline QRect windowGeometry() const
    { return QRect(windowOffset(), m_windowSize);}

    inline int getShadowRadius() const
    { return m_enableShadow ? m_shadowRadius : 0;}
    inline QPoint getShadowOffset() const
    { return m_shadowOffset;}

    inline int getWindowRadius() const;

    bool canUseClipPath() const;
    bool canResize() const;

    void onWindowStateChanged();
    void handlePropertyNotifyEvent(const xcb_property_notify_event_t *event);

    QSize m_size;
    QSize m_windowSize;
    QImage m_image;

    QXcbBackingStore *m_proxy;
    WindowEventListener *m_eventListener = Q_NULLPTR;
//    DXcbShmGraphicsBuffer *m_graphicsBuffer = Q_NULLPTR;
    DPlatformWindowHook *m_windowHook = Q_NULLPTR;

    int m_windowRadius = 4;
    bool isUserSetWindowRadius = false;
    int m_borderWidth = 1;
    bool isUserSetClipPath = false;
    QPainterPath m_clipPath;
    QPainterPath m_windowClipPath;
    QColor m_borderColor = QColor(0, 0, 0, 255 * 0.15);

    int m_shadowRadius = 60;
    QPoint m_shadowOffset = QPoint(0, 16);
    QColor m_shadowColor = QColor(0, 0, 0, 255 * 0.6);
    QPixmap shadowPixmap;

    bool m_translucentBackground = false;
    bool m_enableSystemResize = true;
    bool m_enableSystemMove = true;
    bool m_enableBlurWindow = false;
    bool m_autoInputMaskByClipPath = true;
    bool m_enableShadow = true;

    QVector<Utility::BlurArea> m_blurAreaList;
    QList<QPainterPath> m_blurPathList;

    QRect windowValidRect;
    QMargins windowMargins;

    bool isUserSetFrameMask = false;

    QBasicTimer updateShadowTimer;

    friend class WindowEventListener;

#ifndef QT_NO_OPENGL
    GLuint m_textureId;
    QSize m_textureSize;
    bool m_needsSwizzle;
    bool m_premultiplied;
    QOpenGLTextureBlitter *m_blitter;
#endif
};

DPP_END_NAMESPACE

Q_DECLARE_METATYPE(QMargins)

#endif // DXCBBACKINGSTORE_H
