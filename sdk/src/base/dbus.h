// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "base/dbus_types.h"

struct sd_bus;
struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_vtable;

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::base::dbus {

class Connection;

struct Error
{
    std::string name;
    std::string message;

    bool isSet() const { return !name.empty(); }
};

// A message received or being built; copies share it.
class Message
{
public:
    Message() = default;
    // Takes a reference of its own.
    explicit Message(sd_bus_message* message);

    sd_bus_message* get() const;
    bool isNull() const { return !m_data; }

    std::string path() const;
    std::string interface() const;
    std::string member() const;
    std::string sender() const;
    // The body's signature.
    std::string signature() const;

    // The body, read on first use.
    const Variant::Items& arguments() const;
    // Reads the leading arguments; false when one has another type.
    template<typename... Ts>
    bool read(Ts&... out) const
    {
        std::tuple<Ts...> values;
        if (!readArguments(arguments(), values))
            return false;
        std::tie(out...) = std::move(values);
        return true;
    }
    template<typename T>
    std::optional<T> argument(size_t index = 0) const
    {
        const auto& all = arguments();
        return index < all.size() ? all[index].get<T>() : std::nullopt;
    }

    // For a message being built.
    template<typename... Ts>
    Message& append(const Ts&... values)
    {
        (appendAs(Type<std::decay_t<Ts>>::toVariant(values)), ...);
        return *this;
    }
    // Appends the value with its own signature, not as a "v".
    Message& appendAs(const Variant& value);

    // Lets the service ask the user to authorize the call, such as polkit's
    // password dialog, instead of refusing it.
    void setAllowInteractiveAuthorization(bool allow);
    bool allowsInteractiveAuthorization() const;

private:
    struct Data;
    std::shared_ptr<Data> m_data;
};

// The outcome of a method call: the reply's arguments, or an error.
class Reply
{
public:
    Reply() = default;
    Reply(Message message, Error error);

    bool isError() const { return m_error.isSet(); }
    explicit operator bool() const { return !isError(); }
    const Error& error() const { return m_error; }
    const Message& message() const { return m_message; }

    // Empty for an error.
    const Variant::Items& arguments() const;
    template<typename... Ts>
    bool read(Ts&... out) const
    {
        return !isError() && m_message.read(out...);
    }
    template<typename T>
    std::optional<T> value(size_t index = 0) const
    {
        return isError() ? std::nullopt : m_message.argument<T>(index);
    }

private:
    Message m_message;
    Error m_error;
};

// Owns a callback registered with the bus: a reply awaited, a signal match.
// Destroying, reassigning or resetting it unregisters the callback.
class [[nodiscard]] Slot
{
public:
    Slot() = default;
    explicit Slot(sd_bus_slot* slot) : m_slot(slot) {}
    ~Slot() { reset(); }

    Slot(Slot&& other) noexcept : m_slot(std::exchange(other.m_slot, nullptr)) {}
    Slot& operator=(Slot&& other) noexcept
    {
        if (this != &other) {
            reset();
            m_slot = std::exchange(other.m_slot, nullptr);
        }
        return *this;
    }

    bool isNull() const { return !m_slot; }
    void reset();

private:
    sd_bus_slot* m_slot = nullptr;
};

// A reply on its way; dropped when this goes.
using PendingCall = Slot;
// A signal subscription; ends when this goes.
using Match = Slot;

// Follows the owner of a bus name; stops when it goes.
class [[nodiscard]] NameWatch
{
public:
    NameWatch() = default;

private:
    friend class Connection;
    struct State;
    std::shared_ptr<State> m_state;
};

// Selects the signals a subscription receives; empty fields match anything.
struct MatchRule
{
    std::string sender;
    std::string path;
    // The path and everything below it.
    std::string pathNamespace;
    std::string interface;
    std::string member;
    std::string arg0;

    std::string toString() const;
};

// A method call received by an exported object. Reply now or keep a copy
// and reply later; only the first reply counts.
class MethodCall
{
public:
    explicit MethodCall(Message message);

    const Message& message() const { return m_message; }
    const Variant::Items& arguments() const { return m_message.arguments(); }
    template<typename... Ts>
    bool read(Ts&... out) const
    {
        return m_message.read(out...);
    }

    template<typename... Ts>
    void reply(const Ts&... values)
    {
        replyWith({Type<std::decay_t<Ts>>::toVariant(values)...});
    }
    void replyWith(const Variant::Items& values);
    void replyError(const std::string& name, const std::string& message);
    bool isReplied() const { return *m_replied; }

private:
    Message m_message;
    std::shared_ptr<bool> m_replied;
};

// A connection to the session or system bus, dispatched by an event loop.
class Connection
{
public:
    enum class Bus {
        Session,
        System,
    };

    // The loop's shared connection, made on first use and closed with the
    // loop; the calling thread's loop when none is given. Null when the bus
    // cannot be reached.
    static Connection* session(EventLoop* loop = nullptr);
    static Connection* system(EventLoop* loop = nullptr);
    // A connection of its own.
    static std::unique_ptr<Connection> open(Bus bus, EventLoop* loop = nullptr);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    sd_bus* bus() const { return m_bus; }
    std::string uniqueName() const;

    Message newMethodCall(const std::string& service, const std::string& path, const std::string& interface,
                          const std::string& method) const;
    Message newSignal(const std::string& path, const std::string& interface, const std::string& member) const;

    // Zero is the bus's default, 25 seconds.
    Reply call(const Message& call, std::chrono::microseconds timeout = {}) const;
    PendingCall callAsync(const Message& call, std::function<void(const Reply&)> done,
                          std::chrono::microseconds timeout = {}) const;
    // Sends a signal, or a method call whose reply nobody waits for.
    bool send(const Message& message) const;

    template<typename... Args>
    bool emitSignal(const std::string& path, const std::string& interface, const std::string& member,
                    const Args&... args) const
    {
        Message signal = newSignal(path, interface, member);
        signal.append(args...);
        return send(signal);
    }

    Match subscribe(const MatchRule& rule, std::function<void(const Message&)> callback) const;
    // Calls back with the name's owner at once and on every change; the
    // owner is empty while there is none.
    NameWatch watchName(const std::string& name, std::function<void(const std::string& owner)> callback) const;

    // org.freedesktop.DBus.RequestName's flags.
    enum NameFlag : uint32_t {
        AllowReplacement = 1,
        ReplaceExisting = 2,
        DoNotQueue = 4,
    };
    // Becomes the name's primary owner, without queueing; false when another
    // connection owns it, true when this one does already.
    bool requestName(const std::string& name) const;
    // With RequestName's flags: ReplaceExisting takes the name from an owner
    // that allows replacement. True when this connection is the primary
    // owner; without DoNotQueue the request may wait in the queue, and false
    // is returned.
    bool requestName(const std::string& name, uint32_t flags) const;
    void releaseName(const std::string& name) const;

private:
    Connection(sd_bus* bus, EventLoop& loop);
    void process();
    void prepare();

    sd_bus* m_bus = nullptr;
    EventLoop& m_loop;
    int m_watch = 0;
    int m_hook = 0;
    int m_timerFd = -1;
    int m_timerWatch = 0;
    uint32_t m_events = 0;
    uint64_t m_armedTimeout = UINT64_MAX;
};

// A remote object's interface.
class Proxy
{
public:
    Proxy() = default;
    Proxy(Connection* connection, std::string service, std::string path, std::string interface);

    bool isValid() const { return m_connection != nullptr; }
    Connection* connection() const { return m_connection; }
    const std::string& service() const { return m_service; }
    const std::string& path() const { return m_path; }
    const std::string& interface() const { return m_interface; }

    // Zero is the bus's default, 25 seconds.
    void setTimeout(std::chrono::microseconds timeout) { m_timeout = timeout; }
    // Every call of the proxy, properties included, lets the service ask the
    // user to authorize it (polkit). Allow a long timeout for the dialog.
    void setInteractiveAuthorizationAllowed(bool allow) { m_interactiveAuthorization = allow; }

    template<typename... Args>
    Reply call(const std::string& method, const Args&... args) const
    {
        if (!m_connection)
            return noConnection();
        return m_connection->call(newCall(method).append(args...), m_timeout);
    }
    template<typename... Args>
    PendingCall callAsync(const std::string& method, std::function<void(const Reply&)> done,
                          const Args&... args) const
    {
        if (!m_connection)
            return {};
        return m_connection->callAsync(newCall(method).append(args...), std::move(done), m_timeout);
    }
    // Calls without waiting for or keeping the reply.
    template<typename... Args>
    bool send(const std::string& method, const Args&... args) const
    {
        return m_connection && m_connection->send(newCall(method).append(args...));
    }

    // org.freedesktop.DBus.Properties on the proxy's interface. A Get reply
    // holds a Variant; a GetAll reply a VariantMap.
    Reply getProperty(const std::string& name) const;
    template<typename T>
    std::optional<T> property(const std::string& name) const
    {
        const std::optional<Variant> value = getProperty(name).value<Variant>();
        return value ? value->get<T>() : std::nullopt;
    }
    PendingCall getPropertyAsync(const std::string& name, std::function<void(const Reply&)> done) const;
    Reply getAll() const;
    PendingCall getAllAsync(std::function<void(const Reply&)> done) const;
    Reply setProperty(const std::string& name, const Variant& value) const;
    PendingCall setPropertyAsync(const std::string& name, const Variant& value,
                                 std::function<void(const Reply&)> done) const;

    // Signals of the proxy's object. For a well-known service name only what
    // its current owner sends gets through: other connections' objects at the
    // same path (two players, two tray items) are not heard. Signals arriving
    // before the owner is known wait for it.
    Match onSignal(const std::string& member, std::function<void(const Message&)> callback) const;
    Match onPropertiesChanged(
        std::function<void(const VariantMap& changed, const std::vector<std::string>& invalidated)> callback) const;

private:
    Message newCall(const std::string& method) const;
    Proxy properties() const;
    static Reply noConnection();

    Connection* m_connection = nullptr;
    std::string m_service;
    std::string m_path;
    std::string m_interface;
    std::chrono::microseconds m_timeout{0};
    bool m_interactiveAuthorization = false;
};

template<typename F>
struct FunctionTraits : FunctionTraits<decltype(&F::operator())>
{
};
template<typename C, typename R, typename... A>
struct FunctionTraits<R (C::*)(A...) const>
{
    using Result = R;
    using Arguments = std::tuple<std::decay_t<A>...>;
};
template<typename C, typename R, typename... A>
struct FunctionTraits<R (C::*)(A...)> : FunctionTraits<R (C::*)(A...) const>
{
};
template<typename R, typename... A>
struct FunctionTraits<R (*)(A...)>
{
    using Result = R;
    using Arguments = std::tuple<std::decay_t<A>...>;
};

template<typename T>
struct TupleSignature
{
    static constexpr bool isTuple = false;
};
template<typename... Ts>
struct TupleSignature<std::tuple<Ts...>>
{
    static constexpr bool isTuple = true;
    static std::string signature() { return signatureOf<Ts...>(); }
};

// Argument names for introspection, one per complete type of the signature
// in order. A list whose length does not match its signature is dropped,
// with the other one, and the arguments go unnamed.
struct ArgumentNames
{
    std::vector<std::string> in;
    std::vector<std::string> out;
};

// An interface of an object this process exports. Add the members, then
// publish(); members added later need another publish(). It may be
// destroyed or published again from one of its own handlers: messages after
// that no longer reach the old members, and a table published from a
// handler is exported once the handler has returned, on the next loop turn.
class Object
{
public:
    Object(Connection* connection, std::string path, std::string interface);
    ~Object();

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    const std::string& path() const { return m_path; }
    const std::string& interface() const { return m_interface; }

    // The arguments and the result come from the handler's parameters and
    // return type; a std::tuple returns several values.
    template<typename F>
    void addMethod(const std::string& name, F handler);
    // With argument names for introspection.
    template<typename F>
    void addMethod(const std::string& name, F handler, ArgumentNames names);
    // A handler that replies itself, now or later.
    void addMethod(const std::string& name, std::string inSignature, std::string outSignature,
                   std::function<void(MethodCall&)> handler);
    void addMethod(const std::string& name, std::string inSignature, std::string outSignature,
                   std::function<void(MethodCall&)> handler, ArgumentNames names);

    // Read-only with a getter alone; a setter takes the value and may return
    // false to refuse it.
    template<typename Getter>
    void addProperty(const std::string& name, Getter getter);
    template<typename Getter, typename Setter>
    void addProperty(const std::string& name, Getter getter, Setter setter);

    void addSignal(const std::string& name, std::string signature);
    // With argument names for introspection, one per complete type.
    void addSignal(const std::string& name, std::string signature, std::vector<std::string> names);

    bool publish();

    template<typename... Args>
    bool emitSignal(const std::string& member, const Args&... args) const
    {
        return m_connection && m_connection->emitSignal(m_path, m_interface, member, args...);
    }
    // Sends PropertiesChanged with the properties' current values.
    void emitPropertiesChanged(const std::vector<std::string>& names) const;

private:
    struct Method
    {
        std::string inSignature;
        std::string outSignature;
        std::function<void(MethodCall&)> handler;
        // In names, then out names, each ending in NUL; empty without names.
        std::string names;
    };
    struct Signal
    {
        std::string signature;
        std::string names;
    };
    struct Property
    {
        std::string signature;
        std::function<Variant()> get;
        std::function<bool(const Variant&)> set;
    };

    friend struct ObjectAccess;

    // The table published last; its slot owns it.
    struct Table;

    Connection* m_connection;
    std::string m_path;
    std::string m_interface;
    std::map<std::string, Method> m_methods;
    std::map<std::string, Property> m_properties;
    std::map<std::string, Signal> m_signals;
    Table* m_table = nullptr;
    Slot m_slot;
    // For a publish() waiting for the table it replaces to go.
    std::shared_ptr<Object*> m_self = std::make_shared<Object*>(this);
};

template<typename F>
void Object::addMethod(const std::string& name, F handler)
{
    using Traits = FunctionTraits<F>;
    using Arguments = typename Traits::Arguments;
    using Result = typename Traits::Result;

    std::string outSignature;
    if constexpr (!std::is_void_v<Result>) {
        if constexpr (TupleSignature<Result>::isTuple)
            outSignature = TupleSignature<Result>::signature();
        else
            outSignature = signatureOf<Result>();
    }
    addMethod(name, TupleSignature<Arguments>::signature(), outSignature,
              [handler = std::move(handler)](MethodCall& call) mutable {
                  Arguments arguments;
                  if (!readArguments(call.arguments(), arguments)) {
                      call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
                      return;
                  }
                  if constexpr (std::is_void_v<Result>) {
                      std::apply(handler, std::move(arguments));
                      call.reply();
                  } else if constexpr (TupleSignature<Result>::isTuple) {
                      std::apply([&call](const auto&... values) { call.reply(values...); },
                                 std::apply(handler, std::move(arguments)));
                  } else {
                      call.reply(std::apply(handler, std::move(arguments)));
                  }
              });
}

template<typename F>
void Object::addMethod(const std::string& name, F handler, ArgumentNames names)
{
    addMethod(name, std::move(handler));
    Method& method = m_methods[name];
    addMethod(name, method.inSignature, method.outSignature, std::move(method.handler), std::move(names));
}

template<typename Getter>
void Object::addProperty(const std::string& name, Getter getter)
{
    using Value = std::decay_t<std::invoke_result_t<Getter&>>;
    m_properties[name] = Property{Type<Value>::signature(),
                                  [getter = std::move(getter)]() mutable { return Type<Value>::toVariant(getter()); },
                                  nullptr};
}

template<typename Getter, typename Setter>
void Object::addProperty(const std::string& name, Getter getter, Setter setter)
{
    using Value = std::decay_t<std::invoke_result_t<Getter&>>;
    addProperty(name, std::move(getter));
    m_properties[name].set = [setter = std::move(setter)](const Variant& variant) mutable {
        Value value{};
        if (!Type<Value>::fromVariant(variant, value))
            return false;
        if constexpr (std::is_same_v<std::invoke_result_t<Setter&, Value>, bool>)
            return setter(std::move(value));
        setter(std::move(value));
        return true;
    };
}

} // namespace Lingmo::base::dbus
