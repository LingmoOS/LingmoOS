namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/file_chooser.h"

#include <algorithm>
#include <cstdint>
#include <random>
#include <tuple>
#include <unistd.h>
#include <utility>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/translation.h"
#include "ui/window.h"

namespace Lingmo::ui {
namespace dbus = Lingmo::base::dbus;

static const char* const kPortalService = "org.freedesktop.portal.Desktop";
static const char* const kPortalPath = "/org/freedesktop/portal/desktop";
static const char* const kFileChooserInterface = "org.freedesktop.portal.FileChooser";
static const char* const kRequestInterface = "org.freedesktop.portal.Request";

// (us): 0 a glob pattern, 1 a MIME type.
using PortalRule = std::tuple<uint32_t, std::string>;
using PortalFilter = std::tuple<std::string, std::vector<PortalRule>>;
// (ss): an option of a choice, or a choice and the option picked.
using PortalOption = std::tuple<std::string, std::string>;
// (ssa(ss)s): id, label, options (none for a check box) and the initial selection.
using PortalChoice = std::tuple<std::string, std::string, std::vector<PortalOption>, std::string>;

struct FileChooser::Request
{
    // Empty until the call is made.
    std::string path;
    std::string token;
    bool save = false;
    std::string title;
    dbus::VariantMap options;
    dbus::Match response;
    dbus::PendingCall call;
};

FileChooser::Filter FileChooser::Filter::fromNameFilter(std::string_view nameFilter)
{
    Filter filter;
    const size_t open = nameFilter.rfind('(');
    const size_t close = nameFilter.rfind(')');
    std::string_view patterns = nameFilter;
    if (open != std::string_view::npos && close != std::string_view::npos && close > open
        && close == nameFilter.find_last_not_of(" \t")) {
        patterns = nameFilter.substr(open + 1, close - open - 1);
        std::string_view name = nameFilter.substr(0, open);
        while (!name.empty() && (name.back() == ' ' || name.back() == '\t'))
            name.remove_suffix(1);
        while (!name.empty() && (name.front() == ' ' || name.front() == '\t'))
            name.remove_prefix(1);
        filter.name = std::string(name);
    }
    size_t start = 0;
    while (start < patterns.size()) {
        const size_t end = std::min(patterns.find(' ', start), patterns.size());
        if (end > start)
            filter.patterns.emplace_back(patterns.substr(start, end - start));
        start = end + 1;
    }
    // A filter without a name is named after its patterns.
    if (filter.name.empty()) {
        for (const std::string& pattern : filter.patterns)
            filter.name += (filter.name.empty() ? "" : " ") + pattern;
    }
    return filter;
}

static PortalFilter portalFilter(const FileChooser::Filter& filter)
{
    std::vector<PortalRule> rules;
    for (const std::string& mimeType : filter.mimeTypes)
        rules.emplace_back(1, mimeType);
    for (const std::string& pattern : filter.patterns)
        rules.emplace_back(0, pattern);
    return {filter.name, rules};
}

// The portal takes paths as NUL-terminated bytes.
static std::vector<uint8_t> portalPath(const std::string& path)
{
    std::vector<uint8_t> bytes(path.begin(), path.end());
    bytes.push_back(0);
    return bytes;
}

// "file:///a%20b" to "/a b"; empty for anything not local.
static std::string localPath(const std::string& uri)
{
    static const std::string scheme = "file://";
    if (!uri.starts_with(scheme))
        return {};
    std::string_view rest = std::string_view(uri).substr(scheme.size());
    // An authority of "localhost" or none.
    if (rest.starts_with("localhost/"))
        rest.remove_prefix(9);
    if (!rest.starts_with('/'))
        return {};
    std::string path;
    for (size_t i = 0; i < rest.size(); ++i) {
        const auto hex = [](char c) {
            return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
        };
        if (rest[i] == '%' && i + 2 < rest.size() && hex(rest[i + 1]) >= 0 && hex(rest[i + 2]) >= 0) {
            path += char(hex(rest[i + 1]) * 16 + hex(rest[i + 2]));
            i += 2;
        } else {
            path += rest[i];
        }
    }
    return path;
}

FileChooser::FileChooser(Window* parent)
    : m_parent(parent)
{
}

FileChooser::~FileChooser()
{
    close();
}

void FileChooser::setMode(Mode mode)
{
    m_mode = mode;
}

void FileChooser::setTitle(std::string title)
{
    m_title = std::move(title);
}

void FileChooser::setAcceptLabel(std::string label)
{
    m_acceptLabel = std::move(label);
}

void FileChooser::setFilters(std::vector<Filter> filters)
{
    m_filters = std::move(filters);
}

void FileChooser::setCurrentFilter(std::string name)
{
    m_currentFilter = std::move(name);
}

void FileChooser::setChoices(std::vector<Choice> choices)
{
    m_choices = std::move(choices);
}

void FileChooser::setCurrentFolder(std::string path)
{
    m_currentFolder = std::move(path);
}

void FileChooser::setCurrentFile(std::string path)
{
    m_currentFile = std::move(path);
}

void FileChooser::setModal(bool modal)
{
    m_modal = modal;
}

bool FileChooser::isOpen() const
{
    return m_request != nullptr;
}

void FileChooser::close()
{
    if (!m_request)
        return;
    dbus::Connection* bus = Application::instance() ? Application::instance()->sessionBus() : nullptr;
    if (bus && !m_request->path.empty())
        bus->send(bus->newMethodCall(kPortalService, m_request->path, kRequestInterface, "Close"));
    m_request.reset();
}

void FileChooser::open()
{
    close();
    dbus::Connection* bus = Application::instance() ? Application::instance()->sessionBus() : nullptr;
    if (!bus) {
        // Answered from the loop, as the portal's answer would be.
        base::EventLoop::current()->post([callback = onRejected] {
            if (callback)
                callback();
        });
        return;
    }

    const bool save = m_mode == Mode::SaveFile;
    const bool folder = m_mode == Mode::OpenFolder;
    std::random_device random;
    const std::string token = "lingmo" + std::to_string(random());
    dbus::VariantMap options;
    options["handle_token"] = dbus::Variant(token);
    options["modal"] = dbus::Variant(m_modal);
    if (!save) {
        options["multiple"] = dbus::Variant(m_mode == Mode::OpenFiles);
        options["directory"] = dbus::Variant(folder);
    }
    if (!m_acceptLabel.empty())
        options["accept_label"] = dbus::Variant(m_acceptLabel);
    if (!m_filters.empty()) {
        std::vector<PortalFilter> filters;
        for (const Filter& filter : m_filters) {
            filters.push_back(portalFilter(filter));
            if (filter.name == m_currentFilter)
                options["current_filter"] = dbus::Variant(portalFilter(filter));
        }
        options["filters"] = dbus::Variant(filters);
    }
    if (!m_choices.empty()) {
        std::vector<PortalChoice> choices;
        for (const Choice& choice : m_choices) {
            std::vector<PortalOption> values;
            for (const auto& [id, label] : choice.options)
                values.emplace_back(id, label);
            choices.emplace_back(choice.id, choice.label, std::move(values), choice.selected);
        }
        options["choices"] = dbus::Variant(choices);
    }
    if (!m_currentFolder.empty())
        options["current_folder"] = dbus::Variant(portalPath(m_currentFolder));
    if (save && !m_currentFile.empty()) {
        if (access(m_currentFile.c_str(), F_OK) == 0)
            options["current_file"] = dbus::Variant(portalPath(m_currentFile));
        const size_t slash = m_currentFile.rfind('/');
        options["current_name"] = dbus::Variant(
                slash == std::string::npos ? m_currentFile : m_currentFile.substr(slash + 1));
    }
    std::string title = m_title;
    if (title.empty())
        title = save ? tr("Save File") : folder ? tr("Open Folder") : tr("Open File");

    auto request = std::make_unique<Request>();
    request->token = token;
    request->save = save;
    request->title = std::move(title);
    request->options = std::move(options);
    Request* current = request.get();
    m_request = std::move(request);
    if (!m_parent) {
        call(current, {});
        return;
    }
    // The dialog runs in another process, so it is parented onto the
    // window's exported handle, which comes back asynchronously.
    m_parent->exportHandle([this, alive = std::weak_ptr<char>(m_alive), current](const std::string& handle) {
        if (alive.lock() && m_request.get() == current)
            call(current, handle);
    });
}

void FileChooser::call(Request* current, const std::string& parentWindow)
{
    dbus::Connection* bus = Application::instance() ? Application::instance()->sessionBus() : nullptr;
    if (!bus)
        return;
    // The request's path follows from the token, so its answer is listened for
    // before the call: a portal that answers at once is not missed.
    std::string sender = bus->uniqueName();
    if (sender.starts_with(':'))
        sender.erase(0, 1);
    std::replace(sender.begin(), sender.end(), '.', '_');
    const auto listen = [this, bus, current](const std::string& path) {
        current->path = path;
        dbus::MatchRule rule;
        rule.sender = kPortalService;
        rule.path = path;
        rule.interface = kRequestInterface;
        rule.member = "Response";
        current->response = bus->subscribe(rule, [this, current](const dbus::Message& message) {
            if (m_request.get() != current)
                return;
            uint32_t response = 2;
            dbus::VariantMap results;
            message.read(response, results);
            // The subscription answering goes once the loop is back.
            base::EventLoop::current()->post([finished = std::shared_ptr<Request>(std::move(m_request))] {});
            std::vector<std::string> paths;
            std::string filter;
            if (response == 0) {
                if (const auto uris = results.find("uris"); uris != results.end()) {
                    for (const std::string& uri : uris->second.value<std::vector<std::string>>({})) {
                        const std::string path = localPath(uri);
                        if (!path.empty())
                            paths.push_back(path);
                    }
                }
                if (const auto chosen = results.find("current_filter"); chosen != results.end())
                    filter = std::get<0>(chosen->second.value<PortalFilter>({}));
                m_chosenOptions.clear();
                if (const auto chosen = results.find("choices"); chosen != results.end()) {
                    for (const auto& [id, value] : chosen->second.value<std::vector<PortalOption>>({}))
                        m_chosenOptions.emplace_back(id, value);
                }
            }
            if (response == 0) {
                if (auto accepted = onAccepted)
                    accepted(paths, filter);
            } else if (auto rejected = onRejected) {
                rejected();
            }
        });
    };
    listen(std::string(kPortalPath) + "/request/" + sender + "/" + current->token);

    dbus::Message message = bus->newMethodCall(kPortalService, kPortalPath, kFileChooserInterface,
                                                     current->save ? "SaveFile" : "OpenFile");
    message.append(parentWindow, current->title, current->options);
    current->call = bus->callAsync(message, [this, current, listen](const dbus::Reply& reply) {
        if (m_request.get() != current)
            return;
        if (reply.isError()) {
            // The call answering goes once the loop is back.
            base::EventLoop::current()->post([finished = std::shared_ptr<Request>(std::move(m_request))] {});
            if (auto rejected = onRejected)
                rejected();
            return;
        }
        // An older portal ignores the token and picks the path itself.
        const std::string path = reply.value<dbus::ObjectPath>(0).value_or(dbus::ObjectPath{}).path;
        if (!path.empty() && path != current->path)
            listen(path);
    });
}

} // namespace Lingmo::ui
