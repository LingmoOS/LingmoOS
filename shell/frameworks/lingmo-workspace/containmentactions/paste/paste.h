/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <lingmo/containmentactions.h>

class QAction;

class Paste : public Lingmo::ContainmentActions
{
    Q_OBJECT
public:
    Paste(QObject *parent, const QVariantList &args);

    QList<QAction *> contextualActions() override;

private Q_SLOTS:
    void doPaste();

private:
    QAction *const m_action;
};
