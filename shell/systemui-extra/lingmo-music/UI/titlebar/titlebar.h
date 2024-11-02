/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSqlTableModel>
#include <QListWidget>
#include <QSqlQuery>
#include <QDebug>
#include <QListWidgetItem>
#include <QEvent>

//#include "changelistwid.h"
//#include "allpupwindow.h"
//#include "widgetstyle.h"
#include "menumodule.h"
#include "UI/player/searchedit.h"

const QString main_style = "QFrame{background:#FFFFFF;border-top-right-radius:12px;}";

#pragma pack(push)
#pragma pack(1)

class TitleBar : public QFrame
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar(){}

    menuModule *menumodule = nullptr;

    QLabel *nullLabel = nullptr;

    QPushButton *leftBtn = nullptr;
    QPushButton *rightBtn = nullptr;

    SearchEdit *searchEdit = nullptr;

    QPushButton *userIconBtn = nullptr;
    QPushButton *loginBtn = nullptr;

    QPushButton *setBtn = nullptr;
    QPushButton *miniBtn = nullptr;
    QPushButton *minimumBtn = nullptr;
    QPushButton *maximumBtn = nullptr;
    QPushButton *recoverNormalSizeBtn = nullptr;
    QPushButton *closeBtn = nullptr;

    //搜索相关
    QSqlTableModel *searchModel = nullptr;
    QListWidget *searchWidget = nullptr;
//    MusicListWid *searchResultWidget; /*搜索部分暂时注释掉*/


    //设置菜单
    QMenu *settingMenu = nullptr;
    QMenu *changeThemeColorMenu = nullptr;
    QAction *darkThemeAct = nullptr;
    QAction *lightThemeAct = nullptr;
    QAction *aboutAct = nullptr;

//    AllPupWindow *aboutWidget;

    void titlecolor();

public Q_SLOTS:
    void searchMusic();
    void slotFoucusIn();

//    void showAboutWidget();

protected:
//    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
private Q_SLOTS:
    //进行最小化、最大化、还原、关闭
//    void slot_showMaximized();
//    void slot_quit();

private:
    void initTitle();

    QHBoxLayout *titleLayout = nullptr;
    QHBoxLayout *searchLayout = nullptr;
    QHBoxLayout *LayoutLeft = nullptr;
    QHBoxLayout *LayoutRight = nullptr;

    QWidget *m_parent = nullptr;
};



#pragma pack(pop)


#endif // TITLEBAR_H
