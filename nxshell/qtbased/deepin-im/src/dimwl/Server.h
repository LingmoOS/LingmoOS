// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVER_H
#define SERVER_H

#include "Listener.h"
#include "XcbHelper.h"
#include "common/common.h"

#include <wayland-server-core.h>

#include <string>

extern "C" {
#include <wlr/backend.h>
#define static
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output_layout.h>
#undef static
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
}

#include <memory>

struct wlr_scene;
struct wlr_xdg_shell;
struct wlr_virtual_keyboard_manager_v1;
struct wlr_input_method_manager_v2;
struct wlr_text_input_manager_v3;

class Output;
class View;
class InputMethodV2;
class TextInputV3;
class X11ActiveWindowMonitor;

enum class SessionType {
    WL,
    X11,
};

class Server
{
public:
    Server();
    ~Server();

    std::string addSocketAuto();
    bool addSocket(const std::string &name);
    void run();

    bool startBackend();

    wl_event_loop *getLoop();

    SessionType sessionType() { return sessionType_; }

    wlr_allocator *allocator() { return allocator_.get(); }

    wlr_renderer *renderer() { return renderer_.get(); }

    wlr_output_layout *outputLayout() { return output_layout_.get(); }

    wlr_scene *scene() { return scene_.get(); }

    wl_list *views() { return &views_; }

    wlr_seat *seat() { return seat_.get(); }

    InputMethodV2 *inputMethod() { return input_method_; }

    wl_list *textInputs() { return &text_inputs_; }

    void setTextInputFocus(wlr_surface *surface);
    void setTextInputUnfocus(wlr_surface *surface);

private:
    void backendNewOutputNotify(void *data);
    void xdgShellNewSurfaceNotify(void *data);
    void cursorMotionNotify(void *data);
    void cursorMotionAbsoluteNotify(void *data);
    void cursorButtonNotify(void *data);
    void cursorAxisNotify(void *data);
    void cursorFrameNotify(void *data);
    void backendNewInputNotify(void *data);
    void outputPresentNotify(void *data);
    void seatRequestCursorNotify(void *data);
    void seatRequestSetSelectionNotify(void *data);
    void virtualKeyboardManagerNewVirtualKeyboardNotify(void *data);
    void inputMethodManagerV2InputMethodNotify(void *data);
    void inputMethodV2DestroyNotify(void *data);
    void textInputManagerV3TextInputNotify(void *data);
    void textInputV3DestroyNotify(void *data);
    void x11ActiveWindowNotify(void *data);
    void textInputCursorRectangleNotify(void *data);

    void processCursorMotion(uint32_t time);
    View *desktopViewAt(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);

private:
    SessionType sessionType_;
    std::unique_ptr<wl_display, Deleter<wl_display_destroy>> display_;
    std::unique_ptr<wlr_backend, Deleter<wlr_backend_destroy>> backend_;
    std::unique_ptr<wlr_renderer, Deleter<wlr_renderer_destroy>> renderer_;
    std::unique_ptr<wlr_allocator, Deleter<wlr_allocator_destroy>> allocator_;
    std::unique_ptr<wlr_output_layout, Deleter<wlr_output_layout_destroy>> output_layout_;
    Output *output_ = nullptr;
    Listener<&Server::backendNewOutputNotify> backend_new_output_;
    Listener<&Server::outputPresentNotify> output_present_;

    std::unique_ptr<wlr_scene> scene_;

    wl_list views_;
    std::unique_ptr<wlr_xdg_shell> xdg_shell_;
    Listener<&Server::xdgShellNewSurfaceNotify> xdg_shell_new_surface_;

    std::unique_ptr<wlr_cursor, Deleter<wlr_cursor_destroy>> cursor_;
    std::unique_ptr<wlr_xcursor_manager, Deleter<wlr_xcursor_manager_destroy>> cursor_mgr_;
    Listener<&Server::cursorMotionNotify> cursor_motion_;
    Listener<&Server::cursorMotionAbsoluteNotify> cursor_motion_absolute_;
    Listener<&Server::cursorButtonNotify> cursor_button_;
    Listener<&Server::cursorAxisNotify> cursor_axis_;
    Listener<&Server::cursorFrameNotify> cursor_frame_;

    std::unique_ptr<wlr_seat, Deleter<wlr_seat_destroy>> seat_;
    Listener<&Server::seatRequestCursorNotify> seat_request_cursor_;
    Listener<&Server::seatRequestSetSelectionNotify> seat_request_set_selection_;
    wl_list keyboards_;
    Listener<&Server::backendNewInputNotify> backend_new_input_;

    std::unique_ptr<wlr_virtual_keyboard_manager_v1> virtual_keyboard_manager_v1_;
    Listener<&Server::virtualKeyboardManagerNewVirtualKeyboardNotify>
        virtual_keyboard_manager_v1_new_virtual_keyboard_;

    std::unique_ptr<wlr_input_method_manager_v2> input_method_manager_v2_;
    Listener<&Server::inputMethodManagerV2InputMethodNotify> input_method_manager_v2_input_method_;
    InputMethodV2 *input_method_ = nullptr;
    Listener<&Server::inputMethodV2DestroyNotify> input_method_v2_destroy_;

    std::unique_ptr<wlr_text_input_manager_v3> text_input_manager_v3_;
    wl_list text_inputs_;
    Listener<&Server::textInputManagerV3TextInputNotify> text_input_manager_v3_text_input_;

    XcbHelper xcb_helper_;

    std::unique_ptr<X11ActiveWindowMonitor> x11ActiveWindowMonitor_;
    Listener<&Server::x11ActiveWindowNotify> x11ActiveWindow_;

    Listener<&Server::textInputCursorRectangleNotify> textInputCursorRectangle_;
};

#endif // !SERVER_H
