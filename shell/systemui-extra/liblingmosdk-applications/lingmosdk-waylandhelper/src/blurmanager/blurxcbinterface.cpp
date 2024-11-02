#include "blurxcbinterface.h"
#include <QX11Info>
#include <QApplication>

BlurXcbInterface::BlurXcbInterface(QObject *parent)
    : BlurAbstractInterface(parent)
{

}

bool BlurXcbInterface::setBlurBehindWithStrength(QWindow *window, bool enable, const QRegion &region, uint32_t strength)
{
    xcb_connection_t *c = QX11Info::connection();
    if (!c) {
        return false;
    }
    const QByteArray effectName = QByteArrayLiteral("_KDE_NET_WM_BLUR_BEHIND_REGION");
    xcb_intern_atom_cookie_t atomCookie = xcb_intern_atom_unchecked(c, false, effectName.length(), effectName.constData());
    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> atom(xcb_intern_atom_reply(c, atomCookie, nullptr));
    if (!atom)
    {
        return false;
    }

    if (enable)
    {
        QVector<uint32_t> data;
        data.reserve(region.rectCount() * 4 + 1);
        for (const QRect &r : region)
        {
            auto dpr = qApp->devicePixelRatio();
            data << r.x() * dpr << r.y() * dpr << r.width() * dpr << r.height() * dpr;
        }
        data << strength;

        xcb_change_property(c, XCB_PROP_MODE_REPLACE, window->winId(), atom->atom, XCB_ATOM_CARDINAL, 32, data.size(), data.constData());
        return true;
    }
    else
    {
        xcb_delete_property(c, window->winId(), atom->atom);
        xcb_flush(c);
        return true;
    }
}
