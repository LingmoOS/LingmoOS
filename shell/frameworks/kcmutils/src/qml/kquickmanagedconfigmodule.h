/*
    SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MANAGEDCONFIGMODULE_H
#define MANAGEDCONFIGMODULE_H

#include "kquickconfigmodule.h"
#include <memory>

class KCoreConfigSkeleton;

class KQuickManagedConfigModulePrivate;

/**
 * @class KQuickManagedConfigModule managedconfigmodule.h KQuickAddons/ManagedConfigModule
 *
 * The base class for configuration modules using KConfigXT settings.
 *
 * We are assuming here that SettingsObject is a class generated from a kcfg file
 * and that it will be somehow exposed as a constant property to be used from the QML side.
 * It will be automatically discovered by ManagedConfigModule which will update
 * the saveNeeded and defaults inherited properties by itself. Thus by inheriting from
 * this class you shall not try to manage those properties yourselves.
 * By passing in "this" as a parent, we prevent memory leaks and allow KQuickManagedConfigModule to
 * automatically find the created settings object.
 *
 * The constructor of the ConfigModule then looks like this:
 * \code
 * YourConfigModule::YourConfigModule(QObject *parent, const KPluginMetaData &metaData)
 *   : ManagedConfigModule(parent, metaData)
 *   , m_settingsObject(new SettingsObject(this))
 * {
 * }
 * \endcode
 *
 * @since 6.0
 */
class KCMUTILSQUICK_EXPORT KQuickManagedConfigModule : public KQuickConfigModule
{
    Q_OBJECT

public:
    /**
     * Destroys the module.
     */
    ~KQuickManagedConfigModule() override;

public Q_SLOTS:
    /**
     * Load the configuration data into the module.
     *
     * This method is invoked whenever the module should read its configuration
     * (most of the times from a config file) and update the user interface.
     * This happens when the user clicks the "Reset" button in the control
     * center, to undo all of his changes and restore the currently valid
     * settings. It is also called right after construction.
     *
     * By default this will load the settings from the child setting objects
     * of this module.
     */
    void load() override;

    /**
     * Save the configuration data.
     *
     * The save method stores the config information as shown
     * in the user interface in the config files.
     * It is called when the user clicks "Apply" or "Ok".
     *
     * By default this will save the child setting objects
     * of this module.
     */
    void save() override;

    /**
     * Sets the configuration to sensible default values.
     *
     * This method is called when the user clicks the "Default"
     * button. It should set the display to useful values.
     *
     * By default this will reset to defaults the child setting objects
     * of this module.
     */
    void defaults() override;

protected Q_SLOTS:
    /**
     * Forces the module to reevaluate the saveNeeded and
     * representsDefault state.
     *
     * This is required for some modules which might have
     * some settings managed outside of KConfigXT objects.
     */
    void settingsChanged();

    /**
     * Allow to register manually settings class generated from a kcfg file.
     * Used by derived class when automatic discovery is not possible.
     * After skeleton is registered it will automatically call settingsChanged().
     */
    void registerSettings(KCoreConfigSkeleton *skeleton);

protected:
    /**
     * Base class for all KControlModules.
     * Use KQuickConfigModuleLoader to instantiate this class
     *
     * @note do not emit changed signals here, since they are not yet connected to any slot.
     */
    explicit KQuickManagedConfigModule(QObject *parent, const KPluginMetaData &metaData);

private:
    /**
     * Allows to indicate if the module requires saving.
     *
     * By default this returns false, it needs to be overridden only
     * if the module has state outside of the settings declared in
     * the KConfigXT classes it uses.
     */
    virtual bool isSaveNeeded() const;

    /**
     * Allows to indicate if the module state is representing its defaults.
     *
     * By default this returns true, it needs to be overridden only
     * if the module has state outside of the settings declared in
     * the KConfigXT classes it uses.
     */
    virtual bool isDefaults() const;

    const std::unique_ptr<KQuickManagedConfigModulePrivate> d;
    friend class KQuickManagedConfigModulePrivate;
};

#endif // MANAGEDCONFIGMODULE_H
