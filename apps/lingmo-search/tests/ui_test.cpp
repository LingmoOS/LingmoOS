// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "searchwindow.h"
#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/test_input.h"
#include "ui/clipboard.h"
#include "ui/keys.h"
#include "ui/theme.h"
#include <cstdlib>
#include <iostream>
#include <poll.h>
using namespace Search;
using namespace Lingmo::ui;
void check(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
void wait(SearchWindow& window) {
    auto& loop = Lingmo::ui::Application::instance()->loop(); auto end = std::chrono::steady_clock::now() + std::chrono::seconds(4);
    while (window.searching() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 10); loop.dispatch(); }
    check(!window.searching(), "query timed out");
}
int main(int argc, char** argv) {
    Lingmo::ui::Application app(argc, argv, Lingmo::ui::Application::Platform::Headless);
    SearchWindow window(false); window.setTestSources({{"editor.desktop", "Editor", "Text", "", "", {}}, {"files.desktop", "Files", "", "", "", {}}}, {});
    window.show(); check(window.window().size().height == 64, "compact initial bar");
    window.setQuery("   "); check(!window.searching() && window.window().size().height == 64, "blank query stays compact");
    TestInput::setKeyboardFocus(window.window(), true);
    TestInput::typeText(window.window(), "Editor"); wait(window);
    check(window.response().results.size() == 1 && window.response().results[0].id == "editor.desktop", "typing queries");
    check(window.window().size().height > 64, "results expand below bar");
    window.setQuery("Editor"); window.setQuery("Files"); wait(window);
    check(window.response().results[0].id == "files.desktop", "new query supersedes stale results");
    window.setCategory(Category::Calculator); window.setQuery("2+3*4"); wait(window);
    check(window.response().results[0].title == "14", "calculator filter");
    window.activateSelected(); check(!window.visible(), "copy closes overlay");
    bool copied = false; Clipboard::requestText([&](std::string value) { copied = value == "14"; }); check(copied, "result copied");
    window.show(); window.setQuery("2+2"); window.hide();
    app.loop().dispatch(); check(window.response().results.empty(), "dismiss clears results");
    window.show(); window.setQuery("1+2"); wait(window);
    for (bool dark : {false, true}) {
        Theme::setCurrent(dark ? Theme::dark() : Theme::light()); TestInput::setGlassAvailable(true);
        window.window().requestLayout(); auto image = window.window().grab();
        check(TestInput::backgroundGlass(window.window()).has_value(), "glass requested");
        check(image.pixel(12, 30).a < 255 && image.pixel(8, 155).a == 255, "glass input solid results");
        TestInput::setGlassAvailable(false); window.window().requestLayout(); image = window.window().grab();
        check(image.pixel(12, 30).a == 255, "fallback opaque");
    }
    TestInput::typeKey(window.window(), key::Escape); check(!window.visible(), "Escape closes");
    app.loop().dispatch(); check(app.windows().empty(), "closed search has no live window");
    for (int i = 0; i < 5; ++i) {
        window.toggle(); check(window.visible(), "toggle opens");
        window.toggle(); app.loop().dispatch(); check(app.windows().empty(), "toggle destroys surface");
    }
    window.show(); window.setCategory(Category::Applications); window.setQuery("e"); wait(window);
    check(window.response().results.size() == 2, "multiple navigation results");
    window.moveSelection(-1); window.moveSelection(1);
    TestInput::configure(window.window(), 0, 0, {.activated = true});
    app.loop().dispatch();
    TestInput::configure(window.window(), 0, 0, {.activated = false});
    app.loop().dispatch(); check(!window.visible(), "losing focus dismisses search");
    std::cout << "Search SDK UI tests passed\n";
}
