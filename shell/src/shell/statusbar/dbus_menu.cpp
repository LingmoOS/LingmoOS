// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/dbus_menu.h"

#include <algorithm>
#include <chrono>

#include "base/strings.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static constexpr const char *kInterface = "com.canonical.dbusmenu";

DBusMenu::DBusMenu(dbus::Connection *bus, std::string service, std::string path)
    : m_bus(bus)
    , m_service(std::move(service))
    , m_path(std::move(path))
    , m_proxy(bus, m_service, m_path, kInterface)
    , m_sender(bus, m_service)
{
    DBusMenuItem root;
    root.hasSubmenu = true;
    m_items.emplace(0, std::move(root));

    m_layoutTimer.setSingleShot(true);
    m_layoutTimer.onTimeout = [this] {
        const std::set<int32_t> ids = std::exchange(m_pendingLayoutUpdates, {});
        for (int32_t id : ids)
            refresh(id);
    };

    m_matches.push_back(m_proxy.onSignal("LayoutUpdated", [this](const dbus::Message &message) {
        uint32_t revision = 0;
        int32_t parent = 0;
        if (!m_sender.accepts(message) || !message.read(revision, parent))
            return;
        if (m_refreshedByAboutToShow.erase(parent))
            return;
        m_pendingLayoutUpdates.insert(parent);
        if (!m_layoutTimer.isActive())
            m_layoutTimer.start(std::chrono::nanoseconds(0));
    }));
    m_matches.push_back(m_proxy.onSignal("ItemsPropertiesUpdated", [this](const dbus::Message &message) {
        std::vector<std::tuple<int32_t, dbus::VariantMap>> updated;
        std::vector<std::tuple<int32_t, std::vector<std::string>>> removed;
        if (!m_sender.accepts(message) || !message.read(updated, removed))
            return;
        for (const auto &[id, properties] : updated) {
            // An entry of a menu not fetched yet arrives with that menu.
            const auto it = m_items.find(id);
            if (it == m_items.end())
                continue;
            applyProperties(it->second, properties, false);
            itemChanged(id);
        }
        for (const auto &[id, keys] : removed) {
            const auto it = m_items.find(id);
            if (it == m_items.end())
                continue;
            for (const std::string &key : keys)
                resetProperty(it->second, key);
            itemChanged(id);
        }
    }));
    m_matches.push_back(m_proxy.onSignal("ItemActivationRequested", [this](const dbus::Message &message) {
        int32_t id = 0;
        uint32_t timestamp = 0;
        if (m_sender.accepts(message) && message.read(id, timestamp) && m_items.count(id))
            activationRequested(id);
    }));

    refresh(0);
}

DBusMenu::DBusMenu(dbus::Connection *bus, std::string service)
    : m_bus(bus)
    , m_service(std::move(service))
    , m_sender(bus, m_service)
{
    DBusMenuItem root;
    root.hasSubmenu = true;
    m_items.emplace(0, std::move(root));
}

DBusMenu::~DBusMenu() = default;

const DBusMenuItem *DBusMenu::item(int32_t id) const
{
    const auto it = m_items.find(id);
    return it == m_items.end() ? nullptr : &it->second;
}

void DBusMenu::refresh(int32_t id)
{
    m_calls.track([&](std::function<void()> finished) {
        return m_proxy.callAsync(
                "GetLayout",
                [this, id, finished](const dbus::Reply &reply) {
                    // The revision is of no use: each fetch takes the whole level.
                    if (reply.isError() || reply.arguments().size() < 2) {
                        if (m_items.count(id))
                            menuUpdated(id);
                    } else {
                        applyLayout(id, reply.arguments()[1]);
                    }
                    finished();
                },
                id, int32_t(1), std::vector<std::string>());
    });
}

void DBusMenu::applyLayout(int32_t parentId, const dbus::Variant &layout)
{
    const auto parentIt = m_items.find(parentId);
    if (parentIt == m_items.end() || layout.signature() != "(ia{sv}av)")
        return;

    std::vector<int32_t> children;
    for (const dbus::Variant &child : layout.items()[2].items()) {
        const std::optional<dbus::Variant> inner = child.get<dbus::Variant>();
        if (!inner || inner->signature() != "(ia{sv}av)")
            continue;
        const int32_t id = inner->items()[0].value<int32_t>();
        const dbus::VariantMap properties = inner->items()[1].value<dbus::VariantMap>();
        // The root, or an entry listed twice, would make a cycle.
        if (id == 0 || id == parentId || std::find(children.begin(), children.end(), id) != children.end())
            continue;

        auto it = m_items.find(id);
        if (it == m_items.end()) {
            if (m_items.size() >= kMaxItems)
                continue;
            DBusMenuItem item;
            item.id = id;
            it = m_items.emplace(id, std::move(item)).first;
            applyProperties(it->second, properties, true);
        } else {
            applyProperties(it->second, properties, false);
            if (it->second.parent != parentId) {
                // An entry that moved to another menu leaves the old one.
                if (auto old = m_items.find(it->second.parent); old != m_items.end())
                    std::erase(old->second.children, id);
            }
        }
        it->second.parent = parentId;
        children.push_back(id);
    }

    // Entries no longer listed go, with everything under them.
    const std::vector<int32_t> previous = m_items.at(parentId).children;
    for (int32_t id : previous) {
        if (std::find(children.begin(), children.end(), id) == children.end()) {
            const auto it = m_items.find(id);
            if (it != m_items.end() && it->second.parent == parentId)
                removeItem(id);
        }
    }

    DBusMenuItem &parent = m_items.at(parentId);
    parent.children = std::move(children);
    parent.childrenLoaded = true;
    menuUpdated(parentId);
}

void DBusMenu::removeItem(int32_t id)
{
    const auto it = m_items.find(id);
    if (it == m_items.end() || id == 0)
        return;
    const std::vector<int32_t> children = it->second.children;
    m_items.erase(it);
    m_iconJobs.erase(id);
    for (int32_t child : children) {
        const auto childIt = m_items.find(child);
        if (childIt != m_items.end() && childIt->second.parent == id)
            removeItem(child);
    }
}

static std::string cappedLabel(const std::string &label)
{
    if (label.size() <= DBusMenu::kMaxLabelBytes)
        return label;
    // Cut on a character boundary.
    size_t size = DBusMenu::kMaxLabelBytes;
    while (size > 0 && (uint8_t(label[size]) & 0xc0) == 0x80)
        --size;
    return label.substr(0, size);
}

void DBusMenu::applyProperties(DBusMenuItem &item, const dbus::VariantMap &properties, bool created)
{
    for (const auto &[key, value] : properties) {
        if (key == "type") {
            if (created)
                item.separator = value.value<std::string>() == "separator";
        } else if (key == "toggle-type") {
            if (created) {
                const std::string type = value.value<std::string>();
                item.toggle = type == "checkmark" ? DBusMenuItem::Toggle::Checkmark
                        : type == "radio"         ? DBusMenuItem::Toggle::Radio
                                                  : DBusMenuItem::Toggle::None;
            }
        } else if (key == "children-display") {
            if (created)
                item.hasSubmenu = value.value<std::string>() == "submenu";
        } else if (key == "label") {
            item.label = cappedLabel(value.value<std::string>());
        } else if (key == "enabled") {
            item.enabled = value.value<bool>(true);
        } else if (key == "visible") {
            item.visible = value.value<bool>(true);
        } else if (key == "toggle-state") {
            item.toggleState = value.value<int32_t>(-1);
        } else if (key == "icon-name") {
            item.iconName = value.value<std::string>();
        } else if (key == "icon-data") {
            const std::optional<std::vector<uint8_t>> bytes = value.get<std::vector<uint8_t>>();
            if (bytes && !bytes->empty())
                decodeIcon(item.id, std::string(bytes->begin(), bytes->end()));
            else
                item.icon.reset();
        } else if (key == "shortcut") {
            item.shortcut = value.value<std::vector<std::vector<std::string>>>();
        }
    }
}

void DBusMenu::resetProperty(DBusMenuItem &item, const std::string &key)
{
    if (key == "label")
        item.label.clear();
    else if (key == "enabled")
        item.enabled = true;
    else if (key == "visible")
        item.visible = true;
    else if (key == "toggle-state")
        item.toggleState = -1;
    else if (key == "icon-name")
        item.iconName.clear();
    else if (key == "icon-data")
        item.icon.reset();
    else if (key == "shortcut")
        item.shortcut.clear();
}

// The width and height of a PNG, from its header, so a bomb is refused
// before it is decoded.
static bool pngSize(const std::string &bytes, uint32_t *width, uint32_t *height)
{
    static constexpr unsigned char signature[] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};
    if (bytes.size() < 24 || !std::equal(std::begin(signature), std::end(signature), bytes.begin(),
                                          [](unsigned char a, char b) { return a == uint8_t(b); }))
        return false;
    const auto read32 = [&bytes](size_t offset) {
        return (uint32_t(uint8_t(bytes[offset])) << 24) | (uint32_t(uint8_t(bytes[offset + 1])) << 16)
                | (uint32_t(uint8_t(bytes[offset + 2])) << 8) | uint32_t(uint8_t(bytes[offset + 3]));
    };
    *width = read32(16);
    *height = read32(20);
    return true;
}

void DBusMenu::decodeIcon(int32_t id, const std::string &bytes)
{
    uint32_t width = 0;
    uint32_t height = 0;
    if (bytes.size() > kMaxIconBytes || !pngSize(bytes, &width, &height) || width == 0 || height == 0
        || width > uint32_t(kMaxIconSide) || height > uint32_t(kMaxIconSide))
        return;
    m_iconJobs[id] = base::runInThread([bytes] { return std::make_shared<const ui::Bitmap>(ui::Bitmap::decode(bytes)); },
                                       [this, id](std::shared_ptr<const ui::Bitmap> icon) {
                                           m_iconJobs.erase(id);
                                           const auto it = m_items.find(id);
                                           if (it == m_items.end())
                                               return;
                                           it->second.icon = icon->isNull() ? nullptr : icon;
                                           itemChanged(id);
                                       });
}

void DBusMenu::aboutToShow(int32_t id)
{
    if (!m_items.count(id))
        return;
    prepare(id);
    // Some exporters act on AboutToShow and others only on "opened": both go.
    opened(id);
}

void DBusMenu::prepare(int32_t id)
{
    if (!m_items.count(id))
        return;

    // A refetch from the last show whose LayoutUpdated never came must not
    // swallow the next real update; this show refetches anyway.
    m_refreshedByAboutToShow.erase(id);
    m_calls.track([&](std::function<void()> finished) {
        return m_proxy.callAsync(
                "AboutToShow",
                [this, id, finished](const dbus::Reply &reply) {
                    const auto it = m_items.find(id);
                    if (it != m_items.end()) {
                        if (reply.isError()) {
                            menuUpdated(id);
                        } else if (reply.value<bool>().value_or(false) || it->second.children.empty()) {
                            m_refreshedByAboutToShow.insert(id);
                            refresh(id);
                        } else {
                            menuUpdated(id);
                        }
                    }
                    finished();
                },
                id);
    });
}

void DBusMenu::opened(int32_t id)
{
    sendEvent(id, "opened");
}

void DBusMenu::closed(int32_t id)
{
    sendEvent(id, "closed");
}

void DBusMenu::activate(int32_t id)
{
    sendEvent(id, "clicked");
}

void DBusMenu::sendEvent(int32_t id, const std::string &event)
{
    m_proxy.send("Event", id, event, dbus::Variant(std::string()), uint32_t(0));
}

} // namespace Lingmo::Shell
