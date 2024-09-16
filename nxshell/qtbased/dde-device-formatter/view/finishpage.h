// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINISHPAGE_H
#define FINISHPAGE_H

#include <QFrame>

class FinishPage : public QFrame
{
    Q_OBJECT
public:
    explicit FinishPage(QWidget *parent = 0);
    void initUI();

signals:

public slots:
};

#endif // FINISHPAGE_H
