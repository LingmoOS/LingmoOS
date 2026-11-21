// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/screenshot/screen_capture.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <format>
#include <mutex>
#include <set>

#include "base/dbus.h"
#include "base/file.h"
#include "base/i18n.h"
#include "base/logging.h"
#include "base/event_loop.h"
#include "core/main_loop.h"
#include "shell/common/task_manager.h"
#include "shell/common/task_windows.h"
#include "shell/screenshot/capture_clipboard.h"
#include "shell/screenshot/capture_notifier.h"
#include "shell/screenshot/capture_overlay.h"
#include "shell/screenshot/capture_settings.h"
#include "shell/screenshot/screen_grabber.h"
#include "shell/screenshot/screen_recording.h"
#include "ui/bitmap_painter.h"
#include "ui/file_chooser.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static const base::LogCategory lingmoScreenshot("lingmo.shell.screenshot");

// The names saves are writing, so two captures in one second pick two.
struct ScreenCapture::SavingNames {
    std::mutex mutex;
    std::set<std::string> taken;
};

// The centre rounded down to the pixel before the middle, one less than ours
// for even sizes; it decides which screen a rectangle is on.
static base::Point roundedDownCenter(const base::Rect &rect)
{
    return base::Point(rect.x() + (rect.width() - 1) / 2, rect.y() + (rect.height() - 1) / 2);
}

static std::string timestamp()
{
    const std::time_t now = std::time(nullptr);
    std::tm local {};
    localtime_r(&now, &local);
    char date[16];
    char time[16];
    std::strftime(date, sizeof(date), "%Y-%m-%d", &local);
    std::strftime(time, sizeof(time), "%H.%M.%S", &local);
    return std::format("{} at {}", date, time);
}

// On a worker: the first free name, taken until released.
static std::string uniqueName(ScreenCapture::SavingNames &names, const std::string &folder,
                              const std::string &stem, const std::string &suffix)
{
    const std::lock_guard lock(names.mutex);
    std::string path = folder + "/" + stem + "." + suffix;
    for (int n = 2;; ++n) {
        std::error_code error;
        if (!std::filesystem::exists(path, error) && !names.taken.contains(path))
            break;
        path = std::format("{}/{} ({}).{}", folder, stem, n, suffix);
    }
    names.taken.insert(path);
    return path;
}

// Every screen side by side as the user sees them, at the finest scale.
static ui::Bitmap combine(const std::vector<ui::Bitmap> &images, const std::vector<base::Rect> &geometries)
{
    base::Rect bounds;
    double scale = 1;
    for (size_t i = 0; i < images.size(); ++i) {
        bounds = bounds.united(geometries[i]);
        scale = std::max(scale, double(images[i].width()) / geometries[i].width());
    }
    ui::Bitmap canvas(base::SizeF(bounds.size().width() * scale, bounds.size().height() * scale).toSize(),
                      ui::Bitmap::Format::Argb32Premultiplied);
    canvas.fill(0xff000000u);
    ui::BitmapPainter painter(canvas);
    for (size_t i = 0; i < images.size(); ++i) {
        const base::RectF target((geometries[i].x() - bounds.x()) * scale, (geometries[i].y() - bounds.y()) * scale,
                                 geometries[i].width() * scale, geometries[i].height() * scale);
        painter.drawBitmap(target, images[i], true);
    }
    return canvas;
}

ScreenCapture::ScreenCapture(Shell *shell, ScreenCaptureServices services)
    : m_shell(shell)
    , m_services(std::move(services))
    , m_settings(std::make_unique<CaptureSettings>(m_services.settingsPath))
    , m_clipboard(std::make_unique<CaptureClipboard>(m_services.selection))
    , m_notifier(std::make_unique<CaptureNotifier>(shell->bus()))
    , m_recording(std::make_unique<ScreenRecording>(m_services.recorder, m_services.recorderEnvironment,
                                                    m_services.recorderConnection))
    , m_saving(std::make_shared<SavingNames>())
{
    m_countdownTimer.setInterval(std::chrono::milliseconds(1000));
    m_countdownTimer.onTimeout = [this] {
        if (--m_countdown > 0) {
            notifyChanged();
            return;
        }
        m_countdownTimer.stop();
        m_countdown = 0;
        notifyChanged();
        if (auto pending = std::exchange(m_pending, nullptr))
            pending();
    };

    m_subscriptions.push_back(m_recording->recordingChanged.connect([this] {
        recordingChanged();
        if (m_service)
            m_service->emitPropertiesChanged({"recording"});
        notifyChanged();
    }));
    m_subscriptions.push_back(m_recording->finished.connect([this](bool ok, const std::string &error) {
        if (ok)
            m_notifier->notifySaved(m_recording->path(), true);
        else
            m_notifier->notifyFailed(tr("Screen Recording Failed"), error);
        recordingChanged();
        if (m_service)
            m_service->emitPropertiesChanged({"recording"});
        notifyChanged();
    }));
    // Its microphone use shows under Screenshot, as its notifications do.
    m_recording->onStarted = [this](int64_t pid) { m_shell->tasks().claimProcess(pid, "lingmo-screenshot"); };
    m_recording->onStopped = [this](int64_t pid) { m_shell->tasks().releaseProcess(pid); };

    // What is under a still pointer changes as windows come, go, move and are
    // raised; Ash picks again on the same occasions. The dock grows and
    // shrinks with them, which moves the toolbar's place.
    TaskWindows &windows = m_shell->taskWindows();
    m_subscriptions.push_back(windows.windowChanged.connect([this](uint64_t, uint32_t changes) {
        if (changes & (kGeometryChange | kStateChange)) {
            pick();
            notifyChanged();
        }
    }));
    m_subscriptions.push_back(windows.windowAdded.connect([this](uint64_t) {
        pick();
        notifyChanged();
    }));
    m_subscriptions.push_back(windows.windowRemoved.connect([this](uint64_t) {
        pick();
        notifyChanged();
    }));
    m_subscriptions.push_back(windows.activeWindowChanged.connect([this](uint64_t) { pick(); }));
    m_subscriptions.push_back(m_shell->outputsChanged.connect([this] {
        std::erase_if(m_overlays, [this](const std::unique_ptr<CaptureOverlay> &overlay) {
            return !std::any_of(m_shell->outputs().begin(), m_shell->outputs().end(),
                                [&](const Shell::Output &output) { return output.name == overlay->outputName(); });
        });
        for (const auto &overlay : m_overlays) {
            if (const Shell::Output *output = m_shell->output(overlay->outputName()))
                overlay->setOutput(*output);
        }
        if (m_active)
            syncOverlays();
        notifyChanged();
    }));
    m_subscriptions.push_back(m_settings->changed.connect([this] { notifyChanged(); }));
}

ScreenCapture::~ScreenCapture()
{
    *m_alive = false;
    m_overlays.clear();
}

bool ScreenCapture::registerService()
{
    if (m_service)
        return true;
    dbus::Connection *bus = m_shell->bus();
    if (!bus)
        return false;
    auto object = std::make_unique<dbus::Object>(bus, "/Screenshot", "org.lingmo.Screenshot");
    object->addProperty("recording", [this] { return isRecording(); });
    object->addMethod("takeScreenshot", [this](bool toClipboard) { takeScreenshot(toClipboard); },
                      dbus::ArgumentNames {{"toClipboard"}, {}});
    object->addMethod("takePartialScreenshot", [this](bool toClipboard) { takePartialScreenshot(toClipboard); },
                      dbus::ArgumentNames {{"toClipboard"}, {}});
    object->addMethod("startCaptureMode", [this] { startCaptureMode(); });
    object->addMethod("stopRecording", [this] { stopRecording(); });
    object->addMethod(
            "captureOutput", "sb", "ay",
            [this](dbus::MethodCall &call) {
                std::string output;
                bool cursor = false;
                if (!call.read(output, cursor)) {
                    call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Expected an output and a flag");
                    return;
                }
                captureOutput(output, cursor,
                              [call](std::optional<std::string> png, const std::string &error) mutable {
                                  if (!png) {
                                      call.replyError("org.lingmo.Screenshot.Error.Failed", error);
                                      return;
                                  }
                                  call.reply(std::vector<uint8_t>(png->begin(), png->end()));
                              });
            },
            dbus::ArgumentNames {{"output", "cursor"}, {"png"}});
    if (!object->publish() || !bus->requestName("org.lingmo.Screenshot", dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoScreenshot) << "Cannot own org.lingmo.Screenshot";
        return false;
    }
    m_service = std::move(object);
    return true;
}

std::optional<CaptureDock> ScreenCapture::dock() const
{
    if (!m_services.dock)
        return std::nullopt;
    std::optional<CaptureDock> dock = m_services.dock();
    if (dock && (!dock->window || dock->geometry.isEmpty()))
        return std::nullopt;
    return dock;
}

const Shell::Output *ScreenCapture::outputAt(const base::Point &point) const
{
    for (const Shell::Output &output : m_shell->outputs()) {
        if (output.bounds.contains(point))
            return &output;
    }
    return nullptr;
}

const Shell::Output *ScreenCapture::toolbarScreen() const
{
    // The dock's screen: the toolbar sits right above the dock.
    if (const std::optional<CaptureDock> panel = dock()) {
        if (const Shell::Output *output = outputAt(roundedDownCenter(panel->geometry)))
            return output;
    }
    return m_shell->output({});
}

std::string ScreenCapture::toolbarOutput() const
{
    const Shell::Output *output = toolbarScreen();
    return output ? output->name : std::string();
}

base::Rect ScreenCapture::toolbarArea() const
{
    const Shell::Output *output = toolbarScreen();
    if (!output)
        return {};
    const base::Rect geometry = output->bounds;
    // Inclusive bottom edges, one less than ours.
    int bottom = geometry.bottom() - 1 - kToolbarScreenMargin;
    // Right above the dock when it runs along the bottom.
    if (const std::optional<CaptureDock> panel = dock(); panel && panel->bottom && geometry.intersects(panel->geometry))
        bottom = panel->geometry.y() - kToolbarGap;
    return base::Rect(geometry.x(), geometry.y(), geometry.width(), bottom - geometry.y() + 1);
}

bool ScreenCapture::isRecording() const
{
    return m_recording->isRunning();
}

CaptureOverlay *ScreenCapture::overlay(const std::string &output) const
{
    for (const auto &overlay : m_overlays) {
        if (overlay->outputName() == output)
            return overlay.get();
    }
    return nullptr;
}

void ScreenCapture::takeScreenshot(bool toClipboard)
{
    Target target;
    for (const Shell::Output &output : m_shell->outputs())
        target.outputs.push_back(output.name);
    if (m_active)
        end();
    captureImage(target, toClipboard);
}

void ScreenCapture::takePartialScreenshot(bool toClipboard)
{
    if (m_active)
        return;
    m_type = Image;
    m_source = Region;
    m_selection = {};
    m_clipboardShortcut = toClipboard;
    begin(Style::Selection);
}

void ScreenCapture::startCaptureMode()
{
    if (m_active) {
        // The same shortcut closes the toolbar again, or stops its timer.
        if (m_style == Style::Toolbar)
            end();
        return;
    }

    m_type = m_settings->lastType() == Video && !isRecording() ? Video : Image;
    m_source = std::clamp(m_settings->lastSource(), int(Screen), int(Region));
    m_clipboardShortcut = false;
    m_selection = {};
    const base::Rect last = m_settings->lastSelection();
    if (m_settings->rememberSelection() && !last.isEmpty()) {
        for (const Shell::Output &output : m_shell->outputs()) {
            if (output.bounds.contains(last))
                m_selection = last;
        }
    }
    if (m_selection.isEmpty())
        m_selection = defaultSelection();
    begin(Style::Toolbar);
}

void ScreenCapture::stopRecording()
{
    m_recording->stop();
}

void ScreenCapture::captureOutput(const std::string &output, bool cursor, PictureDone done)
{
    const std::vector<Shell::Output> &outputs = m_shell->outputs();
    const auto named = std::find_if(outputs.begin(), outputs.end(), [&](const Shell::Output &candidate) {
        return !output.empty() && candidate.name == output;
    });
    if (output.empty() ? outputs.size() != 1 : named == outputs.end()) {
        done(std::nullopt, "Select an output with --output NAME; use --list to see names");
        return;
    }
    const std::string name = output.empty() ? outputs.front().name : output;
    ScreenGrabber *grabber = m_services.grabber.get();
    if (!grabber) {
        done(std::nullopt, "The screen could not be captured.");
        return;
    }
    grabber->grabOutputs({name}, cursor,
                         [this, done, alive = std::weak_ptr<bool>(m_alive)](std::vector<ui::Bitmap> images) {
                             if (alive.expired())
                                 return;
                             if (images.empty() || images.front().isNull()) {
                                 done(std::nullopt, "The screen could not be captured.");
                                 return;
                             }
                             m_notifier->playShutter();
                             m_notifier->notifyTaken(images.front());
                             m_jobs.push_back(base::runInThread(
                                     [image = std::move(images.front())] { return image.encodePng(); },
                                     [this, done](std::optional<std::string> png) {
                                         std::erase_if(m_jobs, [](const base::Job &job) { return !job.isPending(); });
                                         if (!png)
                                             done(std::nullopt, "The screenshot could not be encoded.");
                                         else
                                             done(std::move(png), {});
                                     }));
                         });
}

void ScreenCapture::setMode(int type, int source)
{
    if (!m_active || m_countdown > 0)
        return;
    if (type == Video && isRecording())
        return;
    m_type = type == Video ? Video : Image;
    m_source = std::clamp(source, int(Screen), int(Region));
    if (m_style == Style::Toolbar)
        m_settings->setLastMode(m_type, m_source);
    pick();
    if (m_source == Region && m_selection.isEmpty() && m_style == Style::Toolbar)
        m_selection = defaultSelection();
    notifyChanged();
}

void ScreenCapture::cancel()
{
    end();
}

void ScreenCapture::toggleWindowSelection()
{
    if (!m_active || m_style != Style::Selection)
        return;
    m_source = m_source == Window ? Region : Window;
    pick();
    notifyChanged();
}

void ScreenCapture::pointerMoved(const base::Point &position)
{
    m_pointer = position;
    pick();
}

void ScreenCapture::pick()
{
    if (m_countdown > 0)
        return;
    if (!m_active || m_source != Window || !m_pointer) {
        setPicked({});
        return;
    }
    setPicked(resolve(*m_pointer));
}

ScreenCapture::Picked ScreenCapture::resolve(const base::Point &point) const
{
    Picked picked;
    const Shell::Output *output = outputAt(point);
    const wm::Window *window = m_shell->windowManager()->capturableWindowAt(base::PointF(point));
    if (!window)
        return {};
    picked.id = window->id();
    switch (window->role()) {
    case wm::WindowRole::Normal:
        if (const TaskWindow *task = m_shell->taskWindows().window(window->id())) {
            picked.kind = PickedKind::Window;
            picked.geometry = task->geometry;
            // A window that fills its area has square corners.
            picked.radius = task->maximized || task->fullscreen ? 0 : kWindowRadius;
        }
        break;
    case wm::WindowRole::Dock:
        if (const std::optional<CaptureDock> panel = dock(); panel && panel->window == window) {
            picked.kind = PickedKind::Dock;
            picked.geometry = panel->geometry;
            picked.radius = panel->radius;
            output = outputAt(roundedDownCenter(panel->geometry));
        }
        break;
    case wm::WindowRole::Panel:
        picked.kind = PickedKind::Panel;
        picked.geometry = window->bounds();
        output = outputAt(roundedDownCenter(window->bounds()));
        break;
    case wm::WindowRole::Desktop:
        picked.kind = PickedKind::Desktop;
        if (output)
            picked.geometry = output->bounds;
        break;
    default:
        break;
    }
    if (!output || picked.geometry.isEmpty())
        return {};
    picked.output = output->name;
    return picked;
}

void ScreenCapture::setPicked(const Picked &picked)
{
    if (picked == m_picked)
        return;
    m_picked = picked;
    notifyChanged();
}

ScreenCapture::Target ScreenCapture::targetFor(const Picked &picked) const
{
    Target target;
    switch (picked.kind) {
    case PickedKind::Window:
        target.window = picked.id;
        break;
    case PickedKind::Dock:
        // The compositor draws the glass, the rim and the shadow around it.
        target.outputs = {picked.output};
        target.dock = true;
        break;
    case PickedKind::Panel:
        // As it is seen: the glass behind it is the compositor's.
        target.outputs = {picked.output};
        target.region = picked.geometry;
        break;
    case PickedKind::Desktop:
        target.outputs = {picked.output};
        target.desktop = picked.id;
        break;
    case PickedKind::None:
        break;
    }
    return target;
}

void ScreenCapture::setSelection(const base::Rect &selection)
{
    if (selection == m_selection)
        return;
    m_selection = selection;
    notifyChanged();
}

void ScreenCapture::finishSelection(const base::Rect &selection, bool toClipboard)
{
    if (!m_active || selection.width() < 2 || selection.height() < 2)
        return;
    const Shell::Output *output = outputAt(roundedDownCenter(selection));
    if (!output)
        return;
    Target target;
    target.outputs = {output->name};
    target.region = selection.intersected(output->bounds);
    execute(target, toClipboard || m_clipboardShortcut);
}

void ScreenCapture::clickAt(const base::Point &position, const std::string &outputName, bool toClipboard)
{
    if (!m_active || m_countdown > 0)
        return;

    Target target;
    base::Rect area;
    if (m_source == Window) {
        m_pointer = position;
        setPicked(resolve(position));
        if (!m_picked.isValid())
            return;
        target = targetFor(m_picked);
        area = m_picked.geometry;
    } else if (m_source == Screen) {
        const Shell::Output *output = nullptr;
        for (const Shell::Output &candidate : m_shell->outputs()) {
            if (candidate.name == outputName)
                output = &candidate;
        }
        if (!output)
            return;
        target.outputs = {outputName};
        area = output->bounds;
    } else {
        return;
    }
    schedule(target, toClipboard || m_clipboardShortcut, area);
}

void ScreenCapture::perform(bool toClipboard)
{
    if (!m_active || m_countdown > 0)
        return;

    Target target;
    base::Rect area;
    switch (m_source) {
    case Region: {
        const Shell::Output *output = outputAt(roundedDownCenter(m_selection));
        if (!output || m_selection.isEmpty())
            return;
        target.outputs = {output->name};
        target.region = m_selection.intersected(output->bounds);
        area = target.region;
        break;
    }
    case Screen: {
        // A picture of every screen; a recording of the one with the toolbar.
        const Shell::Output *toolbar = toolbarScreen();
        if (!toolbar)
            return;
        if (m_type == Video) {
            target.outputs = {toolbar->name};
        } else {
            for (const Shell::Output &output : m_shell->outputs())
                target.outputs.push_back(output.name);
        }
        area = toolbar->bounds;
        break;
    }
    case Window:
        if (!m_picked.isValid())
            return;
        target = targetFor(m_picked);
        area = m_picked.geometry;
        break;
    }
    schedule(target, toClipboard || m_clipboardShortcut, area);
}

void ScreenCapture::chooseFolder()
{
    const bool video = m_type == Video;
    end();

    m_chooser = std::make_unique<ui::FileChooser>();
    m_chooser->setMode(ui::FileChooser::Mode::OpenFolder);
    m_chooser->setTitle(tr("Choose a Folder"));
    m_chooser->setCurrentFolder(video ? m_settings->videoFolder() : m_settings->imageFolder());
    // Back to the toolbar either way, the way it was left.
    const auto reopen = [this, alive = std::weak_ptr<bool>(m_alive)] {
        core::mainLoop()->post([this, alive] {
            if (alive.expired())
                return;
            m_chooser.reset();
            startCaptureMode();
        });
    };
    m_chooser->onAccepted = [this, video, reopen](const std::vector<std::string> &paths, const std::string &) {
        if (!paths.empty())
            m_settings->setCustomFolder(video, paths.front());
        reopen();
    };
    m_chooser->onRejected = reopen;
    m_chooser->open();
}

void ScreenCapture::begin(Style style)
{
    m_style = style;
    m_toolbar = style == Style::Toolbar;
    m_active = true;
    m_picked = {};
    syncOverlays();
    const std::string focus = toolbarOutput();
    for (const auto &overlay : m_overlays)
        overlay->present(overlay->outputName() == focus);
    notifyChanged();
}

void ScreenCapture::end()
{
    if (!m_active)
        return;
    m_pointer.reset();
    if (m_style == Style::Toolbar && m_settings->rememberSelection() && !m_selection.isEmpty())
        m_settings->setLastSelection(m_selection);
    m_active = false;
    m_countdownTimer.stop();
    m_pending = nullptr;
    m_countdown = 0;
    m_picked = {};
    for (const auto &overlay : m_overlays)
        overlay->dismiss();
    notifyChanged();
}

void ScreenCapture::syncOverlays()
{
    for (const Shell::Output &output : m_shell->outputs()) {
        if (!overlay(output.name))
            m_overlays.push_back(std::make_unique<CaptureOverlay>(this, m_shell, output));
    }
}

void ScreenCapture::notifyChanged()
{
    for (const auto &overlay : m_overlays)
        overlay->update();
}

base::Rect ScreenCapture::defaultSelection() const
{
    const Shell::Output *output = toolbarScreen();
    if (!output)
        return {};
    const base::Rect geometry = output->bounds;
    const base::Size size(geometry.width() / 2, geometry.height() / 2);
    const base::Point centre = roundedDownCenter(geometry);
    return base::Rect(centre.x() - size.width() / 2, centre.y() - size.height() / 2, size.width(), size.height());
}

void ScreenCapture::schedule(const Target &target, bool toClipboard, const base::Rect &area)
{
    const int seconds = m_style == Style::Toolbar ? m_settings->timer() : 0;
    if (seconds <= 0) {
        execute(target, toClipboard);
        return;
    }
    m_countdownArea = area;
    m_countdown = seconds;
    m_pending = [this, target, toClipboard] { execute(target, toClipboard); };
    m_countdownTimer.start();
    // The screen stays usable until the capture is taken; a click elsewhere
    // takes the keyboard back from the interface.
    for (const auto &overlay : m_overlays)
        overlay->setPassThrough(true);
    notifyChanged();
}

void ScreenCapture::execute(const Target &target, bool toClipboard)
{
    const bool wasActive = m_active;
    const bool video = m_active && m_type == Video;
    if (m_active && m_source == Region && m_style == Style::Toolbar)
        m_settings->setLastSelection(m_selection);
    end();
    if (video) {
        startRecording(target);
        return;
    }
    // The crosshair or the camera is the pointer until the window under it
    // sets its own again; a picture that shows the pointer waits for that.
    if (wasActive && m_settings->showPointer()) {
        m_settle = std::make_unique<base::Timer>();
        m_settle->setSingleShot(true);
        m_settle->onTimeout = [this, target, toClipboard] { captureImage(target, toClipboard); };
        m_settle->start(kPointerSettle);
        return;
    }
    captureImage(target, toClipboard);
}

void ScreenCapture::captureImage(const Target &target, bool toClipboard)
{
    ScreenGrabber *grabber = m_services.grabber.get();
    const bool cursor = m_settings->showPointer();
    const auto deliver = [this, toClipboard, alive = std::weak_ptr<bool>(m_alive)](std::vector<ui::Bitmap> images) {
        if (!alive.expired())
            deliverImages(std::move(images), {}, toClipboard);
    };
    if (!grabber) {
        deliverImages({}, {}, toClipboard);
        return;
    }
    wm::WindowManager *manager = m_shell->windowManager();
    if (target.desktop) {
        // The desktop alone, without the windows over it.
        wm::Window *desktop = manager->windowById(target.desktop);
        std::shared_ptr<compositor::CaptureSource> source;
        if (m_services.desktop && !target.outputs.empty())
            source = m_services.desktop(desktop, target.outputs.front());
        if (source)
            grabber->grabSource(std::move(source), cursor, deliver);
        else
            grabber->grabWindow(desktop, cursor, deliver);
        return;
    }
    if (target.window) {
        grabber->grabWindow(manager->windowById(target.window), cursor, deliver);
        return;
    }
    if (target.dock) {
        const std::optional<CaptureDock> panel = dock();
        grabber->grabWindow(panel ? panel->window : nullptr, cursor, deliver);
        return;
    }

    std::vector<base::Rect> geometries;
    for (const std::string &name : target.outputs) {
        const Shell::Output *output = nullptr;
        for (const Shell::Output &candidate : m_shell->outputs()) {
            if (candidate.name == name)
                output = &candidate;
        }
        geometries.push_back(output ? output->bounds : base::Rect());
    }
    const base::Rect region = target.region;
    grabber->grabOutputs(target.outputs, cursor,
                         [this, geometries, region, toClipboard,
                          alive = std::weak_ptr<bool>(m_alive)](std::vector<ui::Bitmap> images) {
                             if (alive.expired())
                                 return;
                             if (region.isEmpty() || images.empty()) {
                                 deliverImages(std::move(images), geometries, toClipboard);
                                 return;
                             }
                             // The portion, at the screen's own pixel density.
                             const ui::Bitmap &image = images.front();
                             const base::Rect geometry = geometries.front();
                             if (image.isNull() || geometry.isEmpty()) {
                                 deliverImages({}, {}, toClipboard);
                                 return;
                             }
                             const double scale = double(image.width()) / geometry.width();
                             const base::Rect local = region.translated(-geometry.x(), -geometry.y());
                             const base::Rect pixels = base::RectF(local.x() * scale, local.y() * scale,
                                                                   local.width() * scale, local.height() * scale)
                                                               .toEnclosingRect()
                                                               .intersected(image.rect());
                             deliverImages({image.copy(pixels)}, {region}, toClipboard);
                         });
}

void ScreenCapture::deliverImages(std::vector<ui::Bitmap> images, std::vector<base::Rect> geometries,
                                  bool toClipboard)
{
    std::vector<ui::Bitmap> pictures;
    std::vector<base::Rect> placed;
    for (size_t i = 0; i < images.size(); ++i) {
        if (images[i].isNull())
            continue;
        pictures.push_back(std::move(images[i]));
        placed.push_back(i < geometries.size() ? geometries[i] : base::Rect());
    }
    if (pictures.empty()) {
        m_notifier->notifyFailed(tr("Screenshot Failed"), tr("The screen could not be captured."));
        return;
    }

    m_notifier->playShutter();

    if (toClipboard || m_settings->imageDestination() == CaptureSettings::kClipboard) {
        const bool placedAll = std::none_of(placed.begin(), placed.end(),
                                            [](const base::Rect &rect) { return rect.isEmpty(); });
        const ui::Bitmap image = pictures.size() > 1 && placedAll ? combine(pictures, placed) : pictures.front();
        if (m_clipboard->setImage(image))
            m_notifier->notifyCopied(image);
        else
            m_notifier->notifyFailed(tr("Screenshot Failed"), tr("The screenshot could not be copied."));
        return;
    }
    saveImages(std::move(pictures));
}

void ScreenCapture::saveImages(std::vector<ui::Bitmap> pictures)
{
    const std::string folder = m_settings->imageFolder();
    const std::string stem = "Screenshot " + timestamp();
    // PNG compression of a whole screen takes long enough to show.
    m_jobs.push_back(base::runInThread(
            [folder, stem, pictures = std::move(pictures), names = m_saving] {
                std::vector<std::string> saved;
                std::vector<std::string> taken;
                std::error_code error;
                std::filesystem::create_directories(folder, error);
                for (const ui::Bitmap &picture : pictures) {
                    const std::string path = uniqueName(*names, folder, stem, "png");
                    taken.push_back(path);
                    const std::optional<std::string> png = picture.encodePng();
                    if (png && base::file::writeAtomically(path, *png))
                        saved.push_back(path);
                }
                const std::lock_guard lock(names->mutex);
                for (const std::string &path : taken)
                    names->taken.erase(path);
                return saved;
            },
            [this](std::vector<std::string> saved) {
                std::erase_if(m_jobs, [](const base::Job &job) { return !job.isPending(); });
                if (onSaved)
                    onSaved(saved);
                if (saved.empty()) {
                    m_notifier->notifyFailed(tr("Screenshot Failed"), tr("The screenshot could not be saved."));
                    return;
                }
                for (const std::string &path : saved)
                    m_notifier->notifySaved(path, false);
            }));
}

void ScreenCapture::startRecording(const Target &target)
{
    if (m_recording->isRunning() || m_recordingStarting)
        return;

    ScreenRecording::Target recording;
    recording.output = target.outputs.empty() ? std::string() : target.outputs.front();
    recording.region = target.region;
    recording.window = target.window;
    recording.cursor = m_settings->recordPointer();
    recording.microphone = m_settings->microphone();
    const std::string folder = m_settings->videoFolder();
    const std::string stem = "Screen Recording " + timestamp();
    m_recordingStarting = true;
    // The folder and the name on a worker; the recorder needs both first.
    m_jobs.push_back(base::runInThread(
            [folder, stem, names = m_saving] {
                std::error_code error;
                std::filesystem::create_directories(folder, error);
                if (!std::filesystem::is_directory(folder, error))
                    return std::string();
                const std::string path = uniqueName(*names, folder, stem, "mp4");
                const std::lock_guard lock(names->mutex);
                names->taken.erase(path);
                return path;
            },
            [this, folder, recording](std::string path) {
                std::erase_if(m_jobs, [](const base::Job &job) { return !job.isPending(); });
                m_recordingStarting = false;
                if (path.empty()) {
                    m_notifier->notifyFailed(tr("Screen Recording Failed"),
                                             format(tr("{folder} cannot be written to."), {{"folder", folder}}));
                    return;
                }
                if (m_recording->isRunning())
                    return;
                m_recording->start(recording, path);
                recordingChanged();
                if (m_service)
                    m_service->emitPropertiesChanged({"recording"});
                notifyChanged();
            }));
}

} // namespace Lingmo::Shell
