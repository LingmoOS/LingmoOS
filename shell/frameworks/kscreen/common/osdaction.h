/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>


    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QString>

namespace KScreen
{
class Config;
class SetConfigOperation;

struct OsdAction {
    Q_GADGET
    Q_PROPERTY(QString label MEMBER label CONSTANT)
    Q_PROPERTY(QString iconName MEMBER iconName CONSTANT)
    Q_PROPERTY(Action action MEMBER action CONSTANT)
public:
    enum Action {
        NoAction,
        SwitchToExternal,
        SwitchToInternal,
        Clone,
        ExtendLeft,
        ExtendRight,
    };
    Q_ENUM(Action)

    Action action;
    QString label;
    QString iconName;

    static QList<OsdAction> availableActions();
    static KScreen::SetConfigOperation *applyAction(const QSharedPointer<KScreen::Config> &config, Action action);
};

} // namespace KScreen
