/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2014, 2016 René Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "AuthServicesBackend.h"

#include <qplugin.h>

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(KAUTH_OSX)
// logging category for this backend, default: log stuff >= warning
Q_LOGGING_CATEGORY(KAUTH_OSX, "kf.auth.apple", QtWarningMsg)

namespace KAuth
{
static AuthorizationRef s_authRef = NULL;

AuthorizationRef authRef()
{
    if (!s_authRef) {
        AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &s_authRef);
    }

    return s_authRef;
}

// GetActionRights return codes:
//     errAuthorizationSuccess                 = 0,
//     errAuthorizationInvalidSet              = -60001, /* The authorization rights are invalid. */
//     errAuthorizationInvalidRef              = -60002, /* The authorization reference is invalid. */
//     errAuthorizationInvalidTag              = -60003, /* The authorization tag is invalid. */
//     errAuthorizationInvalidPointer          = -60004, /* The returned authorization is invalid. */
//     errAuthorizationDenied                  = -60005, /* The authorization was denied. */
//     errAuthorizationCanceled                = -60006, /* The authorization was cancelled by the user. */
//     errAuthorizationInteractionNotAllowed   = -60007, /* The authorization was denied since no user interaction was possible. */
//     errAuthorizationInternal                = -60008, /* Unable to obtain authorization for this operation. */
//     errAuthorizationExternalizeNotAllowed	= -60009, /* The authorization is not allowed to be converted to an external format. */
//     errAuthorizationInternalizeNotAllowed	= -60010, /* The authorization is not allowed to be created from an external format. */
//     errAuthorizationInvalidFlags            = -60011, /* The provided option flag(s) are invalid for this authorization operation. */
//     errAuthorizationToolExecuteFailure      = -60031, /* The specified program could not be executed. */
//     errAuthorizationToolEnvironmentError    = -60032, /* An invalid status was returned during execution of a privileged tool. */
//     errAuthorizationBadAddress              = -60033, /* The requested socket address is invalid (must be 0-1023 inclusive). */
static OSStatus GetActionRights(const QString &action, AuthorizationFlags flags, AuthorizationRef auth)
{
    AuthorizationItem item;
    item.name = action.toUtf8().constData();
    item.valueLength = 0;
    item.value = NULL;
    item.flags = 0;

    AuthorizationRights rights;
    rights.count = 1;
    rights.items = &item;

    OSStatus result = AuthorizationCopyRights(auth, &rights, kAuthorizationEmptyEnvironment, flags, NULL);
    return result;
}

// On OS X we avoid using a helper but grab privilege from here, the client.
AuthServicesBackend::AuthServicesBackend()
    : AuthBackend()
{
    setCapabilities(AuthorizeFromClientCapability);
}

AuthServicesBackend::~AuthServicesBackend()
{
    if (s_authRef) {
        OSStatus err = AuthorizationFree(s_authRef, kAuthorizationFlagDefaults);
        qCDebug(KAUTH_OSX) << "AuthorizationFree(" << s_authRef << ") returned" << err;
        s_authRef = NULL;
    }
}

void AuthServicesBackend::setupAction(const QString &)
{
    // Nothing to do here...
}

Action::AuthStatus AuthServicesBackend::authorizeAction(const QString &action)
{
    Action::AuthStatus retval;
    OSStatus result = GetActionRights(action, kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed, authRef());
    qCDebug(KAUTH_OSX) << "AuthServicesBackend::authorizeAction(" << action << ") AuthorizationCopyRights returned" << result;
    switch (result) {
    case errAuthorizationSuccess:
        retval = Action::AuthorizedStatus;
        break;
    case errAuthorizationCanceled:
        retval = Action::UserCancelledStatus;
        break;
    case errAuthorizationInteractionNotAllowed:
    case errAuthorizationDenied:
        retval = Action::DeniedStatus;
        break;
    case errAuthorizationInternal:
        // does this make sense?
        retval = Action::AuthRequiredStatus;
        break;
    case errAuthorizationExternalizeNotAllowed:
    case errAuthorizationInternalizeNotAllowed:
    case errAuthorizationToolExecuteFailure:
    case errAuthorizationToolEnvironmentError:
    case errAuthorizationBadAddress:
        retval = Action::ErrorStatus;
        break;
    default:
        retval = Action::InvalidStatus;
        break;
    }
    return retval;
}

Action::AuthStatus AuthServicesBackend::actionStatus(const QString &action)
{
    Action::AuthStatus retval;
    OSStatus result = GetActionRights(action, kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize, authRef());
    qCDebug(KAUTH_OSX) << "AuthServicesBackend::actionStatus(" << action << ") AuthorizationCopyRights returned" << result;
    // this function has a simpler return code parser:
    switch (result) {
    case errAuthorizationSuccess:
        retval = Action::AuthorizedStatus;
        break;
    case errAuthorizationCanceled:
        retval = Action::UserCancelledStatus;
        break;
    case errAuthorizationInteractionNotAllowed:
        retval = Action::AuthRequiredStatus;
        break;
    default:
        retval = Action::DeniedStatus;
        break;
    }
    return retval;
}

QByteArray AuthServicesBackend::callerID() const
{
    AuthorizationExternalForm ext;
    AuthorizationMakeExternalForm(authRef(), &ext);
    QByteArray id((const char *)&ext, sizeof(ext));

    return id;
}

bool AuthServicesBackend::isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details)
{
    Q_UNUSED(details);

    AuthorizationExternalForm ext;
    memcpy(&ext, callerID.data(), sizeof(ext));

    AuthorizationRef auth;

    if (AuthorizationCreateFromExternalForm(&ext, &auth) != noErr) {
        qCWarning(KAUTH_OSX()) << "AuthorizationCreateFromExternalForm(" << action << "," << callerID.constData() << ") failed";
        return false;
    }

    OSStatus result = GetActionRights(action, kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed, auth);

    AuthorizationFree(auth, kAuthorizationFlagDefaults);
    qCDebug(KAUTH_OSX) << "AuthServicesBackend::isCallerAuthorized(" << action << "," << callerID.constData() << ") AuthorizationCopyRights returned" << result;

    return result == errAuthorizationSuccess;
}

}; // namespace KAuth

#include "moc_AuthServicesBackend.cpp"
