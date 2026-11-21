// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/wallpaper/wallpaper.h"

#include <algorithm>
#include <cmath>
#include <optional>

#include "base/settings.h"
#include "base/strings.h"
#include "base/url.h"
#include "compositor/capture_source.h"
#include "compositor/compositor.h"
#include "core/preferences.h"
#include "lingmo/appearance/appearance.h"
#include "scene/node.h"
#include "scene/wallpaper_node.h"
#include "wm/overview/overview_controller.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include <string_view>
namespace scene = Lingmo::Scene;

namespace Lingmo::Shell {

static const char kDefaultWallpaper[] = "/usr/share/backgrounds/lingmoos/default.jpg";
static const char kDefaultBackgroundColor[] = "#2B8ADA";
// What a desktop shows while it has nothing else: the desktop window's
// colour.
static constexpr render::Color kBlack {0, 0, 0, 1};
// Samples kept for wallpapers switched back to.
static constexpr size_t kMaxSamples = 8;

// #RGB, #RRGGBB or #AARRGGBB (alpha first); invalid (none) otherwise.
static std::optional<ui::Color> parseColor(std::string_view text)
{
    if (text.empty() || text.front() != '#')
        return std::nullopt;
    text.remove_prefix(1);
    const std::optional<uint64_t> value = base::strings::toNumber<uint64_t>(text, 16);
    if (!value || text.find_first_of("+-") != std::string_view::npos)
        return std::nullopt;
    switch (text.size()) {
    case 3:
        return ui::Color {uint8_t(((*value >> 8) & 0xf) * 17), uint8_t(((*value >> 4) & 0xf) * 17),
                          uint8_t((*value & 0xf) * 17), 255};
    case 6:
        return ui::Color::rgb(uint32_t(*value));
    case 8:
        return ui::Color::fromArgb(uint32_t(*value));
    default:
        return std::nullopt;
    }
}

// A translucent colour over the desktop's black window colour, since the
// desktop is opaque.
static render::Color opaqueColor(const ui::Color &color)
{
    const float alpha = color.a / 255.0f;
    return {color.r / 255.0f * alpha, color.g / 255.0f * alpha, color.b / 255.0f * alpha, 1};
}

static std::string localFile(const std::string &setting)
{
    return base::url::toLocalFile(setting).value_or(setting);
}

// The desktop as window capture picks it: this output's wallpaper and the
// desktop window over it.
class Wallpaper::CaptureNode final : public scene::Node
{
public:
    CaptureNode(const scene::WallpaperNode *wallpaper, wm::WindowManager *manager)
        : Node("desktop-capture")
        , m_wallpaper(wallpaper)
        , m_manager(manager)
    {
    }

    void setDesktop(uint64_t window) { m_desktop = window; }

    void render(render::Renderer &renderer) const override
    {
        m_wallpaper->render(renderer);
        const wm::Window *desktop = m_manager && m_desktop ? m_manager->windowById(m_desktop) : nullptr;
        if (desktop && desktop->node())
            desktop->node()->render(renderer);
    }

    bool acceptsInput(const base::PointF &) const override { return false; }

private:
    const scene::WallpaperNode *m_wallpaper;
    wm::WindowManager *m_manager;
    uint64_t m_desktop = 0;
};

struct Wallpaper::Screen {
    Shell::Output output;
    base::Size device;
    scene::WallpaperNode *node = nullptr;
    // When there is no scene to put it in.
    std::unique_ptr<scene::WallpaperNode> ownedNode;
    std::unique_ptr<CaptureNode> captureNode;
    std::shared_ptr<compositor::CaptureSource> captureSource;

    // The picture loading, and the one shown.
    std::string pendingPath;
    base::Size pendingSize;
    std::shared_ptr<void> request;
    bool pendingDone = false;
    ui::Bitmap pendingImage;
    base::Job sampling;
    std::string shownPath;
    base::Size shownSize;
    bool showingImage = false;
    bool showingColor = false;
};

Wallpaper::Wallpaper(Shell *shell, std::string cacheDirectory, std::string servicesSettings)
    : m_shell(shell)
    , m_cache(std::move(cacheDirectory))
    , m_servicesSettings(servicesSettings.empty() ? preferencesFile("services") : std::move(servicesSettings))
{
    m_outputsChanged = m_shell->outputsChanged.connect([this] { updateOutputs(); });
    updateOutputs();
    if (m_shell->bus()) {
        m_ownAppearance = std::make_unique<appearance::Appearance>();
        followAppearance(m_ownAppearance.get());
    }
}

BarBackdrop::Sample Wallpaper::sampleBehind(const ui::RectF& area) const
{
    for(const auto& screen:m_screens) {
        const auto bounds=screen->output.bounds;
        const double left=std::max(area.x,double(bounds.x()));
        const double top=std::max(area.y,double(bounds.y()));
        const double right=std::min(area.right(),double(bounds.x()+bounds.width()));
        const double bottom=std::min(area.bottom(),double(bounds.y()+bounds.height()));
        if(right<=left || bottom<=top)continue;
        BarBackdrop::Sample sample;
        if(screen->showingImage && !screen->node->picture().isNull()) {
            const auto image=screen->node->picture().converted(ui::Bitmap::Format::Argb32Premultiplied);
            const double sx=double(image.width())/std::max(1,bounds.width());
            const double sy=double(image.height())/std::max(1,bounds.height());
            // A bounded grid avoids scanning a full wallpaper on every layout.
            for(int row=0;row<4;++row)for(int col=0;col<8;++col){
                const int x=std::clamp(int((left-bounds.x()+(col+.5)*(right-left)/8)*sx),0,image.width()-1);
                const int y=std::clamp(int((top-bounds.y()+(row+.5)*(bottom-top)/4)*sy),0,image.height()-1);
                const auto pixel=reinterpret_cast<const uint32_t*>(image.constScanLine(y))[x];
                sample.red+=(pixel>>16)&255;sample.green+=(pixel>>8)&255;sample.blue+=pixel&255;
            }
            sample.red/=32;sample.green/=32;sample.blue/=32;sample.valid=true;
        } else if(screen->showingColor)sample=BarBackdrop::sampleColor(m_background.color);
        if(sample.valid && screen->output.primary && m_style.gradientVisible){
            const auto color=m_style.gradientColor;
            const double alpha=color.a/255.0*std::clamp(1.0-(top+bottom-2*bounds.y())/(2*BarBackdrop::kGradientHeight),0.0,1.0);
            sample.red=sample.red*(1-alpha)+color.r*alpha;
            sample.green=sample.green*(1-alpha)+color.g*alpha;
            sample.blue=sample.blue*(1-alpha)+color.b*alpha;
        }
        return sample;
    }
    return {};
}

Wallpaper::~Wallpaper()
{
    m_appearanceChanged.reset();
    m_outputsChanged.reset();
    if (wm::WindowManager *manager = m_shell->windowManager())
        manager->overview()->setWallpaperNodes({});
    for (const std::unique_ptr<Screen> &screen : m_screens) {
        if (screen->captureSource) {
            screen->captureSource->node = nullptr;
            screen->captureSource->active = false;
        }
        if (!screen->ownedNode && screen->node->parent())
            screen->node->parent()->removeChild(screen->node);
    }
    m_screens.clear();
}

void Wallpaper::followAppearance(appearance::Appearance *appearance)
{
    m_appearanceChanged.reset();
    m_appearance = appearance;
    if (!m_appearance)
        return;
    m_appearanceChanged = m_appearance->changed.connect([this] { reloadAppearance(); });
    reloadAppearance();
}

void Wallpaper::reloadAppearance()
{
    // Until the service answers, its settings file names what it will; one
    // that goes away leaves the desktop as it is.
    if (!m_appearance->isAvailable()) {
        if (!m_hasBackground)
            readServicesSettings();
        return;
    }
    m_settingsJob.cancel();
    Background background;
    background.type = m_appearance->backgroundType();
    background.path = localFile(m_appearance->wallpaper());
    background.color = parseColor(m_appearance->backgroundColor()).value_or(ui::Color {0, 0, 0, 255});
    setBackground(background);
}

void Wallpaper::readServicesSettings()
{
    if (m_settingsJob.isPending())
        return;
    m_settingsJob = base::ThreadPool::global().run(
            [path = m_servicesSettings] {
                const base::Settings settings(path);
                Background background;
                background.type = int(settings.intValue("Appearance/BackgroundType", 0));
                background.path = localFile(settings.stringValue("Appearance/Wallpaper", kDefaultWallpaper));
                background.color =
                        parseColor(settings.stringValue("Appearance/BackgroundColor", kDefaultBackgroundColor))
                                .value_or(ui::Color {0, 0, 0, 255});
                return background;
            },
            [this](Background background) {
                // A background given meanwhile, by the service or directly, wins.
                if (!m_hasBackground)
                    setBackground(background);
            });
}

void Wallpaper::setBackground(const Background &background)
{
    if (m_hasBackground && background == m_background)
        return;
    m_background = background;
    m_hasBackground = true;
    for (const std::unique_ptr<Screen> &screen : m_screens)
        reload(*screen);
    backgroundChanged();
}

void Wallpaper::updateOutputs()
{
    const std::vector<Shell::Output> &outputs = m_shell->outputs();
    std::erase_if(m_screens, [&](const std::unique_ptr<Screen> &screen) {
        const bool gone = std::none_of(outputs.begin(), outputs.end(), [&](const Shell::Output &output) {
            return output.name == screen->output.name;
        });
        if (!gone)
            return false;
        if (screen->captureSource) {
            screen->captureSource->node = nullptr;
            screen->captureSource->active = false;
            screen->captureSource->changed();
        }
        if (!screen->ownedNode && screen->node->parent())
            screen->node->parent()->removeChild(screen->node);
        return true;
    });

    scene::Node *container = m_shell->compositor() ? m_shell->compositor()->surfaceContainer() : nullptr;
    std::vector<Screen *> changed;
    for (const Shell::Output &output : outputs) {
        auto it = std::find_if(m_screens.begin(), m_screens.end(), [&](const std::unique_ptr<Screen> &screen) {
            return screen->output.name == output.name;
        });
        Screen *screen = it != m_screens.end() ? it->get() : nullptr;
        const bool fresh = !screen;
        if (fresh) {
            screen = m_screens.emplace_back(std::make_unique<Screen>()).get();
            auto node = std::make_unique<scene::WallpaperNode>();
            if (container) {
                screen->node = static_cast<scene::WallpaperNode *>(container->addChild(std::move(node)));
                // Under everything the window manager stacks: the desktop
                // layer's own bottom.
                container->lower(screen->node);
            } else {
                screen->ownedNode = std::move(node);
                screen->node = screen->ownedNode.get();
            }
            screen->node->setColor(kBlack);
            screen->captureNode = std::make_unique<CaptureNode>(screen->node, m_shell->windowManager());
        }
        const base::Size device(int(std::lround(output.bounds.width() * output.scale)),
                                int(std::lround(output.bounds.height() * output.scale)));
        const bool reloads = fresh || device != screen->device || output.primary != screen->output.primary;
        screen->output = output;
        screen->device = device;
        screen->node->setGeometry(output.bounds);
        screen->captureNode->setGeometry(output.bounds);
        if (reloads)
            changed.push_back(screen);
    }
    for (Screen *screen : changed)
        reload(*screen);
    updateGradient();

    if (wm::WindowManager *manager = m_shell->windowManager())
        manager->overview()->setWallpaperNodes(nodes());
}

void Wallpaper::reload(Screen &screen)
{
    screen.request.reset();
    screen.sampling.cancel();
    screen.pendingDone = false;
    screen.pendingImage = {};
    if (!m_hasBackground)
        return;

    const Background &background = m_background;
    if (background.type != 0) {
        screen.pendingPath.clear();
        screen.pendingSize = {};
        screen.shownPath.clear();
        screen.shownSize = {};
        screen.showingImage = false;
        screen.showingColor = true;
        screen.node->setPicture({});
        screen.node->setColor(opaqueColor(background.color));
        if (screen.output.primary)
            publish(true, BarBackdrop::styleForSample(BarBackdrop::sampleColor(background.color)));
        updateGradient();
        return;
    }
    if (screen.device.isEmpty())
        return;

    screen.pendingPath = background.path;
    screen.pendingSize = screen.device;
    if (screen.pendingPath != screen.shownPath || screen.pendingSize != screen.shownSize) {
        // A decode in memory comes back at once.
        screen.request = m_cache.load(screen.pendingPath, screen.pendingSize,
                                      [this, &screen](const ui::Bitmap &image) { decoded(screen, image); });
        return;
    }
    tryCommit(screen);
}

std::string Wallpaper::sampleKey(const std::string &path, const base::Size &size)
{
    return path + '\n' + std::to_string(size.width()) + 'x' + std::to_string(size.height());
}

void Wallpaper::decoded(Screen &screen, const ui::Bitmap &image)
{
    screen.pendingDone = true;
    screen.pendingImage = image;
    tryCommit(screen);
}

void Wallpaper::tryCommit(Screen &screen)
{
    const Background &background = m_background;
    if (!m_hasBackground || background.type != 0)
        return;

    const bool swap = screen.pendingPath != screen.shownPath || screen.pendingSize != screen.shownSize;
    if (swap && !screen.pendingDone)
        return;

    // A picture that cannot be read replaces nothing: what is up stays up,
    // and a screen with nothing up yet shows the colour instead.
    if (swap && screen.pendingImage.isNull()) {
        if (!screen.showingImage) {
            screen.showingColor = true;
            screen.node->setColor(opaqueColor(background.color));
            updateGradient();
        }
        return;
    }

    // On the primary output the bar must know how to read against it first.
    if (screen.output.primary) {
        const std::string key = sampleKey(screen.pendingPath, screen.pendingSize);
        if (!m_samples.count(key)) {
            if (screen.sampling.isPending())
                return;
            const ui::Bitmap image = swap ? screen.pendingImage : screen.node->picture();
            const int barHeight = int(std::lround(double(BarBackdrop::kBarHeight) * screen.device.height()
                                                  / std::max(1, screen.output.bounds.height())));
            screen.sampling = base::ThreadPool::global().run(
                    [image, barHeight] { return BarBackdrop::sampleImage(image, barHeight); },
                    [this, &screen, key](BarBackdrop::Sample sample) {
                        if (m_samples.size() >= kMaxSamples)
                            m_samples.clear();
                        m_samples[key] = sample;
                        tryCommit(screen);
                    });
            return;
        }
    }

    if (swap) {
        screen.node->setPicture(screen.pendingImage);
        screen.shownPath = screen.pendingPath;
        screen.shownSize = screen.pendingSize;
        screen.pendingImage = {};
    }
    screen.showingImage = true;
    screen.showingColor = false;
    if (screen.output.primary)
        present(screen);
    updateGradient();
}

void Wallpaper::present(const Screen &screen)
{
    const auto it = m_samples.find(sampleKey(screen.shownPath, screen.shownSize));
    if (it == m_samples.end())
        return;
    publish(it->second.valid, BarBackdrop::styleForSample(it->second));
}

void Wallpaper::publish(bool judged, const BarBackdrop::Style &style)
{
    // A new image can keep the same strip average but change local contrast.
    // Notify local icon samplers even when the aggregate verdict is unchanged.
    m_judged = judged;
    m_style = style;
    updateGradient();
    judgementChanged();
}

void Wallpaper::updateGradient()
{
    for (const std::unique_ptr<Screen> &screen : m_screens) {
        const bool shows = screen->output.primary && m_judged && m_style.gradientVisible
                && (screen->showingImage || screen->showingColor);
        screen->node->setGradient(shows ? std::optional<scene::WallpaperNode::Gradient>(
                                                  {m_style.gradientColor, BarBackdrop::kGradientHeight})
                                        : std::nullopt);
    }
}

ui::Bitmap Wallpaper::primaryPicture() const
{
    for (const std::unique_ptr<Screen> &screen : m_screens) {
        if (screen->output.primary)
            return screen->showingImage ? screen->node->picture() : ui::Bitmap();
    }
    return {};
}

bool Wallpaper::isReady() const
{
    if (!m_hasBackground || m_screens.empty())
        return false;
    if (m_background.type != 0)
        return true;
    return std::all_of(m_screens.begin(), m_screens.end(), [](const std::unique_ptr<Screen> &screen) {
        const bool committed = screen->pendingPath == screen->shownPath && screen->pendingSize == screen->shownSize
                && screen->showingImage;
        const bool failed = screen->pendingDone && screen->pendingImage.isNull();
        return committed || failed || screen->showingColor;
    });
}

scene::WallpaperNode *Wallpaper::node(const std::string &output) const
{
    for (const std::unique_ptr<Screen> &screen : m_screens) {
        if (screen->output.name == output)
            return screen->node;
    }
    return nullptr;
}

std::vector<const scene::Node *> Wallpaper::nodes() const
{
    std::vector<const scene::Node *> result;
    for (const std::unique_ptr<Screen> &screen : m_screens)
        result.push_back(screen->node);
    return result;
}

std::shared_ptr<compositor::CaptureSource> Wallpaper::desktopCaptureSource(wm::Window *desktop,
                                                                           const std::string &output)
{
    const auto it = std::find_if(m_screens.begin(), m_screens.end(), [&](const std::unique_ptr<Screen> &screen) {
        return screen->output.name == output;
    });
    if (it == m_screens.end())
        return nullptr;
    Screen &screen = **it;
    screen.captureNode->setDesktop(desktop ? desktop->id() : 0);
    if (!screen.captureSource) {
        screen.captureSource = std::make_shared<compositor::CaptureSource>();
        screen.captureSource->kind = compositor::CaptureSource::Kind::Window;
        screen.captureSource->node = screen.captureNode.get();
    }
    screen.captureSource->bounds = screen.output.bounds;
    screen.captureSource->scale = screen.output.scale;
    screen.captureSource->outputName = screen.output.name;
    if (const compositor::Compositor *compositor = m_shell->compositor()) {
        // A render context names the wl_output whose composition carries it.
        for (const compositor::Compositor::OutputView &view : compositor->outputViews()) {
            if (view.waylandName == screen.output.name || view.id == screen.output.name
                || (screen.output.name.empty() && view.bounds == screen.output.bounds)) {
                screen.captureSource->outputName = view.waylandName.empty() ? view.id : view.waylandName;
                break;
            }
        }
    }
    return screen.captureSource;
}

} // namespace Lingmo::Shell
