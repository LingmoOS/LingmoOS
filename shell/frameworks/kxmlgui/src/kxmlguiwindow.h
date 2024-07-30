/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KXMLGUIWINDOW_H
#define KXMLGUIWINDOW_H

#include "kmainwindow.h"
#include "kxmlguibuilder.h"
#include "kxmlguiclient.h"

class KMenu;
class KXMLGUIFactory;
class KConfig;
class KConfigGroup;
class KToolBar;
class KXmlGuiWindowPrivate;

/**
 * @class KXmlGuiWindow kxmlguiwindow.h KXmlGuiWindow
 *
 * @brief KMainWindow with convenience functions and integration with XmlGui files.
 *
 * This class includes several convenience \<action\>Enabled() functions
 * to toggle the presence of functionality in your main window,
 * including a KCommandBar instance.
 *
 * The @ref StandardWindowOptions enum can be used to pass additional options
 * to describe the main window behavior/appearance.
 * Use it in conjunction with setupGUI() to load an appnameui.rc file
 * to manage the main window's actions.
 *
 * setCommandBarEnabled() is set by default.
 *
 * A minimal example can be created with
 * QMainWindow::setCentralWidget() and setupGUI():
 *
 * @code
 * MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent) {
 *   textArea = new KTextEdit();
 *   setCentralWidget(textArea);
 *   setupGUI(Default);
 * }
 * @endcode
 *
 * With this, a ready-made main window with menubar and statusbar is created,
 * as well as two default menus, Settings and Help.
 *
 * Management of QActions is made trivial in combination with
 * KActionCollection and KStandardAction.
 *
 * @code
 * void MainWindow::setupActions() {
 *   QAction *clearAction = new QAction(this);
 *   clearAction->setText(i18n("&Clear"));
 *   clearAction->setIcon(QIcon::fromTheme("document-new"));
 *   KActionCollection::setDefaultShortcut(clearAction, Qt::CTRL + Qt::Key_W);
 *   actionCollection()->addAction("clear", clearAction);
 *   connect(clearAction, &QAction::triggered, textArea, &KTextEdit::clear);
 *   KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
 *   setupGUI(Default, "texteditorui.rc");
 * }
 * @endcode
 *
 * See https://develop.kde.org/docs/use/kxmlgui/ for a tutorial
 * on how to create a simple text editor using KXmlGuiWindow.
 *
 * See https://develop.kde.org/docs/use/session-managment for more information on session management.
 *
 * @see KMainWindow
 * @see KActionCollection
 * @see KStandardAction
 * @see setupGUI()
 * @see createGUI()
 * @see setCommandBarEnabled()
 */

class KXMLGUI_EXPORT KXmlGuiWindow : public KMainWindow, public KXMLGUIBuilder, virtual public KXMLGUIClient
{
    Q_OBJECT
    Q_PROPERTY(bool hasMenuBar READ hasMenuBar)
    Q_PROPERTY(bool autoSaveSettings READ autoSaveSettings)
    Q_PROPERTY(QString autoSaveGroup READ autoSaveGroup)
    Q_PROPERTY(bool standardToolBarMenuEnabled READ isStandardToolBarMenuEnabled WRITE setStandardToolBarMenuEnabled)

public:
    /**
     * @brief Construct a main window.
     *
     * Note that by default a KXmlGuiWindow is created with the
     * Qt::WA_DeleteOnClose attribute set, i.e. it is automatically destroyed
     * when the window is closed. If you do not want this behavior, call:
     *
     * @code
     * window->setAttribute(Qt::WA_DeleteOnClose, false);
     * @endcode
     *
     * KXmlGuiWindows must be created on the heap with 'new', like:
     *
     * @code
     * KXmlGuiWindow *kmw = new KXmlGuiWindow(...);
     * kmw->setObjectName(...);
     * @endcode
     *
     * IMPORTANT: For session management and window management to work
     * properly, all main windows in the application should have a
     * different name. Otherwise, the base class KMainWindow will create
     * a unique name, but it's recommended to explicitly pass a window name that will
     * also describe the type of the window. If there can be several windows of the same
     * type, append '#' (hash) to the name, and KMainWindow will replace it with numbers to make
     * the names unique. For example, for a mail client which has one main window showing
     * the mails and folders, and which can also have one or more windows for composing
     * mails, the name for the folders window should be e.g. "mainwindow" and
     * for the composer windows "composer#".
     *
     * @param parent The widget parent. This is usually @c nullptr,
     * but it may also be the window group leader.
     * In that case, the KXmlGuiWindow becomes a secondary window.
     *
     * @param flags Specify the window flags. The default is none.
     *
     * @see KMainWindow::KMainWindow
     */
    explicit KXmlGuiWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    /**
     * @brief Destructor.
     *
     * Will also destroy the toolbars and menubar if needed.
     */
    ~KXmlGuiWindow() override;

    /**
     * @brief Creates a standard help menu when calling createGUI()
     * or setupGUI().
     *
     * @param showHelpMenu Whether to create a Help Menu. @c true by default.
     *
     * @see isHelpMenuEnabled()
     */
    void setHelpMenuEnabled(bool showHelpMenu = true);

    /**
     * @returns @c true if the help menu is enabled, @c false if setHelpMenuEnabled(false) was set.
     * @see setHelpMenuEnabled()
     */
    bool isHelpMenuEnabled() const;

    virtual KXMLGUIFactory *guiFactory();

    /**
     * @brief Generates the interface based on a local XML file.
     *
     * This is the function that generates UI elements such as the main menu,
     * toolbar (if any) and statusbar. This is called by setupGUI(Create) as well.
     *
     * Typically, in a regular application, you would use setupGUI()
     * instead, as it sets up the toolbar/shortcut
     * edit actions, among other things.
     *
     * If @p xmlfile is an empty string, this method will try to construct
     * a local XML filename like appnameui.rc where 'appname' is your app's
     * name. Typically that app name is what KXMLGUIClient::componentName()
     * returns. If that file does not exist, then the XML UI code will use only
     * the global (standard) XML file for its layout purposes.
     *
     * @param xmlfile The path (relative or absolute) to the local xmlfile
     *
     * @see setupGUI()
     */
    void createGUI(const QString &xmlfile = QString());

    /**
     * @brief Creates a toggle under the 'Settings' menu to show/hide the available toolbars.
     *
     * The standard toolbar menu toggles the visibility of one or multiple toolbars.
     *
     * If there is only one toolbar configured, a simple 'Show \<toolbar name\>'
     * menu item is shown; if more than one toolbar is configured, a "Shown Toolbars"
     * menu is created instead, with 'Show \<toolbar1 name\>', 'Show \<toolbar2 name\>'
     * ... sub-menu actions.
     *
     * If your application uses a non-default XmlGui resource file, then you can
     * specify the exact position of the menu/menu item by adding a
     * &lt;Merge name="StandardToolBarMenuHandler" /&gt;
     * line to the settings menu section of your resource file ( usually appname.rc ).
     *
     * @param showToolBarMenu Whether to show the standard toolbar menu. @c false by default.
     *
     * @note This function only makes sense before calling createGUI().
     * Using setupGUI(ToolBar) overrides this function.
     *
     * @see createGUI()
     * @see setupGUI()
     * @see KToggleBarAction
     * @see StandardWindowOption
     * @see KMainWindow::toolBar()
     * @see KMainWindow::toolBars()
     * @see QMainWindow::addToolBar()
     * @see QMainWindow::removeToolBar()
     * @see createStandardStatusBarAction()
     */
    void setStandardToolBarMenuEnabled(bool showToolBarMenu);

    /**
     * @brief Returns whether setStandardToolBarMenuEnabled() was set.
     *
     * @note This function only makes sense if createGUI() was used.
     * This function returns true only if setStandardToolBarMenuEnabled() was set
     * and will return false even if @ref StandardWindowOption::ToolBar was used.
     *
     * @returns @c true if setStandardToolBarMenuEnabled() was set, @c false otherwise.
     *
     * @see createGUI()
     * @see setupGUI()
     * @see setStandardToolBarMenuEnabled()
     * @see StandardWindowOption
     */
    bool isStandardToolBarMenuEnabled() const;

    /**
     * @brief Creates a toggle under the 'Settings' menu to show/hide the statusbar.
     *
     * Calling this method will create a statusbar if one doesn't already exist.
     *
     * If an application maintains the action on its own (i.e. never calls
     * this function), a connection needs to be made to let KMainWindow
     * know when the hidden/shown status of the statusbar has changed.
     * For example:
     * @code
     * connect(action, &QAction::triggered,
     *         kmainwindow, &KMainWindow::setSettingsDirty);
     * @endcode
     * Otherwise the status might not be saved by KMainWindow.
     *
     * @note This function only makes sense before calling createGUI()
     * or when using setupGUI() without @ref StandardWindowOption::StatusBar.
     *
     * @see createGUI()
     * @see setupGUI()
     * @see StandardWindowOption
     * @see KStandardAction::showStatusbar()
     * @see setStandardToolBarMenuEnabled()
     * @see QMainWindow::setStatusBar()
     * @see QMainWindow::statusBar()
     */
    void createStandardStatusBarAction();

    /**
     * @brief Use these options for the first argument of setupGUI().
     * @see setupGUI()
     * @see StandardWindowOption
     */
    enum StandardWindowOption {
        /**
         * Adds action(s) to show/hide the toolbar(s) and adds a menu
         * action to configure the toolbar(s).
         *
         * @see setStandardToolBarMenuEnabled()
         * @see isStandardToolBarMenuEnabled()
         */
        ToolBar = 1,

        /**
         * @brief Adds an action in the 'Settings' menu
         * to open the configure keyboard shortcuts dialog.
         */
        Keys = 2,

        /**
         * @brief Adds an action to show/hide the statusbar in the 'Settings' menu.
         * Note that setting this value will create a statusbar
         * if one doesn't already exist.
         *
         * @see createStandardStatusBarAction()
         */
        StatusBar = 4,

        /**
         * @brief Autosaves (and loads) the toolbar/menubar/statusbar settings and
         * window size using the default name.
         *
         * Like KMainWindow::setAutoSaveSettings(), enabling this causes the application
         * to save state data upon close in a KConfig-managed configuration file.
         *
         * Typically you want to let the default window size be determined by
         * the widgets' size hints. Make sure that setupGUI() is called after
         * all the widgets are created (including QMainWindow::setCentralWidget())
         * so that the default size is managed properly.
         *
         * @see KMainWindow::setAutoSaveSettings()
         * @see KConfig
         */
        Save = 8,

        /**
         * @brief Calls createGUI() once ToolBar, Keys and Statusbar have been
         * taken care of.
         *
         * @note When using KParts::MainWindow, remove this flag from
         * the setupGUI() call, since you'll be using createGUI(part)
         * instead:
         *
         * @code
         *     setupGUI(ToolBar | Keys | StatusBar | Save);
         * @endcode
         *
         * @see createGUI()
         */
        Create = 16,

        /**
         * @brief Sets all of the above options as true.
         */
        Default = ToolBar | Keys | StatusBar | Save | Create,
    };
    Q_FLAG(StandardWindowOption)
    /**
     * @brief Stores a combination of @ref StandardWindowOptions values.
     *
     * Use these options for the first argument of setupGUI().
     * @see setupGUI()
     * @see StandardWindowOption
     */
    Q_DECLARE_FLAGS(StandardWindowOptions, StandardWindowOption)

    /**
     * @brief Configures the current window and its actions in the typical KDE
     * fashion.
     *
     * You can specify which window options/features are going to be set up using
     * @p options, see the @ref StandardWindowOptions enum for more details.
     *
     * @code
     * MainWindow::MainWindow(QWidget* parent) : KXmlGuiWindow(parent){
     *   textArea = new KTextEdit();
     *   setCentralWidget(textArea);
     *   setupGUI(Default, "appnameui.rc");
     * }
     * @endcode
     *
     * Use a bitwise OR (|) to select multiple enum choices for setupGUI()
     * (except when using StandardWindowOption::Default).
     *
     * @code
     * setupGUI(Save | Create, "appnameui.rc");
     * @endcode
     *
     * Typically this function replaces createGUI(),
     * but it is possible to call setupGUI(Create) together with helper functions
     * such as setStandardToolBarMenuEnabled() and createStandardStatusBarAction().
     *
     * @warning To use createGUI() and setupGUI()
     * for the same window, you must avoid using
     * @ref StandardWindowOption::Create. Prefer using only setupGUI().
     *
     * @note When @ref StandardWindowOption::Save is used,
     * this method will restore the state of the application
     * window (toolbar, dockwindows positions ...etc), so you need to have
     * added all your actions to your UI before calling this
     * method.
     *
     * @param options A combination of @ref StandardWindowOptions to specify
     * UI elements to be present in your application window.
     * @param xmlfile The relative or absolute path to the local xmlfile.
     * If this is an empty string, the code will look for a local XML file
     * appnameui.rc, where 'appname' is the name of your app. See the note
     * about the xmlfile argument in createGUI().
     * @see StandardWindowOption
     */
    void setupGUI(StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /**
     * @brief This is an overloaded function.
     *
     * @param defaultSize A manually specified window size that overrides the saved size.
     * @param options A combination of @ref StandardWindowOptions to specify
     * UI elements to be present in your application window.
     * @param xmlfile The relative or absolute path to the local xmlfile.
     * @see setupGUI()
     */
    void setupGUI(const QSize &defaultSize, StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /**
     * @returns A pointer to the main window's action responsible for the toolbar's menu.
     */
    QAction *toolBarMenuAction();

    /**
     * @internal for KToolBar
     */
    void setupToolbarMenuActions();

    /**
     * @internal
     */
    void finalizeGUI(bool force);
    using KXMLGUIBuilder::finalizeGUI;

    // reimplemented for internal reasons
    void applyMainWindowSettings(const KConfigGroup &config) override;

    /**
     * @brief Enable a KCommandBar to list and quickly execute actions.
     *
     * A KXmlGuiWindow by default automatically creates a KCommandBar,
     * but it is inaccessible unless createGUI() or setupGUI(Create) is used.
     *
     * It provides a HUD-like menu that lists all QActions in your application
     * and can be activated via Ctrl+Atl+i or via an action in the 'Help' menu.
     *
     * If you need more than a global set of QActions listed for your application,
     * use KCommandBar directly instead.
     *
     * @param showCommandBar Whether to show the command bar. @c true by default.
     *
     * @since 5.83
     *
     * @see KCommandBar
     * @see KCommandBar::setActions()
     * @see isCommandBarEnabled()
     */
    void setCommandBarEnabled(bool showCommandBar);

    /**
     * @brief Returns whether a KCommandBar was set.
     * @returns @c true by default, @c false if setCommandBarEnabled(false) was set.
     * @since 5.83
     * @see setCommandBarEnabled()
     */
    bool isCommandBarEnabled() const;

public Q_SLOTS:
    /**
     * @brief Show a standard configure toolbar dialog.
     *
     * This slot can be connected directly to the action to configure the toolbar.
     *
     * @code
     * KStandardAction::configureToolbars(this, &KXmlGuiWindow::configureToolbars, actionCollection);
     * @endcode
     */
    virtual void configureToolbars();

    /**
     * @brief Applies a state change
     *
     * Reimplement this to enable and disable actions as defined in the XmlGui rc file.
     *
     * @param newstate The state change to be applied.
     */
    virtual void slotStateChanged(const QString &newstate);

    /**
     * @brief Applies a state change
     *
     * Reimplement this to enable and disable actions as defined in the XmlGui rc file.
     *
     * This function can "reverse" the state (disable the actions which should be
     * enabled, and vice-versa) if specified.
     *
     * @param newstate The state change to be applied.
     * @param reverse Whether to reverse @p newstate or not.
     */
    void slotStateChanged(const QString &newstate, bool reverse);

protected:
    /**
     * Reimplemented to catch QEvent::Polish in order to adjust the object name
     * if needed, once all constructor code for the main window has run.
     * Also reimplemented to catch when a QDockWidget is added or removed.
     */
    bool event(QEvent *event) override;

    /**
     * @brief Checks if there are actions using the same shortcut.
     *
     * This is called automatically from createGUI().
     *
     * @since 5.30
     */
    void checkAmbiguousShortcuts();

protected Q_SLOTS:
    /**
     * @brief Rebuilds the GUI after KEditToolBar changes the toolbar layout.
     * @see configureToolbars()
     */
    virtual void saveNewToolbarConfig();

private:
    Q_DECLARE_PRIVATE(KXmlGuiWindow)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KXmlGuiWindow::StandardWindowOptions)

#endif
