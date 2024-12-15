// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USER_LIST_WIDGET_H
#define USER_LIST_WIDGET_H

#include <memory>
#include <DListView>

DWIDGET_USE_NAMESPACE

class User;
class SessionBaseModel;
class UserItemDelegate;
class QStandardItemModel;

/*!
 * \brief The UserListPopupWidget class
 */
class UserListPopupWidget : public DListView
{
    Q_OBJECT
public:
    explicit UserListPopupWidget(const SessionBaseModel *model, QWidget *parent = nullptr);

signals:
    void requestSwitchToUser(std::shared_ptr<User> user);

protected:
    void mouseMoveEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void userInfoChanged();
    void currentUserChanged(const std::shared_ptr<User> &user);

private:
    void initUI();
    void initConnections();
    void loadUsers();

    void handlerBeforeAddUser(const std::shared_ptr<User> &user);
    void addUser(const std::shared_ptr<User> &user);
    void removeUser(const std::shared_ptr<User> &user);
    void setItemData(QStandardItem *item, const User *user);
    QString accountStrType(int accountType) const;

    void updateViewWidth();
    void updateViewHeight();
    int calculateItemWidth();
    int getItemHeight();
    int stringWidth(const QString &str, int fontSize, bool isBold = false);

private:
    const SessionBaseModel *m_model;
    UserItemDelegate *m_userItemDelegate;
    QStandardItemModel *m_userItemModel;

    QMap<uid_t, QStandardItem *> m_userItemMap;
    std::shared_ptr<User> m_currentUser;
};

#endif // USER_LIST_WIDGET_H
