// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockhelper.h"
#include "constants.h"
#include "dockpanel.h"

#include <QGuiApplication>

namespace dock
{
DockHelper::DockHelper(DockPanel *parent)
    : QObject(parent)
    , m_hideTimer(new QTimer(this))
    , m_showTimer(new QTimer(this))
{
    m_hideTimer->setInterval(400);
    m_showTimer->setInterval(400);
    m_hideTimer->setSingleShot(true);
    m_showTimer->setSingleShot(true);

    qApp->installEventFilter(this);
    QMetaObject::invokeMethod(this, &DockHelper::initAreas, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, &DockHelper::checkNeedShowOrNot, Qt::QueuedConnection);

    connect(parent, &DockPanel::rootObjectChanged, this, &DockHelper::initAreas);
    connect(parent, &DockPanel::showInPrimaryChanged, this, &DockHelper::updateAllDockWakeArea);
    connect(parent, &DockPanel::hideStateChanged, this, &DockHelper::updateAllDockWakeArea);
    connect(parent, &DockPanel::hideModeChanged, m_hideTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(parent, &DockPanel::hideModeChanged, m_showTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(parent, &DockPanel::positionChanged, this, [this](Position pos) {
        std::for_each(m_areas.begin(), m_areas.end(), [pos](const auto &area) {
            if (!area)
                return;
            area->updateDockWakeArea(pos);
        });
    });

    connect(m_hideTimer, &QTimer::timeout, this, &DockHelper::checkNeedHideOrNot);
    connect(m_showTimer, &QTimer::timeout, this, &DockHelper::checkNeedShowOrNot);

    connect(this, &DockHelper::isWindowOverlapChanged, this, [this](bool overlap) {
        if (overlap) {
            m_hideTimer->start();
        } else {
            m_showTimer->start();
        }
    });
    connect(this, &DockHelper::currentActiveWindowFullscreenChanged, this, [this] (bool isFullscreen) {
        if (isFullscreen) {
            checkNeedHideOrNot();
        } else {
            checkNeedShowOrNot();
        }
    });
}

bool DockHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (!watched->isWindowType()) {
        return false;
    }

    auto window = static_cast<QWindow *>(watched);
    if (!window) {
        return false;
    }

    auto topTransientParent = window;
    while (topTransientParent->transientParent()) {
        topTransientParent = topTransientParent->transientParent();
    }

    // not dock panel or dock popup has a enter event
    if (window != parent()->rootObject() && topTransientParent != parent()->rootObject()) {
        return false;
    }

    switch (event->type()) {
    case QEvent::Enter: {
        m_enters.insert(window, true);
        if (m_hideTimer->isActive()) {
            m_hideTimer->stop();
        }

        m_showTimer->start();
        break;
    }
    case QEvent::Leave: {
        m_enters.insert(window, false);
        if (m_showTimer->isActive()) {
            m_showTimer->stop();
        }

        m_hideTimer->start();
        break;
    }

    case QEvent::Hide: {
        m_enters.remove(window);
        break;
    }
    default: {
    }
    }

    return false;
}

bool DockHelper::wakeUpAreaNeedShowOnThisScreen(QScreen *screen)
{
    auto isDockAllowedShownOnThisScreen = [this, screen]() -> bool {
        return (parent()->showInPrimary() && screen == qApp->primaryScreen()) || !parent()->showInPrimary();
    };

    auto isDockShownThisScreen = [this, screen]() -> bool {
        return parent()->dockScreen() == screen && parent()->hideState() == Show;
    };

    return (isDockAllowedShownOnThisScreen() && !isDockShownThisScreen());
}

void DockHelper::enterScreen(QScreen *screen)
{
    auto nowScreen = parent()->dockScreen();

    if (nowScreen == screen) {
        parent()->setHideState(Show);
        return;
    }

    QTimer::singleShot(200, [this, screen]() {
        parent()->setDockScreen(screen);
        parent()->setHideState(Show);
        updateAllDockWakeArea();
    });
}

void DockHelper::leaveScreen()
{
    m_hideTimer->start();
}

void DockHelper::updateAllDockWakeArea()
{
    for (auto screen : m_areas.keys()) {
        auto area = m_areas.value(screen);
        if (nullptr == area)
            continue;

        if (wakeUpAreaNeedShowOnThisScreen(screen)) {
            area->open();
            area->updateDockWakeArea(parent()->position());
        } else {
            area->close();
        }
    }
}

void DockHelper::checkNeedHideOrNot()
{
    bool needHide;
    switch (parent()->hideMode()) {
    case KeepShowing: {
        // KeepShow. current activeWindow is fullscreend.
        needHide = currentActiveWindowFullscreened();
        break;
    }
    case SmartHide: {
        // SmartHide. window overlap.
        needHide = isWindowOverlap();
        break;
    }
    case KeepHidden: {
        // only any enter
        needHide = true;
        break;
    }
    }

    // any enter will not make hide
    for (auto enter : m_enters) {
        needHide &= !enter;
    }

    if (needHide)
        parent()->setHideState(Hide);
}

void DockHelper::checkNeedShowOrNot()
{
    bool needShow;
    switch (parent()->hideMode()) {
    case KeepShowing: {
        // KeepShow. currentWindow is not fullscreened.
        needShow = !currentActiveWindowFullscreened();
        break;
    }
    case SmartHide: {
        // SmartHide. no window overlap.
        needShow = !isWindowOverlap();
        break;
    }
    case KeepHidden: {
        // KeepHidden only any enter.
        needShow = false;
        break;
    }
    }

    for (auto enter : m_enters) {
        needShow |= enter;
    }

    if (needShow)
        parent()->setHideState(Show);
}

DockPanel* DockHelper::parent()
{
    return static_cast<DockPanel *>(QObject::parent());
}

void DockHelper::initAreas()
{
    // clear old area
    for (auto area : m_areas) {
        area->close();
        delete area;
    }

    m_areas.clear();
    qApp->disconnect(this);

    if (!parent()->rootObject())
        return;

    // init areas
    for (auto screen : qApp->screens()) {
        m_areas.insert(screen, createArea(screen));
    }

    connect(qApp, &QGuiApplication::screenAoceand, this, [this](QScreen *screen) {
        if (m_areas.contains(screen))
            return;
        m_areas.insert(screen, createArea(screen));
    });

    connect(qApp, &QGuiApplication::screenRemoved, this, [this](QScreen *screen) {
        if (!m_areas.contains(screen))
            return;

        destroyArea(m_areas.value(screen));
        m_areas.remove(screen);
    });

    // init state
    updateAllDockWakeArea();
}

DockWakeUpArea::DockWakeUpArea(QScreen *screen, DockHelper *helper)
    : m_screen(screen)
    , m_helper(helper)
{
}

QScreen *DockWakeUpArea::screen()
{
    return m_screen;
}
}
