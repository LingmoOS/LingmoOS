/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#ifdef __FreeBSD__
#include <dev/evdev/input.h>
#include <dev/evdev/uinput.h>
#else
#include <linux/input.h>
#include <linux/uinput.h>
#endif

// edit uidev.id.product to change tablet id

void check(int ioctlresult, const std::string &errormsg)
{
    if (ioctlresult < 0) {
        std::cout << "ioctl failed: " << errormsg << std::endl;
        exit(-1);
    }
}

void init_device(int fd)
{
    struct uinput_setup uidev;

    // enable synchronization
    check(ioctl(fd, UI_SET_EVBIT, EV_SYN), "UI_SET_EVBIT EV_SYN");

    // enable 1 button
    check(ioctl(fd, UI_SET_EVBIT, EV_KEY), "UI_SET_EVBIT EV_KEY");
    check(ioctl(fd, UI_SET_KEYBIT, BTN_TOUCH), "UI_SET_KEYBIT BTN_TOUCH");

    // Needed only for stylus device, comment out for touch
    check(ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_PEN), "UI_SET_KEYBIT BTN_TOOL_PEN");
    check(ioctl(fd, UI_SET_KEYBIT, BTN_STYLUS), "UI_SET_KEYBIT BTN_STYLUS");
    check(ioctl(fd, UI_SET_KEYBIT, BTN_STYLUS2), "UI_SET_KEYBIT BTN_STYLUS2");

    // enable 2 main axis + pressure (absolute positioning)
    check(ioctl(fd, UI_SET_EVBIT, EV_ABS), "UI_SET_EVBIT EV_ABS");
    check(ioctl(fd, UI_SET_ABSBIT, ABS_X), "UI_SETEVBIT ABS_X");
    check(ioctl(fd, UI_SET_ABSBIT, ABS_Y), "UI_SETEVBIT ABS_Y");
    check(ioctl(fd, UI_SET_ABSBIT, ABS_PRESSURE), "UI_SETEVBIT ABS_PRESSURE");
    check(ioctl(fd, UI_SET_PROPBIT, INPUT_PROP_POINTER), "UI_SET_PROPBIT INPUT_PROP_POINTER");

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Dummy Tablet");

    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor = 0x056a; // wacom
    // Product ID really should match specific device
    // pen
    uidev.id.product = 0x0307;
    // touch
    // uidev.id.product = 0x0309;
    uidev.id.version = 1;

    uidev.ff_effects_max = 0;

    struct uinput_abs_setup xabs;
    xabs.code = ABS_X;
    xabs.absinfo.minimum = 0;
    xabs.absinfo.maximum = UINT16_MAX;
    xabs.absinfo.fuzz = 0;
    xabs.absinfo.flat = 0;
    xabs.absinfo.resolution = 100;
    xabs.absinfo.value = 0;

    struct uinput_abs_setup yabs;
    yabs.code = ABS_Y;
    yabs.absinfo.minimum = 0;
    yabs.absinfo.maximum = UINT16_MAX;
    yabs.absinfo.fuzz = 0;
    yabs.absinfo.flat = 0;
    yabs.absinfo.resolution = 100;
    yabs.absinfo.value = 0;

    struct uinput_abs_setup pressure;
    pressure.code = ABS_PRESSURE;
    pressure.absinfo.minimum = 0;
    pressure.absinfo.maximum = UINT16_MAX;
    pressure.absinfo.fuzz = 0;
    pressure.absinfo.flat = 0;
    pressure.absinfo.resolution = 0;
    pressure.absinfo.value = 0;

    struct uinput_abs_setup distance;
    distance.code = ABS_DISTANCE;
    distance.absinfo.minimum = 0;
    distance.absinfo.maximum = UINT16_MAX;
    distance.absinfo.fuzz = 0;
    distance.absinfo.flat = 0;
    distance.absinfo.resolution = 0;
    distance.absinfo.value = 0;

    check(ioctl(fd, UI_DEV_SETUP, &uidev), "UI_DEV_SETUP");
    check(ioctl(fd, UI_ABS_SETUP, &xabs), "X setup");
    check(ioctl(fd, UI_ABS_SETUP, &yabs), "Y setup");
    check(ioctl(fd, UI_ABS_SETUP, &pressure), "pressure setup");
    check(ioctl(fd, UI_ABS_SETUP, &distance), "distance setup");

    check(ioctl(fd, UI_DEV_CREATE), "device creation");
}

void doNothing(int unused)
{
    (void)unused;
}

int main(void)
{
    int device;
    if ((device = open("/dev/uinput", O_WRONLY | O_NONBLOCK)) < 0) {
        std::cout << "Can't open /dev/uinput";
        exit(-2);
    }

    init_device(device);

    std::cout << "Fake device created. "
              << "Press CTRL+C to disconnect the device" << std::endl;
    signal(SIGINT, &doNothing);
    pause();

    std::cout << "Shutting down" << std::endl;
    ioctl(device, UI_DEV_DESTROY);
    close(device);
    std::cout << "Ok" << std::endl;

    return 0;
}
