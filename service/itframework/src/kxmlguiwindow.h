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
 * @short Top level main window with predefined action layout
 *
 * Instead of creating a KMainWindow manually and assigning menus, menu entries,
 * toolbar buttons and actions to it by hand, this class can be used to load an
 * rc file to manage the main window's actions.
 *
 * See https://techbase.kde.org/Development/Tutorials/Using_KActions#XMLGUI
 * for essential information on the XML file format and usage of this class.
 *
 * @see KMainWindow
 * @author Reginald Stadlbauer (reggie@kde.org)
 * @author Stephan Kulow (coolo@kde.org)
 * @author Matthias Ettrich (ettrich@kde.org)
 * @author Chris Schlaeger (cs@kde.org)
 * @author Sven Radej (radej@kde.org)
 * Maintained by Sven Radej (radej@kde.org)
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
     * Construct a main window.
     *
     * @param parent The widget parent. This is usually 0 but it may also be the window
     * group leader. In that case, the KXmlGuiWindow becomes sort of a
     * secondary window.
     *
     * @param f Specify the window flags. The default is none.
     *
     * Note that a KXmlGuiWindow per-default is created with the
     * Qt::WA_DeleteOnClose attribute set, i.e. it is automatically destroyed
     * when the window is closed. If you do not want this behavior, call
     * \code
     * window->setAttribute(Qt::WA_DeleteOnClose, false);
     * \endcode
     *
     * KXmlGuiWindows must be created on the heap with 'new', like:
     * \code
     * KXmlGuiWindow *kmw = new KXmlGuiWindow(...);
     * kmw->setObjectName(...);
     * \endcode
     *
     * IMPORTANT: For session management and window management to work
     * properly, all main windows in the application should have a
     * different name. If you don't do it, the base class KMainWindow will create
     * a unique name, but it's recommended to explicitly pass a window name that will
     * also describe the type of the window. If there can be several windows of the same
     * type, append '#' (hash) to the name, and KMainWindow will replace it with numbers to make
     * the names unique. For example, for a mail client which has one main window showing
     * the mails and folders, and which can also have one or more windows for composing
     * mails, the name for the folders window should be e.g. "mainwindow" and
     * for the composer windows "composer#".
     *
     */
    explicit KXmlGuiWindow(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /**
     * \brief Destructor.
     *
     * Will also destroy the toolbars, and menubar if
     * needed.
     */
    ~KXmlGuiWindow() override;

    /**
     * Enables the build of a standard help menu when calling @ref createGUI()
     * or @ref setupGUI().
     *
     * The default behavior is to create a help menu, you must call this
     * function with @c false to disable that.
     */
    void setHelpMenuEnabled(bool showHelpMenu = true);

    /**
     * Returns @c true if the help menu is enabled, @c false otherwise.
     */
    bool isHelpMenuEnabled() const;

    virtual KXMLGUIFactory *guiFactory();

    /**
     * Creates a GUI based on a local XML file.
     *
     * Typically, i.e. in a regular application, you should use @ref setupGUI()
     * instead, since it runs some extra code, like setting up the toolbar/shortcut
     * edit actions ...etc.
     *
     * If @p xmlfile is an empty string, this method will try to construct
     * a local XML filename like appnameui.rc where 'appname' is your app's
     * name. Typically that app name is what @ref KXMLGUIClient::componentName()
     * returns. If that file does not exist, then the XML UI code will only use
     * the global (standard) XML file for the layout purposes.
     *
     * @param xmlfile the path (relative or absolute) to the local xmlfile
     */
    void createGUI(const QString &xmlfile = QString());

    /**
     * Sets whether @ref KMainWindow should provide a menu that allows
     * showing/hiding the available toolbars (using KToggleToolBarAction).
     *
     * If there is only one toolbar configured, a simple 'Show \<toolbar name\>'
     * menu item is shown; if more than one toolbar are configured, a "Shown Toolbars"
     * menu is created instead, with 'Show \<toolbar1 name\>', 'Show \<toolbar2 name\>'
     * ...etc sub-menu actions.
     *
     * The menu / menu item is implemented using xmlgui. It will be inserted in your
     * menu structure in the 'Settings' menu.
     *
     * If your application uses a non-standard xmlgui resource file then you can
     * specify the exact position of the menu / menu item by adding a
     * &lt;Merge name="StandardToolBarMenuHandler" /&gt;
     * line to the settings menu section of your resource file ( usually appname.rc ).
     *
     * @note You should enable this feature before calling createGUI() ( or similar ).
     */
    void setStandardToolBarMenuEnabled(bool enable);

    /**
     * Returns @c true if the toolbar menu is enabled, @c false otherwise.
     */
    bool isStandardToolBarMenuEnabled() const;

    /**
     * Sets whether KMainWindow should provide a menu that allows showing/hiding
     * of the statusbar (using KStandardAction::showStatusbar()). Note that calling
     * this method will create a statusbar if one doesn't already exist.
     *
     * The menu / menu item is implemented using xmlgui. It will be inserted
     * in your menu structure in the 'Settings' menu.
     *
     * @note You should enable this feature before calling createGUI() (or similar).
     *
     * If an application maintains the action on its own (i.e. never calls
     * this function) a connection needs to be made to let KMainWindow
     * know when that status (hidden/shown) of the statusbar has changed.
     * For example:
     * @code
     * connect(action, &QAction::triggered,
     *         kmainwindow, &KMainWindow::setSettingsDirty);
     * @endcode
     * Otherwise the status (hidden/show) of the statusbar might not be saved
     * by KMainWindow.
     */
    void createStandardStatusBarAction();

    /**
     * @see setupGUI()
     * @see StandardWindowOptions
     */
    enum StandardWindowOption {
        /**
         * Adds action(s) to show/hide the toolbar(s) and adds a menu
         * action to configure the toolbar(s).
         *
         * @see setStandardToolBarMenuEnabled
         */
        ToolBar = 1,

        /**
         * Adds an action to open the configure keyboard shortcuts dialog.
         */
        Keys = 2,

        /**
         * Adds action to show/hide the statusbar. Note that setting this
         * value will create a statusbar if one doesn't already exist.
         *
         * @see createStandardStatusBarAction
         */
        StatusBar = 4,

        /**
         * Auto-saves (and loads) the toolbar/menubar/statusbar settings and
         * window size using the default name.
         *
         * Typically you want to let the default window size be determined by
         * the widgets size hints. Make sure that setupGUI() is called after
         * all the widgets are created (including setCentralWidget) so that the
         * default size will be correct.
         *
         * @see setAutoSaveSettings
         */
        Save = 8,

        /**
         * calls createGUI() once ToolBar, Keys and Statusbar have been
         * taken care of.
         *
         * @note When using @ref KParts::MainWindow, remove this flag from
         * the @ref setupGUI() call, since you'll be using @c createGUI(part)
         * instead.
         *
         * @code
         *     setupGUI(ToolBar | Keys | StatusBar | Save);
         * @endcode
         *
         * @see createGUI
         */
        Create = 16,

        /**
         * All the above option
         * (this is the default)
         */
        Default = ToolBar | Keys | StatusBar | Save | Create,
    };
    Q_FLAG(StandardWindowOption)
    /**
     * Stores a combination of #StandardWindowOption values.
     */
    Q_DECLARE_FLAGS(StandardWindowOptions, StandardWindowOption)

    /**
     * Configures the current window and its actions in the typical KDE
     * fashion.
     *
     * You can specify which window options/features are going to be set up using
     * @p options, @see the @ref StandardWindowOption enum for more details.
     *
     * Typically this function replaces @ref createGUI().
     *
     * @warning If you are calling @ref createGUI() yourself, remember to
     * remove the @ref StandardWindowOption::Create flag from @p options.
     *
     * @see @ref StandardWindowOptions
     *
     * @note Since this method will restore the state of the application
     * window (toolbar, dockwindows positions ...etc), you need to have
     * added all your actions to your toolbars ...etc before calling this
     * method. (This note is only applicable if you are using the
     * @c StandardWindowOption::Default or @c StandardWindowOption::Save flags).
     *
     * @param options an OR'ed combination of StandardWindowOption to specify
     * which options are going to be set up for your application window, the
     * default is @ref StandardWindowOption::Default
     * @param xmlfile the path (relative or absolute) to the local xmlfile,
     * if this is an empty string the code will look for a local XML file
     * appnameui.rc, where 'appname' is the name of your app, see the note
     * about the xmlfile argument in the @ref createGUI() docs.
     */
    void setupGUI(StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /**
     * This is an overloaded method.
     *
     * You can use @p defaultSize to override the saved window size (e.g.
     * the window size is saved to the config file if the @ref StandardWindowOption::Save
     * flag was set previously).
     */
    void setupGUI(const QSize &defaultSize, StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /**
     * Returns a pointer to the mainwindows action responsible for the toolbars menu
     */
    QAction *toolBarMenuAction();

    /**
     * @internal for KToolBar
     */
    void setupToolbarMenuActions();

    // TODO KF6 change it to "using KXMLGUIBuilder::finalizeGUI;"
    void finalizeGUI(KXMLGUIClient *client) override;

    /**
     * @internal
     */
    void finalizeGUI(bool force);

    // reimplemented for internal reasons
    void applyMainWindowSettings(const KConfigGroup &config) override;

    /**
     * Enable a hud style menu which allows listing and executing actions
     *
     * The menu is launchable with the shortcut Alt+Ctrl+I
     *
     * @since 5.83
     */
    void setCommandBarEnabled(bool enable);

    /**
     * @since 5.83
     */
    bool isCommandBarEnabled() const;

public Q_SLOTS:
    /**
     * Show a standard configure toolbar dialog.
     *
     * This slot can be connected directly to the action to configure toolbar.
     * This is very simple to do that by adding a single line
     * \code
     * KStandardAction::configureToolbars( this, SLOT( configureToolbars() ),
     *                                actionCollection() );
     * \endcode
     */
    virtual void configureToolbars();

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file
     */
    virtual void slotStateChanged(const QString &newstate);

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file,
     * can "reverse" the state (disable the actions which should be
     * enabled, and vice-versa) if specified.
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
     * Checks if there are actions using the same shortcut. This is called
     * automatically from createGUI.
     * @since 5.30
     */
    void checkAmbiguousShortcuts();

protected Q_SLOTS:
    /**
     * Rebuilds the GUI after KEditToolBar changed the toolbar layout.
     * @see configureToolbars()
     */
    virtual void saveNewToolbarConfig();

private:
    Q_DECLARE_PRIVATE_D(k_ptr, KXmlGuiWindow)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KXmlGuiWindow::StandardWindowOptions)

#endif
