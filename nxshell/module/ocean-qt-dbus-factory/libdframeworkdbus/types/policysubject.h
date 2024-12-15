// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POLICYSUBJECT_H
#define POLICYSUBJECT_H

#include <QDBusMetaType>
#include <QString>
#include <QDBusArgument>
#include <QDebug>

class PolicySubject
{
public:
    QString name;
    QMap<QString, QVariant> sub;

    friend QDebug operator<<(QDebug argument, const PolicySubject &subject) {
        argument << subject.name;

        return argument;
    }

    friend QDBusArgument &operator<<(QDBusArgument &argument, const PolicySubject &subject) {
        argument.beginStructure();
        argument << subject.name << subject.sub;
        argument.endStructure();

        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, PolicySubject &subject) {
        argument.beginStructure();
        argument >> subject.name >> subject.sub;
        argument.endStructure();

        return argument;
    }

    bool operator==(const PolicySubject subject) const {
        return subject.name == name;
    }

    bool operator!=(const PolicySubject subject) const {
        return subject.name != name;
    }
};

Q_DECLARE_METATYPE(PolicySubject)

void registerPolicySubjectMetaType();

#endif // POLICYSUBJECT_H
