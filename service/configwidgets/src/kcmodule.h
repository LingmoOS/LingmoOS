/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCMODULE_H
#define KCMODULE_H

#include <kconfigwidgets_export.h>

#if KCONFIGWIDGETS_WITH_KAUTH
#include <KAuth/Action>
#endif

#include <QVariant>
#include <QWidget>
#include <memory>

class KAboutData;
class KConfigDialogManager;
class KCoreConfigSkeleton;
class KConfigSkeleton;
class KCModulePrivate;

/**
 * @class KCModule kcmodule.h KCModule
 *
 * The base class for configuration modules.
 *
 * Configuration modules are realized as plugins that are loaded only when
 * needed.
 *
 * The module in principle is a simple widget displaying the
 * item to be changed. The module has a very small interface.
 *
 * All the necessary glue logic and the GUI bells and whistles
 * are provided by the control center and must not concern
 * the module author.
 *
 * To write a config module, you have to create a library
 * that contains a factory function like the following:
 *
 * \code
 * #include <KPluginFactory>
 *
 * K_PLUGIN_FACTORY(MyKCModuleFactory, registerPlugin<MyKCModule>() )
 * \endcode
 *
 * The constructor of the KCModule then looks like this:
 * \code
 * YourKCModule::YourKCModule( QWidget* parent )
 *   : KCModule( parent )
 * {
 *   KAboutData *about = new KAboutData(
 *     <kcm name>, i18n( "..." ),
 *     KDE_VERSION_STRING, QString(), KAboutLicense::GPL,
 *     i18n( "Copyright 2006 ..." ) );
 *   about->addAuthor( i18n(...) );
 *   setAboutData( about );
 *   .
 *   .
 *   .
 * }
 * \endcode
 *
 * If you want to make the KCModule available only conditionally (i.e. show in
 * the list of available modules only if some test succeeds) then you can use
 * Hidden in the .desktop file. An example:
 * \code
 * Hidden[$e]=$(if test -e /dev/js*; then echo "false"; else echo "true"; fi)
 * \endcode
 * The example executes the given code in a shell and uses the stdout output for
 * the Hidden value (so it's either Hidden=true or Hidden=false).
 *
 * See http://techbase.kde.org/Development/Tutorials/KCM_HowTo
 * for more detailed documentation.
 *
 * @author Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 */
class KCONFIGWIDGETS_EXPORT KCModule : public QWidget
{
    Q_OBJECT

public:
    /**
     * An enumeration type for the buttons used by this module.
     * You should only use Help, Default and Apply. The rest is obsolete.
     * NoAdditionalButton can be used when we do not want have other button that Ok Cancel
     *
     * @see KCModule::buttons
     * @see KCModule::setButtons
     * @see Buttons
     */
    enum Button { NoAdditionalButton = 0, Help = 1, Default = 2, Apply = 4, Export = 8 };
    /**
     * Stores a combination of #Button values.
     */
    Q_DECLARE_FLAGS(Buttons, Button)

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 85)
    /**
     * Base class for all KControlModules.
     *
     * @note do not emit changed signals here, since they are not yet connected
     *       to any slot.
     * @param aboutData becomes owned by the KCModule
     * @deprecated since 5.85, use other constructor and setAboutData()
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 85, "Use other constructor and setAboutData()")
    explicit KCModule(const KAboutData *aboutData, QWidget *parent = nullptr, const QVariantList &args = QVariantList());
#endif

    /**
     * Base class for all KControlModules.
     *
     * @note do not emit changed signals here, since they are not yet connected
     *       to any slot.
     */
    explicit KCModule(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    /**
     * Destroys the module.
     */
    ~KCModule() override;

    /**
     * Return a quick-help text.
     *
     * This method may be called when the module is docked. The quick help would generally
     * be used as "what's this" text if the view container supports the "what's this"
     * system.
     * The quick-help text should contain a short description of the module and
     * links to the module's help files. You can use QML formatting tags in the text.
     *
     * @note make sure the quick help text gets translated (use i18n()).
     */
    virtual QString quickHelp() const;

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 90)
    /**
     * This is generally only called for the KBugReport.
     * If you override you should  have it return a pointer to a constant.
     *
     *
     * @returns the KAboutData for this module
     * @deprecated since 5.90. Use the KPluginMetaData the KCModule was instantiated from.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 90, "Use the KPluginMetaData the KCModule was instantiated from")
    virtual const KAboutData *aboutData() const;
#endif

    /**
     * This sets the KAboutData returned by aboutData()
     * The about data is now owned by KCModule.
     */
    void setAboutData(const KAboutData *about);

    /**
     * Indicate which buttons will be used.
     *
     * The return value is a value or'ed together from
     * the Button enumeration type.
     *
     * @see KCModule::setButtons
     */
    Buttons buttons() const;

    /**
     * Get the RootOnly message for this module.
     *
     * When the module must be run as root, or acts differently
     * for root and a normal user, it is sometimes useful to
     * customize the message that appears at the top of the module
     * when used as a normal user. This function returns this
     * customized message. If none has been set, a default message
     * will be used.
     *
     * @see KCModule::setRootOnlyMessage
     */
    QString rootOnlyMessage() const;

    /**
     * Tell if KControl should show a RootOnly message when run as
     * a normal user.
     *
     * In some cases, the module don't want a RootOnly message to
     * appear (for example if it has already one). This function
     * tells KControl if a RootOnly message should be shown
     *
     * @see KCModule::setUseRootOnlyMessage
     */
    bool useRootOnlyMessage() const;

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 90)
    /**
     * @deprecated since 5.90. Use the KPluginMetaData the KCModule was instantiated from.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 90, "Use the KPluginMetaData the KCModule was instantiated from")
    KAboutData componentData() const;
#endif

    /**
     * @return a list of @ref KConfigDialogManager's in use, if any.
     */
    QList<KConfigDialogManager *> configs() const;

    /**
     * @brief Set if the module's save() method requires authorization to be executed.
     *
     * The module can set this property to @c true if it requires authorization.
     * It will still have to execute the action itself using the KAuth library, so
     * this method is not technically needed to perform the action, but
     * using this and/or the setAuthAction() method will ensure that hosting
     * applications like System Settings or kcmshell behave correctly.
     *
     * Called with @c true, this method will set the action to  "org.kde.kcontrol.name.save" where
     * "name" is aboutData()->appName() return value. This default action won't be set if
     * the aboutData() object is not valid.
     *
     * Note that called with @c false, this method will reset the action name set with setAuthAction().
     *
     * @param needsAuth Tells if the module's save() method requires authorization to be executed.
     */
    void setNeedsAuthorization(bool needsAuth);

    /**
     * Returns the value previously set with setNeedsAuthorization() or setAuthAction(). By default it's @c false.
     *
     * @return @c true if the module's save() method requires authorization, @c false otherwise
     */
    bool needsAuthorization() const;

    /**
     * Returns whether an indicator is shown when a setting differs from default.
     *
     * @since 5.73
     */
    bool defaultsIndicatorsVisible() const;

#if KCONFIGWIDGETS_WITH_KAUTH
    /**
     * @brief Set if the module's save() method requires authorization to be executed
     *
     * It will still have to execute the action itself using the KAuth library, so
     * this method is not technically needed to perform the action, but
     * using this method will ensure that hosting
     * applications like System Settings or kcmshell behave correctly.
     *
     * @param action the action that will be used by this KCModule
     */
    void setAuthAction(const KAuth::Action &action);

    /**
     * Returns the action previously set with setAuthAction(). By default its an invalid action.
     *
     * @return The action that has to be authorized to execute the save() method.
     */
    KAuth::Action authAction() const;
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Returns the value set by setExportText();
     * @deprecated since 5.0, obsolete feature
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 0, "Obsolete feature")
    QString exportText() const;
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 0)
    /**
     * Sets the export QString value, used for exporting data.
     * @deprecated since 5.0, obsolete feature
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 0, "Obsolete feature")
    void setExportText(const QString &);
#endif

public Q_SLOTS:
    /**
     * Load the configuration data into the module.
     *
     * The load method sets the user interface elements of the
     * module to reflect the current settings stored in the
     * configuration files.
     *
     * This method is invoked whenever the module should read its configuration
     * (most of the times from a config file) and update the user interface.
     * This happens when the user clicks the "Reset" button in the control
     * center, to undo all of his changes and restore the currently valid
     * settings. It is also called right after construction.
     */
    virtual void load();

    /**
     * Save the configuration data.
     *
     * The save method stores the config information as shown
     * in the user interface in the config files.
     *
     * If necessary, this method also updates the running system,
     * e.g. by restarting applications. This normally does not apply for
     * KSettings::Dialog modules where the updating is taken care of by
     * KSettings::Dispatcher.
     *
     * save is called when the user clicks "Apply" or "Ok".
     *
     * If you use KConfigXT, saving is taken care off automatically and
     * you do not need to load manually. However, if you for some reason reimplement it and
     * also are using KConfigXT, you must call this function, otherwise the saving of KConfigXT
     * options will not work. Call it at the very end of your reimplementation, to avoid
     * changed() signals getting emitted when you modify widgets.
     */
    virtual void save();

    /**
     * Sets the configuration to sensible default values.
     *
     * This method is called when the user clicks the "Default"
     * button. It should set the display to useful values.
     *
     * If you use KConfigXT, you do not have to reimplement this function since
     * the fetching and settings of default values is done automatically. However, if you
     * reimplement and also are using KConfigXT, remember to call the base function at the
     * very end of your reimplementation.
     */
    virtual void defaults();

    /**
     * Show an indicator when settings value differ from default
     *
     * @since 5.73
     */
    void setDefaultsIndicatorsVisible(bool show);

protected:
    /**
     * Adds a KCoreConfigskeleton @p config to watch the widget @p widget
     *
     * This function is useful if you need to handle multiple configuration files.
     *
     * @return a pointer to the KCoreConfigDialogManager in use
     * @param config the KCoreConfigSkeleton to use
     * @param widget the widget to watch
     */
    KConfigDialogManager *addConfig(KCoreConfigSkeleton *config, QWidget *widget);

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 84)
    // No deprecation warning by compiler here, as the replacement will be
    // automatically picked by the compiler in the future, being the method
    // overload using the base-class of the argument type.
    // Avoids the need to do extra-casting right now on the caller side.
    /**
     * Adds a KConfigskeleton @p config to watch the widget @p widget
     *
     * This function is useful if you need to handle multiple configuration files.
     *
     * @return a pointer to the KConfigDialogManager in use
     * @param config the KConfigSkeleton to use
     * @param widget the widget to watch
     * @deprecated since 5.84, use addConfig(KCoreConfigSkeleton *config, QWidget *widget);
     */
    KConfigDialogManager *addConfig(KConfigSkeleton *config, QWidget *widget);
#endif

    /**
     * Sets the quick help.
     */
    void setQuickHelp(const QString &help);

    void showEvent(QShowEvent *ev) override;

    friend class KCModuleProxy;

Q_SIGNALS:

    /**
     * Indicate that the state of the modules contents has changed.
     *
     * This signal is emitted whenever the state of the configuration
     * shown in the module changes. It allows the module container to
     * keep track of unsaved changes.
     */
    void changed(bool state); // clazy:exclude=overloaded-signal

    /**
     * Indicate that the state of the modules contents matches the default
     * settings.
     *
     * This signal is emitted whenever the state of the configuration
     * shown in the module changes. It allows the module container to
     * keep track of defaults.
     *
     * @since 5.65
     */
    void defaulted(bool state);

    /**
     * Indicate that the module's quickhelp has changed.
     *
     * Emit this signal whenever the module's quickhelp changes.
     * Modules implemented as tabbed dialogs might want to implement
     * per-tab quickhelp for example.
     *
     */
    void quickHelpChanged();

    /**
     * Indicate that the module's root message has changed.
     *
     * Emits this signal whenever the module's root message changes.
     *
     * @since 4.4
     *
     */
    void rootOnlyMessageChanged(bool use, QString message);

    /**
     * Emitted when show defaults indicators changed
     * @since 5.73
     */
    void defaultsIndicatorsVisibleChanged(bool show);

protected Q_SLOTS:

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 64)
    /**
     * Calling this slot is equivalent to emitting changed(true).
     * @deprecated Since 5.64, use markAsChanged
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 64, "Use KCModule::markAsChanged()")
    void changed();
#endif

    /**
     * Calling this slot is equivalent to emitting changed(true).
     * @since 5.64
     */
    void markAsChanged();

    /**
     * A managed widget was changed, the widget settings and the current
     * settings are compared and a corresponding changed() signal is emitted
     */
    void widgetChanged();

#if KCONFIGWIDGETS_WITH_KAUTH
    /**
     * The status of the auth action, if one, has changed
     */
    void authStatusChanged(KAuth::Action::AuthStatus status);
#endif

protected:
    /**
     * Sets the buttons to display.
     *
     * Help: shows a "Help" button.
     *
     * Default: shows a "Use Defaults" button.
     *
     * Apply: in kcontrol this will show an "Apply" and "Reset" button,
     *        in kcmshell this will show an "Ok", "Apply", "Reset" and "Cancel" button.
     *
     * If Apply is not specified, kcmshell will show a "Close" button.
     *
     * @see KCModule::buttons
     */
    void setButtons(Buttons btn);

    /**
     * Sets the RootOnly message.
     *
     * This message will be shown at the top of the module if useRootOnlyMessage is
     * set. If no message is set, a default one will be used.
     *
     * @see KCModule::rootOnlyMessage
     */
    void setRootOnlyMessage(const QString &message);

    /**
     * Change whether or not the RootOnly message should be shown.
     *
     * Following the value of @p on, the RootOnly message will be
     * shown or not.
     *
     * @see KCModule::useRootOnlyMessage
     */
    void setUseRootOnlyMessage(bool on);

    /**
     * Returns the changed state of automatically managed widgets in this dialog
     */
    bool managedWidgetChangeState() const;

    /**
     * Returns the defaulted state of automatically managed widgets in this dialog
     *
     * @since 5.65
     */
    bool managedWidgetDefaultState() const;

    /**
     * Call this method when your manually managed widgets change state between
     * changed and not changed
     */
    void unmanagedWidgetChangeState(bool);

    /**
     * Call this method when your manually managed widgets change state between
     * defaulted and not defaulted
     *
     * @since 5.65
     */
    void unmanagedWidgetDefaultState(bool);

private:
    std::unique_ptr<KCModulePrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KCModule::Buttons)

#endif // KCMODULE_H
