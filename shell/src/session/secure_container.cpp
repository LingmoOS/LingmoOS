// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/secure_container.h"

#include "base/event_loop.h"
#include "base/settings.h"
#include "base/strings.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "base/url.h"
#include "core/main_loop.h"
#include "core/preferences.h"
#include "scene/node.h"
#include "ui/bitmap.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string_view>
#include <sys/stat.h>
namespace scene = Lingmo::Scene;

namespace Lingmo::Session {

namespace {

constexpr render::Color kShieldColor {0.0f, 0.0f, 0.0f, 1.0f};

// A lock whose first frame comes this late has no screen showing its start;
// it goes straight to its end instead.
constexpr double kLateStartMs = 250;

// The backdrop and panel fade in over this, while the panel's raise runs the
// whole kCoverDurationMs. Leaving, the panel is gone before the backdrop.
constexpr double kCoverFadeMs = 300;
constexpr double kPanelLiftFadeMs = 200;
constexpr double kPanelRaiseFrom = 0.92;
constexpr double kPanelLiftTo = 1.06;

// Another account's wallpaper fades in over the one shown.
constexpr double kWallpaperFadeMs = 300;

double steadyNow()
{
    return std::chrono::duration<double, std::milli>(
                   std::chrono::steady_clock::now().time_since_epoch())
            .count();
}

// gfx::Tween::EASE_IN_OUT and EASE_OUT, which ash's session state animator
// gives opacity and transform.
double easeInOut(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return t < 0.5 ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t);
}

double easeOut(double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return 1 - (1 - t) * (1 - t);
}

const std::string kDefaultWallpaper = "/usr/share/backgrounds/lingmoos/default.jpg";
const std::string kDefaultBackgroundColor = "#2B8ADA";

// The file a wallpaper setting names: a path, or a file: URL.
std::string wallpaperFile(const std::string &setting)
{
    return base::url::toLocalFile(setting).value_or(setting);
}

// Milliseconds since the epoch; -1 for a file that is not there.
int64_t modifiedMs(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return -1;
    return int64_t(info.st_mtim.tv_sec) * 1000 + info.st_mtim.tv_nsec / 1000000;
}

// #RGB, #RRGGBB, #AARRGGBB and the 12-bit-per-channel forms of a colour
// setting, as premultiplied 0xAARRGGBB; opaque black for anything else.
uint32_t settingColor(std::string_view text)
{
    if (text.empty() || text.front() != '#')
        return 0xff000000;
    text.remove_prefix(1);
    const std::optional<uint64_t> value = base::strings::toNumber<uint64_t>(text, 16);
    if (!value || text.find_first_of("+-") != std::string_view::npos)
        return 0xff000000;
    uint32_t a = 255, r = 0, g = 0, b = 0;
    switch (text.size()) {
    case 3:
        r = ((*value >> 8) & 0xf) * 17;
        g = ((*value >> 4) & 0xf) * 17;
        b = (*value & 0xf) * 17;
        break;
    case 6:
        r = (*value >> 16) & 0xff;
        g = (*value >> 8) & 0xff;
        b = *value & 0xff;
        break;
    case 8:
        a = (*value >> 24) & 0xff;
        r = (*value >> 16) & 0xff;
        g = (*value >> 8) & 0xff;
        b = *value & 0xff;
        break;
    case 9:
        r = ((*value >> 24) & 0xfff) >> 4;
        g = ((*value >> 12) & 0xfff) >> 4;
        b = (*value & 0xfff) >> 4;
        break;
    case 12:
        r = ((*value >> 32) & 0xffff) >> 8;
        g = ((*value >> 16) & 0xffff) >> 8;
        b = (*value & 0xffff) >> 8;
        break;
    default:
        return 0xff000000;
    }
    const auto premultiply = [a](uint32_t channel) { return (channel * a + 127) / 255; };
    return a << 24 | premultiply(r) << 16 | premultiply(g) << 8 | premultiply(b);
}

// On a worker thread: the wallpaper covering `size` at the output's density,
// cropped to its middle.
ui::Bitmap loadWallpaper(const std::string &settingsFile, const base::Size &size, double scale)
{
    const base::Settings settings(settingsFile);
    // Device pixels: the wallpaper is shown sharp, as the desktop shows it.
    const base::Size target(std::max(1, base::roundToInt(size.width() * scale)),
                            std::max(1, base::roundToInt(size.height() * scale)));
    ui::Bitmap image;
    if (settings.intValue("Appearance/BackgroundType", 0) == 0) {
        const std::string path =
                wallpaperFile(settings.stringValue("Appearance/Wallpaper", kDefaultWallpaper));
        image = ui::Bitmap::decodeFile(path, target, ui::Bitmap::Scaling::Cover, true);
        if (image.isNull())
            return {};
        const base::Size covering =
                ui::scaledSize(image.size(), target, ui::Bitmap::Scaling::Cover);
        if (image.size() != covering)
            image = image.scaled(covering);
        image = image.copy(base::Rect((image.width() - target.width()) / 2,
                                      (image.height() - target.height()) / 2, target.width(),
                                      target.height()));
    } else {
        image = ui::Bitmap(target, ui::Bitmap::Format::Argb32Premultiplied);
        image.fill(settingColor(
                settings.stringValue("Appearance/BackgroundColor", kDefaultBackgroundColor)));
    }
    return image.converted(ui::Bitmap::Format::Rgba8888Premultiplied);
}

class WallpaperNode final : public scene::Node
{
public:
    // `fadesIn` over the wallpaper below it, which goes once it is covered.
    WallpaperNode(const SecureContainer *container, const base::Size &size, double scale, bool fadesIn,
                  std::function<void()> onLoaded)
        : Node("lock-wallpaper")
        , m_container(container)
        , m_fadesIn(fadesIn)
        , m_onLoaded(std::move(onLoaded))
        , m_covered(core::mainLoop())
    {
        setGeometry(base::Rect(size));
        const SecureContainer::PictureLoader &loader = container->pictureLoader();
        const std::string picture = container->picture();
        if (loader && !picture.empty()) {
            // The desktop's decode of the same picture at the same size.
            const base::Size target(std::max(1, base::roundToInt(size.width() * scale)),
                                    std::max(1, base::roundToInt(size.height() * scale)));
            m_request = loader(picture, target, [this](const ui::Bitmap &image) {
                m_image = image;
                m_format = render::PixelFormat::Bgra8888;
                loaded();
            });
            return;
        }
        m_loader = base::ThreadPool::global().run(
                [settingsFile = container->settingsFile(), size, scale] {
                    return loadWallpaper(settingsFile, size, scale);
                },
                [this](ui::Bitmap image) {
                    m_image = std::move(image);
                    loaded();
                });
    }

    Content content() const override { return Content::Translucent; }

    bool isLoaded() const { return m_loaded; }

    const ui::Bitmap &image() const { return m_image; }

    void releaseRenderResources() override { m_texture.reset(); }

    void render(render::Renderer &renderer) const override
    {
        Node::render(renderer);
        if (m_fadeStart && fade(renderer) < 1)
            notifyChanged();
    }

protected:
    void paint(render::Renderer &renderer, const base::Rect &bounds) const override
    {
        if (m_image.isNull())
            return;
        if (!m_texture)
            m_texture = renderer.createTexture(m_image.size(),
                                               m_image.constBits(), int(m_image.stride()),
                                               m_format);
        if (m_texture)
            renderer.drawTexture(m_texture.get(), bounds,
                                 m_container->sample(renderer).backdrop * float(fade(renderer)));
    }

private:
    void loaded()
    {
        m_loaded = true;
        // One that cannot be read leaves the wallpaper below it up.
        if (m_fadesIn && !m_image.isNull()) {
            m_fadeStart = steadyNow();
            m_covered.setSingleShot(true);
            m_covered.onTimeout = [this] { removeBelow(); };
            m_covered.start(std::chrono::milliseconds(int(kWallpaperFadeMs)));
        }
        notifyChanged();
        if (m_onLoaded)
            m_onLoaded();
    }

    double fade(const render::Renderer &renderer) const
    {
        if (!m_fadeStart)
            return 1;
        const double time = renderer.frameTime >= 0 ? renderer.frameTime : steadyNow();
        return easeInOut((time - *m_fadeStart) / kWallpaperFadeMs);
    }

    void removeBelow()
    {
        m_fadeStart.reset();
        while (!parent()->children().empty() && parent()->children().front() != this)
            parent()->removeChild(parent()->children().front());
    }

    const SecureContainer *m_container;
    const bool m_fadesIn;
    std::function<void()> m_onLoaded;
    base::Job m_loader;
    std::shared_ptr<void> m_request;
    ui::Bitmap m_image;
    render::PixelFormat m_format = render::PixelFormat::Rgba8888;
    bool m_loaded = false;
    std::optional<double> m_fadeStart;
    base::Timer m_covered;
    mutable std::unique_ptr<render::Texture> m_texture;
};

// Black under the wallpaper, and in its place until it has loaded. It fades in
// only when there is no wallpaper to fade: under one it would darken the fade.
class ShieldNode final : public scene::ColorNode
{
public:
    explicit ShieldNode(const SecureContainer *container)
        : ColorNode(kShieldColor, "shield")
        , m_container(container)
    {
    }

    void render(render::Renderer &renderer) const override
    {
        Node::render(renderer);
        const double time = renderer.frameTime >= 0 ? renderer.frameTime : steadyNow();
        if (isVisible() && !m_container->isSettled(time))
            m_container->requestAnimationFrame();
    }

protected:
    void paint(render::Renderer &renderer, const base::Rect &bounds) const override
    {
        const float backdrop = m_container->sample(renderer).backdrop;
        const bool wallpaper = std::any_of(children().begin(), children().end(), [](auto *child) {
            const auto *node = dynamic_cast<const WallpaperNode *>(child);
            return node && node->isLoaded();
        });
        const float alpha = wallpaper ? (backdrop >= 1 ? 1 : 0) : backdrop;
        if (alpha > 0 && !bounds.isEmpty())
            renderer.fillRect(bounds, {0, 0, 0, alpha});
    }

private:
    const SecureContainer *m_container;
};

} // namespace

SecureContainer::SecureContainer(scene::Node *root)
    : m_settingsFile(preferencesFile("services"))
    , m_root(root)
{
    m_shield = m_root->addChild(std::make_unique<scene::Node>("lock-shield"));
    m_auth = m_root->addChild(std::make_unique<scene::Node>("auth-ui"));
    m_root->setVisible(false);
    refreshWallpaper();
}

SecureContainer::~SecureContainer() = default;

void SecureContainer::setBlocked(bool blocked)
{
    if (m_blocked == blocked)
        return;
    m_blocked = blocked;
    m_root->setVisible(blocked);
    if (blocked)
        refreshWallpaper();
}

void SecureContainer::setCovered(bool covered, bool animate)
{
    m_covered = covered;
    m_animating = animate;
    m_requested = steadyNow();
    m_start.reset();
    requestAnimationFrame();
}

void SecureContainer::finishTransition()
{
    if (!m_animating)
        return;
    m_animating = false;
    requestAnimationFrame();
}

int SecureContainer::transitionDuration() const
{
    return m_covered ? kCoverDurationMs : kUncoverDurationMs;
}

CoverSample SecureContainer::sample(const render::Renderer &renderer) const
{
    return sample(renderer.frameTime, renderer.framePeriod);
}

CoverSample SecureContainer::sample(double frameTime, double framePeriod) const
{
    const double time = frameTime >= 0 ? frameTime : steadyNow();
    const double duration = transitionDuration();
    if (m_animating && !m_start) {
        // Already moving in its first frame, as window animations do.
        m_start = time - kLateStartMs > m_requested ? time - duration : time - framePeriod;
    }
    const bool over = !m_animating || time >= *m_start + duration;
    if (over)
        return m_covered ? CoverSample {} : CoverSample {0, 0, 1};

    const double elapsed = time - *m_start;
    if (m_covered) {
        const double fade = easeInOut(elapsed / kCoverFadeMs);
        const double raise = easeOut(elapsed / kCoverDurationMs);
        return {float(fade), float(fade), kPanelRaiseFrom + (1 - kPanelRaiseFrom) * raise};
    }
    const double lift = easeOut(elapsed / kUncoverDurationMs);
    return {float(1 - easeInOut(elapsed / kUncoverDurationMs)),
            float(1 - easeInOut(elapsed / kPanelLiftFadeMs)), 1 + (kPanelLiftTo - 1) * lift};
}

bool SecureContainer::isSettled(double frameTime) const
{
    if (!m_animating)
        return true;
    return m_start && frameTime >= *m_start + transitionDuration();
}

void SecureContainer::requestAnimationFrame() const
{
    for (const auto *shield : m_shieldRects)
        shield->notifyChanged();
    for (const auto *panel : m_auth->children())
        panel->notifyChanged();
}

void SecureContainer::setOutputs(const std::vector<SecureOutput> &outputs)
{
    if (m_outputs == outputs)
        return;
    m_outputs = outputs;
    rebuildShield();
}

bool SecureContainer::isWallpaperReady() const
{
    if (m_outputs.empty() || m_shieldRects.size() != m_outputs.size())
        return false;
    return std::all_of(m_shieldRects.begin(), m_shieldRects.end(), [](const scene::ColorNode *shield) {
        return std::all_of(shield->children().begin(), shield->children().end(),
                           [](const scene::Node *child) {
                               const auto *wallpaper = dynamic_cast<const WallpaperNode *>(child);
                               return !wallpaper || wallpaper->isLoaded();
                           });
    });
}

ui::Bitmap SecureContainer::wallpaperImage(const std::string &output) const
{
    for (size_t i = 0; i < m_outputs.size() && i < m_shieldRects.size(); ++i) {
        if (m_outputs[i].name != output)
            continue;
        const auto &children = m_shieldRects[i]->children();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            const auto *wallpaper = dynamic_cast<const WallpaperNode *>(*it);
            if (wallpaper && wallpaper->isLoaded() && !wallpaper->image().isNull())
                return wallpaper->image();
        }
    }
    return {};
}

void SecureContainer::setSettingsFile(const std::string &path, bool crossfade)
{
    if (path == m_settingsFile)
        return;
    m_settingsFile = path;
    refreshWallpaper(crossfade);
}

void SecureContainer::refreshWallpaper(bool crossfade)
{
    const base::Settings settings(m_settingsFile);
    const std::string path = settings.stringValue("Appearance/Wallpaper", kDefaultWallpaper);
    const std::string file = wallpaperFile(path);
    if (base::EventLoop *loop = core::mainLoop()) {
        // The settings file and the wallpaper are replaced by rename; their
        // directories are what is watched, so the new file is followed too.
        const auto follow = [this, loop](const std::string &path, std::string &watched,
                                         base::FileWatch &watch) {
            if (path == watched && watch.isActive())
                return;
            watched = path;
            watch = base::FileWatch::file(path, [this] {
                if (!m_blocked)
                    refreshWallpaper();
            }, loop);
        };
        follow(settings.path(), m_watchedSettings, m_settingsWatch);
        follow(file, m_watchedWallpaper, m_wallpaperWatch);
    }
    m_picture = settings.intValue("Appearance/BackgroundType", 0) == 0 ? file : std::string();
    const std::string key = path + '|' + settings.stringValue("Appearance/BackgroundType", "0")
            + '|' + settings.stringValue("Appearance/BackgroundColor", kDefaultBackgroundColor)
            + '|' + std::to_string(modifiedMs(file));
    if (key == m_wallpaperKey)
        return;
    m_wallpaperKey = key;
    rebuildShield(crossfade);
}

void SecureContainer::rebuildShield(bool crossfade)
{
    while (m_shieldRects.size() > m_outputs.size()) {
        m_shield->removeChild(m_shieldRects.back());
        m_shieldRects.pop_back();
    }
    while (m_shieldRects.size() < m_outputs.size()) {
        auto node = std::make_unique<ShieldNode>(this);
        m_shieldRects.push_back(
                static_cast<scene::ColorNode *>(m_shield->addChild(std::move(node))));
    }
    for (size_t i = 0; i < m_outputs.size(); ++i) {
        auto *shield = m_shieldRects[i];
        shield->setGeometry(m_outputs[i].bounds);
        // The wallpapers up stay under the new one until it covers them.
        bool shown = false;
        for (scene::Node *child : std::vector<scene::Node *>(shield->children())) {
            const auto *wallpaper = dynamic_cast<const WallpaperNode *>(child);
            if (crossfade && wallpaper && wallpaper->isLoaded())
                shown = true;
            else
                shield->removeChild(child);
        }
        if (core::mainLoop())
            shield->addChild(
                    std::make_unique<WallpaperNode>(this, m_outputs[i].bounds.size(),
                                                    m_outputs[i].scale, shown, [this] { wallpaperLoaded(); }));
    }
}

} // namespace Lingmo::Session
