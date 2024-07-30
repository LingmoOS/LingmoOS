/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktoolbarspaceraction.h"

#include <QWidget>

class KToolBarSpacerActionPrivate
{
};

KToolBarSpacerAction::KToolBarSpacerAction(QObject *parent)
    : QWidgetAction(parent)
    , d(nullptr)
{
}

KToolBarSpacerAction::~KToolBarSpacerAction() = default;

QWidget *KToolBarSpacerAction::createWidget(QWidget *parent)
{
    if (!parent) {
        return QWidgetAction::createWidget(parent);
    }

    QWidget *spacer = new QWidget(parent);
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    return spacer;
}

#include "moc_ktoolbarspaceraction.cpp"
