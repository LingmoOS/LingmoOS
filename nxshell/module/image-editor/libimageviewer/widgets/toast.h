// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <QFrame>
#include <QIcon>

#include "dtkwidget_global.h"
#include <DObject>
#include <DObjectPrivate>

#include <QLabel>
#include <DIconButton>
#include "imagebutton.h"
#include <dimagebutton.h>
#include "dthememanager.h"
#include "dgraphicsgloweffect.h"
#include <QPropertyAnimation>

DWIDGET_BEGIN_NAMESPACE

class ToastPrivate;
class LIBDTKWIDGETSHARED_EXPORT Toast : public QFrame, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit Toast(QWidget *parent = Q_NULLPTR);
    ~Toast();

    QString text() const;
    QIcon icon() const;

public Q_SLOTS:
//    void pop();
//    void pack();

    void setText(QString text);
    void setIcon(QString icon);
    void setIcon(QIcon icon, QSize defaultSize = QSize(20, 20));

private:
    qreal opacity() const;
    void setOpacity(qreal);

    D_DECLARE_PRIVATE(Toast)
};
class ToastPrivate: public DTK_CORE_NAMESPACE::DObjectPrivate

{
public:
    explicit ToastPrivate(Toast *qq);

    QIcon   icon;
    QLabel  *iconLabel      = Q_NULLPTR;
    QLabel  *textLabel      = Q_NULLPTR;

    ImageButton *closeBt    = Q_NULLPTR;

    QPropertyAnimation  *animation  = Q_NULLPTR;
    DGraphicsGlowEffect *effect     = Q_NULLPTR;

    void initUI();
private:
    D_DECLARE_PUBLIC(Toast)
};

DWIDGET_END_NAMESPACE
