// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WARNPAGE_H
#define WARNPAGE_H

#include <QFrame>

class WarnPage : public QFrame
{
    Q_OBJECT
public:
    explicit WarnPage(QWidget *parent = 0);
    void initUI();

signals:

public slots:
};

#endif // WARNPAGE_H
