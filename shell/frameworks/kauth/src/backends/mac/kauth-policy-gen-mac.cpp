/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "../../policy-gen/policy-gen.h"

#include <Security/Security.h>
#include <iostream>

#include <QDebug>

using namespace std;

void output(const QList<Action> &actions, const QMap<QString, QString> &domain)
{
    AuthorizationRef auth;
    AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &auth);

    OSStatus err;

    for (const Action &action : std::as_const(actions)) {
        err = AuthorizationRightGet(action.name.toLatin1().constData(), NULL);

        if (err != errAuthorizationSuccess) {
            QString rule;

            if (action.policy == QLatin1String("yes")) {
                rule = QString::fromLatin1(kAuthorizationRuleClassAllow);
            } else if (action.policy == QLatin1String("no")) {
                rule = QString::fromLatin1(kAuthorizationRuleClassDeny);
            } else if (action.policy == QLatin1String("auth_self")) {
                rule = QString::fromLatin1(kAuthorizationRuleAuthenticateAsSessionUser);
            } else if (action.policy == QLatin1String("auth_admin")) {
                rule = QString::fromLatin1(kAuthorizationRuleAuthenticateAsAdmin);
            }

            CFStringRef cfRule = CFStringCreateWithCString(NULL, rule.toLatin1().constData(), kCFStringEncodingASCII);
            CFStringRef cfPrompt =
                CFStringCreateWithCString(NULL, action.descriptions.value(QLatin1String("en")).toLatin1().constData(), kCFStringEncodingASCII);

            err = AuthorizationRightSet(auth, action.name.toLatin1().constData(), cfRule, cfPrompt, NULL, NULL);
            if (err != noErr) {
                cerr << "You don't have the right to edit the security database (try to run cmake with sudo): " << err << endl;
                exit(1);
            } else {
                qInfo() << "Created or updated rule" << rule << "for right entry" << action.name << "policy" << action.policy << "; domain=" << domain;
            }
        }
    }
}
