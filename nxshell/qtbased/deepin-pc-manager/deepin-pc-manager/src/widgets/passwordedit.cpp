// Copyright (C) 2019 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/passwordedit.h"

#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

PasswordEdit::PasswordEdit(QWidget *parent)
    : DPasswordEdit(parent)
{
    lineEdit()->setAcceptDrops(false);
    lineEdit()->setContextMenuPolicy(Qt::NoContextMenu);
    lineEdit()->installEventFilter(this);
}

bool PasswordEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == lineEdit() && event->type() == QEvent::KeyPress) {
        QKeyEvent *e = dynamic_cast<QKeyEvent *>(event);
        if (e
            && (e->matches(QKeySequence::Copy) || e->matches(QKeySequence::Cut)
                || e->matches(QKeySequence::Paste))) {
            return true;
        }
    }

    return DPasswordEdit::eventFilter(obj, event);
}
