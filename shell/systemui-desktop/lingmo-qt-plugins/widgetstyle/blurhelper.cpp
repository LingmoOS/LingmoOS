//////////////////////////////////////////////////////////////////////////////
// breezeblurhelper.cpp
// handle regions passed to kwin for blurring
// -------------------
//
// Copyright (C) 2024 Reion Wong <reionwong@gmail.com>
// Copyright (C) 2018 Alex Nemeth <alex.nemeth329@gmail.com>
//
// Largely rewritten from Oxygen widget style
// Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "blurhelper.h"

// KF5
#include <KWindowEffects>

// Qt
#include <QWidget>
#include <QScreen>
#include <QRect>
#include <QVariant>
#include <QEvent>
#include <QPainterPath>
#include <QApplication>
#include <QX11Info>

// XCB
#include <xcb/xcb.h>
#include <xcb/shape.h>
#include <xcb/xcb_icccm.h>

BlurHelper::BlurHelper(QObject *parent)
    : QObject(parent)
{
}

void BlurHelper::registerWidget(QWidget *widget)
{
    // install event filter
    addEventFilter(widget);

    // schedule shadow area repaint
    update(widget);
}

void BlurHelper::unregisterWidget(QWidget *widget)
{
    // remove event filter
    widget->removeEventFilter(this);
}

bool BlurHelper::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Hide:
    case QEvent::Show:
    case QEvent::Resize: {
        // cast to widget and check
        QWidget *widget(qobject_cast<QWidget*>(object));

        if (!widget)
            break;

        update(widget);
        break;
    }

    default: break;
    }

    // never eat events
    return false;
}


void BlurHelper::update(QWidget *widget) const
{
    /*
      directly from bespin code. Supposedly prevent playing with some 'pseudo-widgets'
      that have winId matching some other -random- window
    */
    if (!(widget->testAttribute(Qt::WA_WState_Created) || widget->internalWinId()))
        return;

    if (widget->mask().isEmpty()) {
        KWindowEffects::enableBlurBehind(widget->winId(), true);
    } else {
        KWindowEffects::enableBlurBehind(widget->winId(), true, widget->mask());
    }

    // force update
    if (widget->isVisible()) {
        widget->update();
    }
}

void BlurHelper::enableBlurBehind(QWidget *widget, bool enable, qreal windowRadius)
{
    if (!widget)
        return;

    xcb_connection_t *c = QX11Info::connection();
    if (!c)
        return;

    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_BLUR_BEHIND_REGION");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());
    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom)
        return;

    if (enable) {
        qreal devicePixelRatio = qApp->devicePixelRatio();
        QPainterPath path;
        path.addRoundedRect(QRectF(QPoint(0, 0), widget->size() * devicePixelRatio),
                            windowRadius * devicePixelRatio,
                            windowRadius * devicePixelRatio);
        QVector<uint32_t> data;
        for (const QPolygonF &polygon : path.toFillPolygons()) {
            QRegion region = polygon.toPolygon();
            for (auto i = region.begin(); i != region.end(); ++i) {
                data << i->x() << i->y() << i->width() << i->height();
            }
        }

        xcb_change_property(c, XCB_PROP_MODE_REPLACE, widget->winId(),
                            atom->atom, XCB_ATOM_CARDINAL,
                            32, data.size(), data.constData());

    } else {
        xcb_delete_property(c, widget->winId(), atom->atom);
    }
}
