// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef XWORKSPACEWORKER_H
#define XWORKSPACEWORKER_H

#include "desktops.h"

#include <QObject>
namespace dock {
class WorkspaceModel;
class XWorkspaceWorker : public QObject
{
    Q_OBJECT
public:
    explicit XWorkspaceWorker(WorkspaceModel *model);

public slots:
    void updateData();
    void setIndex(int index);
    void appearanceChanged(const QString &changedStr, const QString &value);
signals:

private:
    QDBusInterface *m_interKwinProp;
    WorkspaceModel *m_model;

    DBusDesktopDataVector m_desktops;
    QString m_currentId;
    int m_currentIndex;
};
}
#endif // XWORKSPACEWORKER_H
