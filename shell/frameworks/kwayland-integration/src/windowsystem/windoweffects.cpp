/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "windoweffects.h"

#include <QDebug>
#include <QExposeEvent>
#include <QGuiApplication>
#include <QWidget>

#include <private/qwaylandwindow_p.h>

#include <QWaylandClientExtensionTemplate>
#include <qwaylandclientextension.h>

#include "qwayland-blur.h"
#include "qwayland-contrast.h"
#include "qwayland-slide.h"

#include "surfacehelper.h"

static wl_region *createRegion(const QRegion &region)
{
    QPlatformNativeInterface *native = qGuiApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    auto compositor = reinterpret_cast<wl_compositor *>(native->nativeResourceForIntegration(QByteArrayLiteral("compositor")));
    if (!compositor) {
        return nullptr;
    }
    auto wl_region = wl_compositor_create_region(compositor);
    for (const auto &rect : region) {
        wl_region_add(wl_region, rect.x(), rect.y(), rect.width(), rect.height());
    }
    return wl_region;
}

class BlurManager : public QWaylandClientExtensionTemplate<BlurManager>, public QtWayland::org_kde_kwin_blur_manager
{
public:
    BlurManager()
        : QWaylandClientExtensionTemplate<BlurManager>(1)
    {
    }
};

class Blur : public QObject, public QtWayland::org_kde_kwin_blur
{
public:
    Blur(struct ::org_kde_kwin_blur *object, QObject *parent)
        : QObject(parent)
        , QtWayland::org_kde_kwin_blur(object)
    {
    }

    ~Blur() override
    {
        release();
    }
};

class ContrastManager : public QWaylandClientExtensionTemplate<ContrastManager>, public QtWayland::org_kde_kwin_contrast_manager
{
public:
    ContrastManager()
        : QWaylandClientExtensionTemplate<ContrastManager>(2)
    {
    }
};

class Contrast : public QObject, public QtWayland::org_kde_kwin_contrast
{
public:
    Contrast(struct ::org_kde_kwin_contrast *object, QObject *parent)
        : QObject(parent)
        , QtWayland::org_kde_kwin_contrast(object)
    {
    }

    ~Contrast() override
    {
        release();
    }
};

class SlideManager : public QWaylandClientExtensionTemplate<SlideManager>, public QtWayland::org_kde_kwin_slide_manager
{
public:
    SlideManager()
        : QWaylandClientExtensionTemplate<SlideManager>(1)
    {
    }
};

class Slide : public QObject, public QtWayland::org_kde_kwin_slide
{
public:
    Slide(struct ::org_kde_kwin_slide *object, QObject *parent)
        : QObject(parent)
        , QtWayland::org_kde_kwin_slide(object)
    {
    }

    ~Slide() override
    {
        release();
    }
};

WindowEffects::WindowEffects()
    : QObject()
    , KWindowEffectsPrivateV2()
{
    m_blurManager = new BlurManager();
    m_contrastManager = new ContrastManager();
    m_slideManager = new SlideManager();

    // The KWindowEffects API doesn't provide any signals to notify that the particular
    // effect has become unavailable. So we re-install effects when the corresponding globals
    // are added.

    connect(m_blurManager, &BlurManager::activeChanged, this, [this] {
        for (auto it = m_blurRegions.constBegin(); it != m_blurRegions.constEnd(); ++it) {
            installBlur(it.key(), m_blurManager->isActive(), *it);
        }
    });

    connect(m_contrastManager, &ContrastManager::activeChanged, this, [this] {
        for (auto it = m_backgroundConstrastRegions.constBegin(); it != m_backgroundConstrastRegions.constEnd(); ++it) {
            if (m_contrastManager->isActive()) {
                installContrast(it.key(), true, it->contrast, it->intensity, it->saturation, it->region);
            } else {
                installContrast(it.key(), false);
            }
        }
    });

    connect(m_slideManager, &SlideManager::activeChanged, this, [this] {
        for (auto it = m_slideMap.constBegin(); it != m_slideMap.constEnd(); ++it) {
            if (m_slideManager->isActive()) {
                installSlide(it.key(), it->location, it->offset);
            } else {
                installSlide(it.key(), KWindowEffects::SlideFromLocation::NoEdge, 0);
            }
        }
    });
}

WindowEffects::~WindowEffects()
{
    delete m_blurManager;
    delete m_contrastManager;
    delete m_slideManager;
}

QWindow *WindowEffects::windowForId(WId wid)
{
    QWindow *window = nullptr;

    for (auto win : qApp->allWindows()) {
        if (win->winId() == wid) {
            window = win;
            break;
        }
    }
    return window;
}

void WindowEffects::trackWindow(QWindow *window)
{
    if (!m_windowWatchers.contains(window)) {
        window->installEventFilter(this);
        auto conn = connect(window, &QObject::destroyed, this, [this, window]() {
            resetBlur(window);
            m_blurRegions.remove(window);
            resetContrast(window);
            m_backgroundConstrastRegions.remove(window);
            m_slideMap.remove(window);
            m_windowWatchers.remove(window);
        });
        m_windowWatchers[window] << conn;
        auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
        if (waylandWindow) {
            auto conn = connect(waylandWindow, &QtWaylandClient::QWaylandWindow::wlSurfaceDestroyed, this, [this, window]() {
                resetBlur(window);
                resetContrast(window);
            });
            m_windowWatchers[window] << conn;
        }
    }
}

void WindowEffects::releaseWindow(QWindow *window)
{
    if (!m_blurRegions.contains(window) && !m_backgroundConstrastRegions.contains(window) && !m_slideMap.contains(window)) {
        for (const auto &conn : m_windowWatchers[window]) {
            disconnect(conn);
        }
        window->removeEventFilter(this);
        m_windowWatchers.remove(window);
    }
}

// Helper function to replace a QObject value in the map and delete the old one.
template<typename MapType>
void replaceValue(MapType &map, typename MapType::key_type key, typename MapType::mapped_type value)
{
    if (auto oldValue = map.take(key)) {
        oldValue->deleteLater();
    }
    if (value) {
        map[key] = value;
    }
}

void WindowEffects::resetBlur(QWindow *window, Blur *blur)
{
    replaceValue(m_blurs, window, blur);
}

void WindowEffects::resetContrast(QWindow *window, Contrast *contrast)
{
    replaceValue(m_contrasts, window, contrast);
}

bool WindowEffects::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        auto ee = static_cast<QExposeEvent *>(event);

        if ((ee->region().isNull())) {
            return false;
        }

        auto window = qobject_cast<QWindow *>(watched);
        if (!window) {
            return false;
        }

        {
            auto it = m_blurRegions.constFind(window);
            if (it != m_blurRegions.constEnd()) {
                installBlur(window, true, *it);
            }
        }
        {
            auto it = m_backgroundConstrastRegions.constFind(window);
            if (it != m_backgroundConstrastRegions.constEnd()) {
                installContrast(window, true, it->contrast, it->intensity, it->saturation, it->region);
            }
        }
        {
            auto it = m_slideMap.constFind(window);
            if (it != m_slideMap.constEnd()) {
                installSlide(window, it->location, it->offset);
            }
        }
    }
    return false;
}

bool WindowEffects::isEffectAvailable(KWindowEffects::Effect effect)
{
    switch (effect) {
    case KWindowEffects::BackgroundContrast:
        return m_contrastManager->isActive();
    case KWindowEffects::BlurBehind:
        return m_blurManager->isActive();
    case KWindowEffects::Slide:
        return m_slideManager->isActive();
    default:
        return false;
    }
}

void WindowEffects::slideWindow(WId id, KWindowEffects::SlideFromLocation location, int offset)
{
    auto window = windowForId(id);
    if (!window) {
        return;
    }
    if (location != KWindowEffects::SlideFromLocation::NoEdge) {
        m_slideMap[window] = SlideData{
            .location = location,
            .offset = offset,
        };
        trackWindow(window);
    } else {
        m_slideMap.remove(window);
        releaseWindow(window);
    }

    installSlide(window, location, offset);
}

void WindowEffects::installSlide(QWindow *window, KWindowEffects::SlideFromLocation location, int offset)
{
    if (!m_slideManager->isActive()) {
        return;
    }
    wl_surface *surface = surfaceForWindow(window);
    if (surface) {
        if (location != KWindowEffects::SlideFromLocation::NoEdge) {
            auto slide = new Slide(m_slideManager->create(surface), window);

            Slide::location convertedLoc;
            switch (location) {
            case KWindowEffects::SlideFromLocation::TopEdge:
                convertedLoc = Slide::location::location_top;
                break;
            case KWindowEffects::SlideFromLocation::LeftEdge:
                convertedLoc = Slide::location::location_left;
                break;
            case KWindowEffects::SlideFromLocation::RightEdge:
                convertedLoc = Slide::location::location_right;
                break;
            case KWindowEffects::SlideFromLocation::BottomEdge:
            default:
                convertedLoc = Slide::location::location_bottom;
                break;
            }

            slide->set_location(convertedLoc);
            slide->set_offset(offset);
            slide->commit();
        } else {
            m_slideManager->unset(surface);
        }
    }
}

QList<QSize> WindowEffects::windowSizes(const QList<WId> &ids)
{
    Q_UNUSED(ids)
    QList<QSize> sizes;
    return sizes;
}

void WindowEffects::presentWindows(WId controller, const QList<WId> &ids)
{
    Q_UNUSED(controller)
    Q_UNUSED(ids)
}

void WindowEffects::presentWindows(WId controller, int desktop)
{
    Q_UNUSED(controller)
    Q_UNUSED(desktop)
}

void WindowEffects::highlightWindows(WId controller, const QList<WId> &ids)
{
    Q_UNUSED(controller)
    Q_UNUSED(ids)
}

void WindowEffects::enableBlurBehind(WId winId, bool enable, const QRegion &region)
{
    auto window = windowForId(winId);
    if (!window) {
        return;
    }
    if (enable) {
        trackWindow(window);
        m_blurRegions[window] = region;
    } else {
        resetBlur(window);
        m_blurRegions.remove(window);
        releaseWindow(window);
    }

    installBlur(window, enable, region);
}

void WindowEffects::installBlur(QWindow *window, bool enable, const QRegion &region)
{
    if (!m_blurManager->isActive()) {
        return;
    }

    wl_surface *surface = surfaceForWindow(window);

    if (surface) {
        if (enable) {
            auto wl_region = createRegion(region);
            if (!wl_region) {
                return;
            }
            auto blur = new Blur(m_blurManager->create(surface), window);
            blur->set_region(wl_region);
            blur->commit();
            wl_region_destroy(wl_region);
            resetBlur(window, blur);
        } else {
            resetBlur(window);
            m_blurManager->unset(surface);
        }
    }
}

void WindowEffects::enableBackgroundContrast(WId winId, bool enable, qreal contrast, qreal intensity, qreal saturation, const QRegion &region)

{
    auto window = windowForId(winId);
    if (!window) {
        return;
    }
    if (enable) {
        trackWindow(window);
        m_backgroundConstrastRegions[window].contrast = contrast;
        m_backgroundConstrastRegions[window].intensity = intensity;
        m_backgroundConstrastRegions[window].saturation = saturation;
        m_backgroundConstrastRegions[window].region = region;
    } else {
        resetContrast(window);
        m_backgroundConstrastRegions.remove(window);
        releaseWindow(window);
    }

    installContrast(window, enable, contrast, intensity, saturation, region);
}

void WindowEffects::installContrast(QWindow *window, bool enable, qreal contrast, qreal intensity, qreal saturation, const QRegion &region)
{
    if (!m_contrastManager->isActive()) {
        return;
    }

    wl_surface *surface = surfaceForWindow(window);
    if (surface) {
        if (enable) {
            auto wl_region = createRegion(region);
            if (!wl_region) {
                return;
            }
            auto backgroundContrast = new Contrast(m_contrastManager->create(surface), window);
            backgroundContrast->set_region(wl_region);
            backgroundContrast->set_contrast(wl_fixed_from_double(contrast));
            backgroundContrast->set_intensity(wl_fixed_from_double(intensity));
            backgroundContrast->set_saturation(wl_fixed_from_double(saturation));
            backgroundContrast->commit();
            wl_region_destroy(wl_region);
            resetContrast(window, backgroundContrast);
        } else {
            resetContrast(window);
            m_contrastManager->unset(surface);
        }
    }
}

void WindowEffects::setBackgroundFrost(QWindow *window, QColor color, const QRegion &region)
{
    if (!m_contrastManager->isActive()) {
        return;
    }

    wl_surface *surface = surfaceForWindow(window);
    if (!surface) {
        return;
    }
    if (!color.isValid()) {
        resetContrast(window);
        m_contrastManager->unset(surface);
        return;
    }

    auto wl_region = createRegion(region);
    if (!wl_region) {
        return;
    }
    auto backgroundContrast = new Contrast(m_contrastManager->create(surface), window);
    backgroundContrast->set_region(wl_region);
    backgroundContrast->set_frost(color.red(), color.green(), color.blue(), color.alpha());
    backgroundContrast->commit();
    wl_region_destroy(wl_region);
    resetContrast(window, backgroundContrast);
}

void WindowEffects::markAsDashboard(WId window)
{
    Q_UNUSED(window)
}
