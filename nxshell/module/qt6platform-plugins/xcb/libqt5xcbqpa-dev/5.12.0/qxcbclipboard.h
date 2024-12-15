// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBCLIPBOARD_H
#define QXCBCLIPBOARD_H

#include <qpa/qplatformclipboard.h>
#include <qxcbobject.h>
#include <xcb/xcb.h>
#include <xcb/xfixes.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_CLIPBOARD

class QXcbConnection;
class QXcbScreen;
class QXcbClipboardMime;

class QXcbClipboard : public QXcbObject, public QPlatformClipboard
{
public:
    QXcbClipboard(QXcbConnection *connection);
    ~QXcbClipboard();

    QMimeData *mimeData(QClipboard::Mode mode) override;
    void setMimeData(QMimeData *data, QClipboard::Mode mode) override;

    bool supportsMode(QClipboard::Mode mode) const override;
    bool ownsMode(QClipboard::Mode mode) const override;

    QXcbScreen *screen() const;

    xcb_window_t requestor() const;
    void setRequestor(xcb_window_t window);

    xcb_window_t owner() const;

    void handleSelectionRequest(xcb_selection_request_event_t *event);
    void handleSelectionClearRequest(xcb_selection_clear_event_t *event);
    void handleXFixesSelectionRequest(xcb_xfixes_selection_notify_event_t *event);

    bool clipboardReadProperty(xcb_window_t win, xcb_atom_t property, bool deleteProperty, QByteArray *buffer, int *size, xcb_atom_t *type, int *format);
    QByteArray clipboardReadIncrementalProperty(xcb_window_t win, xcb_atom_t property, int nbytes, bool nullterm);

    QByteArray getDataInFormat(xcb_atom_t modeAtom, xcb_atom_t fmtatom);

    void setProcessIncr(bool process) { m_incr_active = process; }
    bool processIncr() { return m_incr_active; }
    void incrTransactionPeeker(xcb_generic_event_t *ge, bool &accepted);

    xcb_window_t getSelectionOwner(xcb_atom_t atom) const;
    QByteArray getSelection(xcb_atom_t selection, xcb_atom_t target, xcb_atom_t property, xcb_timestamp_t t = 0);

private:
    xcb_generic_event_t *waitForClipboardEvent(xcb_window_t window, int type, bool checkManager = false);

    xcb_atom_t sendTargetsSelection(QMimeData *d, xcb_window_t window, xcb_atom_t property);
    xcb_atom_t sendSelection(QMimeData *d, xcb_atom_t target, xcb_window_t window, xcb_atom_t property);

    xcb_atom_t atomForMode(QClipboard::Mode mode) const;
    QClipboard::Mode modeForAtom(xcb_atom_t atom) const;

    // Selection and Clipboard
    QScopedPointer<QXcbClipboardMime> m_xClipboard[2];
    QMimeData *m_clientClipboard[2];
    xcb_timestamp_t m_timestamp[2];

    xcb_window_t m_requestor = XCB_NONE;
    xcb_window_t m_owner = XCB_NONE;

    static const int clipboard_timeout;

    bool m_incr_active = false;
    bool m_clipboard_closing = false;
    xcb_timestamp_t m_incr_receive_time = 0;
};

#endif // QT_NO_CLIPBOARD

QT_END_NAMESPACE

#endif // QXCBCLIPBOARD_H
