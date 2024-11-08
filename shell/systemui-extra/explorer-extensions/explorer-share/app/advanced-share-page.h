/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef ADVANCEDSHAREPAGE_H
#define ADVANCEDSHAREPAGE_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFutureWatcher>
#include <QListWidget>
#include <QMutex>
#include <QFrame>
#include <file-info.h>
#include "file-utils.h"
#include "systemd-bus-accounts.h"
using namespace Peony;

class AdvancedSharePage : public QWidget
{
    Q_OBJECT
public:
    explicit AdvancedSharePage(const QString &uri, const QMap<QString, QString> &userInfo, QWidget *parent = nullptr);
    ~AdvancedSharePage();

    void init();
    void initTableWidget();
    void initListWidget();
    void initFloorOne();
    void initFloorTwo();
    void initFloorThree();
    void initFloorFour();
    void addSeparate();

    void updateShareUserInfo();
    void parseUserShareAcl(QString strAcl);
    bool updateCheckBox(int col, QString &name);
    void updateUserInfo(int row, int col, bool checked);
    QString parseSetAcl(QMap<QString, QString> &userInfo);
    QString converPermission(QString &usershareAcl);

protected Q_SLOTS:
    void updateDelBtnState(int row, int col);

Q_SIGNALS:
    void getUserInfo(QString &acl, QMap<QString, QString> &userInfo);

private:
    QString m_uri;
    QMap<QString, QString> m_userInfo;
    QMutex m_mutex;
    SystemDbusAccounts *m_sysAccounts = nullptr;
    QStringList m_userNames;
    bool m_thisPageChanged = false;

    QLabel *m_label = nullptr;
    QLabel *m_tabLabel = nullptr;
    QLabel *m_listLabel = nullptr;
    QTableWidget *m_tabWidget = nullptr;
    QListWidget *m_listWidget = nullptr;
    QPushButton *m_saveBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
    QPushButton *m_addShareUserBtn = nullptr;
    QPushButton *m_delShareUserBtn = nullptr;
    QVBoxLayout *m_layout = nullptr;
};

#endif // ADVANCEDSHAREPAGE_H
