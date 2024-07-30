/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BREADCRUMBDIRECTIONWIDGET_H
#define BREADCRUMBDIRECTIONWIDGET_H

#include <QWidget>

class BreadcrumbDirectionWidget : public QWidget
{
    Q_OBJECT
public:
    BreadcrumbDirectionWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
    bool eventFilter(QObject *, QEvent *) override;
};

#endif
