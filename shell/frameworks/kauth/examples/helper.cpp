/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#undef QT_NO_CAST_FROM_ASCII

#include <QFile>
#include <QTextStream>
#include <QThread>

//! [helper_declaration]
#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

using namespace KAuth;

class MyHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    ActionReply read(const QVariantMap &args);
    ActionReply write(const QVariantMap &args);
    ActionReply longaction(const QVariantMap &args);
};
//! [helper_declaration]

//! [helper_read_action]
ActionReply MyHelper::read(const QVariantMap &args)
{
    ActionReply reply;
    QString filename = args["filename"].toString();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        reply = ActionReply::HelperErrorReply();
        reply.setErrorDescription(file.errorString());
        return reply;
    }
    QTextStream stream(&file);
    QString contents;
    stream >> contents;
    reply.addData("contents", contents);
    return reply;
}
//! [helper_read_action]

ActionReply MyHelper::write(const QVariantMap &args)
{
    Q_UNUSED(args)
    return ActionReply::SuccessReply();
}

//! [helper_longaction]
ActionReply MyHelper::longaction(const QVariantMap &)
{
    for (int i = 1; i <= 100; i++) {
        if (HelperSupport::isStopped()) {
            break;
        }
        HelperSupport::progressStep(i);
        QThread::usleep(250000);
    }
    return ActionReply::SuccessReply();
}
//! [helper_longaction]

//! [helper_main]
KAUTH_HELPER_MAIN("org.kde.kf6auth.example", MyHelper)
//! [helper_main]

#include "helper.moc"
