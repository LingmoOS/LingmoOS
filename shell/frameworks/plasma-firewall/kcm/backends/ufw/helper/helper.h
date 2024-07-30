// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
#ifndef UFW_HELPER_H
#define UFW_HELPER_H

/*
 * UFW KControl Module
 */

#include <KAuth/ActionReply>
#include <QObject>
#include <QVariantMap>
#include <QtContainerFwd>

class QByteArray;

using namespace KAuth;

namespace UFW
{

class Helper : public QObject
{
    Q_OBJECT

public:
    enum Status {
        STATUS_OK,
        STATUS_INVALID_CMD = -100,
        STATUS_INVALID_ARGUMENTS = -101,
        STATUS_OPERATION_FAILED = -102,
    };

public Q_SLOTS:

    ActionReply query(const QVariantMap &args);
    ActionReply viewlog(const QVariantMap &args);
    ActionReply modify(const QVariantMap &args);

    ActionReply queryapps(const QVariantMap &args);

private:
    ActionReply setStatus(const QVariantMap &args, const QString &cmd);
    ActionReply setDefaults(const QVariantMap &args, const QString &cmd);
    ActionReply setModules(const QVariantMap &args, const QString &cmd);
    ActionReply setProfile(const QVariantMap &args, const QString &cmd);
    ActionReply saveProfile(const QVariantMap &args, const QString &cmd);
    ActionReply deleteProfile(const QVariantMap &args, const QString &cmd);
    ActionReply addRules(const QVariantMap &args, const QString &cmd);
    ActionReply removeRule(const QVariantMap &args, const QString &cmd);
    ActionReply moveRule(const QVariantMap &args, const QString &cmd);
    ActionReply editRule(const QVariantMap &args, const QString &cmd);
    //     ActionReply editRuleDescr(const QVariantMap &args, const QString &cmd);
    ActionReply reset(const QString &cmd);
    ActionReply run(const QStringList &args, const QString &cmd);
    ActionReply run(const QStringList &args, const QStringList &second, const QString &cmd);

private:
};

}

#endif
