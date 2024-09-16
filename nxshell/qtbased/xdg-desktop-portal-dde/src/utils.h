// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QWidget>
#include <QWindow>

#define DECLEAR_PARA_WITH_FALLBACK(para, toFun, fallback) \
    auto para = options.value(QStringLiteral("#para"), fallback).toFun();

#define DECLEAR_PARA(para, toFun) \
    DECLEAR_PARA_WITH_FALLBACK(para, toFun, QVariant())

class Utils : public QObject
{
public:
    static void setParentWindow(QWidget *w, const QString &parent_window);
    static void setParentWindow(QWindow *w, const QString &parent_window);
};

#endif // UTILS_H
