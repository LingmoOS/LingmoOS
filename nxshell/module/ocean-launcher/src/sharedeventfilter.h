// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREDEVENTFILTER_H
#define SHAREDEVENTFILTER_H

#include <QObject>
#include <QKeyEvent>

#include "launcherinterface.h"

class SharedEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit SharedEventFilter(QObject *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    bool handleKeyEvent(QKeyEvent *event);

private:
    LauncherInterface *m_frame;
};

#endif // SHAREDEVENTFILTER_H
