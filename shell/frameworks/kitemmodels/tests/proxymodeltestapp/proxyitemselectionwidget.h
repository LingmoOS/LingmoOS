/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROXYITEMSELECTIONWIDGET_H
#define PROXYITEMSELECTIONWIDGET_H

#include <QWidget>

class ProxyItemSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    ProxyItemSelectionWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};

#endif
