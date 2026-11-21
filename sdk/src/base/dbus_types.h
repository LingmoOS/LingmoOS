// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <compare>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace Lingmo::base::dbus {

struct ObjectPath
{
    std::string path;

    auto operator<=>(const ObjectPath&) const = default;
};

struct Signature
{
    std::string signature;

    auto operator<=>(const Signature&) const = default;
};

// A file descriptor, closed when the last copy goes.
class UnixFd
{
public:
    UnixFd() = default;

    static UnixFd adopt(int fd);
    // A copy of the descriptor, which the caller keeps owning.
    static UnixFd duplicate(int fd);

    bool isValid() const { return m_fd != nullptr; }
    int fd() const { return m_fd ? *m_fd : -1; }
    // A descriptor of the caller's own.
    int release() const;

    bool operator==(const UnixFd& other) const { return fd() == other.fd(); }

private:
    std::shared_ptr<const int> m_fd;
};

// Converts between C++ types and D-Bus values; defined for bool, the fixed
// size integers, double, std::string, ObjectPath, Signature, UnixFd,
// Variant, std::vector, std::map and std::tuple.
template<typename T>
struct Type;

template<typename T, char Code, typename Stored = T>
struct BasicType;

// Any D-Bus value with its signature: what an argument of type "v" holds,
// and the form every argument takes on its way in or out.
class Variant
{
public:
    using Items = std::vector<Variant>;

    Variant() = default;
    template<typename T>
        requires requires { Type<std::decay_t<T>>::signature(); }
    Variant(const T& value)
        : Variant(Type<std::decay_t<T>>::toVariant(value))
    {
    }

    bool isValid() const { return !m_signature.empty(); }
    const std::string& signature() const { return m_signature; }

    // The elements of an array, the members of a struct or dict entry, or the
    // value a "v" holds. Byte arrays keep their bytes in bytes() instead.
    const Items& items() const;
    const std::string& bytes() const;

    // Empty when the value is not of the type.
    template<typename T>
    std::optional<T> get() const
    {
        T value{};
        if (!Type<T>::fromVariant(*this, value))
            return std::nullopt;
        return value;
    }
    template<typename T>
    T value(const T& fallback = T()) const
    {
        return get<T>().value_or(fallback);
    }

    friend bool operator==(const Variant& a, const Variant& b)
    {
        return a.m_signature == b.m_signature && a.m_value == b.m_value;
    }

private:
    template<typename>
    friend struct Type;
    template<typename, char, typename>
    friend struct BasicType;
    friend struct VariantAccess;

    using Storage = std::variant<std::monostate, bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
                                 uint64_t, double, std::string, UnixFd, Items>;

    Variant(std::string signature, Storage value)
        : m_signature(std::move(signature))
        , m_value(std::move(value))
    {
    }

    std::string m_signature;
    Storage m_value;
};

// a{sv}, the common property and option map.
using VariantMap = std::map<std::string, Variant>;

template<typename T, char Code, typename Stored>
struct BasicType
{
    static std::string signature() { return std::string(1, Code); }
    static Variant toVariant(const T& value) { return Variant(signature(), Stored(value)); }
    static bool fromVariant(const Variant& variant, T& out)
    {
        if (variant.m_signature.size() != 1 || variant.m_signature[0] != Code)
            return false;
        out = T(std::get<Stored>(variant.m_value));
        return true;
    }
};

template<> struct Type<bool> : BasicType<bool, 'b'> {};
template<> struct Type<uint8_t> : BasicType<uint8_t, 'y'> {};
template<> struct Type<int16_t> : BasicType<int16_t, 'n'> {};
template<> struct Type<uint16_t> : BasicType<uint16_t, 'q'> {};
template<> struct Type<int32_t> : BasicType<int32_t, 'i'> {};
template<> struct Type<uint32_t> : BasicType<uint32_t, 'u'> {};
template<> struct Type<int64_t> : BasicType<int64_t, 'x'> {};
template<> struct Type<uint64_t> : BasicType<uint64_t, 't'> {};
template<> struct Type<double> : BasicType<double, 'd'> {};
template<> struct Type<std::string> : BasicType<std::string, 's'> {};
template<> struct Type<UnixFd> : BasicType<UnixFd, 'h'> {};

// Write only: string literals and views go out as strings.
template<>
struct Type<const char*>
{
    static std::string signature() { return "s"; }
    static Variant toVariant(const char* value) { return Type<std::string>::toVariant(value); }
};

// What a string literal decays to as a deduced const reference.
template<>
struct Type<char*> : Type<const char*>
{
};

template<>
struct Type<std::string_view>
{
    static std::string signature() { return "s"; }
    static Variant toVariant(std::string_view value) { return Type<std::string>::toVariant(std::string(value)); }
};

template<>
struct Type<ObjectPath>
{
    static std::string signature() { return "o"; }
    static Variant toVariant(const ObjectPath& value) { return Variant("o", value.path); }
    static bool fromVariant(const Variant& variant, ObjectPath& out)
    {
        if (variant.m_signature != "o")
            return false;
        out.path = std::get<std::string>(variant.m_value);
        return true;
    }
};

template<>
struct Type<Signature>
{
    static std::string signature() { return "g"; }
    static Variant toVariant(const Signature& value) { return Variant("g", value.signature); }
    static bool fromVariant(const Variant& variant, Signature& out)
    {
        if (variant.m_signature != "g")
            return false;
        out.signature = std::get<std::string>(variant.m_value);
        return true;
    }
};

template<>
struct Type<Variant>
{
    static std::string signature() { return "v"; }
    static Variant toVariant(const Variant& value) { return Variant("v", Variant::Items{value}); }
    static bool fromVariant(const Variant& variant, Variant& out)
    {
        if (variant.m_signature != "v")
            return false;
        out = std::get<Variant::Items>(variant.m_value).front();
        return true;
    }
};

template<>
struct Type<std::vector<uint8_t>>
{
    static std::string signature() { return "ay"; }
    static Variant toVariant(const std::vector<uint8_t>& value)
    {
        return Variant("ay", std::string(value.begin(), value.end()));
    }
    static bool fromVariant(const Variant& variant, std::vector<uint8_t>& out)
    {
        if (variant.m_signature != "ay")
            return false;
        const auto& bytes = std::get<std::string>(variant.m_value);
        out.assign(bytes.begin(), bytes.end());
        return true;
    }
};

template<typename T>
struct Type<std::vector<T>>
{
    static std::string signature() { return "a" + Type<T>::signature(); }
    static Variant toVariant(const std::vector<T>& value)
    {
        Variant::Items items;
        items.reserve(value.size());
        for (const T& element : value)
            items.push_back(Type<T>::toVariant(element));
        return Variant(signature(), std::move(items));
    }
    static bool fromVariant(const Variant& variant, std::vector<T>& out)
    {
        if (variant.m_signature != signature())
            return false;
        const auto& items = std::get<Variant::Items>(variant.m_value);
        out.clear();
        out.reserve(items.size());
        for (const Variant& item : items) {
            T element{};
            if (!Type<T>::fromVariant(item, element))
                return false;
            out.push_back(std::move(element));
        }
        return true;
    }
};

template<typename K, typename V>
struct Type<std::map<K, V>>
{
    static std::string entrySignature() { return "{" + Type<K>::signature() + Type<V>::signature() + "}"; }
    static std::string signature() { return "a" + entrySignature(); }
    static Variant toVariant(const std::map<K, V>& value)
    {
        Variant::Items items;
        items.reserve(value.size());
        for (const auto& [key, element] : value)
            items.push_back(Variant(entrySignature(),
                                    Variant::Items{Type<K>::toVariant(key), Type<V>::toVariant(element)}));
        return Variant(signature(), std::move(items));
    }
    static bool fromVariant(const Variant& variant, std::map<K, V>& out)
    {
        if (variant.m_signature != signature())
            return false;
        out.clear();
        for (const Variant& entry : std::get<Variant::Items>(variant.m_value)) {
            const auto& pair = std::get<Variant::Items>(entry.m_value);
            K key{};
            V element{};
            if (!Type<K>::fromVariant(pair[0], key) || !Type<V>::fromVariant(pair[1], element))
                return false;
            out.insert_or_assign(std::move(key), std::move(element));
        }
        return true;
    }
};

template<typename... Ts>
struct Type<std::tuple<Ts...>>
{
    static std::string signature() { return "(" + (Type<Ts>::signature() + ... + std::string()) + ")"; }
    static Variant toVariant(const std::tuple<Ts...>& value)
    {
        Variant::Items items;
        std::apply([&items](const Ts&... members) { (items.push_back(Type<Ts>::toVariant(members)), ...); },
                   value);
        return Variant(signature(), std::move(items));
    }
    static bool fromVariant(const Variant& variant, std::tuple<Ts...>& out)
    {
        if (variant.m_signature != signature())
            return false;
        const auto& items = std::get<Variant::Items>(variant.m_value);
        return read(items, out, std::index_sequence_for<Ts...>());
    }

private:
    template<size_t... I>
    static bool read(const Variant::Items& items, std::tuple<Ts...>& out, std::index_sequence<I...>)
    {
        return (Type<Ts>::fromVariant(items[I], std::get<I>(out)) && ...);
    }
};

// Reads the leading arguments into the tuple's members.
template<typename... Ts, size_t... I>
bool readArguments(const Variant::Items& arguments, std::tuple<Ts...>& out, std::index_sequence<I...>)
{
    if (arguments.size() < sizeof...(Ts))
        return false;
    return (Type<Ts>::fromVariant(arguments[I], std::get<I>(out)) && ...);
}

template<typename... Ts>
bool readArguments(const Variant::Items& arguments, std::tuple<Ts...>& out)
{
    return readArguments(arguments, out, std::index_sequence_for<Ts...>());
}

// The signature of a list of argument types.
template<typename... Ts>
std::string signatureOf()
{
    return (Type<std::decay_t<Ts>>::signature() + ... + std::string());
}

} // namespace Lingmo::base::dbus
