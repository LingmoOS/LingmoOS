// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/previewclient.h"
#include "core/previewkeys.h"
#include "model/foldermodel.h"
#include "views/folderview.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/json.h"
#include "base/uuid.h"
#include "ui/application.h"
#include "ui/keys.h"
#include "ui/window.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace fm {

static constexpr const char* kService = "org.lingmo.FilePreview";
static constexpr const char* kPath = "/org/lingmo/FilePreview";

PreviewClient::PreviewClient()
    : m_id(base::Uuid::generate().toString())
{
    base::dbus::Connection* bus = ui::Application::instance() ? ui::Application::instance()->sessionBus() : nullptr;
    if (bus) {
        m_service = base::dbus::Proxy(bus, kService, kPath, kService);
        m_matches.push_back(m_service.onSignal("VisibilityChanged", [this](const base::dbus::Message& message) {
            bool visible = false;
            if (message.read(visible))
                setVisible(visible);
        }));
        m_matches.push_back(m_service.onSignal("Navigate", [this](const base::dbus::Message& message) {
            std::string source;
            int key = 0;
            int modifiers = 0;
            if (!message.read(source, key, modifiers) || source != m_id || !m_view)
                return;
            m_navigating = true;
            m_view->sendKey(ui::KeyEvent {.key = previewkeys::fromWireKey(key), .modifiers = previewkeys::fromWireModifiers(modifiers)});
            m_navigating = false;
        }));
        m_matches.push_back(m_service.onSignal("RestoreFocus", [this](const base::dbus::Message& message) {
            std::string source;
            if (!message.read(source) || source != m_id || !m_view || !m_view->isVisible())
                return;
            m_view->contentView()->setFocus();
            if (m_view->window())
                m_view->window()->activate();
        }));
        m_nameWatch = bus->watchName(kService, [this](const std::string& owner) {
            const bool started = !m_serviceRunning && !owner.empty();
            m_serviceRunning = !owner.empty();
            if (owner.empty())
                setVisible(false);
            else if (started)
                queryVisibility();
        });
    }

    m_updateTimer.setSingleShot(true);
    m_updateTimer.onTimeout = [this] {
        if (!follows())
            return;
        // The active window takes the preview over, as it may not hold it yet.
        send(viewIsActive() ? "Activate" : "Update", m_id, snapshot());
    };
}

PreviewClient::~PreviewClient()
{
    *m_alive = false;
    if (m_service.isValid() && m_serviceRunning)
        m_service.send("Release", m_id);
}

template<typename... Args>
void PreviewClient::send(const std::string& method, const Args&... args)
{
    // Only Toggle starts the service; anything else would bring up a file
    // manager just to be told about a preview nobody shows.
    if (!m_service.isValid() || (method != "Toggle" && !m_serviceRunning))
        return;
    const int id = m_nextCall++;
    m_calls[id] = m_service.callAsync(
            method,
            [this, id, method, alive = m_alive](const base::dbus::Reply& reply) {
                if (method == "Toggle" && reply.isError()) {
                    setVisible(false);
                    if (m_model)
                        m_model->notification.emit(
                                format(tr("Could not open preview: {error}"), {{"error", reply.error().message}}));
                }
                // Not from inside the call's own callback.
                base::EventLoop::current()->post([this, alive, id] {
                    if (*alive)
                        m_calls.erase(id);
                });
            },
            args...);
}

void PreviewClient::attach(FolderModel* model, FolderView* view)
{
    m_subscriptions.clear();
    m_model = model;
    m_view = view;
    if (!m_model || !m_view)
        return;
    m_subscriptions.push_back(m_model->selectionChanged.connect([this] { m_updateTimer.start(); }));
    m_subscriptions.push_back(m_model->urlChanged.connect([this] { m_updateTimer.start(); }));
    m_subscriptions.push_back(m_model->rowDataChanged.connect([this](int) { m_updateTimer.start(); }));
    m_subscriptions.push_back(m_view->currentIndexChanged.connect([this] { m_updateTimer.start(); }));
    queryVisibility();
    activate();
}

void PreviewClient::queryVisibility()
{
    if (!m_service.isValid() || !m_serviceRunning)
        return;
    const int id = m_nextCall++;
    m_calls[id] = m_service.callAsync("IsVisible", [this, id, alive = m_alive](const base::dbus::Reply& reply) {
        bool visible = false;
        if (reply.read(visible))
            setVisible(visible);
        base::EventLoop::current()->post([this, alive, id] {
            if (*alive)
                m_calls.erase(id);
        });
    });
}

bool PreviewClient::follows() const
{
    return m_model && m_view && (m_followsEmptySelection || m_model->selectionCount() > 0);
}

bool PreviewClient::viewIsActive() const
{
    return m_view && m_view->isVisible() && m_view->window() && m_view->window()->isActive();
}

std::string PreviewClient::snapshot() const
{
    base::json::Value::Array items;
    int currentIndex = 0;
    const int currentRow = m_view ? m_view->currentRow() : -1;
    for (int row = 0; row < m_model->count(); ++row) {
        if (!m_model->isSelected(row))
            continue;
        if (row == currentRow)
            currentIndex = int(items.size());
        const FileItemPtr item = m_model->item(row);
        base::json::Value entry = base::json::Value::object();
        entry.insert("url", item->url);
        entry.insert("name", m_model->displayName(row));
        entry.insert("icon", item->isDir ? std::string("folder") : item->iconName);
        entry.insert("thumbnail", std::string());
        items.push_back(std::move(entry));
    }
    if (items.empty()) {
        const std::string url = m_model->resolvedUrl();
        base::json::Value entry = base::json::Value::object();
        entry.insert("url", url);
        const size_t slash = url.find_last_of('/', url.size() > 1 ? url.size() - 2 : std::string::npos);
        entry.insert("name", slash == std::string::npos ? url : url.substr(slash + 1));
        entry.insert("icon", "folder");
        items.push_back(std::move(entry));
    }
    base::json::Value data = base::json::Value::object();
    data.insert("items", base::json::Value(std::move(items)));
    data.insert("currentIndex", currentIndex);
    data.insert("ratio", m_view && m_view->window() ? m_view->window()->scale() : 0.0);
    return data.serialize();
}

void PreviewClient::activate()
{
    if (follows() && viewIsActive())
        send("Activate", m_id, snapshot());
}

void PreviewClient::toggle()
{
    if (!m_model || !m_view || m_view->isRenaming())
        return;
    // Keep the desktop selection when the preview window takes keyboard focus.
    setVisible(true);
    send("Toggle", m_id, snapshot());
}

void PreviewClient::setVisible(bool visible)
{
    if (m_previewVisible == visible)
        return;
    m_previewVisible = visible;
    previewVisibleChanged.emit();
}

bool PreviewClient::filterKey(const ui::KeyEvent& event)
{
    if (!m_previewVisible || m_navigating)
        return false;
    if (event.key == ui::key::Escape
        || (m_model && m_model->selectionCount() > 1 && event.modifiers == ui::NoModifier
            && (event.key == ui::key::Left || event.key == ui::key::Right))) {
        send("Key", m_id, previewkeys::toWireKey(event.key), previewkeys::toWireModifiers(event.modifiers));
        return true;
    }
    return false;
}

} // namespace fm
