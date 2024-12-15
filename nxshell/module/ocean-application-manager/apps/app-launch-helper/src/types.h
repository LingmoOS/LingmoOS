// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TYPES_H
#define TYPES_H

#include <systemd/sd-bus.h>
#include <systemd/sd-journal.h>
#include <string>
#include <string_view>

enum class ExitCode { SystemdError = -3, InvalidInput = -2, InternalError = -1, Done = 0, Waiting = 1 };

enum class DBusValueType { String, ArrayOfString };

using msg_ptr = sd_bus_message *;
using bus_ptr = sd_bus *;

struct JobRemoveResult
{
    std::string_view id;
    int removedFlag{0};
    ExitCode result{ExitCode::Waiting};
};

#endif
