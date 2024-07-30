/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIDENTITYPROXYMODELWIDGET_H
#define KIDENTITYPROXYMODELWIDGET_H

#include <QWidget>

class KIdentityProxyModelWidget : public QWidget
{
    Q_OBJECT
public:
    KIdentityProxyModelWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};

#endif
