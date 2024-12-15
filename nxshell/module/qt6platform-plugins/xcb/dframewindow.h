// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFRAMEWINDOW_H
#define DFRAMEWINDOW_H

#include "global.h"
#include "utility.h"

#include <QPaintDeviceWindow>
#include <QVariantAnimation>
#include <QTimer>
#include <QPointer>

#ifdef Q_OS_LINUX
#include <cairo.h>
struct xcb_rectangle_t;
#endif

QT_BEGIN_NAMESPACE
class QPlatformBackingStore;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DFrameWindowPrivate;
class DFrameWindow : public QPaintDeviceWindow
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(DFrameWindow)

public:
    explicit DFrameWindow(QWindow *content);
    ~DFrameWindow();

    QWindow *contentWindow() const;

    int shadowRadius() const;
    void setShadowRadius(int radius);
    QPoint shadowOffset() const;
    void setShadowOffset(const QPoint &offset);
    QColor shadowColor() const;
    void setShadowColor(const QColor &color);

    int borderWidth() const;
    void setBorderWidth(int width);
    QColor borderColor() const;
    void setBorderColor(const QColor &color);

    QPainterPath contentPath() const;
    void setContentPath(const QPainterPath &path);
    void setContentRoundedRect(const QRect &rect, int radius = 0);

    QMargins contentMarginsHint() const;
    QPoint contentOffsetHint() const;

    bool isClearContentAreaForShadowPixmap() const;
    void setClearContentAreaForShadowPixmap(bool clear);

    bool isEnableSystemResize() const;
    void setEnableSystemResize(bool enable);
    bool isEnableSystemMove() const;
    void setEnableSystemMove(bool enable);

    void disableRepaintShadow();
    void enableRepaintShadow();

    bool redirectContent() const;

signals:
    void contentMarginsHintChanged(const QMargins &oldMargins) const;

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;

    void updateFromContents(void *);
    void drawShadowTo(QPaintDevice *device);

private:
    QPaintDevice *redirected(QPoint *) const Q_DECL_OVERRIDE;

    void setContentPath(const QPainterPath &path, bool isRoundedRect, int radius = 0);
#ifdef Q_OS_LINUX
    void drawNativeWindowXPixmap(xcb_rectangle_t *rects = 0, int length = 0);
    bool updateNativeWindowXPixmap(int width, int height);
    void markXPixmapToDirty(int width = -1, int height = -1);
#endif

    void updateShadow();
    void updateShadowAsync(int delaye = 30);
    void updateContentMarginsHint(bool force = false);
    void updateMask();
    void updateFrameMask();

    bool canResize() const;
    void cancelAdsorbCursor();
    void adsorbCursor(Utility::CornerEdge cornerEdge);
    void startCursorAnimation();

    bool disableFrame() const;

    void onDevicePixelRatioChanged();

    static QList<DFrameWindow*> frameWindowList;

    QPlatformBackingStore *platformBackingStore;

    QImage m_shadowImage;
    bool m_clearContent = false;
    bool m_redirectContent = false;

    int m_shadowRadius = 60;
    QPoint m_shadowOffset = QPoint(0, 16);
    QColor m_shadowColor = QColor(0, 0, 0, 255 * 0.6);

    int m_borderWidth = 1;
    QColor m_borderColor = QColor(0, 0, 0, 255 * 0.15);
    QPainterPath m_clipPathOfContent;
    QPainterPath m_clipPath;
    QPainterPath m_borderPath;
    QRect m_contentGeometry;
    QMargins m_contentMarginsHint;
    bool m_pathIsRoundedRect = true;
    int m_roundedRectRadius = 0;

    bool m_enableSystemResize = true;
    bool m_enableSystemMove = true;
    bool m_enableAutoInputMaskByContentPath = true;
    bool m_enableAutoFrameMask = true;
    bool m_canUpdateShadow = true;

    bool m_canAdsorbCursor = false;
    bool m_isSystemMoveResizeState = false;
    Utility::CornerEdge m_lastCornerEdge;
    QTimer m_startAnimationTimer;
    QVariantAnimation m_cursorAnimation;

    QPointer<QWindow> m_contentWindow;
    QPlatformBackingStore *m_contentBackingStore = nullptr;

    QTimer m_updateShadowTimer;
    int m_paintShadowOnContentTimerId = -1;

#ifdef Q_OS_LINUX
    uint32_t nativeWindowXPixmap = 0;
    cairo_surface_t *nativeWindowXSurface = 0;
    QSize xsurfaceDirtySize;
#endif

    friend class DPlatformWindowHelper;
    friend class DPlatformBackingStoreHelper;
    friend class DPlatformOpenGLContextHelper;
    friend class DPlatformIntegration;
    friend class WindowEventHook;
    friend class DXcbWMSupport;
    friend class DFrameWindowPrivate;
    friend class XcbNativeEventFilter;
};

DPP_END_NAMESPACE

#endif // DFRAMEWINDOW_H
