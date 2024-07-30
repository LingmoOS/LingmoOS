/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QQuickView>
#include <QRect>
#include <QString>

#include <memory>

class QQuickView;

class QTimer;

namespace PowerDevil
{
class Osd : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    void showActionSelector(const QString &currentProfile = QString());
    void hideOsd();

Q_SIGNALS:
    void osdActionSelected(QString action);
    void osdActiveChanged(bool active);

private Q_SLOTS:
    void onOsdActionSelected(const QString &action);

private:
    QQmlEngine m_engine;
    std::unique_ptr<QQuickView> m_osdActionSelector;
};

} // namespace PowerDevil
