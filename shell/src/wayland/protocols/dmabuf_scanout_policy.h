#ifndef LINGMO_WAYLAND_PROTOCOLS_DMABUF_SCANOUT_POLICY_H_
#define LINGMO_WAYLAND_PROTOCOLS_DMABUF_SCANOUT_POLICY_H_

#include <set>

#include "base/signal.h"
#include "wm/window_observer.h"

namespace Lingmo::WM {
class Window;
class WindowManager;
}

namespace Lingmo::Wayland {

class DmaBufFeedbackManager;

// Which surfaces get a scanout tranche in their linux-dmabuf feedback: the
// root surface of every fullscreen window, which is what the overlay planner
// can put straight on the primary plane. Exo's surface feedback listens to the
// shell surface's fullscreen state for the same reason.
//
// Must be destroyed before the WindowManager it observes.
class DmaBufScanoutPolicy : public wm::WindowObserver
{
public:
    DmaBufScanoutPolicy(DmaBufFeedbackManager* manager, wm::WindowManager* windowManager);
    ~DmaBufScanoutPolicy() override;

    // --- wm::WindowObserver ---
    void onWindowStateTypeChanged(wm::Window* window, wm::WindowStateType oldType) override;

private:
    void onWindowAdded(wm::Window* window);
    void onWindowRemoved(wm::Window* window);
    void apply(wm::Window* window, bool removed);

    DmaBufFeedbackManager* const m_manager;
    wm::WindowManager* const m_windowManager;
    std::set<wm::Window*> m_windows;
    base::Subscription m_windowAdded;
    base::Subscription m_windowRemoved;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_DMABUF_SCANOUT_POLICY_H_
