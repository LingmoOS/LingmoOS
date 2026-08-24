/*
 * Copyright (C) 2021 CuteOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "application.h"

// Qt Core
#include <QAbstractNativeEventFilter>
#include <QScreen>
#include <QX11Info>
#include <QEvent>

// Qt Quick
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>

// X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include "fixx11h.h"

// Xcb
#include <xcb/xcb.h>

// this is usable to fake a "screensaver" installation for testing
// *must* be "0" for every public commit!
#define TEST_SCREENSAVER 0

class FocusOutEventFilter : public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long int *result) override {
        Q_UNUSED(result)
        if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
            return false;
        }

        xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(message);
        if ((event->response_type & ~0x80) == XCB_FOCUS_OUT) {
            return true;
        }

        return false;
    }
};

Application::Application(int &argc, char **argv)
    : QGuiApplication(argc, argv)
    , m_authenticator(new Authenticator(AuthenticationMode::Direct, this))
{
    // It's a queued connection to give the QML part time to eventually execute code connected to Authenticator::succeeded if any
    connect(m_authenticator, &Authenticator::succeeded, this, &Application::onSucceeded, Qt::QueuedConnection);

    installEventFilter(this);

    // Screens
    connect(this, &Application::screenAdded, this, &Application::onScreenAdded);
    connect(this, &Application::screenRemoved, this, &Application::desktopResized);

    if (QX11Info::isPlatformX11()) {
        installNativeEventFilter(new FocusOutEventFilter);
    }
}

Application::~Application()
{
    // workaround QTBUG-55460
    // will be fixed when themes port to QQC2
    for (auto view : qAsConst(m_views)) {
        if (QQuickItem *focusItem = view->activeFocusItem()) {
            focusItem->setFocus(false);
        }
    }
    qDeleteAll(m_views);
}

void Application::initialViewSetup()
{
    for (QScreen *screen : screens()) {
        connect(screen, &QScreen::geometryChanged, this, [this, screen](const QRect &geo) {
            screenGeometryChanged(screen, geo);
        });
    }

    desktopResized();
}

void Application::desktopResized()
{
    const int nScreens = screens().count();
    // remove useless views and savers
    while (m_views.count() > nScreens) {
        m_views.takeLast()->deleteLater();
    }

    // extend views and savers to current demand
    for (int i = m_views.count(); i < nScreens; ++i) {
        // create the view
        auto *view = new QQuickView;
        view->create();

        // engine stuff
        QQmlContext *context = view->engine()->rootContext();
        context->setContextProperty(QStringLiteral("authenticator"), m_authenticator);

        view->setSource(QUrl("qrc:/qml/LockScreen.qml"));
        view->setResizeMode(QQuickView::SizeRootObjectToView);

        view->setColor(Qt::black);
        auto screen = QGuiApplication::screens()[i];
        view->setGeometry(screen->geometry());

        if (!m_testing) {
            if (QX11Info::isPlatformX11()) {
                view->setFlags(Qt::X11BypassWindowManagerHint);
            } else {
                view->setFlags(Qt::FramelessWindowHint);
            }
        }

        // overwrite the factory set by kdeclarative
        // auto oldFactory = view->engine()->networkAccessManagerFactory();
        // view->engine()->setNetworkAccessManagerFactory(nullptr);
        // delete oldFactory;
        // view->engine()->setNetworkAccessManagerFactory(new NoAccessNetworkAccessManagerFactory);

        view->setGeometry(screen->geometry());

        connect(view, &QQuickView::frameSwapped, this, [=] { markViewsAsVisible(view); }, Qt::QueuedConnection);

        m_views << view;
    }

    // update geometry of all views and savers
    for (int i = 0; i < nScreens; ++i) {
        auto *view = m_views.at(i);
        auto screen = QGuiApplication::screens()[i];
        view->setScreen(screen);

        // on Wayland we may not use fullscreen as that puts all windows on one screen
        if (m_testing || QX11Info::isPlatformX11()) {
            view->show();
        } else {
            view->showFullScreen();
        }

        view->raise();
    }
}

void Application::onScreenAdded(QScreen *screen)
{
    // Lambda connections can not have uniqueness constraints, ensure
    // geometry change signals are only connected once
    connect(screen, &QScreen::geometryChanged, this, [this, screen](const QRect &geo) {
        screenGeometryChanged(screen, geo);
    });

    desktopResized();
}

void Application::onSucceeded()
{
    QQuickView *mainView = nullptr;

    // 寻找主屏幕的 view
    for (int i = 0; i < m_views.size(); ++i) {
        if (m_views.at(i)->screen() == QGuiApplication::primaryScreen()) {
            mainView = m_views.at(i);
            break;
        }
    }

    if (mainView) {
        QVariantAnimation *ani = new QVariantAnimation;

        connect(ani, &QVariantAnimation::valueChanged, [mainView] (const QVariant &value) {
            mainView->setY(value.toInt());
        });

        connect(ani, &QVariantAnimation::finished, this, [=] {
            QCoreApplication::exit();
        });

        ani->setDuration(500);
        ani->setEasingCurve(QEasingCurve::OutSine);
        ani->setStartValue(mainView->geometry().y());
        ani->setEndValue(mainView->geometry().y() + -mainView->geometry().height());
        ani->start();
    } else {
        QCoreApplication::exit();
    }
}

void Application::getFocus()
{
    QWindow *activeScreen = getActiveScreen();

    if (!activeScreen) {
        return;
    }

    // this loop is required to make the qml/graphicsscene properly handle the shared keyboard input
    // ie. "type something into the box of every greeter"
    for (QQuickView *view : qAsConst(m_views)) {
        if (!m_testing) {
            view->setKeyboardGrabEnabled(true); // TODO - check whether this still works in master!
        }
    }

    // activate window and grab input to be sure it really ends up there.
    // focus setting is still required for proper internal QWidget state (and eg. visual reflection)
    if (!m_testing) {
        activeScreen->setKeyboardGrabEnabled(true); // TODO - check whether this still works in master!
    }

    activeScreen->requestActivate();
}

void Application::markViewsAsVisible(QQuickView *view)
{
    disconnect(view, &QQuickWindow::frameSwapped, this, nullptr);
    QQmlProperty showProperty(view->rootObject(), QStringLiteral("viewVisible"));
    showProperty.write(true);

    // random state update, actually rather required on init only
    QMetaObject::invokeMethod(this, "getFocus", Qt::QueuedConnection);
}

bool Application::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != this && event->type() == QEvent::Show) {
        QQuickView *view = nullptr;
        for (QQuickView *v : qAsConst(m_views)) {
            if (v == obj) {
                view = v;
                break;
            }
        }
        if (view && view->winId() && QX11Info::isPlatformX11()) {
            // showing greeter view window, set property
            static Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);
            XChangeProperty(QX11Info::display(), view->winId(), tag, tag, 32, PropModeReplace, nullptr, 0);
        }
        // no further processing
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress && QX11Info::isPlatformX11()) {
        if (getActiveScreen()) {
            getActiveScreen()->requestActivate();
        }
        return false;
    }

    if (event->type() == QEvent::KeyPress) { // react if saver is visible
        shareEvent(event, qobject_cast<QQuickView *>(obj));
        return false; // we don't care
    } else if (event->type() == QEvent::KeyRelease) { // conditionally reshow the saver
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() != Qt::Key_Escape) {
            shareEvent(event, qobject_cast<QQuickView *>(obj));
            return false; // irrelevant
        }
        return true; // don't pass
    }

    return false;
}

QWindow *Application::getActiveScreen()
{
    QWindow *activeScreen = nullptr;

    if (m_views.isEmpty()) {
        return activeScreen;
    }

    for (QQuickView *view : qAsConst(m_views)) {
        if (view->geometry().contains(QCursor::pos())) {
            activeScreen = view;
            break;
        }
    }
    if (!activeScreen) {
        activeScreen = m_views.first();
    }

    return activeScreen;
}

void Application::shareEvent(QEvent *e, QQuickView *from)
{
    // from can be NULL any time (because the parameter is passed as qobject_cast)
    // m_views.contains(from) is atm. supposed to be true but required if any further
    // QQuickView are added (which are not part of m_views)
    // this makes "from" an optimization (nullptr check aversion)
    if (from && m_views.contains(from)) {
        // NOTICE any recursion in the event sharing will prevent authentication on multiscreen setups!
        // Any change in regarded event processing shall be tested thoroughly!
        removeEventFilter(this); // prevent recursion!
        const bool accepted = e->isAccepted(); // store state
        for (QQuickView *view : qAsConst(m_views)) {
            if (view != from) {
                QCoreApplication::sendEvent(view, e);
                e->setAccepted(accepted);
            }
        }
        installEventFilter(this);
    }
}

void Application::screenGeometryChanged(QScreen *screen, const QRect &geo)
{
    // We map screens() to m_views by index and Qt is free to
    // reorder screens, so pointer to pointer connections
    // may not remain matched by index, perform index
    // mapping in the change event itself
    const int screenIndex = QGuiApplication::screens().indexOf(screen);
    if (screenIndex < 0) {
        qWarning() << "Screen not found, not updating geometry" << screen;
        return;
    }

    if (screenIndex >= m_views.size()) {
        qWarning() << "Screen index out of range, not updating geometry" << screenIndex;
        return;
    }

    QQuickView *view = m_views[screenIndex];
    view->setGeometry(geo);
}
