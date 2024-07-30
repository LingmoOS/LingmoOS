/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KABSTRACTCONFIGMODULE_H
#define KABSTRACTCONFIGMODULE_H

#include "kcmutilscore_export.h"

#include <QObject>

#include <memory>

class KPluginMetaData;
class KAbstractConfigModulePrivate;

/**
 * Base class for QML and QWidgets config modules.
 *
 * @author Alexander Lohnau
 * @since 6.0
 */
class KCMUTILSCORE_EXPORT KAbstractConfigModule : public QObject
{
    Q_OBJECT

    Q_PROPERTY(KAbstractConfigModule::Buttons buttons READ buttons WRITE setButtons NOTIFY buttonsChanged)
    Q_PROPERTY(bool defaultsIndicatorsVisible READ defaultsIndicatorsVisible WRITE setDefaultsIndicatorsVisible NOTIFY defaultsIndicatorsVisibleChanged)
    Q_PROPERTY(bool needsAuthorization READ needsAuthorization NOTIFY authActionNameChanged)
    Q_PROPERTY(bool representsDefaults READ representsDefaults WRITE setRepresentsDefaults NOTIFY representsDefaultsChanged)
    Q_PROPERTY(bool needsSave READ needsSave WRITE setNeedsSave NOTIFY needsSaveChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
public:
    /**
     * An enumeration type for the buttons used by this module.
     * You should only use Help, Default and Apply. The rest is obsolete.
     * NoAdditionalButton can be used when we do not want have other button that Ok Cancel
     *
     * @see ConfigModule::buttons @see ConfigModule::setButtons
     */
    enum Button {
        NoAdditionalButton = 0,
        Help = 1,
        Default = 2,
        Apply = 4,
        Export = 8,
    };
    Q_ENUM(Button)
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_FLAG(Buttons)

    explicit KAbstractConfigModule(QObject *parent, const KPluginMetaData &metaData);

    ~KAbstractConfigModule() override;

    /**
     * @brief Set if the module's save() method requires authorization to be executed
     *
     * It will still have to execute the action itself using the KAuth library, so
     * this method is not technically needed to perform the action, but
     * using this method will ensure that hosting
     * applications like System Settings or kcmshell behave correctly.
     *
     * @param action the action that will be used by this ConfigModule
     */
    void setAuthActionName(const QString &action);

    /**
     * Returns the action previously set with setAuthActionName(). By default this will be a empty string.
     *
     * @return The action that has to be authorized to execute the save() method.
     */
    QString authActionName() const;

    /**
     * The auth action name has changed
     */
    Q_SIGNAL void authActionNameChanged();

    /**
     * Set this property to true when the user changes something in the module,
     * signaling that a save (such as user pressing Ok or Apply) is needed.
     */
    void setNeedsSave(bool needs);

    /**
     * True when the module has something changed and needs save.
     */
    bool needsSave() const;

    /**
     * Indicate that the state of the modules contents has changed.
     *
     * This signal is emitted whenever the state of the configuration
     * shown in the module changes. It allows the module container to
     * keep track of unsaved changes.
     */
    Q_SIGNAL void needsSaveChanged();

    /**
     * Set this property to true when the user sets the state of the module
     * to the default settings (e.g. clicking Defaults would do nothing).
     */
    void setRepresentsDefaults(bool defaults);

    /**
     * True when the module state represents the default settings.
     */
    bool representsDefaults() const;

    /**
     * Indicate that the state of the modules contents has changed
     * in a way that it might represents the defaults settings, or
     * stopped representing them.
     */
    Q_SIGNAL void representsDefaultsChanged();

    /**
     * Sets the buttons to display.
     *
     * Help: shows a "Help" button.
     *
     * Default: shows a "Use Defaults" button.
     *
     * Apply: shows an "Ok", "Apply" and "Cancel" button.
     *
     * If Apply is not specified, kcmshell will show a "Close" button.
     *
     * @see ConfigModule::buttons
     */
    void setButtons(const Buttons btn);

    /**
     * Indicate which buttons will be used.
     *
     * The return value is a value or'ed together from
     * the Button enumeration type.
     *
     * @see ConfigModule::setButtons
     */
    Buttons buttons() const;

    /**
     * Buttons to display changed.
     */
    Q_SIGNAL void buttonsChanged();

    /**
     * @return true, if the authActionName is not empty
     * @sa setAuthActionName
     */
    bool needsAuthorization() const;

    /**
     * @returns the name of the config module
     */
    QString name() const;

    /**
     * @returns the description of the config module
     */
    QString description() const;

    /**
     * Change defaultness indicator visibility
     * @param visible
     */
    void setDefaultsIndicatorsVisible(bool visible);

    /**
     * @returns defaultness indicator visibility
     */
    bool defaultsIndicatorsVisible() const;

    /**
     * Emitted when kcm need to display indicators for field with non default value
     */
    Q_SIGNAL void defaultsIndicatorsVisibleChanged();

    /**
     * Returns the metaData that was used when instantiating the plugin
     */
    KPluginMetaData metaData() const;

    /**
     * This signal will be emited by a single-instance application (such as
     * System Settings) to request activation and update arguments to a module
     * that is already running
     *
     * The module should connect to this signal when it needs to handle
     * the activation request and specially the new arguments
     *
     * @param args A list of arguments that get passed to the module
     */
    Q_SIGNAL void activationRequested(const QVariantList &args);

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
     * The save method stores the config information as shown
     * in the user interface in the config files.
     *
     * This method is called when the user clicks "Apply" or "Ok".
     *
     */
    virtual void save();

    /**
     * Sets the configuration to default values.
     *
     * This method is called when the user clicks the "Default"
     * button.
     */
    virtual void defaults();

private:
    const std::unique_ptr<KAbstractConfigModulePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAbstractConfigModule::Buttons)

#endif // KABSTRACTCONFIGMODULE_H
