// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GLES3/gl3.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "base/elapsed_timer.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/strings.h"
#include "base/url.h"
#include "clipboard_test_util.h"
#include "compositor/output.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "decoration/shadow_node.h"
#include "egl_test_util.h"
#include "process_test_util.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "temporary_directory.h"
#include "ui/bitmap.h"
#include "wayland/protocols/wl_output.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/protocols/wl_subcompositor.h"
#include "wayland/protocols/wp_fractional_scale.h"
#include "wayland/protocols/wp_viewporter.h"
#include "wayland/protocols/xdg_decoration.h"
#include "wayland/protocols/xdg_shell.h"
#include "wm/shadow_controller.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_visual_controller.h"
#include "wm/wm_shadow_controller_delegate.h"

using namespace Lingmo;

namespace {

// The bottom-right pixel inside: right() and bottom() are the far edges.
base::Point lastPixel(const base::Rect &rect)
{
    return base::Point(rect.right() - 1, rect.bottom() - 1);
}

// A frame read back from GL, bottom row first.
std::vector<uint8_t> readFrame()
{
    std::vector<uint8_t> pixels(size_t(1920) * 1080 * 4);
    glReadPixels(0, 0, 1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    return pixels;
}

bool saveFrame(std::vector<uint8_t> &pixels, const std::string &path)
{
    return ui::Bitmap::wrap(pixels.data(), 1920, 1080, 1920 * 4,
                            ui::Bitmap::Format::Rgba8888Premultiplied)
            .mirrored(false, true)
            .savePng(path);
}

std::string environmentVariable(const char *name)
{
    const char *value = std::getenv(name);
    return value ? value : "";
}

void acceptClient(const std::string &program, std::vector<std::string> arguments,
                  scene::WindowVisualMode expected,
                  bool exerciseResize = false, bool interactiveResize = false,
                  double bufferScale = 1.0, bool exerciseDamage = false)
{
    test::TemporaryDirectory runtime;
    ASSERT_TRUE(runtime.isValid());
    chmod(runtime.path().c_str(), 0700);

    const test::ScopedEnvironmentVariable runtimeVariable("XDG_RUNTIME_DIR", runtime.path());

    struct DeviceFd {
        int fd = -1;

        ~DeviceFd()
        {
            if (fd >= 0)
                close(fd);
        }
    } deviceFd;

    std::unique_ptr<gbm_device, decltype(&gbm_device_destroy)> device(nullptr, gbm_device_destroy);
    std::unique_ptr<gbm_surface, decltype(&gbm_surface_destroy)> gbmSurface(nullptr,
                                                                            gbm_surface_destroy);
    std::unique_ptr<render::EglRenderer> renderer;
    std::unique_ptr<render::EglRenderTarget> target;
    const std::string devicePath = environmentVariable("LINGMO_TEST_DRM_DEVICE");
    if (devicePath.empty()) {
        renderer = render::EglRendererTestAccess::create();
        ASSERT_NE(renderer, nullptr);
        const EGLint attrs[] = {EGL_WIDTH, 1920, EGL_HEIGHT, 1080, EGL_NONE};
        const auto eglSurface =
                eglCreatePbufferSurface(renderer->eglDisplay(), renderer->eglConfig(), attrs);
        ASSERT_NE(eglSurface, EGL_NO_SURFACE);
        target = std::make_unique<render::EglRenderTarget>(renderer->eglDisplay(), eglSurface,
                                                           nullptr, base::Size(1920, 1080));
    } else {
        deviceFd.fd = open(devicePath.c_str(), O_RDWR | O_CLOEXEC);
        ASSERT_GE(deviceFd.fd, 0);
        device.reset(gbm_create_device(deviceFd.fd));
        ASSERT_TRUE(device);
        renderer = render::EglRenderer::create(device.get(), DRM_FORMAT_ARGB8888);
        ASSERT_TRUE(renderer);
        gbmSurface.reset(gbm_surface_create(device.get(), 1920, 1080, DRM_FORMAT_ARGB8888,
                                            GBM_BO_USE_RENDERING));
        ASSERT_TRUE(gbmSurface);
        target = renderer->createTarget(gbmSurface.get(), {1920, 1080});
        ASSERT_TRUE(target);
    }
    render::TargetRetirement retire {renderer.get(), target.get()};
    ASSERT_TRUE(renderer->beginFrame(target.get()));
    clipboard_test::Harness h;
    compositor::Output output;
    output.pixelSize = {1920, 1080};
    output.scale = *compositor::OutputScale::parse(std::to_string(bufferScale));
    h.compositor.setOutputScale(output.scale);
    h.scene.setSize(output.logicalSize());
    decoration::Theme theme;
    decoration::DecorationProvider frames(&theme);
    wm::WindowManager manager(&h.compositor, &h.seat);
    manager.setOutputBounds(base::Rect(output.logicalSize()));
    manager.setFrameProvider(&frames);
    decoration::ShadowProvider shadows(&theme);
    wm::WmShadowControllerDelegate delegate;
    wm::ShadowController shadowController(&manager, &shadows, &delegate);
    wayland::WaylandXdgShell xdg(&h.compositor, &manager, &h.seat, h.server.serialTracker());

    struct Cleanup {
        wl_display *display;

        ~Cleanup() { wl_display_destroy_clients(display); }
    } cleanup {h.server.display()};

    wayland::createSubcompositorGlobal(h.server.display());
    wayland::createShmGlobal(h.server.display(), &h.compositor);
    auto *outputGlobal = wayland::createOutputGlobal(h.server.display(), &output);
    wayland::createXdgOutputManagerGlobal(h.server.display());
    wayland::createFractionalScaleGlobal(h.server.display());
    wayland::createViewporterGlobal(h.server.display());
    wayland::createXdgShellGlobal(h.server.display(), &xdg);
    wayland::createXdgDecorationGlobal(h.server.display());
    ASSERT_TRUE(h.server.addSocket("window-visual-toolkit"));
    ASSERT_TRUE(h.server.start());

    // Killed and waited for when it goes.
    base::Process client;

    std::vector<std::string_view> removed = {"DISPLAY", "LD_PRELOAD"};
    std::map<std::string, std::string> env = {
            {"WAYLAND_DISPLAY", h.server.socketName()},
            {"XDG_RUNTIME_DIR", runtime.path()},
            {"XDG_CONFIG_HOME", runtime.path() + "/config"},
            {"GDK_BACKEND", "wayland"},
            {"GSK_RENDERER", "cairo"},
            {"NO_AT_BRIDGE", "1"},
            {"WAYLAND_DEBUG", "1"}};
    bool chromium = false;
    for (auto &arg : arguments) {
        chromium = chromium || arg.find("@PROFILE@") != std::string::npos;
        arg = base::strings::replaceAll(arg, "@PROFILE@", runtime.path() + "/profile");
    }
    // Chromium and Electron start a11y, secret, portal and gvfs services on
    // any session bus they reach, and those run with the bus's environment
    // rather than this test's display.
    if (chromium)
        env["DBUS_SESSION_BUS_ADDRESS"] = "unix:path=" + runtime.path() + "/no-bus";
    client.setEnvironment(test::environmentWith(removed, env));
    client.start(program, arguments);
    std::string log;
    uint64_t generation = 0;
    ASSERT_TRUE(h.until(
            [&] {
                log = client.standardError();
                h.compositor.updateTextures(*renderer);
                h.compositor.frameSubmitted(++generation);
                h.compositor.frameDrawn(wayland::nowInMilliseconds(), generation);
                wayland::updateOutputMembership(outputGlobal);
                return !manager.windows().empty()
                        && manager.windows().front()->rootSurface()->hasContents();
            },
            15000))
            << log;
    auto *window = manager.windows().front();
    const auto checkRoundedCorners = [&] {
        const auto visual = wm::WindowVisualController::evaluate(*window);
        const base::RectF body = visual.shape.bounds.translated(window->bounds().topLeft());
        const double scale = renderer->rasterScale();
        const base::Rect pixels(base::roundToInt(body.x() * scale), base::roundToInt(body.y() * scale),
                                base::roundToInt(body.width() * scale),
                                base::roundToInt(body.height() * scale));
        const base::Point last = lastPixel(pixels);
        for (const auto corner : {pixels.topLeft(), base::Point(last.x(), pixels.y()),
                                  base::Point(pixels.x(), last.y()), last}) {
            for (int dx : {0, 2, 4, 6}) {
                for (int dy : {0, 2, 4, 6}) {
                    const base::Point sample = corner
                            + base::Point(corner.x() == pixels.x() ? dx : -dx,
                                          corner.y() == pixels.y() ? dy : -dy);
                    const base::PointF local = (base::PointF(sample) + base::PointF(.5, .5)) / scale
                            - base::PointF(window->bounds().topLeft());
                    if (!base::Rect(0, 0, 1920, 1080).contains(sample) || visual.shape.contains(local))
                        continue;
                    uint8_t pixel[4] = {};
                    glReadPixels(sample.x(), 1080 - sample.y() - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                 pixel);
                    EXPECT_LT(pixel[3], 240) << "corner=" << sample.x() << ',' << sample.y();
                }
            }
        }
    };
    if (expected == scene::WindowVisualMode::ServerFrame) {
        const auto oldShape = wm::WindowVisualController::evaluate(*window).shape.bounds;
        const base::Size nextContent = window->rootSurface()->contentSize() + base::Size(40, 24);
        window->requestBounds({window->bounds().topLeft(), window->frameInsets().outset(nextContent)});
        EXPECT_EQ(wm::WindowVisualController::evaluate(*window).shape.bounds, oldShape);
        ASSERT_TRUE(h.until(
                [&] {
                    log = client.standardError();
                    h.compositor.updateTextures(*renderer);
                    h.compositor.frameSubmitted(++generation);
                    h.compositor.frameDrawn(wayland::nowInMilliseconds(), generation);
                    return window->rootSurface()->contentSize() == nextContent;
                },
                5000))
                << log;
        EXPECT_EQ(wm::WindowVisualController::evaluate(*window).shape.bounds.size(),
                  base::SizeF(window->bounds().size()));
    }
    if (interactiveResize) {
        static_cast<scene::WindowAnimation *>(window->node())->finish();
        auto *surface = window->rootSurface();
        const base::Rect initial = window->bounds();
        const base::Point fixedCorner = lastPixel(surface->node()->absoluteGeometry());
        ASSERT_TRUE(manager.eventHandler()->beginResize(window, wm::ResizeEdge::TopLeft,
                                                        initial.topLeft()));
        base::ElapsedTimer timer;
        timer.start();
        int step = 0;
        ASSERT_TRUE(h.until(
                [&] {
                    log = client.standardError();
                    if (step < 30 && timer.elapsedMs() >= step * 12) {
                        ++step;
                        manager.eventHandler()->onPointerMotion(initial.topLeft()
                                                                - base::Point(step * 3, step * 2));
                    }
                    h.compositor.updateTextures(*renderer);
                    EXPECT_EQ(surface->bufferSize(),
                              base::Size(base::roundToInt(surface->contentSize().width() * bufferScale),
                                         base::roundToInt(surface->contentSize().height() * bufferScale)));
                    renderer->clear({0, 0, 0, 0});
                    window->node()->render(*renderer);
                    checkRoundedCorners();
                    uint8_t pixel[4] = {};
                    const base::Point sample = fixedCorner - base::Point(20, 20);
                    glReadPixels(sample.x(), 1080 - sample.y() - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                 pixel);
                    EXPECT_GT(pixel[3], 240);
                    EXPECT_EQ(lastPixel(surface->node()->absoluteGeometry()), fixedCorner)
                            << "step=" << step << " buffer=" << surface->bufferSize().width();
                    h.compositor.frameSubmitted(++generation);
                    h.compositor.frameDrawn(wayland::nowInMilliseconds(), generation);
                    return step == 30 && surface->contentSize() == window->bounds().size();
                },
                5000))
                << log;
        manager.eventHandler()->completeDrag(wm::ToplevelEventHandler::DragResult::Success);
    }
    if (exerciseResize) {
        static_cast<scene::WindowAnimation *>(window->node())->finish();
        const base::Size baseSize = window->bounds().size();
        for (int step = 0; step < 8; ++step) {
            const base::Size nextSize = baseSize + (step % 2 ? base::Size(30, 20) : base::Size(100, 80));
            const base::Rect initial = window->bounds();
            const auto edge = step % 2 ? wm::ResizeEdge::TopLeft : wm::ResizeEdge::BottomRight;
            const base::Point start = step % 2 ? initial.topLeft() : lastPixel(initial);
            ASSERT_TRUE(manager.eventHandler()->beginResize(window, edge, start));
            const base::Point delta(nextSize.width() - initial.width(),
                                    nextSize.height() - initial.height());
            manager.eventHandler()->onPointerMotion(start + (step % 2 ? -delta : delta));
            ASSERT_TRUE(h.until(
                    [&] {
                        log = client.standardError();
                        h.compositor.updateTextures(*renderer);
                        auto *surface = window->rootSurface();
                        auto *texture = surface->node()->texture();
                        EXPECT_NE(texture, nullptr);
                        if (texture)
                            EXPECT_EQ(texture->size(), surface->bufferSize());
                        renderer->clear({0, 0, 0, 0});
                        window->node()->render(*renderer);
                        checkRoundedCorners();
                        const base::Point center = wm::centerPixel(window->bounds());
                        uint8_t pixel[4] = {};
                        glReadPixels(center.x(), 1080 - center.y() - 1, 1, 1, GL_RGBA,
                                     GL_UNSIGNED_BYTE, pixel);
                        EXPECT_GT(pixel[3], 240)
                                << "step=" << step << " bounds=" << window->bounds().width()
                                << " surface=" << window->rootSurface()->contentSize().width()
                                << " texture=" << window->rootSurface()->node()->texture()
                                << " buffer=" << window->rootSurface()->buffer().get()
                                << " visible="
                                << window->rootSurface()->node()->isEffectivelyVisible();
                        h.compositor.frameSubmitted(++generation);
                        h.compositor.frameDrawn(wayland::nowInMilliseconds(), generation);
                        return window->rootSurface()->contentSize()
                                == window->clientExteriorInsets().outset(nextSize);
                    },
                    5000))
                    << log;
            manager.eventHandler()->completeDrag(wm::ToplevelEventHandler::DragResult::Success);
        }
    }
    h.compositor.updateTextures(*renderer);
    ASSERT_NE(window->rootSurface()->node()->texture(), nullptr);
    static_cast<scene::WindowAnimation *>(window->node())->finish();
    renderer->clear({0.2f, 0.3f, 0.4f, 1});
    window->node()->render(*renderer);
    std::vector<uint8_t> image = readFrame();
    ASSERT_EQ(glGetError(), GLenum(GL_NO_ERROR));
    const std::string artifacts = environmentVariable("LINGMO_VISUAL_ARTIFACT_DIR");
    if (!artifacts.empty()) {
        const std::string path =
                artifacts + "/" + std::filesystem::path(program).filename().string();
        EXPECT_TRUE(saveFrame(image, path + ".png"));
        const auto original = manager.visualStyle();
        for (bool light : {true, false}) {
            auto preview = original;
            const float value = light ? 1.f : 0.f;
            preview.activeOutline = preview.inactiveOutline = {value, value, value, 0.14f};
            manager.setVisualStyle(preview);
            renderer->clear({0.2f, 0.3f, 0.4f, 1});
            window->node()->render(*renderer);
            image = readFrame();
            ASSERT_EQ(glGetError(), GLenum(GL_NO_ERROR));
            EXPECT_TRUE(saveFrame(image, path + (light ? "-light-edge.png" : "-dark-edge.png")));
        }
        manager.setVisualStyle(original);
    }
    const auto state = wm::WindowVisualController::evaluate(*window);
    EXPECT_EQ(state.mode, expected) << log;
    EXPECT_EQ(window->isDecorated(), expected == scene::WindowVisualMode::ServerFrame);
    EXPECT_TRUE(state.clipContent);
    EXPECT_TRUE(state.drawOutline);
    EXPECT_EQ(state.shape.radius, 14);
    EXPECT_NE(shadowController.shadowForWindow(window)->elevation(), wm::ShadowElevation::None);
    if (expected == scene::WindowVisualMode::ServerExterior) {
        EXPECT_TRUE(window->frameInsets().isNull());
        EXPECT_EQ(window->contentBounds(), window->bounds());
    }
    if (exerciseDamage) {
        renderer->setOutputContext(
                {"toolkit", base::Rect(base::Point(), output.logicalSize()), {1920, 1080},
                 bufferScale});
        scene::DamageTracker damageTracker;
        int partialFrames = 0;
        for (int frame = 0; frame < 8; ++frame) {
            const auto revision = window->rootSurface()->node()->contentRevision();
            ASSERT_TRUE(h.until(
                    [&] {
                        log = client.standardError();
                        h.compositor.updateTextures(*renderer);
                        h.compositor.frameSubmitted(++generation);
                        h.compositor.frameDrawn(wayland::nowInMilliseconds(), generation);
                        return window->rootSurface()->node()->contentRevision() != revision;
                    },
                    5000))
                    << log;
            const auto damage = damageTracker.update(
                    *h.scene.root(), renderer->outputContext().logicalViewport, bufferScale);
            const auto physical =
                    render::physicalDamage(damage, renderer->outputContext(), {1920, 1080});
            partialFrames += physical.width() * physical.height() < 1920 * 1080;
            render::EglRendererTestAccess::applyRepaint(*renderer, physical);
            renderer->clear(h.scene.background());
            h.scene.root()->render(*renderer);
            const std::vector<uint8_t> partial = readFrame();
            glDisable(GL_SCISSOR_TEST);
            renderer->clear(h.scene.background());
            h.scene.root()->render(*renderer);
            const std::vector<uint8_t> complete = readFrame();
            EXPECT_TRUE(partial == complete) << "frame=" << frame << " scale=" << bufferScale;
        }
        EXPECT_GT(partialFrames, 0);
        std::printf("DAMAGE renderer=%s scale=%.2f partial=%d/8\n", glGetString(GL_RENDERER),
                    bufferScale, partialFrames);
    }
    std::printf("OBSERVED %s mode=%d client=%dx%d exterior=%d,%d,%d,%d\n",
                program.c_str(), int(state.mode), window->contentBounds().width(),
                window->contentBounds().height(), window->clientExteriorInsets().left,
                window->clientExteriorInsets().top, window->clientExteriorInsets().right,
                window->clientExteriorInsets().bottom);
}

} // namespace

TEST(WindowVisualToolkit, GtkCsd)
{
    const auto gtk = base::paths::findExecutable("gtk3-demo");
    if (gtk.empty())
        GTEST_SKIP() << "gtk3-demo is not installed";
    acceptClient(gtk, {}, scene::WindowVisualMode::ServerExterior, true);
}

TEST(WindowVisualToolkit, Gtk4CsdResize)
{
    const auto gtk = base::paths::findExecutable("gtk4-demo");
    if (gtk.empty())
        GTEST_SKIP() << "gtk4-demo is not installed";
    acceptClient(gtk, {}, scene::WindowVisualMode::ServerExterior, true);
}

TEST(WindowVisualToolkit, ChromiumClientChrome)
{
    const auto chromium = base::paths::findExecutable("chromium");
    if (chromium.empty())
        GTEST_SKIP() << "Chromium is not installed";
    acceptClient(chromium,
                 {"--ozone-platform=wayland", "--disable-gpu", "--no-first-run",
                  "--no-default-browser-check", "--disable-background-networking",
                  "--disable-dev-shm-usage", "--user-data-dir=@PROFILE@", "--window-size=640,480",
                  "about:blank"},
                 scene::WindowVisualMode::ServerExterior, true);
}

TEST(WindowVisualToolkit, ElectronClientChrome)
{
    const auto electron = base::paths::findExecutable("electron");
    if (electron.empty())
        GTEST_SKIP() << "Electron is not installed";
    acceptClient(electron,
                 {ELECTRON_VISUAL_CLIENT, "--ozone-platform=wayland",
                  "--disable-gpu", "--user-data-dir=@PROFILE@"},
                 scene::WindowVisualMode::ServerExterior);
}

TEST(WindowVisualToolkit, ToolkitInteractiveResize)
{
    for (const double scale : {1.0, 1.25, 1.5, 1.75, 2.0}) {
        SCOPED_TRACE(scale);
        acceptClient(TOOLKIT_RESIZE_CLIENT, {},
                     scene::WindowVisualMode::ServerExterior, false, true, scale);
    }
}

TEST(WindowVisualToolkit, ToolkitScrollingDamage)
{
    for (double scale : {1.25, 1.5})
        acceptClient(TOOLKIT_RESIZE_CLIENT, {"--scroll"},
                     scene::WindowVisualMode::ServerExterior, false, true, scale, true);
}

TEST(WindowVisualToolkit, ChromiumScrollingDamage)
{
    const auto chromium = base::paths::findExecutable("chromium");
    if (chromium.empty())
        GTEST_SKIP() << "Chromium is not installed";
    const std::string html =
            "<body style='height:10000px;background:repeating-linear-gradient(white 0px,white "
            "20px,blue 20px,blue 40px)'><script>setInterval(()=>scrollBy(0,9),50)</script>";
    const std::string page = "data:text/html," + base::url::percentEncoded(html);
    for (double scale : {1.25, 1.5})
        acceptClient(chromium,
                     {"--ozone-platform=wayland", "--disable-gpu", "--no-first-run",
                      "--no-default-browser-check", "--disable-background-networking",
                      "--disable-dev-shm-usage", "--user-data-dir=@PROFILE@",
                      "--window-size=640,480", page},
                     scene::WindowVisualMode::ServerExterior, true, false, scale, true);
}
