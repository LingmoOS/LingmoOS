// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userlistpopupwidget.h"
#include "sessionbasemodel.h"
#include "useritemdelegate.h"

#include <DFontSizeManager>

#include <QMap>
#include <QKeyEvent>
#include <QStandardItem>

DWIDGET_USE_NAMESPACE

const int ITEM_MAX_WIDTH = 365;
const int ITEM_MIN_WIDTH = 260;
const int ITEM_HEIGHT = 64;
const int MAX_ROW = 5;
const int ITEM_SPACING = 5;
const int MAX_HEIGHT = ITEM_HEIGHT * MAX_ROW + (MAX_ROW * 2) * ITEM_SPACING;

UserListPopupWidget::UserListPopupWidget(const SessionBaseModel *model, QWidget *parent)
    : DListView(parent)
    , m_model(model)
    , m_userItemDelegate(new UserItemDelegate(this))
    , m_userItemModel(new QStandardItemModel(this))
{
    initUI();
    initConnections();

    loadUsers();
}

void UserListPopupWidget::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        blockSignals(true);
        setCurrentIndex(index);
        blockSignals(false);

        update();
    }

    DListView::mouseMoveEvent(event);
}

void UserListPopupWidget::keyPressEvent(QKeyEvent *event)
{
    const QModelIndex currentIndex = this->currentIndex();
    int row = currentIndex.row();
    QModelIndex targetIndex;

    switch (event->key()) {
    case Qt::Key_Up:
        if (row)
            targetIndex = currentIndex.sibling(row - 1, 0);
        else
            targetIndex = currentIndex.sibling(m_userItemModel->rowCount() - 1, 0);
        break;
    case Qt::Key_Down:
        if ((row + 1) != m_userItemModel->rowCount())
            targetIndex = currentIndex.sibling(row + 1, 0);
        else
            targetIndex = currentIndex.sibling(0, 0);
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (currentIndex.isValid()) {
            Q_EMIT clicked(currentIndex);
        }
        break;
    }

    if (targetIndex.isValid()) {
        blockSignals(true);
        setCurrentIndex(targetIndex);
        blockSignals(false);

        // 更新避免选中背景色上下移动后有残影
        update();
    }

    DListView::keyPressEvent(event);
}

void UserListPopupWidget::showEvent(QShowEvent *event)
{
    setFocus();

    QWidget::showEvent(event);
}

void UserListPopupWidget::handlerBeforeAddUser(const std::shared_ptr<User> &user)
{
    // 处理服务器模式
    if (m_model->isServerModel()) {
        if (user->isLogin() || user->type() == User::Default)
            addUser(user);

        connect(user.get(), &User::loginStateChanged, this, [ = ](bool isLogined) {
            isLogined ? addUser(user) : removeUser(user);
        });

        return;
    }

    addUser(user);
}

void UserListPopupWidget::addUser(const std::shared_ptr<User> &user)
{
    if (!user.get())
        return;

    QStandardItem *item = new QStandardItem();
    setItemData(item, user.get());
    m_userItemModel->appendRow(item);

    m_userItemMap[user->uid()] = item;

    connect(user.get(), &User::avatarChanged, this, &UserListPopupWidget::userInfoChanged);
    connect(user.get(), &User::displayNameChanged, this, &UserListPopupWidget::userInfoChanged);
    connect(user.get(), &User::loginStateChanged, this, &UserListPopupWidget::userInfoChanged);

    // 新增一个用户，可能改变整个view中item的宽度
    updateViewWidth();

    updateViewHeight();
}

void UserListPopupWidget::removeUser(const std::shared_ptr<User> &user)
{
    if (!user.get())
        return;

    disconnect(user.get(), &User::avatarChanged, this, &UserListPopupWidget::userInfoChanged);
    disconnect(user.get(), &User::displayNameChanged, this, &UserListPopupWidget::userInfoChanged);
    disconnect(user.get(), &User::loginStateChanged, this, &UserListPopupWidget::userInfoChanged);

    if (m_userItemMap.contains(user->uid()) && m_userItemMap[user->uid()]) {
        m_userItemModel->removeRow(m_userItemMap[user->uid()]->row());
    }

    // 删除一个用户，可能改变整个view中item的宽度
    updateViewWidth();

    updateViewHeight();
}

void UserListPopupWidget::setItemData(QStandardItem *item, const User *user)
{
    if (!item || !user)
        return;

    UserItemDelegate::UserItemData itemData;
    itemData.imagePath = user->avatar();
    itemData.displayName = user->displayName();
    // TODO 这块需要结合配置来决定是否显示，待后续配置出来再放开
    itemData.name = "";//user->fullName().isEmpty() ? "" : user->name();
    itemData.userStrType = accountStrType(user->accountType());
    itemData.userType = user->accountType();
    itemData.isLogined = user->isLogin();
    itemData.userId = user->uid();

    item->setData(QVariant::fromValue(itemData), UserItemDelegate::StaticUserDataRole);
    bool isCurrentUser = (m_model->currentUser() && user->uid() == m_model->currentUser()->uid());
    item->setData(isCurrentUser, UserItemDelegate::IsCurrentUserDataRole);
}

void UserListPopupWidget::initUI()
{
    setMouseTracking(true);

    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setViewportMargins(5, 5, 5, 5);
    setMaximumHeight(MAX_HEIGHT);
    setSpacing(0);

    setItemDelegate(m_userItemDelegate);
    setModel(m_userItemModel);
}

void UserListPopupWidget::initConnections()
{
    connect(m_model, &SessionBaseModel::userAdded, this, &UserListPopupWidget::handlerBeforeAddUser);
    connect(m_model, &SessionBaseModel::userRemoved, this, &UserListPopupWidget::removeUser);
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &UserListPopupWidget::currentUserChanged);

    connect(this, &DListView::clicked, [ this ](const QModelIndex &index) {
        if (!index.isValid())
            return;

        UserItemDelegate::UserItemData data = index.data(UserItemDelegate::StaticUserDataRole).value<UserItemDelegate::UserItemData>();
        if (m_model->currentUser()->uid() == data.userId)
            return;

        qInfo() << "request switch user id:" << data.userId << " displayName:" << data.displayName;
        Q_EMIT requestSwitchToUser(m_model->findUserByUid(data.userId));
    });

    connect(qGuiApp, &QGuiApplication::fontChanged, this, [ this ] {
        updateViewHeight();
    });
}

void UserListPopupWidget::loadUsers()
{
    m_currentUser = m_model->currentUser();

    QList<std::shared_ptr<User>> userList = m_model->userList();
    for (const auto &user : userList) {
        handlerBeforeAddUser(user);
    }
}

void UserListPopupWidget::userInfoChanged()
{
    User *user = qobject_cast<User *>(sender());
    if (!user)
        return;

    if (!m_userItemMap.contains(user->uid()) || !m_userItemMap[user->uid()])
        return;

    QStandardItem *item = m_userItemMap[user->uid()];
    setItemData(item, user);

    update(item->index());

    updateViewWidth();
}

void UserListPopupWidget::currentUserChanged(const std::shared_ptr<User> &user)
{
    if (m_currentUser->uid() == user->uid())
        return;

    if (m_userItemMap.contains(m_currentUser->uid()) && m_userItemMap[m_currentUser->uid()]) {
        m_userItemMap[m_currentUser->uid()]->setData(false, UserItemDelegate::IsCurrentUserDataRole);
        update(m_userItemMap[m_currentUser->uid()]->index());
    }

    if (m_userItemMap.contains(user->uid()) && m_userItemMap[user->uid()]) {
        m_userItemMap[user->uid()]->setData(true, UserItemDelegate::IsCurrentUserDataRole);
        update(m_userItemMap[user->uid()]->index());
    }

    m_currentUser = user;

    for (const auto &item : m_userItemMap) {
        UserItemDelegate::UserItemData userData = item->data(UserItemDelegate::StaticUserDataRole).value<UserItemDelegate::UserItemData>();
        userData.userStrType = accountStrType(userData.userType);
        item->setData(QVariant::fromValue(userData), UserItemDelegate::StaticUserDataRole);
    }
}

// 设计给出item的最小宽度和最大长度，需要根据内容来计算item大小
int UserListPopupWidget::calculateItemWidth()
{
    // 设计图上常量
    const int textLeftMargin = 65;  // 文字内容距离左边距
    const int textRightMargin = 48; // 文字内容距离右边距
    const int itemSpacing = 10;

    int displayNameFontSize = DFontSizeManager::instance()->t4().pixelSize();
    int nameFontSize = DFontSizeManager::instance()->t6().pixelSize();
    int typeFontSize = DFontSizeManager::instance()->t8().pixelSize();;

    int textAreaWidth = ITEM_MIN_WIDTH - textLeftMargin - textRightMargin;

    QList<std::shared_ptr<User>> userList = m_model->userList();
    for (const auto &user : userList) {
        int displayNameWidth = stringWidth(user->displayName(), displayNameFontSize, true);
        QString name = user->fullName().isEmpty() ? "" : user->name();
        int nameAndTypeWidth = stringWidth(name, nameFontSize) + itemSpacing +
                stringWidth(accountStrType(user->accountType()), typeFontSize);

        int maxWidth = qMax(displayNameWidth, nameAndTypeWidth);
        textAreaWidth = qMax(textAreaWidth, maxWidth);
    }

    return qMin((textAreaWidth + textLeftMargin + textRightMargin), ITEM_MAX_WIDTH);
}

int UserListPopupWidget::stringWidth(const QString &str, int fontSize, bool isBold)
{
    if (str.isEmpty())
        return 0;

    QFont font;
    font.setBold(isBold);
    font.setPixelSize(fontSize);

    QFontMetrics fm(font);
    return fm.boundingRect(str).width();
}

void UserListPopupWidget::updateViewWidth()
{
    int itemWidth = calculateItemWidth();

    setFixedWidth(itemWidth + ITEM_SPACING * 2);
    update();
}

void UserListPopupWidget::updateViewHeight()
{
    int height = m_userItemModel->rowCount() * getItemHeight();
    setFixedHeight(qMin(height, MAX_HEIGHT));
}

QString UserListPopupWidget::accountStrType(int accountType) const
{
    // 0:标准用户 1:管理员 2:域账户
    switch (accountType) {
        case User::Standard:
            return tr("Standard User");
        case User::Admin:
            return tr("Administrator");
        case User::Other:
            return tr("Other Users");
        default:
            return "";
    }
}

int UserListPopupWidget::getItemHeight()
{
    int height = ITEM_SPACING * 2 + UserItemDelegate::displayNameHeight() + 2 + UserItemDelegate::userTypeHeight() + ITEM_SPACING * 2;

    return height > ITEM_HEIGHT ? height : ITEM_HEIGHT;
}
