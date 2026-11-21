#include "filechooserportal.h"
#include "filechoosertypes.h"
#include "options.h"
#include "requestobject.h"

#include "base/json.h"
#include "base/logging.h"
#include "base/strings.h"

#include <systemd/sd-bus.h>

#include <chrono>
#include <climits>
#include <cmath>
#include <tuple>
#include <vector>

using namespace Lingmo;
using base::dbus::MethodCall;
using base::dbus::Variant;
using base::dbus::VariantMap;
using base::json::Value;

static const std::string chooserService = "org.lingmo.FileChooser";
static const std::string chooserPath = "/org/lingmo/FileChooser";

static Value filterJson(const FileFilter &filter)
{
    Value::Array rules;
    for (const auto &[type, value] : std::get<1>(filter)) {
        Value rule = Value::object();
        rule.insert("type", int64_t(type));
        rule.insert("value", value);
        rules.push_back(std::move(rule));
    }
    Value json = Value::object();
    json.insert("name", std::get<0>(filter));
    json.insert("rules", std::move(rules));
    return json;
}

static std::string localPath(const VariantMap &options, const std::string &key)
{
    const auto it = options.find(key);
    if (it == options.end())
        return {};
    std::string bytes = toBytes(it->second);
    if (!bytes.empty() && bytes.back() == '\0')
        bytes.pop_back();
    return base::strings::sanitizedUtf8(bytes);
}

// A whole number that fits an int; the fallback for anything else.
static int jsonInt(const Value &value, int fallback)
{
    if (!value.isNumber())
        return fallback;
    const double number = value.toDouble();
    if (number != std::trunc(number) || number < INT_MIN || number > INT_MAX)
        return fallback;
    return int(number);
}

static bool isHex(char c)
{
    return base::strings::isAsciiDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int hexValue(char c)
{
    return base::strings::isAsciiDigit(c) ? c - '0' : base::strings::toAsciiLower(c) - 'a' + 10;
}

static bool isUnreserved(unsigned char c)
{
    return base::strings::isAsciiAlpha(char(c)) || base::strings::isAsciiDigit(char(c)) || c == '-' || c == '.'
            || c == '_' || c == '~';
}

static void appendEscaped(std::string &out, unsigned char c)
{
    static const char digits[] = "0123456789ABCDEF";
    out += '%';
    out += digits[c >> 4];
    out += digits[c & 15];
}

// Escapes of unreserved characters decoded, other escapes upper-cased, and
// spaces, controls, non-ASCII bytes and the characters URLs never hold raw
// escaped.
static std::string recoded(std::string_view text)
{
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        const auto c = static_cast<unsigned char>(text[i]);
        if (c == '%') {
            if (i + 2 < text.size() && isHex(text[i + 1]) && isHex(text[i + 2])) {
                const auto decoded = static_cast<unsigned char>(hexValue(text[i + 1]) * 16 + hexValue(text[i + 2]));
                if (isUnreserved(decoded))
                    out += char(decoded);
                else
                    appendEscaped(out, decoded);
                i += 2;
            } else {
                appendEscaped(out, c);
            }
        } else if (c <= 0x20 || c >= 0x7f || std::string_view("<>\"\\^`{|}").find(char(c)) != std::string_view::npos) {
            appendEscaped(out, c);
        } else {
            out += char(c);
        }
    }
    return out;
}

std::optional<std::string> normalizedFileUrl(std::string_view url)
{
    const size_t colon = url.find(':');
    if (colon == std::string_view::npos || !base::strings::equalsIgnoreAsciiCase(url.substr(0, colon), "file"))
        return std::nullopt;
    std::string_view rest = url.substr(colon + 1);
    if (base::strings::startsWith(rest, "//")) {
        rest.remove_prefix(2);
        const size_t end = rest.find_first_of("/?#");
        if (!rest.empty() && end != 0)
            return std::nullopt;
    }
    const size_t pathEnd = rest.find_first_of("?#");
    const std::string_view path = rest.substr(0, pathEnd);
    if (!base::strings::startsWith(path, "/"))
        return std::nullopt;

    std::string result = "file://" + recoded(path);
    if (pathEnd != std::string_view::npos) {
        std::string_view tail = rest.substr(pathEnd);
        const size_t hash = tail.find('#');
        if (tail.front() == '?') {
            result += '?';
            result += recoded(tail.substr(1, hash == std::string_view::npos ? std::string_view::npos : hash - 1));
        }
        if (hash != std::string_view::npos) {
            result += '#';
            result += recoded(tail.substr(hash + 1));
        }
    }
    return result;
}

class FileChooserRequest
{
public:
    FileChooserRequest(FileChooserPortal *portal, const MethodCall &call, const std::string &path)
        : portal(portal)
        , bus(portal->m_bus)
        , call(call)
        , requester(call.message().sender())
        , path(path)
    {
    }

    bool exportObject()
    {
        object = exportRequest(bus, path, [this](MethodCall &close) { Close(close); });
        if (!object)
            return false;
        requesterWatch = watchRequester(bus, requester, [this] { finish(1); });
        return true;
    }

    void start(const std::string &payload)
    {
        // The session's file manager daemon shows the dialog; the session
        // restarts it when it dies, so nothing else is started for it.
        base::dbus::Message open = bus->newMethodCall(chooserService, chooserPath, chooserService, "Open");
        open.append(path, payload);
        sd_bus_message_set_auto_start(open.get(), 0);
        isOpen = true;
        // The dialog stays open as long as the user leaves it.
        pending = bus->callAsync(
                open,
                [this](const base::dbus::Reply &reply) {
                    isOpen = false;
                    const std::optional<std::string> result = reply.value<std::string>();
                    if (!result) {
                        LM_WARNING(base::LogCategory::defaultCategory())
                                << "File Manager chooser failed:" << reply.error().message;
                        finish(2);
                        return;
                    }
                    readResult(*result);
                },
                std::chrono::milliseconds(INT_MAX));
    }

    void finish(uint32_t response, const VariantMap &results = {})
    {
        if (finished)
            return;
        finished = true;
        if (isOpen) {
            base::dbus::Message close = bus->newMethodCall(chooserService, chooserPath, chooserService, "Close");
            close.append(path);
            sd_bus_message_set_auto_start(close.get(), 0);
            bus->send(close);
        }
        object.reset();
        call.reply(response, results);
        portal->deleteLater(this);
    }

    void Close(MethodCall &close)
    {
        if (close.message().sender() != requester) {
            close.replyError("org.freedesktop.DBus.Error.AccessDenied",
                             "Only the requesting service may close this dialog");
            return;
        }
        finish(1);
    }

private:
    void readResult(const std::string &result)
    {
        if (finished)
            return;
        const std::optional<Value> document =
                result.size() > 1024 * 1024 ? std::nullopt : Value::parse(result);
        if (!document || !document->isObject()) {
            finish(2);
            return;
        }
        const Value &reply = *document;
        const int response = jsonInt(reply["response"], 2);
        if (response != 0) {
            finish(response == 1 ? 1 : 2);
            return;
        }
        std::vector<std::string> uris;
        if (reply["uris"].isArray()) {
            for (const Value &entry : reply["uris"].toArray()) {
                const std::optional<std::string> url = normalizedFileUrl(entry.toString());
                if (!url) {
                    finish(2);
                    return;
                }
                uris.push_back(*url);
            }
        }
        if (uris.empty() || (single && uris.size() != 1)) {
            finish(2);
            return;
        }
        VariantMap results{{"uris", Variant(uris)}};
        const int filterIndex = jsonInt(reply["filter_index"], -1);
        if (filterIndex >= 0 && size_t(filterIndex) < filters.size())
            results["current_filter"] = Variant(filters[size_t(filterIndex)]);
        std::vector<ChoiceOption> selected;
        const Value &values = reply["choices"];
        for (const FileChoice &choice : choices) {
            const std::string &id = std::get<0>(choice);
            const auto &options = std::get<2>(choice);
            const std::string value = values.isObject() ? values[id].toString() : std::string();
            bool valid = options.empty() && (value == "true" || value == "false");
            for (const auto &option : options)
                valid |= std::get<0>(option) == value;
            if (!valid) {
                finish(2);
                return;
            }
            selected.emplace_back(id, value);
        }
        if (!choices.empty())
            results["choices"] = Variant(selected);
        finish(0, results);
    }

public:
    FileChooserPortal *portal;
    base::dbus::Connection *bus;
    MethodCall call;
    std::string requester;
    std::string path;
    std::unique_ptr<base::dbus::Object> object;
    base::dbus::NameWatch requesterWatch;
    base::dbus::PendingCall pending;
    std::vector<FileFilter> filters;
    std::vector<FileChoice> choices;
    bool single = true;
    bool isOpen = false;
    bool finished = false;
};

FileChooserPortal::FileChooserPortal(base::dbus::Connection *bus)
    : m_bus(bus)
    , m_object(bus, "/org/freedesktop/portal/desktop", "org.freedesktop.impl.portal.FileChooser")
{
    m_object.addProperty("version", [this] { return version(); });
    const base::dbus::ArgumentNames names {{"handle", "appId", "parentWindow", "title", "options"},
                                           {"response", "results"}};
    for (const char *mode : {"OpenFile", "SaveFile", "SaveFiles"}) {
        m_object.addMethod(
                mode, "osssa{sv}", "ua{sv}", [this, mode = std::string(mode)](MethodCall &call) { choose(mode, call); },
                names);
    }
}

FileChooserPortal::~FileChooserPortal() = default;

bool FileChooserPortal::publish()
{
    return m_object.publish();
}

void FileChooserPortal::choose(const std::string &mode, MethodCall &call)
{
    base::dbus::ObjectPath handle;
    std::string appId;
    std::string parentWindow;
    std::string title;
    VariantMap options;
    call.read(handle, appId, parentWindow, title, options);

    auto owned = std::make_unique<FileChooserRequest>(this, call, handle.path);
    if (!owned->exportObject()) {
        call.reply(uint32_t(2), VariantMap());
        return;
    }
    FileChooserRequest *request = owned.get();
    m_requests.emplace(request, std::move(owned));

    const auto option = [&options](const std::string &key) {
        const auto it = options.find(key);
        return it == options.end() ? Variant() : it->second;
    };
    const FileFilter preferredFilter = option("current_filter").value<FileFilter>();
    request->filters = option("filters").value<std::vector<FileFilter>>();
    if (request->filters.empty() && options.count("current_filter"))
        request->filters.push_back(preferredFilter);
    request->choices = option("choices").value<std::vector<FileChoice>>();
    request->single = mode == "SaveFile" || (mode == "OpenFile" && !optionBool(options, "multiple", false));

    Value::Array filters;
    int currentFilter = 0;
    const Value preferred = filterJson(preferredFilter);
    for (const FileFilter &filter : request->filters) {
        Value json = filterJson(filter);
        if (json == preferred)
            currentFilter = int(filters.size());
        filters.push_back(std::move(json));
    }
    Value::Array choices;
    for (const FileChoice &choice : request->choices) {
        Value::Array values;
        for (const auto &[id, label] : std::get<2>(choice)) {
            Value value = Value::object();
            value.insert("id", id);
            value.insert("label", label);
            values.push_back(std::move(value));
        }
        Value json = Value::object();
        json.insert("id", std::get<0>(choice));
        json.insert("label", std::get<1>(choice));
        json.insert("options", std::move(values));
        json.insert("selected", std::get<3>(choice));
        choices.push_back(std::move(json));
    }
    Value::Array files;
    for (const std::vector<uint8_t> &name : option("files").value<std::vector<std::vector<uint8_t>>>()) {
        std::string bytes(name.begin(), name.end());
        if (!bytes.empty() && bytes.back() == '\0')
            bytes.pop_back();
        files.push_back(base::strings::sanitizedUtf8(bytes));
    }

    // Keys in alphabetical order, as the chooser has always received them.
    Value payload = Value::object();
    payload.insert("accept_label", optionString(options, "accept_label"));
    payload.insert("app_id", appId);
    payload.insert("choices", std::move(choices));
    payload.insert("current_file", localPath(options, "current_file"));
    payload.insert("current_folder", localPath(options, "current_folder"));
    payload.insert("current_name", optionString(options, "current_name"));
    payload.insert("directory", optionBool(options, "directory", false));
    payload.insert("files", std::move(files));
    payload.insert("filter_index", currentFilter);
    payload.insert("filters", std::move(filters));
    payload.insert("modal", optionBool(options, "modal", true));
    payload.insert("mode", mode);
    payload.insert("multiple", optionBool(options, "multiple", false));
    payload.insert("parent_window", parentWindow);
    payload.insert("title", title);
    request->start(payload.serialize());
}

void FileChooserPortal::deleteLater(FileChooserRequest *request)
{
    later(m_alive, [this, request] { m_requests.erase(request); });
}
