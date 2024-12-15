// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERFRAMELIST_H
#define USERFRAMELIST_H

#include <dflowlayout.h>

#include <QMap>
#include <QPointer>

#include <memory>

class UserWidget;
class User;
class SessionBaseModel;
class FrameDataBind;
class QVBoxLayout;
class QScrollArea;

DWIDGET_USE_NAMESPACE

class UserFrameList : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPair<int, int> gridBound READ gridBound WRITE setGridBound NOTIFY gridBoundChanged)
    Q_PROPERTY(int rowBound READ rowBound)
    Q_PROPERTY(int colBound READ colBound)

public:
    explicit UserFrameList(QWidget *parent = nullptr);
    void setModel(SessionBaseModel *model);

    void setFixedSize(int w, int h);
    void setMaximumSize(int maxw, int maxh);
    void setMaximumSize(const QSize &maximumSize);

    void updateLayout();

    inline QPair<int, int> gridBound() const { return m_gridBound; }
    inline int colBound() const { return m_gridBound.first; }
    inline int rowBound() const { return m_gridBound.second; }


Q_SIGNALS:
    void requestSwitchUser(std::shared_ptr<User> user);
    void clicked();
    void gridBoundChanged(QPair<int, int> bound);

protected:
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
    void onUserClicked();
    void onOtherPageChanged(const QVariant &value);
    void onMaximumSizeChanged(int maxw, int maxh);
    void onCurrentUserChanged(std::shared_ptr<User> currentUser);

private:
    void initUI();
    void handlerBeforeAddUser(std::shared_ptr<User> user);
    void addUser(const std::shared_ptr<User> user);
    void removeUser(const std::shared_ptr<User> user);
    void switchNextUser();
    void switchPreviousUser();
    void setGridBound(QPair<int, int> bound);

private:
    QScrollArea *m_scrollArea;
    DFlowLayout *m_flowLayout;
    QList<UserWidget *> m_loginWidgets;
    QMap<uid_t, UserWidget *> m_userWidgets;
    QPointer<UserWidget> currentSelectedUser;
    SessionBaseModel *m_model;
    FrameDataBind *m_frameDataBind;
    QWidget *m_centerWidget;
    int m_colCount;
    int m_rowCount;
    QPair<int, int> m_gridBound;
};

#endif // USERFRAMELIST_H
