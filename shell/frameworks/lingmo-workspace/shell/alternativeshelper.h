/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Lingmo/Applet>

#include <QQuickItem>

class AlternativesHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList appletProvides READ appletProvides CONSTANT)
    Q_PROPERTY(QString currentPlugin READ currentPlugin CONSTANT)
    Q_PROPERTY(QQuickItem *applet READ applet CONSTANT)

public:
    explicit AlternativesHelper(Lingmo::Applet *applet, QObject *parent = nullptr);
    ~AlternativesHelper() override;

    QQuickItem *applet() const;
    QStringList appletProvides() const;
    QString currentPlugin() const;

    Q_INVOKABLE void loadAlternative(const QString &plugin);

private:
    Lingmo::Applet *m_applet;
};
