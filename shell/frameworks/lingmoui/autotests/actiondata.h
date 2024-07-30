// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QAction>
#include <QObject>
#include <qqmlregistration.h>

class ActionData : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QAction *enabledAction READ enabledAction CONSTANT)
    Q_PROPERTY(QAction *disabledAction READ disabledAction CONSTANT)

public:
    explicit ActionData(QObject *parent = nullptr);

    QAction *enabledAction() const;
    QAction *disabledAction() const;

private:
    QAction *const m_enabledAction;
    QAction *const m_disabledAction;
};
