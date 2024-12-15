// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OUTPUT_H
#define OUTPUT_H

#include "Listener.h"

#include <wayland-server-core.h>

struct wlr_output;

class Server;

class Output
{
public:
    Output(Server *server, struct wlr_output *output, wl_list *list = nullptr);
    ~Output();

    wlr_output *output() { return output_; }

private:
    void frameNotify(void *data);
    void destroyNotify(void *data);

private:
    Server *server_;
    wl_list link_;
    struct wlr_output *output_;

    Listener<&Output::frameNotify> frame_;
    Listener<&Output::destroyNotify> destroy_;
};

#endif // !OUTPUT_H
