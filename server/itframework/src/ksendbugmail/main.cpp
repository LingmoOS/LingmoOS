/*
    SPDX-FileCopyrightText: 2000 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "main.h"
#include <qplatformdefs.h>

#include <QApplication>
#include <QHostInfo>
#include <QTextStream>

#include <KEMailSettings>
#include <KLocalizedString>

#include <qcommandlineoption.h>
#include <qcommandlineparser.h>

#include "../systeminformation_p.h"
#include "smtp.h"

void BugMailer::slotError(int errornum)
{
    QString lstr;

    switch (errornum) {
    case SMTP::ConnectError:
        lstr = i18n("Error connecting to server.");
        break;
    case SMTP::NotConnected:
        lstr = i18n("Not connected.");
        break;
    case SMTP::ConnectTimeout:
        lstr = i18n("Connection timed out.");
        break;
    case SMTP::InteractTimeout:
        lstr = i18n("Time out waiting for server interaction.");
        break;
    default:
        lstr = QString::fromLatin1(sm->getLastLine().trimmed());
        lstr = i18n("Server said: \"%1\"", lstr);
    }
    // qCDebug(DEBUG_KXMLGUI) << lstr;

    fputs(lstr.toUtf8().data(), stdout);
    fflush(stdout);

    qApp->exit(1);
}

void BugMailer::slotSend()
{
    // qCDebug(DEBUG_KXMLGUI);
    qApp->exit(0);
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("ksendbugmail"));
    a.setApplicationVersion(QStringLiteral("1.0"));

    KLocalizedString::setApplicationDomain("kxmlgui5");

    // d.addAuthor(ki18n("Stephan Kulow"), ki18n("Author"), "coolo@kde.org");

    QString subject;
    QString recipient;
    {
        QCommandLineParser parser;
        parser.addVersionOption();
        parser.setApplicationDescription(i18n("Sends a bug report by email."));
        parser.addHelpOption();
        parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("subject"), i18n("The subject line of the email."), QStringLiteral("argument")));
        parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("recipient"),
                                            i18n("The email address to send the bug report to."),
                                            QStringLiteral("argument"),
                                            QStringLiteral("submit@bugs.kde.org")));
        parser.process(a);
        recipient = parser.value(QStringLiteral("recipient"));
        subject = parser.value(QStringLiteral("subject"));
    }
    if (recipient.isEmpty()) {
        recipient = QStringLiteral("submit@bugs.kde.org");
    } else {
        if (recipient.at(0) == QLatin1Char('\'')) {
            recipient = recipient.mid(1, recipient.length() - 2);
        }
    }
    // qCDebug(DEBUG_KXMLGUI) << "recp" << recipient;

    if (subject.isEmpty()) {
        subject = QStringLiteral("(no subject)");
    } else {
        if (subject.at(0) == QLatin1Char('\'')) {
            subject = subject.mid(1, subject.length() - 2);
        }
    }
    QTextStream input(stdin, QIODevice::ReadOnly);
    // The default in Qt6 is UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    input.setCodec("UTF-8");
#endif
    QString text;
    QString line;
    while (!input.atEnd()) {
        line = input.readLine();
        text += line + QLatin1String("\r\n");
    }
    // qCDebug(DEBUG_KXMLGUI) << text;

    KEMailSettings emailConfig;
    emailConfig.setProfile(emailConfig.defaultProfileName());
    QString fromaddr = emailConfig.getSetting(KEMailSettings::EmailAddress);
    if (!fromaddr.isEmpty()) {
        QString name = emailConfig.getSetting(KEMailSettings::RealName);
        if (!name.isEmpty()) {
            fromaddr = name + QLatin1String(" <") + fromaddr + QLatin1Char('>');
        }
    } else {
        fromaddr = SystemInformation::userName() + QLatin1Char('@') + QHostInfo::localHostName();
    }
    // qCDebug(DEBUG_KXMLGUI) << "fromaddr \"" << fromaddr << "\"";

    QString server = emailConfig.getSetting(KEMailSettings::OutServer);
    if (server.isEmpty()) {
        server = QStringLiteral("bugs.kde.org");
    }

    SMTP *sm = new SMTP;
    BugMailer bm(sm);

    QObject::connect(sm, &SMTP::messageSent, &bm, &BugMailer::slotSend);
    QObject::connect(sm, &SMTP::error, &bm, &BugMailer::slotError);
    sm->setServerHost(server);
    sm->setPort(25);
    sm->setSenderAddress(fromaddr);
    sm->setRecipientAddress(recipient);
    sm->setMessageSubject(subject);
    sm->setMessageHeader(QStringLiteral("From: %1\r\nTo: %2\r\n").arg(fromaddr, recipient));
    sm->setMessageBody(text);
    sm->sendMessage();

    int r = a.exec();
    // qCDebug(DEBUG_KXMLGUI) << "execing " << r;
    delete sm;
    return r;
}
