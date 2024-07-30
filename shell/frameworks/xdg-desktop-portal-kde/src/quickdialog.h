/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QEventLoop>
#include <QObject>

class QWindow;

class QuickDialog : public QObject
{
    Q_OBJECT
public:
    QuickDialog(QObject *parent = nullptr);
    ~QuickDialog() override;

    QWindow *windowHandle() const
    {
        return m_theDialog;
    }
    bool exec();

    void create(const QString &file, const QVariantMap &props);

public Q_SLOTS:
    void reject();
    virtual void accept();

protected:
    QWindow *m_theDialog = nullptr;
    QEventLoop m_execLoop;
};
