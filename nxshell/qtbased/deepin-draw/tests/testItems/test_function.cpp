// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define protected public
#define private public
#include "mainwindow.h"
#include "cexportimagedialog.h"
#include "qfiledialog.h"
#include "cspinbox.h"
#include "csizehandlerect.h"
#include "qgraphicssceneevent.h"
#include "globaldefine.h"
#include "ccuttool.h"
#include "ciconbutton.h"
#include "colorlabel.h"
#include "dzoommenucombobox.h"
#include "progresslayout.h"
#include "qevent.h"
#include "seperatorline.h"
#include "cexportimagedialog.h"
#include "drawdialog.h"
#include "baseutils.h"
#include "application.h"
#include "global.h"
#include "cvalidator.h"
#include "dbusdraw_adaptor.h"
#include "dfiledialog.h"
#include "cprogressdialog.h"
#include "qcoreevent.h"
#include "dzoommenucombobox.h"
#include "shortcut.h"
#include "accessiblefunctions.h"
#include "ctextedit.h"
#include "toolbutton.h"
#include "DataHanderInterface.h"
#include "cmenu.h"
#include "drawtoolfactory.h"
#include "toolsceneevent.h"
#include "pentool.h"
#include "textitem.h"
#undef protected
#undef private

#include <QTimer>
#include <QImage>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "toptoolbar.h"
#include "publicApi.h"
#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "sitemdata.h"
#include <QDebug>
#include <DLineEdit>
#include <DStandardPaths>
#include <QtConcurrent>
#include <DStandardPaths>
DCORE_USE_NAMESPACE;
//#include <stub-tool/cpp-stub/stub.h>
//#include <stub-tool/stub-ext/stubext.h>

#if TEST_FUNCTION_ITEM

TEST(TestFunction, LoadOldBLurItemAccept)
{
    // 打开ddf
    QFile file(":/oldBlurItem.ddf");
    ASSERT_EQ(true, file.open(QIODevice::ReadOnly));
    QByteArray data =  file.readAll();
    file.close();

    QString oldBlurItemDdf = QApplication::applicationDirPath() + "/oldBlurItem.ddf";
    QFile bfile(oldBlurItemDdf);
    bfile.open(QIODevice::WriteOnly);
    ASSERT_TRUE(bfile.write(data));
    QTest::qWait(200);
    bfile.close();

    PageContext *context = nullptr;
    setQuitDialogResult(0);
    context = getMainWindow()->drawBoard()->fileHander()->loadDdf(oldBlurItemDdf);
    setQuitDialogResult(-1);

    EXPECT_NE(context, nullptr);

    auto page = getMainWindow()->drawBoard()->addPage(context);

    EXPECT_NE(page, nullptr);

    page->close(true);
}

TEST(TestFunction, LoadOldPenItemAccept)
{
    // 打开ddf
    QFile file(":/oldPenItem.ddf");
    ASSERT_EQ(true, file.open(QIODevice::ReadOnly));
    QByteArray data =  file.readAll();
    file.close();

    QString oldPenItemDdf = QApplication::applicationDirPath() + "/oldPenItem.ddf";
    QFile bfile(oldPenItemDdf);
    bfile.open(QIODevice::WriteOnly);
    ASSERT_TRUE(bfile.write(data));
    QTest::qWait(200);
    bfile.close();

    PageContext *context = nullptr;
    setQuitDialogResult(0);
    context = getMainWindow()->drawBoard()->fileHander()->loadDdf(oldPenItemDdf);
    setQuitDialogResult(-1);

    EXPECT_NE(context, nullptr);

    auto page = getMainWindow()->drawBoard()->addPage(context);

    EXPECT_NE(page, nullptr);

    page->close(true);
}

TEST(TestFunction, Shortcut)
{
    Shortcut sc;
    ASSERT_EQ(sc.toStr().isEmpty(), false);
}

TEST(TestFunction, Global)
{
    //GlobalShortcut
    auto gsc = GlobalShortcut::instance();
    ASSERT_NE(gsc, nullptr);
    delete gsc;
    gsc = nullptr;

    //Global
    auto configPath = Global::configPath();
    ASSERT_EQ(configPath, DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

    //NSBlur
    QImage img_1;
    img_1 = NSBlur::blurImage(img_1);
    ASSERT_EQ(img_1.isNull(), true);

    QImage img_2(QSize(20, 20), QImage::Format_RGB32);
    img_2.fill(20);
    NSBlur::blurImage(img_2);

    QRect rect(10, 10, 10, 10);
    NSBlur::blurImage(img_2, 10, 1, rect);
}

TEST(TestFunction, CIntValidator)
{
    CIntValidator validator(20, 20);
    QString str("123");
    int n = 1;
    auto state = validator.validate(str, n);
    ASSERT_EQ(state, CIntValidator::State::Invalid);

    validator.fixup(str);
}

TEST(TestFunction, baseutils)
{
    //color list
    auto list = specifiedColorList();

    //pick color cursor
    auto cursor = pickColorCursor();
}

TEST(TestFunction, ACCESSIBILITY)
{
    //这个case在本地debug会进函数，但不会进覆盖率

    //getObjPrefix
    auto prefix_1 = getObjPrefix(QAccessible::Role::Button);
    ASSERT_EQ(prefix_1, QString("Btn"));

    auto prefix_2 = getObjPrefix(QAccessible::Role::StaticText);
    ASSERT_EQ(prefix_2, QString("Label"));

    auto prefix_3 = getObjPrefix(QAccessible::Role::Row);
    ASSERT_EQ(prefix_3, QString::fromLatin1(QMetaEnum::fromType<QAccessible::Role>().valueToKeys(QAccessible::Role::Row)));

    //getAccessibleName
    QWidget w;
    getAccessibleName(&w, QAccessible::Role::Window, "Window_");
}

TEST(TestFunction, ProgressLayout)
{
    ProgressLayout layout;

    layout.showInCenter(getMainWindow());
    layout.setRange(222, 555);
    layout.setProgressValue(255);
}

TEST(TestFunction, CIconButton)
{
    //补齐CIconButton的函数测试
    QMap<int, QMap<CIconButton::EIconButtonSattus, QString>> pictureMap;
    CIconButton button(pictureMap, QSize(55, 36));

    //checked
    button.setChecked(true);
    ASSERT_EQ(button.isChecked(), true);
    button.setChecked(false);
    ASSERT_EQ(button.isChecked(), false);

    //theme
    button.setTheme(0);

    //leave event
    button.enterEvent(nullptr);
    button.leaveEvent(nullptr);
}

TEST(TestFunction, CMenu)
{
    //补齐CMenu测试
    CMenu menu("123123");
}

TEST(TestFunction, ColorLabel)
{
    //补齐ColorLabel测试
    ColorLabel label;
    label.getColor(121, 0.5, 0.5);
    label.getColor(181, 0.5, 0.5);
    label.getColor(241, 0.5, 0.5);
    label.getColor(301, 0.5, 0.5);

    label.setHue(1);
    QTest::qWait(200);

    label.pickColor(QPoint(1, 1), true);
    label.pickColor(QPoint(1, 1), false);
    label.pickColor(QPoint(-1, 1), false);

    //event
    DTestEventList e;
    e.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(), 100);
    e.addMouseMove();
    e.addMouseRelease(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(), 100);
    e.simulate(&label);
    QTest::qWait(500);
}

TEST(TestFunction, CSpinBox)
{
    //补齐CSpinBox测试
    CSpinBox box;
    box.isChangedByWheelEnd();
    box.setSpecialText("123");

    QFocusEvent e_in(QEvent::Type::FocusIn);
    box.focusInEvent(&e_in);

    QFocusEvent e_out(QEvent::Type::FocusOut);
    box.focusOutEvent(&e_out);

    box.contextMenuEvent(nullptr);

    QMouseEvent e_md(QEvent::Type::MouseButtonDblClick, QPointF(1, 1), Qt::MouseButton::LeftButton, Qt::MouseButtons(), Qt::KeyboardModifier::NoModifier);
    box.mouseDoubleClickEvent(&e_md);
}

TEST(TestFunction, TextEdit)
{
    //补齐CTextEdit测试
    TextEdit textEdit(nullptr);
    textEdit.currentFormat(true);
    textEdit.toWeight("Black");
    textEdit.toStyle(87);
    textEdit.updateBgColorTo(QColor(50, 50, 50), true);
    QTest::qWait(500);

    //key event
    DTestEventList e;
    e.addKeyPress(Qt::Key_Z, Qt::KeyboardModifier::ControlModifier, 100);
    e.addKeyPress(Qt::Key_Y, Qt::KeyboardModifier::ControlModifier, 100);
    e.addKeyPress(Qt::Key_Z, Qt::KeyboardModifier::ShiftModifier | Qt::KeyboardModifier::ControlModifier, 100);
    e.simulate(&textEdit);
    QTest::qWait(500);
}

TEST(TestFunction, DZoomMenuComboBox)
{
    //补齐DZoomMenuComboBox测试
    QAction a1("123");
    QAction a2;
    QAction a3("321");
    QAction a4("123321");
    DZoomMenuComboBox box;
    box.addAction(&a1);
    box.addAction(&a2);
    box.addAction(&a3);
    box.addAction(&a4);

    //current
    box.setCurrentIndex(1);
    box.setCurrentIndex(-1);

    //remove
    box.removeItem("123");
    box.removeItem(0);
    box.removeItem(&a4);

    //set
    box.setItemICon("123456", QIcon());
}

TEST(TestFunction, SeperatorLine)
{
    //补齐SeperatorLine测试
    SeperatorLine line;
    line.updateTheme();
}

TEST(TestFunction, Page)
{
    //补齐Page测试
    auto page = getMainWindow()->drawBoard()->currentPage();

    //直接遍历一次，不崩就行。。。。。。
    page->setFile("123");
    page->file();
    page->setPageRect(QRectF());
    page->key();
    page->view();
    page->scene();
    page->saveToImage("");
    //page->updateContext();
}

TEST(TestFunction, DrawBoard)
{
    //补齐DrawBoard测试
    auto board = drawApp->topMainWindow()->drawBoard();
    Page *c = getMainWindow()->drawBoard()->currentPage();

    board->count();
    board->isAnyPageModified();
    board->setCurrentPage("123");
    board->setCurrentPage(c);
    board->load(QApplication::applicationDirPath() + "/test_cut.ddf");
    board->load(QApplication::applicationDirPath() + "/test.png");
}

TEST(TestFunction, CPenTool)
{
    //补齐CPenTool测试
    auto tool = DrawToolFactory::createTool(pen);

    //DrawTool::ITERecordInfo info;
    //info.elapsedFromStartToUpdate();

    //max touch point
    //tool->allowedMaxTouchPointCount();

    delete tool;
}

TEST(TestFunction, CDrawToolEvent)
{
    //补齐CDrawToolEvent测试
    auto page = getMainWindow()->drawBoard()->currentPage();
    auto scene = page->scene();

    //mouse
    QMouseEvent e_1(QEvent::Type::MouseButtonPress, QPointF(100, 100), Qt::MouseButton::LeftButton, Qt::MouseButtons(), Qt::KeyboardModifier::NoModifier);
    ToolSceneEvent::fromQEvent(&e_1, scene);

    //touch
    QTouchEvent e_2(QEvent::Type::TouchBegin);
    ToolSceneEvent::fromQEvent(&e_2, scene);

    //from touch
    ToolSceneEvent::fromTouchPoint(QTouchEvent::TouchPoint(), scene);
}

TEST(TestFunction, IDrawTool)
{
    //补齐IDrawTool测试
    PenTool tool;

    //init
    //tool.initToolButton();
    tool.attributions();

    //others
    tool.drawBoard();
    tool.setToolManager(drawApp->drawBoard()->toolManager());
    tool.isEnable();

    //tool.setDrawBoard(getMainWindow()->drawBoard());
    //tool.isEnable(nullptr);
    //tool.refresh();

    //event
    auto page = getMainWindow()->drawBoard()->currentPage();
    auto scene = page->scene();
//    ToolSceneEvent::CDrawToolEvents event(QPointF(100, 100), QPointF(150, 150), QPointF(200, 200), scene);
//    tool.drawItemStart(&event);
//    tool.drawItemUpdate(&event, nullptr);
//    tool.drawItemFinish(&event, nullptr);
}

TEST(TestFunction, MainWindow)
{
    //补齐mainwindow测试
    auto w = getMainWindow();

    //openfiles
    w->openFiles({QApplication::applicationDirPath() + "/test.png"}, true);
    w->openFiles({QApplication::applicationDirPath() + "/test_cut.ddf"}, false);

    //slots
    w->activeWindow();

    qMyWaitFor([ & ]() {
        w->slotShowOpenFileDialog();
        return true;
    }, 1000);
}

TEST(TestFunction, TextItem)
{
    //补齐CGraphicsTextItem测试

    TextItem tool("test");

    tool.isEditing();
    tool.setEditing(true);
    tool.setEditing(false);
    tool.textEditor();
    tool.setAlignment(Qt::AlignLeft);
    tool.setAlignment(Qt::AlignRight);
    tool.setAlignment(Qt::AlignTop);
    tool.setAlignment(Qt::AlignBottom);
    tool.setAlignment(Qt::AlignVCenter);
    tool.setAlignment(Qt::AlignCenter);
}

//TEST(TestFunction, ManageViewSigleton)
//{
//    //补齐CGraphicsTextItem测试
//    ASSERT_EQ(CManageViewSigleton::GetInstance()->isEmpty(), false);
//    ASSERT_GT(CManageViewSigleton::GetInstance()->viewCount(), 1);
//    ASSERT_NE(CManageViewSigleton::GetInstance()->getCurScene(), nullptr);
//    CManageViewSigleton::GetInstance()->quitIfEmpty();
//    CManageViewSigleton::GetInstance()->removeView(nullptr);
//    CManageViewSigleton::GetInstance()->removeView(getMainWindow()->drawBoard()->currentPage()->view());
//}

//TEST(TestFunction, TestCreateView)
//{
//    createNewViewByShortcutKey();
//}

//TEST(TestFunction, TestViewItem)
//{
//    CGraphicsView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    CCentralwidget *c = getMainWindow()->getCCentralwidget();
//    ASSERT_NE(c, nullptr);

//    drawApp->setCurrentTool(rectangle);

//    int addedCount = view->drawScene()->getBzItems().count();
//    createItemByMouse(view);
//    ASSERT_EQ(view->drawScene()->getBzItems().count(), addedCount + 1);
//    ASSERT_EQ(view->drawScene()->getBzItems().first()->type(), RectType);
//}
//TEST(TestFunction, TestCgraphicsItem)
//{

//    CGraphicsView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    CGraphicsItem *grap = dynamic_cast<CGraphicsItem *>(view->drawScene()->getBzItems().first());

//    //TestCellipsetool
//    CEllipseTool ellipsetool;
//    IDrawTool::ITERecordInfo pInfo;
//    pInfo.eventLife = IDrawTool::EEventLifeTo::EDoQtCoversion;
//    pInfo.businessItem = grap;
//    CDrawToolEvent event;
//    CDrawToolEvent event2;
//    CDrawToolEvent event3;
//    event._kbMods = Qt::ShiftModifier;
//    event2._kbMods = Qt::AltModifier;
//    event3._kbMods = Qt::NoModifier;
//    ellipsetool.toolCreatItemUpdate(&event, &pInfo);
//    QTest::qWait(200);
//    ellipsetool.toolCreatItemUpdate(&event2, &pInfo);
//    QTest::qWait(200);
//    ellipsetool.toolCreatItemUpdate(&event3, &pInfo);

//    //Testpolygonalstartool
//    CPolygonalStarTool polygonalstartool;
//    polygonalstartool.toolCreatItemUpdate(&event, &pInfo);
//    polygonalstartool.toolCreatItemUpdate(&event2, &pInfo);
//    polygonalstartool.toolCreatItemUpdate(&event3, &pInfo);

//    //Testpolygontool
//    CPolygonTool polygonltool;
//    polygonltool.toolCreatItemUpdate(&event, &pInfo);
//    polygonltool.toolCreatItemUpdate(&event2, &pInfo);
//    polygonltool.toolCreatItemUpdate(&event3, &pInfo);

//    //Testpolygontool
//    CRectTool rectTool;
//    rectTool.toolCreatItemUpdate(&event, &pInfo);
//    rectTool.toolCreatItemUpdate(&event2, &pInfo);
//    rectTool.toolCreatItemUpdate(&event3, &pInfo);

//    //Testpolygontool
//    CTriangleTool triangleTool;
//    triangleTool.toolCreatItemUpdate(&event, &pInfo);
//    triangleTool.toolCreatItemUpdate(&event2, &pInfo);
//    triangleTool.toolCreatItemUpdate(&event3, &pInfo);

//    grap->getCenter(CSizeHandleRect::LeftTop);
//    grap->getCenter(CSizeHandleRect::RightTop);
//    grap->getCenter(CSizeHandleRect::Top);
//    grap->getCenter(CSizeHandleRect::Right);
//    grap->getCenter(CSizeHandleRect::RightBottom);
//    grap->getCenter(CSizeHandleRect::Bottom);
//    grap->getCenter(CSizeHandleRect::LeftBottom);
//    grap->getCenter(CSizeHandleRect::Left);
//    grap->getCenter(CSizeHandleRect::Rotation);
//    grap->getCenter(CSizeHandleRect::InRect);
//    grap->getCenter(CSizeHandleRect::None);

//    grap->getSelfOrgShape();

//    QGraphicsSceneMouseEvent Event(QEvent::None);
//    grap->mousePressEvent(&Event);
//    grap->mouseMoveEvent(&Event);
//    grap->mouseReleaseEvent(&Event);
//    grap->mouseDoubleClickEvent(&Event);

//    grap->blurBegin(QPointF(1, 1));
//    grap->blurUpdate(QPointF(1, 1));
//    grap->blurEnd();

//    struct SBlurInfo sblurinfo;
//    grap->addBlur(sblurinfo);
//    QPixmap img(QSize(100, 100));
//    QPainter qpainter(&img);
//    QPainter *painter = &qpainter;
//    grap->paintBlur(painter, sblurinfo);
//    grap->resetCachePixmap();

//    grap->type();

//    bool b = true;
//    bool c = false;
//    grap->setFilpBaseOrg(CPictureItem::EFilpHor, b);
//    grap->setFilpBaseOrg(CPictureItem::EFilpVer, b);

//    grap->setCache(b);
//    grap->setCache(c);

//    int a = 1;
//    grap->setAutoCache(b, a);
//    //grap->isAutoCache();

//    QRectF rect;
//    grap->isRectPenetrable(rect);

//    CGraphicsUnit data;
//    grap->loadGraphicsUnit(data);
//    grap->hitTest(QPointF(1, 1));
//    data.release();

//    QStyleOptionGraphicsItem *style = nullptr;
//    grap->paintSelf(painter, style);


//    CGraphicsUnit un;
//    un = grap->getGraphicsUnit(ENormal);
//    grap->loadGraphicsUnit(un);
//    un.release();
//}

//TEST(TestFunction, TestCgraphicsCutItem)
//{
//    CGraphicsView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    CGraphicsCutItem cutitem;
//    cutitem.move(QPointF(0, 0), QPointF(0, 0));
//    cutitem.hitTest(QPointF(0, 0));
//    cutitem.getHighLightPath();
//    cutitem.isPosPenetrable(QPointF(0, 0));
//    QPointF point(1, 1);
//    cutitem.resizeCutSize(CSizeHandleRect::Left, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::LeftTop, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::Top, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::RightTop, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::Right, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::RightBottom, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::Bottom, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::LeftBottom, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::Rotation, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::InRect, QPointF(1, 1), QPointF(1, 1), &point);
//    cutitem.resizeCutSize(CSizeHandleRect::None, QPointF(1, 1), QPointF(1, 1), &point);

//    cutitem.getWHRadio();
//    cutitem.nodes();
//}

//TEST(TestFunction, TestCgraphicsTextItem)
//{
//    CGraphicsView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    CGraphicsTextItem textitem;
//    textitem.beginPreview();
//    bool loadorg = true;
//    textitem.endPreview(loadorg);
//    bool loador = false;
//    textitem.endPreview(loador);
//    textitem.isPreview();
//    QRectF qrect;
//    textitem.setRect(qrect);
//    QString str = "a";

//    CGraphicsUnit data;
//    data.reson = ENormal;
//    textitem.loadGraphicsUnit(data);
//    data.release();

//    textitem.setSelectTextBlockAlign(Qt::AlignTop);
//    textitem.setSelectTextBlockAlign(Qt::AlignBottom);
//    textitem.setSelectTextBlockAlign(Qt::AlignVCenter);
//    textitem.setSelectTextBlockAlign(Qt::AlignCenter);
//    textitem.doUndo();
//    textitem.doRedo();
//    textitem.doDelete();
//}

//TEST(TestFunction, TestSitemdata)
//{
//    CGraphicsUnit unit;
//    CGraphicsUnit cs;
//    CGraphicsUnit a;
//    cs.data.pRect = nullptr;
//    a.head.dataType = RectType;
//    unit.deepCopy(cs, a);
//    cs.data.pCircle = nullptr;
//    a.head.dataType = EllipseType;
//    unit.deepCopy(cs, a);
//    cs.data.pTriangle = nullptr;
//    a.head.dataType = TriangleType;
//    unit.deepCopy(cs, a);
//    cs.data.pPolygonStar = nullptr;
//    a.head.dataType = PolygonalStarType;
//    unit.deepCopy(cs, a);
//    cs.data.pPolygon = nullptr;
//    a.head.dataType = PolygonType;
//    unit.deepCopy(cs, a);
//    cs.data.pLine = nullptr;
//    a.head.dataType = LineType;
//    unit.deepCopy(cs, a);
//    cs.data.pPen = nullptr;
//    a.head.dataType = PenType;
//    unit.deepCopy(cs, a);
//    cs.data.pText = nullptr;
//    a.head.dataType = TextType;
//    unit.deepCopy(cs, a);
//    cs.data.pPic = nullptr;
//    a.head.dataType = PictureType;
//    unit.deepCopy(cs, a);
//    cs.data.pBlur = nullptr;
//    a.head.dataType = BlurType;
//    unit.deepCopy(cs, a);
//    cs.data.pGroup = nullptr;
//    a.head.dataType = MgrType;
//    unit.deepCopy(cs, a);

//    unit.release();
//    cs.release();
//    a.release();

//}

//TEST(TestFunction, TestCviewmanagement)
//{
//    int type = 0;
//    CManageViewSigleton sigleton;
//    QString ddfFile = QApplication::applicationDirPath() + "/test_ellipse.ddf";
//    CManageViewSigleton::GetInstance()->setThemeType(type);
//    CManageViewSigleton::GetInstance()->updateTheme();


//    CManageViewSigleton::GetInstance()->getNoticeFileDialog(ddfFile);
//    QWidget parent;
//    CManageViewSigleton::GetInstance()->creatOneNoticeFileDialog("s", &parent);

//    CManageViewSigleton::GetInstance()->getFileSrcData(ddfFile);


//    QTimer::singleShot(1000, drawApp->topMainWindowWidget(), [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(1);
//        }
//    });
//    CManageViewSigleton::GetInstance()->onDDfFileChanged(ddfFile);



//    CManageViewSigleton::GetInstance()->onDdfFileChanged(ddfFile, type);


//}

//TEST(TestFunction, TestMultiptabbarwidget)
//{
//    CMultipTabBarWidget tabbar;
//    tabbar.getAllTabBarUUID();
//    tabbar.clearHoverState();
//    QObject o;
//    QMouseEvent *mouseevent = new QMouseEvent(QMouseEvent::MouseButtonPress, QPointF(1, 1), Qt::MouseButton::LeftButton,
//                                              Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
//    tabbar.eventFilter(&o, mouseevent);

//    delete mouseevent;
////    TEST(TestFunction, TestCentralWidget) {
////        CGraphicsView *view = getCurView();
////        QString filepath = "Unnamed";
////        QStringList str;
////        str << "rect" << "ellipse";
////        DWidget parent;
////        CCentralwidget *tralwidget = drawApp->topMainWindow()->getCCentralwidget();
////        tralwidget->getLeftToolBar();
////        tralwidget->getGraphicsView();
////        int type = 1;
////        tralwidget->switchTheme(type);
////        tralwidget->getAllTabBarUUID();

////        bool isimagesize = true;
////        bool a = true;

////        tralwidget->closeSceneView(view, isimagesize, a);

////        tralwidget->slotShowCutItem();

////        tralwidget->slotShowExportDialog();
////        qreal scale = true;
////        tralwidget->slotSetScale(scale);
////        tralwidget->getSceneImage(type);
////        tralwidget->initConnect();
////        >>> >>> > uos - feature
////    }
//}

////TEST(TestFunction, TestCentralWidget)
////{
////    CGraphicsView *view = getCurView();
////    QString filepath = "Unnamed";
////    QStringList str;
////    str << "rect" << "ellipse";
////    //DWidget parent;
////    CCentralwidget *tralwidget = /*new CCentralwidget(str, &parent)*/drawApp->topMainWindow()->getCCentralwidget();
////    tralwidget->getLeftToolBar();
////    tralwidget->getGraphicsView();
////    int type = 1;
////    tralwidget->switchTheme(type);
////    tralwidget->getAllTabBarUUID();

////    bool isimagesize = true;
////    bool a = true;

////    //tralwidget->closeSceneView(view, isimagesize, a);
////    tralwidget->slotShowCutItem();
////    tralwidget->slotShowExportDialog();
////    qreal scale = true;
////    tralwidget->slotSetScale(scale);
////    tralwidget->getSceneImage(type);
////    tralwidget->initConnect();
////}

//TEST(TestFunction, TestGraphicsview)
//{
//    CGraphicsView *view = getCurView();
//    //view->slotDoCutScene();
//    view->slotViewZoomIn();
//    view->slotViewZoomOut();
//    view->slotViewOrignal();
//    view->slotSetTextAlignment(Qt::AlignmentFlag::AlignLeft);
//    bool type = true;
//    bool finishClose = false;
//    QString save = "sad";
//    QTimer::singleShot(1000, view, [ = ]() {
//        auto dial = qobject_cast<QFileDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    });
//    view->setSaveDialogMoreOption(QFileDialog::DontUseNativeDialog);
//    view->showSaveDDFDialog(type, finishClose, save);
//    view->setSaveDialogMoreOption(QFileDialog::DontResolveSymlinks);

//    QPinchGesture gesture2;
//    view->pinchTriggered(&gesture2);

//}

//TEST(TestFunction, TestLefttoolbar)
//{
//    QMouseEvent mouseevent(QEvent::None, QPointF(1, 1), Qt::MouseButton::NoButton,
//                           Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
//    QObject o;
//    CMultipTabBarWidget tiptabar;
//    tiptabar.getAllTabBarUUID();
//    tiptabar.getAllTabBarName();
//    tiptabar.eventFilter(&o, &mouseevent);
//}

//TEST(TestFunction, TestDrawTools)
//{
//    CGraphicsView *view = getCurView();
//    ASSERT_NE(view, nullptr);
//    QString uuid = "123";
//    bool isModified = false;
//    CDrawScene *scence = new CDrawScene(view, uuid, isModified);
//    CDrawToolEvent event(QPointF(1, 1), QPointF(1, 1), QPointF(1, 1), scence);
//    IDrawTool::ITERecordInfo info;
//    CCutTool cuttool;
//    cuttool.toolStart(&event, &info);
//    cuttool.toolUpdate(&event, &info);
//    cuttool.toolFinish(&event, &info);
//    cuttool.mouseHoverEvent(&event);
//    cuttool.getCutRect(scence);
//    cuttool.toolCreatItemStart(&event, &info);
//    cuttool.getCurVaildActivedPointCount();
//    cuttool.isWorking();
//    event.pos(CDrawToolEvent::EPosType::EScenePos);
//    event.orgQtEvent();

//    CEllipseTool ellipsetool;
//    ellipsetool.toolCreatItemUpdate(&event, &info);

//    CLineTool linetool;
//    linetool.toolCreatItemUpdate(&event, &info);
//    linetool.toolCreatItemFinish(&event, &info);

////    CMasicoTool masicotool;
////    masicotool.toolCreatItemUpdate(&event, &info);
////    masicotool.toolCreatItemFinish(&event, &info);
////    masicotool.decideUpdate(&event, &info);

//    CPolygonalStarTool startool;
//    startool.toolCreatItemUpdate(&event, &info);

//    CPolygonTool gontool;
//    gontool.toolCreatItemUpdate(&event, &info);

//    CRectTool recttool;
//    recttool.toolCreatItemUpdate(&event, &info);

//    CSelectTool selecttool;
//    selecttool.toolStart(&event, &info);
//    selecttool.decideUpdate(&event, &info);

//    CTextTool texttool;
//    texttool.toolCreatItemFinish(&event, &info);

//    CTriangleTool triangletool;
//    triangletool.toolCreatItemUpdate(&event, &info);
//}

//TEST(TestFunction, Testwidegets)
//{
//    int index = 1;

//    bool ismultcolorsame = true;
//    bool picked = false;

//    QMouseEvent mouseevent(QEvent::None, QPointF(1, 1), Qt::MouseButton::NoButton,
//                           Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);

//    ColorLabel colorlabel;
//    colorlabel.setHue(index);
//    colorlabel.pickColor(QPoint(1, 1), ismultcolorsame);
//    colorlabel.pickColor(QPoint(1, 1), picked);
//    colorlabel.mouseMoveEvent(&mouseevent);
//    colorlabel.mouseReleaseEvent(&mouseevent);

//    QString text = "ellipse";

//    QIcon icon;
//    QAction action;
//    int a = -1;
//#if 0
//    DMenuComboBox menucombobox;
//    menucombobox.addItem(text);
//    menucombobox.addItem(text, icon);
//    menucombobox.addItem(&action);
//    menucombobox.removeItem(text);
//    menucombobox.removeItem(index);
//    menucombobox.removeItem(&action);
//    menucombobox.setCurrentIndex(index);
//    menucombobox.setCurrentIndex(a);
//    menucombobox.setCurrentText(text);
//    menucombobox.setMenuFlat(ismultcolorsame);
//    menucombobox.setItemICon(text, icon);
//    menucombobox.setItemICon(index, icon);
//    menucombobox.slotActionToggled(&action);
//    menucombobox.slotAboutToShow();
//    menucombobox.initUI();
//    menucombobox.initConnection();
//#endif

//    DZoomMenuComboBox zoomcombox;
//    zoomcombox.addItem(text);
//    zoomcombox.addItem(text, icon);
//    zoomcombox.removeItem(&action);
//    zoomcombox.setCurrentIndex(index);
//    zoomcombox.setItemICon(index, icon);
//    zoomcombox.setItemICon(a, icon);
//    zoomcombox.setMenuButtonTextAndIcon(text, icon);
//    zoomcombox.slotActionToggled(&action);


//    ProgressLayout gresslayout;
//    int end = 2;
//    gresslayout.setRange(index, end);


//    QRect rect;
//    QPaintEvent paintevent(rect);

//    SeperatorLine line;
//    line.updateTheme();


//    CExportImageDialog portimage;
//    portimage.getQuality();
//    portimage.initUI();
//    int pic = 0, doc = 1, dow = 2;

//    portimage.slotOnDialogButtonClick(pic, text);
//    portimage.slotOnDialogButtonClick(doc, text);
//    portimage.slotOnDialogButtonClick(dow, text);
//    portimage.slotOnQuestionDialogButtonClick(doc, text);
//    portimage.slotOnQualityChanged(doc);
//    portimage.showQuestionDialog(text);
////    portimage.isHaveSuffix(text);

//    DrawDialog dialog;
//    QKeyEvent keyEvent(QEvent::None, Qt::Key_Down, Qt::KeyboardModifier::NoModifier);
//    dialog.keyPressEvent(&keyEvent);
////    char ch = 'b';
////    char *p;
////    p = &ch;
////    Application lication(doc, &p);
////    QStringList drawpath;
////    drawpath << "deepin";
////    lication.execDraw(drawpath);
////    lication.leftToolBar();
////    lication.onAppQuit();
////    lication.setTouchFeelingEnhanceValue(doc);
////    lication.activateWindow();

//    pickColorCursor();
//    GlobalShortcut shortcut;
//    Global lobal;
//    shortcut.instance();
//    lobal.configPath();
//    CIntValidator validator;
//    validator.fixup(text);

//    CGraphicsItemGroup itemgroup;
//    itemgroup.getSelfOrgShape();
//    //qreal angle = true;
//    //itemgroup.rotatAngle(angle);
//    itemgroup.move(QPointF(1, 1), QPointF(1, 1));
//    //itemgroup.isNoContent();
//    //itemgroup.nodes();
//    itemgroup.getMinZ();

//    CDrawParamSigleton sigleton;
//    QPen pen;
//    sigleton.setPen(pen);
//    sigleton.setAltKeyStatus(ismultcolorsame);
//    //sigleton.getAltKeyStatus();
//    //sigleton.getTextSize();
//    //sigleton.setTextColorAlpha(doc);
//    sigleton.getCutType();
//    //sigleton.getCutAttributeType();
//}

//TEST(TestFunction, Testitem)
//{
//    QLineF linef;
//    bool init = true;
//    CGraphicsLineItem hicslineitem(linef);
//    CGraphicsLineItem hicslineitem2(QPointF(1, 2), QPointF(1, 1));
//    hicslineitem.line();
//    hicslineitem.type();
//    hicslineitem.setLine(linef, init);
//    hicslineitem2.updateShape();

//    CGraphicsPenItem penitem;
//    penitem.type();
//    penitem.updateStartPathStyle();
//    QPainterPath path;
//    //penitem.setPenEndpath(path);
//    penitem.getPenEndpath();
//    penitem.setPath(path);

//    QStringList list;
//    list << "deepin";
////    drawApp->execDraw(list);
////    drawApp->onAppQuit();
//    int value = 1;
//    drawApp->setTouchFeelingEnhanceValue(value);
//    drawApp->activateWindow();
//    drawApp->leftToolBar();
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->noticeFileRightProblem(list, Application::EFileClassEnum::ENotExist, init);
//    QTest::qWait(200);
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->noticeFileRightProblem(list, Application::EFileClassEnum::ENotFile, init);
//    QTest::qWait(200);
////    QTimer::singleShot(1000, drawApp->topMainWindowWidget(), [ = ]() {
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->noticeFileRightProblem(list, Application::EFileClassEnum::EDrawAppSup, init);
//    QTest::qWait(200);
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->noticeFileRightProblem(list, Application::EFileClassEnum::EDrawAppNotSup, init);
//    QTest::qWait(200);
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->noticeFileRightProblem(list, Application::EFileClassEnum::EDrawAppSupAndReadable, init);
//    QTest::qWait(200);
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->noticeFileRightProblem(list, Application::EFileClassEnum::EDrawAppSupButNotReadable, init);
//    QTest::qWait(200);

//    QList <int> intlist;
//    QString str = " ";

//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->exeMessage(str, Application::EMessageType::ENormalMsg, init, list, intlist);
//    QTest::qWait(200);
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->exeMessage(str, Application::EMessageType::EWarningMsg, init, list, intlist);
//    QTest::qWait(200);
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    drawApp->exeMessage(str, Application::EMessageType::EQuestionMsg, init, list, intlist);
//    QTest::qWait(200);
//    /*QTimer::singleShot*/QMetaObject::invokeMethod(drawApp/*, drawApp->topMainWindowWidget()*/, [ = ]() {
//        auto dial = qobject_cast<DDialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    }, Qt::QueuedConnection);
//    QString ddfFile = QApplication::applicationDirPath() + "/test_ellipse.ddf";
//    CManageViewSigleton::GetInstance()->onDDfFileChanged(ddfFile);
//}

////TEST(TestFunction, TestDrawScene)
////{
////    CGraphicsView *view = getCurView();
////    ASSERT_NE(view, nullptr);
////    QString uuid = "123";
////    bool isModified = false;
////    CDrawScene *scene = new CDrawScene(view, uuid, isModified);
////    QGraphicsSceneMouseEvent mouseevent;
////    scene->doLeave();
////    scene->event(&mouseevent);

////    CGroupBzItemsTreeInfo info;
////    bool notclear = true;
////    scene->loadGroupTreeInfo(info, notclear);
////    scene->setDrawForeground(notclear);

////    CGraphicsItemGroup *itemgroup = nullptr;
////    scene->cancelGroup(itemgroup, isModified);

////    scene->blockMouseMoveEvent(isModified);
////    scene->isBlockMouseMoveEvent();
////}

//TEST(TestFunction, TestDbusdraw)
//{
//    QObject parent;
//    dbusdraw_adaptor adaptor(&parent);
////    QString filepath;
////    filepath = "";
////    adaptor.openFile(filepath);
//    QList<QVariant> images;
//    //adaptor.openImages(images);
//    adaptor.openFiles(images);
//}

//TEST(TestFunction, TestDialog)
//{
////    QMap<int, QMap<CCheckButton::EButtonSattus, QString> > pictureMap;
////    QSize size(100, 100);
////    CCheckButton checkbutton(pictureMap, size, nullptr, true);
////    checkbutton.setChecked(true);
////    checkbutton.setChecked(false);
////    checkbutton.isChecked();
////    QMouseEvent *mouseevent = new QMouseEvent(QMouseEvent::MouseButtonPress, QPointF(1, 1), Qt::MouseButton::LeftButton,
////                                              Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
////    checkbutton.mousePressEvent(mouseevent);

////    delete mouseevent;

//    getMainWindow()->showSaveQuestionDialog();
//    getMainWindow()->activeWindow();
//    getMainWindow()->slotIsNeedSave();
//    getMainWindow()->slotContinueDoSomeThing();
//    QTest::qWait(200);
//    //getMainWindow()->onViewShortcut();
//    QString ellipseitempath = QApplication::applicationDirPath() + "/test_ellipse.ddf";
//    //QString BlurItemPath = QApplication::applicationDirPath() + "/test_blur.ddf";
//    QStringList strlist;
//    strlist.append(ellipseitempath);
//    //strlist.append(BlurItemPath);
//    QTest::qWait(200);
//    QTimer::singleShot(1000, drawApp->topMainWindowWidget(), [ = ]() {
//        auto dial = qobject_cast<Dialog *>(qApp->activeModalWidget());
//        if (dial != nullptr) {
//            dial->done(0);
//        }
//    });
//    getMainWindow()->openFiles(strlist);
//    getMainWindow()->slotOnThemeChanged(DGuiApplicationHelper::ColorType::DarkType);
//    getMainWindow()->closeTabViews();
//}

#endif
