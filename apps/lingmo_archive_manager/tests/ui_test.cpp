// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "archivewindow.h"
#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/label.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <poll.h>
using namespace ArchiveManager;
using namespace Lingmo::ui;
void require(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
void wait(ArchiveWindow& window) {
    auto& loop = Application::instance()->loop(); const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (window.busy() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 5); loop.dispatch(); }
    require(!window.busy(), "background operation timed out");
}
View* find(View* view, const std::string& name) {
    if (view->name() == name) return view;
    for (const auto& child : view->children()) if (auto* found = find(child.get(), name)) return found;
    return nullptr;
}
int main(int argc, char** argv) {
    std::string path = "/tmp/lingmo-archive-ui-XXXXXX"; require(::mkdtemp(path.data()), "temporary folder");
    struct Temp { std::filesystem::path path; ~Temp() { std::filesystem::remove_all(path); } } temp{path};
    std::filesystem::create_directory(path + "/documents"); std::ofstream(path + "/documents/readme.txt") << "Hello archive UI";
    std::filesystem::create_directory(path + "/output");
    Application app(argc, argv, Application::Platform::Headless);
    ArchiveWindow window; window.show(); auto image = window.window().grab();
    auto* title = find(window.window().contentView(), "window-title"); require(title, "title");
    require(title->geometry().x + title->geometry().width / 2 == window.window().size().width / 2, "centered title");
    require(find(window.window().contentView(), "local-menu")->isVisible(), "menu fallback");
    window.newArchive(); window.addSources({path + "/documents"}); window.createAt(path + "/sample.zip"); wait(window);
    require(window.listing().entries.size() == 2, "created archive reopened");
    require(window.page() == ArchiveWindow::Page::Browse, "browse after create");
    window.navigate("documents"); require(window.directory() == "documents", "navigation");
    window.select("documents/readme.txt"); window.extractTo(path + "/output", true); wait(window);
    require(std::filesystem::exists(window.lastOutput() + "/documents/readme.txt"), "selected extraction");
    window.verifyArchive(); wait(window);
    window.window().grab(); auto* status = dynamic_cast<Label*>(find(window.window().contentView(), "status"));
    require(status && status->text().find("Integrity check passed") != std::string::npos, "verify status");
    for (bool dark : {false, true}) {
        Theme::setCurrent(dark ? Theme::dark() : Theme::light()); TestInput::setGlassAvailable(true);
        window.window().requestLayout(); image = window.window().grab();
        require(TestInput::backgroundGlass(window.window()).has_value(), "glass requested");
        require(image.pixel(8, 300).a < 255 && image.pixel(250, 300).a == 255, "left glass right solid");
        TestInput::setGlassAvailable(false); window.window().requestLayout(); image = window.window().grab();
        require(!TestInput::backgroundGlass(window.window()) && image.pixel(8, 300).a == 255, "solid fallback");
    }
    window.openArchive(path + "/missing.zip"); wait(window); require(window.listing().output.empty(), "failed open clears previous list");
    std::cout << "Archive SDK UI checks passed\n";
}
