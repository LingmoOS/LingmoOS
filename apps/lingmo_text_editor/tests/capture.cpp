// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Renders the editor's reference states headless for the reference script:
// capture SAMPLES OUTDIR SCALE light|dark [STATE...].
// The session, drafts and settings live in a scratch folder.

#include "application.h"
#include "editorpage.h"
#include "editorsettings.h"
#include "editorview.h"
#include "editorwindow.h"
#include "pagebars.h"
#include "session.h"

#include "base/event_loop.h"
#include "base/file.h"
#include "ui/application.h"
#include "ui/keys.h"
#include "ui/menu.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <poll.h>
#include <set>
#include <thread>

using namespace Lingmo::ui;
namespace fs = std::filesystem;

static std::string s_samples;
static std::string s_out;
static std::string s_suffix;
static double s_scale = 1;
static std::set<std::string> s_only;

static void spin(int ms)
{
    Lingmo::base::EventLoop &loop = Lingmo::ui::Application::instance()->loop();
    const auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
    while (std::chrono::steady_clock::now() < end) {
        pollfd descriptor{loop.fd(), POLLIN, 0};
        ::poll(&descriptor, 1, 10);
        loop.dispatch();
    }
}

static bool wanted(const std::string &state)
{
    return s_only.empty() || s_only.count(state);
}

static void save(Window &window, const std::string &state)
{
    spin(50);
    const std::string path = s_out + "/" + state + "-" + s_suffix + ".png";
    window.grab().savePng(path);
    // Popups open over the window: each saved beside it.
    std::printf("%s\n", path.c_str());
}

static void savePopups(Window &parent, const std::string &state)
{
    int index = 0;
    for (Window *window : Lingmo::ui::Application::instance()->windows()) {
        if (window == &parent || window->isHidden())
            continue;
        const std::string path = s_out + "/" + state + "-" + s_suffix + ".popup" + std::to_string(index++) + ".png";
        window->grab().savePng(path);
        std::printf("%s\n", path.c_str());
    }
}

static void configure(Window &window, SizeF size)
{
    window.setScale(s_scale);
    TestInput::configure(window, int(size.width), int(size.height), {true});
    TestInput::setKeyboardFocus(window, true);
    spin(30);
}

static void writeSettings(const std::string &path, const std::vector<std::pair<std::string, std::string>> &overrides)
{
    std::string text = "[Editor]\nautoIndent=true\nfindCaseSensitive=false\nfindRegex=false\nfindWholeWords=false\n"
                       "fontFamily=\nfontSize=0\nhighlightCurrentLine=true\ninsertSpaces=true\nlastFolder="
            + s_samples + "\nlineNumbers=true\nmatchBrackets=true\nrestoreSession=true\nshowWhitespace=false\n"
                          "tabWidth=4\nwordWrap=true\n";
    for (const auto &[key, value] : overrides) {
        const size_t at = text.find(key + "=");
        const size_t end = text.find('\n', at);
        text.replace(at, end - at, key + "=" + value);
    }
    Lingmo::base::file::writeAtomically(path, text);
}

struct Run
{
    explicit Run(const std::string &scratch, const std::vector<std::pair<std::string, std::string>> &overrides = {})
    {
        fs::remove_all(scratch + "/data");
        Session::setDataDirectory(scratch + "/data");
        writeSettings(scratch + "/text-editor.conf", overrides);
        settings = std::make_unique<EditorSettings>(scratch + "/text-editor.conf");
        application = std::make_unique<EditorApplication>(*settings);
    }

    ~Run()
    {
        application.reset();
        spin(30);
    }

    EditorWindow &start(const std::vector<std::string> &files)
    {
        std::vector<std::string> paths;
        for (const std::string &file : files)
            paths.push_back(s_samples + "/" + file);
        application->start(paths, s_samples, std::nullopt);
        EditorWindow &window = *application->windows().back();
        configure(window.window(), {900, 640});
        for (EditorPage *page : window.pages())
            page->document().waitForLoaded();
        spin(100);
        return window;
    }

    std::unique_ptr<EditorSettings> settings;
    std::unique_ptr<EditorApplication> application;
};

int main(int argc, char **argv)
{
    if (argc < 5) {
        std::fprintf(stderr, "usage: %s SAMPLES OUTDIR SCALE light|dark [STATE...]\n", argv[0]);
        return 1;
    }
    s_samples = argv[1];
    s_out = argv[2];
    s_scale = std::stod(argv[3]);
    const bool dark = std::string(argv[4]) == "dark";
    s_suffix = std::string(dark ? "dark" : "light") + "@" + (s_scale == 1 ? "1.0" : argv[3]);
    for (int i = 5; i < argc; ++i)
        s_only.insert(argv[i]);

    Lingmo::ui::Application app(argc, argv, Lingmo::ui::Application::Platform::Headless);
    app.setApplicationId("lingmo-text-editor");
    Theme::setCurrent(dark ? Theme::dark() : Theme::light());
    Lingmo::base::file::TemporaryDirectory scratch(fs::temp_directory_path().string(), "text-editor-capture");
    const std::string root = scratch.path();
    fs::create_directories(s_out);

    if (wanted("empty") || wanted("dump")) {
        Run run(root);
        EditorWindow &window = run.start({});
        save(window.window(), "empty");
        if (wanted("dump"))
            std::printf("%s\n", window.window().layoutDump().c_str());
    }
    if (wanted("main-menu")) {
        Run run(root);
        EditorWindow &window = run.start({});
        window.showMainMenu();
        spin(50);
        save(window.window(), "main-menu");
        savePopups(window.window(), "main-menu");
    }
    if (wanted("settings")) {
        Run run(root);
        EditorWindow &window = run.start({});
        window.showPreferences();
        spin(50);
        savePopups(window.window(), "settings");
    }
    if (wanted("shortcuts")) {
        Run run(root);
        EditorWindow &window = run.start({});
        window.showShortcuts();
        spin(50);
        savePopups(window.window(), "shortcuts");
    }

    const std::vector<std::string> files{"hello.cpp", "script.py", "notes.md", "data.json", "page.html", "cjk.txt"};
    const std::vector<std::string> names{"syntax-cpp", "syntax-python", "syntax-markdown", "syntax-json", "syntax-html",
                                         "cjk-wrap"};
    {
        Run run(root);
        EditorWindow &editor = run.start(files);
        Window &window = editor.window();
        for (size_t i = 0; i < names.size(); ++i) {
            TestInput::typeKey(window, key::PageDown, ControlModifier);
            spin(80);
            if (wanted(names[i]))
                save(window, names[i]);
        }
        TestInput::typeKey(window, key::PageDown, ControlModifier);
        spin(80);
        if (wanted("find")) {
            TestInput::typeKey(window, key::letter('f'), ControlModifier);
            TestInput::typeText(window, "greet");
            spin(150);
            save(window, "find");
            TestInput::typeKey(window, key::Escape);
            spin(50);
        }
        if (wanted("find-replace")) {
            TestInput::typeKey(window, key::letter('h'), ControlModifier);
            TestInput::typeText(window, "Hi");
            spin(150);
            save(window, "find-replace");
            TestInput::typeKey(window, key::Escape);
            spin(50);
        }
        if (wanted("goto-line")) {
            TestInput::typeKey(window, key::letter('l'), ControlModifier);
            TestInput::typeText(window, "12");
            spin(80);
            save(window, "goto-line");
            TestInput::typeKey(window, key::Escape);
            spin(50);
        }
        if (wanted("modified")) {
            TestInput::click(window, {300, 200});
            TestInput::typeText(window, "x");
            spin(80);
            save(window, "modified");
            TestInput::typeKey(window, key::letter('z'), ControlModifier);
        }
    }
    {
        // Twenty tabs.
        Run run(root);
        std::vector<std::string> tabs = files;
        for (int i = 1; i <= 14; ++i) {
            char name[16];
            std::snprintf(name, sizeof(name), "tab%02d.txt", i);
            tabs.push_back(name);
        }
        if (wanted("tabs-overflow")) {
            EditorWindow &editor = run.start(tabs);
            spin(200);
            save(editor.window(), "tabs-overflow");
        }
    }
    const std::vector<std::tuple<std::string, std::string, std::pair<std::string, std::string>>> options{
        {"cjk-nowrap", "cjk.txt", {"wordWrap", "false"}},
        {"no-line-numbers", "hello.cpp", {"lineNumbers", "false"}},
        {"whitespace", "cjk.txt", {"showWhitespace", "true"}},
    };
    for (const auto &[name, file, setting] : options) {
        if (!wanted(name))
            continue;
        Run run(root, {setting});
        EditorWindow &editor = run.start({file});
        save(editor.window(), name);
    }
    if (wanted("notice-mixed") || wanted("notice-binary")) {
        Run run(root);
        EditorWindow &editor = run.start({"mixed-endings.txt", "binary.dat"});
        editor.selectTab(0);
        spin(80);
        if (wanted("notice-mixed"))
            save(editor.window(), "notice-mixed");
        editor.selectTab(1);
        spin(80);
        if (wanted("notice-binary"))
            save(editor.window(), "notice-binary");
    }
    return 0;
}
