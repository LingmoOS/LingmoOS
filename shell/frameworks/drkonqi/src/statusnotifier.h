/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DRKONQI_STATUSNOTIFIER_H
#define DRKONQI_STATUSNOTIFIER_H

#include <QObject>

class QTimer;

class KStatusNotifierItem;

class CrashedApplication;

class StatusNotifier : public QObject
{
    Q_OBJECT

public:
    enum class Activation { NotAllowed, Allowed, AlreadySubmitting };

    explicit StatusNotifier(QObject *parent = nullptr);
    ~StatusNotifier() override;

    void show();
    void notify(Activation activation);

    static bool notificationServiceRegistered();

Q_SIGNALS:
    void expired();
    void activated();
    void sentryActivated();

private:
    static bool canBeRestarted(CrashedApplication *app);

    KStatusNotifierItem *m_sni = nullptr;
    QString m_title;
};

#endif // DRKONQI_STATUSNOTIFIER_H
