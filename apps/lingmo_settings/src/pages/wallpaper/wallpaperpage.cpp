// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <memory>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "lingmo/appearance/appearance.h"
#include "pages/wallpaper/wallpapergrid.h"
#include "pages/wallpaper/wallpapers.h"
#include "ui/file_chooser.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/menu.h"
#include "ui/cell_layout.h"
#include "widgets/card.h"
#include "widgets/metrics.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace settings {

namespace {

class WallpaperPage : public Page
{
public:
    explicit WallpaperPage(SettingsWindow& window)
        : Page(window, tr("Wallpaper"))
    {
        Card* card = content()->addChild<Card>();
        card->setSpacing(metrics::sectionSpacing);

        m_user = card->addChild<WallpaperGrid>(tr("Your wallpapers"));
        m_user->setName("wallpaper:user");
        m_user->setLayoutHints({.fillWidth = true});
        m_user->setShowAddWallpaper(true);
        m_user->setRemovable(true);
        m_user->onSelected = [this](const std::string& path) { select(path); };
        m_user->onAddRequested = [this](PointF position) { m_addMenu.popup(&settingsWindow().window(), position); };
        m_user->onRemoveRequested = [this](const std::string& path) { m_appearance.removeUserWallpaper(path); };

        m_system = card->addChild<WallpaperGrid>(tr("System"));
        m_system->setName("wallpaper:system");
        m_system->setLayoutHints({.fillWidth = true});
        m_system->setWallpapers(wallpapers::system());
        m_system->onSelected = [this](const std::string& path) { select(path); };

        m_folderColumn = card->addChild<ColumnLayout>();
        m_folderColumn->setSpacing(metrics::sectionSpacing);
        m_folderColumn->setLayoutHints({.fillWidth = true});

        ColumnLayout* colors = card->addChild<ColumnLayout>();
        colors->setSpacing(metrics::cardSpacing);
        colors->setLayoutHints({.fillWidth = true});
        colors->addChild<Label>(tr("Colors"));
        m_swatches = colors->addChild<ColorSwatches>();
        m_swatches->setName("wallpaper:colors");
        m_swatches->setLayoutHints({.fillWidth = true});
        m_swatches->onSelected = [this](const std::string& color) {
            m_appearance.setBackgroundType(1);
            m_appearance.setBackgroundColor(color);
        };

        m_addMenu.addItem(tr("Choose file…"), [this] { chooseFile(); });
        m_addMenu.addItem(tr("Choose folder…"), [this] { chooseFolder(); });

        m_changed = m_appearance.changed.connect([this] { update(); });
        m_folderImagesChanged = m_appearance.wallpaperFolderImagesChanged.connect([this](const std::string& folder) {
            for (size_t i = 0; i < m_folders.size(); ++i) {
                if (m_folders[i] == folder)
                    m_folderGrids[i]->setWallpapers(folderWallpapers(folder));
            }
        });
        update();
    }

private:
    std::vector<std::string> folderWallpapers(const std::string& folder) const
    {
        std::vector<std::string> images = m_appearance.wallpaperFolderImages(folder);
        wallpapers::sortByFileName(images);
        return images;
    }

    void chooseFile()
    {
        m_chooser = std::make_unique<FileChooser>(&settingsWindow().window());
        m_chooser->setTitle(tr("Add wallpaper"));
        m_chooser->setFilters({FileChooser::Filter::fromNameFilter(
                format(tr("Image files ({patterns})"),
                       {{"patterns", "*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.avif"}}))});
        m_chooser->onAccepted = [this](const std::vector<std::string>& paths, const std::string&) {
            if (!paths.empty())
                select(paths.front());
        };
        m_chooser->open();
    }

    void chooseFolder()
    {
        m_chooser = std::make_unique<FileChooser>(&settingsWindow().window());
        m_chooser->setMode(FileChooser::Mode::OpenFolder);
        m_chooser->setTitle(tr("Add Wallpaper Folder"));
        m_chooser->onAccepted = [this](const std::vector<std::string>& paths, const std::string&) {
            if (!paths.empty())
                m_appearance.addWallpaperFolder(paths.front());
        };
        m_chooser->open();
    }

    void select(const std::string& path)
    {
        m_appearance.setBackgroundType(0);
        m_appearance.setWallpaper(path);
    }

    void update()
    {
        m_user->setWallpapers(m_appearance.userWallpapers());

        const std::vector<std::string> folders = m_appearance.wallpaperFolders();
        if (folders != m_folders) {
            while (!m_folderColumn->children().empty())
                m_folderColumn->takeChild(m_folderColumn->children().back().get());
            m_folderGrids.clear();
            for (const std::string& folder : folders) {
                WallpaperGrid* grid = m_folderColumn->addChild<WallpaperGrid>(wallpapers::folderTitle(folder), "folder");
                grid->setLayoutHints({.fillWidth = true});
                grid->setClosable(true);
                grid->setWallpapers(folderWallpapers(folder));
                grid->onSelected = [this](const std::string& path) { select(path); };
                grid->onCloseRequested = [this, folder] { m_appearance.removeWallpaperFolder(folder); };
                m_folderGrids.push_back(grid);
            }
            m_folders = folders;
            m_folderColumn->setVisible(!folders.empty());
        }

        const bool imageSelected = m_appearance.backgroundType() == 0;
        const std::string current = m_appearance.wallpaper();
        m_user->setCurrent(current, imageSelected);
        m_system->setCurrent(current, imageSelected);
        for (WallpaperGrid* grid : m_folderGrids)
            grid->setCurrent(current, imageSelected);
        m_swatches->setCurrent(m_appearance.backgroundColor(), m_appearance.backgroundType() == 1);
    }

    appearance::Appearance m_appearance;
    base::Subscription m_changed;
    base::Subscription m_folderImagesChanged;
    WallpaperGrid* m_user;
    WallpaperGrid* m_system;
    ColumnLayout* m_folderColumn;
    std::vector<std::string> m_folders;
    std::vector<WallpaperGrid*> m_folderGrids;
    ColorSwatches* m_swatches;
    Menu m_addMenu;
    std::unique_ptr<FileChooser> m_chooser;
};

} // namespace

static const PageRegistration registration("background", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<WallpaperPage>(window);
});

} // namespace settings
