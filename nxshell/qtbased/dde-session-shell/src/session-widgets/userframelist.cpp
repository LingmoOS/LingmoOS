// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userframelist.h"

#include "dflowlayout.h"
#include "framedatabind.h"
#include "sessionbasemodel.h"
#include "user_widget.h"
#include "userinfo.h"

#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QVBoxLayout>

#include <algorithm>

static constexpr int UserWidgetSpacing = 40;
static constexpr int ContentsMargin = 10;

static inline int listWidth(int col)
{
    return qMax(0, (UserFrameWidth + UserWidgetSpacing) * col - UserWidgetSpacing);
}

static inline int listHeight(int row)
{
    return qMax(0, (UserFrameHeight + UserWidgetSpacing) * row - UserWidgetSpacing);
}

UserFrameList::UserFrameList(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(new QScrollArea(this))
    , m_frameDataBind(FrameDataBind::Instance())
{
    setObjectName(QStringLiteral("UserFrameList"));
    setAccessibleName(QStringLiteral("UserFrameList"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);

    initUI();

    // 设置用户列表支持触屏滑动,TouchGesture存在bug,滑动过程中会响应其他事件,打断滑动事件,改为LeftMouseButtonGesture
    QScroller::grabGesture(m_scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(m_scrollArea->viewport());
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootWhenScrollable);
    scroller->setScrollerProperties(sp);

    std::function<void(QVariant)> function = std::bind(&UserFrameList::onOtherPageChanged, this, std::placeholders::_1);
    int index = m_frameDataBind->registerFunction("UserFrameList", function);

    connect(this, &UserFrameList::destroyed, this, [this, index] {
        m_frameDataBind->unRegisterFunction("UserFrameList", index);
    });
    connect(this, &UserFrameList::gridBoundChanged, this, &UserFrameList::updateLayout);
}

void UserFrameList::initUI()
{
    m_centerWidget = new QWidget;
    m_centerWidget->setAccessibleName("UserFrameListCenterWidget");

    m_flowLayout = new DFlowLayout(m_centerWidget);
    m_flowLayout->setFlow(QListView::LeftToRight);
    m_flowLayout->setContentsMargins(ContentsMargin, ContentsMargin, ContentsMargin, ContentsMargin);
    m_flowLayout->setSpacing(UserWidgetSpacing);

    m_scrollArea->setAccessibleName("UserFrameListCenterWidget");
    m_scrollArea->setContentsMargins(0, 0, 0, 0);
    m_scrollArea->setWidget(m_centerWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    m_centerWidget->setAutoFillBackground(false);
    m_scrollArea->viewport()->setAutoFillBackground(false);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea, 0, Qt::AlignCenter);
}

//设置SessionBaseModel，创建用户列表窗体
void UserFrameList::setModel(SessionBaseModel *model)
{
    m_model = model;

    connect(model, &SessionBaseModel::userAdded, this, &UserFrameList::handlerBeforeAddUser);
    connect(model, &SessionBaseModel::userRemoved, this, &UserFrameList::removeUser);

    for (auto user : m_model->userList()) {
        handlerBeforeAddUser(user);
    }
    onCurrentUserChanged(m_model->currentUser());
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &UserFrameList::onCurrentUserChanged);
}

void UserFrameList::setFixedSize(int w, int h)
{
    QWidget::setFixedSize(w, h);
    onMaximumSizeChanged(w, h);
}

void UserFrameList::setMaximumSize(int maxw, int maxh)
{
    QWidget::setMaximumSize(maxw, maxh);
    onMaximumSizeChanged(maxw, maxh);
}

void UserFrameList::setMaximumSize(const QSize &maximumSize)
{
    setMaximumSize(maximumSize.width(), maximumSize.height());
}

void UserFrameList::handlerBeforeAddUser(std::shared_ptr<User> user)
{
    if (m_model->isServerModel()) {
        if (user->isLogin() || user->type() == User::Default)
            addUser(user);
        connect(user.get(), &User::loginStateChanged, this, [ = ](bool is_login) {
            if (is_login) {
                addUser(user);
            } else {
                removeUser(user);
            }
        });
    } else {
        addUser(user);
    }
}

//创建用户窗体
void UserFrameList::addUser(const std::shared_ptr<User> user)
{
    UserWidget *widget = new UserWidget(m_centerWidget);
    widget->setUser(user);
    widget->setSelected(m_model->currentUser()->uid() == user->uid());

    connect(widget, &UserWidget::clicked, this, &UserFrameList::onUserClicked);

    //多用户的情况按照其uid排序，升序排列，符合账户先后创建顺序
    m_loginWidgets.push_back(widget);
    std::sort(m_loginWidgets.begin(), m_loginWidgets.end(), [ = ](UserWidget *w1, UserWidget *w2) {
        return (w1->uid() < w2->uid());
    });
    int index = m_loginWidgets.indexOf(widget);
    m_flowLayout->insertWidget(index, widget);

    //添加用户和删除用户时，重新计算区域大小
    updateLayout();
}

//删除用户
void UserFrameList::removeUser(const std::shared_ptr<User> user)
{
    qDebug() << "UserFrameList::removeUser:" << user->path();
    foreach (auto w, m_loginWidgets) {
        if (w->uid() == user->uid()) {
            m_loginWidgets.removeOne(w);

            if (w == currentSelectedUser) {
                currentSelectedUser = nullptr;
            }
            w->deleteLater();
            break;
        }
    }

    //添加用户和删除用户时，重新计算区域大小
    updateLayout();
}

//点击用户
void UserFrameList::onUserClicked()
{
    UserWidget *widget = static_cast<UserWidget *>(sender());
    if (!widget) return;

    currentSelectedUser = widget;
    for (int i = 0; i != m_loginWidgets.size(); ++i) {
        if (m_loginWidgets[i]->isSelected()) {
            m_loginWidgets[i]->setFastSelected(false);
        }
    }
    emit clicked();
    emit requestSwitchUser(m_model->findUserByUid(widget->uid()));
}

/**
 * @brief 切换下一个用户
 */
void UserFrameList::switchNextUser()
{
    for (int i = 0; i != m_loginWidgets.size(); ++i) {
        if (m_loginWidgets[i]->isSelected()) {
            m_loginWidgets[i]->setSelected(false);
            if (i == (m_loginWidgets.size() - 1)) {
                currentSelectedUser = m_loginWidgets.first();
                currentSelectedUser->setSelected(true);
                //处理m_scrollArea翻页显示
                m_scrollArea->verticalScrollBar()->setValue(0);
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            } else {
                //处理m_scrollArea翻页显示
                int selectedRight = m_loginWidgets[i]->geometry().right();
                int scrollRight = m_scrollArea->widget()->geometry().right();
                if (selectedRight + UserWidgetSpacing == scrollRight) {
                    QPoint topLeft;
                    if (m_rowCount == 1) {
                        topLeft = m_loginWidgets[i + 1]->geometry().topLeft();
                    } else {
                        topLeft = m_loginWidgets[i]->geometry().topLeft();
                    }
                    m_scrollArea->verticalScrollBar()->setValue(topLeft.y());
                }

                currentSelectedUser = m_loginWidgets[i + 1];
                currentSelectedUser->setSelected(true);
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            }
            break;
        }
    }
}

/**
 * @brief 切换上一个用户
 */
void UserFrameList::switchPreviousUser()
{
    for (int i = 0; i != m_loginWidgets.size(); ++i) {
        if (m_loginWidgets[i]->isSelected()) {
            m_loginWidgets[i]->setSelected(false);
            if (i == 0) {
                currentSelectedUser = m_loginWidgets.last();
                currentSelectedUser->setSelected(true);
                //处理m_scrollArea翻页显示
                m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            } else {
                //处理m_scrollArea翻页显示
                QPoint topLeft = m_loginWidgets[i]->geometry().topLeft();
                if (topLeft.x() == 0) {
                    m_scrollArea->verticalScrollBar()->setValue(m_loginWidgets[i - 1]->geometry().topLeft().y());
                }

                currentSelectedUser = m_loginWidgets[i - 1];
                currentSelectedUser->setSelected(true);
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            }
            break;
        }
    }
}

void UserFrameList::setGridBound(QPair<int, int> bound)
{
    if (m_gridBound != bound) {
        m_gridBound = bound;
        Q_EMIT this->gridBoundChanged(bound);
    }
}

void UserFrameList::onOtherPageChanged(const QVariant &value)
{
    foreach (auto w, m_loginWidgets) {
        w->setSelected(w->uid() == value.toUInt());
    }
}

void UserFrameList::onMaximumSizeChanged(int maxw, int maxh)
{
    static constexpr qreal DisplayRatio = 0.8;
    static constexpr QMargins margin{ContentsMargin, ContentsMargin, ContentsMargin, ContentsMargin};
    QSize bound = (QSize(maxw, maxh) * DisplayRatio).shrunkBy(margin);
    int candidateCol = qCeil(static_cast<qreal>(bound.width()) / (UserFrameWidth + UserWidgetSpacing));
    int candidateRow = qCeil(static_cast<qreal>(bound.height()) / (UserFrameHeight + UserWidgetSpacing));
    if (listWidth(candidateCol) > bound.width()) {
        --candidateCol;
    }
    if (listHeight(candidateRow) > bound.height()) {
        --candidateRow;
    }
    setGridBound({candidateCol, candidateRow});
}

void UserFrameList::onCurrentUserChanged(std::shared_ptr<User> currentUser)
{
    if (!currentUser) return;
    for (const auto login_widget : m_loginWidgets) {
        if (login_widget->uid() == currentUser->uid()) {
            currentSelectedUser = login_widget;
            currentSelectedUser->setSelected(true);
        } else {
            login_widget->setSelected(false);
        }
    }
}

void UserFrameList::updateLayout()
{
    // 用户控件分行显示，超过最大显示行数时显示滚动条
    const int MaxDisplayCol = colBound();
    const int MaxDisplayRow = rowBound();
    int count = m_flowLayout->count();
    int row = qCeil(static_cast<qreal>(count) / MaxDisplayCol);
    int areaWidth = listWidth(qMin(count, MaxDisplayCol)) + ContentsMargin  * 2;
    int areaHeight = listHeight(qMin(row, MaxDisplayRow)) + ContentsMargin  * 2;
    m_scrollArea->setFixedSize(areaWidth, areaHeight);
    m_centerWidget->setFixedSize(areaWidth, listHeight(row) + ContentsMargin  * 2);
}

void UserFrameList::hideEvent(QHideEvent *event)
{
    return QWidget::hideEvent(event);
}

void UserFrameList::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        switchPreviousUser();
        break;
    case Qt::Key_Right:
        switchNextUser();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        for (auto it = m_loginWidgets.constBegin(); it != m_loginWidgets.constEnd(); ++it) {
            if ((*it)->isSelected()) {
                emit(*it)->clicked();
                break;
            }
        }
        break;
    case Qt::Key_Escape:
        emit clicked();
        emit requestSwitchUser(m_model->currentUser());
        break;
    default:
        break;
    }
    return QWidget::keyPressEvent(event);
}

void UserFrameList::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    if (currentSelectedUser != nullptr) {
        currentSelectedUser->setSelected(true);
    }
}

void UserFrameList::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    if (currentSelectedUser != nullptr)
        currentSelectedUser->setSelected(false);
}

void UserFrameList::resizeEvent(QResizeEvent *event)
{
    updateLayout();

    QWidget::resizeEvent(event);
}
