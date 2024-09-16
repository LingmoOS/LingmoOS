// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBSCREEN_H
#define QXCBSCREEN_H

#include <qpa/qplatformscreen.h>
#include <QtCore/QString>

#include <xcb/xcb.h>
#include <xcb/randr.h>

#include "qxcbobject.h"
#include "qxcbscreen.h"

#include <private/qfontengine_p.h>

QT_BEGIN_NAMESPACE

class QXcbConnection;
class QXcbCursor;
class QXcbXSettings;
#ifndef QT_NO_DEBUG_STREAM
class QDebug;
#endif

class QXcbVirtualDesktop : public QXcbObject
{
public:
    QXcbVirtualDesktop(QXcbConnection *connection, xcb_screen_t *screen, int number);
    ~QXcbVirtualDesktop();

    xcb_screen_t *screen() const { return m_screen; }
    int number() const { return m_number; }
    QSize size() const { return QSize(m_screen->width_in_pixels, m_screen->height_in_pixels); }
    QSize physicalSize() const { return QSize(m_screen->width_in_millimeters, m_screen->height_in_millimeters); }

    QXcbXSettings *xSettings() const;

private:
    xcb_screen_t *m_screen;
    int m_number;

    QXcbXSettings *m_xSettings;
};

class Q_XCB_EXPORT QXcbScreen : public QXcbObject, public QPlatformScreen
{
public:
    QXcbScreen(QXcbConnection *connection, QXcbVirtualDesktop *virtualDesktop,
               xcb_randr_output_t outputId, xcb_randr_get_output_info_reply_t *output,
               QString outputName);
    ~QXcbScreen();

    QPixmap grabWindow(WId window, int x, int y, int width, int height) const Q_DECL_OVERRIDE;

    QWindow *topLevelAt(const QPoint &point) const Q_DECL_OVERRIDE;

    QRect geometry() const Q_DECL_OVERRIDE { return m_geometry; }
    QRect nativeGeometry() const { return m_nativeGeometry; }
    QRect availableGeometry() const Q_DECL_OVERRIDE {return m_availableGeometry;}
    int depth() const Q_DECL_OVERRIDE { return screen()->root_depth; }
    QImage::Format format() const Q_DECL_OVERRIDE;
    QSizeF physicalSize() const Q_DECL_OVERRIDE { return m_sizeMillimeters; }
    QSize virtualSize() const { return m_virtualSize; }
    QSizeF physicalVirtualSize() const { return m_virtualSizeMillimeters; }
    QDpi virtualDpi() const;
    QDpi logicalDpi() const Q_DECL_OVERRIDE;
    qreal devicePixelRatio() const Q_DECL_OVERRIDE;
    QPlatformCursor *cursor() const Q_DECL_OVERRIDE;
    qreal refreshRate() const Q_DECL_OVERRIDE { return m_refreshRate; }
    Qt::ScreenOrientation orientation() const Q_DECL_OVERRIDE { return m_orientation; }
    QList<QPlatformScreen *> virtualSiblings() const Q_DECL_OVERRIDE { return m_siblings; }
    void setVirtualSiblings(QList<QPlatformScreen *> sl) { m_siblings = sl; }
    void removeVirtualSibling(QPlatformScreen *s) { m_siblings.removeOne(s); }
    void addVirtualSibling(QPlatformScreen *s) { ((QXcbScreen *) s)->isPrimary() ? m_siblings.prepend(s) : m_siblings.append(s); }

    void setPrimary(bool primary) { m_primary = primary; }
    bool isPrimary() const { return m_primary; }

    int screenNumber() const { return m_virtualDesktop->number(); }

    xcb_screen_t *screen() const { return m_virtualDesktop->screen(); }
    xcb_window_t root() const { return screen()->root; }
    xcb_randr_output_t output() const { return m_output; }
    xcb_randr_crtc_t crtc() const { return m_crtc; }
    xcb_randr_mode_t mode() const { return m_mode; }

    xcb_window_t clientLeader() const { return m_clientLeader; }

    void windowShown(QXcbWindow *window);
    QString windowManagerName() const { return m_windowManagerName; }
    bool syncRequestSupported() const { return m_syncRequestSupported; }

    const xcb_visualtype_t *visualForId(xcb_visualid_t) const;
    quint8 depthOfVisual(xcb_visualid_t) const;

    QString name() const Q_DECL_OVERRIDE { return m_outputName; }

    void handleScreenChange(xcb_randr_screen_change_notify_event_t *change_event);
    void updateGeometry(const QRect &geom, uint8_t rotation);
    void updateGeometry(xcb_timestamp_t timestamp = XCB_TIME_CURRENT_TIME);
    void updateRefreshRate(xcb_randr_mode_t mode);

    void readXResources();

    QFontEngine::HintStyle hintStyle() const { return m_hintStyle; }
    bool noFontHinting() const { return m_noFontHinting; }
    QFontEngine::SubpixelAntialiasingType subpixelType() const { return m_subpixelType; }
    int antialiasingEnabled() const { return m_antialiasingEnabled; }

    QXcbXSettings *xSettings() const;

    QPoint mapToNative(const QPoint &pos) const;
    QPoint mapFromNative(const QPoint &pos) const;
    QRect mapToNative(const QRect &rect) const;
    QRect mapFromNative(const QRect &rect) const;

private:
    static bool xResource(const QByteArray &identifier,
                          const QByteArray &expectedIdentifier,
                          QByteArray &stringValue);
    void sendStartupMessage(const QByteArray &message) const;

    QXcbVirtualDesktop *m_virtualDesktop;
    xcb_randr_output_t m_output;
    xcb_randr_crtc_t m_crtc;
    xcb_randr_mode_t m_mode;
    bool m_primary;
    uint8_t m_rotation;

    QString m_outputName;
    QSizeF m_outputSizeMillimeters;
    QSizeF m_sizeMillimeters;
    QRect m_geometry;
    QRect m_nativeGeometry;
    QRect m_availableGeometry;
    QSize m_virtualSize;
    QSizeF m_virtualSizeMillimeters;
    QList<QPlatformScreen *> m_siblings;
    Qt::ScreenOrientation m_orientation;
    QString m_windowManagerName;
    bool m_syncRequestSupported;
    xcb_window_t m_clientLeader;
    QMap<xcb_visualid_t, xcb_visualtype_t> m_visuals;
    QMap<xcb_visualid_t, quint8> m_visualDepths;
    QXcbCursor *m_cursor;
    int m_refreshRate;
    int m_forcedDpi;
    int m_devicePixelRatio;
    QFontEngine::HintStyle m_hintStyle;
    bool m_noFontHinting;
    QFontEngine::SubpixelAntialiasingType m_subpixelType;
    int m_antialiasingEnabled;
};

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QXcbScreen *);
#endif

QT_END_NAMESPACE

#endif
