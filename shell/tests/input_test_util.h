// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_INPUT_TEST_UTIL_H_
#define LINGMO_TESTS_INPUT_TEST_UTIL_H_

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/file.h"

namespace input_test {

// A directory of its own under /tmp, removed with its contents.
class TemporaryDirectory
{
public:
    TemporaryDirectory()
    {
        char directory[] = "/tmp/lingmo-aurora-test-XXXXXX";
        if (mkdtemp(directory))
            m_path = directory;
    }
    ~TemporaryDirectory()
    {
        if (!m_path.empty())
            std::filesystem::remove_all(m_path);
    }

    TemporaryDirectory(const TemporaryDirectory &) = delete;
    TemporaryDirectory &operator=(const TemporaryDirectory &) = delete;

    bool isValid() const { return !m_path.empty(); }
    const std::string &path() const { return m_path; }
    std::string filePath(const std::string &name) const { return m_path + "/" + name; }

private:
    std::string m_path;
};

inline bool writeFile(const std::string &path, const std::string &contents)
{
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    return Lingmo::base::file::writeAtomically(path, contents);
}

// Runs the loop until `done` holds or the time is up; whether `done` held.
inline bool runUntil(Lingmo::base::EventLoop &loop, const std::function<bool()> &done,
                     std::chrono::milliseconds limit = std::chrono::seconds(5))
{
    if (done())
        return true;
    const int hook = loop.addPrepareHook([&] {
        if (done())
            loop.quit();
    });
    const int timer = loop.startTimer(limit, false, [&loop] { loop.quit(); });
    loop.run();
    loop.stopTimer(timer);
    loop.removePrepareHook(hook);
    return done();
}

// Calls a service of this process from a connection of its own, running the
// loop the service is on until the reply comes: a call on the service's own
// connection would wait for itself.
class RemoteObject
{
public:
    RemoteObject(Lingmo::base::EventLoop &loop, std::string service, std::string path,
                 std::string interface)
        : m_loop(loop)
        , m_connection(Lingmo::base::dbus::Connection::open(
                  Lingmo::base::dbus::Connection::Bus::Session, &loop))
        , m_proxy(m_connection.get(), service, path, std::move(interface))
        , m_introspectable(m_connection.get(), std::move(service), std::move(path),
                           "org.freedesktop.DBus.Introspectable")
    {
    }

    bool isValid() const { return m_connection != nullptr; }
    Lingmo::base::dbus::Connection *connection() const { return m_connection.get(); }

    template<typename... Args>
    Lingmo::base::dbus::Reply call(const std::string &method, const Args &...args)
    {
        return wait([&](auto done) { return m_proxy.callAsync(method, std::move(done), args...); });
    }

    template<typename T>
    std::optional<T> property(const std::string &name)
    {
        const Lingmo::base::dbus::Reply reply =
                wait([&](auto done) { return m_proxy.getPropertyAsync(name, std::move(done)); });
        const auto value = reply.value<Lingmo::base::dbus::Variant>();
        return value ? value->get<T>() : std::nullopt;
    }

    std::string introspect()
    {
        const Lingmo::base::dbus::Reply reply = wait(
                [&](auto done) { return m_introspectable.callAsync("Introspect", std::move(done)); });
        return reply.value<std::string>().value_or(std::string());
    }

private:
    template<typename Start>
    Lingmo::base::dbus::Reply wait(Start start)
    {
        std::optional<Lingmo::base::dbus::Reply> result;
        Lingmo::base::dbus::PendingCall pending =
                start([&result](const Lingmo::base::dbus::Reply &reply) { result = reply; });
        runUntil(m_loop, [&] { return result.has_value(); });
        return result.value_or(Lingmo::base::dbus::Reply());
    }

    Lingmo::base::EventLoop &m_loop;
    std::unique_ptr<Lingmo::base::dbus::Connection> m_connection;
    Lingmo::base::dbus::Proxy m_proxy;
    Lingmo::base::dbus::Proxy m_introspectable;
};

} // namespace input_test

#endif // LINGMO_TESTS_INPUT_TEST_UTIL_H_
