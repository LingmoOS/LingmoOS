/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009-2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KAUTH_ACTION_REPLY_H
#define KAUTH_ACTION_REPLY_H

#include "kauthcore_export.h"

#include <QDataStream>
#include <QMap>
#include <QSharedDataPointer>
#include <QString>
#include <QVariant>

/**
 @namespace KAuth

 @section kauth_intro Introduction

 The KDE Authorization API allows developers to write desktop applications that
 run high-privileged tasks in an easy, secure and cross-platform way.
 Previously, if an application had to do administrative tasks, it had to be run
 as root, using mechanisms such as sudo or graphical equivalents, or by setting
 the executable's setuid bit. This approach has some drawbacks. For example, the
 whole application code, including GUI handling and network communication, had
 to be done as root. More code that runs as root means more possible security
 holes.

 The solution is the caller/helper pattern. With this pattern, the privileged
 code is isolated in a small helper tool that runs as root. This tool includes
 only the few lines of code that actually need to be run with privileges, not
 the whole application logic. All the other parts of the application are run as
 a normal user, and the helper tool is called when needed, using a secure
 mechanism that ensures that the user is authorized to do so. This pattern is
 not very easy to implement, because the developer has to deal with a lot of
 details about how to authorize the user, how to call the helper with the right
 privileges, how to exchange data with the helper, etc.. This is where the new
 KDE Authorization API becomes useful. Thanks to this new library, every
 developer can implement the caller/helper pattern to write application that
 require high privileges, with a few lines of code in an easy, secure and
 cross-platform way.

 Not only: the library can also be used to lock down some actions in your
 application without using a helper but just checking for authorization and
 verifying if the user is allowed to perform it.

 The KDE Authorization library uses different backends depending on the system
 where it's built. As far as the user authorization is concerned, it currently
 uses polkit-1 on linux and Authorization Services on Mac OSX, and a Windows
 backend will eventually be written, too. At the communication layer, the
 library uses D-Bus on every supported platform.


 @section kauth_concepts Concepts

 There are a few concepts to understand when using the library. Much of those
 are carried from underlying APIs such as polkit-1, so if you know something
 about them there shouldn't be problems.

 An <i>action</i> is a single task that needs to be done by the application. You
 refer to an action using an action identifier, which is a string in reverse
 domain name syntax (to avoid duplicates). For example, if the date/time control
 center module needs to change the date, it would need an action like
 "org.kde.datatime.change". If your application has to perform more than one
 privileged task, you should configure more than one action. This allows system
 administrators to fine tune the policies that allow users to perform your
 actions.

 The <i>authorization</i> is the process that is executed to decide if a user
 can perform an action or not. In order to execute the helper as root, the user
 has to be authorized. For example, on linux, che policykit backend will look at
 the policykit policy database to see what requirements the user has to meet in
 order to execute the action you requested. The policy set for that action could
 allow or deny that user, or could say the user has to authenticate in order to
 gain the authorization.

 The <i>authentication</i> is the process that allows the system to know that
 the person is in front of the console is who he says to be. If an action can be
 allowed or not depending on the user's identity, it has to be proved by
 entering a password or any other identification data the system requires.

 A typical session with the authorization API is like this:
 - The user want to perform some privileged task
 - The application asks the system if the user is authorized.
 - The system asks the user to authenticate, if needed, and reply the application.
 - The application uses some system-provided mechanism to execute the helper's
   code as the root user. Previously, you had to set the setuid bit to do this,
   but we have something cool called
   "D-Bus activation" that doesn't require the setuid bit and is much more flexible.
 - The helper code, immediately after starting, checks if the caller is
   authorized to do what it asks. If not the helper immediately exits!
 - If the caller is authorized, the helper executes the task and exits.
 - The application receives data back from the helper.

 All these steps are managed by the library. Following sections will focus on
 how to write the helper to implement your actions and how to call the helper
 from the application.

 @section kauth_helper Writing the helper tool

 The first thing you need to do before writing anything is to decide what
 actions you need to implement. Every action needs to be identified by a string
 in the reverse domain name syntax. This helps to avoid duplicates. An example
 of action id is "org.kde.datetime.change" or "org.kde.ksysguard.killprocess".
 Action names can only contain lowercase letters and dots (not as the first or
 last char).  You also need an identifier for your helper. An application using
 the KDE auth api can implement and use more than one helper, implementing
 different actions. An helper is uniquely identified in the system context with
 a string. It, again, is in reverse domain name syntax to avoid duplicates. A
 common approach is to call the helper like the common prefix of your action
 names.  For example, the Date/Time kcm module could use a helper called
 "org.kde.datetime", to perform actions like "org.kde.datetime.changedate" and
 "org.kde.datetime.changetime". This naming convention simplifies the
 implementation of the helper.

 From the code point of view, the helper is implemented as a QObject subclass.
 Every action is implemented by a public slot. In the example/ directory in the
 source code tree you find a complete example.  Let's look at that.  The
 helper.h file declares the class that implements the helper. It looks like:

 @snippet helper.cpp helper_declaration

 The slot names are the last part of the action name, without the helper's ID if
 it's a prefix, with all the dots replaced by underscores. In this case, the
 helper ID is "org.kde.kf6auth.example", so those three slots implement the
 actions "org.kde.kf6auth.example.read", "org.kde.kf6auth.example.write" and
 "org.kde.kf6auth.example.longaction". The helper ID doesn't have to appear at
 the beginning of the action name, but it's good practice. If you want to extend
 MyHelper to implement also a different action like
 "org.kde.datetime.changetime", since the helper ID doesn't match you'll have to
 implement a slot called org_kde_datetime_changetime().

 The slot's signature is fixed: the return type is ActionReply, a class that
 allows you to return results, error codes and custom data to the application
 when your action has finished to run.

 Let's look at the read action implementation. Its purpose is to read files:

 @snippet helper.cpp helper_read_action

 First, the code creates a default reply object. The default constructor creates
 a reply that reports success. Then it gets the filename parameter from the
 argument QVariantMap, that has previously been set by the application, before
 calling the helper. If it fails to open the file, it creates an ActionReply
 object that notifies that some error has happened in the helper, then set the
 error code to that returned by QFile and returns. If there is no error, it
 reads the file. The contents are added to the reply.

 Because this class will be compiled into a standalone executable, we need a
 main() function and some code to initialize everything: you don't have to write
 it. Instead, you use the KAUTH_HELPER_MAIN() macro that will take care of
 everything. It's used like this:

 @snippet helper.cpp helper_main

 The first parameter is the string containing the helper identifier. Please note
 that you need to use this same string in the application's code to tell the
 library which helper to call, so please stay away from typos, because we don't
 have any way to detect them. The second parameter is the name of the helper's
 class.  Your helper, if complex, can be composed of a lot of source files, but
 the important thing is to include this macro in at least one of them.

 To build the helper, KDE macros provide a function named
 kauth_install_helper_files(). Use it in your cmake file like this:

 @code
 add_executable(<helper_target> your sources...)
 target_link_libraries(<helper_target> your libraries...)
 install(TARGETS <helper_target> DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

 kauth_install_helper_files(<helper_target> <helper_id> <user>)
 @endcode

 As locale is not inherited, the auth helper will have the text codec explicitly set
 to use UTF-8.

 The first argument is the cmake target name for the helper executable, which
 you have to build and install separately. Make sure to INSTALL THE HELPER IN
 @c ${KAUTH_HELPER_INSTALL_DIR}, otherwise @c kauth_install_helper_files will not work. The
 second argument is the helper id. Please be sure to don't misspell it, and to
 not quote it. The user parameter is the user that the helper has to be run as.
 It usually is root, but some actions could require less strict permissions, so
 you should use the right user where possible (for example the user apache if
 you have to mess with apache settings). Note that the target created by this
 macro already links to libkauth and QtCore.

 @section kauth_actions Action registration

 To be able to authorize the actions, they have to be added to the policy
 database. To do this in a cross-platform way, we provide a cmake macro. It
 looks like:
 @code
 kauth_install_actions(<helper_id> <actions definition file>)
 @endcode

 The action definition file describes which actions are implemented by your code
 and which default security options they should have. It is a common text file
 in ini format, with one section for each action and some parameters. The
 definition for the read action is:

 @verbatim
 [org.kde.kf6auth.example.read]
 Name=Read action
 Description=Read action description
 Policy=auth_admin
 Persistence=session
 @endverbatim

 The name parameter is a text describing the action for <i>who reads the
 file</i>. The description parameter is the message shown to the user in the
 authentication dialog. It should be a finite phrase.  The policy attribute
 specify the default rule that the user must satisfy to be authorized. Possible
 values are:
 - yes: the action should be always allowed
 - no: the action should be always denied
 - auth_self: the user should authenticate as itself
 - auth_admin: the user should authenticate as an administrator user

 The persistence attribute is optional. It says how long an authorization should
 be retained for that action. The values could be:
 - session: the authorization persists until the user logs-out
 - always: the authorization will persist indefinitely

 If this attribute is missing, the authorization will be queried every time.

 @note Only the PolicyKit and polkit-1 backends use this attribute.
 @warning With the polkit-1 backend, 'session' and 'always' have the same meaning.
          They just make the authorization persists for a few minutes.

 @section kauth_app Calling the helper from the application

 Once the helper is ready, we need to call it from the main application.
 In examples/client.cpp you can see how this is done. To create a reference to
 an action, an object of type Action has to be created. Every Action object
 refers to an action by its action id. Two objects with the same action id will
 act on the same action.  With an Action object, you can authorize and execute
 the action. To execute an action you need to retrieve an ExecuteJob, which is
 a standard KJob that you can run synchronously or asynchronously.
 See the KJob documentation (from KCoreAddons) for more details.

 The piece of code that calls the action of the previous example is:

 @snippet client.cpp client_how_to_call_helper

 First of all, it creates the action object specifying the action id. Then it
 loads the filename (we want to read a forbidden file) into the arguments()
 QVariantMap, which will be directly passed to the helper in the read() slot's
 parameter. This example code uses a synchronous call to execute the action and
 retrieve the reply. If the reply succeeded, the reply data is retrieved from
 the returned QVariantMap object. Please note that you have
 to explicitly set the helper ID to the action: this is done for added safety,
 to prevent the caller from accidentally invoking a helper, and also because
 KAuth actions may be used without a helper attached (the default).

 Please note that if your application is calling the helper multiple times it
 must do so from the same thread.

 @section kauth_async Asynchronous calls, data reporting, and action termination

 For a more advanced example, we look at the action
 "org.kde.kf6auth.example.longaction" in the example helper. This is an action
 that takes a long time to execute, so we need some features:
 - The helper needs to regularly send data to the application, to inform about
   the execution status.
 - The application needs to be able to stop the action execution if the user
   stops it or close the application.
 The example code follows:

 @snippet helper.cpp helper_longaction

 In this example, the action is only waiting a "long" time using a loop, but we
 can see some interesting line. The progress status is sent to the application
 using the HelperSupport::progressStep(int) and
 HelperSupport::progressStep(const QVariantMap &) methods.
 When those methods are called, the HelperProxy associated with this action
 will emit the HelperProxy::progressStep(const QString &, int) and
 HelperProxy::progressStepData(const QString &, const QVariantMap &) signals,
 respectively, reporting back the data to the application.
 The method that takes an integer argument is the one used here.
 Its meaning is application dependent, so you can use it as a sort of
 percentage. If you want to report custom data back to the application, you
 can use the other method that takes a QVariantMap object which is directly
 passed to the app.

 In this example code, the loop exits when the HelperSupport::isStopped()
 returns true. This happens when the application calls the HelperProxy::stopAction()
 method on the corresponding action object.
 The stopAction() method, this way, asks the helper to
 stop the action execution. It's up to the helper to obbey to this request, and
 if it does so, it should return from the slot, _not_ exit.

 @section kauth_other Other features

 It doesn't happen very frequently that you code something that doesn't require
 some debugging, and you'll need some tool, even a basic one, to debug your
 helper code as well. For this reason, the KDE Authorization library provides a
 message handler for the Qt debugging system. This means that every call to
 qDebug() & co. will be reported to the application, and printed using the same
 qt debugging system, with the same debug level.  If, in the helper code, you
 write something like:
 @code
 qDebug() << "I'm in the helper";
 @endcode
 You'll see something like this in the <i>application</i>'s output:

 @verbatim
 Debug message from the helper: I'm in the helper
 @endverbatim

 Remember that the debug level is preserved, so if you use qFatal() you won't
 only abort the helper (which isn't suggested anyway), but also the application.

 */
namespace KAuth
{
class ActionReplyData;

/**
 * @class ActionReply actionreply.h <KAuth/ActionReply>
 *
 * @brief Class that encapsulates a reply coming from the helper after executing
 * an action
 *
 * Helper applications will return this to describe the result of the action.
 *
 * Callers should access the reply though the KAuth::ExecuteJob job.
 *
 * @since 4.4
 */
class KAUTHCORE_EXPORT ActionReply
{
public:
    /**
     * Enumeration of the different kinds of replies.
     */
    enum Type {
        KAuthErrorType, ///< An error reply generated by the library itself.
        HelperErrorType, ///< An error reply generated by the helper.
        SuccessType, ///< The action has been completed successfully
    };

    static const ActionReply SuccessReply(); ///< An empty successful reply. Same as using the default constructor
    static const ActionReply HelperErrorReply(); ///< An empty reply with type() == HelperError and errorCode() == -1
    static const ActionReply HelperErrorReply(int error); ///< An empty reply with type() == HelperError and error is set to the passed value

    static const ActionReply NoResponderReply(); ///< errorCode() == NoResponder
    static const ActionReply NoSuchActionReply(); ///< errorCode() == NoSuchAction
    static const ActionReply InvalidActionReply(); ///< errorCode() == InvalidAction
    static const ActionReply AuthorizationDeniedReply(); ///< errorCode() == AuthorizationDenied
    static const ActionReply UserCancelledReply(); ///< errorCode() == UserCancelled
    static const ActionReply HelperBusyReply(); ///< errorCode() == HelperBusy
    static const ActionReply AlreadyStartedReply(); ///< errorCode() == AlreadyStartedError
    static const ActionReply DBusErrorReply(); ///< errorCode() == DBusError

    /**
     * The enumeration of the possible values of errorCode() when type() is ActionReply::KAuthError
     */
    enum Error {
        NoError = 0, ///< No error.
        NoResponderError, ///< The helper responder object hasn't been set. This shouldn't happen if you use the KAUTH_HELPER macro in the helper source
        NoSuchActionError, ///< The action you tried to execute doesn't exist.
        InvalidActionError, ///< You tried to execute an invalid action object
        AuthorizationDeniedError, ///< You don't have the authorization to execute the action
        UserCancelledError, ///< Action execution has been cancelled by the user
        HelperBusyError, ///< The helper is busy executing another action (or group of actions). Try later
        AlreadyStartedError, ///< The action was already started and is currently running
        DBusError, ///< An error from D-Bus occurred
        BackendError, ///< The underlying backend reported an error
    };

    /// Default constructor. Sets type() to Success and errorCode() to zero.
    ActionReply();

    /**
     * @brief Constructor to directly set the type.
     *
     * This constructor directly sets the reply type. You shouldn't need to
     * directly call this constructor, because you can use the more convenient
     * predefined replies constants. You also shouldn't create a reply with
     * the KAuthError type because it's reserved for errors coming from the
     * library.
     *
     * @param type The type of the new reply
     */
    ActionReply(Type type);

    /**
     * @brief Constructor that creates a KAuthError reply with a specified error code.
     *        Do not use outside the library.
     *
     * This constructor is for internal use only, since it creates a reply
     * with KAuthError type, which is reserved for errors coming from the library.
     *
     * @param errorCode The error code of the new reply
     */
    ActionReply(int errorCode);

    /// Copy constructor
    ActionReply(const ActionReply &reply);

    /// Virtual destructor
    virtual ~ActionReply();

    /**
     * @brief Sets the custom data to send back to the application
     *
     * In the helper's code you can use this function to set an QVariantMap
     * with custom data that will be sent back to the application.
     *
     * @param data The new QVariantMap object.
     */
    void setData(const QVariantMap &data);

    /**
     * @brief Returns the custom data coming from the helper.
     *
     * This method is used to get the object that contains the custom
     * data coming from the helper. In the helper's code, you can set it
     * using setData() or the convenience method addData().
     *
     * @return The data coming from (or that will be sent by) the helper
     */
    QVariantMap data() const;

    /**
     * @brief Convenience method to add some data to the reply.
     *
     * This method adds the pair @c key/value to the QVariantMap used to
     * report back custom data to the application.
     *
     * Use this method if you don't want to create a new QVariantMap only to
     * add a new entry.
     *
     * @param key The new entry's key
     * @param value The value of the new entry
     */
    void addData(const QString &key, const QVariant &value);

    /// Returns the reply's type
    Type type() const;

    /**
     * @brief Sets the reply type
     *
     * Every time you create an action reply, you implicitly set a type.
     * Default constructed replies or ActionReply::SuccessReply have
     * type() == Success.
     * ActionReply::HelperErrorReply has type() == HelperError.
     * Predefined error replies have type() == KAuthError.
     *
     * This means you rarely need to change the type after the creation,
     * but if you need to, don't set it to KAuthError, because it's reserved
     * for errors coming from the library.
     *
     * @param type The new reply type
     */
    void setType(Type type);

    /// Returns true if type() == Success
    bool succeeded() const;

    /// Returns true if type() != Success
    bool failed() const;

    /**
     * @brief Returns the error code of an error reply
     *
     * The error code returned is one of the values in the ActionReply::Error
     * enumeration if type() == KAuthError, or is totally application-dependent if
     * type() == HelperError. It also should be zero for successful replies.
     *
     * @return The reply error code
     */
    int error() const;

    /**
     * @brief Returns the error code of an error reply
     *
     * The error code returned is one of the values in the ActionReply::Error
     * enumeration if type() == KAuthError.
     * Result is only valid if the type() == HelperError
     *
     * @return The reply error code
     */
    Error errorCode() const;

    /**
     * @brief Sets the error code of an error reply
     *
     * If you're setting the error code in the helper because
     * you need to return an error to the application, please make sure
     * you already have set the type to HelperError, either by calling
     * setType() or by creating the reply in the right way.
     *
     * If the type is Success when you call this method, it will become KAuthError
     *
     * @param error The new reply error code
     */
    void setError(int error);

    /**
     * @brief Sets the error code of an error reply
     *
     * @see
     * If you're setting the error code in the helper, use setError(int)
     *
     * If the type is Success when you call this method, it will become KAuthError
     *
     * @param errorCode The new reply error code
     */
    void setErrorCode(Error errorCode);

    /**
     * @brief Gets a human-readble description of the error, if available
     *
     * Currently, replies of type KAuthError rarely report an error description.
     * This situation could change in the future.
     *
     * By now, you can use this method for custom errors of type HelperError.
     *
     * @return The error human-readable description
     */
    QString errorDescription() const;

    /**
     * @brief Sets a human-readble description of the error
     *
     * Call this method from the helper if you want to send back a description for
     * a custom error. Note that this method doesn't affect the errorCode in any way
     *
     * @param error The new error description
     */
    void setErrorDescription(const QString &error);

    /**
     * @brief Serialize the reply into a QByteArray.
     *
     * This is a convenience method used internally to sent the reply to a remote peer.
     * To recreate the reply, use deserialize()
     *
     * @return A QByteArray representation of this reply
     */
    QByteArray serialized() const;

    /**
     * @brief Deserialize a reply from a QByteArray
     *
     * This method returns a reply from a QByteArray obtained from
     * the serialized() method.
     *
     * @param data A QByteArray obtained with serialized()
     */
    static ActionReply deserialize(const QByteArray &data);

    /// Assignment operator
    ActionReply &operator=(const ActionReply &reply);

    /**
     * @brief Comparison operator
     *
     * This operator checks if the type and the error code of two replies are the same.
     * It <b>doesn't</b> compare the data or the error descriptions, so be careful.
     *
     * The suggested use is to compare a reply against one of the predefined error replies:
     * @code
     * if(reply == ActionReply::HelperBusyReply) {
     *    // Do something...
     * }
     * @endcode
     *
     * Note that you can do it also by compare errorCode() with the relative enumeration value.
     */
    bool operator==(const ActionReply &reply) const;

    /**
     * @brief Negated comparison operator
     *
     * See the operator==() for an important notice.
     */
    bool operator!=(const ActionReply &reply) const;

private:
    QSharedDataPointer<ActionReplyData> d;
};

} // namespace Auth

Q_DECLARE_METATYPE(KAuth::ActionReply)

#endif
