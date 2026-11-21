#include "wayland/protocols/dmabuf_scanout_policy.h"

#include "wayland/protocols/dmabuf_feedback_manager.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::Wayland {

DmaBufScanoutPolicy::DmaBufScanoutPolicy(DmaBufFeedbackManager* manager,
                                         wm::WindowManager* windowManager)
    : m_manager(manager)
    , m_windowManager(windowManager)
{
    m_windowAdded = m_windowManager->windowAdded.connect([this](wm::Window* window) {
        onWindowAdded(window);
    });
    m_windowRemoved = m_windowManager->windowRemoved.connect([this](wm::Window* window) {
        onWindowRemoved(window);
    });
    for (wm::Window* window : m_windowManager->windows())
        onWindowAdded(window);
}

DmaBufScanoutPolicy::~DmaBufScanoutPolicy()
{
    for (wm::Window* window : m_windows)
        window->removeObserver(this);
}

void DmaBufScanoutPolicy::onWindowStateTypeChanged(wm::Window* window, wm::WindowStateType)
{
    apply(window, false);
}

void DmaBufScanoutPolicy::onWindowAdded(wm::Window* window)
{
    if (!window || !m_windows.insert(window).second)
        return;
    window->addObserver(this);
    apply(window, false);
}

void DmaBufScanoutPolicy::onWindowRemoved(wm::Window* window)
{
    if (m_windows.erase(window) == 0)
        return;
    window->removeObserver(this);
    apply(window, true);
}

void DmaBufScanoutPolicy::apply(wm::Window* window, bool removed)
{
    const bool candidate = !removed && window->stateType() == wm::WindowStateType::Fullscreen;
    m_manager->setScanoutCandidate(window->rootSurface(), candidate);
}

} // namespace Lingmo::Wayland
