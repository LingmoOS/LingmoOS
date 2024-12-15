#ifndef ACCOUNTSWIDGET_H
#define ACCOUNTSWIDGET_H

#include "model/accounts_info_model.h"
#include "onlineicon.h"
#include "user_list_view.h"
#include "kill_process_confirm_dialog.h"

#include <QWidget>
#include <DListView>
#include <QDebug>
#include <QIcon>
#include <QSize>
#include <QPainter>
#include <QPixmap>
DWIDGET_USE_NAMESPACE
using LogoutConfirmDialog = KillProcessConfirmDialog;

class AccountsWidget  : public QWidget
{
    Q_OBJECT

public:

    enum AccountRole {
        ItemDataRole = Dtk::UserRole + 1
    };


    explicit AccountsWidget(QWidget *parent = nullptr);
    virtual ~AccountsWidget() override;
    void    addInfo(AccountsInfoModel *model) ;
    QPixmap pixmapToRound(const QPixmap &src);
    void addUser(User *user);
    void removeUser(User *user);
    void showDefaultAccountInfo();
    void initUI();
    void initConnection();


    QString getCurrentItemUserName();
    void onRightButtonClicked(const QPoint &p);

protected:
//    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
//    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
//    void keyPressEvent(QKeyEvent *event) override;

    void getUserToBeOperated(const QString &userName);
Q_SIGNALS:
    void signalCurrentChanged();
public Q_SLOTS:
    void onUpdateUserList();
    void onItemClicked(const QModelIndex &);
    void onDisconnectTriggered();
    void onConnectTriggered();
    void onLogoutTriggered();
    void onEditAccountTriggered();
private:
    AccountsInfoModel *m_userModel;
    QStandardItemModel *m_userItemModel;
    QList<User *> m_userList;
    UserListView *m_userlistView;
    QList<OnlineIcon *> m_onlineIconList;
    // User Login Control menu
    DMenu *m_contextMenu {};
    //User to be operated
    User *m_userToBeOperated {};
    User::UserType m_currentUserType;


};
#endif // ACCOUNTSWIDGET_H
