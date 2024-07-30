/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#undef QT_NO_CAST_FROM_ASCII

#include <KAuth/Action>
#include <KAuth/ExecuteJob>

#include <QCoreApplication>
#include <QDebug>

using namespace KAuth;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QString filename = "foo.txt";

    //! [client_how_to_call_helper]
    QVariantMap args;
    args["filename"] = filename;
    Action readAction("org.kde.kf6auth.example.read");
    readAction.setHelperId("org.kde.kf6auth.example");
    readAction.setArguments(args);
    ExecuteJob *job = readAction.execute();
    if (!job->exec()) {
        qDebug() << "KAuth returned an error code:" << job->error();
    } else {
        QString contents = job->data()["contents"].toString();
        qDebug() << "KAuth succeeded. Contents: " << contents;
    }
    //! [client_how_to_call_helper]

    return app.exec();
}
