// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwayland_server.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fcntl.h>
#include <filesystem>
#include <linux/close_range.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/json.h"
#include "base/logging.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/strings.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "core/process_environment.h"
#include "wayland/server.h"
#include "wayland/server_util.h"
#include "wm/window_manager.h"
#include "xwayland_keyboard_grab.h"
#include "xwayland_shell.h"
#include "xwm.h"

namespace Lingmo::XWayland {

namespace {

const base::LogCategory& logs()
{
    return base::LogCategory::defaultCategory();
}

bool writeAll(int fd, const void* data, size_t size)
{
    const char* bytes = static_cast<const char*>(data);
    while (size > 0) {
        const ssize_t written = write(fd, bytes, size);
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            return false;
        bytes += written;
        size -= size_t(written);
    }
    return true;
}

void appendBigEndian(std::string& data, uint16_t value)
{
    data += char(value >> 8);
    data += char(value & 0xff);
}

} // namespace

XwaylandServer::XwaylandServer(wl_display* display, wayland::PrivilegedClients* privileged,
                               wm::WindowManager* manager)
    : m_display(display)
    , m_privileged(privileged)
    , m_manager(manager)
    , m_loop(*core::mainLoop())
    , m_timeout(&m_loop)
    , m_deferredFailure(&m_loop)
{
    m_timeout.setSingleShot(true);
    m_timeout.onTimeout = [this] {
        fail("startup timeout");
    };
    // One crash reports through the process, the Wayland connection and the
    // XWM connection: deferred failures of one epoch count as one, and the
    // server is not restarted from inside its own callbacks.
    m_deferredFailure.setSingleShot(true);
    m_deferredFailure.onTimeout = [this] {
        fail(std::exchange(m_deferredError, {}));
    };
}

XwaylandServer::~XwaylandServer()
{
    stop();
}

bool XwaylandServer::start(const Options& options)
{
    stop();
    m_error.clear();
    m_options = options;
    m_restarts.clear();
    m_readinessFile = options.readinessFile;
    const char* runtime = getenv("XDG_RUNTIME_DIR");
    std::string pattern = std::string(runtime && *runtime ? runtime : "/tmp")
            + "/lingmo-xwayland-XXXXXX";
    if (!mkdtemp(pattern.data())) {
        fail("cannot create private runtime directory");
        return false;
    }
    m_runtime = pattern;
    m_authority = m_runtime + "/Xauthority";
    const int auth = open(m_authority.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (auth < 0) {
        fail("cannot create Xauthority");
        return false;
    }
    char cookie[16];
    if (getrandom(cookie, sizeof(cookie), 0) != sizeof(cookie)) {
        close(auth);
        fail("cannot generate authentication cookie");
        return false;
    }
    // FamilyWild permits displayfd allocation before the display number is known.
    std::string entry;
    for (uint16_t value : {uint16_t(65535), uint16_t(0), uint16_t(0), uint16_t(18)})
        appendBigEndian(entry, value);
    entry += "MIT-MAGIC-COOKIE-1";
    appendBigEndian(entry, uint16_t(sizeof(cookie)));
    entry.append(cookie, sizeof(cookie));
    const bool written = writeAll(auth, entry.data(), entry.size());
    explicit_bzero(entry.data(), entry.size());
    explicit_bzero(cookie, sizeof(cookie));
    if (close(auth) != 0 || !written) {
        fail("cannot write Xauthority");
        return false;
    }
    if (!m_options.lazy)
        return launch();
    if (!openSockets()) {
        fail("cannot open X11 display sockets");
        return false;
    }
    if (!writeReadiness())
        return false;
    m_published = true;
    LM_INFO(logs()) << "Xwayland on demand: DISPLAY=" + m_displayName
                    << "XAUTHORITY=" + m_authority;
    arm();
    if (ready)
        ready();
    return true;
}

// The X server's own display reservation: a lock file naming a live process,
// and a filesystem and an abstract socket, both handed to Xwayland with
// -listenfd once a client arrives.
bool XwaylandServer::openSockets()
{
    struct stat directory {};
    if (lstat("/tmp/.X11-unix", &directory) != 0 || !S_ISDIR(directory.st_mode)
        || (directory.st_uid != 0 && directory.st_uid != getuid())
        || !(directory.st_mode & S_ISVTX))
        return false;
    auto listenOn = [](const std::string& path, bool abstract) {
        sockaddr_un address {};
        address.sun_family = AF_UNIX;
        if (path.size() + 1 > sizeof(address.sun_path))
            return -1;
        std::memcpy(address.sun_path + (abstract ? 1 : 0), path.data(), path.size());
        const socklen_t length = offsetof(sockaddr_un, sun_path) + path.size() + 1;
        const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (fd < 0)
            return -1;
        if (bind(fd, reinterpret_cast<sockaddr*>(&address), length) != 0 || listen(fd, 1) != 0) {
            close(fd);
            return -1;
        }
        return fd;
    };
    for (int display = 0; display < 100; ++display) {
        const std::string lock = "/tmp/.X" + std::to_string(display) + "-lock";
        int fd = -1;
        for (int attempt = 0; attempt < 2 && fd < 0; ++attempt) {
            fd = open(lock.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0444);
            if (fd >= 0 || errno != EEXIST)
                break;
            // A lock left by a server that died is taken over, as X servers do.
            int64_t owner = 0;
            if (FILE* stale = fopen(lock.c_str(), "re")) {
                char line[32] = {};
                if (fgets(line, sizeof(line), stale)) {
                    std::string_view text(line);
                    text = text.substr(0, text.find('\n'));
                    owner = base::strings::toNumber<int64_t>(text).value_or(0);
                }
                fclose(stale);
            }
            if (owner <= 0 || ::kill(pid_t(owner), 0) == 0 || errno != ESRCH)
                break;
            unlink(lock.c_str());
        }
        if (fd < 0)
            continue;
        char pid[12];
        std::snprintf(pid, sizeof(pid), "%10d\n", int(getpid()));
        const bool written = write(fd, pid, 11) == 11;
        close(fd);
        const std::string path = "/tmp/.X11-unix/X" + std::to_string(display);
        unlink(path.c_str());
        const int filesystem = written ? listenOn(path, false) : -1;
        const int abstract = filesystem >= 0 ? listenOn(path, true) : -1;
        if (abstract < 0) {
            if (filesystem >= 0) {
                close(filesystem);
                unlink(path.c_str());
            }
            unlink(lock.c_str());
            continue;
        }
        m_listenFds = {filesystem, abstract};
        m_lockFile = lock;
        m_socketFile = path;
        m_displayName = ':' + std::to_string(display);
        return true;
    }
    return false;
}

void XwaylandServer::closeListenWatches()
{
    for (int watch : std::exchange(m_listenWatches, {}))
        m_loop.unwatch(watch);
}

void XwaylandServer::closeSockets()
{
    closeListenWatches();
    for (int fd : std::exchange(m_listenFds, {}))
        close(fd);
    if (!m_socketFile.empty())
        unlink(std::exchange(m_socketFile, {}).c_str());
    if (!m_lockFile.empty())
        unlink(std::exchange(m_lockFile, {}).c_str());
}

void XwaylandServer::arm()
{
    if (!m_listenWatches.empty())
        return;
    for (int fd : m_listenFds) {
        m_listenWatches.push_back(m_loop.watch(fd, EPOLLIN, [this](uint32_t) {
            // Xwayland accepts on the same sockets from here on.
            closeListenWatches();
            if (!m_client)
                launch();
        }));
    }
}

bool XwaylandServer::launch()
{
    int wayland[2] {-1, -1}, wm[2] {-1, -1}, notify[2] {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, wayland) ||
        socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, wm) || pipe2(notify, O_CLOEXEC)) {
        for (int fd : {wayland[0], wayland[1], wm[0], wm[1], notify[0], notify[1]})
            if (fd >= 0)
                close(fd);
        fail("cannot create Xwayland channels");
        return false;
    }
    m_client = wl_client_create(m_display, wayland[0]);
    if (!m_client) {
        for (int fd : {wayland[0], wayland[1], wm[0], wm[1], notify[0], notify[1]})
            close(fd);
        fail("cannot adopt Wayland channel");
        return false;
    }
    // Before Xwayland binds anything: its outputs are sized in its own units.
    if (auto* server = wayland::Server::serverForDisplay(m_display))
        server->setClientScale(m_client, m_scale);
    m_destroy.owner = this;
    m_destroy.listener.notify = [](wl_listener* listener, void*) {
        auto* self = reinterpret_cast<Listener*>(listener)->owner;
        wl_list_remove(&self->m_destroy.listener.link);
        if (auto* server = wayland::Server::serverForDisplay(self->m_display))
            server->setClientScale(self->m_client, 1);
        self->m_client = nullptr;
        if (self->m_shell)
            self->m_shell->revokeClient();
        if (!self->m_stopping)
            self->deferFailure("Wayland connection lost");
    };
    wl_client_add_destroy_listener(m_client, &m_destroy.listener);
    m_association = std::make_unique<SurfaceAssociation>();
    m_shell =
        std::make_unique<XwaylandShell>(m_display, m_privileged, m_client, m_association.get());
    m_keyboardGrab = std::make_unique<XwaylandKeyboardGrab>(m_display, m_privileged, m_client,
                                                            m_manager->seat());
    m_shell->bound = [this] {
        publish();
    };
    m_xwm = std::make_unique<Xwm>(m_manager, m_association.get());
    m_xwm->setScale(m_scale);
    m_xwm->setPingTimeout(m_options.pingTimeoutMs);
    m_xwm->setWindowLimit(m_options.windowLimit);
    m_xwm->initialized = [this](bool ok) {
        if (!ok) {
            deferFailure("XWM initialization failed");
            return;
        }
        m_xwmReady = true;
        publish();
    };
    m_xwm->failed = [this] {
        deferFailure("XWM connection lost");
    };
    m_readFd = notify[0];
    fcntl(m_readFd, F_SETFL, O_NONBLOCK);
    m_readWatch = m_loop.watch(m_readFd, EPOLLIN, [this](uint32_t) {
        readDisplay();
    });
    const int waylandChild = wayland[1], wmChild = wm[1], notifyChild = notify[1];
    std::vector<std::string> arguments {"-rootless", "-terminate",
                                        "-nolisten", "tcp",
                                        "-auth",     m_authority,
                                        "-displayfd", std::to_string(notifyChild),
                                        "-wm",       std::to_string(wmChild)};
    // A restart takes the display the session already exported.
    if (!m_displayName.empty())
        arguments.push_back(m_displayName);
    // The lock and the sockets are already ours.
    if (!m_listenFds.empty())
        arguments.push_back("-nolock");
    for (int fd : m_listenFds) {
        arguments.push_back("-listenfd");
        arguments.push_back(std::to_string(fd));
    }
    if (m_options.software)
        arguments.push_back("-shm");
    std::vector<int> inherited {waylandChild, wmChild, notifyChild};
    inherited.insert(inherited.end(), m_listenFds.begin(), m_listenFds.end());
    spawn(arguments, inherited, waylandChild);
    m_xwm->setServerProcess(processId());
    close(waylandChild);
    close(wmChild);
    close(notifyChild);
    m_xwm->start(wm[0]);
    m_timeout.start(std::chrono::milliseconds(m_options.timeoutMs));
    return true;
}

// The descriptors Xwayland needs, and nothing else, are inherited; stdout and
// stderr are the compositor's.
bool XwaylandServer::spawn(const std::vector<std::string>& arguments,
                           const std::vector<int>& inherited, int waylandSocket)
{
    std::string program = m_options.executable;
    if (program.find('/') == std::string::npos)
        program = base::paths::findExecutable(program);
    // Built before the fork: the child may only make async-signal-safe calls.
    std::vector<std::string> environment;
    for (const std::string& variable : childEnvironment()) {
        if (!variable.starts_with("WAYLAND_SOCKET="))
            environment.push_back(variable);
    }
    environment.push_back("WAYLAND_SOCKET=" + std::to_string(waylandSocket));
    // X11 cursor themes are rasterized before Xwayland applies its client
    // scale. Keep the physical cursor size aligned with the compositor's
    // logical 24px pointer so XWayland clients do not switch to a tiny cursor
    // when the output is scaled.
    environment.push_back("XCURSOR_SIZE=" + std::to_string(std::max(1, int(std::lround(24 * m_scale)))));
    std::vector<char*> argv {const_cast<char*>(program.c_str())};
    for (const std::string& argument : arguments)
        argv.push_back(const_cast<char*>(argument.c_str()));
    argv.push_back(nullptr);
    std::vector<char*> envp;
    for (const std::string& variable : environment)
        envp.push_back(const_cast<char*>(variable.c_str()));
    envp.push_back(nullptr);

    // The child reports a failed exec through this; a successful one closes it.
    int status[2];
    if (pipe2(status, O_CLOEXEC) != 0) {
        deferFailure("cannot create a pipe");
        return false;
    }
    const pid_t pid = fork();
    if (pid == 0) {
        if (close_range(3, ~0u, CLOSE_RANGE_CLOEXEC) < 0)
            _exit(127);
        sigset_t empty;
        sigemptyset(&empty);
        sigprocmask(SIG_SETMASK, &empty, nullptr);
        for (int signal : {SIGINT, SIGTERM, SIGPIPE, SIGHUP, SIGCHLD})
            ::signal(signal, SIG_DFL);
        for (int fd : inherited)
            fcntl(fd, F_SETFD, 0);
        // Its memory is mostly the X clients', and a lost server is restarted.
        base::dropOomProtection();
        execve(argv[0], argv.data(), envp.data());
        const int error = errno;
        (void)!write(status[1], &error, sizeof(error));
        _exit(127);
    }
    close(status[1]);
    if (pid < 0) {
        close(status[0]);
        deferFailure(std::string("fork: ") + strerror(errno));
        return false;
    }
    int error = 0;
    ssize_t received;
    do {
        received = read(status[0], &error, sizeof(error));
    } while (received < 0 && errno == EINTR);
    close(status[0]);
    if (received == sizeof(error)) {
        waitpid(pid, nullptr, 0);
        deferFailure(std::string("execve: ") + strerror(error));
        return false;
    }
    m_pid = pid;
    m_pidfd = int(syscall(SYS_pidfd_open, pid, 0));
    if (m_pidfd >= 0)
        m_pidWatch = m_loop.watch(m_pidfd, EPOLLIN, [this](uint32_t) { reap(); });
    return true;
}

void XwaylandServer::reap()
{
    if (m_pid <= 0 || waitpid(m_pid, nullptr, WNOHANG) != m_pid)
        return;
    m_pid = -1;
    if (m_pidWatch)
        m_loop.unwatch(std::exchange(m_pidWatch, 0));
    if (m_pidfd >= 0)
        close(std::exchange(m_pidfd, -1));
    if (!m_stopping)
        deferFailure("Xwayland exited");
}

// SIGTERM, 100 ms for it to work, then SIGKILL and a wait for the end.
void XwaylandServer::stopProcess()
{
    if (m_pidWatch)
        m_loop.unwatch(std::exchange(m_pidWatch, 0));
    if (m_pid > 0) {
        ::kill(m_pid, SIGTERM);
        bool gone = false;
        if (m_pidfd >= 0) {
            pollfd descriptor {m_pidfd, POLLIN, 0};
            gone = poll(&descriptor, 1, 100) > 0;
        }
        if (!gone)
            ::kill(m_pid, SIGKILL);
        waitpid(m_pid, nullptr, 0);
        m_pid = -1;
    }
    if (m_pidfd >= 0)
        close(std::exchange(m_pidfd, -1));
}

void XwaylandServer::readDisplay()
{
    char buffer[32];
    while (true) {
        const auto n = read(m_readFd, buffer, sizeof(buffer));
        if (n > 0) {
            m_readiness.append(buffer, size_t(n));
            if (m_readiness.size() > 12) {
                fail("oversized displayfd response");
                return;
            }
        } else if (n < 0 && errno == EINTR) {
            continue;
        } else if (n < 0 && errno == EAGAIN) {
            break;
        } else {
            if (!m_readiness.ends_with('\n')) {
                fail("displayfd closed before readiness");
                return;
            }
            break;
        }
    }
    if (m_readiness.find('\n') == std::string::npos)
        return;
    const std::string value = m_readiness.substr(0, m_readiness.size() - 1);
    bool valid = !value.empty() && m_readiness.ends_with('\n');
    for (char c : value)
        valid &= c >= '0' && c <= '9';
    const std::optional<uint32_t> number = base::strings::toNumber<uint32_t>(value);
    if (!valid || !number || *number > 65535) {
        fail("malformed displayfd response");
        return;
    }
    const std::string name = ':' + std::to_string(*number);
    if (!m_displayName.empty() && name != m_displayName) {
        fail("restarted on another display");
        return;
    }
    m_displayName = name;
    m_loop.unwatch(std::exchange(m_readWatch, 0));
    close(std::exchange(m_readFd, -1));
    publish();
}

// Written to a file of its own and renamed over the target, readable by the
// user alone.
bool XwaylandServer::writeReadiness()
{
    if (m_readinessFile.empty())
        return true;
    const std::filesystem::path target(m_readinessFile);
    std::string temporary =
            (target.parent_path() / (target.filename().string() + ".XXXXXX")).string();
    const int fd = mkostemp(temporary.data(), O_CLOEXEC);
    if (fd < 0 || fchmod(fd, 0600) != 0) {
        if (fd >= 0) {
            close(fd);
            unlink(temporary.c_str());
        }
        fail("cannot create readiness file");
        return false;
    }
    const std::string json =
            base::json::Value::object({{"DISPLAY", m_displayName}, {"XAUTHORITY", m_authority}})
                    .serialize();
    const bool written = writeAll(fd, json.data(), json.size()) && fsync(fd) == 0;
    if (close(fd) != 0 || !written || rename(temporary.c_str(), m_readinessFile.c_str()) != 0) {
        unlink(temporary.c_str());
        fail("cannot publish readiness file");
        return false;
    }
    return true;
}

void XwaylandServer::publish()
{
    if (m_ready || !m_xwmReady || !m_shell || !m_shell->isBound() || m_displayName.empty() ||
        !m_error.empty())
        return;
    // A lazy server published its display before Xwayland ran.
    if (!m_options.lazy && !writeReadiness())
        return;
    m_timeout.stop();
    m_ready = true;
    m_published = true;
    LM_INFO(logs()) << "Xwayland ready: DISPLAY=" + m_displayName << "XAUTHORITY=" + m_authority;
    if (!m_options.lazy && ready)
        ready();
    if (started)
        started();
}

void XwaylandServer::deferFailure(const std::string& error)
{
    if (m_deferredFailure.isActive())
        return;
    m_deferredError = error;
    m_deferredFailure.start(std::chrono::nanoseconds(0));
}

void XwaylandServer::fail(const std::string& error)
{
    if (m_stopping || !m_error.empty())
        return;
    // X11 clients die with the server, but new ones keep working: the session
    // exported DISPLAY and XAUTHORITY once, and a restart keeps both.
    if (m_published && mayRestart()) {
        LM_WARNING(logs()) << "Xwayland lost:" << error << "- restarting";
        teardown(true);
        // A lazy server waits for the next client again.
        if (m_options.lazy)
            arm();
        else
            launch(); // A launch that fails reports through fail() itself.
        return;
    }
    m_error = error;
    LM_WARNING(logs()) << "Xwayland unavailable:" << error;
    stop();
    if (failure)
        failure(error);
}

void XwaylandServer::setScale(double scale)
{
    if (scale <= 0 || scale == m_scale)
        return;
    m_scale = scale;
    applyScale();
}

void XwaylandServer::applyScale()
{
    if (m_client) {
        if (auto* server = wayland::Server::serverForDisplay(m_display))
            server->setClientScale(m_client, m_scale);
        // Existing surfaces take it at their next commit, which the X11
        // windows being resized below brings.
        wl_client_for_each_resource(
            m_client,
            [](wl_resource* resource, void* data) {
                if (std::strcmp(wl_resource_get_class(resource), "wl_surface") == 0) {
                    if (auto* surface = wayland::surfaceFromResource(resource))
                        surface->setClientScale(*static_cast<double*>(data));
                }
                return WL_ITERATOR_CONTINUE;
            },
            &m_scale);
    }
    if (m_xwm)
        m_xwm->setScale(m_scale);
}

bool XwaylandServer::mayRestart()
{
    const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch())
                                .count();
    while (!m_restarts.empty() && now - m_restarts.front() > 60000)
        m_restarts.pop_front();
    if (int(m_restarts.size()) >= m_options.restartLimit)
        return false;
    m_restarts.push_back(now);
    return true;
}

void XwaylandServer::stop()
{
    teardown(false);
    m_published = false;
}

void XwaylandServer::teardown(bool keepSession)
{
    if (m_stopping)
        return;
    m_stopping = true;
    m_timeout.stop();
    m_deferredFailure.stop();
    m_deferredError.clear();
    m_ready = m_xwmReady = false;
    if (m_readWatch)
        m_loop.unwatch(std::exchange(m_readWatch, 0));
    if (m_readFd >= 0)
        close(std::exchange(m_readFd, -1));
    m_xwm.reset();
    if (m_client)
        wl_client_destroy(m_client);
    m_shell.reset();
    m_keyboardGrab.reset();
    m_association.reset();
    stopProcess();
    m_readiness.clear();
    if (!keepSession) {
        closeSockets();
        if (!m_readinessFile.empty())
            unlink(m_readinessFile.c_str());
        if (!m_runtime.empty()) {
            std::error_code error;
            std::filesystem::remove_all(std::exchange(m_runtime, {}), error);
        }
        m_displayName.clear();
        m_authority.clear();
    }
    m_stopping = false;
}

} // namespace Lingmo::XWayland
