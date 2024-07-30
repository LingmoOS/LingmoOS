/*
 *   SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 *   SPDX-FileCopyrightText: 2009 Mathias Soeken <msoeken@informatik.uni-bremen.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MODULE_VIEW_H
#define MODULE_VIEW_H

#include <KAuth/Action>
#include <KPageView>

#include <QModelIndex>
#include <QWidget>

class QQmlEngine;

class KCModule;
class KPageWidgetItem;
class KPluginMetaData;
class MenuItem;

/**
 * @brief Provides a convenient way to display modules
 *
 * Provides a standardised interface for displaying multiple modules simultaneously
 * and provides facilities to access the current module, and to load its help, restore
 * default settings, save new settings and revert changes to settings
 *
 * It also provides checking for when a module has changed its configuration, and will prompt
 * if the user tries to change module or view if BaseMode is reimplemented correctly
 *
 * It also provides signals for active module changes, configuration changes and for when it has
 * been requested to close by button press
 *
 * @author Mathias Soeken <msoeken@informatik.uni-bremen.de>
 * @author Ben Cooksley <bcooksley@kde.org>
 */
class ModuleView : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructs a ModuleView, with the parent specified.
     */
    explicit ModuleView(const std::shared_ptr<QQmlEngine> &engine, QWidget *parent = nullptr);

    /**
     * Destroys the module view, along with all modules loaded, and any changes present in them.
     *
     * @warning The user will not be prompted to save changes if any exist.
     */
    ~ModuleView() override;

    /**
     * Provides the module information, which is used to set the caption of the window when either the
     * active module or configuration changes.
     */
    QString activeModuleName() const;

    /**
     * Resolves any changes in the currently active module by prompting the user if they exist.
     *
     * @returns true if the user saved or discarded changes, or there were no changes at all.
     * @returns false if the user canceled the module change.
     */
    bool resolveChanges();

    /**
     * Closes all active modules, after checking there are no active changes.
     *
     * @warning This forces all modules to be destroyed regardless of if changes exist or not
     * If possible, always check with resolveChanges() first.
     */
    void closeModules();

    void setFaceType(KPageView::FaceType type);

    /**
     * Sets whether Systemsettings should save statisctics about
     * most used modules using KActivities::Stats
     */
    void setSaveStatistics(bool save);

    /**
     * @returns whether Systemsettings should save statisctics about
     * most used module
     */
    bool saveStatistics() const;

    /**
     * Shows or hides the Apply button.
     */
    void setApplyVisible(bool visible);

    /**
     * @returns True if the Apply button is visible.
     */
    bool isApplyVisible() const;

    /**
     * Shows or hides the Defaults button.
     */
    void setDefaultsVisible(bool visible);

    /**
     * @returns True if the Defaults button is visible.
     */
    bool isDefaultsVisible() const;

    /**
     * @returns True if the Reset button is visible.
     */
    bool isResetVisible() const;

    /**
     * Show or hide defaults indicators (field level)
     */
    void moduleShowDefaultsIndicators(bool show);

    qreal headerHeight() const;
    void setHeaderHeight(qreal height);

    void setActiveModule(const QString &pluginId);

    void requestActivation(const QVariantList &args);

    /**
     * The active module's KPluginMetaData.
     */
    KPluginMetaData activeModuleMetadata() const;

public Q_SLOTS:
    /**
     * Loads the module specified by menuItem.\n
     * If the module has children, they will all be loaded instead of the module.
     *
     * @param menuItem the QModelIndex that you want to load. Must be sourced from either MenuModel or MenuProxyModel
     */
    void loadModule(const QModelIndex &menuItem, const QStringList &args);

    /**
     * Will open KHelpCenter, and load the help for the active module.
     */
    void moduleHelp();

    /**
     * Causes the active module to reload its configuration, reverting all changes.
     */
    void moduleLoad();

    /**
     * Causes the active module to save its configuration, applying all changes.
     */
    bool moduleSave();

    /**
     * Causes the active module to revert all changes to the configuration, and return to defaults.
     */
    void moduleDefaults();

    /**
     * Reimplemented for internal reasons.\n
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool resolveChanges(KCModule *currentProxy);
    void addModule(MenuItem *item, const QStringList &args);
    bool moduleSave(KCModule *module);
    void updatePageIconHeader(KPageWidgetItem *page);
    void updateButtons();

private Q_SLOTS:
    void activeModuleChanged(KPageWidgetItem *current, KPageWidgetItem *previous);
    void stateChanged();
    void authStatusChanged(KAuth::Action::AuthStatus status);

Q_SIGNALS:
    /**
     * Emitted when the currently active module is changed. This occurs whenever the active module or
     * its configuration changes. This causes the window caption to update.
     */
    void moduleChanged(bool state);

    /**
     * Emitted after the currently active module was saved.
     */
    void moduleSaved();

    /**
     * Emitted when the ModuleView is asked to close.\n
     */
    void closeRequest();

    /**
     * Emitted when showDefaultsIndicators state changed
     */
    void showDefaultsIndicatorsChanged(bool show);

private:
    class Private;
    Private *const d;
};

#endif
