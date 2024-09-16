// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POLICYRESULT_H
#define POLICYRESULT_H

#include <QDBusMetaType>
#include <QString>
#include <QDBusArgument>
#include <QDebug>

class PolicyResult
{
public:
    bool isAuthorized;
    bool isChallenge;
    QMap<QString, QString> stringList;

    inline bool getAuthorized() const { return isAuthorized; }
    inline bool getChallenge() const { return isChallenge; }

    friend QDebug operator<<(QDebug argument, const PolicyResult &result) {
        argument << result.isAuthorized << result.isChallenge << result.stringList;

        return argument;
    }

    friend QDBusArgument &operator<<(QDBusArgument &argument, const PolicyResult &result) {
        argument.beginStructure();
        argument << result.isAuthorized << result.isChallenge << result.stringList;
        argument.endStructure();

        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, PolicyResult &result) {
        argument.beginStructure();
        argument >> result.isAuthorized >> result.isChallenge >> result.stringList;
        argument.endStructure();

        return argument;
    }

    bool operator==(const PolicyResult result) const {
        return result.isAuthorized == isAuthorized && result.isChallenge == isChallenge && result.stringList == stringList;
    }

    bool operator!=(const PolicyResult result) const {
        return result.isAuthorized != isAuthorized || result.isChallenge != isChallenge || result.stringList != stringList;
    }
};

Q_DECLARE_METATYPE(PolicyResult)

void registerPolicyResultMetaType();

#endif // POLICYRESULT_H
