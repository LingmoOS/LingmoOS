// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "base/json.h"

namespace Lingmo::Shell {

// One notification, from Notify() to the banner and the history.
struct Notification {
    uint32_t id = 0;
    // The sender's unique bus name.
    std::string service;
    std::string summary;
    std::string body;
    std::string appName;
    // An icon name or a local file.
    std::string appIcon;
    std::string desktopEntry;
    // The local file of the picture attached to the notification.
    std::string image;
    // The spec's flat list: key, label, key, label, ...
    std::vector<std::string> actions;
    int32_t timeout = -1;
    int32_t urgency = 1;
    // Notification sound hints; only a displayed banner may play one.
    bool suppressSound = false;
    std::string soundName;
    // Progress in percent from the "value" hint, or -1.
    int32_t value = -1;
    // Milliseconds since the epoch, UTC.
    std::optional<int64_t> created;
    std::optional<int64_t> updated;

    struct Action {
        std::string key;
        std::string label;
    };

    // Names the sending application in the per-application settings: its
    // desktop id, else its lower-cased name; may be empty.
    std::string applicationKey() const;
    // What the history groups notifications by.
    std::string groupKey() const;
    // The actions other than "default".
    std::vector<Action> actionList() const;
    bool hasAction(const std::string &key) const;

    // The history's record: service, summary, body, app name, app icon,
    // created, updated, desktop entry and image.
    base::json::Value toJson() const;
    static Notification fromJson(const base::json::Value &value);
};

} // namespace Lingmo::Shell

