// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WARNINGCONTENT_H
#define WARNINGCONTENT_H

#include <QObject>
#include <QWidget>

#include "sessionbasewindow.h"
#include "sessionbasemodel.h"
#include "warningview.h"
#include "inhibitwarnview.h"
#include "multiuserswarningview.h"
#include "dbus/dbuslogin1manager.h"

class WarningContent : public SessionBaseWindow
{
    Q_OBJECT

public:
    explicit WarningContent(SessionBaseModel *const model, const SessionBaseModel::PowerAction action, QWidget *parent = nullptr);
    ~WarningContent() override;
    void beforeInvokeAction(bool needConfirm);
    void setPowerAction(const SessionBaseModel::PowerAction action);

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    QList<InhibitWarnView::InhibitorData> listInhibitors(const SessionBaseModel::PowerAction action);
    void doCancelShutdownInhibit();
    void doAccecpShutdownInhibit();

private:
    SessionBaseModel *m_model;
    DBusLogin1Manager *m_login1Inter;
    WarningView * m_warningView = nullptr;
    QStringList m_inhibitorBlacklists;
    SessionBaseModel::PowerAction m_powerAction;
};

class InhibitHint
{
public:
    QString name, icon, why;

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitHint &obj)
    {
        argument.beginStructure();
        argument >> obj.name >> obj.icon >> obj.why;
        argument.endStructure();
        return argument;
    }
};

#endif // WARNINGCONTENT_H
