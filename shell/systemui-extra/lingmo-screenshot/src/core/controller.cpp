/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*
* This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "controller.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/utils/confighandler.h"
#include "src/config/configwindow.h"
#include "src/widgets/capture/capturebutton.h"
#include "src/utils/systemnotification.h"
#include "src/utils/screengrabber.h"
#include "my_qt.h"
#include "src/utils/systemnotification.h"
#include "kwidget.h"

// #include "Logger.h"
#ifdef Q_OS_WIN
#include "src/core/globalshortcutfilter.h"
#endif

// Controller is the core component of lingmo-screenshot, creates the trayIcon and
// launches the capture widget

Controller::Controller() : m_captureWindow(nullptr)
{
    qApp->setQuitOnLastWindowClosed(false);

    // init tray icon
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (!ConfigHandler().disabledTrayIconValue()) {
        enableTrayIcon();
    }
#elif defined(Q_OS_WIN)
    enableTrayIcon();

    GlobalShortcutFilter *nativeFilter = new GlobalShortcutFilter(this);
    qApp->installNativeEventFilter(nativeFilter);
    connect(nativeFilter, &GlobalShortcutFilter::printPressed,
            this, [this](){
        this->requestCapture(CaptureRequest(CaptureRequest::GRAPHICAL_MODE));
    });
#endif

//    QString StyleSheet = CaptureButton::globalStyleSheet();
//    qApp->setStyleSheet(StyleSheet);
}

Controller *Controller::getInstance()
{
    static Controller c;
    return &c;
}

void Controller::enableExports()
{
    connect(this, &Controller::captureTaken,
            this, &Controller::handleCaptureTaken);
    connect(this, &Controller::captureFailed,
            this, &Controller::handleCaptureFailed);
}

void Controller::requestCapture(const CaptureRequest &request)
{
    uint id = request.id();
    m_requestMap.insert(id, request);

    QGSettings *screen = new QGSettings("org.lingmo.screenshot");
    QString screenshot = screen->get("screenshot").toString();
    switch (request.captureMode()) {
    case CaptureRequest::FULLSCREEN_MODE:
        if (screenshot.compare("true") == 0) {
            doLater(request.delay(), this, [this, id](){
                this->startFullscreenCapture(id);
            });
        } else {
            disableScreenCut();
        }
        break;
    case CaptureRequest::SCREEN_MODE:
        if (screenshot.compare("true") == 0) {
            int &&number = request.data().toInt();
            doLater(request.delay(), this, [this, id, number](){
                this->startScreenGrab(id, number);
            });
        } else {
            disableScreenCut();
        }
        break;
    case CaptureRequest::GRAPHICAL_MODE:
        if (screenshot.compare("true") == 0) {
            QString &&path = request.path();
            doLater(request.delay(), this, [this, id, path](){
                this->startVisualCapture(id, path);
            });
        } else {
            disableScreenCut();
        }
        break;
    default:
        emit captureFailed(id);
        break;
    }
}

// creation of a new capture in GUI mode
void Controller::startVisualCapture(const uint id, const QString &forcedSavePath)
{
#ifdef SUPPORT_LINGMO
    QGSettings *screen = new QGSettings("org.lingmo.screenshot");
// if (!screen) {
// Logger::LogInfo("[Controller::startVisualCapture] " + Logger::tr("not support org.lingmo.screenshot..."));
// exit(1);
// }
    QString screenshot = screen->get("screenshot").toString();
#endif
    if (!m_captureWindow) {
#ifdef SUPPORT_LINGMO
        if (screenshot.compare("true") == 0) {
#endif
        QWidget *modalWidget = nullptr;
        do {
            modalWidget = qApp->activeModalWidget();
            if (modalWidget) {
                modalWidget->close();
                modalWidget->deleteLater();
            }
        } while (modalWidget);

        m_captureWindow = new CaptureWidget(id, forcedSavePath);
        QTimer::singleShot(100,this,[=](){
            kdk::WindowManager::setWindowLayer(m_captureWindow->windowHandle(),kdk::WindowLayer::OnScreenDisplay);
        });
        // m_captureWindow = new CaptureWidget(id, forcedSavePath, false); // debug
        connect(m_captureWindow, &CaptureWidget::captureFailed,
                this, &Controller::captureFailed);
        connect(m_captureWindow, &CaptureWidget::captureTaken,
                this, &Controller::captureTaken);
        connect(m_captureWindow, &CaptureWidget::captureExit,
                this, &Controller::captureExit);
        connect(m_captureWindow, &CaptureWidget::captureCopy,
                this, &Controller::captureCopy);
#ifdef SUPPORT_LINGMO
    } else {
        disconnect(m_captureWindow, &CaptureWidget::captureFailed,
                   this, &Controller::captureFailed);
        disconnect(m_captureWindow, &CaptureWidget::captureTaken,
                   this, &Controller::captureTaken);
        disableScreenCut();
    }
#endif
#ifdef Q_OS_WIN
        m_captureWindow->show();
#else
#ifdef SUPPORT_LINGMO
        if (screenshot.compare("true") == 0) {
#endif
        m_captureWindow->showFullScreen();
#ifdef SUPPORT_LINGMO
    }
#endif
        // m_captureWindow->show(); // Debug
#endif
    } else {
        emit captureFailed(id);
    }
}

void Controller::startScreenGrab(const uint id, const int screenNumber)
{
    bool ok = true;
    int n = screenNumber;

    if (n < 0) {
        QPoint globalCursorPos = QCursor::pos();
        n = qApp->desktop()->screenNumber(globalCursorPos);
    }
    QPixmap p(ScreenGrabber().grabScreen(n, ok));
    if (ok) {
        emit captureTaken(id, p);
    } else {
        emit captureFailed(id);
    }
}

// creation of the configuration window
void Controller::openConfigWindow()
{
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
        m_configWindow->show();
    }
}

void Controller::openShotCutWindow()
{
    if (m_shortCutWindow) {
        delete m_shortCutWindow;
    }
    if (!m_shortCutWindow) {
        m_shortCutWindow = new ShortCutWidget();
        kdk::LingmoUIStyleHelper::self()->removeHeader(m_shortCutWindow);
        m_shortCutWindow->show();
        m_shortCutWindow->move(
            ((qApp->desktop()->screenGeometry().width()-m_shortCutWindow->width())/2),
            ((qApp->desktop()->screenGeometry().height()-m_shortCutWindow->height())
             /2));
        kdk::WindowManager::setGeometry(m_shortCutWindow->windowHandle(),m_shortCutWindow->geometry());
    }
}

// creation of the window of information
void Controller::openInfoWindow()
{
//    if (m_infoWindow) {
//        delete m_infoWindow;
//    }
//    if (!m_infoWindow) {
//        m_infoWindow = new infoWidget();
//        m_infoWindow->move(
//            ((qApp->desktop()->screenGeometry().width()-m_infoWindow->width())/2),
//            ((qApp->desktop()->screenGeometry().height()-m_infoWindow->height())
//             /2));
//        kdk::LingmoUIStyleHelper::self()->removeHeader(m_infoWindow);
//        kdk::WindowManager::setGeometry(m_infoWindow->windowHandle(),m_infoWindow->geometry());
//        m_infoWindow->show();
//    }
    if (m_aboutdialog) {
        delete m_aboutdialog;
    }
    if (!m_aboutdialog) {
        m_aboutdialog = new AboutDialog();
        kdk::LingmoUIStyleHelper::self()->removeHeader(m_aboutdialog);
        m_aboutdialog->show();
        m_aboutdialog->move(
                    ((qApp->desktop()->geometry().width()-m_aboutdialog->width())/2),
                    ((qApp->desktop()->geometry().height()-m_aboutdialog->height())/2));
        kdk::WindowManager::setGeometry(m_aboutdialog->windowHandle(),m_aboutdialog->geometry());
    }
}

void Controller::openLauncherWindow()
{
    QGSettings *screen = new QGSettings("org.lingmo.screenshot");
    QString screenshot = screen->get("screenshot").toString();
    if (m_launcherWindow) {
        delete m_launcherWindow;
    }
    if (!m_launcherWindow) {
        m_launcherWindow = new CaptureLauncher();
        kdk::LingmoUIStyleHelper::self()->removeHeader(m_launcherWindow);

        if (screenshot.compare("true") == 0) {
            m_launcherWindow->show();
            m_launcherWindow->move(((qApp->desktop()->geometry().width()
                                     -m_launcherWindow->width())/2),
                                   ((qApp->desktop()->geometry().height()
                                     -m_launcherWindow->height())/2));
            kdk::WindowManager::setGeometry(m_launcherWindow->windowHandle(),m_launcherWindow->geometry());
        } else
            disableScreenCut();
    }
}

void Controller::enableTrayIcon()
{
    if (m_trayIcon) {
        return;
    }
    ConfigHandler().setDisabledTrayIcon(false);
    // Start the screenshot from the start menu and add a delay by zhanghanhuan
    // doLater(400, this, [this](){ this->startVisualCapture(); });
    QAction *captureAction = new QAction(tr("&Take Screenshot"), this);
    connect(captureAction, &QAction::triggered, this, [this](){
        // Wait 400 ms to hide the QMenu
        doLater(400, this, [this](){
            this->startVisualCapture();
        });
    });
    QAction *launcherAction = new QAction(tr("&Open Screenshot Option"), this);
    connect(launcherAction, &QAction::triggered, this,
            &Controller::openLauncherWindow);
    QAction *configAction = new QAction(tr("&Configuration"), this);
    connect(configAction, &QAction::triggered, this,
            &Controller::openConfigWindow);
    QAction *infoAction = new QAction(tr("&About"), this);
    connect(infoAction, &QAction::triggered, this,
            &Controller::openInfoWindow);
    QAction *quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, this,
            &Controller::deletePidfile);
    connect(quitAction, &QAction::triggered, qApp,
            &QCoreApplication::quit);

    QAction *shortcutAction = new QAction(tr("&ShortCut"), this);
    connect(shortcutAction, &QAction::triggered, this,
            &Controller::openShotCutWindow);

    QMenu *trayIconMenu = new QMenu();
    trayIconMenu->addAction(captureAction);
    trayIconMenu->addAction(launcherAction);
    trayIconMenu->addSeparator();
    // trayIconMenu->addAction(configAction);
    trayIconMenu->addAction(shortcutAction);
    trayIconMenu->addAction(infoAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon();
    m_trayIcon->setToolTip(tr("Screenshot"));
    m_trayIcon->setContextMenu(trayIconMenu);
    QIcon trayicon = QIcon::fromTheme("lingmo-screenshot");
    // QIcon trayicon = QIcon("/usr/share/icons/lingmo-icon-theme-default/128x128/apps/lingmo-screenshot.png");
    m_trayIcon->setIcon(trayicon);

    auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason r){
                                 if (r == QSystemTrayIcon::Trigger) {
                                     startVisualCapture();
                                 }
                             };
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, trayIconActivated);
    m_trayIcon->show();
}

void Controller::disableTrayIcon()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_trayIcon) {
        m_trayIcon->deleteLater();
    }
    ConfigHandler().setDisabledTrayIcon(true);
#endif
}

void Controller::sendTrayNotification(
    const QString &text, const QString &title, const int timeout)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, text, QSystemTrayIcon::Information, timeout);
    }
}

void Controller::updateConfigComponents()
{
    if (m_configWindow) {
        m_configWindow->updateChildren();
    }
}

void Controller::deletePidfile()
{
    QFile file("/tmp/screenshot.pid");
    if (file.exists()) {
        file.remove();
    }
}

void Controller::startFullscreenCapture(const uint id)
{
    bool ok = true;
    QPixmap p(ScreenGrabber().grabEntireDesktop(ok));
    if (ok) {
        emit captureTaken(id, p);
    } else {
        emit captureFailed(id);
    }
}

void Controller::handleCaptureTaken(uint id, QPixmap p)
{
    auto it = m_requestMap.find(id);
    if (it != m_requestMap.end()) {
        it.value().exportCapture(p);
        m_requestMap.erase(it);
    }
    if (ConfigHandler().closeAfterScreenshotValue()) {
        QApplication::quit();
    }
}

void Controller::handleCaptureFailed(uint id)
{
    m_requestMap.remove(id);

    if (ConfigHandler().closeAfterScreenshotValue()) {
        QApplication::quit();
    }
}

void Controller::emitSCreenSaveLock()
{
    m_onScreenLock = true;
}

void Controller::emitSCreenSaveUnLock()
{
    m_onScreenLock = false;
}

void Controller::doLater(int msec, QObject *receiver, lambda func)
{
    DelayWidget *delaywidget = new DelayWidget();
    delaywidget->hide();
    kdk::LingmoUIStyleHelper::self()->removeHeader(delaywidget);
    QTimer *timer = new QTimer(receiver);
    if (msec > 1000) {
        QObject::connect(timer, &QTimer::timeout, this,
                         [ msec, timer, delaywidget, func]()
        {
            static bool isRestart = true;
            static int count;
            if (isRestart == true) {
                count = msec/1000;
            }
            if (count == 0) {
                timer->stop();
                timer->deleteLater();
                count = msec/1000;
                isRestart = true;
                func();
            } else if (count > 0) {
                delaywidget->show();
                delaywidget->move(((qApp->desktop()->geometry().width()
                                    -delaywidget->width())/2),
                                  ((qApp->desktop()->geometry().height()
                                    -delaywidget->height())/2));
                kdk::WindowManager::setGeometry(delaywidget->windowHandle(),delaywidget->geometry());
                delaywidget->updateText(count -1);
                isRestart = false;
                count--;
                if (count == 0) {
                    delaywidget->hide();
                    delaywidget->deleteLater();
                }
            }
        });
        timer->start(1000);
    } else {
        QObject::connect(timer, &QTimer::timeout, receiver,
                         [timer, func](){
            func();
            timer->deleteLater();
        });
        timer->setInterval(msec);
        timer->start();
    }
}

void Controller::disableScreenCut()
{
    SystemNotification().sendMessage(tr("Unable to use lingmo-screenshot"));
}

bool Controller::getScreenSaveLockStatus()
{
    return m_onScreenLock;
}

void Controller::setScreenSaveLockStatus(bool status)
{
    m_onScreenLock = status;
}
