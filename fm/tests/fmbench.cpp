// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The folder model's costs:
//   fmbench DIR readonly | order
#include "testutil.h"

#include "core/preferences.h"
#include "model/foldermodel.h"

#include "base/url.h"
#include "ui/application.h"

#include <cstdio>
#include <ctime>
#include <sys/resource.h>

using namespace Lingmo;

static double cpuMs()
{
    rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (ru.ru_utime.tv_sec + ru.ru_stime.tv_sec) * 1000.0 + (ru.ru_utime.tv_usec + ru.ru_stime.tv_usec) / 1000.0;
}

static double threadMs()
{
    timespec ts;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static double wallMs()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(int argc, char** argv)
{
    if (argc < 3)
        return 1;
    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    const std::string dir = argv[1];
    const std::string mode = argv[2];
    fm::FolderModel model;
    int completed = 0;
    const base::Subscription subscription = model.listingCompleted.connect([&] { ++completed; });
    const auto step = [&](const char* name, const std::function<void()>& action) {
        const int before = completed;
        const double w0 = wallMs();
        const double c0 = cpuMs();
        const double g0 = threadMs();
        action();
        if (!fmtest::runUntil(app.loop(), [&] { return completed > before; }, 120000))
            std::printf("%-22s TIMEOUT\n", name);
        else
            std::printf("%-22s wall %8.1f ms  cpu %8.1f ms  gui %7.1f ms\n", name, wallMs() - w0, cpuMs() - c0,
                        threadMs() - g0);
        std::fflush(stdout);
    };

    if (mode == "readonly") {
        model.setSortMode(fm::Preferences::Name);
        step("open", [&] { model.setUrl(base::url::fromLocalFile(dir)); });
        step("sort modified", [&] { model.setSortMode(2); });
        step("sort name", [&] { model.setSortMode(fm::Preferences::Name); });
        step("refresh", [&] { model.refresh(); });
        return 0;
    }
    if (mode == "order") {
        const auto dump = [&](const std::string& label) {
            std::printf("== %s\n", label.c_str());
            for (int row = 0; row < model.count(); ++row)
                std::printf("%s\n", model.item(row)->name.c_str());
        };
        model.setSortMode(fm::Preferences::Name);
        int before = completed;
        model.setUrl(base::url::fromLocalFile(dir));
        fmtest::runUntil(app.loop(), [&] { return completed > before; }, 120000);
        dump("name");
        for (int column : {1, 2, 3, 0}) {
            for (bool descending : {true, false}) {
                for (bool dirsFirst : {false, true}) {
                    before = completed;
                    model.setSortMode(column);
                    model.setSortDesc(descending);
                    model.setSortDirsFirst(dirsFirst);
                    model.refresh();
                    fmtest::runUntil(app.loop(), [&] { return completed > before; }, 120000);
                    fmtest::runFor(app.loop(), 300);
                    dump(std::to_string(column) + (descending ? " desc" : " asc") + (dirsFirst ? " dirs" : " mixed"));
                }
            }
        }
        return 0;
    }
    return 1;
}
