// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_tabbar.h"
#include "QStyleOptionTab"
#include "../src/widgets/window.h"
#include "../src/editor/editwrapper.h"
#include <QMouseEvent>
#include "src/stub.h"


namespace tabbarstub {

int retintstub()
{
    return 1;
}

}

using namespace tabbarstub;

UT_Tabbar::UT_Tabbar()
{

}

TEST(UT_Tabbar_Tabbar, UT_Tabbar_Tabbar)
{
    Tabbar* tab = new Tabbar;
    tab->m_moreWaysCloseMenu = new QMenu(tab);
    tab->m_rightMenu = new QMenu(tab);

    EXPECT_NE(tab,nullptr);
    EXPECT_NE(tab->m_moreWaysCloseMenu,nullptr);
    EXPECT_NE(tab->m_rightMenu,nullptr);

    tab->deleteLater();


}

TEST(UT_Tabbar_openFilesInWindow, UT_Tabbar_openFilesInWindow)
{
//    Tabbar * tab = new Tabbar();
//    tab->addTab(".cache/deepin/deepin-editor","aabb");

//    EXPECT_NE(tab,nullptr);
//    EXPECT_EQ(tab->currentIndex(),0);

//    tab->deleteLater();
}

//addTabWithIndex
TEST(UT_Tabbar_addTabWithIndex, UT_Tabbar_addTabWithIndex)
{
//    Tabbar * tab = new Tabbar();
//    tab->addTabWithIndex(0,".cache/deepin/deepin-editor","aabb");

//    EXPECT_NE(tab,nullptr);
//    EXPECT_NE(tab->tabToolTip(0),"aabb");

//    tab->deleteLater();
}

//closeTab
TEST(UT_Tabbar_closeTab, UT_Tabbar_closeTab)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->closeTab(index);
    window->getTabbar()->closeTab(-1);


    EXPECT_EQ(window->getTabbar()->count(),0);
    EXPECT_NE(window->getTabbar(),nullptr);
    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();

}
//void closeCurrentTab();
TEST(UT_Tabbar_closeCurrentTab, UT_Tabbar_closeCurrentTab)
{
    Tabbar * tab = new Tabbar();
    tab->closeCurrentTab();

    EXPECT_EQ(tab->count(),0);
    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}
//void closeOtherTabs();
TEST(UT_Tabbar_closeOtherTabs, UT_Tabbar_closeOtherTabs)
{
    Tabbar * tab = new Tabbar();
    tab->closeOtherTabs();

    EXPECT_EQ(tab->count(),0);
    EXPECT_NE(tab,nullptr);


    tab->deleteLater();

}

//void closeLeftTabs(const QString &filePath);
TEST(UT_Tabbar_closeLeftTabs, UT_Tabbar_closeLeftTabs)
{
    Tabbar * tab = new Tabbar();
    tab->m_tabPaths.push_back("aa");
    tab->closeLeftTabs("aa");
    tab->closeLeftTabs("bb");


    EXPECT_EQ(tab->count(),0);
    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}
//void closeRightTabs(const QString &filePath);
TEST(UT_Tabbar_closeRightTabs, UT_Tabbar_closeRightTabs)
{
    Tabbar * tab = new Tabbar();
    tab->m_tabPaths.push_back("aa");
    tab->closeRightTabs("aa");
    tab->closeLeftTabs("bb");

    EXPECT_EQ(tab->count(),0);
    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}

//void closeOtherTabsExceptFile(const QString &filePath);
TEST(UT_Tabbar_closeOtherTabsExceptFile, UT_Tabbar_closeOtherTabsExceptFile)
{
    Tabbar * tab = new Tabbar();
    tab->m_tabPaths.push_back("bb");
    tab->closeOtherTabsExceptFile("aa");

    EXPECT_EQ(tab->count(),0);
    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}

//void updateTab(int index, const QString &filePath, const QString &tabName);
TEST(UT_Tabbar_updateTab, UT_Tabbar_updateTab)
{
//    Tabbar * tab = new Tabbar();
//    tab->addTab("/.cache/deepin/deepin-editor","aa");
//    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

//    tab->updateTab(0,"/.cache/deepin/deepin-editor","aa");

//    EXPECT_NE(tab,nullptr);
//    tab->deleteLater();
}

//void previousTab();
TEST(UT_Tabbar_previousTab, UT_Tabbar_previousTab)
{
//    Tabbar * tab = new Tabbar();
//    tab->addTab("/.cache/deepin/deepin-editor","aa");
//    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);
//    tab->previousTab();

//    tab->addTab("/.cache/deepin/deepin-editor","bb");
//    tab->previousTab();

//    EXPECT_NE(tab,nullptr);

//    tab->deleteLater();

}
//void nextTab();
TEST(UT_Tabbar_nextTab, UT_Tabbar_nextTab)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    tab->nextTab();

    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}

//int indexOf(const QString &filePath);
TEST(UT_Tabbar_indexOf, UT_Tabbar_indexOf)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);
    EXPECT_NE(tab->indexOf("/.cache/deepin/deepin-editor"),2);

    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}

//QString currentName() const;
TEST(UT_Tabbar_currentName, UT_Tabbar_currentName)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EXPECT_NE(tab->currentName(),"");

    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}
//QString currentPath() const;
TEST(UT_Tabbar_currentPath, UT_Tabbar_currentPath)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EXPECT_NE(tab->currentPath(),"");

    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}
//QString fileAt(int index) const;
TEST(UT_Tabbar_fileAt, UT_Tabbar_fileAt)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EXPECT_NE(tab->fileAt(0),"");
    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}
//QString textAt(int index) const;
TEST(UT_Tabbar_textAt, UT_Tabbar_textAt)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);
    EXPECT_NE(tab->textAt(0),"");

    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}

//void setTabPalette(const QString &activeColor, const QString &inactiveColor);
TEST(UT_Tabbar_setTabPalette, UT_Tabbar_setTabPalette)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);
    tab->setTabPalette("red","red");

    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}
//void setBackground(const QString &startColor, const QString &endColor);
TEST(UT_Tabbar_setBackground, UT_Tabbar_setBackground)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    tab->setBackground("red","red");

    EXPECT_EQ(tab->m_backgroundStartColor,"red");

    tab->deleteLater();

}
//void setDNDColor(const QString &startColor, const QString &endColor);
TEST(UT_Tabbar_setDNDColor, UT_Tabbar_setDNDColor)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    tab->setDNDColor("red","red");

    EXPECT_EQ(tab->m_dndStartColor,"red");
    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}
//bool canInsertFromMimeData(int index, const QMimeData *source) const;
//bool eventFilter(QObject *, QEvent *event);
TEST(UT_Tabbar_eventFilter, UT_Tabbar_eventFilter)
{
//    Window* window = new Window;
//    EditWrapper* wrapper = new EditWrapper(window);
//    TextEdit* edit = new TextEdit(wrapper);
//    edit->m_wrapper = wrapper;
//    window->m_wrappers["12"]=wrapper;
//    Tabbar * tab = new Tabbar(window);

//    tab->addTab("/.cache/deepin/deepin-editor","aa");
//    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

//    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonPress,QPointF(76,29),Qt::RightButton,Qt::RightButton,Qt::NoModifier);

//    Stub s1;
//    s1.set((QAction* (QMenu::*)(const QPoint& , QAction *))ADDR(QMenu,exec),retintstub);

//    tab->eventFilter(tab,e);

//    EXPECT_NE(tab,nullptr);

//    tab->deleteLater();
//    window->deleteLater();
//    wrapper->deleteLater();
//    edit->deleteLater();
}

//QSize minimumTabSizeHint(int index) const;
TEST(UT_Tabbar_minimumTabSizeHint, UT_Tabbar_minimumTabSizeHint)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");
    EXPECT_EQ(tab->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EXPECT_EQ(tab->minimumTabSizeHint(0),QSize(110, 40));


    EXPECT_NE(tab,nullptr);

    tab->deleteLater();


}
//QSize maximumTabSizeHint(int index) const;
TEST(UT_Tabbar_maximumTabSizeHint, UT_Tabbar_maximumTabSizeHint)
{
    Tabbar * tab = new Tabbar();
    tab->addTab("/.cache/deepin/deepin-editor","aa");

    EXPECT_EQ(tab->maximumTabSizeHint(0),QSize(160, 40));

    EXPECT_NE(tab,nullptr);

    tab->deleteLater();

}

TEST(UT_Tabbar_createDragPixmapFromTab, UT_Tabbar_createDragPixmapFromTab)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->createDragPixmapFromTab(index,option,&p);


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}


TEST(UT_Tabbar_createMimeDataFromTab, UT_Tabbar_createMimeDataFromTab)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->createMimeDataFromTab(index,option);



    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);


    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_insertFromMimeDataOnDragEnter, UT_Tabbar_insertFromMimeDataOnDragEnter)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("dedit/tabbar","test");

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->insertFromMimeDataOnDragEnter(index,mimeData);
    EXPECT_EQ(wrapper->getFileLoading(),false);

    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);
    EXPECT_NE(mimeData,nullptr);

    delete mimeData;mimeData = nullptr;
    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_insertFromMimeData, UT_Tabbar_insertFromMimeData)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("dedit/tabbar","test");

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->insertFromMimeData(index,mimeData);
    EXPECT_NE(window->getTabbar()->count(),0);



    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);
    EXPECT_NE(mimeData,nullptr);

    delete mimeData;mimeData = nullptr;
    window->deleteLater();
    wrapper->deleteLater();


}


TEST(UT_Tabbar_canInsertFromMimeData, UT_Tabbar_canInsertFromMimeData)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("dedit/tabbar","test");

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    EXPECT_EQ(window->getTabbar()->canInsertFromMimeData(index,mimeData),true);



    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);
    EXPECT_NE(mimeData,nullptr);


    delete mimeData;mimeData = nullptr;
    window->deleteLater();
    wrapper->deleteLater();


}


TEST(UT_Tabbar_handleDragActionChanged, UT_Tabbar_handleDragActionChanged)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    Qt::DropAction actions[2] = {Qt::IgnoreAction,Qt::MoveAction};

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->handleDragActionChanged(actions[0]);
    window->getTabbar()->handleDragActionChanged(actions[1]);


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_mousePressEvent, UT_Tabbar_mousePressEvent)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    QMouseEvent* event = new QMouseEvent(QEvent::None,QPoint(),Qt::MidButton,Qt::MidButton,Qt::NoModifier);


    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->mousePressEvent(event);


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    delete event;event = nullptr;
    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_dropEvent, UT_Tabbar_dropEvent)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("dedit/tabbar","test");
    QDropEvent* event = new QDropEvent(QPointF(100,100),Qt::CopyAction,mimeData,Qt::LeftButton,Qt::NoModifier);


    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->dropEvent(event);


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);
    EXPECT_NE(event,nullptr);
    EXPECT_NE(mimeData,nullptr);

    delete mimeData;mimeData = nullptr;
    delete event;event = nullptr;
    window->deleteLater();
    wrapper->deleteLater();



}

TEST(UT_Tabbar_tabSizeHint, UT_Tabbar_tabSizeHint)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    EXPECT_NE(window->getTabbar()->tabSizeHint(index),QSize(1,1));




    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_handleTabMoved, UT_Tabbar_handleTabMoved)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->handleTabMoved(index,index);

    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_handleTabReleased, UT_Tabbar_handleTabReleased)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->m_listOldTabPath.push_back("/.cache/deepin/deepin-editor");
    EXPECT_EQ(window->getTabbar()->m_listOldTabPath.contains("/.cache/deepin/deepin-editor"),true);
    window->getTabbar()->handleTabReleased(index);


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_handleTabIsRemoved, UT_Tabbar_handleTabIsRemoved)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;

    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->handleTabIsRemoved(index);


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_handleTabDroped, UT_Tabbar_handleTabDroped)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    Qt::DropAction actions[2] = {Qt::IgnoreAction,Qt::MoveAction};


    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->handleTabDroped(index,actions[0],nullptr);
    window->getTabbar()->handleTabDroped(index,actions[0],window->getTabbar());


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_onTabDrapStart, UT_Tabbar_onTabDrapStart)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    Qt::DropAction actions[2] = {Qt::IgnoreAction,Qt::MoveAction};


    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);


    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->onTabDrapStart();
    EXPECT_EQ(window->isVisible(),true);

    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);

    window->deleteLater();
    wrapper->deleteLater();


}

TEST(UT_Tabbar_resizeEvent, UT_Tabbar_resizeEvent)
{
    int index = 0;
    QStyleOptionTab option;
    QPoint p;
    Qt::DropAction actions[2] = {Qt::IgnoreAction,Qt::MoveAction};
    QResizeEvent* e = new QResizeEvent(QSize(),QSize());


    Window * window = new Window;
    window->addTab("/.cache/deepin/deepin-editor",true);
    EXPECT_EQ(window->m_tabbar->m_tabPaths.contains("/.cache/deepin/deepin-editor"),true);

    EditWrapper* wrapper = new EditWrapper(window);
    window->getTabbar()->resizeEvent(e);


    EXPECT_NE(window,nullptr);
    EXPECT_NE(wrapper,nullptr);
    EXPECT_NE(e,nullptr);



    delete e;  e = nullptr;
    window->deleteLater();
    wrapper->deleteLater();


}


