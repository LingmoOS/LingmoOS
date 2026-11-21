// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/chooserservice.h"
#include "chooser/chooserwindow.h"
#include "chooser/filechooser.h"

#include "base/event_loop.h"
#include "base/json.h"

#include <algorithm>

namespace fm {

struct ChooserService::Dialog
{
    std::string caller;
    std::string id;
    std::unique_ptr<FileChooser> chooser;
    std::unique_ptr<ChooserWindow> window;
    // The delayed reply's call.
    std::unique_ptr<base::dbus::MethodCall> call;
    base::Subscription finished;
};

ChooserService::ChooserService(base::dbus::Connection* bus)
    : m_bus(bus)
    , m_object(bus, "/org/lingmo/FileChooser", "org.lingmo.FileChooser")
{
    m_object.addMethod("Open", "ss", "s", [this](base::dbus::MethodCall& call) { open(call); });
    m_object.addMethod("Close", "s", "", [this](base::dbus::MethodCall& call) {
        std::string id;
        call.read(id);
        close(call.message().sender(), id);
        call.reply();
    });
    m_object.publish();
}

ChooserService::~ChooserService()
{
    *m_alive = false;
}

void ChooserService::open(base::dbus::MethodCall& call)
{
    std::string id;
    std::string text;
    call.read(id, text);
    const std::string caller = call.message().sender();
    const std::optional<base::json::Value> request = base::json::Value::parse(text);
    const bool duplicate = std::any_of(m_dialogs.begin(), m_dialogs.end(), [&](const auto& dialog) {
        return dialog->caller == caller && dialog->id == id;
    });
    if (!request || !request->isObject() || !FileChooser::isValidRequest(*request) || duplicate) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Invalid file chooser request");
        return;
    }

    auto dialog = std::make_unique<Dialog>();
    Dialog* raw = dialog.get();
    raw->caller = caller;
    raw->id = id;
    raw->call = std::make_unique<base::dbus::MethodCall>(call);
    raw->chooser = std::make_unique<FileChooser>(*request);
    raw->finished = raw->chooser->finished.connect(
            [this, raw](const base::json::Value& result) { finished(raw, result.serialize()); });
    raw->window = std::make_unique<ChooserWindow>(*raw->chooser);
    m_dialogs.push_back(std::move(dialog));
    watch(caller);
    raw->window->show();
}

void ChooserService::close(const std::string& caller, const std::string& id)
{
    for (const auto& dialog : m_dialogs) {
        if (dialog->caller == caller && dialog->id == id) {
            dialog->chooser->cancel();
            return;
        }
    }
}

void ChooserService::finished(Dialog* dialog, const std::string& result)
{
    dialog->call->reply(result);
    auto it = std::find_if(m_dialogs.begin(), m_dialogs.end(),
                           [dialog](const auto& entry) { return entry.get() == dialog; });
    if (it == m_dialogs.end())
        return;
    const std::string caller = dialog->caller;
    // The dialog goes after the event that finished it.
    std::shared_ptr<Dialog> gone(std::move(*it));
    m_dialogs.erase(it);
    base::EventLoop::current()->post([gone] {});
    const bool others = std::any_of(m_dialogs.begin(), m_dialogs.end(),
                                    [&caller](const auto& entry) { return entry->caller == caller; });
    if (!others)
        m_callers.erase(caller);
}

// Nobody is left to read the result once the caller leaves the bus, so its dialogs go.
void ChooserService::watch(const std::string& caller)
{
    if (m_callers.contains(caller))
        return;
    m_callers[caller] = m_bus->watchName(caller, [this, caller, alive = m_alive](const std::string& owner) {
        if (!*alive || !owner.empty())
            return;
        base::EventLoop::current()->post([this, caller, alive] {
            if (!*alive)
                return;
            std::vector<FileChooser*> choosers;
            for (const auto& dialog : m_dialogs) {
                if (dialog->caller == caller)
                    choosers.push_back(dialog->chooser.get());
            }
            for (FileChooser* chooser : choosers)
                chooser->cancel();
        });
    });
}

} // namespace fm
