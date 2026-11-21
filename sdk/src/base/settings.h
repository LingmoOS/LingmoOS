// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Lingmo::base {

// An INI file in the common desktop dialect, read and written both ways with
// other programs: keys outside a group in [General], "group/key" keys, %XX
// key escaping and C-style value escaping, comma-separated lists,
// @Invalid() for an empty list, and @Size, @Point and @Rect values. Values
// written by others that are not understood here are kept unchanged. A
// write sorts the groups and keys.
class Settings
{
public:
    struct Size
    {
        int width = 0;
        int height = 0;
        bool operator==(const Size&) const = default;
    };
    struct Point
    {
        int x = 0;
        int y = 0;
        bool operator==(const Point&) const = default;
    };
    struct Rect
    {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool operator==(const Rect&) const = default;
    };

    // Reads the file; a missing one is empty.
    explicit Settings(std::string path);
    // Writes what changed, as sync() does.
    ~Settings();

    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    const std::string& path() const { return m_path; }

    bool contains(const std::string& key) const;
    // The keys directly in a group, "" being the top level, and its groups.
    std::vector<std::string> childKeys(const std::string& group = {}) const;
    std::vector<std::string> childGroups(const std::string& group = {}) const;

    // The fallback when the key is missing or its value is of another type.
    std::string stringValue(const std::string& key, const std::string& fallback = {}) const;
    bool boolValue(const std::string& key, bool fallback = false) const;
    int64_t intValue(const std::string& key, int64_t fallback = 0) const;
    double doubleValue(const std::string& key, double fallback = 0) const;
    std::vector<std::string> stringListValue(const std::string& key,
                                             const std::vector<std::string>& fallback = {}) const;
    std::optional<Size> sizeValue(const std::string& key) const;
    std::optional<Point> pointValue(const std::string& key) const;
    std::optional<Rect> rectValue(const std::string& key) const;

    void setValue(const std::string& key, std::string_view value);
    void setValue(const std::string& key, const char* value) { setValue(key, std::string_view(value)); }
    template<typename T>
        requires std::is_arithmetic_v<T>
    void setValue(const std::string& key, T value)
    {
        if constexpr (std::is_same_v<T, bool>)
            setRaw(key, value ? "true" : "false");
        else if constexpr (std::is_integral_v<T>)
            setRaw(key, std::to_string(value));
        else
            setRaw(key, formatDouble(double(value)));
    }
    void setValue(const std::string& key, const std::vector<std::string>& value);
    void setValue(const std::string& key, const Size& value);
    void setValue(const std::string& key, const Point& value);
    void setValue(const std::string& key, const Rect& value);
    // Removes the key, or the group and everything in it.
    void remove(const std::string& key);

    // Writes the changes into what the file holds by now, which another
    // program may have written since it was read. False when it cannot.
    bool sync();
    // Reads the file again; changes not written yet stay on top.
    void reload();

    // The value as written in the file, escaped; for tests.
    std::optional<std::string> rawValue(const std::string& key) const;

private:
    void setRaw(const std::string& key, std::string raw);
    void apply(const std::string& key, const std::optional<std::string>& raw);
    static std::string formatDouble(double value);

    std::string m_path;
    // Escaped as in the file.
    std::map<std::string, std::string> m_values;
    // Not written yet; an empty optional removes the key.
    std::map<std::string, std::optional<std::string>> m_changes;
};

} // namespace Lingmo::base
