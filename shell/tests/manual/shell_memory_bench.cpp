// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// What the compositor's shell components cost in memory, headless through
// the compositor's own GL renderer, one step at a time:
//
//   lingmo-shell-memory-bench [--scale S] [--dark] [--wallpaper IMAGE]
//                               [--glibc-defaults] [--mmap-threshold BYTES]
//                               [--arena-max N] [--no-thp] [--cpu-panels]
//                               [--hold S] COMPONENT...
//
// Steps: the renderer with a wallpaper, the Shell, then each COMPONENT
// (osd, dock) added and drawn for a second, then each one used once, then
// Skia's GPU cache purged and the heap trimmed. Each step prints this
// process's PSS split by kind of mapping, malloc's bytes in use and free,
// the GPU memory of its DRM client, its threads and the main thread's CPU
// time, with the change since the step before. malloc is set up as the
// compositor sets it up unless --glibc-defaults. Run it on a private session
// bus (dbus-run-session): the components then reach the session's PipeWire
// and services as they do in the compositor.

#include <malloc.h>
#include <unistd.h>
#include <sys/prctl.h>

#include <cctype>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <EGL/egl.h>

#include "base/dbus.h"
#include "compositor/compositor.h"
#include "compositor/output_scale.h"
#include "core/main_loop.h"
#include "core/malloc_policy.h"
#include "core/preferences.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "decoration/shadow_node.h"
#include "egl_test_util.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "render/egl_renderer.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/dock/dock.h"
#include "shell/dock/dock_window.h"
#include "shell/osd/osd.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "toolkit/toolkit_host.h"
#include "ui/bitmap.h"
#include "wm/shadow_controller.h"
#include "wm/window_manager.h"
#include "wm/wm_shadow_controller_delegate.h"

#include <linux/input-event-codes.h>

using namespace Lingmo;

namespace {

const base::Size kNative(2160, 1440);

const char *const kKinds[] = {"anon", "toolkit", "gl-driver", "icu", "fonts", "icons", "libs", "other"};

// KiB, as the kernel reports them.
struct Sample {
    std::map<std::string, int64_t> pss;
    int64_t pssTotal = 0;
    int64_t anonHuge = 0;
    int64_t mallocUsed = 0;
    int64_t mallocFree = 0;
    int64_t vram = 0;
    int64_t gtt = 0;
    int64_t threads = 0;
    double mainCpuMs = 0;
    std::multiset<std::string> threadNames;
};

std::string kindOf(const std::string &path)
{
    const auto has = [&](const char *part) { return path.find(part) != std::string::npos; };
    if (path.empty() || path == "[heap]" || path.rfind("[anon", 0) == 0 || path == "[stack]")
        return "anon";
    if (has("/fonts/") || has("fontconfig"))
        return "fonts";
    if (has("/icons/"))
        return "icons";
    if (has("libicu"))
        return "icu";
    if (has("lingmo-toolkit"))
        return "toolkit";
    if (has("LLVM") || has("gallium") || has("dri") || has("mesa") || has("libz3")
        || has("drm_amdgpu") || has("libelf") || has("libsensors"))
        return "gl-driver";
    if (has(".so"))
        return "libs";
    return "other";
}

Sample sample()
{
    Sample s;
    std::ifstream smaps("/proc/self/smaps");
    std::string line;
    std::string kind = "other";
    while (std::getline(smaps, line)) {
        const size_t dash = line.find('-');
        if (dash != std::string::npos && dash < 17 && std::isxdigit(uint8_t(line[0]))
            && line.find(':') > dash + 10) {
            std::istringstream fields(line);
            std::string range, perms, offset, device, inode, path;
            fields >> range >> perms >> offset >> device >> inode;
            std::getline(fields, path);
            path.erase(0, std::min(path.size(), path.find_first_not_of(' ')));
            kind = kindOf(path);
            continue;
        }
        int64_t value = 0;
        if (std::sscanf(line.c_str(), "Pss: %" SCNd64, &value) == 1) {
            s.pss[kind] += value;
            s.pssTotal += value;
        } else if (std::sscanf(line.c_str(), "AnonHugePages: %" SCNd64, &value) == 1) {
            s.anonHuge += value;
        }
    }

    timespec cpu {};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu);
    s.mainCpuMs = cpu.tv_sec * 1e3 + cpu.tv_nsec / 1e6;

    const struct mallinfo2 info = mallinfo2();
    s.mallocUsed = int64_t(info.uordblks + info.hblkhd) / 1024;
    s.mallocFree = int64_t(info.fordblks) / 1024;

    std::set<std::string> clients;
    for (const auto &entry : std::filesystem::directory_iterator("/proc/self/fdinfo")) {
        std::ifstream fdinfo(entry.path());
        std::string id;
        int64_t vram = 0, gtt = 0;
        while (std::getline(fdinfo, line)) {
            int64_t value = 0;
            if (line.rfind("drm-client-id:", 0) == 0)
                id = line;
            else if (std::sscanf(line.c_str(), "drm-memory-vram: %" SCNd64, &value) == 1)
                vram = value;
            else if (std::sscanf(line.c_str(), "drm-memory-gtt: %" SCNd64, &value) == 1)
                gtt = value;
        }
        if (id.empty() || !clients.insert(id).second)
            continue;
        s.vram += vram;
        s.gtt += gtt;
    }
    for (const auto &task : std::filesystem::directory_iterator("/proc/self/task")) {
        std::ifstream comm(task.path() / "comm");
        std::string name;
        std::getline(comm, name);
        s.threadNames.insert(name);
        ++s.threads;
    }
    return s;
}

void printHeader()
{
    std::printf("%-22s %8s", "step", "PSS");
    for (const char *kind : kKinds)
        std::printf(" %9s", kind);
    std::printf(" %8s %8s %8s %8s %8s %4s %8s\n", "thp", "m-used", "m-free", "vram", "gtt", "thr", "main-ms");
}

// MB (10^6) for PSS, as /proc and the session's memory scripts report; MiB
// for the GPU, as its fdinfo does.
void printSample(const std::string &step, const Sample &s, const Sample *before)
{
    const auto mb = [](int64_t kib) { return kib * 1024 / 1e6; };
    const auto mib = [](int64_t kib) { return kib / 1024.0; };
    std::printf("%-22s %8.1f", step.c_str(), mb(s.pssTotal));
    for (const char *kind : kKinds)
        std::printf(" %9.1f", mb(s.pss.count(kind) ? s.pss.at(kind) : 0));
    std::printf(" %8.1f %8.1f %8.1f %8.1f %8.1f %4" PRId64 " %8.1f\n", mib(s.anonHuge), mb(s.mallocUsed),
                mb(s.mallocFree), mib(s.vram), mib(s.gtt), s.threads, s.mainCpuMs);
    if (!before)
        return;
    const auto at = [](const Sample &x, const char *kind) {
        return x.pss.count(kind) ? x.pss.at(kind) : 0;
    };
    std::printf("%-22s %+8.1f", "  change", mb(s.pssTotal - before->pssTotal));
    for (const char *kind : kKinds)
        std::printf(" %+9.1f", mb(at(s, kind) - at(*before, kind)));
    std::printf(" %+8.1f %+8.1f %+8.1f %+8.1f %+8.1f %+4" PRId64 " %+8.1f\n", mib(s.anonHuge - before->anonHuge),
                mb(s.mallocUsed - before->mallocUsed), mb(s.mallocFree - before->mallocFree),
                mib(s.vram - before->vram), mib(s.gtt - before->gtt), s.threads - before->threads,
                s.mainCpuMs - before->mainCpuMs);
    std::multiset<std::string> started, ended;
    std::set_difference(s.threadNames.begin(), s.threadNames.end(), before->threadNames.begin(),
                        before->threadNames.end(), std::inserter(started, started.end()));
    std::set_difference(before->threadNames.begin(), before->threadNames.end(), s.threadNames.begin(),
                        s.threadNames.end(), std::inserter(ended, ended.end()));
    if (!started.empty() || !ended.empty()) {
        std::printf("%-22s", "  threads");
        for (const std::string &name : started)
            std::printf(" +%s", name.c_str());
        for (const std::string &name : ended)
            std::printf(" -%s", name.c_str());
        std::printf("\n");
    }
    std::fflush(stdout);
}

class Renderer
{
public:
    bool init()
    {
        m_renderer = render::EglRendererTestAccess::create();
        if (!m_renderer)
            return false;
        const EGLint attributes[] = {EGL_WIDTH, kNative.width(), EGL_HEIGHT, kNative.height(), EGL_NONE};
        const auto surface = eglCreatePbufferSurface(m_renderer->eglDisplay(), m_renderer->eglConfig(),
                                                     attributes);
        if (surface == EGL_NO_SURFACE)
            return false;
        m_target = std::make_unique<render::EglRenderTarget>(m_renderer->eglDisplay(), surface, nullptr,
                                                             kNative);
        return true;
    }

    render::EglRenderer *renderer() const { return m_renderer.get(); }
    render::EglRenderTarget *target() const { return m_target.get(); }

private:
    std::unique_ptr<render::EglRenderer> m_renderer;
    std::unique_ptr<render::EglRenderTarget> m_target;
};

// One output with a wallpaper and a status bar's reservation, rendered as
// the compositor renders it; the shell and its components come later.
class Desktop
{
public:
    Desktop(Renderer &gl, double scale, bool dark, const ui::Bitmap &wallpaper)
        : m_gl(gl)
        , m_scale(scale)
        , m_output(0, 0, int(std::lround(kNative.width() / scale)), int(std::lround(kNative.height() / scale)))
    {
        m_theme.setDarkMode(dark);
        m_scene.setSize(m_output.size());
        m_compositor.setOutputScale(*compositor::OutputScale::fromNumerator(uint32_t(std::lround(scale * 120))));
        m_manager.setOutputBounds(m_output);
        m_input.setBounds(m_output.size());
        m_input.setEventSink(&m_manager);
        m_manager.setFrameProvider(&m_frames);
        m_manager.setVisualStyle(m_theme.visualStyle());
        m_manager.setExclusiveZone(&m_statusBar, wm::ScreenEdge::Top, 32);

        if (!wallpaper.isNull()) {
            render::EglRenderer *renderer = gl.renderer();
            renderer->beginFrame(gl.target());
            const ui::Bitmap bgra = wallpaper.converted(ui::Bitmap::Format::Argb32Premultiplied);
            m_wallpaper = renderer->createTexture(bgra.size(), bgra.constBits(), int(bgra.stride()),
                                                  render::PixelFormat::Bgra8888);
            renderer->cancelFrame();
            m_background->setTexture(m_wallpaper.get());
            m_background->setGeometry(m_output);
        }
    }

    ~Desktop()
    {
        render::EglRenderer *renderer = m_gl.renderer();
        renderer->beginFrame(m_gl.target());
        m_shell.reset();
        m_background->setTexture(nullptr);
        m_wallpaper.reset();
        m_scene.root()->releaseRenderResources();
        renderer->clearTextureCache();
        renderer->cancelFrame();
        m_seat.shutdown();
    }

    void createShell(base::dbus::Connection *bus)
    {
        shell::ShellEnvironment environment;
        environment.compositor = &m_compositor;
        environment.windowManager = &m_manager;
        environment.seat = &m_seat;
        environment.bus = bus;
        environment.theme = &m_theme;
        m_shell = std::make_unique<shell::Shell>(environment);
    }

    shell::Shell *shell() const { return m_shell.get(); }
    const base::Rect &output() const { return m_output; }

    void move(double x, double y)
    {
        m_time += 1;
        m_input.onPointerMotionAbsolute(stamp(), {x, y});
    }

    void click(double x, double y, uint32_t button)
    {
        move(x, y);
        m_input.onPointerButton(stamp(), button, true);
        m_input.onPointerButton(stamp(), button, false);
    }

    void key(uint32_t code)
    {
        m_input.onKey(stamp(), code, true);
        m_input.onKey(stamp(), code, false);
    }

    // Frames 16.7 ms apart, with the loop turning between them, for `ms`.
    void run(int ms)
    {
        for (int elapsed = 0; elapsed <= ms; elapsed += 17) {
            test::runFor(std::chrono::milliseconds(17));
            frame();
        }
    }

private:
    input::Timestamp stamp() const
    {
        return input::Timestamp(std::chrono::microseconds(int64_t(m_time * 1000)));
    }

    void frame()
    {
        render::EglRenderer *renderer = m_gl.renderer();
        m_time += 1000.0 / 60;
        m_compositor.frameStarted(m_time);
        renderer->setOutputContext({"out", m_output, kNative, m_scale, 0, ++m_frame});
        renderer->frameTime = m_time;
        renderer->framePeriod = 1000.0 / 60;
        if (!renderer->beginFrame(m_gl.target()))
            return;
        m_compositor.updateTextures(*renderer);
        m_scene.render(*renderer);
        renderer->endFrame();
        renderer->frameTime = -1;
    }

    Renderer &m_gl;
    const double m_scale;
    const base::Rect m_output;
    double m_time = 100000;
    uint64_t m_frame = 0;
    int m_statusBar = 0;
    scene::Scene m_scene;
    scene::TextureNode *m_background =
            static_cast<scene::TextureNode *>(m_scene.root()->addChild(
                    std::make_unique<scene::TextureNode>(nullptr, "wallpaper")));
    scene::Node *m_surfaces = m_scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor m_compositor {m_surfaces};
    decoration::Theme m_theme;
    decoration::DecorationProvider m_frames {&m_theme};
    input::Seat m_seat {&m_compositor};
    input::InputManager m_input {&m_seat};
    wm::WindowManager m_manager {&m_compositor, &m_seat};
    decoration::ShadowProvider m_shadows {&m_theme};
    wm::WmShadowControllerDelegate m_shadowDelegate;
    wm::ShadowController m_shadowController {&m_manager, &m_shadows, &m_shadowDelegate};
    std::unique_ptr<render::Texture> m_wallpaper;
    std::unique_ptr<shell::Shell> m_shell;
};

double msSince(std::chrono::steady_clock::time_point start)
{
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

// The user's shell.conf, copied so that nothing here writes to it.
std::string shellConf()
{
    static const std::string copy = [] {
        const std::string path =
                (std::filesystem::temp_directory_path() / "lingmo-shell-memory-bench.conf").string();
        std::error_code error;
        std::filesystem::copy_file(preferencesFile("shell"), path,
                                   std::filesystem::copy_options::overwrite_existing, error);
        return path;
    }();
    return copy;
}

// A component: added to the shell, then used as a person would once.
struct Component {
    std::function<void(Desktop &, base::dbus::Connection *)> add;
    std::function<void(Desktop &)> exercise;
};

std::map<std::string, Component> components()
{
    std::map<std::string, Component> all;
    static shell::Osd *osd = nullptr;
    all["osd"] = {
            [](Desktop &desktop, base::dbus::Connection *bus) {
                osd = desktop.shell()->addComponent(std::make_unique<shell::Osd>(
                        desktop.shell(), shell::createSessionOsdLevels(bus)));
            },
            [](Desktop &desktop) {
                osd->volumeUp();
                desktop.run(600);
                osd->volumeDown();
                osd->brightnessUp();
                desktop.run(600);
                osd->brightnessDown();
                desktop.run(2000);
            }};
    static shell::DockWindow *dock = nullptr;
    all["dock"] = {
            [](Desktop &desktop, base::dbus::Connection *bus) {
                auto service = std::make_unique<shell::Dock>(bus, desktop.shell()->tasks(),
                                                             desktop.shell()->launcher(),
                                                             shellConf());
                shell::Dock *dockService = service.get();
                dock = desktop.shell()->addComponent(
                        std::make_unique<shell::DockWindow>(desktop.shell(), std::move(service)));
                test::runUntil([&] { return dockService->applications().isLoaded(); },
                               std::chrono::seconds(10));
            },
            [](Desktop &desktop) {
                const base::Rect bounds = dock->window()->geometry();
                const double y = bounds.y() + bounds.height() / 2.0;
                const auto icons = dock->icons();
                for (size_t i = 0; i < icons.size(); ++i) {
                    desktop.move(bounds.x() + bounds.width() * (i + 0.5) / icons.size(), y);
                    desktop.run(300);
                }
                desktop.click(bounds.x() + bounds.width() * 1.5 / std::max<size_t>(icons.size(), 1), y,
                              BTN_RIGHT);
                desktop.run(600);
                desktop.key(KEY_ESC);
                desktop.move(desktop.output().width() * 0.45, desktop.output().height() * 0.45);
                desktop.run(1200);
            }};
    return all;
}

} // namespace

int main(int argc, char **argv)
{
    double scale = 1.5;
    bool dark = false;
    int holdSeconds = 0;
    std::string wallpaperPath = "/usr/share/backgrounds/lingmoos/default.jpg";
    std::vector<std::string> wanted;
    if (std::none_of(argv + 1, argv + argc, [](const char *arg) { return std::string(arg) == "--glibc-defaults"; }))
        core::applyMallocPolicy();
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--scale" && i + 1 < argc)
            scale = std::stod(argv[++i]);
        else if (arg == "--dark")
            dark = true;
        else if (arg == "--glibc-defaults")
            continue;
        else if (arg == "--cpu-panels")
            toolkit::setGpuRenderingAllowed(false);
        else if (arg == "--hold" && i + 1 < argc)
            holdSeconds = std::stoi(argv[++i]);
        else if (arg == "--no-thp")
            prctl(PR_SET_THP_DISABLE, 1, 0, 0, 0);
        else if (arg == "--mmap-threshold" && i + 1 < argc)
            mallopt(M_MMAP_THRESHOLD, std::stoi(argv[++i]));
        else if (arg == "--arena-max" && i + 1 < argc)
            mallopt(M_ARENA_MAX, std::stoi(argv[++i]));
        else if (arg == "--wallpaper" && i + 1 < argc)
            wallpaperPath = argv[++i];
        else
            wanted.push_back(arg);
    }
    auto all = components();
    for (const std::string &name : wanted) {
        if (!all.count(name)) {
            std::fprintf(stderr, "unknown component %s; known:", name.c_str());
            for (const auto &[known, component] : all)
                std::fprintf(stderr, " %s", known.c_str());
            std::fprintf(stderr, "\n");
            return 2;
        }
    }

    core::MainLoop loop;
    base::dbus::Connection *bus = base::dbus::Connection::session(core::mainLoop());
    Sample last = sample();
    printHeader();
    printSample("process", last, nullptr);
    const auto step = [&](const std::string &name) {
        const Sample now = sample();
        printSample(name, now, &last);
        last = now;
    };

    Renderer gl;
    if (!gl.init()) {
        std::fprintf(stderr, "no EGL renderer\n");
        return 1;
    }
    const ui::Bitmap wallpaper = ui::Bitmap::decodeFile(wallpaperPath, kNative, ui::Bitmap::Scaling::Cover);
    {
        Desktop desktop(gl, scale, dark, wallpaper);
        desktop.run(500);
        step("renderer+wallpaper");
        auto started = std::chrono::steady_clock::now();
        desktop.createShell(bus);
        std::printf("created shell in %.1f ms\n", msSince(started));
        desktop.run(500);
        step("shell");
        for (const std::string &name : wanted) {
            started = std::chrono::steady_clock::now();
            all[name].add(desktop, bus);
            std::printf("created %s in %.1f ms\n", name.c_str(), msSince(started));
            desktop.run(1000);
            step("+" + name);
        }
        for (const std::string &name : wanted) {
            all[name].exercise(desktop);
            step(name + " used");
        }
        desktop.run(3000);
        step("idle 3 s");
        toolkit::purgeGpuResources();
        // The driver keeps freed buffers about a second for reuse.
        desktop.run(2000);
        step("skia purged");
        malloc_trim(0);
        step("malloc_trim");
        // For a look from outside, e.g. at the GPU driver's buffer list.
        if (holdSeconds > 0) {
            std::printf("holding pid %d for %d s\n", int(getpid()), holdSeconds);
            std::fflush(stdout);
            desktop.run(holdSeconds * 1000);
            step("held " + std::to_string(holdSeconds) + " s");
        }
    }
    return 0;
}
