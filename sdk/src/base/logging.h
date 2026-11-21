// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <concepts>
#include <cstdint>
#include <format>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace Lingmo::base {

enum class LogLevel { Debug, Info, Warning, Critical, Fatal };

// A named switch for a group of messages, defined once with static storage:
//
//   const LogCategory inputLog("lingmo.input");
//   LM_DEBUG(inputLog) << "button" << code;
//   LM_WARNING(inputLog).format("{} devices", count);
//
// Fatal messages are always written and then abort.
class LogCategory
{
public:
    // Levels from `minimum` up are enabled until rules say otherwise.
    explicit LogCategory(const char* name, LogLevel minimum = LogLevel::Info);
    ~LogCategory();

    LogCategory(const LogCategory&) = delete;
    LogCategory& operator=(const LogCategory&) = delete;

    const char* name() const { return m_name; }
    bool isEnabled(LogLevel level) const
    {
        return m_enabled.load(std::memory_order_relaxed) & (1u << unsigned(level));
    }
    void setEnabled(LogLevel level, bool enabled);
    // Enables `minimum` and the levels above it, disables those below.
    void setMinimumLevel(LogLevel minimum);

    // "default", for messages without a category of their own; Info and up.
    static LogCategory& defaultCategory();

private:
    friend struct LogRegistry;
    void reset();

    const char* m_name;
    LogLevel m_minimum;
    std::atomic<unsigned> m_enabled;
};

namespace logging {

// Rules, one per line or separated by ';': `<category>[.<level>]=true|false`,
// level one of debug, info, warning, critical (all four when left out), and
// '*' as the first or last character of the category matching anything.
// Later rules win; they replace any earlier set and apply to categories
// created afterwards as well. Malformed rules are ignored.
//
//   "lingmo.*.debug=true;lingmo.drm.debug=false"
void setRules(std::string_view rules);

// Receives every enabled message instead of standard error; null restores
// standard error. Called on the thread that logs.
using Handler = std::function<void(const LogCategory& category, LogLevel level, std::string_view message)>;
void setHandler(Handler handler);

// What the default output writes for one message, without the newline:
// "category: message", with "warning: ", "critical: " or "fatal: " before the
// message at those levels; the default category's name is left out.
std::string formatLine(const LogCategory& category, LogLevel level, std::string_view message);

} // namespace logging

template<typename T>
concept StreamWritable = requires(std::ostream& stream, const T& value) { stream << value; };

// Collects one message and writes it when destroyed. Items written with <<
// are separated by spaces unless nospace() was called.
class LogStream
{
public:
    LogStream(const LogCategory& category, LogLevel level) : m_category(category), m_level(level) {}
    ~LogStream();

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    LogStream& space() { m_space = true; return *this; }
    LogStream& nospace() { m_space = false; return *this; }

    LogStream& operator<<(bool value) { return append(value ? "true" : "false"); }
    LogStream& operator<<(char value) { return append(std::string_view(&value, 1)); }
    LogStream& operator<<(const char* value) { return append(value ? value : "(null)"); }
    LogStream& operator<<(std::string_view value) { return append(value); }
    LogStream& operator<<(const std::string& value) { return append(value); }
    LogStream& operator<<(std::nullptr_t) { return append("(null)"); }
    template<typename T>
        requires(std::is_arithmetic_v<T> && !std::same_as<T, bool> && !std::same_as<T, char>)
    LogStream& operator<<(T value) { return append(std::format("{}", value)); }
    template<typename T>
        requires std::is_enum_v<T>
    LogStream& operator<<(T value)
    {
        return append(std::format("{}", static_cast<std::underlying_type_t<T>>(value)));
    }
    LogStream& operator<<(const void* pointer) { return append(std::format("{}", pointer)); }
    // Anything with an std::ostream operator<<, the geometry types among them.
    template<typename T>
        requires(StreamWritable<T> && !std::is_arithmetic_v<T> && !std::is_enum_v<T> && !std::is_pointer_v<T>
                 && !std::convertible_to<T, std::string_view>)
    LogStream& operator<<(const T& value) { return append(streamed(value)); }

    // Appends std::format text as one item.
    template<typename... Args>
    LogStream& format(std::format_string<Args...> format, Args&&... args)
    {
        return append(std::format(format, std::forward<Args>(args)...));
    }
    // Appends printf text as one item.
    LogStream& printf(const char* format, ...) __attribute__((format(printf, 2, 3)));

private:
    LogStream& append(std::string_view text);
    template<typename T>
    static std::string streamed(const T& value);

    const LogCategory& m_category;
    LogLevel m_level;
    std::string m_message;
    bool m_space = true;
};

std::string streamToString(const std::function<void(std::ostream&)>& write);

template<typename T>
std::string LogStream::streamed(const T& value)
{
    return streamToString([&value](std::ostream& stream) { stream << value; });
}

} // namespace Lingmo::base

// The arguments after << are only evaluated when the level is enabled.
#define LM_LOG(category, level) \
    if (!(category).isEnabled(level)) { \
    } else \
        ::Lingmo::base::LogStream((category), (level))

#define LM_DEBUG(category)    LM_LOG(category, ::Lingmo::base::LogLevel::Debug)
#define LM_INFO(category)     LM_LOG(category, ::Lingmo::base::LogLevel::Info)
#define LM_WARNING(category)  LM_LOG(category, ::Lingmo::base::LogLevel::Warning)
#define LM_CRITICAL(category) LM_LOG(category, ::Lingmo::base::LogLevel::Critical)
#define LM_FATAL(category)    LM_LOG(category, ::Lingmo::base::LogLevel::Fatal)
