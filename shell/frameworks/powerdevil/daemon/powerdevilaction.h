/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "powerdevilpolicyagent.h"

#include <QObject>
#include <QVariantMap>

#include <chrono>

#include "powerdevilcore_export.h"

namespace PowerDevil
{
class Core;
class ProfileSettings;

/**
 * @brief The base class for Power Management Actions
 *
 * The Action class is the very base class for writing a new Power Management action.
 * Developers wishing to implement their own action are supposed to subclass Action.
 *
 * @par Creating a brand new Action
 *
 * If you are already familiar with KDE's plugin system, you have to know that actions are
 * nothing but a KCoreAddons plugin which will be loaded on demand. Each action has an ID associated to it
 * which uniquely identifies it in the loading phase.
 *
 * In addition to standard parameters, the .json file representing your action should contain the following
 * entry:
 *
 * @code
 * X-KDE-PowerDevil-Action-ID: YourActionID
 * @endcode
 *
 * The most important functions you need to reimplement are loadAction and triggerImpl. The first is called
 * whenever an action is loaded, carrying the action's configuration. The other is called whenever
 * the action is triggered. You usually want to process the action here as triggerImpl is guaranteed
 * to be called just when policies are satisfied.
 *
 * @par Runtime requirements
 *
 * Some actions might be available only when the system satisfies certain hardware or software runtime requirements.
 * In this case, powerdevil provides a way for the action to advertise to the outside whether it is supported or
 * not. This can be done by reimplementing @c isSupported and adding to the .json file the field
 *
 * @code
 * X-KDE-PowerDevil-Action-HasRuntimeRequirement: true
 * @endcode
 *
 * Done that, powerdevil will take care of exposing your action only if support for it is advertised. In addition,
 * the UI will expose the configuration of your action only if its support is advertised. Of course, this means the
 * action will be temporarily loaded by the config UI to verify its support. If your action is performing some tasks in
 * the constructor besides setting policies, revise your design since you probably don't need or want to do that.
 *
 * @par Handling policies from within the action
 *
 * As you might know, the KDE Power Management system features a very efficient policy handler, which
 * prevents Power Management actions when certain condition occurs. The integration with Actions is very easy:
 * in your Action's constructor, you want to call setRequiredPolicies, stating which policies have to be
 * satisfied to perform the action. For example, if your action should not be performed whenever the session
 * cannot be interrupted, you would pass @c InterruptSession.
 *
 * Done that, your action is already obeying to the policy. trigger, in fact, calls triggerImpl just when
 * the policies are allowing your action to be performed.
 *
 * @since 4.6
 */
class POWERDEVILCORE_EXPORT Action : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Action)

public:
    /**
     * Default constructor
     */
    explicit Action(QObject *parent);
    /**
     * Default destructor
     */
    ~Action() override;

    /**
     * Reimplement this function when creating a new Action. This function is called whenever the action is loaded or
     * its configuration changes. It carries the ProfileSettings associated with the active power management
     * profile and generated from your config interface.
     *
     * @param profileSettings The profile settings containing the action's configuration which should be loaded.
     * @returns Whether the action has been successfully loaded. Should return false if not enabled for the given @p profileSettings.
     */
    virtual bool loadAction(const PowerDevil::ProfileSettings &profileSettings) = 0;
    /**
     * Unloads the action.
     */
    void unloadAction();

    /**
     * This function is meant to find out if this action is available on this system. Actions
     * CAN reimplement this function if they are dependent on specific hardware/software requirements.
     * By default, this function will always return true.
     *
     * Should this function return false, the core will delete and ignore the action right after creation.
     *
     * @returns Whether this action is supported or not by the current system
     */
    virtual bool isSupported();

    /**
     * Triggers the action with the given argument. This function is meant to be used by the caller only -
     * if you are implementing your own action, reimplement triggerImpl instead.
     *
     * @param args The arguments for triggering the action
     */
    void trigger(const QVariantMap &args);

protected:
    /**
     * Registers an idle timeout for this action. Call this function and not KIdleTime directly to take advantage
     * of Action's automated handling of idle timeouts. Also, please reimplement onIdleTimeout instead of listening
     * to KIdleTime's signals to catch idle timeout events.
     */
    void registerIdleTimeout(std::chrono::milliseconds timeout);
    /**
     * Removes any previously registered idle timeouts for this action.
     */
    void unregisterIdleTimeouts();
    /**
     * Sets the required policies needed for this Action to run. Usually, you want to call this function in your
     * Action's constructor.
     *
     * @param requiredPolicies A set of policies which are required to run this action. It can be empty if your
     *                         Action does not rely on policies.
     */
    void setRequiredPolicies(PowerDevil::PolicyAgent::RequiredPolicies requiredPolicies);

    /**
     * This function's body should undertake the Action's execution. It has to be reimplemented in a new Action.
     *
     * @param args The arguments for triggering the action
     */
    virtual void triggerImpl(const QVariantMap &args) = 0;

    /**
     * @returns The PowerDevil Core
     */
    PowerDevil::Core *core() const;

protected Q_SLOTS:
    /**
     * This function is called whenever a profile is loaded. Please note that this is slightly different from
     * loadAction: in fact a profile can be reloaded without having the action change its configuration.
     * If your action should do something as soon as a profile switches, it should be done inside this function.
     */
    virtual void onProfileLoad(const QString &previousProfile, const QString &newProfile);
    /**
     * This slot is triggered whenever an idle timeout registered with registerIdleTimeout is reached.
     *
     * @param timeout The idle timeout reached
     */
    virtual void onIdleTimeout(std::chrono::milliseconds timeout);
    /**
     * This slot is triggered whenever the PC wakes up from an Idle state. It is @b always called after a registered
     * idle timeout has been reached.
     */
    virtual void onWakeupFromIdle();
    /**
     * This function is called when the profile is unloaded.
     */
    virtual void onProfileUnload();

private:
    PowerDevil::Core *m_core;

    QList<std::chrono::milliseconds> m_registeredIdleTimeouts;
    PowerDevil::PolicyAgent::RequiredPolicies m_requiredPolicies;

    friend class Core;
};

}
