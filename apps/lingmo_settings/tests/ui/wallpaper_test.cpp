// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "input_test_support.h"
#include "pages/wallpaper/wallpapergrid.h"
#include "pages/wallpaper/wallpapers.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo::ui;
using namespace settings;

TEST(Wallpapers, SortByFileNameWithNumbersByValue)
{
    std::vector<std::string> paths{"/a/b/img10.jpg", "/a/img2.jpg", "/c/Img1.png", "/b/img2.jpg"};
    wallpapers::sortByFileName(paths);
    EXPECT_EQ(paths, (std::vector<std::string>{"/c/Img1.png", "/a/img2.jpg", "/b/img2.jpg", "/a/b/img10.jpg"}));
    EXPECT_EQ(wallpapers::folderTitle("/home/user/Pictures/"), "Pictures");
    EXPECT_EQ(wallpapers::folderTitle("/srv/walls"), "walls");
}

TEST(WallpaperGrid, AddsAColumnOnceEveryColumnCanBe128Wide)
{
    WallpaperGrid grid("System");
    grid.setWallpapers(std::vector<std::string>(10, "/x.jpg"));
    // Title, spacing, one row of thumbnails 0.625 as high as wide.
    EXPECT_EQ(grid.heightForWidth(590), 24 + 12 + 81);  // 4 columns of 129.5
    EXPECT_EQ(grid.heightForWidth(500), 24 + 12 + 94);  // 3 of 150.67
    EXPECT_EQ(grid.heightForWidth(300), 24 + 12 + 86);  // 2 of 138
    EXPECT_EQ(grid.heightForWidth(200), 24 + 12 + 100); // 1 of 160
}

TEST(WallpaperGrid, ShowsAllRowsOnRequest)
{
    Window window;
    window.setScale(1.5);
    window.resize({590, 400});
    auto content = std::make_unique<WallpaperGrid>("System");
    WallpaperGrid* grid = content.get();
    grid->setWallpapers(std::vector<std::string>(10, "/x.jpg"));
    window.setContentView(std::move(content));
    window.show();
    window.grab();
    const RectF showAll = settings::testing::windowRect(window, "wallpaper:showAll");
    EXPECT_EQ(showAll.x + showAll.width, 590);
    TestInput::click(window, {showAll.x + showAll.width / 2, showAll.y + showAll.height / 2});
    window.grab();
    // Three rows of 81 with 24 between.
    EXPECT_EQ(grid->heightForWidth(590), 24 + 12 + 3 * 81 + 2 * 24);
}
