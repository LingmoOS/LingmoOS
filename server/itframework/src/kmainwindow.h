/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KMAINWINDOW_H
#define KMAINWINDOW_H

#include <kxmlgui_export.h>

#include <QMainWindow>
#include <memory>

class QMenu;
class KConfig;
class KConfigGroup;
class KMWSessionManager;
class KMainWindowPrivate;
class KToolBar;

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 65)
/**
 * @relates KMainWindow
 * @deprecated Since 5.65, use Qt::WindowFlags()
 */
#define KDE_DEFAULT_WINDOWFLAGS 0
#else
#define KDE_DEFAULT_WINDOWFLAGS KDE_DEFAULT_WINDOWFLAGS_is_deprecated_use_Qt_WindowFlags()
#endif

/**
 * @class KMainWindow kmainwindow.h KMainWindow
 *
 * @short Top level main window
 *
 * Top level widget that provides toolbars, a status line and a frame.
 *
 * It should be used as a top level (parent-less) widget.
 * It manages the geometry for all its children, including your
 * main widget.
 *
 * Normally, you will inherit from KMainWindow,
 * then construct (or use some existing) widget as
 * your main view. You can set only one main view.
 *
 * You can add as many toolbars as you like. There can be only one menubar
 * and only one statusbar.
 *
 * The toolbars, menubar, and statusbar can be created by the
 * KMainWindow and - unlike the old KMainWindow - may, but do not
 * have to, be deleted by you. KMainWindow will handle that internally.
 *
 * Height and width can be operated independently from each other. Simply
 * define the minimum/maximum height/width of your main widget and
 * KMainWindow will take this into account. For fixed size windows set
 * your main widget to a fixed size.
 *
 * Fixed aspect ratios (heightForWidth()) and fixed width widgets are
 * not supported.
 *
 * KMainWindow will set icon, mini icon and caption, which it gets
 * from KApplication. It provides full session management, and
 * will save its position, geometry and positions of toolbars and
 * menubar on logout. If you want to save additional data, reimplement
 * saveProperties() and (to read them again on next login)
 * readProperties(). To save special data about your data, reimplement
 * saveGlobalProperties(). To warn user that application or
 * windows have unsaved data on close or logout, reimplement
 * queryClose().
 *
 * You have to implement session restoring also in your main() function.
 * There are also kRestoreMainWindows convenience functions which
 * can do this for you and restore all your windows on next login.
 *
 * Note that KMainWindow uses KGlobal::ref() and KGlobal::deref() so that closing
 * the last mainwindow will quit the application unless there is still something
 * that holds a ref in KGlobal - like a KIO job, or a systray icon.
 *
 * @author Reginald Stadlbauer (reggie@kde.org)
 * @author Stephan Kulow (coolo@kde.org)
 * @author Matthias Ettrich (ettrich@kde.org)
 * @author Chris Schlaeger (cs@kde.org)
 * @author Sven Radej (radej@kde.org)
 * Maintained by David Faure (faure@kde.org)
 */

class KXMLGUI_EXPORT KMainWindow : public QMainWindow
{
    friend class KMWSessionManager;
    friend class DockResizeListener;
    Q_OBJECT
    Q_PROPERTY(bool hasMenuBar READ hasMenuBar)
    Q_PROPERTY(bool autoSaveSettings READ autoSaveSettings)
    Q_PROPERTY(QString autoSaveGroup READ autoSaveGroup)

public:
    /**
     * Construct a main window.
     *
     * @param parent The widget parent. This is usually 0 but it may also be the window
     * group leader. In that case, the KMainWindow becomes sort of a
     * secondary window.
     *
     * @param f Specify the window flags. The default is none.
     *
     * Note that a KMainWindow per-default is created with the
     * Qt::WA_DeleteOnClose attribute set, i.e. it is automatically destroyed
     * when the window is closed. If you do not want this behavior, call
     * \code
     * window->setAttribute(Qt::WA_DeleteOnClose, false);
     * \endcode
     *
     * KMainWindows must be created on the heap with 'new', like:
     * \code
     * KMainWindow *kmw = new KMainWindow(...);
     * kmw->setObjectName(...);
     * \endcode
     *
     * Since the KDE Frameworks 5.16 release, KMainWindow will also enter information regarding
     * the application's translators by default, using KAboutData::setTranslator(). This only occurs
     * if no translators are already assigned in KAboutData (see KAboutData::setTranslator() for
     * details -- the auto-assignment here uses the same translated strings as specified for that
     * function).
     *
     * IMPORTANT: For session management and window management to work
     * properly, all main windows in the application should have a
     * different name. If you don't do it, KMainWindow will create
     * a unique name, but it's recommended to explicitly pass a window name that will
     * also describe the type of the window. If there can be several windows of the same
     * type, append '#' (hash) to the name, and KMainWindow will replace it with numbers to make
     * the names unique. For example, for a mail client which has one main window showing
     * the mails and folders, and which can also have one or more windows for composing
     * mails, the name for the folders window should be e.g. "mainwindow" and
     * for the composer windows "composer#".
     *
     */
    explicit KMainWindow(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /**
     * \brief Destructor.
     *
     * Will also destroy the toolbars, and menubar if
     * needed.
     */
    ~KMainWindow() override;

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Retrieve the standard help menu.
     *
     * It contains entries for the
     * help system (activated by F1), an optional "What's This?" entry
     * (activated by Shift F1), an application specific dialog box,
     * and an "About KDE" dialog box.
     *
     * Example (adding a standard help menu to your application):
     * \code
     * QMenu *help = helpMenu( <myTextString> );
     * menuBar()->addMenu( help );
     * \endcode
     *
     * @param aboutAppText The string that is used in the application
     *        specific dialog box. If you leave this string empty the
     *        information in the global KAboutData of the
     *        application will be used to make a standard dialog box.
     *
     * @param showWhatsThis Set this to false if you do not want to include
     *        the "What's This" menu entry.
     *
     * @return A standard help menu.
     * @deprecated Since 5.0, use KHelpMenu directly
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KHelpMenu directly")
    QMenu *helpMenu(const QString &aboutAppText = QString(), bool showWhatsThis = true);
#endif

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Returns the help menu. Creates a standard help menu if none exists yet.
     *
     * It contains entries for the
     * help system (activated by F1), an optional "What's This?" entry
     * (activated by Shift F1), an application specific dialog box,
     * and an "About KDE" dialog box. You must create the application
     * specific dialog box yourself. When the "About application"
     * menu entry is activated, a signal will trigger the
     * showAboutApplication slot. See showAboutApplication for more
     * information.
     *
     * Example (adding a help menu to your application):
     * \code
     * menuBar()->addMenu( customHelpMenu() );
     * \endcode
     *
     * @param showWhatsThis Set this to @c false if you do not want to include
     *        the "What's This" menu entry.
     *
     * @return A standard help menu.
     * @deprecated Since 5.0, use XMLGUI instead, or KHelpMenu directly
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use XMLGUI or KHelpMenu")
    QMenu *customHelpMenu(bool showWhatsThis = true);
#endif

    /**
     * If the session did contain so high a number, @c true is returned,
     * else @c false.
     * @see restore()
     **/
    static bool canBeRestored(int number);

    /**
     * Returns the className() of the @p number of the toplevel window which
     * should be restored.
     *
     * This is only useful if your application uses
     * different kinds of toplevel windows.
     */
    static const QString classNameOfToplevel(int number);

    /**
     * Try to restore the toplevel widget as defined by @p number (1..X).
     *
     * You should call canBeRestored() first.
     *
     * If the session did not contain so high a number, the configuration
     * is not changed and @c false returned.
     *
     * That means clients could simply do the following:
     * \code
     * if (qApp->isSessionRestored()){
     *   int n = 1;
     *   while (KMainWindow::canBeRestored(n)){
     *     (new childMW)->restore(n);
     *     n++;
     *   }
     * } else {
     *   // create default application as usual
     * }
     * \endcode
     * Note that if @p show is @c true (default), QWidget::show() is called
     * implicitly in restore.
     *
     * With this you can easily restore all toplevel windows of your
     * application.
     *
     * If your application uses different kinds of toplevel
     * windows, then you can use KMainWindow::classNameOfToplevel(n)
     * to determine the exact type before calling the childMW
     * constructor in the example from above.
     *
     * @note You don't need to deal with this function. Use the
     * kRestoreMainWindows() convenience template function instead!
     * @see kRestoreMainWindows()
     * @see readProperties()
     * @see canBeRestored()
     */
    bool restore(int number, bool show = true);

    /**
     * Returns true, if there is a menubar
     */
    bool hasMenuBar();

    /**
     * List of members of KMainWindow class.
     */
    static QList<KMainWindow *> memberList();

    /**
     * Returns a pointer to the toolbar with the specified name.
     * This refers to toolbars created dynamically from the XML UI
     * framework.  If the toolbar does not exist one will be created.
     *
     * @param name The internal name of the toolbar. If no name is
     *             specified "mainToolBar" is assumed.
     *
     * @return A pointer to the toolbar
     **/
    KToolBar *toolBar(const QString &name = QString());

    /**
     * @return A list of all toolbars for this window
     */
    QList<KToolBar *> toolBars() const;

    /**
     * Call this to enable "auto-save" of toolbar/menubar/statusbar settings
     * (and optionally window size).
     * If the *bars were moved around/shown/hidden when the window is closed,
     * saveMainWindowSettings( KConfigGroup(KSharedConfig::openConfig(), groupName) ) will be called.
     *
     * @param groupName a name that identifies this "type of window".
     * You can have several types of window in the same application.
     *
     * @param saveWindowSize set it to true to include the window size
     * when saving.
     *
     * Typically, you will call setAutoSaveSettings() in your
     * KMainWindow-inherited class constructor, and it will take care
     * of restoring and saving automatically. Make sure you call this
     * _after all_ your *bars have been created.
     *
     * To make sure that KMainWindow properly obtains the default
     * size of the window you should do the following:
     * - Remove hard coded resize() calls in the constructor or main, they
     *   should be removed in favor of letting the automatic resizing
     *   determine the default window size.  Hard coded window sizes will
     *   be wrong for users that have big fonts, use different styles,
     *   long/small translations, large toolbars, and other factors.
     * - Put the setAutoSaveSettings() ( or setupGUI() ) call after all widgets
     *   have been created and placed inside the main window (i.e. for 99% of
     *   apps setCentralWidget())
     * - Widgets that inherit from QWidget (like game boards) should overload
     *   "virtual QSize sizeHint() const;" to specify a default size rather
     *   than letting QWidget::adjust use the default size of 0x0.
     */
    void setAutoSaveSettings(const QString &groupName = QStringLiteral("MainWindow"), bool saveWindowSize = true);

    /**
     * Overload that lets you specify a KConfigGroup.
     * This allows the settings to be saved into another file than KSharedConfig::openConfig().
     * @since 4.1
     */
    void setAutoSaveSettings(const KConfigGroup &group, bool saveWindowSize = true);

    /**
     * Disable the auto-save-settings feature.
     * You don't normally need to call this, ever.
     */
    void resetAutoSaveSettings();

    /**
     * @return the current autosave setting, i.e. true if setAutoSaveSettings() was called,
     * false by default or if resetAutoSaveSettings() was called.
     */
    bool autoSaveSettings() const;

    /**
     * @return the group used for setting-autosaving.
     * Only meaningful if setAutoSaveSettings(const QString&, bool) was called.
     * This can be useful for forcing a save or an apply, e.g. before and after
     * using KEditToolBar.
     *
     * @note You should rather use saveAutoSaveSettings() for saving or autoSaveConfigGroup() for loading.
     * This method doesn't make sense if setAutoSaveSettings(const KConfigGroup&, bool) was called.
     */
    QString autoSaveGroup() const;

    /**
     * @return the group used for setting-autosaving.
     * Only meaningful if setAutoSaveSettings() was called.
     * This can be useful for forcing an apply, e.g. after using KEditToolBar.
     * @since 4.1
     */
    KConfigGroup autoSaveConfigGroup() const;

    /**
     * Set the config group name for state config returned by @p stateConfigGroup
     * If this method is called the window size and window state are stored in the resulting KConfigGroup.
     * @note If this is used in combination with @p setAutoSaveSettings, you should call this method first
     *
     * @see KSharedConfig::openStateConfig
     *
     * @since 5.88
     */
    void setStateConfigGroup(const QString &configGroup);

    /*
     * Returns the KConfigGroup used to store state data like window sizes or window state.
     * The resulting group in invalid if @p setStateConfig is not called explicitly.
     * @since 5.88
     */
    KConfigGroup stateConfigGroup() const;

    /**
     * Read settings for statusbar, menubar and toolbar from their respective
     * groups in the config file and apply them.
     *
     * @param config Config group to read the settings from.
     *
     * KF5 porting note: the unused bool argument was removed, make sure to remove it from your
     * reimplementations too! And add a override for good measure.
     */
    virtual void applyMainWindowSettings(const KConfigGroup &config);

    /**
     * Save settings for statusbar, menubar and toolbar to their respective
     * groups in the config group @p config.
     *
     * @param config Config group to save the settings to.
     */
    void saveMainWindowSettings(KConfigGroup &config);

    /**
     * Returns the path under which this window's D-Bus object is exported.
     * @since 4.0.1
     */
    QString dbusName() const;

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * @returns Always @c false
     * @deprecated since 5.0, the functionality got removed
     **/
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Remove usage, is a no-op now")
    bool initialGeometrySet() const
    {
        return false;
    }
#endif

public Q_SLOTS:
    /**
     * Makes a KDE compliant caption (window title).
     *
     * @param caption Your caption. @em Do @em not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption(const QString &caption);
    /**
     * Makes a KDE compliant caption.
     *
     * @param caption Your caption. @em Do @em not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     * @param modified Specify whether the document is modified. This displays
     * an additional sign in the title bar, usually "**".
     */
    virtual void setCaption(const QString &caption, bool modified);

    /**
     * Make a plain caption without any modifications.
     *
     * @param caption Your caption. This is the string that will be
     * displayed in the window title.
     */
    virtual void setPlainCaption(const QString &caption);

    /**
     * Open the help page for the application.
     *
     *  The application name is
     * used as a key to determine what to display and the system will attempt
     * to open \<appName\>/index.html.
     *
     * This method is intended for use by a help button in the toolbar or
     * components outside the regular help menu. Use helpMenu() when you
     * want to provide access to the help system from the help menu.
     *
     * Example (adding a help button to the first toolbar):
     *
     * \code
     * toolBar(0)->addAction(QIcon::fromTheme("help-contents"), i18n("Help"),
     *                       this, SLOT(appHelpActivated()));
     * \endcode
     *
     */
    void appHelpActivated();

    /**
     * Tell the main window that it should save its settings when being closed.
     * This is part of the auto-save-settings feature.
     * For everything related to toolbars this happens automatically,
     * but you have to call setSettingsDirty() in the slot that toggles
     * the visibility of the statusbar.
     */
    void setSettingsDirty();

protected:
    /**
     * Reimplemented to catch QEvent::Polish in order to adjust the object name
     * if needed, once all constructor code for the main window has run.
     * Also reimplemented to catch when a QDockWidget is added or removed.
     */
    bool event(QEvent *event) override;

    /**
     * Reimplemented to autosave settings and call queryClose().
     *
     * We recommend that you reimplement queryClose() rather than closeEvent().
     * If you do it anyway, ensure to call the base implementation to keep
     * the feature of auto-saving window settings working.
     */
    void closeEvent(QCloseEvent *) override;

    /**
       Called before the window is closed, either by the user or indirectly by
       the session manager.

       The purpose of this function is to prepare the window in a way that it is
       safe to close it, i.e. without the user losing some data.

       Default implementation returns true. Returning @c false will cancel
       the closing, and, if KApplication::sessionSaving() is true, it will also
       cancel KDE logout.

       Reimplement this function to prevent the user from losing data.
       Example:
       \code
       switch ( KMessageBox::warningTwoActionsCancel( this,
                i18n("Save changes to document foo?"), QString(),
                KStandardGuiItem::save(), KStandardGuiItem::discard())) ) {
       case KMessageBox::PrimaryAction :
         // save document here. If saving fails, return false;
         return true;
       case KMessageBox::SecondaryAction :
         return true;
       default: // cancel
         return false;
       \endcode

       Note that you should probably @em not actually close the document from
       within this method, as it may be called by the session manager before the
       session is saved. If the document is closed before the session save occurs,
       its location might not be properly saved. In addition, the session shutdown
       may be canceled, in which case the document should remain open.

       @see KApplication::sessionSaving()
    */
    virtual bool queryClose();

    /**
     * Save your instance-specific properties. The function is
     * invoked when the session manager requests your application
     * to save its state.
     *
     * Please reimplement these function in childclasses.
     *
     * Note: No user interaction is allowed
     * in this function!
     *
     */
    virtual void saveProperties(KConfigGroup &)
    {
    }

    /**
     * Read your instance-specific properties.
     *
     * Is called indirectly by restore().
     */
    virtual void readProperties(const KConfigGroup &)
    {
    }

    /**
      * Save your application-wide properties. The function is
      * invoked when the session manager requests your application
      * to save its state.
      *
      * This function is similar to saveProperties() but is only called for
      * the very first main window, regardless how many main window are open.

      * Override it if you need to save other data about your documents on
      * session end. sessionConfig is a config to which that data should be
      * saved. Normally, you don't need this function. But if you want to save
      * data about your documents that are not in opened windows you might need
      * it.
      *
      * Default implementation does nothing.
      */
    virtual void saveGlobalProperties(KConfig *sessionConfig);

    /**
     * The counterpart of saveGlobalProperties().
     *
     * Read the application-specific properties in again.
     */
    virtual void readGlobalProperties(KConfig *sessionConfig);
    void savePropertiesInternal(KConfig *, int);
    bool readPropertiesInternal(KConfig *, int);

    /**
     * For inherited classes
     */
    bool settingsDirty() const;
#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * For inherited classes
     * @deprecated Since 5.0, use KWindowConfig::saveWindowSize
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KWindowConfig::saveWindowSize(...)")
    void saveWindowSize(KConfigGroup &config) const;
#endif

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * For inherited classes
     * @deprecated Since 5.0, use KWindowConfig::restoreWindowSize
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KWindowConfig::restoreWindowSize(...)")
    void restoreWindowSize(const KConfigGroup &config);
#endif

protected Q_SLOTS:
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
    /**
     * This slot does nothing.
     *
     * It must be reimplemented if you want
     * to use a custom About Application dialog box. This slot is
     * connected to the About Application entry in the menu returned
     * by customHelpMenu.
     *
     * Example:
     * \code
     *
     * void MyMainLevel::setupInterface()
     * {
     *   ..
     *   menuBar()->addMenu( customHelpMenu() );
     *   ..
     * }
     *
     * void MyMainLevel::showAboutApplication()
     * {
     *   <activate your custom dialog>
     * }
     * \endcode
     * @deprecated Since 5.0, use KHelpMenu
     */
    KXMLGUI_DEPRECATED_VERSION(5, 0, "Use KHelpMenu")
    virtual void showAboutApplication()
    {
    }
#endif

    /**
     * This slot should only be called in case you reimplement closeEvent() and
     * if you are using the "auto-save" feature. In all other cases,
     * setSettingsDirty() should be called instead to benefit from the delayed
     * saving.
     *
     * @see setAutoSaveSettings
     * @see setSettingsDirty
     *
     * Example:
     * \code
     *
     * void MyMainWindow::closeEvent( QCloseEvent *e )
     * {
     *   // Save settings if auto-save is enabled, and settings have changed
     *   if ( settingsDirty() && autoSaveSettings() )
     *     saveAutoSaveSettings();
     *   ..
     * }
     * \endcode
     */
    void saveAutoSaveSettings();

protected:
    KMainWindow(KMainWindowPrivate &dd, QWidget *parent, Qt::WindowFlags f);

    std::unique_ptr<KMainWindowPrivate> const k_ptr;
    // KF6 TODO: change k_ptr to d_ptr, use normal Q_DECLARE_PRIVATE

private:
    Q_DECLARE_PRIVATE_D(k_ptr, KMainWindow)

    Q_PRIVATE_SLOT(d_func(), void _k_slotSettingsChanged(int))
    Q_PRIVATE_SLOT(d_func(), void _k_slotSaveAutoSaveSize())
    Q_PRIVATE_SLOT(d_func(), void _k_slotSaveAutoSavePosition())
};

/**
 * @defgroup KXMLGUI_Session KXMLGUI Session Macros and Functions
 *
 * @{
 */

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(5, 0)
/**
 * @def RESTORE
 * Restores the last session.
 *
 * @deprecated since 5.0, use kRestoreMainWindows() instead
 **/
#define RESTORE(type)                                                                                                                                          \
    {                                                                                                                                                          \
        int n = 1;                                                                                                                                             \
        while (KMainWindow::canBeRestored(n)) {                                                                                                                \
            (new type)->restore(n);                                                                                                                            \
            n++;                                                                                                                                               \
        }                                                                                                                                                      \
    }
#endif

/**
 * @def KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS
 * Returns the maximal number of arguments that are actually
 * supported by kRestoreMainWindows().
 **/
#define KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS 3

/**
 * Restores the last session. (To be used in your main function).
 *
 * These functions work also if you have more than one kind of toplevel
 * widget (each derived from KMainWindow, of course).
 *
 * Imagine you have three kinds of toplevel widgets: the classes @c childMW1,
 * @c childMW2 and @c childMW3. Then you can just do:
 *
 * \code
 * int main(int argc, char *argv[])
 * {
 *     // [...]
 *     if (qApp->isSessionRestored())
 *         kRestoreMainWindows<childMW1, childMW2, childMW3>();
 *     else {
 *       // create default application as usual
 *     }
 *     // [...]
 * }
 * \endcode
 *
 * kRestoreMainWindows<>() will create (on the heap) as many instances
 * of your main windows as have existed in the last session and
 * call KMainWindow::restore() with the correct arguments. Note that
 * also QWidget::show() is called implicitly.
 *
 * Currently, these functions are provided for up to three
 * template arguments. If you need more, tell us. To help you in
 * deciding whether or not you can use kRestoreMainWindows, a
 * define #KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS is provided.
 *
 * @tparam T toplevel widget class
 *
 * @see KMainWindow::restore()
 * @see KMainWindow::classNameOfToplevel()
 **/
template<typename T>
inline void kRestoreMainWindows()
{
    for (int n = 1; KMainWindow::canBeRestored(n); ++n) {
        const QString className = KMainWindow::classNameOfToplevel(n);
        if (className == QLatin1String(T::staticMetaObject.className())) {
            (new T)->restore(n);
        }
    }
}

/**
 * Restores the last session.
 * Overloaded method for usage with multiple different toplevel widget classes.
 *
 * @tparam T0 one toplevel widget class
 * @tparam T1 explicit other toplevel widget class for disambiguation from base template
 * @tparam Tn Parameter pack to take 0..n further KMainWindows
 */
template<typename T0, typename T1, typename... Tn>
inline void kRestoreMainWindows()
{
    kRestoreMainWindows<T0>();
    kRestoreMainWindows<T1, Tn...>();
}
/** @}  */

#endif
