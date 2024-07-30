/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SETTINGS_BASE_H
#define SETTINGS_BASE_H

#include "MenuItem.h"
#include "SidebarMode.h"

#include <QButtonGroup>
#include <QMap>
#include <QStackedWidget>

#include <KAboutApplicationDialog>
#include <KActionMenu>
#include <KConfigDialog>
#include <KLineEdit>
#include <KMainWindow>

class QScreen;
class KHelpMenu;

class SettingsBase : public KMainWindow
{
    Q_OBJECT

public:
    explicit SettingsBase(SidebarMode::ApplicationMode mode, const QString &startupModule, const QStringList &startupModuleArgs, QWidget *parent = nullptr);
    ~SettingsBase() override;
    bool queryClose() override;

    void setStartupModule(const QString &startupModule);
    void setStartupModuleArgs(const QStringList &startupModuleArgs);
    void reloadStartupModule();

protected:
    QSize sizeHint() const override;

private Q_SLOTS:
    void initApplication();
    void initToolBar();
    void initMenuList(MenuItem *parent);
    void about();
    void viewChange(bool state);

private Q_SLOTS:
    /**
     * Updates the window size limit
     */
    void slotGeometryChanged();

private:
    /**
     * Initializes the sidebar plugin
     */
    void loadCurrentView();

    // Follow screen resolution
    QScreen *m_screen = nullptr;

    // The plugins
    SidebarMode *view = nullptr;
    // The toolbar
    QWidgetAction *spacerAction = nullptr;
    QAction *highlightChangesAction = nullptr;
    QAction *reportPageSpecificBugAction = nullptr;
    QAction *quitAction = nullptr;
    // The help menu
    QAction *aboutViewAction = nullptr;
    KActionMenu *helpActionMenu = nullptr;
    // The control module
    QStackedWidget *stackedWidget = nullptr;
    // The module list
    MenuItem *rootModule = nullptr;
    MenuItem *homeModule = nullptr;
    MenuItem *lostFound = nullptr;
    QStringList categories;
    QList<KPluginMetaData> pluginModules;
    // The about dialog
    KAboutApplicationDialog *aboutDialog = nullptr;
    SidebarMode::ApplicationMode m_mode = SidebarMode::SystemSettings;
    QString m_startupModule;
    QStringList m_startupModuleArgs;
    KActionCollection *m_actionCollection;
    KHelpMenu *m_helpMenu;
};
#endif
