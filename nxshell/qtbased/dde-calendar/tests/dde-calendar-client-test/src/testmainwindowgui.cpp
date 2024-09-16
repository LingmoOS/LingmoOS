// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "testmainwindowgui.h"
//#include "configsettings.h"
//#include "view/monthgraphiview.h"
//#include "yearWidget/yearwindow.h"
//#include "monthWidget/monthwindow.h"
//#include "dialog_stub.h"
//#include "testscheduledata.h"

//#include <QProcess>

//testMainWindowGUI::testMainWindowGUI()
//{
//}

//QVariant getSetView(const QString &str)
//{
//    QVariant variant;
//    if (str == "base.geometry") {
//        QByteArray byteArray("\x01\xD9\xD0\xCB\x00\x02\x00\x00\x00\x00\x02\x12\x00\x00\x00\xF3\x00\x00\x05m\x00\x00\x03l\x00\x00\x02\x12\x00\x00\x00\xF3\x00\x00\x05m\x00\x00\x03l\x00\x00\x00\x00\x00\x00\x00\x00\x07\x80");
//        variant.setValue(byteArray);
//    } else if (str == "base.state") {
//        variant.setValue(1);

//    } else if (str == "base.view") {
//        variant.setValue(2);
//    }
//    return variant;
//}

//void testMainWindowGUI::SetUp()
//{
//    Stub stub;
//    stub.set(ADDR(CConfigSettings, value), getSetView);
//}

//void testMainWindowGUI::TearDown()
//{
//}

//TEST_F(testMainWindowGUI, yearGUITest)
//{
//    Calendarmainwindow mainWindow(0);

//    CYearWindow *myYearWindow = mainWindow.findChild<CYearWindow *>("yearwindow");
//    if (myYearWindow != nullptr) {
//        DIconButton *m_prevButton = myYearWindow->findChild<DIconButton *>("PrevButton");
//        DIconButton *m_nextButton = myYearWindow->findChild<DIconButton *>("NextButton");
//        LabelWidget *m_today = myYearWindow->findChild<LabelWidget *>("yearToDay");

//        if (m_prevButton != nullptr) {
//            QTest::mouseClick(m_prevButton, Qt::LeftButton);
//        }

//        if (m_today != nullptr) {
//            QTest::mouseClick(m_today, Qt::LeftButton);
//        }

//        if (m_nextButton != nullptr) {
//            QTest::mouseClick(m_nextButton, Qt::LeftButton);
//        }
//        //获取QStackedWidget
//        QStackedWidget *stackedWidget = myYearWindow->findChild<QStackedWidget *>();
//        if (stackedWidget) {
//            QWidget *currentWidget = stackedWidget->currentWidget();
//            if (currentWidget) {
//                currentWidget->setFocus();
//                QTest::keyPress(currentWidget, Qt::Key_Up);
//            }
//            currentWidget = stackedWidget->currentWidget();
//            if (currentWidget) {
//                currentWidget->setFocus();
//                QTest::keyPress(currentWidget, Qt::Key_Down);
//            }
//        }
//        //year_resizeEvent
//        myYearWindow->setSearchWFlag(true);
//        mainWindow.resize(1000, 400);
//        myYearWindow->setSearchWFlag(false);
//        mainWindow.resize(700, 400);
//    }
//}

//TEST_F(testMainWindowGUI, monthGUITest)
//{
//    Calendarmainwindow mainWindow(1);
//    CMonthWindow *mymonthWindow = mainWindow.findChild<CMonthWindow *>("monthWindow");
//    if (mymonthWindow != nullptr) {
//    }
//}
//TEST_F(testMainWindowGUI, weekGUIKeyTest)
//{
//    Calendarmainwindow mainWindow(2);
//}

//TEST_F(testMainWindowGUI, dayGUIKeyTest)
//{
//    Calendarmainwindow mainWindow(3);
//}

//TEST_F(testMainWindowGUI, slotCurrentDateUpdate)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotCurrentDateUpdate();
//}

////slotSetSearchFocus
//TEST_F(testMainWindowGUI, slotSetSearchFocus)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotSetSearchFocus();
//}

////viewWindow
//TEST_F(testMainWindowGUI, viewWindow)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.viewWindow(3, false);
//    mainWindow.viewWindow(0, true);
//}

////setSearchWidth
//TEST_F(testMainWindowGUI, setSearchWidth)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.setSearchWidth(500);
//}

////slotTheme
//TEST_F(testMainWindowGUI, slotTheme)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotTheme(0);
//    mainWindow.slotTheme(1);
//    mainWindow.slotTheme(2);
//}

////slotOpenSchedule
//TEST_F(testMainWindowGUI, slotOpenSchedule)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    Calendarmainwindow mainWindow(1);
//    QString job = ScheduleDataInfo::ScheduleToJsonStr(TestDataInfo::getScheduleItemDInfo().first());
//    mainWindow.slotOpenSchedule(job);
//}

////slotWUpdateSchedule
//TEST_F(testMainWindowGUI, slotWUpdateSchedule)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotWUpdateSchedule();
//}

////slotStextChanged
//TEST_F(testMainWindowGUI, slotStextChanged)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotStextChanged();
//}

////slotStextfocusChanged
//TEST_F(testMainWindowGUI, slotStextfocusChanged)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotStextfocusChanged(true);
//}

////slotDeleteitem
//TEST_F(testMainWindowGUI, slotDeleteitem)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotDeleteitem();
//}

////slotNewSchedule
//TEST_F(testMainWindowGUI, slotNewSchedule)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotNewSchedule();
//}

////slotSwitchView
//TEST_F(testMainWindowGUI, slotSwitchView)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotSwitchView(0);
//    mainWindow.slotSwitchView(1);
//    mainWindow.slotSwitchView(2);
//    mainWindow.slotSwitchView(3);
//}

////slotViewtransparentFrame
//TEST_F(testMainWindowGUI, slotViewtransparentFrame)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotViewtransparentFrame(true);
//    mainWindow.slotViewtransparentFrame(false);
//}

//bool stub_startDetached(const QString &program, const QStringList &arguments)
//{
//    Q_UNUSED(program)
//    Q_UNUSED(arguments)
//    return true;
//}

////slotViewShortcut
//TEST_F(testMainWindowGUI, slotViewShortcut)
//{
//    Stub stub;
//    stub.set((bool (*)(const QString &, const QStringList &))ADDR(QProcess, startDetached),
//             stub_startDetached);
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotViewShortcut();
//}

//TEST_F(testMainWindowGUI, resizeEvent_01)
//{
//    QResizeEvent event(QSize(10, 10), QSize(100, 100));
//    Calendarmainwindow mainWindow(1);
//    mainWindow.resizeEvent(&event);
//}

//TEST_F(testMainWindowGUI, resizeEvent_02)
//{
//    QResizeEvent event(QSize(10, 10), QSize(1000, 1000));
//    Calendarmainwindow mainWindow(1);
//    mainWindow.resize(1000, 1000);
//    mainWindow.resizeEvent(&event);
//}

//TEST_F(testMainWindowGUI, slotstackWClicked_01)
//{
//    Calendarmainwindow mainWindow(1);
//    QPushButton btn;
//    mainWindow.m_buttonBox->setId(&btn, 0);
//    mainWindow.slotstackWClicked(&btn);
//}

//TEST_F(testMainWindowGUI, slotWUpdateSchedule_01)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.m_opensearchflag = true;
//    mainWindow.m_searchEdit->setText("123");
//    mainWindow.slotWUpdateSchedule();
//}

//TEST_F(testMainWindowGUI, slotSreturnPressed_01)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.m_opensearchflag = true;
//    mainWindow.m_searchEdit->setText("123");
//    mainWindow.slotSreturnPressed();
//}

//TEST_F(testMainWindowGUI, slotStextChanged_01)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.m_searchEdit->setText("123");
//    mainWindow.slotStextChanged();
//}

//TEST_F(testMainWindowGUI, slotStextChanged_02)
//{
//    Calendarmainwindow mainWindow(1);
//    mainWindow.m_searchEdit->setText("");
//    mainWindow.slotStextChanged();
//}

//TEST_F(testMainWindowGUI, slotSearchSelectSchedule_02)
//{
//    Calendarmainwindow mainWindow(1);
//    ScheduleDataInfo info;
//    mainWindow.slotSearchSelectSchedule(info);
//}

//TEST_F(testMainWindowGUI, mouseMoveEvent_01)
//{
//    Calendarmainwindow mainWindow(1);
//    QMouseEvent event(QMouseEvent::MouseMove, QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
//    mainWindow.mouseMoveEvent(&event);
//}

//TEST_F(testMainWindowGUI, changeEvent_01)
//{
//    Calendarmainwindow mainWindow(1);
//    QEvent event(QEvent::ActivationChange);
//    mainWindow.changeEvent(&event);
//}

//TEST_F(testMainWindowGUI, slotOpenSettingDialog_01)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    Calendarmainwindow mainWindow(1);
//    mainWindow.slotOpenSettingDialog();
//}
