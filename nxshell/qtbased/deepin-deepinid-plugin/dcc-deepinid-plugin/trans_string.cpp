// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "trans_string.h"
#include <QMap>



QString TransString::getTransString(TransStrType type)
{
    static QMap<TransStrType, const char *> gTransMap = {
        {STRING_CLOUDTITLE , QT_TR_NOOP("deepin Cloud Sync")},
        {STRING_CANOTUSE , QT_TR_NOOP("Subject to your local laws and regulations, it is currently unavailable in your region.")},
        {STRING_CANCEL , QT_TR_NOOP("Cancel")},
        {STRING_CLOUDMSG , QT_TR_NOOP("Securely store system settings and personal data in the cloud, and keep them in sync across devices.")},

        {STRING_SYSCONFIG , QT_TR_NOOP("System Settings")},
        {STRING_NETWORK , QT_TR_NOOP("Network")},
        {STRING_SOUND , QT_TR_NOOP("Sound")},
        {STRING_MOUSE , QT_TR_NOOP("Mouse")},
        {STRING_UPDATE , QT_TR_NOOP("Updates")},
        {STRING_DOCK , QT_TR_NOOP("Dock")},
        {STRING_LAUNCHER , QT_TR_NOOP("Launcher")},
        {STRING_WALLPAPER , QT_TR_NOOP("Wallpaper")},
        {STRING_THEME , QT_TR_NOOP("Theme")},
        {STRING_POWER , QT_TR_NOOP("Power")},
        {STRING_SELECTALL , QT_TR_NOOP("Select All")},
        {STRING_UNSELECTALL , QT_TR_NOOP("Unselect All")},
        {STRING_EMPTY , QT_TR_NOOP("Clear cloud data")},
        {STRING_EMPTYTITLE, QT_TR_NOOP("Are you sure you want to clear your system settings and personal data saved in the cloud?")},
        {STRING_EMPTYMSG , QT_TR_NOOP("Once the data is cleared, it cannot be recovered!")},
        {STRING_CLEARBTN , QT_TR_NOOP("Clear")},
        {STRING_SYNCING , QT_TR_NOOP("Syncing...")},
        {STRING_LASTSYNC , QT_TR_NOOP("Last sync: %1")},
        {STRING_CONFIRM , QT_TR_NOOP("Confirm")},
        {STRING_LOCALTITLE , QT_TR_NOOP("Link local accounts to deepin ID")},
        {STRING_LOCALMSG , QT_TR_NOOP("If linked, you can use deepin ID to log in and reset passwords of local accounts. Switch it off to get them unlinked.")},

        {STRING_DEVTITLE , QT_TR_NOOP("Trusted devices")},
        {STRING_DEVMSG , QT_TR_NOOP("Manage trusted devices to keep your deepin ID secure.")},

        {STRING_OSVER , QT_TR_NOOP("OS version: %1")},
        {STRING_CURRDEV , QT_TR_NOOP("In use")},
        {STRING_REMOVE , QT_TR_NOOP("Remove")},
        {STRING_DELDLGTITLE , QT_TR_NOOP("Are you sure you want to\nremove this device from trusted devices?")},

        {STRING_CLOUDSV , QT_TR_NOOP("Cloud Service of deepin ID")},
        {STRING_CLOUDTITLEMSG , QT_TR_NOOP("Manage your deepin ID and sync your personal data across devices.\nSign in to deepin ID to get personalized features and services of Browser, App Store, Support, and more.")},
        {STRING_LOGIN , QT_TR_NOOP("Sign In to deepin ID")},

        {STRING_CLOUDITEM , QT_TR_NOOP("deepin Cloud")},
        {STRING_DEVITEM , QT_TR_NOOP("Devices and Local Accounts")},
        {STRING_ACCOUNTINFO , QT_TR_NOOP("Account Info")},
        {STRING_INACTIVE , QT_TR_NOOP("The feature is not available at present, please activate your system first")},

        {STRING_TITLEPHONE , QT_TR_NOOP("Trusted phone numbers and email addresses")},
        {STRING_MSGPHONE , QT_TR_NOOP("Use the trusted mobile number or email address to sign in to deepin ID, and verify sensitive operations if they are performed on deepin ID.")},
        {STRING_PHONEBIND , QT_TR_NOOP("Link phone number to deepin ID")},
        {STRING_MAILBIND , QT_TR_NOOP("Link email address to deepin ID")},
        {STRING_WECHATTITLE , QT_TR_NOOP("Link third-party accounts")},
        {STRING_WECHATMSG , QT_TR_NOOP("Securely and quickly log in to your deepin ID and local account by scanning the QR code with the linked WeChat.")},
        {STRING_WECHATUNBINDTITLE , QT_TR_NOOP("If unlinked, you will not be able to use WeChat to sign in to deepin ID.")},
        {STRING_WECHATUNBINDMESSAGE , QT_TR_NOOP("Are you sure you want to unlink it?")},
        {STRING_PASSWORDTITLE , QT_TR_NOOP("Password")},
        {STRING_PASSWORDMSG , QT_TR_NOOP("Change your deepin ID password regularly to ensure its security.")},
        {STRING_PWDACTION , QT_TR_NOOP("Change password")},
        {STRING_PWDMODSUCCESS , QT_TR_NOOP("Password changed")},
        {STRING_FAILTIP , QT_TR_NOOP("Operation failed, please try later")},
        {STRING_SUCCESSTIP, QT_TR_NOOP("Operation Successful")},
        {STRING_RESETPWDTIP , QT_TR_NOOP("Reset password successfully")},

        {STRING_PHONEHOLDER , QT_TR_NOOP("Phone number")},
        {STRING_MAILHOLDER , QT_TR_NOOP("Email address")},
        {STRING_PHONEINVALID , QT_TR_NOOP("Enter a correct phone number")},
        {STRING_MAILINVALID , QT_TR_NOOP("Enter a correct email address")},
        {STRING_USEANOPHONE , QT_TR_NOOP("Use another phone number")},
        {STRING_USEANOMAIL , QT_TR_NOOP("Use another email address")},

        {STRING_BINDTIP , QT_TR_NOOP("Link")},
        {STRING_UNBINDTIP , QT_TR_NOOP("Unlink")},
        {STRING_UNLINKED , QT_TR_NOOP("Unlinked")},
        {STRING_MODIFY , QT_TR_NOOP("Change")},
        {STRING_PWDCHECKERROR , QT_TR_NOOP("Wrong password, %1 chances left")},
        {STRING_LOGINLIMIT , QT_TR_NOOP("The login error has reached the limit today. You can reset the password and try again.")},
        {STRING_REBINDPHONE , QT_TR_NOOP("The phone number has been linked to deepin ID: %1.\nPlease check and choose one action below.")},
        {STRING_REBINDMAIL , QT_TR_NOOP("The email address has been linked to deepin ID: %1.\nPlease check and choose one action below.")},

        {STRING_VERIFYTITLE , QT_TR_NOOP("Security Verification")},
        {STRING_VERIFYMSG , QT_TR_NOOP("The action is sensitive, please enter the login password first")},
        {STRING_RESETPWDTITLE , QT_TR_NOOP("Change deepin ID password")},
        {STRING_FORGETPWD , QT_TR_NOOP("Forgot password?")},
        {STRING_BINDLOCAL , QT_TR_NOOP("Link to current deepin ID")},
        {STRING_REGISTERPWD , QT_TR_NOOP("Set a Password")},

        {STRING_VERIFYCODE , QT_TR_NOOP("Get Code")},
        {STRING_VERIFYHOLDER , QT_TR_NOOP("Verification code")},
        {STRING_RESENDCODE , QT_TR_NOOP("Resend (%1s)")},
        {STRING_VERIFYINVALID , QT_TR_NOOP("Enter a correct verification code")},

        {STRING_WECHATBIND , QT_TR_NOOP("Link WeChat to deepin ID")},
        {STRING_WECHATSCAN , QT_TR_NOOP("Scan QR code with WeChat")},

        {STRING_PWDDIGCHAR , QT_TR_NOOP("Password must contain numbers and letters")},
        {STRING_PWDLENGTH , QT_TR_NOOP("Password must be between 8 and 64 characters")},

        {STRING_PWDHOLDER , QT_TR_NOOP("8-64 characters")},
        {STRING_PWDNOTMATCH , QT_TR_NOOP("Passwords don't match")},
        {STRING_AGAINHOLDER , QT_TR_NOOP("Repeat the password")},
        {STRING_PWDINVALID , QT_TR_NOOP("8-64 characters, including uppercase or lowercase English letters and numbers")},
        {STRING_NICKNAMEINVALID, QT_TR_NOOP("Invalid nickname, please enter a new one")},
        {STRING_NICKNAMENULL, QT_TR_NOOP("The nickname cannot contain special characters ('<>&\") and whitespaces")},
        {STRING_NICKNAMEONCE, QT_TR_NOOP("The nickname can be modified only once a day")},
    };


    return tr(gTransMap[type]);
}
