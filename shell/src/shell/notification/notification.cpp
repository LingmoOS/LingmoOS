// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification.h"

#include "shell/common/text_match.h"

namespace Lingmo::Shell {

std::string Notification::applicationKey() const
{
    if (!desktopEntry.empty())
        return desktopEntry;
    return toLower(appName);
}

std::string Notification::groupKey() const
{
    const std::string key = applicationKey();
    return key.empty() ? appIcon : key;
}

std::vector<Notification::Action> Notification::actionList() const
{
    std::vector<Action> list;
    for (size_t i = 0; i + 1 < actions.size(); i += 2) {
        if (actions[i] == "default")
            continue;
        list.push_back({actions[i], actions[i + 1]});
    }
    return list;
}

bool Notification::hasAction(const std::string &key) const
{
    for (const std::string &action : actions) {
        if (action == key)
            return true;
    }
    return false;
}

base::json::Value Notification::toJson() const
{
    base::json::Value value = base::json::Value::object();
    value.insert("service", service);
    value.insert("summary", summary);
    value.insert("body", body);
    value.insert("appName", appName);
    value.insert("appIcon", appIcon);
    if (created)
        value.insert("created", *created);
    if (updated)
        value.insert("updated", *updated);
    value.insert("desktopEntry", desktopEntry);
    value.insert("image", image);
    return value;
}

Notification Notification::fromJson(const base::json::Value &value)
{
    Notification notification;
    notification.service = value["service"].toString();
    notification.summary = value["summary"].toString();
    notification.body = value["body"].toString();
    notification.appName = value["appName"].toString();
    notification.appIcon = value["appIcon"].toString();
    if (value["created"].isNumber())
        notification.created = value["created"].toInt();
    if (value["updated"].isNumber())
        notification.updated = value["updated"].toInt();
    notification.desktopEntry = value["desktopEntry"].toString();
    notification.image = value["image"].toString();
    return notification;
}

} // namespace Lingmo::Shell
