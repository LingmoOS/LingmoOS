// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "processespage.h"

#include "base/signal.h"
#include "ui/menu.h"
#include "ui/toast.h"
#include "ui/window.h"

#include <memory>

class Monitor;
class ProcessDetails;
class ResourcePage;

namespace Lingmo::ui {
class ConfirmDialog;
}

// The system monitor's window: the resources in a sidebar, the process list
// or a resource's page beside it.
class MainWindow
{
public:
    explicit MainWindow(Monitor& monitor);
    ~MainWindow();

    Lingmo::ui::Window& window() { return m_window; }
    int currentPage() const { return m_page; }
    void setCurrentPage(int page);
    // Shows, raises and activates the window.
    void activate();
    void openDetails();
    void confirmEnd(bool force);
    // The selected process or application, while it runs.
    const ProcessRecord* selectedProcess() const;
    ProcessesPage* processesPage() const { return m_processes; }
    ResourcePage* resourcePage() const { return m_resource; }
    ProcessDetails* details() const { return m_details.get(); }
    Lingmo::ui::Menu& contextMenu() { return m_contextMenu; }
    Lingmo::ui::Menu& intervalMenu() { return m_intervalMenu; }
    Lingmo::ui::ConfirmDialog* confirmDialog() const { return m_confirm.get(); }

private:
    class Root;

    // The views and menus, all of which show text.
    void buildContent();
    void retranslate();
    void update();
    void updateContextMenu();

    Monitor& m_monitor;
    Lingmo::ui::Window m_window;
    // Errors from ending processes, as FishUI's passive notification.
    Lingmo::ui::Toast m_toast;
    Root* m_root;
    Selection m_selection;
    int m_page = 0;
    ProcessesPage* m_processes;
    ResourcePage* m_resource = nullptr;
    Lingmo::ui::Menu m_intervalMenu;
    Lingmo::ui::Menu m_contextMenu;
    std::unique_ptr<Lingmo::ui::ConfirmDialog> m_confirm;
    std::unique_ptr<ProcessDetails> m_details;
    Lingmo::base::Subscription m_updated;
    Lingmo::base::Subscription m_intervalChanged;
    Lingmo::base::Subscription m_languageChanged;
};
