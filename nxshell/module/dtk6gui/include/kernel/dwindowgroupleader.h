// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWINDOWGROUPLEADER_H
#define DWINDOWGROUPLEADER_H

#include <dtkgui_global.h>

#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DWindowGroupLeaderPrivate;
class DWindowGroupLeader
{
public:
    explicit DWindowGroupLeader(quint32 groupId = 0);
    ~DWindowGroupLeader();

    quint32 groupLeaderId() const;
    quint32 clientLeaderId() const;

    void addWindow(QWindow *window);
    void removeWindow(QWindow *window);

private:
    QScopedPointer<DWindowGroupLeaderPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DWindowGroupLeader)
};

DGUI_END_NAMESPACE

#endif // DWINDOWGROUPLEADER_H
