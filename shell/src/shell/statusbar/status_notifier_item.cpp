// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/status_notifier_item.h"

#include <chrono>
#include <filesystem>

#include "shell/common/task_manager.h"
#include "shell/statusbar/dbus_menu.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static constexpr const char *kInterface = "org.kde.StatusNotifierItem";
static constexpr std::chrono::milliseconds kRefreshDelay(10);

StatusNotifierItem::StatusNotifierItem(dbus::Connection *bus, std::string id, TaskManager *tasks,
                                       IconExists iconExists)
    : m_bus(bus)
    , m_id(std::move(id))
    , m_tasks(tasks)
    , m_iconExists(std::move(iconExists))
{
    const size_t slash = m_id.find('/');
    if (slash == std::string::npos || slash == 0)
        return;
    m_service = m_id.substr(0, slash);
    m_path = m_id.substr(slash);
    m_proxy = dbus::Proxy(bus, m_service, m_path, kInterface);
    m_sender = std::make_unique<SenderFilter>(bus, m_service);

    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.onTimeout = [this] { performRefresh(); };

    for (const char *signal : {"NewTitle", "NewIcon", "NewAttentionIcon", "NewOverlayIcon", "NewToolTip"})
        m_matches.push_back(m_proxy.onSignal(signal, [this](const dbus::Message &message) {
            if (m_sender->accepts(message))
                refresh();
        }));

    if (m_tasks) {
        m_applicationsChanged = m_tasks->applicationSource().changed.connect([this] {
            if (m_iconName.empty() && m_pixmaps.empty())
                refresh();
        });
    }

    refresh();
}

StatusNotifierItem::~StatusNotifierItem() = default;

void StatusNotifierItem::refresh()
{
    if (m_proxy.isValid() && !m_refreshTimer.isActive())
        m_refreshTimer.start(kRefreshDelay);
}

void StatusNotifierItem::performRefresh()
{
    if (m_refreshing) {
        m_needsRefresh = true;
        return;
    }
    m_refreshing = true;
    m_calls.track([&](std::function<void()> finished) {
        return m_proxy.getAllAsync([this, finished](const dbus::Reply &reply) {
            m_refreshing = false;
            if (m_needsRefresh) {
                // What arrived is already out of date.
                m_needsRefresh = false;
                performRefresh();
            } else if (const std::optional<dbus::VariantMap> properties = reply.value<dbus::VariantMap>()) {
                apply(*properties);
            }
            finished();
        });
    });
}

static std::string property(const dbus::VariantMap &properties, const char *name)
{
    const auto it = properties.find(name);
    if (it == properties.end())
        return {};
    if (auto path = it->second.get<dbus::ObjectPath>())
        return path->path;
    return it->second.value<std::string>();
}

void StatusNotifierItem::apply(const dbus::VariantMap &properties)
{
    std::string themePath = property(properties, "IconThemePath");
    if (!themePath.empty())
        themePath = std::filesystem::path(themePath).lexically_normal().string();
    while (themePath.size() > 1 && themePath.back() == '/')
        themePath.pop_back();
    m_iconThemePath = themePath;

    m_title = property(properties, "Title");
    m_appId = property(properties, "Id");
    const auto itemIsMenu = properties.find("ItemIsMenu");
    m_itemIsMenu = itemIsMenu != properties.end() && itemIsMenu->second.value<bool>(false);

    const auto iconByName = [this](const std::string &name) -> std::string {
        if (name.empty() || !m_iconExists)
            return {};
        if (m_iconExists(name, m_iconThemePath))
            return name;
        if (!name.ends_with("-symbolic") && m_iconExists(name + "-symbolic", m_iconThemePath))
            return name + "-symbolic";
        return {};
    };
    m_iconName = iconByName(property(properties, "IconName"));
    if (m_iconName.empty())
        m_iconName = iconByName(m_appId);

    m_toolTip.clear();
    if (const auto it = properties.find("ToolTip"); it != properties.end()) {
        if (it->second.signature() == "(sa(iiay)ss)")
            m_toolTip = it->second.items()[2].value<std::string>();
    }

    const auto pixmaps = properties.find("IconPixmap");
    const bool hasPixmaps = pixmaps != properties.end() && pixmaps->second.signature() == "a(iiay)"
            && !pixmaps->second.items().empty();
    if (hasPixmaps) {
        decodePixmaps(pixmaps->second);
    } else {
        m_decode.cancel();
        m_pixmaps.clear();
    }

    // Some tray clients have no usable icon: the one of the matching desktop
    // entry stands in before a generic one would.
    if (!hasPixmaps)
        lookUpApplicationIcon();

    if (!m_menu) {
        const std::string menuPath = property(properties, "Menu");
        // "/NO_DBUSMENU" is how KStatusNotifierItem turns the menu off.
        if (!menuPath.empty() && !menuPath.starts_with("/NO_DBUSMENU"))
            m_menu = std::make_unique<DBusMenu>(m_bus, m_service, menuPath);
    }

    changed();
}

void StatusNotifierItem::decodePixmaps(const dbus::Variant &pixmaps)
{
    struct Raw {
        int32_t width;
        int32_t height;
        std::string bytes;
    };
    std::vector<Raw> raws;
    for (const dbus::Variant &pixmap : pixmaps.items()) {
        if (raws.size() >= kMaxPixmaps)
            break;
        const int32_t width = pixmap.items()[0].value<int32_t>();
        const int32_t height = pixmap.items()[1].value<int32_t>();
        const std::string &bytes = pixmap.items()[2].bytes();
        // A short array from a misbehaving item must not be read past its end.
        if (width <= 0 || height <= 0 || width > kMaxPixmapSide || height > kMaxPixmapSide
            || bytes.size() < size_t(width) * size_t(height) * 4)
            continue;
        raws.push_back({width, height, bytes});
    }

    m_decode = base::runInThread(
            [raws = std::move(raws)] {
                std::vector<ui::Bitmap> bitmaps;
                for (const Raw &raw : raws) {
                    ui::Bitmap bitmap(raw.width, raw.height, ui::Bitmap::Format::Argb32Premultiplied);
                    if (bitmap.isNull())
                        continue;
                    // ARGB32 in network byte order, not premultiplied.
                    const auto *source = reinterpret_cast<const uint8_t *>(raw.bytes.data());
                    for (int y = 0; y < raw.height; ++y) {
                        auto *target = reinterpret_cast<uint32_t *>(bitmap.scanLine(y));
                        for (int x = 0; x < raw.width; ++x, source += 4) {
                            const uint32_t alpha = source[0];
                            const auto premultiply = [alpha](uint32_t value) { return (value * alpha + 127) / 255; };
                            target[x] = (alpha << 24) | (premultiply(source[1]) << 16) | (premultiply(source[2]) << 8)
                                    | premultiply(source[3]);
                        }
                    }
                    bitmaps.push_back(std::move(bitmap));
                }
                return bitmaps;
            },
            [this](std::vector<ui::Bitmap> bitmaps) {
                m_pixmaps = std::move(bitmaps);
                if (m_pixmaps.empty())
                    lookUpApplicationIcon();
                changed();
            });
}

void StatusNotifierItem::lookUpApplicationIcon()
{
    if (!m_tasks || !m_bus || m_service.empty()) {
        m_applicationIcon.clear();
        return;
    }
    const dbus::Proxy bus(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    m_calls.track([&](std::function<void()> finished) {
        return bus.callAsync(
                "GetConnectionUnixProcessID",
                [this, finished](const dbus::Reply &reply) {
                    std::string icon;
                    if (const std::optional<uint32_t> pid = reply.value<uint32_t>()) {
                        const DesktopEntryPtr entry = m_tasks->resolveProcess(m_appId, *pid, m_appId);
                        if (entry && !entry->icon.empty()) {
                            const std::string &name = entry->icon;
                            if (name.starts_with('/'))
                                icon = name;
                            else if (m_iconExists && m_iconExists(name, {}))
                                icon = name;
                            else if (m_iconExists && !name.ends_with("-symbolic") && m_iconExists(name + "-symbolic", {}))
                                icon = name + "-symbolic";
                        }
                    }
                    if (icon != m_applicationIcon) {
                        m_applicationIcon = icon;
                        changed();
                    }
                    finished();
                },
                m_service);
    });
}

void StatusNotifierItem::activate(int x, int y)
{
    m_calls.track([&](std::function<void()> finished) {
        return m_proxy.callAsync("Activate", [finished](const dbus::Reply &) { finished(); }, int32_t(x), int32_t(y));
    });
}

void StatusNotifierItem::secondaryActivate(int x, int y)
{
    m_proxy.send("SecondaryActivate", int32_t(x), int32_t(y));
}

void StatusNotifierItem::scroll(int delta, const std::string &orientation)
{
    m_proxy.send("Scroll", int32_t(delta), orientation);
}

void StatusNotifierItem::requestContextMenu(int x, int y)
{
    m_proxy.send("ContextMenu", int32_t(x), int32_t(y));
}

} // namespace Lingmo::Shell
