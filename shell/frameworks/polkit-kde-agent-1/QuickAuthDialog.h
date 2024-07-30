/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QUICKAUTHDIALOG_H
#define QUICKAUTHDIALOG_H

#include <QObject>
#include <QWindow>

#include <PolkitQt1/Details>
#include <PolkitQt1/Identity>

class QuickAuthDialog : public QObject
{
    Q_OBJECT
public:
    QuickAuthDialog(const QString &actionId, const QString &message, const PolkitQt1::Details &details, const PolkitQt1::Identity::List &identities);

    QWindow *windowHandle() const
    {
        return m_theDialog;
    }
    PolkitQt1::Identity adminUserSelected() const;

    QString actionId() const;
    QString password() const;
    void showError(const QString &message);
    void showInfo(const QString &message);
    void authenticationFailure();
    void show();
    void hide();

Q_SIGNALS:
    void okClicked();
    void rejected();

private:
    QWindow *m_theDialog = nullptr;
    QString m_actionId;
};

#endif
