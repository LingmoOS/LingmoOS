// Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLELABEL_H
#define TITLELABEL_H

#include <QLabel>

class FcitxTitleLabel : public QLabel
{
    Q_OBJECT
public:
    FcitxTitleLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    FcitxTitleLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
    bool event(QEvent *e) override;
};

#endif // TITLELABEL_H
