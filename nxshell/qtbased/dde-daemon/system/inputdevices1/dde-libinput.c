// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dde-libinput.h"

#include <libinput.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>

extern void log_handler_go(enum libinput_log_priority priority, char *);
extern void handle_device_added(struct libinput_event *, void *);
extern void handle_device_removed(struct libinput_event *, void *);

struct data {
    int stop;
    void *userdata;
    enum libinput_log_priority log_priority;
};

static int open_restricted(const char *path, int flags, void *user_data) {
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data) {
    close(fd);
}

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static void log_handler(struct libinput *li,
                        enum libinput_log_priority priority,
                        const char *format,
                        va_list args) {
    char buff[1024] = {0};
    int n = vsnprintf(buff, 1024, format, args);
    if (n <= 0) {
        return;
    }

    if (buff[n - 1] == '\n') {
        buff[n - 1] = 0;
    }

    log_handler_go(priority, buff);

    // vfprintf(stderr, format, args);
}

static void handle_events(struct libinput *li, void *userdata) {
    struct libinput_event *event;

    libinput_dispatch(li);
    while ((event = libinput_get_event(li)) != NULL) {
        enum libinput_event_type type = libinput_event_get_type(event);

        switch (type) {
        case LIBINPUT_EVENT_DEVICE_ADDED:
            handle_device_added(event, userdata);
            break;
        case LIBINPUT_EVENT_DEVICE_REMOVED:
            handle_device_removed(event, userdata);
            break;
        case LIBINPUT_EVENT_TOUCH_DOWN:
        case LIBINPUT_EVENT_TOUCH_UP:
        case LIBINPUT_EVENT_TOUCH_MOTION:
        case LIBINPUT_EVENT_TOUCH_CANCEL:
        case LIBINPUT_EVENT_TOUCH_FRAME:

        default:
            break;
        }

        libinput_event_destroy(event);
        libinput_dispatch(li);
    }
}

static void main_loop(struct libinput *li, struct data *d) {
    handle_events(li, d->userdata);

    struct pollfd fds;
    fds.fd = libinput_get_fd(li);
    fds.events = POLLIN;
    fds.revents = 0;

    while (!d->stop && poll(&fds, 1, -1) > -1) {
        handle_events(li, d->userdata);
    }
}

struct data *new_libinput(char *userdata, enum libinput_log_priority log_priority) {
    struct data *d = malloc(sizeof(struct data));
    d->stop = 0;
    d->userdata = userdata;
    d->log_priority = log_priority;
    return d;
}

int start(struct data *d) {
    d->stop = 0;

    struct udev *udev = udev_new();
    if (udev == NULL) {
        fprintf(stderr, "failed to get udev\n");
        return 1;
    }

    struct libinput *li = libinput_udev_create_context(&interface, NULL, udev);
    if (li == NULL) {
        fprintf(stderr, "failed to create libinput context\n");
        udev_unref(udev);
        return 1;
    }

    libinput_log_set_handler(li, log_handler);
    libinput_log_set_priority(li, d->log_priority);

    if (libinput_udev_assign_seat(li, "seat0") < 0) {
        fprintf(stderr, "failed to assign seat0\n");
        libinput_unref(li);
        udev_unref(udev);
        return 1;
    }

    main_loop(li, d);

    libinput_unref(li);
    udev_unref(udev);

    return 0;
}

void stop(struct data *d) {
    d->stop = 1;
}
