// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/foldermenu.h"
#include "core/launcher.h"
#include "model/foldermodel.h"

#include "base/i18n.h"
#include "ui/menu.h"

#include <functional>
#include <map>
#include <vector>

using Lingmo::i18n::tr;

namespace fm {

void triggerAction(FolderModel& model, const std::string& name)
{
    static const std::map<std::string, std::function<void(FolderModel&)>> actions = {
        {"open", [](FolderModel& m) { m.openSelected(); }},
        {"openWith", [](FolderModel& m) { m.showOpenWithDialog(); }},
        {"showInFolder", [](FolderModel& m) { m.showInFolder(); }},
        {"compress", [](FolderModel& m) { m.compressSelected(); }},
        {"cut", [](FolderModel& m) { m.cut(); }},
        {"copy", [](FolderModel& m) { m.copy(); }},
        {"paste", [](FolderModel& m) { m.paste(); }},
        {"pasteInto", [](FolderModel& m) { m.pasteIntoSelection(); }},
        {"newFolder", [](FolderModel& m) { m.newFolder(); }},
        {"newTextFile", [](FolderModel& m) { m.newTextFile(); }},
        {"trash", [](FolderModel& m) { m.moveSelectedToTrash(); }},
        {"emptyTrash", [](FolderModel& m) { m.emptyTrash(); }},
        {"del", [](FolderModel& m) { m.openDeleteDialog(); }},
        {"rename", [](FolderModel& m) { m.requestRename.emit(); }},
        {"terminal", [](FolderModel& m) { m.openInTerminal(); }},
        {"wallpaper", [](FolderModel& m) { m.setWallpaperSelected(); }},
        {"properties", [](FolderModel& m) { m.openPropertiesDialog(); }},
        {"changeBackground", [](FolderModel& m) { m.openChangeWallpaperDialog(); }},
        {"restore", [](FolderModel& m) { m.restoreFromTrash(); }},
        {"showHidden", [](FolderModel& m) { m.setShowHiddenFiles(!m.showHiddenFiles()); }},
        {"openInNewWindow", [](FolderModel& m) { m.openInNewWindow(); }},
        {"openInNewTab", [](FolderModel& m) { m.openInNewTab(); }},
        {"undo", [](FolderModel& m) { m.undo(); }},
        {"redo", [](FolderModel& m) { m.redo(); }},
    };
    if (const auto found = actions.find(name); found != actions.end())
        found->second(model);
}

std::unique_ptr<ui::Menu> createFolderMenu(FolderModel& model, std::function<void()> refresh)
{
    model.prepareContextMenu();
    auto menu = std::make_unique<ui::Menu>();
    const bool selection = model.hasSelection();
    // A separator goes in only between shown rows.
    bool pendingSeparator = false;
    bool rows = false;

    const auto separator = [&](bool visible) {
        if (visible && rows)
            pendingSeparator = true;
    };
    const auto item = [&](const std::string& name, bool visible, std::string text = {}) {
        const FolderModel::Action& action = model.action(name);
        if (!visible || !action.visible)
            return false;
        if (pendingSeparator) {
            menu->addSeparator();
            pendingSeparator = false;
        }
        FolderModel* target = &model;
        const size_t index = menu->addItem(text.empty() ? action.text : text,
                                           [target, name] { triggerAction(*target, name); });
        menu->setItemEnabled(index, action.enabled);
        rows = true;
        return true;
    };

    // Without a selection: the folder.
    const bool newFolder = item("newFolder", !selection);
    bool newDocuments = false;
    if (!selection && model.action("newTextFile").visible) {
        if (pendingSeparator) {
            menu->addSeparator();
            pendingSeparator = false;
        }
        ui::Menu* documents = menu->addSubmenu(tr("New Documents"));
        const FolderModel::Action& action = model.action("newTextFile");
        FolderModel* target = &model;
        const size_t index = documents->addItem(action.text, [target] { triggerAction(*target, "newTextFile"); });
        documents->setItemEnabled(index, action.enabled);
        newDocuments = true;
        rows = true;
    }
    if (!selection && refresh) {
        if (pendingSeparator) {
            menu->addSeparator();
            pendingSeparator = false;
        }
        menu->addItem(tr("Refresh"), std::move(refresh));
        rows = true;
    }
    separator(!selection && (newFolder || newDocuments));
    item("paste", !selection);
    if (!selection) {
        if (pendingSeparator) {
            menu->addSeparator();
            pendingSeparator = false;
        }
        FolderModel* target = &model;
        menu->addItem(tr("Select All"), [target] { target->selectAll(); });
        rows = true;
    }
    separator(!selection && (model.action("terminal").visible
                             || (model.isDesktop() && model.action("changeBackground").visible)));
    item("terminal", !selection);
    item("changeBackground", !selection && model.isDesktop());
    separator(!selection && model.action("showHidden").visible);
    item("showHidden", !selection);
    separator(!selection && model.action("emptyTrash").visible);
    item("emptyTrash", !selection);

    // With one: the selection.
    item("restore", selection);
    item("open", selection);
    item("openInNewTab", selection);
    item("openInNewWindow", selection);
    item("showInFolder", selection);

    const FolderModel::Action& openWith = model.action("openWith");
    if (selection && openWith.visible) {
        const auto rowsSelected = model.selectedRows();
        std::vector<launcher::Application> applications;
        if (rowsSelected.size() == 1) {
            const FileItemPtr file = model.item(rowsSelected.front());
            applications = launcher::applicationsFor(file->path, file->mimeType);
        }
        if (pendingSeparator) {
            menu->addSeparator();
            pendingSeparator = false;
        }
        FolderModel* target = &model;
        if (applications.empty()) {
            const size_t index = menu->addItem(openWith.text, [target] { triggerAction(*target, "openWith"); });
            menu->setItemEnabled(index, openWith.enabled);
        } else {
            ui::Menu* submenu = menu->addSubmenu(openWith.text);
            for (const launcher::Application& application : applications) {
                const std::string desktopFile = application.desktopFile;
                const size_t index = submenu->addItem(application.name,
                                                      [target, desktopFile] { target->openWithApplication(desktopFile); });
                submenu->setItemIcon(index, application.icon);
            }
            submenu->addSeparator();
            // TRANSLATORS: the last item of the Open with submenu, which lists every application.
            submenu->addItem(tr("Other"), [target] { triggerAction(*target, "openWith"); });
        }
        rows = true;
    }
    item("compress", selection);

    separator(selection
              && (model.action("cut").visible || model.action("copy").visible || model.action("trash").visible
                  || model.action("del").visible || model.action("rename").visible));
    item("cut", selection);
    item("copy", selection);
    item("pasteInto", selection);
    item("trash", selection);
    item("del", selection);
    item("rename", selection);
    separator(selection && (model.action("terminal").visible || model.action("wallpaper").visible));
    item("terminal", selection);
    item("wallpaper", selection);
    separator(model.action("properties").visible);
    item("properties", true);
    return menu;
}

} // namespace fm
