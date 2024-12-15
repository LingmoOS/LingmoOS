// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPEVENTFILTER_H
#define APPEVENTFILTER_H

#include <QObject>
#include <QEvent>

class AppEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit AppEventFilter(QObject *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void userIsActive();
};

#endif // APPEVENTFILTER_H
