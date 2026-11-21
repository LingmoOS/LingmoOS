// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clockwindow.h"
#include <cstdlib>
#include <filesystem>
#include <ui/application.h>
#include <ui/test_input.h>
#include <ui/text_field.h>
#include <ui/theme.h>
namespace {
Lingmo::ui::View* find(Lingmo::ui::View* root, const std::string& name) {
    if (root->name() == name)
        return root;
    for (const auto& child : root->children())
        if (auto* found = find(child.get(), name))
            return found;
    return nullptr;
}
void click(Lingmo::ui::Window& window, const std::string& name) {
    window.grab();
    auto* view = find(window.contentView(), name);
    if (!view)
        std::exit(10);
    Lingmo::ui::PointF point{view->geometry().width / 2, view->geometry().height / 2};
    for (auto* node = view; node; node = node->parent())
        point += node->geometry().topLeft();
    Lingmo::ui::TestInput::click(window, point);
}
} // namespace
int main(int argc, char** argv) {
    if (argc != 2)
        return 1;
    std::filesystem::create_directories(argv[1]);
    Lingmo::ui::Application app(argc, argv, Lingmo::ui::Application::Platform::Headless);
    Lingmo::ui::TestInput::setGlassAvailable(true);
    Lingmo::ui::Theme::setCurrent(Lingmo::ui::Theme::light());
    Clock::ClockWindow window(std::string(argv[1]) + "/clock.db");
    window.window().setScale(1.25);
    window.show();
    auto& w = window.window();
    auto image = w.grab();
    if (!Lingmo::ui::TestInput::backgroundGlass(w) || image.pixel(10, 100).a >= 255 || image.pixel(400, 100).a != 255)
        return 4;
    if (!image.savePng(std::string(argv[1]) + "/world-clock-preview.png"))
        return 2;
    for (const auto& page : {std::pair{"alarms", "alarms-preview.png"}, std::pair{"stopwatch", "stopwatch-preview.png"},
                             std::pair{"timer", "timer-preview.png"}}) {
        click(w, std::string("sidebar:") + page.first);
        w.grab();
        if (!w.grab().savePng(std::string(argv[1]) + "/" + page.second))
            return 3;
    }
    Lingmo::ui::TestInput::setGlassAvailable(false);
    Clock::ClockWindow fallbackWindow(std::string(argv[1]) + "/fallback.db");
    fallbackWindow.window().setScale(1.25);
    fallbackWindow.show();
    const auto fallback = fallbackWindow.window().grab();
    if (Lingmo::ui::TestInput::backgroundGlass(fallbackWindow.window()) || fallback.pixel(10, 100).a != 255)
        return 7;
    auto* minutes = dynamic_cast<Lingmo::ui::TextField*>(find(w.contentView(), "timer-minutes"));
    auto* hours = find(w.contentView(), "timer-hours");
    auto* seconds = find(w.contentView(), "timer-seconds");
    auto* addTimer = find(w.contentView(), "add-timer");
    if (!hours || !minutes || !seconds || !addTimer || hours->parent() != minutes->parent() ||
        minutes->parent() != seconds->parent() || seconds->parent() != addTimer->parent() ||
        hours->geometry().x >= minutes->geometry().x || minutes->geometry().x >= seconds->geometry().x ||
        seconds->geometry().x >= addTimer->geometry().x || std::abs(hours->geometry().y - minutes->geometry().y) > 1)
        return 5;
    minutes->setText("1");
    click(w, "add-timer");
    if (!w.grab().savePng(std::string(argv[1]) + "/timer-running-preview.png"))
        return 6;
    return 0;
}
