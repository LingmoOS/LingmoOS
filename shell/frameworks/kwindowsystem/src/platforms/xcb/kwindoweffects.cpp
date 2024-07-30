/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwindoweffects_x11.h"

#include <QGuiApplication>
#include <QVarLengthArray>

#include "kx11extras.h"
#include <config-kwindowsystem.h>

#include <QMatrix4x4>
#include <QWindow>
#include <private/qtx11extras_p.h>

#include <xcb/xcb.h>

#include "cptr_p.h"
#include <cmath>

using namespace KWindowEffects;

KWindowEffectsPrivateX11::KWindowEffectsPrivateX11()
{
}

KWindowEffectsPrivateX11::~KWindowEffectsPrivateX11()
{
}

bool KWindowEffectsPrivateX11::isEffectAvailable(Effect effect)
{
    if (!KX11Extras::self()->compositingActive()) {
        return false;
    }
    QByteArray effectName;

    switch (effect) {
    case Slide:
        effectName = QByteArrayLiteral("_KDE_SLIDE");
        break;
    case BlurBehind:
        effectName = QByteArrayLiteral("_KDE_NET_WM_BLUR_BEHIND_REGION");
        break;
    case BackgroundContrast:
        effectName = QByteArrayLiteral("_KDE_NET_WM_BACKGROUND_CONTRAST_REGION");
        break;
    default:
        return false;
    }

    // hackish way to find out if KWin has the effect enabled,
    // TODO provide proper support
    xcb_connection_t *c = QX11Info::connection();
    xcb_list_properties_cookie_t propsCookie = xcb_list_properties_unchecked(c, QX11Info::appRootWindow());
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());

    UniqueCPointer<xcb_list_properties_reply_t> props(xcb_list_properties_reply(c, propsCookie, nullptr));
    UniqueCPointer<xcb_intern_atom_reply_t> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom || !props) {
        return false;
    }
    xcb_atom_t *atoms = xcb_list_properties_atoms(props.get());
    for (int i = 0; i < props->atoms_len; ++i) {
        if (atoms[i] == atom->atom) {
            return true;
        }
    }
    return false;
}

void KWindowEffectsPrivateX11::slideWindow(QWindow *window, SlideFromLocation location, int offset)
{
    xcb_connection_t *c = QX11Info::connection();
    if (!c) {
        return;
    }

    const QByteArray effectName = QByteArrayLiteral("_KDE_SLIDE");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());

    const int size = 2;
    int32_t data[size];
    data[0] = offset;

    switch (location) {
    case LeftEdge:
        data[1] = 0;
        break;
    case TopEdge:
        data[1] = 1;
        break;
    case RightEdge:
        data[1] = 2;
        break;
    case BottomEdge:
        data[1] = 3;
    default:
        break;
    }

    UniqueCPointer<xcb_intern_atom_reply_t> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom) {
        return;
    }
    if (location == NoEdge) {
        xcb_delete_property(c, window->winId(), atom->atom);
    } else {
        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom->atom, atom->atom, 32, size, data);
    }
}

void KWindowEffectsPrivateX11::enableBlurBehind(QWindow *window, bool enable, const QRegion &region)
{
    xcb_connection_t *c = QX11Info::connection();
    if (!c) {
        return;
    }
    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_BLUR_BEHIND_REGION");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());
    UniqueCPointer<xcb_intern_atom_reply_t> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom) {
        return;
    }

    if (enable) {
        QList<uint32_t> data;
        data.reserve(region.rectCount() * 4);
        for (const QRect &r : region) {
            // kwin on X uses device pixels, convert from logical
            auto dpr = qApp->devicePixelRatio();
            data << std::floor(r.x() * dpr) << std::floor(r.y() * dpr) << std::ceil(r.width() * dpr) << std::ceil(r.height() * dpr);
        }

        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom->atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData());
    } else {
        xcb_delete_property(c, window->winId(), atom->atom);
    }
}

void KWindowEffectsPrivateX11::enableBackgroundContrast(QWindow *window, bool enable, qreal contrast, qreal intensity, qreal saturation, const QRegion &region)
{
    xcb_connection_t *c = QX11Info::connection();
    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_BACKGROUND_CONTRAST_REGION");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());
    UniqueCPointer<xcb_intern_atom_reply_t> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom) {
        return;
    }

    if (enable) {
        QList<uint32_t> data;
        data.reserve(region.rectCount() * 4 + 16);
        for (const QRect &r : region) {
            auto dpr = qApp->devicePixelRatio();
            data << std::floor(r.x() * dpr) << std::floor(r.y() * dpr) << std::ceil(r.width() * dpr) << std::ceil(r.height() * dpr);
        }

        QMatrix4x4 satMatrix; // saturation
        QMatrix4x4 intMatrix; // intensity
        QMatrix4x4 contMatrix; // contrast

        // clang-format off

        //Saturation matrix
        if (!qFuzzyCompare(saturation, 1.0)) {
            const qreal rval = (1.0 - saturation) * .2126;
            const qreal gval = (1.0 - saturation) * .7152;
            const qreal bval = (1.0 - saturation) * .0722;

            satMatrix = QMatrix4x4(rval + saturation, rval,     rval,     0.0,
                gval,     gval + saturation, gval,     0.0,
                bval,     bval,     bval + saturation, 0.0,
                0,        0,        0,        1.0);
        }

        //IntensityMatrix
        if (!qFuzzyCompare(intensity, 1.0)) {
            intMatrix.scale(intensity, intensity, intensity);
        }

        //Contrast Matrix
        if (!qFuzzyCompare(contrast, 1.0)) {
            const float transl = (1.0 - contrast) / 2.0;

            contMatrix = QMatrix4x4(contrast, 0,        0,        0.0,
                0,        contrast, 0,        0.0,
                0,        0,        contrast, 0.0,
                transl,   transl,   transl,   1.0);
        }

        // clang-format on

        QMatrix4x4 colorMatrix = contMatrix * satMatrix * intMatrix;
        colorMatrix = colorMatrix.transposed();

        uint32_t *rawData = reinterpret_cast<uint32_t *>(colorMatrix.data());

        for (int i = 0; i < 16; ++i) {
            data << rawData[i];
        }

        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom->atom, atom->atom, 32, data.size(), data.constData());
    } else {
        xcb_delete_property(c, window->winId(), atom->atom);
    }
}
