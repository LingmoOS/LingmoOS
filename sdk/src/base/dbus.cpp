// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/dbus.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <map>
#include <memory>
#include <optional>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <systemd/sd-bus.h>
#include <unistd.h>
#include <utility>

#include "base/event_loop.h"

namespace Lingmo::base::dbus {

static const Variant::Items s_noItems;
static const std::string s_noBytes;

UnixFd UnixFd::adopt(int fd)
{
    UnixFd result;
    if (fd >= 0)
        result.m_fd = std::shared_ptr<const int>(new int(fd), [](const int* owned) {
            close(*owned);
            delete owned;
        });
    return result;
}

UnixFd UnixFd::duplicate(int fd)
{
    return fd >= 0 ? adopt(fcntl(fd, F_DUPFD_CLOEXEC, 3)) : UnixFd();
}

int UnixFd::release() const
{
    return m_fd ? fcntl(*m_fd, F_DUPFD_CLOEXEC, 3) : -1;
}

const Variant::Items& Variant::items() const
{
    const auto* items = std::get_if<Items>(&m_value);
    return items ? *items : s_noItems;
}

const std::string& Variant::bytes() const
{
    if (m_signature != "ay")
        return s_noBytes;
    return std::get<std::string>(m_value);
}

// Moves values between the Variant tree and sd-bus messages.
struct VariantAccess
{
    static int append(sd_bus_message* message, const Variant& value);
    static int read(sd_bus_message* message, Variant& value);

    template<typename T>
    static int readBasic(sd_bus_message* message, char type, Variant::Storage& storage)
    {
        T number{};
        const int r = sd_bus_message_read_basic(message, type, &number);
        if (r > 0)
            storage = number;
        return r;
    }
};

int VariantAccess::append(sd_bus_message* message, const Variant& value)
{
    const std::string& signature = value.m_signature;
    if (signature.empty())
        return -EINVAL;

    const char type = signature[0];
    switch (type) {
    case 'a': {
        if (signature == "ay") {
            const auto& bytes = std::get<std::string>(value.m_value);
            return sd_bus_message_append_array(message, 'y', bytes.data(), bytes.size());
        }
        int r = sd_bus_message_open_container(message, 'a', signature.c_str() + 1);
        for (const Variant& item : value.items()) {
            if (r < 0)
                return r;
            r = append(message, item);
        }
        return r < 0 ? r : sd_bus_message_close_container(message);
    }
    case '(':
    case '{': {
        const std::string contents = signature.substr(1, signature.size() - 2);
        int r = sd_bus_message_open_container(message, type == '(' ? 'r' : 'e', contents.c_str());
        for (const Variant& item : value.items()) {
            if (r < 0)
                return r;
            r = append(message, item);
        }
        return r < 0 ? r : sd_bus_message_close_container(message);
    }
    case 'v': {
        const Variant& inner = value.items().front();
        int r = sd_bus_message_open_container(message, 'v', inner.m_signature.c_str());
        if (r >= 0)
            r = append(message, inner);
        return r < 0 ? r : sd_bus_message_close_container(message);
    }
    case 'b': {
        const int boolean = std::get<bool>(value.m_value);
        return sd_bus_message_append_basic(message, 'b', &boolean);
    }
    case 's':
    case 'o':
    case 'g':
        return sd_bus_message_append_basic(message, type, std::get<std::string>(value.m_value).c_str());
    case 'h': {
        const int fd = std::get<UnixFd>(value.m_value).fd();
        return sd_bus_message_append_basic(message, 'h', &fd);
    }
    default:
        return std::visit(
            [message, type](const auto& number) -> int {
                using T = std::decay_t<decltype(number)>;
                if constexpr (std::is_arithmetic_v<T>)
                    return sd_bus_message_append_basic(message, type, &number);
                return -EINVAL;
            },
            value.m_value);
    }
}

int VariantAccess::read(sd_bus_message* message, Variant& value)
{
    char type = 0;
    const char* contents = nullptr;
    int r = sd_bus_message_peek_type(message, &type, &contents);
    if (r <= 0)
        return r;

    switch (type) {
    case 'a':
    case 'r':
    case 'e': {
        if (type == 'a' && std::strcmp(contents, "y") == 0) {
            const void* data = nullptr;
            size_t size = 0;
            r = sd_bus_message_read_array(message, 'y', &data, &size);
            if (r < 0)
                return r;
            value = Variant("ay", std::string(static_cast<const char*>(data), size));
            return 1;
        }
        std::string signature = type == 'a' ? std::string("a") + contents
                                : type == 'r' ? std::string("(") + contents + ")"
                                              : std::string("{") + contents + "}";
        r = sd_bus_message_enter_container(message, type, contents);
        if (r < 0)
            return r;
        Variant::Items items;
        while (true) {
            Variant item;
            r = read(message, item);
            if (r < 0)
                return r;
            if (r == 0)
                break;
            items.push_back(std::move(item));
        }
        r = sd_bus_message_exit_container(message);
        if (r < 0)
            return r;
        value = Variant(std::move(signature), std::move(items));
        return 1;
    }
    case 'v': {
        r = sd_bus_message_enter_container(message, 'v', contents);
        if (r < 0)
            return r;
        Variant inner;
        r = read(message, inner);
        if (r < 0)
            return r;
        r = sd_bus_message_exit_container(message);
        if (r < 0)
            return r;
        value = Variant("v", Variant::Items{std::move(inner)});
        return 1;
    }
    case 'b': {
        int boolean = 0;
        r = sd_bus_message_read_basic(message, 'b', &boolean);
        if (r > 0)
            value = Variant("b", bool(boolean));
        return r;
    }
    case 's':
    case 'o':
    case 'g': {
        const char* string = nullptr;
        r = sd_bus_message_read_basic(message, type, &string);
        if (r > 0)
            value = Variant(std::string(1, type), std::string(string ? string : ""));
        return r;
    }
    case 'h': {
        int fd = -1;
        r = sd_bus_message_read_basic(message, 'h', &fd);
        // The message keeps its own descriptor.
        if (r > 0)
            value = Variant("h", UnixFd::duplicate(fd));
        return r;
    }
    default:
        break;
    }

    Variant::Storage storage;
    switch (type) {
    case 'y':
        r = readBasic<uint8_t>(message, type, storage);
        break;
    case 'n':
        r = readBasic<int16_t>(message, type, storage);
        break;
    case 'q':
        r = readBasic<uint16_t>(message, type, storage);
        break;
    case 'i':
        r = readBasic<int32_t>(message, type, storage);
        break;
    case 'u':
        r = readBasic<uint32_t>(message, type, storage);
        break;
    case 'x':
        r = readBasic<int64_t>(message, type, storage);
        break;
    case 't':
        r = readBasic<uint64_t>(message, type, storage);
        break;
    case 'd':
        r = readBasic<double>(message, type, storage);
        break;
    default:
        return -EINVAL;
    }
    if (r > 0)
        value = Variant(std::string(1, type), std::move(storage));
    return r;
}

static Error toError(const sd_bus_error* error, int r)
{
    if (error && sd_bus_error_is_set(error))
        return {error->name, error->message ? error->message : ""};
    sd_bus_error fallback = SD_BUS_ERROR_NULL;
    sd_bus_error_set_errno(&fallback, r);
    Error result{fallback.name ? fallback.name : SD_BUS_ERROR_FAILED, fallback.message ? fallback.message : ""};
    sd_bus_error_free(&fallback);
    return result;
}

template<typename Callback>
static void deleteCallback(void* data)
{
    delete static_cast<Callback*>(data);
}

struct Message::Data
{
    sd_bus_message* message = nullptr;
    std::optional<Variant::Items> arguments;

    ~Data() { sd_bus_message_unref(message); }
};

Message::Message(sd_bus_message* message)
{
    if (!message)
        return;
    m_data = std::make_shared<Data>();
    m_data->message = sd_bus_message_ref(message);
}

sd_bus_message* Message::get() const
{
    return m_data ? m_data->message : nullptr;
}

static std::string orEmpty(const char* string)
{
    return string ? string : "";
}

std::string Message::path() const
{
    return m_data ? orEmpty(sd_bus_message_get_path(m_data->message)) : std::string();
}

std::string Message::interface() const
{
    return m_data ? orEmpty(sd_bus_message_get_interface(m_data->message)) : std::string();
}

std::string Message::member() const
{
    return m_data ? orEmpty(sd_bus_message_get_member(m_data->message)) : std::string();
}

std::string Message::sender() const
{
    return m_data ? orEmpty(sd_bus_message_get_sender(m_data->message)) : std::string();
}

std::string Message::signature() const
{
    return m_data ? orEmpty(sd_bus_message_get_signature(m_data->message, true)) : std::string();
}

const Variant::Items& Message::arguments() const
{
    if (!m_data)
        return s_noItems;
    if (!m_data->arguments) {
        Variant::Items arguments;
        // Only a sealed message, one received or sent, can be read.
        if (sd_bus_message_rewind(m_data->message, true) >= 0) {
            while (true) {
                Variant argument;
                if (VariantAccess::read(m_data->message, argument) <= 0)
                    break;
                arguments.push_back(std::move(argument));
            }
        }
        m_data->arguments = std::move(arguments);
    }
    return *m_data->arguments;
}

Message& Message::appendAs(const Variant& value)
{
    if (m_data && VariantAccess::append(m_data->message, value) < 0)
        fprintf(stderr, "lingmo-toolkit: cannot append a D-Bus value of type %s\n", value.signature().c_str());
    return *this;
}

void Message::setAllowInteractiveAuthorization(bool allow)
{
    if (m_data)
        sd_bus_message_set_allow_interactive_authorization(m_data->message, allow);
}

bool Message::allowsInteractiveAuthorization() const
{
    return m_data && sd_bus_message_get_allow_interactive_authorization(m_data->message) > 0;
}

Reply::Reply(Message message, Error error)
    : m_message(std::move(message))
    , m_error(std::move(error))
{
}

const Variant::Items& Reply::arguments() const
{
    return isError() ? s_noItems : m_message.arguments();
}

void Slot::reset()
{
    sd_bus_slot_unref(m_slot);
    m_slot = nullptr;
}

static void appendRule(std::string& rule, const char* key, const std::string& value)
{
    if (value.empty())
        return;
    rule += ',';
    rule += key;
    rule += "='";
    for (const char c : value) {
        if (c == '\'')
            rule += "'\\''";
        else
            rule += c;
    }
    rule += '\'';
}

std::string MatchRule::toString() const
{
    std::string rule = "type='signal'";
    appendRule(rule, "sender", sender);
    appendRule(rule, "path", path);
    appendRule(rule, "path_namespace", pathNamespace);
    appendRule(rule, "interface", interface);
    appendRule(rule, "member", member);
    appendRule(rule, "arg0", arg0);
    return rule;
}

MethodCall::MethodCall(Message message)
    : m_message(std::move(message))
    , m_replied(std::make_shared<bool>(false))
{
}

void MethodCall::replyWith(const Variant::Items& values)
{
    if (*m_replied)
        return;
    *m_replied = true;
    if (!sd_bus_message_get_expect_reply(m_message.get()))
        return;

    sd_bus_message* reply = nullptr;
    if (sd_bus_message_new_method_return(m_message.get(), &reply) < 0)
        return;
    for (const Variant& value : values)
        VariantAccess::append(reply, value);
    sd_bus_send(nullptr, reply, nullptr);
    sd_bus_message_unref(reply);
}

void MethodCall::replyError(const std::string& name, const std::string& message)
{
    if (*m_replied)
        return;
    *m_replied = true;
    sd_bus_reply_method_errorf(m_message.get(), name.c_str(), "%s", message.c_str());
}

// The connections shared per loop, on the loop's thread.
static thread_local std::map<std::pair<EventLoop*, Connection::Bus>, std::unique_ptr<Connection>> s_shared;

static Connection* shared(Connection::Bus bus, EventLoop* loop)
{
    if (!loop)
        loop = EventLoop::current();
    if (!loop)
        return nullptr;

    const auto key = std::make_pair(loop, bus);
    const auto it = s_shared.find(key);
    if (it != s_shared.end())
        return it->second.get();

    std::unique_ptr<Connection> connection = Connection::open(bus, loop);
    if (!connection)
        return nullptr;
    Connection* result = connection.get();
    s_shared.emplace(key, std::move(connection));
    loop->addDestroyHook([key] { s_shared.erase(key); });
    return result;
}

Connection* Connection::session(EventLoop* loop)
{
    return shared(Bus::Session, loop);
}

Connection* Connection::system(EventLoop* loop)
{
    return shared(Bus::System, loop);
}

std::unique_ptr<Connection> Connection::open(Bus bus, EventLoop* loop)
{
    if (!loop)
        loop = EventLoop::current();
    if (!loop)
        return nullptr;

    sd_bus* connection = nullptr;
    const int r = bus == Bus::Session ? sd_bus_open_user(&connection) : sd_bus_open_system(&connection);
    if (r < 0)
        return nullptr;
    return std::unique_ptr<Connection>(new Connection(connection, *loop));
}

Connection::Connection(sd_bus* bus, EventLoop& loop)
    : m_bus(bus)
    , m_loop(loop)
{
    m_events = EPOLLIN;
    m_watch = m_loop.watch(sd_bus_get_fd(m_bus), m_events, [this](uint32_t) { process(); });
    m_timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    m_timerWatch = m_loop.watch(m_timerFd, EPOLLIN, [this](uint32_t) {
        uint64_t expirations;
        [[maybe_unused]] const auto r = ::read(m_timerFd, &expirations, sizeof(expirations));
        m_armedTimeout = UINT64_MAX;
        process();
    });
    // Messages read during a blocking call wait in the queue, and writes may
    // be pending; neither wakes the loop on its own.
    m_hook = m_loop.addPrepareHook([this] { prepare(); });
}

Connection::~Connection()
{
    m_loop.removePrepareHook(m_hook);
    m_loop.unwatch(m_watch);
    m_loop.unwatch(m_timerWatch);
    close(m_timerFd);
    sd_bus_flush_close_unref(m_bus);
}

void Connection::process()
{
    while (sd_bus_process(m_bus, nullptr) > 0) {
    }
}

void Connection::prepare()
{
    // Messages read during a blocking call make the timeout 0; anything else
    // to read or write wakes the loop through the socket.
    uint64_t timeout = UINT64_MAX;
    if (sd_bus_get_timeout(m_bus, &timeout) < 0)
        timeout = UINT64_MAX;
    if (timeout == 0) {
        process();
        if (sd_bus_get_timeout(m_bus, &timeout) < 0)
            timeout = UINT64_MAX;
    }

    const int events = sd_bus_get_events(m_bus);
    if (events >= 0) {
        const uint32_t wanted = ((events & POLLIN) ? EPOLLIN : 0) | ((events & POLLOUT) ? EPOLLOUT : 0);
        if (wanted != m_events) {
            m_events = wanted;
            m_loop.setWatchEvents(m_watch, wanted);
        }
    }

    // Absolute on CLOCK_MONOTONIC; it ends method calls that get no reply.
    if (timeout == m_armedTimeout)
        return;
    m_armedTimeout = timeout;
    itimerspec spec{};
    if (timeout != UINT64_MAX) {
        timeout = std::max<uint64_t>(timeout, 1);
        spec.it_value.tv_sec = time_t(timeout / 1000000);
        spec.it_value.tv_nsec = long(timeout % 1000000) * 1000;
    }
    timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &spec, nullptr);
}

std::string Connection::uniqueName() const
{
    const char* name = nullptr;
    sd_bus_get_unique_name(m_bus, &name);
    return orEmpty(name);
}

Message Connection::newMethodCall(const std::string& service, const std::string& path,
                                  const std::string& interface, const std::string& method) const
{
    sd_bus_message* message = nullptr;
    if (sd_bus_message_new_method_call(m_bus, &message, service.empty() ? nullptr : service.c_str(), path.c_str(),
                                       interface.empty() ? nullptr : interface.c_str(), method.c_str())
        < 0)
        return {};
    Message result(message);
    sd_bus_message_unref(message);
    return result;
}

Message Connection::newSignal(const std::string& path, const std::string& interface,
                              const std::string& member) const
{
    sd_bus_message* message = nullptr;
    if (sd_bus_message_new_signal(m_bus, &message, path.c_str(), interface.c_str(), member.c_str()) < 0)
        return {};
    Message result(message);
    sd_bus_message_unref(message);
    return result;
}

Reply Connection::call(const Message& call, std::chrono::microseconds timeout) const
{
    if (call.isNull())
        return Reply({}, Error{SD_BUS_ERROR_INVALID_ARGS, "Invalid method call"});

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    const int r = sd_bus_call(m_bus, call.get(), uint64_t(timeout.count()), &error, &reply);
    Reply result = r < 0 ? Reply(Message(reply), toError(&error, r)) : Reply(Message(reply), Error());
    sd_bus_message_unref(reply);
    sd_bus_error_free(&error);
    return result;
}

using ReplyCallback = std::function<void(const Reply&)>;

static int replied(sd_bus_message* message, void* data, sd_bus_error*)
{
    const auto& done = *static_cast<ReplyCallback*>(data);
    if (!done)
        return 0;
    if (sd_bus_message_is_method_error(message, nullptr)) {
        const sd_bus_error* error = sd_bus_message_get_error(message);
        done(Reply(Message(message), toError(error, -EIO)));
    } else {
        done(Reply(Message(message), Error()));
    }
    return 0;
}

PendingCall Connection::callAsync(const Message& call, std::function<void(const Reply&)> done,
                                  std::chrono::microseconds timeout) const
{
    auto* callback = new ReplyCallback(std::move(done));
    sd_bus_slot* slot = nullptr;
    const int r = call.isNull() ? -EINVAL
                                : sd_bus_call_async(m_bus, &slot, call.get(), replied, callback,
                                                    uint64_t(timeout.count()));
    if (r < 0) {
        // Cannot be sent at all: the caller hears at once.
        if (*callback)
            (*callback)(Reply({}, toError(nullptr, r)));
        delete callback;
        return {};
    }
    sd_bus_slot_set_destroy_callback(slot, deleteCallback<ReplyCallback>);
    return PendingCall(slot);
}

bool Connection::send(const Message& message) const
{
    if (message.isNull())
        return false;
    uint8_t type = 0;
    if (sd_bus_message_get_type(message.get(), &type) >= 0 && type == SD_BUS_MESSAGE_METHOD_CALL)
        sd_bus_message_set_expect_reply(message.get(), false);
    return sd_bus_send(m_bus, message.get(), nullptr) >= 0;
}

using SignalCallback = std::function<void(const Message&)>;

static int signalled(sd_bus_message* message, void* data, sd_bus_error*)
{
    const auto& callback = *static_cast<SignalCallback*>(data);
    if (callback)
        callback(Message(message));
    // Other matches for the same message still run.
    return 0;
}

Match Connection::subscribe(const MatchRule& rule, std::function<void(const Message&)> callback) const
{
    auto* data = new SignalCallback(std::move(callback));
    sd_bus_slot* slot = nullptr;
    if (sd_bus_add_match_async(m_bus, &slot, rule.toString().c_str(), signalled, nullptr, data) < 0) {
        delete data;
        return {};
    }
    sd_bus_slot_set_destroy_callback(slot, deleteCallback<SignalCallback>);
    return Match(slot);
}

struct NameWatch::State
{
    std::function<void(const std::string& owner)> callback;
    Match match;
    PendingCall initial;
    bool changed = false;
};

NameWatch Connection::watchName(const std::string& name,
                                std::function<void(const std::string& owner)> callback) const
{
    NameWatch watch;
    watch.m_state = std::make_shared<NameWatch::State>();
    NameWatch::State* state = watch.m_state.get();
    state->callback = std::move(callback);

    MatchRule rule;
    rule.sender = "org.freedesktop.DBus";
    rule.path = "/org/freedesktop/DBus";
    rule.interface = "org.freedesktop.DBus";
    rule.member = "NameOwnerChanged";
    rule.arg0 = name;
    state->match = subscribe(rule, [state](const Message& message) {
        std::string changedName;
        std::string oldOwner;
        std::string newOwner;
        if (!message.read(changedName, oldOwner, newOwner))
            return;
        state->changed = true;
        state->initial.reset();
        // The callback may destroy the watch.
        auto callback = state->callback;
        callback(newOwner);
    });

    Message call = newMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                                 "GetNameOwner");
    call.append(name);
    state->initial = callAsync(call, [state](const Reply& reply) {
        if (state->changed)
            return;
        auto callback = state->callback;
        callback(reply.value<std::string>().value_or(std::string()));
    });
    return watch;
}

bool Connection::requestName(const std::string& name) const
{
    // -EALREADY: this connection owns it already, which is what was asked.
    const int r = sd_bus_request_name(m_bus, name.c_str(), 0);
    return r >= 0 || r == -EALREADY;
}

bool Connection::requestName(const std::string& name, uint32_t flags) const
{
    uint64_t sdFlags = 0;
    if (flags & AllowReplacement)
        sdFlags |= SD_BUS_NAME_ALLOW_REPLACEMENT;
    if (flags & ReplaceExisting)
        sdFlags |= SD_BUS_NAME_REPLACE_EXISTING;
    if (!(flags & DoNotQueue))
        sdFlags |= SD_BUS_NAME_QUEUE;
    // Positive for the primary owner, 0 when queued, -EALREADY when it was.
    const int r = sd_bus_request_name(m_bus, name.c_str(), sdFlags);
    return r > 0 || r == -EALREADY;
}

void Connection::releaseName(const std::string& name) const
{
    sd_bus_release_name(m_bus, name.c_str());
}

Proxy::Proxy(Connection* connection, std::string service, std::string path, std::string interface)
    : m_connection(connection)
    , m_service(std::move(service))
    , m_path(std::move(path))
    , m_interface(std::move(interface))
{
}

Message Proxy::newCall(const std::string& method) const
{
    Message call = m_connection->newMethodCall(m_service, m_path, m_interface, method);
    if (m_interactiveAuthorization)
        call.setAllowInteractiveAuthorization(true);
    return call;
}

Proxy Proxy::properties() const
{
    Proxy proxy(m_connection, m_service, m_path, "org.freedesktop.DBus.Properties");
    proxy.m_timeout = m_timeout;
    proxy.m_interactiveAuthorization = m_interactiveAuthorization;
    return proxy;
}

Reply Proxy::noConnection()
{
    return Reply({}, Error{SD_BUS_ERROR_DISCONNECTED, "No bus connection"});
}

Reply Proxy::getProperty(const std::string& name) const
{
    return properties().call("Get", m_interface, name);
}

PendingCall Proxy::getPropertyAsync(const std::string& name, std::function<void(const Reply&)> done) const
{
    return properties().callAsync("Get", std::move(done), m_interface, name);
}

Reply Proxy::getAll() const
{
    return properties().call("GetAll", m_interface);
}

PendingCall Proxy::getAllAsync(std::function<void(const Reply&)> done) const
{
    return properties().callAsync("GetAll", std::move(done), m_interface);
}

Reply Proxy::setProperty(const std::string& name, const Variant& value) const
{
    return properties().call("Set", m_interface, name, value);
}

PendingCall Proxy::setPropertyAsync(const std::string& name, const Variant& value,
                                    std::function<void(const Reply&)> done) const
{
    return properties().callAsync("Set", std::move(done), m_interface, name, value);
}

// Passes on only what the current owner of a well-known name sends: the bus
// delivers a signal once per connection, and sd-bus then runs every local
// match on its path and interface, whatever their sender.
static std::function<void(const Message&)> fromOwner(const Connection& connection, const std::string& service,
                                                     std::function<void(const Message&)> callback)
{
    if (service.empty() || service.front() == ':' || service == "org.freedesktop.DBus")
        return callback;
    struct State
    {
        std::function<void(const Message&)> callback;
        std::optional<std::string> owner;
        // Until the owner is known.
        std::vector<Message> held;
        NameWatch watch;
    };
    auto state = std::make_shared<State>();
    state->callback = std::move(callback);
    state->watch = connection.watchName(service, [weak = std::weak_ptr<State>(state)](const std::string& owner) {
        const std::shared_ptr<State> state = weak.lock();
        if (!state)
            return;
        const bool first = !state->owner;
        state->owner = owner;
        if (!first)
            return;
        const std::vector<Message> held = std::exchange(state->held, {});
        for (const Message& message : held) {
            if (!owner.empty() && message.sender() == owner && state->callback) {
                // The callback may end the subscription.
                auto callback = state->callback;
                callback(message);
            }
        }
    });
    return [state](const Message& message) {
        const std::shared_ptr<State> keep = state;
        if (!keep->owner) {
            keep->held.push_back(message);
            return;
        }
        if (!keep->owner->empty() && message.sender() == *keep->owner) {
            auto callback = keep->callback;
            callback(message);
        }
    };
}

Match Proxy::onSignal(const std::string& member, std::function<void(const Message&)> callback) const
{
    if (!m_connection)
        return {};
    MatchRule rule;
    rule.sender = m_service;
    rule.path = m_path;
    rule.interface = m_interface;
    rule.member = member;
    return m_connection->subscribe(rule, fromOwner(*m_connection, m_service, std::move(callback)));
}

Match Proxy::onPropertiesChanged(
    std::function<void(const VariantMap& changed, const std::vector<std::string>& invalidated)> callback) const
{
    if (!m_connection)
        return {};
    MatchRule rule;
    rule.sender = m_service;
    rule.path = m_path;
    rule.interface = "org.freedesktop.DBus.Properties";
    rule.member = "PropertiesChanged";
    rule.arg0 = m_interface;
    return m_connection->subscribe(rule, fromOwner(*m_connection, m_service,
                                                   [callback = std::move(callback)](const Message& message) {
        std::string interface;
        VariantMap changed;
        std::vector<std::string> invalidated;
        if (message.read(interface, changed, invalidated))
            callback(changed, invalidated);
    }));
}

struct ObjectAccess
{
    static int handleMethod(sd_bus_message* message, void* data, sd_bus_error* error);
    static int getProperty(sd_bus* bus, const char* path, const char* interface, const char* property,
                           sd_bus_message* reply, void* data, sd_bus_error* error);
    static int setProperty(sd_bus* bus, const char* path, const char* interface, const char* property,
                           sd_bus_message* value, void* data, sd_bus_error* error);
};

// sd-bus walks the table and its names again when the slot goes, which
// may be after the Object when a handler destroys or republishes it: the
// table lives with the slot, which frees it, and forgets the Object first.
struct Object::Table
{
    Object* object = nullptr;
    // Handlers of the table running now, which hold its slot.
    int running = 0;
    // An Object published again while a handler ran: exported once this
    // table is gone.
    std::weak_ptr<Object*> successor;
    std::deque<std::string> strings;
    std::vector<sd_bus_vtable> vtable;

    const char* keep(const std::string& text)
    {
        strings.push_back(text);
        return strings.back().c_str();
    }
};

Object::Object(Connection* connection, std::string path, std::string interface)
    : m_connection(connection)
    , m_path(std::move(path))
    , m_interface(std::move(interface))
{
}

Object::~Object()
{
    if (m_table)
        m_table->object = nullptr;
    m_slot.reset();
}

void Object::addMethod(const std::string& name, std::string inSignature, std::string outSignature,
                       std::function<void(MethodCall&)> handler)
{
    m_methods[name] = Method{std::move(inSignature), std::move(outSignature), std::move(handler)};
}

// The index after the complete type at `i`, or npos when it is malformed.
static size_t skipCompleteType(std::string_view signature, size_t i)
{
    if (i >= signature.size())
        return std::string_view::npos;
    const char c = signature[i];
    if (c == 'a')
        return skipCompleteType(signature, i + 1);
    if (c == '(' || c == '{') {
        const char close = c == '(' ? ')' : '}';
        ++i;
        while (i < signature.size() && signature[i] != close) {
            i = skipCompleteType(signature, i);
            if (i == std::string_view::npos)
                return i;
        }
        return i < signature.size() ? i + 1 : std::string_view::npos;
    }
    if (c == ')' || c == '}')
        return std::string_view::npos;
    return i + 1;
}

// The number of complete types in a signature, or -1 when it is malformed.
static int countCompleteTypes(std::string_view signature)
{
    int count = 0;
    for (size_t i = 0; i < signature.size(); ++count) {
        i = skipCompleteType(signature, i);
        if (i == std::string_view::npos)
            return -1;
    }
    return count;
}

static std::string joinNames(const std::vector<std::string>& names)
{
    std::string joined;
    for (const std::string& name : names) {
        joined += name;
        joined += '\0';
    }
    return joined;
}

void Object::addMethod(const std::string& name, std::string inSignature, std::string outSignature,
                       std::function<void(MethodCall&)> handler, ArgumentNames names)
{
    const bool matches = countCompleteTypes(inSignature) == int(names.in.size())
            && countCompleteTypes(outSignature) == int(names.out.size());
    addMethod(name, std::move(inSignature), std::move(outSignature), std::move(handler));
    if (matches)
        m_methods[name].names = joinNames(names.in) + joinNames(names.out);
}

void Object::addSignal(const std::string& name, std::string signature)
{
    m_signals[name] = Signal{std::move(signature), {}};
}

void Object::addSignal(const std::string& name, std::string signature, std::vector<std::string> names)
{
    const bool matches = countCompleteTypes(signature) == int(names.size());
    m_signals[name] = Signal{std::move(signature), matches ? joinNames(names) : std::string()};
}

bool Object::publish()
{
    if (!m_connection)
        return false;

    if (m_table) {
        m_table->object = nullptr;
        // Its members stay registered until the handler returns.
        if (m_table->running > 0) {
            m_table->successor = m_self;
            m_table = nullptr;
            m_slot.reset();
            return true;
        }
    }
    m_table = nullptr;
    m_slot.reset();
    auto table = std::make_unique<Table>();
    table->object = this;
    std::vector<sd_bus_vtable>& vtable = table->vtable;
    vtable.push_back(SD_BUS_VTABLE_START(0));
    for (const auto& [name, method] : m_methods) {
        sd_bus_vtable entry = SD_BUS_METHOD(table->keep(name), table->keep(method.inSignature),
                                            table->keep(method.outSignature), &ObjectAccess::handleMethod,
                                            SD_BUS_VTABLE_UNPRIVILEGED);
        if (!method.names.empty())
            entry.x.method.names = table->keep(method.names);
        vtable.push_back(entry);
    }
    for (const auto& [name, property] : m_properties) {
        if (property.set)
            vtable.push_back(SD_BUS_WRITABLE_PROPERTY(table->keep(name), table->keep(property.signature),
                                                      &ObjectAccess::getProperty, &ObjectAccess::setProperty, 0,
                                                      SD_BUS_VTABLE_UNPRIVILEGED
                                                          | SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE));
        else
            vtable.push_back(SD_BUS_PROPERTY(table->keep(name), table->keep(property.signature),
                                             &ObjectAccess::getProperty, 0, SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE));
    }
    for (const auto& [name, signal] : m_signals) {
        sd_bus_vtable entry = SD_BUS_SIGNAL(table->keep(name), table->keep(signal.signature), 0);
        if (!signal.names.empty())
            entry.x.signal.names = table->keep(signal.names);
        vtable.push_back(entry);
    }
    vtable.push_back(SD_BUS_VTABLE_END);

    sd_bus_slot* slot = nullptr;
    if (sd_bus_add_object_vtable(m_connection->bus(), &slot, m_path.c_str(), m_interface.c_str(), vtable.data(),
                                 table.get())
        < 0)
        return false;
    sd_bus_slot_set_destroy_callback(slot, [](void* data) {
        std::unique_ptr<Table> table(static_cast<Table*>(data));
        EventLoop* loop = EventLoop::current();
        if (loop && !table->successor.expired()) {
            loop->post([successor = table->successor] {
                if (const std::shared_ptr<Object*> object = successor.lock())
                    (*object)->publish();
            });
        }
    });
    m_table = table.release();
    m_slot = Slot(slot);
    return true;
}

void Object::emitPropertiesChanged(const std::vector<std::string>& names) const
{
    if (!m_connection || names.empty())
        return;
    std::vector<char*> strv;
    for (const std::string& name : names)
        strv.push_back(const_cast<char*>(name.c_str()));
    strv.push_back(nullptr);
    sd_bus_emit_properties_changed_strv(m_connection->bus(), m_path.c_str(), m_interface.c_str(), strv.data());
}

int ObjectAccess::handleMethod(sd_bus_message* message, void* data, sd_bus_error* error)
{
    auto* table = static_cast<Object::Table*>(data);
    Object* self = table->object;
    if (!self)
        return sd_bus_error_set(error, SD_BUS_ERROR_UNKNOWN_OBJECT, "Unknown object");
    const auto it = self->m_methods.find(orEmpty(sd_bus_message_get_member(message)));
    if (it == self->m_methods.end())
        return sd_bus_error_set(error, SD_BUS_ERROR_UNKNOWN_METHOD, "Unknown method");

    MethodCall call{Message(message)};
    // The handler may destroy the Object or publish it again; sd-bus keeps
    // the slot, and with it the table, until this returns.
    auto handler = it->second.handler;
    ++table->running;
    handler(call);
    --table->running;
    return 1;
}

int ObjectAccess::getProperty(sd_bus*, const char*, const char*, const char* property, sd_bus_message* reply,
                        void* data, sd_bus_error* error)
{
    Object* self = static_cast<Object::Table*>(data)->object;
    if (!self)
        return sd_bus_error_set(error, SD_BUS_ERROR_UNKNOWN_OBJECT, "Unknown object");
    const auto it = self->m_properties.find(property);
    if (it == self->m_properties.end())
        return sd_bus_error_set(error, SD_BUS_ERROR_UNKNOWN_PROPERTY, "Unknown property");
    return VariantAccess::append(reply, it->second.get());
}

int ObjectAccess::setProperty(sd_bus*, const char*, const char*, const char* property, sd_bus_message* value,
                        void* data, sd_bus_error* error)
{
    auto* table = static_cast<Object::Table*>(data);
    Object* self = table->object;
    if (!self)
        return sd_bus_error_set(error, SD_BUS_ERROR_UNKNOWN_OBJECT, "Unknown object");
    const auto it = self->m_properties.find(property);
    if (it == self->m_properties.end() || !it->second.set)
        return sd_bus_error_set(error, SD_BUS_ERROR_PROPERTY_READ_ONLY, "Read-only property");
    Variant variant;
    if (VariantAccess::read(value, variant) <= 0)
        return sd_bus_error_set(error, SD_BUS_ERROR_INVALID_ARGS, "Invalid value");
    // The setter may destroy or publish the Object, as a method handler.
    const auto set = it->second.set;
    ++table->running;
    const bool accepted = set(variant);
    --table->running;
    if (!accepted)
        return sd_bus_error_set(error, SD_BUS_ERROR_INVALID_ARGS, "Invalid value");
    return 1;
}

} // namespace Lingmo::base::dbus
