// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_editorapplication.h"
#include "../../src/editorapplication.h"
#include <QPushButton>
#include <QKeyEvent>
#include "src/stub.h"



namespace editappstub
{
QWidget* activeWindowStub()
{
    return new QWidget;
}
}

using namespace editappstub;


UT_EditorApplication::UT_EditorApplication()
{

}


TEST(UT_EditorApplication_EditorApplication, EditorApplication_001)
{
    int argc = 1;
    char* argv[] = {"test"};
    //no deleted...
    EditorApplication *app = new EditorApplication(argc, argv);
    QString strRetOrNmae = app->organizationName();
    QString strRetAppName = app->applicationName();
    ASSERT_TRUE(!strRetOrNmae.compare(QString("deepin")) && !strRetAppName.compare(QString("deepin-editor")));

    app->deleteLater();
}

TEST(UT_EditorApplication_EditorApplication, EditorApplication_002)
{
    int argc = 1;
    char* argv[] = {"test"};
    //EditorApplication e(argc,argv);
}

TEST(UT_EditorApplication_pressSpace, notify_001)
{
    int argc = 1;
    char* argv[] = {"test"};
    //no deleted...
    EditorApplication *app = new EditorApplication(argc,argv);
    Qt::KeyboardModifier modefiers[4] = {Qt::ControlModifier,Qt::AltModifier,Qt::MetaModifier,Qt::NoModifier};
    QKeyEvent *e = new QKeyEvent(QEvent::KeyPress,Qt::Key_Return ,modefiers[0],"\r");

    QPushButton* btn = new QPushButton;
    btn->setObjectName("CustomRebackButton");
    bool bRet = app->notify(btn, e);
    ASSERT_TRUE(bRet == true);

    btn->deleteLater();
    delete e;
    e = nullptr;
    app->deleteLater();
}

TEST(UT_EditorApplication_pressSpace, notify_002)
{
    int argc = 1;
    char* argv[] = {"test"};
    //no deleted...
    EditorApplication *app = new EditorApplication(argc,argv);
    Qt::KeyboardModifier modefiers[4] = {Qt::ControlModifier,Qt::AltModifier,Qt::MetaModifier,Qt::NoModifier};
    QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, modefiers[0],"\r");

    QPushButton* btn = new QPushButton;
    btn->setObjectName("CustomRebackButton");
    bool bRet = app->notify(btn, e);
    ASSERT_TRUE(bRet == true);

    btn->deleteLater();
    delete e;
    e = nullptr;
    app->deleteLater();
}

TEST(UT_EditorApplication_pressSpace, notify_003)
{
    int argc = 1;
    char* argv[] = {"test"};
    //no deleted...
    EditorApplication *app = new EditorApplication(argc,argv);
    Qt::KeyboardModifier modefiers[4] = {Qt::ControlModifier,Qt::AltModifier,Qt::MetaModifier,Qt::NoModifier};
    QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Print, modefiers[0],"\r");

    QPushButton* btn = new QPushButton;
    btn->setObjectName("CustomRebackButton1");
    bool bRet = app->notify(btn, e);
    ASSERT_TRUE(bRet == true);

    btn->deleteLater();
    delete e;
    e = nullptr;
    app->deleteLater();
}

TEST(UT_EditorApplication_pressSpace, notify_004)
{
    int argc = 1;
    char* argv[] = {"test"};
    //no deleted...
    EditorApplication *app = new EditorApplication(argc,argv);
    Qt::KeyboardModifier modefiers[4] = {Qt::ControlModifier,Qt::AltModifier,Qt::MetaModifier,Qt::NoModifier};
    QKeyEvent *e = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Print, modefiers[0], "\r");

    QPushButton* btn = new QPushButton;
    btn->setObjectName("CustomRebackButton1");
    bool bRet = app->notify(btn, e);
    ASSERT_TRUE(bRet == true);

    btn->deleteLater();
    delete e;
    e = nullptr;
    app->deleteLater();
}

TEST(UT_EditorApplication_pressSpace, pressSpace)
{
    int argc = 1;
    char* argv[] = {"test"};
    //no deleted...
    EditorApplication *app = new EditorApplication(argc,argv);

    QPushButton* btn = new QPushButton;
    btn->setObjectName("CustomRebackButton");
    app->pressSpace(btn);
    QString strRetOrNmae = app->organizationName();
    QString strRetAppName = app->applicationName();
    ASSERT_TRUE(!strRetOrNmae.compare(QString("deepin")) && !strRetAppName.compare(QString("deepin-editor")));

    btn->deleteLater();
    app->deleteLater();
}

TEST(UT_EditorApplication_handleQuitAction, handleQuitAction)
{
    int argc = 1;
    char* argv[] = {"test"};
    //no deleted...
    EditorApplication *app = new EditorApplication(argc,argv);
    Stub s1;
    s1.set(ADDR(QApplication,activeWindow),activeWindowStub);

    app->handleQuitAction();

    app->deleteLater();

}
