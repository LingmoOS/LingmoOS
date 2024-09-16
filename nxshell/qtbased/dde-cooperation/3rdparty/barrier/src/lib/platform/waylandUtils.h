#ifndef WAYLANDUTILS_H
#define WAYLANDUTILS_H

#include "XWindowsScreen.h"
#include "base/Log.h"
#include <displayjack_kvm/displayjack_kvm.h>
class WaylandUtils
{
public:
    static void init_wayland_kvm(void *user_data)
    {
        // 使用kvm所有的功能时，必须先调用init_kvm,确保成功才能使用其它函数和功能 0是成功
        int initSucceed = init_kvm();
        LOG((CLOG_NOTE " init kvm status: %d", initSucceed));

        if (initSucceed == 0) {
            kvm_register_pointer_motion(user_data, WaylandUtils::test_kvm_pointer_motion_cb1);
        }
    }

    static void test_kvm_pointer_motion_cb1(void *user_data, unsigned int time, double x, double y)
    {
        XWindowsScreen *xsreen = static_cast<XWindowsScreen *>(user_data);
        if (!xsreen)
            return;
        XMotionEvent xmotion;
        xmotion.type = MotionNotify;
        xmotion.send_event = False;   // Raw motion
        xmotion.x_root = x;
        xmotion.y_root = y;
        xmotion.x = x;
        xmotion.y = y;
        xsreen->handleMouseMove(xmotion);
    }

    static bool isWayland()
    {
        char *type = getenv("XDG_SESSION_TYPE");
        char *display = getenv("WAYLAND_DISPLAY");

        return (type && strcmp(type, "wayland") == 0) || (display && strcmp(display, "wayland") == 0);
    }

    static void set_pointer_pos(double x, double y)
    {
        kvm_pointer_set_pos(x, y);
    }
};

#endif   // WAYLANDUTILS_H
