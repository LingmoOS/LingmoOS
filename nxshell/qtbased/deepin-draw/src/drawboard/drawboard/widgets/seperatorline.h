// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEPERATORLINE_H
#define SEPERATORLINE_H

#include "adaptivedef.h"

#include <QWidget>
#include <QDebug>


#ifdef USE_DTK
#include <DLabel>
DWIDGET_USE_NAMESPACE
#else
#include <QLabel>
#endif
class SeperatorLine : public DLabel
{
    Q_OBJECT
public:
    explicit SeperatorLine(QWidget *parent = nullptr);

    ~SeperatorLine();

    void updateTheme();

};
#endif // SEPERATORLINE_H
