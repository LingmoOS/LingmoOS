// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Renders a browser window headless with its layout:
//   filemanager-capture OUT.png FOLDER [WIDTH HEIGHT SCALE [dark] [list] [select=NAME...]]
#include "model/foldermodel.h"
#include "desktop/desktopview.h"
#include "views/folderwindow.h"

#include "base/event_loop.h"
#include "base/paths.h"
#include "base/timer.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/theme.h"
#include "ui/window.h"

#include "core/preferences.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace Lingmo;
using namespace std::chrono_literals;

namespace {

class Host : public fm::FileManagerHost
{
public:
    fm::FolderWindow* openFolderWindow(const fm::FolderPage::Start&, ui::Window*) override { return nullptr; }
    void openFolders(const std::vector<std::string>&) override {}
    void showProperties(const fm::FileItemList&, ui::Window*) override {}
    void showOpenWith(const std::string&, ui::Window*) override {}
    void confirmDelete(const std::vector<std::string>&, ui::Window*) override {}
    void confirmEmptyTrash(ui::Window*) override {}
    void confirmLaunch(const std::string&, ui::Window*) override {}
    void windowClosed(fm::FolderWindow*) override {}
};

} // namespace

// The desktop's icons of ~/Desktop alone, transparent, under a 32 px bar.
static int captureDesktop(ui::Application& app, const std::string& output, double width, double height,
                          double scale, const std::vector<std::string>& select, bool lateWorkArea)
{
    const std::string path = base::paths::userDirectory(base::paths::UserDirectory::Desktop);
    fm::FolderModel model(nullptr, true);
    model.setSortMode(-1);
    model.setUrl(base::url::fromLocalFile(path));

    ui::Window window;
    window.setScale(scale);
    window.resize({width, height});
    window.setBackgroundColor(ui::Color {0, 0, 0, 0});
    auto view = std::make_unique<fm::DesktopGridView>(model);
    fm::DesktopGridView* raw = view.get();
    window.setContentView(std::move(view));
    // The compositor's work area comes after the first layout on a live session.
    if (!lateWorkArea)
        raw->setWorkArea({0, 32, width, height - 32});
    window.show();

    base::Timer poll;
    int ticks = 0;
    poll.onTimeout = [&] {
        if (model.status() == fm::FolderModel::Status::Ready || ++ticks > 200)
            app.quit();
    };
    poll.start(20ms);
    app.run();
    if (lateWorkArea) {
        window.grab();
        raw->setWorkArea({0, 32, width, height - 32});
    }
    for (const std::string& name : select)
        model.setSelected(model.rowForUrl(base::url::fromLocalFile(path + "/" + name)));
    window.grab();
    base::Timer settle;
    settle.setSingleShot(true);
    settle.onTimeout = [&] { app.quit(); };
    settle.start(1500ms);
    app.run();
    window.grab().savePng(output);
    std::ofstream(output + ".txt") << window.layoutDump();
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s OUT.png FOLDER [WIDTH HEIGHT SCALE [dark] [list] [late-work-area] [select=NAME]]\n", argv[0]);
        return 1;
    }
    const std::string output = argv[1];
    const std::string folder = argv[2];
    const double width = argc > 3 ? std::atof(argv[3]) : 900;
    const double height = argc > 4 ? std::atof(argv[4]) : 580;
    const double scale = argc > 5 ? std::atof(argv[5]) : 1.5;
    bool dark = false;
    bool list = false;
    bool lateWorkArea = false;
    std::vector<std::string> select;
    for (int i = 6; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "dark")
            dark = true;
        else if (argument == "list")
            list = true;
        else if (argument == "late-work-area")
            lateWorkArea = true;
        else if (argument.starts_with("select="))
            select.push_back(argument.substr(7));
    }

    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    ui::Theme::setCurrent(dark ? ui::Theme::dark() : ui::Theme::light());
    if (folder == "desktop")
        return captureDesktop(app, output, width, height, scale, select, lateWorkArea);
    fm::Preferences::instance().setReadOnly(true);
    fm::Preferences::instance().setViewMethod(list ? fm::Preferences::List : fm::Preferences::Grid);

    Host host;
    fm::FolderWindow window(&host, {folder, {}, {}, {}});
    window.window().setScale(scale);
    window.window().resize({width, height});
    window.show();

    base::Timer poll;
    int ticks = 0;
    poll.onTimeout = [&] {
        fm::FolderPage* page = window.currentPage();
        if ((page && page->model().status() == fm::FolderModel::Status::Ready) || ++ticks > 200)
            app.quit();
    };
    poll.start(20ms);
    app.run();

    if (fm::FolderPage* page = window.currentPage()) {
        for (const std::string& name : select) {
            const int row = page->model().rowForUrl(base::url::fromLocalFile(folder + "/" + name));
            page->model().setSelected(row);
        }
    }

    // A headless window lays out only when grabbed; what it asks for then
    // (thumbnails, folder sizes) arrives from workers.
    window.window().grab();
    base::Timer settle;
    settle.setSingleShot(true);
    settle.onTimeout = [&] { app.quit(); };
    settle.start(1500ms);
    app.run();

    window.window().grab().savePng(output);
    std::ofstream(output + ".txt") << window.window().layoutDump();
    return 0;
}
