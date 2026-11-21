// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/input_method_settings.h"
#include "core/logging.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <set>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

#include "base/file.h"
#include "base/paths.h"
#include "core/preferences.h"
#include "input/dbus_export.h"

namespace Lingmo::Input {

static constexpr auto service = "org.lingmo.InputMethod1";
static constexpr auto path = "/org/lingmo/InputMethod1";
static constexpr auto selectionKey = "InputMethod/inputMethod";

static bool isSpace(char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

static std::string_view trimmed(std::string_view value)
{
    while (!value.empty() && isSpace(value.front()))
        value.remove_prefix(1);
    while (!value.empty() && isSpace(value.back()))
        value.remove_suffix(1);
    return value;
}

static std::string unescape(std::string_view value)
{
    std::string result;
    result.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c != '\\' || i + 1 == value.size()) {
            result += c;
            continue;
        }
        switch (value[++i]) {
        case 's':
            result += ' ';
            break;
        case 'n':
            result += '\n';
            break;
        case 't':
            result += '\t';
            break;
        case 'r':
            result += '\r';
            break;
        case '\\':
            result += '\\';
            break;
        default:
            result += '\\';
            result += value[i];
            break;
        }
    }
    return result;
}

static std::map<std::string, std::string> readDesktopEntry(const std::string &fileName)
{
    const std::optional<std::string> contents = base::file::read(fileName);
    if (!contents)
        return {};
    std::map<std::string, std::string> entry;
    bool inGroup = false;
    std::string_view rest = *contents;
    while (!rest.empty()) {
        const size_t end = rest.find('\n');
        const std::string_view line = trimmed(rest.substr(0, end));
        rest = end == std::string_view::npos ? std::string_view() : rest.substr(end + 1);
        if (line.empty() || line.front() == '#')
            continue;
        if (line.front() == '[') {
            if (inGroup)
                break;
            inGroup = line == "[Desktop Entry]";
            continue;
        }
        const size_t separator = line.find('=');
        if (!inGroup || separator == 0 || separator == std::string_view::npos)
            continue;
        const std::string key(trimmed(line.substr(0, separator)));
        if (key.find('[') == std::string::npos && !entry.contains(key))
            entry.emplace(key, trimmed(line.substr(separator + 1)));
    }
    return entry;
}

static std::string value(const std::map<std::string, std::string> &entry, const std::string &key,
                         const std::string &fallback = {})
{
    const auto it = entry.find(key);
    return it == entry.end() ? fallback : it->second;
}

static std::string resolveExecutable(const std::string &program)
{
    if (program.empty() || program.front() != '/')
        return base::paths::findExecutable(program);
    struct stat info;
    return stat(program.c_str(), &info) == 0 && S_ISREG(info.st_mode)
                    && access(program.c_str(), X_OK) == 0
            ? program
            : std::string();
}

static bool hasDesktopSuffix(const std::string &name)
{
    constexpr std::string_view suffix = ".desktop";
    return name.size() >= suffix.size()
            && std::equal(suffix.begin(), suffix.end(), name.end() - suffix.size(),
                          [](char a, char b) { return a == std::tolower(static_cast<unsigned char>(b)); });
}

std::vector<std::string> parseDesktopExec(const std::string &exec)
{
    const std::string value = unescape(exec);
    std::vector<std::string> arguments;
    std::string current;
    bool quoted = false;
    bool pending = false;
    for (size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (quoted) {
            if (c == '\\' && i + 1 < value.size()
                && std::string_view("\"`$\\").find(value[i + 1]) != std::string_view::npos) {
                current += value[++i];
            } else if (c == '"') {
                quoted = false;
            } else {
                current += c;
            }
        } else if (c == '"') {
            quoted = pending = true;
        } else if (isSpace(c)) {
            if (pending || !current.empty())
                arguments.push_back(std::exchange(current, {}));
            pending = false;
        } else {
            current += c;
        }
    }
    if (quoted)
        return {};
    if (pending || !current.empty())
        arguments.push_back(current);

    std::vector<std::string> command;
    for (std::string argument : arguments) {
        // An input method is started without files or URLs, so field codes expand to nothing.
        if (argument.size() == 2 && argument[0] == '%' && argument[1] != '%')
            continue;
        for (size_t at = argument.find("%%"); at != std::string::npos;
             at = argument.find("%%", at + 1))
            argument.replace(at, 2, "%");
        command.push_back(std::move(argument));
    }
    return command;
}

// The desktop files under `directory`, sorted, skipping hidden entries and
// not following links to directories.
static std::vector<std::string> desktopFiles(const std::string &directory)
{
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    std::error_code error;
    for (fs::recursive_directory_iterator it(directory, error), end; !error && it != end;
         it.increment(error)) {
        const std::string name = it->path().filename().string();
        if (name.starts_with('.')) {
            if (it->is_directory(error))
                it.disable_recursion_pending();
            continue;
        }
        if (it->is_regular_file(error) && hasDesktopSuffix(name))
            files.push_back(it->path().string());
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::vector<InputMethodProvider>
findInputMethodProviders(const std::vector<std::string> &applicationDirs)
{
    std::vector<InputMethodProvider> providers;
    std::set<std::string> seen;
    for (const auto &directory : applicationDirs) {
        for (const auto &file : desktopFiles(directory)) {
            std::string id = std::filesystem::path(file)
                                     .lexically_relative(directory)
                                     .lexically_normal()
                                     .string();
            std::replace(id.begin(), id.end(), '/', '-');
            // An earlier directory shadows the id even when its entry is not a provider.
            if (!seen.insert(id).second)
                continue;
            const auto entry = readDesktopEntry(file);
            if (value(entry, "X-KDE-Wayland-VirtualKeyboard") != "true"
                || value(entry, "Type", "Application") != "Application"
                || value(entry, "Hidden") == "true")
                continue;
            const std::string tryExec = unescape(value(entry, "TryExec"));
            if (!tryExec.empty() && resolveExecutable(tryExec).empty())
                continue;
            auto command = parseDesktopExec(value(entry, "Exec"));
            if (command.empty())
                continue;
            command.front() = resolveExecutable(command.front());
            if (command.front().empty())
                continue;
            providers.push_back({id, file, command});
        }
    }
    std::sort(providers.begin(), providers.end(), [](const auto &a, const auto &b) {
        return a.id < b.id;
    });
    return providers;
}

static std::vector<std::string> standardApplicationDirs()
{
    std::vector<std::string> directories {base::paths::dataHome() + "/applications"};
    for (const std::string &directory : base::paths::dataDirectories())
        directories.push_back(directory + "/applications");
    return directories;
}

InputMethodSettings::InputMethodSettings(const std::string &settingsFile,
                                         const std::vector<std::string> &applicationDirs)
    : m_settings(settingsFile.empty() ? preferencesFile("compositor") : settingsFile)
    , m_applicationDirs(applicationDirs.empty() ? standardApplicationDirs() : applicationDirs)
{
    m_selected = m_settings.stringValue(selectionKey);
}

InputMethodSettings::~InputMethodSettings() = default;

bool InputMethodSettings::registerService()
{
    if (m_object)
        return true;
    auto exported = DBusExport::create(service, path, service);
    if (!exported) {
        LM_WARNING(lingmoInput) << "Cannot export input method settings: no session bus";
        return false;
    }
    base::dbus::Object &object = exported->object();
    object.addProperty("inputMethod", [this] { return inputMethod(); });
    object.addProperty("running", [this] { return m_running; });
    object.addMethod("availableInputMethods", [this] { return availableInputMethods(); });
    object.addMethod("setInputMethod",
                     [this](const std::string &id) { return setInputMethod(id); },
                     base::dbus::ArgumentNames{{"id"}, {"result"}});
    for (const char *member : {"inputMethodChanged", "runningChanged", "commandChanged"})
        object.addSignal(member, "");
    if (!exported->publish("input method settings"))
        return false;
    m_object = std::move(exported);
    return true;
}

void InputMethodSettings::notify(const base::Signal<> &signal, const char *member)
{
    signal();
    if (m_object)
        m_object->emitSignal(member);
}

std::optional<InputMethodProvider> InputMethodSettings::provider(const std::string &id) const
{
    if (id.empty())
        return std::nullopt;
    for (const auto &provider : findInputMethodProviders(m_applicationDirs)) {
        if (provider.id == id)
            return provider;
    }
    return std::nullopt;
}

std::string InputMethodSettings::inputMethod() const
{
    return provider(m_selected) ? m_selected : std::string();
}

std::vector<std::string> InputMethodSettings::command() const
{
    const auto selected = provider(m_selected);
    return selected ? selected->command : std::vector<std::string>();
}

void InputMethodSettings::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;
    notify(runningChanged, "runningChanged");
}

void InputMethodSettings::overrideInputMethod(const std::string &id)
{
    m_selected = id;
    notify(inputMethodChanged, "inputMethodChanged");
    notify(commandChanged, "commandChanged");
}

std::vector<std::string> InputMethodSettings::availableInputMethods() const
{
    std::vector<std::string> ids;
    for (const auto &provider : findInputMethodProviders(m_applicationDirs))
        ids.push_back(provider.id);
    return ids;
}

bool InputMethodSettings::setInputMethod(const std::string &id)
{
    if (!id.empty() && !provider(id))
        return false;
    const bool changed = inputMethod() != id;
    m_selected = id;
    m_settings.setValue(selectionKey, id);
    if (!m_settings.sync())
        LM_WARNING(lingmoInput) << "Cannot persist input method settings to" << m_settings.path();
    if (changed)
        notify(inputMethodChanged, "inputMethodChanged");
    // Also sent for an unchanged selection, so selecting it again restarts a stopped method.
    notify(commandChanged, "commandChanged");
    return true;
}

} // namespace Lingmo::Input
