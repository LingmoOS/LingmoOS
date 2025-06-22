/*
    SPDX-FileCopyrightText: 2000 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSENDBUGMAIL_MAIN_H
#define KSENDBUGMAIL_MAIN_H

#include <QObject>

class SMTP;

class BugMailer : public QObject
{
    Q_OBJECT
public:
    BugMailer(SMTP *s)
        : QObject(nullptr)
        , sm(s)
    {
        setObjectName(QStringLiteral("mailer"));
    }

public Q_SLOTS:
    void slotError(int);
    void slotSend();

private:
    SMTP *sm;
};

#endif
