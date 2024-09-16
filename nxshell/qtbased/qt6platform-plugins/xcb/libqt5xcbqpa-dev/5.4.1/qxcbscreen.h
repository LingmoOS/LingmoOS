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

#include <private/qfontengine_p.h>

QT_BEGIN_NAMESPACE

class QXcbConnection;
class QXcbCursor;
class QXcbXSettings;

class QXcbScreen : public QXcbObject, public QPlatformScreen
{
public:
    QXcbScreen(QXcbConnection *connection, xcb_screen_t *screen,
               xcb_randr_get_output_info_reply_t *output, QString outputName, int number);
    ~QXcbScreen();

    QPixmap grabWindow(WId window, int x, int y, int width, int height) const;

    QWindow *topLevelAt(const QPoint &point) const;

    QRect geometry() const { return m_geometry; }
    QRect nativeGeometry() const { return m_nativeGeometry; }
    QRect availableGeometry() const {return m_availableGeometry;}
    int depth() const { return m_screen->root_depth; }
    QImage::Format format() const;
    QSizeF physicalSize() const { return m_sizeMillimeters; }
    QDpi logicalDpi() const;
    qreal devicePixelRatio() const;
    QPlatformCursor *cursor() const;
    qreal refreshRate() const { return m_refreshRate; }
    Qt::ScreenOrientation orientation() const { return m_orientation; }
    QList<QPlatformScreen *> virtualSiblings() const { return m_siblings; }
    void setVirtualSiblings(QList<QPlatformScreen *> sl) { m_siblings = sl; }

    int screenNumber() const { return m_number; }

    xcb_screen_t *screen() const { return m_screen; }
    xcb_window_t root() const { return m_screen->root; }

    xcb_window_t clientLeader() const { return m_clientLeader; }

    void windowShown(QXcbWindow *window);
    QString windowManagerName() const { return m_windowManagerName; }
    bool syncRequestSupported() const { return m_syncRequestSupported; }

    const xcb_visualtype_t *visualForId(xcb_visualid_t) const;
    quint8 depthOfVisual(xcb_visualid_t) const;

    QString name() const { return m_outputName; }

    void handleScreenChange(xcb_randr_screen_change_notify_event_t *change_event);
    void updateGeometry(xcb_timestamp_t timestamp);
    void updateRefreshRate();

    void readXResources();

    QFontEngine::HintStyle hintStyle() const { return m_hintStyle; }
    bool noFontHinting() const { return m_noFontHinting; }
    QFontEngine::SubpixelAntialiasingType subpixelType() const { return m_subpixelType; }
    int antialiasingEnabled() const { return m_antialiasingEnabled; }

    QXcbXSettings *xSettings() const;

private:
    static bool xResource(const QByteArray &identifier,
                          const QByteArray &expectedIdentifier,
                          QByteArray &stringValue);
    void sendStartupMessage(const QByteArray &message) const;

    xcb_screen_t *m_screen;
    xcb_randr_crtc_t m_crtc;
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
    int m_number;
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
    QXcbXSettings *m_xSettings;
};

QT_END_NAMESPACE

#endif
