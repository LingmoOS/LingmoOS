/*
 * Copyright (C) 2021 LingmoOS Team.
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

#include "mainwindow.h"
#include "processprovider.h"
#include "dockadaptor.h"

#include <QGuiApplication>
#include <QScreen>

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QMetaEnum>

#include <NETWM>
#include <KWindowSystem>
#include <KWindowEffects>

MainWindow::MainWindow(QQuickView *parent)
    : QQuickView(parent)
    , m_activity(Activity::self())
    , m_settings(DockSettings::self())
    , m_appModel(new ApplicationModel)
    , m_fakeWindow(nullptr)
    , m_trashManager(new TrashManager)
    , m_hideBlocked(false)
    , m_showTimer(new QTimer(this))
    , m_hideTimer(new QTimer(this))
{
    new DockAdaptor(this);

    installEventFilter(this);

    setDefaultAlphaBuffer(false);
    setColor(Qt::transparent);

    setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    // KWindowSystem::setOnDesktop(winId(), NET::OnAllDesktops);
    KWindowSystem::setType(winId(), NET::Dock);

    engine()->rootContext()->setContextProperty("appModel", m_appModel);
    engine()->rootContext()->setContextProperty("process", new ProcessProvider);
    engine()->rootContext()->setContextProperty("Settings", m_settings);
    engine()->rootContext()->setContextProperty("mainWindow", this);
    engine()->rootContext()->setContextProperty("trash", m_trashManager);

    setSource(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    setScreen(qApp->primaryScreen());
    setResizeMode(QQuickView::SizeRootObjectToView);
    initScreens();

    initSlideWindow();
    resizeWindow();
    onVisibilityChanged();

    m_showTimer->setSingleShot(true);
    m_showTimer->setInterval(200);
    connect(m_showTimer, &QTimer::timeout, this, [=] { setVisible(true); });

    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);
    connect(m_hideTimer, &QTimer::timeout, this, &MainWindow::onHideTimeout);

    // When the current window changes.
    connect(m_activity, &Activity::launchPadChanged, this, &MainWindow::onVisibilityChanged);
    connect(m_activity, &Activity::existsWindowMaximizedChanged, this, &MainWindow::onVisibilityChanged);

    // Screen change.
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &MainWindow::onPrimaryScreenChanged);
    connect(screen(), &QScreen::virtualGeometryChanged, this, &MainWindow::resizeWindow);
    connect(screen(), &QScreen::geometryChanged, this, &MainWindow::resizeWindow);

    connect(m_appModel, &ApplicationModel::countChanged, this, &MainWindow::resizeWindow);
    connect(m_settings, &DockSettings::directionChanged, this, &MainWindow::onPositionChanged);
    connect(m_settings, &DockSettings::iconSizeChanged, this, &MainWindow::onIconSizeChanged);
    connect(m_settings, &DockSettings::visibilityChanged, this, &MainWindow::onVisibilityChanged);
    connect(m_settings, &DockSettings::styleChanged, this, &MainWindow::resizeWindow);
}

MainWindow::~MainWindow()
{
}

void MainWindow::add(const QString &desktop)
{
    m_appModel->addItem(desktop);
}

void MainWindow::remove(const QString &desktop)
{
    m_appModel->removeItem(desktop);
}

bool MainWindow::pinned(const QString &desktop)
{
    return m_appModel->isDesktopPinned(desktop);
}

QRect MainWindow::primaryGeometry() const
{
    return geometry();
}

int MainWindow::direction() const
{
    return DockSettings::self()->direction();
}

int MainWindow::visibility() const
{
    return DockSettings::self()->visibility();
}

void MainWindow::setDirection(int direction)
{
    DockSettings::self()->setDirection(static_cast<DockSettings::Direction>(direction));
}

void MainWindow::setIconSize(int iconSize)
{
    DockSettings::self()->setIconSize(iconSize);
}

void MainWindow::setVisibility(int visibility)
{
    DockSettings::self()->setVisibility(static_cast<DockSettings::Visibility>(visibility));
}

int MainWindow::style() const
{
    return DockSettings::self()->style();
}

void MainWindow::setStyle(int style)
{
    DockSettings::self()->setStyle(static_cast<DockSettings::Style>(style));
}

void MainWindow::updateSize()
{
    resizeWindow();
}

QRect MainWindow::windowRect() const
{
    const QRect screenGeometry = screen()->geometry();
    const QRect availableGeometry = screen()->availableGeometry();

    bool isHorizontal = m_settings->direction() == DockSettings::Bottom;
    bool compositing = false;
    QQuickItem *item = qobject_cast<QQuickItem *>(rootObject());

    if (item) {
        compositing = item->property("compositing").toBool();
    }

    QSize newSize(0, 0);
    QPoint position(0, 0);
    int maxLength = isHorizontal ? screenGeometry.width() - m_settings->edgeMargins()
                                 : availableGeometry.height() - m_settings->edgeMargins();;

    // Add trash item.
    int appCount = m_appModel->rowCount() + 1;
    int iconSize = m_settings->iconSize();
    iconSize += iconSize * 0.1;
    int length = appCount * iconSize;
    int margins = compositing ? DockSettings::self()->edgeMargins() / 2 : 0;

    if (length >= maxLength) {
        iconSize = (maxLength - (maxLength % appCount)) / appCount;
        length = appCount * iconSize;
    }

    switch (m_settings->style()) {
    case DockSettings::Round: {
        switch (m_settings->direction()) {
        case DockSettings::Left:
            newSize = QSize(iconSize, length);
            position.setX(screenGeometry.x() + margins);
            // Handle the top statusbar.
            position.setY(availableGeometry.y() + (availableGeometry.height() - newSize.height()) / 2);
            break;
        case DockSettings::Bottom:
            newSize = QSize(length, iconSize);
            position.setX(screenGeometry.x() + (screenGeometry.width() - newSize.width()) / 2);
            position.setY(screenGeometry.y() + screenGeometry.height() - newSize.height() - margins);
            break;
        case DockSettings::Right:
            newSize = QSize(iconSize, length);
            position.setX(screenGeometry.x() + screenGeometry.width() - newSize.width() - margins);
            position.setY(availableGeometry.y() + (availableGeometry.height() - newSize.height()) / 2);
            break;
        default:
            break;
        }

        break;
    }
    case DockSettings::Straight: {
        switch (m_settings->direction()) {
        case DockSettings::Left:
            newSize = QSize(iconSize, screenGeometry.height());
            position.setX(screenGeometry.x());
            position.setY(screenGeometry.y());
            break;
        case DockSettings::Bottom:
            newSize = QSize(screenGeometry.width(), iconSize);
            position.setX(screenGeometry.x());
            position.setY(screenGeometry.y() + screenGeometry.height() - newSize.height());
            break;
        case DockSettings::Right:
            newSize = QSize(iconSize, screenGeometry.height());
            position.setX(screenGeometry.x() + screenGeometry.width() - newSize.width());
            position.setY(screenGeometry.y() + screenGeometry.height() - newSize.height());
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    return QRect(position, newSize);
}

void MainWindow::resizeWindow()
{
    setGeometry(windowRect());
    updateViewStruts();

    emit resizingFished();
}

void MainWindow::initScreens()
{
    switch (m_settings->direction()) {
// TODO
//    case DockSettings::Left:
//        setScreen(qGuiApp->screens().first());
//        break;
//    case DockSettings::Right:
//        setScreen(qGuiApp->screens().last());
//        break;
//    case DockSettings::Bottom:
//        setScreen(qGuiApp->primaryScreen());
//        break;
    default:
        setScreen(qGuiApp->primaryScreen());
        break;
    }

    if (m_fakeWindow) {
        m_fakeWindow->setScreen(screen());
        m_fakeWindow->updateGeometry();
    }
}

void MainWindow::initSlideWindow()
{
    KWindowEffects::SlideFromLocation location = KWindowEffects::NoEdge;

    if (m_settings->direction() == DockSettings::Left)
        location = KWindowEffects::LeftEdge;
    else if (m_settings->direction() == DockSettings::Right)
        location = KWindowEffects::RightEdge;
    else if (m_settings->direction() == DockSettings::Bottom)
        location = KWindowEffects::BottomEdge;

    KWindowEffects::slideWindow(winId(), location);
}

void MainWindow::updateViewStruts()
{
    bool compositing = false;
    QQuickItem *item = qobject_cast<QQuickItem *>(rootObject());

    if (item) {
        compositing = item->property("compositing").toBool();
    }

    if (m_settings->visibility() == DockSettings::AlwaysShow || m_activity->launchPad()) {
        XWindowInterface::instance()->setViewStruts(this, m_settings->direction(), geometry(), compositing);
    } else {
        clearViewStruts();
    }
}

void MainWindow::clearViewStruts()
{
    XWindowInterface::instance()->clearViewStruts(this);
}

void MainWindow::createFakeWindow()
{
    if (!m_fakeWindow) {
        m_fakeWindow = new FakeWindow;
        m_fakeWindow->setScreen(screen());
        m_fakeWindow->updateGeometry();

        connect(m_fakeWindow, &FakeWindow::containsMouseChanged, this, [=](bool contains) {
            switch (m_settings->visibility()) {
            case DockSettings::AlwaysHide:
            case DockSettings::IntellHide:{
                if (contains) {
                    m_hideTimer->stop();

                    // reionwong: The mouse is moved to fakewindow,
                    // if the dock is not displayed,
                    // it will start to display.
                    if (!isVisible() && !m_showTimer->isActive()) {
                        m_showTimer->start();
                    }
                } else {
                    if (!m_hideBlocked)
                        m_hideTimer->start();
                }

                break;
            }
            default:
                break;
            }
        });
    }
}

void MainWindow::deleteFakeWindow()
{
    if (m_fakeWindow) {
        // removeEventFilter(this);
        disconnect(m_fakeWindow);
        m_fakeWindow->deleteLater();
        m_fakeWindow = nullptr;
    }
}

void MainWindow::onPrimaryScreenChanged(QScreen *screen)
{
    initScreens();
    setScreen(screen);
    resizeWindow();
}

void MainWindow::onPositionChanged()
{
    initScreens();

    if (m_settings->visibility() == DockSettings::AlwaysHide ||
            m_settings->visibility() == DockSettings::IntellHide) {
        setVisible(false);
        initSlideWindow();
        // Setting geometry needs to be displayed, otherwise it will be invalid.
        setVisible(true);
        setGeometry(windowRect());
        updateViewStruts();

        m_hideTimer->start();
    } else if (m_settings->visibility() == DockSettings::AlwaysShow) {
        setVisible(false);
        initSlideWindow();
        setVisible(true);
        setGeometry(windowRect());
        updateViewStruts();
    }

    emit directionChanged();
}

void MainWindow::onIconSizeChanged()
{
    setGeometry(windowRect());
    updateViewStruts();

    emit iconSizeChanged();
}

void MainWindow::onVisibilityChanged()
{
    emit visibilityChanged();

    if (m_activity->launchPad()) {
        m_hideTimer->stop();
        clearViewStruts();
        setVisible(true);
        return;
    }

    // Always show
    // Must remain displayed when launchpad is opened.
    if (m_settings->visibility() == DockSettings::AlwaysShow) {
        m_hideTimer->stop();

        setGeometry(windowRect());
        setVisible(true);
        updateViewStruts();

        // Delete fakewindow
        if (m_fakeWindow) {
            deleteFakeWindow();
        }
    }

    if (m_settings->visibility() == DockSettings::IntellHide) {
        clearViewStruts();
        setGeometry(windowRect());

        if (m_activity->existsWindowMaximized() && !m_hideBlocked) {
            setVisible(false);
        } else {
            setVisible(true);
        }

        if (!m_fakeWindow)
            createFakeWindow();
    }

    // Always hide
    if (m_settings->visibility() == DockSettings::AlwaysHide) {
        clearViewStruts();
        setGeometry(windowRect());
        setVisible(m_hideBlocked);

        // Create
        if (!m_fakeWindow)
            createFakeWindow();
    }
}

void MainWindow::onHideTimeout()
{
    if (m_activity->launchPad())
        return;

    if (m_settings->visibility() == DockSettings::IntellHide
            && !m_activity->existsWindowMaximized()) {
        return;
    }

    setVisible(false);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::Enter:
        if (m_fakeWindow)
            m_hideTimer->stop();
        m_hideBlocked = true;
        break;
    case QEvent::Leave:
        if (m_fakeWindow)
            m_hideTimer->start();
        m_hideBlocked = false;
        break;
    case QEvent::DragEnter:
    case QEvent::DragMove:
        if (m_fakeWindow)
            m_hideTimer->stop();
        break;
    case QEvent::DragLeave:
    case QEvent::Drop:
        if (m_fakeWindow)
            m_hideTimer->stop();
        break;
    default:
        break;
    }

    return QQuickView::eventFilter(obj, e);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    emit primaryGeometryChanged();

    QQuickView::resizeEvent(e);
}
