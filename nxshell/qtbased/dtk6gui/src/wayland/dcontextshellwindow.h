// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkgui_global.h>
#include <QWindow>

DGUI_BEGIN_NAMESPACE
class DContextShellWindowPrivate;

class DContextShellWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int noTitlebar READ noTitlebar WRITE setNoTitlebar NOTIFY noTitlebarChanged)

public:
    ~DContextShellWindow() override;

    bool noTitlebar();
    void setNoTitlebar(bool value);

    static DContextShellWindow *get(QWindow *window);
    static DContextShellWindow *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void noTitlebarChanged();

private:
    DContextShellWindow(QWindow *window);
    QScopedPointer<DContextShellWindowPrivate> d;
};
DGUI_END_NAMESPACE
