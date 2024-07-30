/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2015 Bhushan Shah <bhush94@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
*/
// own
#include "../x11locker.h"
// Qt
#include <QWindow>
#include <private/qtx11extras_p.h>
#include <QTest>
// xcb
#include <xcb/xcb.h>

template<typename T>
using ScopedCPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

class LockWindowTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testBlankScreen();
    void testEmergencyShow();
};

xcb_screen_t *defaultScreen()
{
    int screen = QX11Info::appScreen();
    for (xcb_screen_iterator_t it = xcb_setup_roots_iterator(xcb_get_setup(QX11Info::connection())); it.rem; --screen, xcb_screen_next(&it)) {
        if (screen == 0) {
            return it.data;
        }
    }
    return nullptr;
}

bool isColored(const QColor color, const int x, const int y, const int width, const int height)
{
    xcb_connection_t *c = QX11Info::connection();
    const auto cookie = xcb_get_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, QX11Info::appRootWindow(), x, y, width, height, ~0);
    ScopedCPointer<xcb_get_image_reply_t> xImage(xcb_get_image_reply(c, cookie, nullptr));
    if (xImage.isNull()) {
        return false;
    }

    // this operates on the assumption that X server default depth matches Qt's image format
    QImage image(xcb_get_image_data(xImage.data()), width, height, xcb_get_image_data_length(xImage.data()) / height, QImage::Format_ARGB32_Premultiplied);

    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            if (QColor(image.pixel(i, j)) != color) {
                return false;
            }
        }
    }
    return true;
}

bool isBlack()
{
    xcb_screen_t *screen = defaultScreen();
    const int width = screen->width_in_pixels;
    const int height = screen->height_in_pixels;

    return isColored(Qt::black, 0, 0, width, height);
}

xcb_atom_t screenLockerAtom()
{
    const QByteArray atomName = QByteArrayLiteral("_KDE_SCREEN_LOCKER");
    xcb_connection_t *c = QX11Info::connection();
    const auto cookie = xcb_intern_atom(c, false, atomName.length(), atomName.constData());
    ScopedCPointer<xcb_intern_atom_reply_t> atom(xcb_intern_atom_reply(c, cookie, nullptr));
    if (atom.isNull()) {
        return XCB_ATOM_NONE;
    }
    return atom->atom;
}

void LockWindowTest::initTestCase()
{
    QCoreApplication::setAttribute(Qt::AA_ForceRasterWidgets);
}

void LockWindowTest::testBlankScreen()
{
    // create and show a dummy window to ensure the background doesn't start as black
    QWidget dummy;
    dummy.setWindowFlags(Qt::X11BypassWindowManagerHint);
    QPalette p;
    p.setColor(QPalette::Window, Qt::red);
    dummy.setAutoFillBackground(true);
    dummy.setPalette(p);
    dummy.setGeometry(0, 0, 100, 100);
    dummy.show();
    xcb_flush(QX11Info::connection());

    // Lets wait till it gets shown
    QTest::qWait(1000);

    // Verify that red window is shown
    QVERIFY(isColored(Qt::red, 0, 0, 100, 100));

    ScreenLocker::X11Locker lockWindow;
    lockWindow.showLockWindow();

    // the screen used to be blanked once the first lock window gets mapped, so let's create one
    QWindow fakeWindow;
    fakeWindow.setFlags(Qt::X11BypassWindowManagerHint);
    // it's on purpose outside the visual area
    fakeWindow.setGeometry(-1, -1, 1, 1);
    fakeWindow.create();
    xcb_atom_t atom = screenLockerAtom();
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_connection_t *c = QX11Info::connection();
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, fakeWindow.winId(), atom, atom, 32, 0, nullptr);
    xcb_flush(c);
    fakeWindow.show();

    // give it time to be shown
    QTest::qWait(1000);

    // now lets try to get a screen grab and verify it's not yet black
    QVERIFY(!isBlack());

    // nowadays we need to pass the window id to the lock window
    lockWindow.addAllowedWindow(fakeWindow.winId());

    // give it time to be shown
    QTest::qWait(1000);

    // now lets try to get a screen grab and verify it's black
    QVERIFY(isBlack());
    dummy.hide();

    // destroying the fakeWindow should not remove the blanked screen
    fakeWindow.destroy();
    QTest::qWait(1000);
    QVERIFY(isBlack());

    // let's create another window and try to raise it above the lockWindow
    // using a QWidget to get proper content which won't be black
    QWidget widgetWindow;
    widgetWindow.setGeometry(10, 10, 100, 100);
    QPalette p1;
    p1.setColor(QPalette::Window, Qt::blue);
    widgetWindow.setAutoFillBackground(true);
    widgetWindow.setPalette(p1);
    widgetWindow.show();
    const uint32_t values[] = {XCB_STACK_MODE_ABOVE};
    xcb_configure_window(c, widgetWindow.winId(), XCB_CONFIG_WINDOW_STACK_MODE, values);
    xcb_flush(c);
    QTest::qWait(1000);
    QVERIFY(isBlack());

    lockWindow.hideLockWindow();
}

void LockWindowTest::testEmergencyShow()
{
    QWidget dummy;
    dummy.setWindowFlags(Qt::X11BypassWindowManagerHint);
    QPalette p;
    p.setColor(QPalette::Window, Qt::red);
    dummy.setAutoFillBackground(true);
    dummy.setPalette(p);
    dummy.setGeometry(0, 0, 100, 100);
    dummy.show();
    xcb_flush(QX11Info::connection());

    // Lets wait till it gets shown
    QTest::qWait(1000);

    // Verify that red window is shown
    QVERIFY(isColored(Qt::red, 0, 0, 100, 100));

    qputenv("KSLD_TESTMODE", QByteArrayLiteral("true"));

    ScreenLocker::X11Locker lockWindow;
    lockWindow.showLockWindow();

    QTest::qWait(1000);

    // the screen used to be blanked once the first lock window gets mapped, so let's create one
    QWindow fakeWindow;
    fakeWindow.setFlags(Qt::X11BypassWindowManagerHint);
    // it's on purpose outside the visual area
    fakeWindow.setGeometry(-1, -1, 1, 1);
    fakeWindow.create();
    xcb_atom_t atom = screenLockerAtom();
    QVERIFY(atom != XCB_ATOM_NONE);
    xcb_connection_t *c = QX11Info::connection();
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, fakeWindow.winId(), atom, atom, 32, 0, nullptr);
    xcb_flush(c);
    fakeWindow.show();

    // nowadays we need to pass the window id to the lock window
    lockWindow.addAllowedWindow(fakeWindow.winId());

    QTest::qWait(1000);

    lockWindow.emergencyShow();

    QTest::qWait(1000);
    xcb_flush(c);

    xcb_screen_t *screen = defaultScreen();
    const int width = screen->width_in_pixels;
    const int height = screen->height_in_pixels;

    const auto cookie = xcb_get_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, QX11Info::appRootWindow(), 0, 0, width, height, ~0);
    ScopedCPointer<xcb_get_image_reply_t> xImage(xcb_get_image_reply(c, cookie, nullptr));

    QVERIFY(!xImage.isNull());

    // this operates on the assumption that X server default depth matches Qt's image format
    QImage image(xcb_get_image_data(xImage.data()), width, height, xcb_get_image_data_length(xImage.data()) / height, QImage::Format_ARGB32_Premultiplied);

    bool isColored = false;
    int blackPixelCount = 0;
    int whitePixelCount = 0;

    // This verifies that,
    // - there is at least one white and one black pixel
    // - there is no other colored pixel
    QColor color;
    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            color = QColor(image.pixel(i, j));
            if (color == Qt::black) {
                blackPixelCount++;
            } else if (color == Qt::white) {
                whitePixelCount++;
            } else {
                isColored = true;
                break;
            }
        }
    }

    QVERIFY(!isColored);
    QVERIFY(blackPixelCount > 0);
    QVERIFY(whitePixelCount > 0);

    lockWindow.hideLockWindow();
}

QTEST_MAIN(LockWindowTest)
#include "x11lockertest.moc"
