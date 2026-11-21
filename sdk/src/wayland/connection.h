// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ui/event.h"

struct lingmo_surface_blur_v1;
struct lingmo_shell_v1;
struct lingmo_status_item_manager_v1;
struct lingmo_window_hints_v1;
struct lingmo_window_placement_v1;
struct wl_compositor;
struct wl_subcompositor;
struct wl_shm;
struct xdg_toplevel_drag_manager_v1;
struct wl_data_device_manager;
struct wl_display;
struct wl_output;
struct wl_pointer;
struct wl_touch;
struct wl_registry;
struct wl_seat;
struct wl_surface;
struct wp_cursor_shape_device_v1;
struct wp_cursor_shape_manager_v1;
struct wp_fractional_scale_manager_v1;
struct wp_presentation;
struct wp_viewporter;
struct xdg_activation_token_v1;
struct xdg_activation_v1;
struct xdg_wm_base;
struct zxdg_exported_v2;
struct zxdg_exporter_v2;
struct zxdg_importer_v2;
struct xdg_wm_dialog_v1;
struct zwp_pointer_gesture_hold_v1;
struct zwp_pointer_gestures_v1;
struct zwp_text_input_manager_v3;
struct zwp_primary_selection_device_manager_v1;
struct zxdg_output_manager_v1;
struct zxdg_output_v1;

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::ui::wayland {

class DataDevice;
class PrimarySelection;
class Keyboard;
class TextInput;
class ShellSurface;
struct ConnectionListeners;

// The connection to the compositor: its globals, the seat, and the routing
// of input to the window whose surface it is for.
class Connection
{
public:
    static std::unique_ptr<Connection> open(base::EventLoop& loop);
    // A connection the session handed over as a socket descriptor, which it
    // takes; null when the compositor does not answer on it.
    static std::unique_ptr<Connection> openFd(base::EventLoop& loop, int fd);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    wl_display* display() const { return m_display; }
    wl_compositor* compositor() const { return m_compositor; }
    // Null when the compositor has none.
    wl_subcompositor* subcompositor() const { return m_subcompositor; }
    wl_shm* shm() const { return m_shm; }
    // Null when the compositor cannot carry windows in drags.
    xdg_toplevel_drag_manager_v1* toplevelDragManager() const { return m_toplevelDragManager; }
    // The keyboard's modifiers now, as KeyEvent has them.
    uint32_t keyboardModifiers() const;
    // The compositor closed the connection, as when it exits or restarts.
    bool isLost() const { return m_lost; }
    xdg_wm_base* wmBase() const { return m_wmBase; }
    wp_viewporter* viewporter() const { return m_viewporter; }
    wp_fractional_scale_manager_v1* fractionalScaleManager() const { return m_fractionalScaleManager; }
    wp_presentation* presentation() const { return m_presentation; }
    lingmo_window_hints_v1* windowHints() const { return m_windowHints; }
    // Null unless the compositor lets this client place its own windows.
    lingmo_window_placement_v1* windowPlacement() const { return m_windowPlacement; }
    lingmo_surface_blur_v1* surfaceBlur() const { return m_surfaceBlur; }
    // Null when the compositor has no status bar items.
    lingmo_status_item_manager_v1* statusItems() const { return m_statusItems; }
    xdg_wm_dialog_v1* dialogManager() const { return m_dialogManager; }
    zxdg_importer_v2* importer() const { return m_importer; }
    wl_seat* seat() const { return m_seat; }

    // The serial of the last button press or touch down, for move and resize
    // requests.
    uint32_t buttonSerial() const { return m_buttonSerial; }
    // The serial of the last key or button press, for the clipboard.
    uint32_t inputSerial() const { return m_inputSerial; }

    DataDevice* dataDevice() const { return m_dataDevice.get(); }
    // Null when the compositor has no primary selection.
    PrimarySelection* primarySelection() const { return m_primarySelection.get(); }

    // Runs once the compositor has answered everything sent so far (a
    // wl_display.sync round trip), so a batch of focus changes reads as one.
    void sync(std::function<void()> done);

    // A handle another client can parent its windows to, as
    // "wayland:<handle>", once the compositor gave one; kept while the
    // window lives.
    void exportToplevel(ShellSurface* shell, std::function<void(const std::string& handle)> done);

    // xdg-activation for the window: a token for the last input and the focused
    // window, then the activation.
    void activate(ShellSurface* shell, const std::string& appId);
    // The activation with a token from elsewhere.
    void activateWithToken(ShellSurface* shell, const std::string& token);
    // A token for another process to activate its window with, from the
    // last input in the window; empty when the compositor gives none.
    void requestActivationToken(ShellSurface* shell, std::function<void(const std::string&)> done);
    TextInput* textInput() const { return m_textInput.get(); }

    // Applies while the pointer is over the window's surface.
    void setCursorShape(ShellSurface* shell, CursorShape shape);
    // A window going away loses pointer and keyboard focus.
    void forget(ShellSurface* shell);

    void flush();

    // An output's logical size, as xdg-output gives it; empty until known.
    SizeF outputSize(wl_output* output) const;
    // The first output's, for a window that is on none yet.
    SizeF primaryOutputSize() const;

    struct OutputInfo
    {
        // The wl_output name, as lingmo_shell_surface_v1.set_output takes it.
        std::string name;
        PointF position;
        SizeF logicalSize;
        // In the same coordinates as the position; the whole output until
        // the compositor tells.
        RectF workArea;
        // Device pixels per logical pixel.
        double scale = 1;
        bool primary = false;
    };
    std::vector<OutputInfo> outputs() const;
    // After an output came, went or changed.
    std::function<void()> onOutputsChanged;
    // Only on the session's trusted connection.
    lingmo_shell_v1* shell() const { return m_shell; }

    // The token for an activation came; for the listener.
    void activationDone(xdg_activation_token_v1* token, const char* tokenString);
    // The compositor named an exported window; for the listener.
    void exported(zxdg_exported_v2* exported, const char* handle);

private:
    friend struct ConnectionListeners;

    Connection(base::EventLoop& loop, wl_display* display);
    bool initialize();
    void handleConnectionLost();

    base::EventLoop& m_loop;
    wl_display* m_display = nullptr;
    wl_registry* m_registry = nullptr;
    wl_compositor* m_compositor = nullptr;
    wl_subcompositor* m_subcompositor = nullptr;
    wl_shm* m_shm = nullptr;
    xdg_toplevel_drag_manager_v1* m_toplevelDragManager = nullptr;
    xdg_wm_base* m_wmBase = nullptr;
    wp_viewporter* m_viewporter = nullptr;
    wp_fractional_scale_manager_v1* m_fractionalScaleManager = nullptr;
    wp_presentation* m_presentation = nullptr;
    wp_cursor_shape_manager_v1* m_cursorShapeManager = nullptr;
    lingmo_window_hints_v1* m_windowHints = nullptr;
    lingmo_window_placement_v1* m_windowPlacement = nullptr;
    lingmo_surface_blur_v1* m_surfaceBlur = nullptr;
    lingmo_status_item_manager_v1* m_statusItems = nullptr;
    lingmo_shell_v1* m_shell = nullptr;
    xdg_wm_dialog_v1* m_dialogManager = nullptr;
    xdg_activation_v1* m_activation = nullptr;
    // Tokens on their way, and the window each is for.
    struct Activation
    {
        xdg_activation_token_v1* token;
        ShellSurface* shell;
        // For a token handed to another process rather than used here.
        std::function<void(const std::string&)> done;
    };
    std::vector<Activation> m_activations;
    zxdg_exporter_v2* m_exporter = nullptr;
    zxdg_importer_v2* m_importer = nullptr;
    struct Export
    {
        ShellSurface* shell;
        zxdg_exported_v2* exported;
        std::string handle;
        std::vector<std::function<void(const std::string&)>> waiting;
    };
    std::vector<Export> m_exports;
    zxdg_output_manager_v1* m_outputManager = nullptr;
    struct Output
    {
        uint32_t name;
        wl_output* output;
        zxdg_output_v1* xdgOutput;
        SizeF logicalSize;
        PointF position;
        std::string outputName;
        // The current mode's.
        SizeF pixelSize;
    };
    std::vector<Output> m_outputs;
    struct WorkArea
    {
        RectF area;
        bool primary = false;
    };
    // By output name, as lingmo_window_hints_v1.work_area tells.
    std::map<std::string, WorkArea> m_workAreas;
    void watchOutput(Output& output);
    static void releaseOutput(const Output& output);
    wl_seat* m_seat = nullptr;
    wl_pointer* m_pointer = nullptr;
    wp_cursor_shape_device_v1* m_cursorShapeDevice = nullptr;
    zwp_pointer_gestures_v1* m_pointerGestures = nullptr;
    zwp_pointer_gesture_hold_v1* m_hold = nullptr;
    wl_touch* m_touch = nullptr;
    std::unique_ptr<Keyboard> m_keyboard;
    wl_data_device_manager* m_dataDeviceManager = nullptr;
    std::unique_ptr<DataDevice> m_dataDevice;
    zwp_primary_selection_device_manager_v1* m_primarySelectionManager = nullptr;
    std::unique_ptr<PrimarySelection> m_primarySelection;
    zwp_text_input_manager_v3* m_textInputManager = nullptr;
    std::unique_ptr<TextInput> m_textInput;
    ShellSurface* m_pointerFocus = nullptr;
    // The surface each touch point went down on; up and motion do not name it.
    std::vector<std::pair<int32_t, ShellSurface*>> m_touchFocus;
    uint32_t m_enterSerial = 0;
    uint32_t m_buttonSerial = 0;
    uint32_t m_inputSerial = 0;
    // Axis events gather until the pointer frame that ends them.
    ScrollEvent m_scroll;
    bool m_scrollPending = false;
    int m_watch = 0;
    bool m_lost = false;
    int m_hook = 0;
};

} // namespace Lingmo::ui::wayland
