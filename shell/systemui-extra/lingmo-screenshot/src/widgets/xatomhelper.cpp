
#include "xatomhelper.h"

#include <X11/Xlib.h>

static XAtomHelper *global_instance = nullptr;

XAtomHelper *XAtomHelper::getInstance()
{
    if (! global_instance)
        global_instance = new XAtomHelper;
    return global_instance;
}

bool XAtomHelper::isFrameLessWindow(int winId)
{
    auto hints = getInstance()->getWindowMotifHint(winId);
    if (hints.flags == MWM_HINTS_DECORATIONS && hints.functions == 1) {
        return true;
    }
    return false;
}

bool XAtomHelper::isWindowDecorateBorderOnly(int winId)
{
    return isWindowMotifHintDecorateBorderOnly(getInstance()->getWindowMotifHint(winId));
}

bool XAtomHelper::isWindowMotifHintDecorateBorderOnly(const MotifWmHints &hint)
{
    bool isDeco = false;
    if (hint.flags & MWM_HINTS_DECORATIONS && hint.flags != MWM_HINTS_DECORATIONS) {
        if (hint.decorations == MWM_DECOR_BORDER)
            isDeco = true;
    }
    return isDeco;
}

bool XAtomHelper::isLINGMOCsdSupported()
{
    // fixme:
    return false;
}

bool XAtomHelper::isLINGMODecorationWindow(int winId)
{
    if (m_lingmoDecorationAtion == None)
        return false;

    unsigned long type;
    int format;
    ulong nitems;
    ulong bytes_after;
    uchar *data;

    bool isLINGMODecoration = false;

    XGetWindowProperty(QX11Info::display(), winId, m_lingmoDecorationAtion,
                       0, LONG_MAX, false,
                       m_lingmoDecorationAtion, &type,
                       &format, &nitems,
                       &bytes_after, &data);

    if (type == m_lingmoDecorationAtion) {
        if (nitems == 1) {
            isLINGMODecoration = data[0];
        }
    }

    return isLINGMODecoration;
}

UnityCorners XAtomHelper::getWindowBorderRadius(int winId)
{
    UnityCorners corners;

    Atom type;
    int format;
    ulong nitems;
    ulong bytes_after;
    uchar *data;

    if (m_unityBorderRadiusAtom != None) {
        XGetWindowProperty(QX11Info::display(), winId, m_unityBorderRadiusAtom,
                           0, LONG_MAX, false,
                           XA_CARDINAL, &type,
                           &format, &nitems,
                           &bytes_after, &data);

        if (type == XA_CARDINAL) {
            if (nitems == 4) {
                corners.topLeft = static_cast<ulong>(data[0]);
                corners.topRight = static_cast<ulong>(data[1*sizeof (ulong)]);
                corners.bottomLeft = static_cast<ulong>(data[2*sizeof (ulong)]);
                corners.bottomRight = static_cast<ulong>(data[3*sizeof (ulong)]);
            }
            XFree(data);
        }
    }

    return corners;
}

void XAtomHelper::setWindowBorderRadius(int winId, const UnityCorners &data)
{
    if (m_unityBorderRadiusAtom == None)
        return;

    ulong corners[4] = {data.topLeft, data.topRight, data.bottomLeft, data.bottomRight};

    XChangeProperty(QX11Info::display(), winId, m_unityBorderRadiusAtom, XA_CARDINAL,
                    32, XCB_PROP_MODE_REPLACE, (const unsigned char *) &corners, sizeof (corners)/sizeof (corners[0]));
}

void XAtomHelper::setWindowBorderRadius(int winId, int topLeft, int topRight, int bottomLeft, int bottomRight)
{
    if (m_unityBorderRadiusAtom == None)
        return;

    ulong corners[4] = {(ulong)topLeft, (ulong)topRight, (ulong)bottomLeft, (ulong)bottomRight};

    XChangeProperty(QX11Info::display(), winId, m_unityBorderRadiusAtom, XA_CARDINAL,
                    32, XCB_PROP_MODE_REPLACE, (const unsigned char *) &corners, sizeof (corners)/sizeof (corners[0]));
}

void XAtomHelper::setLINGMODecoraiontHint(int winId, bool set)
{
    if (m_lingmoDecorationAtion == None)
        return;

    XChangeProperty(QX11Info::display(), winId, m_lingmoDecorationAtion, m_lingmoDecorationAtion, 32, XCB_PROP_MODE_REPLACE, (const unsigned char *) &set, 1);
}

void XAtomHelper::setWindowMotifHint(int winId, const MotifWmHints &hints)
{
    if (m_unityBorderRadiusAtom == None)
        return;

    XChangeProperty(QX11Info::display(), winId, m_motifWMHintsAtom, m_motifWMHintsAtom,
                    32, XCB_PROP_MODE_REPLACE, (const unsigned char *)&hints, sizeof (MotifWmHints)/ sizeof (ulong));
}

MotifWmHints XAtomHelper::getWindowMotifHint(int winId)
{
    MotifWmHints hints;

    if (m_unityBorderRadiusAtom == None)
        return hints;

    uchar *data;
    Atom type;
    int format;
    ulong nitems;
    ulong bytes_after;

    XGetWindowProperty(QX11Info::display(), winId, m_motifWMHintsAtom,
                       0, sizeof (MotifWmHints)/sizeof (long), false, AnyPropertyType, &type,
                       &format, &nitems, &bytes_after, &data);

    if (type == None) {
        return hints;
    } else {
        hints = *(MotifWmHints *)data;
        XFree(data);
    }
    return hints;
}

XAtomHelper::XAtomHelper(QObject *parent) : QObject(parent)
{
    if (!QX11Info::isPlatformX11())
        return;

    m_motifWMHintsAtom = XInternAtom(QX11Info::display(), "_MOTIF_WM_HINTS", true);
    m_unityBorderRadiusAtom = XInternAtom(QX11Info::display(), "_UNITY_GTK_BORDER_RADIUS", false);
    m_lingmoDecorationAtion = XInternAtom(QX11Info::display(), "_KWIN_LINGMO_DECORAION", false);
}

Atom XAtomHelper::registerLINGMOCsdNetWmSupportAtom()
{
    // fixme:
    return None;
}

void XAtomHelper::unregisterLINGMOCsdNetWmSupportAtom()
{
    // fixme:
}
