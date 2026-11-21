// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Random request sequences against every global, the counterpart of exo's
// wayland/fuzzer. Exo generates protobuf actions and a harness from the
// protocol XML; here the harness reads the same request signatures from the
// wl_interface tables libwayland already has, and the input bytes pick the
// globals, objects, requests and arguments.
//
// Built as is, it runs seeded random inputs or replays the files it is given.
// Linked with -fsanitize=fuzzer, LLVMFuzzerTestOneInput drives it instead.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <sys/mman.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <wayland-client.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "core/main_loop.h"
#include "core/preferences.h"
#include "protocol_interfaces.h"
#include "temporary_directory.h"

#include "compositor/compositor.h"
#include "compositor/output.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "wayland/privileged_client.h"
#include "wayland/protocol_globals.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/security_delegate.h"
#include "wayland/server.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

// Requests per input, and how many go out between roundtrips.
constexpr int kMaxSteps = 200;
constexpr int kStepsPerRoundtrip = 8;
// Larger than any buffer a request can describe, so a pool the input sizes
// stays mappable; how the compositor copes with a short one is its own case.
constexpr size_t kFdSize = 1 << 20;

// Lets every client reach every global, restricted ones included.
class AllowingSecurityDelegate final : public wayland::SecurityDelegate
{
public:
    bool canManageClipboard(const wl_client *) const override { return true; }

    bool canCaptureOutputs(const wl_client *) const override { return true; }

    bool canControlComputer(const wl_client *) const override { return true; }

    bool showsSystemDialogs(int64_t) const override { return true; }
};

// The compositor's protocol side without a renderer, as exo's
// ServerEnvironment is its server without a display.
class ServerEnvironment
{
public:
    ServerEnvironment()
    {
        for (const char *variable : {"XDG_DATA_HOME", "XDG_RUNTIME_DIR"})
            setenv(variable, m_home.path().c_str(), 1);
        Lingmo::setConfigHomeForTesting(m_home.path());
        m_scene.setSize(Lingmo::base::Size(1280, 800));
        m_manager.setBounds(Lingmo::base::Size(1280, 800));
        m_windowManager.setOutputBounds(Lingmo::base::Rect(0, 0, 1280, 800));
        m_output.pixelSize = Lingmo::base::Size(1280, 800);
        m_output.refreshMilliHz = 60000;

        wayland::ProtocolGlobals::Dependencies dependencies;
        dependencies.server = &m_server;
        dependencies.compositor = &m_compositor;
        dependencies.seat = &m_seat;
        dependencies.inputManager = &m_manager;
        dependencies.windowManager = &m_windowManager;
        dependencies.inputMethodPopupLayer = m_popups;
        dependencies.security = &m_security;
        m_globals = std::make_unique<wayland::ProtocolGlobals>(dependencies);
        if (!m_globals->initialize()) {
            std::fprintf(stderr, "The protocol globals did not initialize\n");
            std::abort();
        }
        m_outputGlobal = wayland::createOutputGlobal(m_server.display(), &m_output);
    }

    ~ServerEnvironment()
    {
        wl_display_destroy_clients(m_server.display());
        wayland::removeOutputGlobal(m_outputGlobal);
        m_globals->shutdown();
    }

    // A client of the privileged shell connection or an ordinary one.
    wl_display *connect(bool trusted)
    {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds) != 0)
            return nullptr;
        if (trusted)
            m_globals->privileged()->adoptShellConnection(fds[0]);
        else
            wl_client_create(m_server.display(), fds[0]);
        return wl_display_connect_to_fd(fds[1]);
    }

    // The client roundtrips on a worker while this thread serves it. A server
    // that stops answering is a hang, which the fuzzer reports as a crash.
    int roundtrip(wl_display *display)
    {
        std::atomic<bool> done = false;
        int result = -1;
        std::thread worker([&] {
            result = wl_display_roundtrip(display);
            done = true;
        });
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (!done && std::chrono::steady_clock::now() < deadline) {
            m_server.dispatch(std::chrono::milliseconds(1));
            m_server.flush();
            core::mainLoop()->dispatch();
        }
        if (!done) {
            std::fprintf(stderr, "The compositor stopped answering\n");
            std::abort();
        }
        worker.join();
        return result;
    }

    // Lets the server notice a client that went away.
    void settle()
    {
        for (int i = 0; i < 10; ++i)
            m_server.dispatch(std::chrono::milliseconds(0));
        core::mainLoop()->dispatch();
    }

private:
    test::TemporaryDirectory m_home;
    scene::Scene m_scene;
    scene::Node *m_surfaces =
            m_scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    scene::Node *m_popups =
            m_scene.root()->addChild(std::make_unique<scene::Node>("popups"));
    compositor::Compositor m_compositor {m_surfaces};
    input::Seat m_seat {&m_compositor};
    input::InputManager m_manager {&m_seat};
    wm::WindowManager m_windowManager {&m_compositor, &m_seat};
    AllowingSecurityDelegate m_security;
    compositor::Output m_output;
    wayland::Server m_server;
    std::unique_ptr<wayland::ProtocolGlobals> m_globals;
    wl_global *m_outputGlobal = nullptr;
};

class Input
{
public:
    Input(const uint8_t *data, size_t size)
        : m_data(data)
        , m_size(size)
    {
    }

    bool atEnd() const { return m_position >= m_size; }

    uint8_t byte() { return atEnd() ? 0 : m_data[m_position++]; }

    uint32_t word()
    {
        uint32_t value = 0;
        for (int i = 0; i < 4; ++i)
            value = value << 8 | byte();
        return value;
    }

    // Mostly the small values protocols mean something by, as exo's
    // small_value receivers are, and now and then anything at all.
    uint32_t number()
    {
        const uint8_t kind = byte();
        if (kind < 192)
            return kind % 8;
        if (kind < 240)
            return byte();
        return word();
    }

private:
    const uint8_t *m_data;
    size_t m_size;
    size_t m_position = 0;
};

struct Global {
    uint32_t name;
    std::string interface;
    uint32_t version;
};

struct Object {
    wl_proxy *proxy;
    const wl_interface *interface;
};

const wl_interface *interfaceNamed(const std::string &name)
{
    const auto &interfaces = Lingmo::testing::knownInterfaces();
    const auto found = interfaces.find(name);
    return found == interfaces.end() ? nullptr : found->second;
}

class Harness
{
public:
    Harness(ServerEnvironment &environment, bool trusted)
        : m_environment(environment)
        , m_display(environment.connect(trusted))
    {
        if (!m_display)
            return;
        m_registry = wl_display_get_registry(m_display);
        static const wl_registry_listener listener = {
                [](void *data, wl_registry *, uint32_t name, const char *interface,
                   uint32_t version) {
                    static_cast<Harness *>(data)->m_globals.push_back({name, interface, version});
                },
                [](void *, wl_registry *, uint32_t) {}};
        wl_registry_add_listener(m_registry, &listener, this);
        m_environment.roundtrip(m_display);
    }

    ~Harness()
    {
        // Local destruction only: the server drops everything with the client.
        for (const Object &object : m_objects) {
            if (object.proxy)
                wl_proxy_destroy(object.proxy);
        }
        if (m_registry)
            wl_registry_destroy(m_registry);
        if (m_display)
            wl_display_disconnect(m_display);
        m_environment.settle();
    }

    void run(Input &input)
    {
        if (!m_display)
            return;
        for (int step = 1; step <= kMaxSteps && !input.atEnd(); ++step) {
            if (input.byte() % 4 == 0)
                bind(input);
            else
                request(input);
            if (step % kStepsPerRoundtrip == 0 && !roundtrip())
                return;
        }
        roundtrip();
    }

private:
    bool roundtrip()
    {
        return m_environment.roundtrip(m_display) >= 0 && wl_display_get_error(m_display) == 0;
    }

    void bind(Input &input)
    {
        if (m_globals.empty())
            return;
        const Global &global = m_globals[input.byte() % m_globals.size()];
        const wl_interface *interface = interfaceNamed(global.interface);
        if (!interface)
            return;
        const uint32_t newest = std::min<uint32_t>(global.version, uint32_t(interface->version));
        const uint32_t version = 1 + input.byte() % newest;
        auto *proxy = static_cast<wl_proxy *>(
                wl_registry_bind(m_registry, global.name, interface, version));
        if (proxy)
            track(proxy, interface);
    }

    // Objects the server creates by events are tracked as requested ones are,
    // so they are destroyed and can be used; descriptors the events carry are
    // closed.
    void track(wl_proxy *proxy, const wl_interface *interface)
    {
        m_objects.push_back({proxy, interface});
        wl_proxy_add_dispatcher(proxy, dispatch, nullptr, this);
    }

    static int dispatch(const void *, void *target, uint32_t, const wl_message *message,
                        wl_argument *arguments)
    {
        auto *harness = static_cast<Harness *>(
                wl_proxy_get_user_data(static_cast<wl_proxy *>(target)));
        int index = 0;
        for (const char *signature = message->signature; *signature; ++signature) {
            if ((*signature >= '0' && *signature <= '9') || *signature == '?')
                continue;
            if (*signature == 'n' && arguments[index].o && message->types[index]) {
                harness->track(reinterpret_cast<wl_proxy *>(arguments[index].o),
                               message->types[index]);
            } else if (*signature == 'h') {
                close(arguments[index].h);
            }
            ++index;
        }
        return 0;
    }

    // An object of `interface` the input picks; any interface when null.
    wl_proxy *pick(Input &input, const wl_interface *interface)
    {
        std::vector<wl_proxy *> candidates;
        for (const Object &object : m_objects) {
            if (object.proxy && (!interface || object.interface == interface))
                candidates.push_back(object.proxy);
        }
        if (candidates.empty())
            return nullptr;
        return candidates[input.byte() % candidates.size()];
    }

    void request(Input &input)
    {
        if (m_objects.empty())
            return;
        Object &object = m_objects[input.byte() % m_objects.size()];
        if (!object.proxy || object.interface->method_count == 0)
            return;
        const uint32_t opcode = input.byte() % object.interface->method_count;
        const wl_message &message = object.interface->methods[opcode];
        const char *signature = message.signature;
        uint32_t since = 0;
        while (*signature >= '0' && *signature <= '9')
            since = since * 10 + uint32_t(*signature++ - '0');
        const uint32_t version = wl_proxy_get_version(object.proxy);
        if (since > version)
            return;

        std::vector<wl_argument> arguments;
        std::vector<std::string> strings;
        std::vector<std::vector<uint8_t>> arrayData;
        std::vector<wl_array> arrays;
        std::vector<int> fds;
        strings.reserve(8);
        arrayData.reserve(8);
        arrays.reserve(8);
        const wl_interface *created = nullptr;
        bool nullable = false;
        bool valid = true;
        for (int index = 0; *signature && valid; ++signature) {
            if (*signature == '?') {
                nullable = true;
                continue;
            }
            wl_argument argument = {};
            const wl_interface *type = message.types[index++];
            switch (*signature) {
            case 'i':
            case 'u':
                argument.u = input.number();
                break;
            case 'f':
                argument.f = wl_fixed_t(input.number() << 8);
                break;
            case 's':
                if (nullable && input.byte() % 8 == 0) {
                    argument.s = nullptr;
                } else {
                    std::string &text = strings.emplace_back();
                    for (uint8_t length = input.byte() % 24; length > 0; --length)
                        text.push_back(char(' ' + input.byte() % 95));
                    argument.s = text.c_str();
                }
                break;
            case 'o':
                argument.o = reinterpret_cast<wl_object *>(pick(input, type));
                valid = argument.o || nullable;
                break;
            case 'n':
                // Only wl_registry.bind leaves the type open, and bind() covers it.
                valid = type != nullptr;
                created = type;
                argument.n = 0;
                break;
            case 'a': {
                std::vector<uint8_t> &bytes = arrayData.emplace_back(input.byte() % 32);
                for (uint8_t &value : bytes)
                    value = input.byte();
                wl_array &array = arrays.emplace_back();
                array.size = bytes.size();
                array.alloc = bytes.size();
                array.data = bytes.data();
                argument.a = &array;
                break;
            }
            case 'h': {
                const int fd = memfd_create("protocol-fuzzer", MFD_CLOEXEC);
                if (fd >= 0 && ftruncate(fd, kFdSize) != 0) {
                    close(fd);
                    valid = false;
                    break;
                }
                fds.push_back(fd);
                argument.h = fd;
                valid = fd >= 0;
                break;
            }
            default:
                valid = false;
                break;
            }
            nullable = false;
            arguments.push_back(argument);
        }
        if (valid) {
            // Requests named so are the destructors in every protocol served.
            const bool destructor = std::strcmp(message.name, "destroy") == 0
                    || std::strcmp(message.name, "release") == 0;
            wl_proxy *proxy = wl_proxy_marshal_array_flags(
                    object.proxy, opcode, created, version,
                    destructor ? WL_MARSHAL_FLAG_DESTROY : 0, arguments.data());
            if (destructor)
                object.proxy = nullptr;
            if (proxy && created)
                track(proxy, created);
        }
        // The request holds its own copies.
        for (int fd : fds) {
            if (fd >= 0)
                close(fd);
        }
    }

    ServerEnvironment &m_environment;
    wl_display *m_display = nullptr;
    wl_registry *m_registry = nullptr;
    std::vector<Global> m_globals;
    std::vector<Object> m_objects;
};

ServerEnvironment &environment()
{
    static core::MainLoop mainLoop;
    static ServerEnvironment environment;
    return environment;
}

void runInput(const uint8_t *data, size_t size)
{
    Input input(data, size);
    Harness harness(environment(), input.byte() % 2 == 0);
    harness.run(input);
}

} // namespace

// Once a client has a protocol error, libwayland no longer dispatches the
// events it has read, so the objects they create never reach the harness.
extern "C" const char *__lsan_default_suppressions()
{
    return "leak:wl_display_read_events\n";
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    runInput(data, size);
    return 0;
}

#ifndef LINGMO_LIBFUZZER
// `protocol_fuzzer [files...]` replays inputs. `protocol_fuzzer --random N SEED`
// runs N random ones from SEED; with no arguments, 400 from a fixed seed, the
// same on every run.
int main(int argc, char **argv)
{
    int runs = 400;
    unsigned seed = 20261006;
    if (argc == 4 && std::strcmp(argv[1], "--random") == 0) {
        runs = std::atoi(argv[2]);
        seed = unsigned(std::strtoul(argv[3], nullptr, 10));
    } else if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            const std::optional<std::string> data = base::file::read(argv[i]);
            if (!data) {
                std::fprintf(stderr, "Cannot read %s\n", argv[i]);
                return 1;
            }
            runInput(reinterpret_cast<const uint8_t *>(data->data()), data->size());
        }
        return 0;
    }
    std::mt19937 random(seed);
    std::vector<uint8_t> data;
    for (int run = 0; run < runs; ++run) {
        data.resize(64 + random() % 960);
        for (uint8_t &value : data)
            value = uint8_t(random());
        // What reproduces a crash: replay this file.
        {
            std::ofstream last("protocol-fuzzer-last-input", std::ios::binary | std::ios::trunc);
            last.write(reinterpret_cast<const char *>(data.data()), std::streamsize(data.size()));
        }
        runInput(data.data(), data.size());
    }
    std::error_code error;
    std::filesystem::remove("protocol-fuzzer-last-input", error);
    return 0;
}
#endif
