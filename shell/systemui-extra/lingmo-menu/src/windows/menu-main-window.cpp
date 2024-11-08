/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
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
 *
 */

#include "menu-main-window.h"
#include "settings.h"
#include "context-menu-manager.h"

#include <QGuiApplication>
#include <QQuickItem>
#include <QX11Info>
#include <QQmlContext>
#include <QPoint>
#include <KWindowSystem>
#include <KWindowEffects>
#include <QPainterPath>
#include <QTimer>
#include <QCursor>

#define LINGMO_PANEL_SETTING          "org.lingmo.panel.settings"
#define LINGMO_PANEL_POSITION_KEY     "panelposition"
#define LINGMO_PANEL_SIZE_KEY         "panelsize"

namespace LingmoUIMenu {

void WindowModule::defineModule(const char *uri, int versionMajor, int versionMinor)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    qmlRegisterRevision<QWindow, 3>(uri, versionMajor, versionMinor);
    qmlRegisterRevision<QQuickWindow, 2>(uri, versionMajor, versionMinor);
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    qmlRegisterRevision<QWindow, 2>(uri, versionMajor, versionMinor);
    qmlRegisterRevision<QQuickWindow, 2>(uri, versionMajor, versionMinor);
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    qmlRegisterRevision<QWindow, 2>(uri, versionMajor, versionMinor);
    qmlRegisterRevision<QQuickWindow, 1>(uri, versionMajor, versionMinor);
#else
    qmlRegisterRevision<QWindow, 1>(uri, versionMajor, versionMinor);
#endif
#endif
#endif
}

void WindowHelper::windowBlur(QWindow *window, bool enable, const QRegion &region)
{
    if (!window) {
        return;
    }
    //use KWindowEffects
    KWindowEffects::enableBlurBehind(window, enable, region);
}

void WindowHelper::setRegion(QWindow *window, qreal x, qreal y, qreal w, qreal h, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(x, y, w, h, radius, radius);

    windowBlur(window, true, QRegion(path.toFillPolygon().toPolygon()));
}

// ====== WindowGeometryHelper ====== //
WindowGeometryHelper::WindowGeometryHelper(QObject *parent) : QObject(parent)
{
    initPanelSetting();
//    initScreenMonitor();
    updateGeometry();

    connect(MenuSetting::instance(), &MenuSetting::changed, this, [this] (const QString& key) {
        if (key == MENU_WIDTH || key == MENU_HEIGHT) {
            updateGeometry();
        }
    });
}

void WindowGeometryHelper::updateGeometry()
{
    if (!m_primaryScreen) {
        return;
    }

    QRect screenRect = m_primaryScreen->geometry(), normalMaskRect, fullRect;
    int width = MenuSetting::instance()->get(MENU_WIDTH).toInt();
    int height = MenuSetting::instance()->get(MENU_HEIGHT).toInt();
    int margin = MenuSetting::instance()->get(MENU_MARGIN).toInt();

    //上: 1, 下: 0, 左: 2, 右: 3
    switch (m_panelPos) {
        default:
        case 0: {
            fullRect.setTopLeft(screenRect.topLeft());
            fullRect.setSize({screenRect.width(), screenRect.height() - m_panelSize});

            QSize normalSize(qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height));
            normalMaskRect.setTopLeft({margin, fullRect.height() - normalSize.height() - margin});
            normalMaskRect.setSize(normalSize);
            break;
        }
        case 1: {
            fullRect.setTopLeft({screenRect.x(), screenRect.y() + m_panelSize});
            fullRect.setSize({screenRect.width(), screenRect.height() - m_panelSize});

            normalMaskRect.setTopLeft({margin, margin});
            normalMaskRect.setSize({qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height)});
            break;
        }
        case 2: {
            fullRect.setTopLeft({screenRect.x() + m_panelSize, screenRect.y()});
            fullRect.setSize({screenRect.width() - m_panelSize, screenRect.height()});

            normalMaskRect.setTopLeft({margin, margin});
            normalMaskRect.setSize({qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height)});
            break;
        }
        case 3: {
            fullRect.setTopLeft(screenRect.topLeft());
            fullRect.setSize({screenRect.width() - m_panelSize, screenRect.height()});

            QSize normalSize(qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height));
            normalMaskRect.setTopLeft({fullRect.width() - normalSize.width() - margin, margin});
            normalMaskRect.setSize(normalSize);
            break;
        }
    }

    m_normalGeometry = normalMaskRect;
    m_fullScreenGeometry = fullRect;

    Q_EMIT geometryChanged();
}

void WindowGeometryHelper::initPanelSetting()
{
    if (!MenuSetting::instance()->get(FOLLOW_LINGMO_PANEL).toBool()) {
        return;
    }

    const QByteArray id(LINGMO_PANEL_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *setting = new QGSettings(id, QByteArray(), this);

        QStringList keys = setting->keys();
        if (keys.contains(LINGMO_PANEL_POSITION_KEY)) {
            m_panelPos = setting->get(LINGMO_PANEL_POSITION_KEY).toInt();
        }
        if (keys.contains(LINGMO_PANEL_SIZE_KEY)) {
            m_panelSize = setting->get(LINGMO_PANEL_SIZE_KEY).toInt();
        }

        connect(setting, &QGSettings::changed, this, [this, setting] (const QString& key) {
            if (key == LINGMO_PANEL_POSITION_KEY || key == LINGMO_PANEL_SIZE_KEY) {
                m_panelPos = setting->get(LINGMO_PANEL_POSITION_KEY).toInt();
                m_panelSize = setting->get(LINGMO_PANEL_SIZE_KEY).toInt();
                updateGeometry();
            }
        });
    }
}

void WindowGeometryHelper::initScreenMonitor()
{
    updatePrimaryScreen(QGuiApplication::primaryScreen());
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, [this] (QScreen *screen) {
        updatePrimaryScreen(screen);
        updateGeometry();
    });
}

void WindowGeometryHelper::updatePrimaryScreen(QScreen *screen)
{
    if (!screen) {
        return;
    }

    if (m_primaryScreen) {
        m_primaryScreen->disconnect(this);
    }

    m_primaryScreen = screen;
    connect(m_primaryScreen, &QScreen::geometryChanged, this, &WindowGeometryHelper::updateGeometry);
}

const QRect &WindowGeometryHelper::fullScreenGeometry()
{
    return m_fullScreenGeometry;
}

const QRect &WindowGeometryHelper::normalGeometry()
{
    return m_normalGeometry;
}

const int WindowGeometryHelper::getPanelPos()
{
    return m_panelPos;
}

//======MenuWindow======//
MenuWindow::MenuWindow(QWindow *parent) : QQuickView(parent)
{
    init();
}

MenuWindow::MenuWindow(QQmlEngine *engine, QWindow *parent)
    : QQuickView(engine, parent)
{
    init();
}

void MenuWindow::init()
{
    initPanelSetting();

    connect(MenuSetting::instance(), &MenuSetting::changed, this, [this] (const QString& key) {
        if (key == MENU_WIDTH || key == MENU_HEIGHT) {
            updateGeometry();
        }
    });
    setScreen(QGuiApplication::primaryScreen());
    connect(screen(), &QScreen::geometryChanged, this, &MenuWindow::updateGeometry);
    connect(qGuiApp, &QGuiApplication::layoutDirectionChanged, this, &MenuWindow::updateGeometry);

    setTitle(QCoreApplication::applicationName());
    setResizeMode(SizeRootObjectToView);
    setColor("transparent");

    //setWindowState(Qt::WindowMaximized);
    setFlags(flags() | Qt::Window | Qt::FramelessWindowHint);
    m_windowProxy = new LingmoUIQuick::WindowProxy2(this);
    m_windowProxy->setWindowType(LingmoUIQuick::WindowType::SystemWindow);

    // 访问窗口api
    rootContext()->setContextProperty("mainWindow", this);

    rootContext()->setContextProperty("isLiteMode", GlobalSetting::instance()->get(GlobalSetting::IsLiteMode));
    connect(GlobalSetting::instance(), &GlobalSetting::styleChanged, this , [this] (const GlobalSetting::Key& key) {
        if (key == GlobalSetting::EffectEnabled) {
            Q_EMIT effectEnabledChanged();
        } else if (key == GlobalSetting::Transparency) {
            Q_EMIT transparencyChanged();
        } else if (key == GlobalSetting::IsLiteMode) {
            rootContext()->setContextProperty("isLiteMode", GlobalSetting::instance()->get(key));
        }
    });

    updateGeometry();

    ContextMenuManager::instance()->setMainWindow(this);
}

void MenuWindow::initPanelSetting()
{
    if (!MenuSetting::instance()->get(FOLLOW_LINGMO_PANEL).toBool()) {
        return;
    }

    const QByteArray id(LINGMO_PANEL_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        m_setting = new QGSettings(id, QByteArray(), this);

        QStringList keys = m_setting->keys();
        if (keys.contains(LINGMO_PANEL_POSITION_KEY)) {
            m_panelPos = m_setting->get(LINGMO_PANEL_POSITION_KEY).toInt();
        }

        if (keys.contains(LINGMO_PANEL_SIZE_KEY)) {
            m_panelSize = m_setting->get(LINGMO_PANEL_SIZE_KEY).toInt();
        }

        connect(m_setting, &QGSettings::changed, this, [this] (const QString& key) {
            if (key == LINGMO_PANEL_POSITION_KEY) {
                m_panelPos = m_setting->get(LINGMO_PANEL_POSITION_KEY).toInt();
                updateGeometry();
            } else if (key == LINGMO_PANEL_SIZE_KEY) {
                m_panelSize = m_setting->get(key).toInt();
                updateGeometry();
            }
        });
    }
}

void MenuWindow::updateGeometry()
{
    updateCurrentScreenGeometry();
    setMinimumSize(m_fullScreenGeometry.size());
    setMaximumSize(m_fullScreenGeometry.size());
    setGeometry(m_fullScreenGeometry);
    m_windowProxy->setPosition(m_fullScreenGeometry.topLeft());

    updateGeometryOfMask();
    WindowHelper::setRegion(this, m_maskGeometry.x(), m_maskGeometry.y(), m_maskGeometry.width(), m_maskGeometry.height(), 16);
}

bool MenuWindow::isFullScreen() const
{
    return m_isFullScreen;
}

/**
 * beforeFullScreenChanged -> (qml)onWidthChanged -> fullScreenChanged
 * @param isFullScreen
 */
void MenuWindow::setFullScreen(bool isFullScreen)
{
    if (m_isFullScreen == isFullScreen) {
        return;
    }

    m_isFullScreen = isFullScreen;

    updateGeometryOfMask();

    // 更新contentItem尺寸
    QEvent event(QEvent::Resize);
    QCoreApplication::sendEvent(this, &event);

    Q_EMIT fullScreenChanged();
}

void MenuWindow::changeWindowBlurRegion(qreal x, qreal y, qreal w, qreal h, qreal radius)
{
    WindowHelper::setRegion(this, x, y, w, h, radius);
}

void MenuWindow::exitFullScreen()
{
    Q_EMIT beforeFullScreenExited();
}

void MenuWindow::exposeEvent(QExposeEvent *event)
{
    QQuickView::exposeEvent(event);
}

void MenuWindow::focusOutEvent(QFocusEvent *event)
{
    // void QQuickWindow::focusOutEvent(QFocusEvent *ev) { Q_D(QQuickWindow); if (d->contentItem)  d->contentItem->setFocus(false, ev->reason()); }
    QQuickView::focusOutEvent(event);
    // setVisible(false);
}

bool MenuWindow::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        if (QX11Info::isPlatformX11()) {
            requestActivate();
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        ContextMenuManager::instance()->closeMenu();
    }

    return QQuickView::event(event);
}

bool MenuWindow::effectEnabled() const
{
    return GlobalSetting::instance()->get(GlobalSetting::EffectEnabled).toBool();
}

double MenuWindow::transparency() const
{
    return GlobalSetting::instance()->get(GlobalSetting::Transparency).toDouble();
}

int MenuWindow::panelPos() const
{
    return m_panelPos;
}

bool MenuWindow::editMode() const
{
    return m_editMode;
}

void MenuWindow::setEditMode(bool mode)
{
    if (mode == m_editMode) {
            return;
        }

    m_editMode = mode;
    Q_EMIT editModeChanged();
}

void MenuWindow::updateGeometryOfMask()
{
    if (m_isFullScreen) {
        m_maskGeometry = QRect(0, 0, m_fullScreenGeometry.width(), m_fullScreenGeometry.height());
    } else {
        m_maskGeometry = m_normalGeometry;
    }

    setMask(m_maskGeometry);
}

void MenuWindow::updateCurrentScreenGeometry()
{
    if (!screen()) {
        if (!QGuiApplication::primaryScreen()) {
            return;
        }
        setScreen(QGuiApplication::primaryScreen());
    }

    // qt的可用区域有问题，暂时使用任务栏尺寸计算
    //QRect normalMaskRect, fullRect = screen()->availableGeometry();
    QRect normalMaskRect, fullRect = screen()->geometry();
    int width = MenuSetting::instance()->get(MENU_WIDTH).toInt();
    int height = MenuSetting::instance()->get(MENU_HEIGHT).toInt();
    int margin = MenuSetting::instance()->get(MENU_MARGIN).toInt();
    LingmoUIQuick::WindowProxy::SlideFromEdge slideFromEdge;
    bool isMirrored = qGuiApp->layoutDirection() == Qt::LayoutDirection::RightToLeft;

    //上: 1, 下: 0, 左: 2, 右: 3
    switch (m_panelPos) {
        default:
        case 0: {
            fullRect.adjust(0, 0, 0, -m_panelSize);

            QSize normalSize(qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height));
            if (isMirrored) {
                normalMaskRect.setTopLeft({fullRect.width() - margin - width, fullRect.height() - normalSize.height() - margin});
            } else {
                normalMaskRect.setTopLeft({margin, fullRect.height() - normalSize.height() - margin});
            }
            normalMaskRect.setSize(normalSize);
            slideFromEdge = LingmoUIQuick::WindowProxy::BottomEdge;
            break;
        }
        case 1: {
            fullRect.adjust(0, m_panelSize, 0, 0);

            QSize normalSize(qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height));
            if (isMirrored) {
                normalMaskRect.setTopLeft({fullRect.width() - margin - width, margin});
            } else {
                normalMaskRect.setTopLeft({margin, margin});
            }
            normalMaskRect.setSize(normalSize);
            slideFromEdge = LingmoUIQuick::WindowProxy::TopEdge;
            break;
        }
        case 2: {
            fullRect.adjust(m_panelSize, 0, 0, 0);

            normalMaskRect.setTopLeft({margin, margin});
            normalMaskRect.setSize({qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height)});
            slideFromEdge = LingmoUIQuick::WindowProxy::LeftEdge;
            break;
        }
        case 3: {
            fullRect.adjust(0, 0, -m_panelSize, 0);

            QSize normalSize(qMin<int>(fullRect.width() - margin*2, width), qMin<int>(fullRect.height() - margin*2, height));
            normalMaskRect.setTopLeft({fullRect.width() - normalSize.width() - margin, margin});
            normalMaskRect.setSize(normalSize);
            slideFromEdge = LingmoUIQuick::WindowProxy::RightEdge;
            break;
        }
    }

    m_normalGeometry = normalMaskRect;
    m_fullScreenGeometry = fullRect;

    m_windowProxy->setSlideWindowArgs(slideFromEdge, 0);

    Q_EMIT panelPosChanged();
    Q_EMIT normalRectChanged();
}

QRect MenuWindow::normalRect() const
{
    return m_normalGeometry;
}

void MenuWindow::activeMenuWindow(bool active)
{
    if (active == isVisible()) {
        return;
    }

    if (active) {
        if (screen() != LingmoUIQuick::WindowProxy::currentScreen()) {
            if (screen()) {
                screen()->disconnect(this);
            }
            this->setScreen(LingmoUIQuick::WindowProxy::currentScreen());
            connect(screen(), &QScreen::geometryChanged, this, &MenuWindow::updateGeometry);
            connect(qGuiApp, &QGuiApplication::layoutDirectionChanged, this, &MenuWindow::updateGeometry);
            updateGeometry();
        }
    }

    setVisible(active);
}

} // LingmoUIMenu
