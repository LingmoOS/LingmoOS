// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITLABEL_H
#define EDITLABEL_H

#include "adaptivedef.h"
#include "globaldefine.h"
#include <QWidget>

#ifdef USE_DTK
#include <DLineEdit>
DWIDGET_USE_NAMESPACE
#else
#include <QLineEdit>
#endif

class DRAWLIB_EXPORT EditLabel : public DLineEdit
{
    Q_OBJECT
public:
    explicit EditLabel(QWidget *parent = nullptr);
    ~EditLabel();

};

#endif // EDITLABEL_H
