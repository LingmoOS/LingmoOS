// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/logging.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <sstream>
#include <unistd.h>
#include <vector>

#include "base/file.h"
#include "base/strings.h"

namespace Lingmo::base {
namespace strings = Lingmo::base::strings;

static constexpr unsigned kFatalBit = 1u << unsigned(LogLevel::Fatal);

static unsigned bitsFrom(LogLevel minimum)
{
    unsigned bits = kFatalBit;
    for (unsigned level = unsigned(minimum); level < unsigned(LogLevel::Fatal); ++level)
        bits |= 1u << level;
    return bits;
}

struct LogRule
{
    std::string pattern;
    unsigned levels = 0;
    bool enabled = false;

    bool matches(std::string_view name) const
    {
        const bool leading = pattern.starts_with('*');
        const bool trailing = pattern.size() > 1 && pattern.ends_with('*');
        std::string_view core = pattern;
        if (leading)
            core.remove_prefix(1);
        if (trailing)
            core.remove_suffix(1);
        if (leading && trailing)
            return name.find(core) != std::string_view::npos;
        if (leading)
            return name.ends_with(core);
        if (trailing)
            return name.starts_with(core);
        return name == core;
    }
};

struct LogRegistry
{
    std::mutex mutex;
    std::vector<LogCategory*> categories;
    std::vector<LogRule> rules;
    logging::Handler handler;

    static LogRegistry& instance()
    {
        // Never destroyed: categories with static storage unregister after
        // ordinary statics are gone.
        static LogRegistry* registry = new LogRegistry;
        return *registry;
    }

    // Under the mutex.
    void apply(LogCategory& category)
    {
        unsigned bits = bitsFrom(category.m_minimum);
        for (const LogRule& rule : rules) {
            if (!rule.matches(category.m_name))
                continue;
            if (rule.enabled)
                bits |= rule.levels;
            else
                bits &= ~rule.levels;
        }
        category.m_enabled.store(bits | kFatalBit, std::memory_order_relaxed);
    }
};

LogCategory::LogCategory(const char* name, LogLevel minimum)
    : m_name(name)
    , m_minimum(minimum)
    , m_enabled(bitsFrom(minimum))
{
    LogRegistry& registry = LogRegistry::instance();
    std::lock_guard lock(registry.mutex);
    registry.categories.push_back(this);
    registry.apply(*this);
}

LogCategory::~LogCategory()
{
    LogRegistry& registry = LogRegistry::instance();
    std::lock_guard lock(registry.mutex);
    std::erase(registry.categories, this);
}

void LogCategory::setEnabled(LogLevel level, bool enabled)
{
    if (level == LogLevel::Fatal)
        return;
    if (enabled)
        m_enabled.fetch_or(1u << unsigned(level), std::memory_order_relaxed);
    else
        m_enabled.fetch_and(~(1u << unsigned(level)), std::memory_order_relaxed);
}

void LogCategory::setMinimumLevel(LogLevel minimum)
{
    m_enabled.store(bitsFrom(minimum), std::memory_order_relaxed);
}

LogCategory& LogCategory::defaultCategory()
{
    static LogCategory* category = new LogCategory("default");
    return *category;
}

namespace logging {

static std::optional<LogLevel> levelFromName(std::string_view name)
{
    if (name == "debug")
        return LogLevel::Debug;
    if (name == "info")
        return LogLevel::Info;
    if (name == "warning")
        return LogLevel::Warning;
    if (name == "critical")
        return LogLevel::Critical;
    return std::nullopt;
}

void setRules(std::string_view text)
{
    std::vector<LogRule> rules;
    for (std::string_view line : strings::splitViews(text, '\n')) {
        for (std::string_view entry : strings::splitViews(line, ';')) {
            entry = strings::trimmed(entry);
            const size_t equals = entry.find('=');
            if (entry.empty() || entry.front() == '#' || equals == std::string_view::npos)
                continue;
            std::string_view pattern = strings::trimmed(entry.substr(0, equals));
            const std::string_view value = strings::trimmed(entry.substr(equals + 1));
            if (pattern.empty() || (value != "true" && value != "false"))
                continue;
            LogRule rule;
            rule.enabled = value == "true";
            rule.levels = bitsFrom(LogLevel::Debug) & ~kFatalBit;
            const size_t dot = pattern.rfind('.');
            if (dot != std::string_view::npos) {
                if (const std::optional<LogLevel> level = levelFromName(pattern.substr(dot + 1))) {
                    rule.levels = 1u << unsigned(*level);
                    pattern = pattern.substr(0, dot);
                }
            }
            rule.pattern = std::string(pattern);
            rules.push_back(std::move(rule));
        }
    }

    LogRegistry& registry = LogRegistry::instance();
    std::lock_guard lock(registry.mutex);
    registry.rules = std::move(rules);
    for (LogCategory* category : registry.categories)
        registry.apply(*category);
}

void setHandler(Handler handler)
{
    LogRegistry& registry = LogRegistry::instance();
    std::lock_guard lock(registry.mutex);
    registry.handler = std::move(handler);
}

std::string formatLine(const LogCategory& category, LogLevel level, std::string_view message)
{
    std::string line;
    if (std::string_view(category.name()) != "default") {
        line += category.name();
        line += ": ";
    }
    switch (level) {
    case LogLevel::Debug:
    case LogLevel::Info:
        break;
    case LogLevel::Warning:
        line += "warning: ";
        break;
    case LogLevel::Critical:
        line += "critical: ";
        break;
    case LogLevel::Fatal:
        line += "fatal: ";
        break;
    }
    line += message;
    return line;
}

} // namespace logging

LogStream::~LogStream()
{
    LogRegistry& registry = LogRegistry::instance();
    logging::Handler handler;
    {
        std::lock_guard lock(registry.mutex);
        handler = registry.handler;
    }
    if (handler) {
        handler(m_category, m_level, m_message);
    } else {
        // One write, so that lines from several threads never interleave.
        std::string line = logging::formatLine(m_category, m_level, m_message);
        line += '\n';
        size_t written = 0;
        while (written < line.size()) {
            const ssize_t result = file::writeWithoutSigpipe(STDERR_FILENO, line.data() + written,
                                                             line.size() - written);
            if (result <= 0)
                break;
            written += size_t(result);
        }
    }
    if (m_level == LogLevel::Fatal)
        std::abort();
}

LogStream& LogStream::append(std::string_view text)
{
    if (m_space && !m_message.empty())
        m_message += ' ';
    m_message += text;
    return *this;
}

LogStream& LogStream::printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    va_list copy;
    va_copy(copy, args);
    const int length = std::vsnprintf(nullptr, 0, format, copy);
    va_end(copy);
    std::string text;
    if (length > 0) {
        text.resize(size_t(length));
        std::vsnprintf(text.data(), size_t(length) + 1, format, args);
    }
    va_end(args);
    return append(text);
}

std::string streamToString(const std::function<void(std::ostream&)>& write)
{
    std::ostringstream stream;
    write(stream);
    return std::move(stream).str();
}

} // namespace Lingmo::base
