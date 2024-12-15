// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOCKITEMCONTEXTMENU_H
#define DOCKITEMCONTEXTMENU_H

#include <QAction>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QPointer>

#include <DSingleton>

const static QByteArray MENU_IS_VISIBLE_OR_JUST_HIDE = "menuIsShownOrJustClicked";

class DockContextMenu : public QMenu
{
public:
    using QMenu::QMenu;

    DockContextMenu();

    void clearShowReminderActions() { m_showReminderActions.clear(); }

    void addShowReminderAction(QAction* action) { m_showReminderActions.append(action); }

    int suitableWidth() const;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    QList<QPointer<QAction>> m_showReminderActions;
};

class DockContextMenuHelper : public QObject, public Dtk::Core::DSingleton<DockContextMenuHelper>
{
    friend class Dtk::Core::DSingleton<DockContextMenuHelper>;

    Q_OBJECT
public:
    void showContextMenu(QWidget* who, const QString& menuJson, const QPoint& pos = QCursor::pos());
    bool menuIsVisible() const;
    const QWidget* currentWidget() const { return m_currentWidget; }

    static QPoint correctMenuPos(QPoint pos, int menuHeight);

signals:
    void menuActionClicked(QAction *action, QObject *obj);

private:
    DockContextMenuHelper();
    ~DockContextMenuHelper() {}

private:
    DockContextMenu m_contextMenu;
    QPointer<QWidget> m_currentWidget;
};

#endif
