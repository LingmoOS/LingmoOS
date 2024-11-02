/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "mainwindow.h"
#include "include/xatom-helper.h"
#include "bottompointlabel.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include "include/global.h"
#include <KWindowSystem>


//MainWindow * MainWindow::mutual = nullptr;

MainWindow::MainWindow(QWidget *parent,QString isoPath)
    : QWidget(parent)
{
        // 去除窗管标题栏，传入参数为QWidget *
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        isWayland = true;
    }
    if(isWayland){
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    }
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, [=](const kdk::WindowId &windowId) {
        if (getpid() == kdk::WindowManager::getPid(windowId)&&m_windowId.isEmpty()) {
            m_windowId = windowId.toString();
        }
    });
    init();
    statusbarInit();
    myStyle();
    initGsetting();
    menu->themeUpdate();        //画一次主题
    fontSizeInit();
    if(!isoPath.isEmpty()){     //支持接收一个命令行参数
        QFileInfo file(isoPath);
        if(file.suffix() == "iso"){
            page1->dealSelectedFile(isoPath);
        }
    }
//    //判断是否是平板
//    QDBusConnection sessionBus = QDBusConnection::sessionBus();

//    QDBusMessage m = QDBusMessage::createMethodCall(QString("com.lingmo.statusmanager.interface"),
//                                                    QString("/"),
//                                                    QString("com.lingmo.statusmanager.interface"),
//                                                    QString("get_current_tabletmode"));

//    QDBusMessage res = sessionBus.call(m);
//    if (res.type() == 1 || res.type() == 2){

//        m_isTabletMode = res.arguments().first().toBool();
//         qInfo()<<"m_isTabletMode"<<m_isTabletMode;
//    }
//    else{
//        qInfo()<<"m_isTabletMode"<<m_isTabletMode;

//        m_isTabletMode = false;
//    }
//    //切换模式
//    QDBusConnection::sessionBus().connect(QString("com.lingmo.statusmanager.interface"),
//                                          QString("/"),
//                                          QString("com.lingmo.statusmanager.interface"),
//                                          QString("mode_change_signal"), this,
//                                          SLOT(uiModeChange(bool)));



}


MainWindow::~MainWindow()
{
}


void MainWindow::fontSizeInit(){
    connect(this,&MainWindow::setFontSize,this,&MainWindow::dealSetFontSize);

    QGSettings* m_fontsize = nullptr;
    if(QGSettings::isSchemaInstalled(LINGMO_STYLE)){
        m_fontsize = new QGSettings(LINGMO_STYLE);
        connect(m_fontsize,&QGSettings::changed,this,[=](const QString &key){
            if(key == LINGMO_FONT_SIZE){
                double fontsize = m_fontsize->get(LINGMO_FONT_SIZE).toString().toDouble();
                if(fontsize > 0){
                    emit setFontSize(fontsize);
                }
            }
        });
    }
    if(m_fontsize){
        dealSetFontSize(m_fontsize->get(LINGMO_FONT_SIZE).toString().toDouble());
    }
}

void MainWindow::dealSetFontSize(double size){
    QFont font;
    font.setPointSize(size);
    this->setFont(font);
    menu->setFont(font);
}
void MainWindow::statusbarInit()
{
    title = new QWidget;
    title->setFixedSize(472,40);
    QHBoxLayout *hlyt = new QHBoxLayout();
    hlyt->setMargin(0);
    hlyt->setSpacing(0);
 //   titleIcon = new QPushButton;
    titleIcon = new QLabel;
    titleIcon->setFixedSize(24,24);

    titleIcon->setPixmap(QIcon::fromTheme("lingmo-usb-creator").pixmap(24,24));
   // titleIcon->setIconSize(QSize(24,24));
    titleIcon->setFocusPolicy(Qt::NoFocus);
    titleText = new QLabel();
    titleText->setText(tr("usb boot maker"));
    titleMin = new QPushButton();
    titleMin->setToolTip(tr("Minimize"));
    titleMin->setProperty("isWindowButton", 0x1);
    titleMin->setProperty("useIconHighlightEffect", 0x2);
    titleMin->setFlat(true);
    titleMin->setFixedSize(30, 30);
    titleMin->setIconSize(QSize(16, 16));
    titleMin->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    connect(titleMin,&QPushButton::clicked,[=](){
       this->setWindowState(Qt::WindowMinimized);
       this->showMinimized();
    });
    menu  = new menuModule(this);
    m_menu = new QMenu(this);
    actionAbout = new QAction(this);
    actionHelp = new QAction(this);
    actionQuit=new QAction(this);
    actionAbout->setText(tr("About"));
    actionHelp->setText(tr("Help"));
    actionQuit->setText(tr("Quit"));


    menuButton = new QToolButton;
    menuButton->setToolTip(tr("Options"));
    menuButton->setProperty("isWindowButton", 0x1);
    menuButton->setProperty("useIconHighlightEffect", 0x2);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setFixedSize(30,30);
    menuButton->setIconSize(QSize(16, 16));
    menuButton->setAutoRaise(true);
    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    menuButton->setMenu(m_menu);
    m_menu->addAction(actionHelp);
    m_menu->addAction(actionAbout);
    m_menu->addAction(actionQuit);
    connect(actionAbout,&QAction::triggered,this,&MainWindow::showaboutslot);
    connect(actionHelp,&QAction::triggered,this,&MainWindow::dealMenuModulePullupHelp);
    connect(actionQuit,&QAction::triggered,[=](){
        QDBusMessage exit_proc = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/","com.lingmousbcreator.interface","MakeExit");
        QDBusConnection::systemBus().call(exit_proc);
        this->close();
    });
    connect(menu,&menuModule::menuModuleSetThemeStyle,this,&MainWindow::setThemeStyle);
    connect(menu,&menuModule::iconchange,this,&MainWindow::setIconchange);
    connect(this,&MainWindow::setFontSize,menu,&menuModule::dealSetFontSize);


    titleClose = new QPushButton();
    titleClose->setToolTip(tr("Close"));
    titleClose->setProperty("isWindowButton", 0x2) ;
    titleClose->setProperty("useIconHighlightEffect", 0x8);;
    titleClose->setFlat(true);
    titleClose->setFixedSize(30, 30);
    titleClose->setIconSize(QSize(16, 16));
    titleClose->setIcon(QIcon::fromTheme("window-close-symbolic"));
    connect(titleClose,&QPushButton::clicked,[=](){
       this->close();
    });
    QHBoxLayout *hlt0 = new QHBoxLayout;
    hlt0->setContentsMargins(0,0,0,0);
    hlt0->setSpacing(0);
    hlt0->addStretch();
    hlt0->addWidget(menuButton,1);
    hlt0->addSpacing(4);
    hlt0->addWidget(titleMin,1);
    hlt0->addSpacing(4);
    hlt0->addWidget(titleClose,1);
    hlt0->addSpacing(4);
    QVBoxLayout *vlt0 = new QVBoxLayout;
    vlt0->setMargin(0);
    vlt0->setSpacing(0);
    vlt0->addSpacing(4);
    vlt0->addLayout(hlt0);
    vlt0->addStretch(); // 顶上充满
    QHBoxLayout *hlt=new QHBoxLayout;//标题栏内部
    hlt->setMargin(0);
    hlt->setSpacing(0);
    hlt->addSpacing(8);
    hlt->addWidget(titleIcon,1);
    hlt->addSpacing(4);
    hlt->addWidget(titleText,1);
    hlt->addStretch(99);
    hlt->addLayout(vlt0);
    hlyt->addLayout(hlt);
    title->setLayout(hlyt);
}
void MainWindow::setIconchange(){
        titleIcon->setPixmap(QIcon::fromTheme("lingmo-usb-creator").pixmap(24,24));
}
void MainWindow::showaboutslot(){
    aboutWindow=new aboutdialog(this);
//    MotifWmHints hints;
//    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
//    hints.functions = MWM_FUNC_ALL;
//    hints.decorations = MWM_DECOR_BORDER;
//    XAtomHelper::getInstance()->setWindowMotifHint(aboutWindow->winId(), hints);

}
void MainWindow::init(){
//    bool isWayland=false;
    this->setWindowTitle(tr("usb boot maker"));
    this->setFixedSize(472,648);
//    在屏幕中央显示
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    qInfo()<<"148availableGeometry"<<availableGeometry;
    this->move((availableGeometry.width()-this->width())/2,(availableGeometry.height()- this->height())/2);
    m_DaemonIpcDbus = new DaemonIpcDbus();
//    连结systembus进程消息
    QDBusConnection::systemBus().connect(QString(),QString("/"),"com.lingmousbcreator.interface","authorityStatus",this,SLOT(dealAuthorityStatus(QString)));

    this->setAcceptDrops(true);
}
void MainWindow::aboutClick()
{
//    about->setFixedSize(424,298);
//    about->setWindowFlag(Qt::FramelessWindowHint);
//    about->show();
}

void MainWindow::myStyle()
{
    timer = new QTimer(this);
    page1 = new Page1();
    page2 = new Page2();
    connect(page1,&Page1::starmake,page2,&Page2::makestar);
    connect(page2,&Page2::mainclose,this,&MainWindow::closemain);
    connect(page2,&Page2::makeFinish,this,&MainWindow::makeFinish);
    connect(page2,&Page2::returnMain,this,&MainWindow::returnMain);
    //内部样式
    QSize pointSize(8,8);
    pointLable1=new bottompointlabel("",this,36,QColor(45,135,255));
   // pointLable1=new QLabel;
    pointLable2=new bottompointlabel("",this,36,QColor(151, 151, 151));
    pointLable3=new bottompointlabel("",this,36,QColor(151, 151, 151));
    pointLable1->setFixedSize(pointSize);
    pointLable2->setFixedSize(pointSize);
    pointLable3->setFixedSize(pointSize);
    //pointLable1->setStyleSheet("border-radius:4px;background:rgba(45,135,255, 1)");
  //  pointLable2->setStyleSheet("border-radius:4px;background:rgba(151, 151, 151, 1)");
  //  pointLable3->setStyleSheet("border-radius:4px;background:rgba(151, 151, 151, 1)");
    pageIndex = PAGE_ONE;
    stackedWidget =new QStackedWidget(this);
    stackedWidget->addWidget(page1);
    stackedWidget->addWidget(page2);
    //布局
    QHBoxLayout *hlt =new QHBoxLayout;
    hlt->addStretch(15);
    hlt->addWidget(pointLable1);
    hlt->addStretch(1);
    hlt->addWidget(pointLable2);
    hlt->addStretch(1);
    hlt->addWidget(pointLable3);
    hlt->addStretch(15);
    bottomWidget = new QWidget();
    bottomWidget->setLayout(hlt);
    QVBoxLayout *vlt =new QVBoxLayout;
    vlt->setMargin(0);
    vlt->setSpacing(0);
    vlt->addWidget(title);
    vlt->addWidget(stackedWidget,99);
//    vlt->addLayout(hlt,1);
    vlt->addWidget(bottomWidget,1);
    vlt->addSpacing(20);
    this->setLayout(vlt);
    // 状态栏初始化部分，需要时打开注释
//    createTrayActions();
//     关于窗口绘制
    //about = new QWidget();

    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
}

void MainWindow::closemain(){
    this->close();
    QDBusMessage exit_proc = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/","com.lingmousbcreator.interface","MakeExit");
    QDBusConnection::systemBus().call(exit_proc);
}
void MainWindow::initGsetting()
{
    //应用主窗口状态
    if(QGSettings::isSchemaInstalled(APPDATA))
    {}
    return ;
}
void MainWindow::setThemeStyle(QString str)
{
    if("dark-theme" == str)
    {
        menu->m_isDarktheme=true;
        setThemeDark();
    }else{
        menu->m_isDarktheme=false;
        setThemeLight();
    }
}

void MainWindow::createTrayActions()
{
    if(!QSystemTrayIcon::isSystemTrayAvailable())
    {
        return;
    }
    m_mainTray = new QSystemTrayIcon(this);
    m_mainTray->setIcon(QIcon(":/data/lingmo-usb-creator.png"));
    m_mainTray->setToolTip(tr("usb boot maker"));
    m_mainTray->show();
}

void MainWindow::dealMenuModulePullupHelp(){
    if(!m_DaemonIpcDbus->daemonIsNotRunning()){
     //   m_DaemonIpcDbus->showGuide("tools/lingmo-usb-creator");
        m_DaemonIpcDbus->showGuide("lingmo-usb-creator");
    }
}
void MainWindow::makeStart()
{
    disconnect(titleClose,&QPushButton::clicked,0,0); //开始制作之后取消之前click触发的应用关闭功能
    connect(titleClose,&QPushButton::clicked,this,&MainWindow::doubleCheck);
//    disconnect()
    page2->startMaking();
    //page2->uDiskPath = page1->getDevPath();
    page2->uDiskPath = page2->usbpath;

    stackedWidget->setCurrentIndex(1);
    pointLable1->setBackgroundColor(QColor(151,151,151));
    pointLable2->setBackgroundColor(QColor(45,135,255));
    pointLable3->setBackgroundColor(QColor(151,151,151));
    //pointLable1->setStyleSheet("border-radius:4px;background:rgba(151, 151, 151, 1)");
    //pointLable2->setStyleSheet("border-radius:4px;background:rgba(45,135,255, 1)");
   // pointLable3->setStyleSheet("border-radius:4px;background:rgba(151, 151, 151, 1)");
    pageIndex = PAGE_TWO;
}

void MainWindow::doubleCheck(){
    QMessageBox::StandardButton result =  QMessageBox::warning(this,tr("Warning"),tr("USB driver is in production.Are you sure you want to stop task and exit the program?"),
                         QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    switch (result){
        case QMessageBox::Yes:{//exit_proc在其他case中也有效，不加花括号exit_proc的生命周期就不会终结，在其他case中就会成为一个没有初始化的变量
            // exit progress and close mainwindow
            QDBusMessage exit_proc = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/","com.lingmousbcreator.interface","MakeExit");
            QDBusConnection::systemBus().call(exit_proc);
            this->close();
            break;
        }
        case QMessageBox::No:
            break;
    }
}
int MainWindow::changePage()
{
    int count = stackedWidget->count();
    int index = stackedWidget->currentIndex();
    ++index;
    if (index >= count)index = 0;
    return index;
}

void MainWindow::handleIconClickedSub(const QString& iso_path)
{
    if(isWayland)
    {
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    }else{
    // 添加窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
//    this->isActiveWindow()
   // this->move((QApplication::desktop()->width() - this->width())/2,(QApplication::desktop()->height() - this->height())/2);

    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    this->move(desk_x/2-x/2+desk_rect.left(),desk_y/2-y/2+desk_rect.top());
    kdk::WindowManager::setGeometry(this->windowHandle(),this->geometry());

//    if(this->menu && this->menu->aboutWindow){
//        this->menu->aboutWindow->close();
//    }
    this->setWindowFlag(Qt::WindowStaysOnTopHint,true);
    this->setWindowFlag(Qt::WindowStaysOnTopHint,false);
    this->setWindowState(Qt::WindowActive | Qt::WindowNoState);
    this->show();
   if(pageIndex == PAGE_ONE){
    if (!iso_path.isEmpty())
        emit this->setIsoPath(iso_path);
   }
}

void MainWindow::slotActiveWindow(const QString& iso_path)
{
    if(isWayland){
        kdk::WindowManager::activateWindow(m_windowId);
    }else{
        KWindowSystem::forceActiveWindow(this->winId());
    }
    qDebug()<<__LINE__<<__func__<<this->winId()<<m_windowId;
    int sw=QGuiApplication::primaryScreen()->availableGeometry().width();
    int sh=QGuiApplication::primaryScreen()->availableGeometry().height();
    kdk::WindowManager::setGeometry(this->windowHandle(),QRect((sw-this->width())/2,(sh-this->height())/2,this->width(),this->height()));
    if(pageIndex == PAGE_ONE){
     if (!iso_path.isEmpty()){
         emit this->setIsoPath(iso_path);
     }
    }
}

void MainWindow::makeFinish()
{
    disconnect(titleClose,&QPushButton::clicked,0,0); //开始制作之后取消之前click触发的应用关闭功能
    connect(titleClose,&QPushButton::clicked,[=](){
       this->close();
    });
    pointLable1->setBackgroundColor(QColor(151,151,151));
    pointLable2->setBackgroundColor(QColor(151,151,151));
    pointLable3->setBackgroundColor(QColor(45,135,255));
  //  pointLable3->setStyleSheet("border-radius:4px;background:rgba(45,135,255, 1);");
  //  pointLable2->setStyleSheet("border-radius:4px;background:rgba(151, 151, 151, 1);");
  //  pointLable1->setStyleSheet("border-radius:4px;background:rgba(151, 151, 151, 1);");
    pageIndex = PAGE_THREE;
}

void MainWindow::returnMain()
{

    if(page2->cancelbtn->text()==tr("Cancel")){
    QMessageBox::StandardButton result =  QMessageBox::warning(this,tr("Warning"),tr("USB driver is in production.Are you sure you want to stop task and exit the program?"),
                         QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    switch (result){
        case QMessageBox::Yes:{//exit_proc在其他case中也有效，不加花括号exit_proc的生命周期就不会终结，在其他case中就会成为一个没有初始化的变量
            // exit progress and close mainwindow
            QDBusMessage exit_proc = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/","com.lingmousbcreator.interface","MakeExit");
            QDBusConnection::systemBus().call(exit_proc);
            stackedWidget->setCurrentIndex(changePage());
            page1->ifStartBtnChange();
            page1->creatStart->setEnabled(false);
            page2->cancelbtn->setText(tr("Cancel"));
            // page2->round->setPersent(0);
            page2->round1->setValue(0);
            //page2->round->setFormat("0%");
           // page2->round->setValue(0);
        //    page1->findIso->setText(tr("select file"));
            page1->urlIso->clear();
            page1->urlIso->setText(tr("Click or drag to add a mirror file"));
            pointLable1->setBackgroundColor(QColor(45,135,255));
            pointLable2->setBackgroundColor(QColor(151,151,151));
            pointLable3->setBackgroundColor(QColor(151,151,151));
            disconnect(titleClose,&QPushButton::clicked,0,0); //开始制作之后取消之前click触发的应用关闭功能
            connect(titleClose,&QPushButton::clicked,[=](){
               this->close();
            });
            pageIndex = PAGE_ONE;
            break;
        }
        case QMessageBox::No:
            break;
    }
    }else{
//        QDBusMessage exit_proc = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/","com.lingmousbcreator.interface","MakeExit");
//        QDBusConnection::systemBus().call(exit_proc);
        stackedWidget->setCurrentIndex(changePage());
        page1->ifStartBtnChange();
        page1->creatStart->setEnabled(false);
        page2->cancelbtn->setText(tr("Cancel"));
        page2->round1->setValue(0);
        // page2->round->setPersent(0);
        //page2->round->setFormat("0%");
        // page2->round->setValue(0);
       // page1->findIso->setText(tr("select file"));
        page1->urlIso->clear();
        page1->urlIso->setText(tr("Click or drag to add a mirror file"));
        pointLable1->setBackgroundColor(QColor(45,135,255));
        pointLable2->setBackgroundColor(QColor(151,151,151));
        pointLable3->setBackgroundColor(QColor(151,151,151));
        pageIndex = PAGE_ONE;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F1)
    {
        if(!m_DaemonIpcDbus->daemonIsNotRunning()){
           // m_DaemonIpcDbus->showGuide("tools/lingmo-usb-creator");
            m_DaemonIpcDbus->showGuide("lingmo-usb-creator");
        }
    }
}

void MainWindow::dealAuthorityStatus(QString status){
    if("success" == status){
        makeStart();
    }else{
//        授权失败，回到到page1

    }
}
void MainWindow::setThemeDark()
{
    QPixmap pix(":/data/button darkmode inactive.png");
//    page1->tabUdisk1->setPixmap(pix);
    page2->setThemeStyleDark();
    page1->setThemeStyleDark();
}

void MainWindow::setThemeLight()
{
    QPixmap pix(":/data/button lightmode inactive.png");
//    page1->tabUdisk1->setPixmap(pix);
    page1->setThemeStyleLight();
    page2->setThemeStyleLight();
}
void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    page1->dragEnterEvent(event);

    return QWidget::dragEnterEvent(event);
}
void MainWindow::dragLeaveEvent(QDragLeaveEvent *event){
    page1->dragLeaveEvent(event);
    return QWidget::dragLeaveEvent(event);
}
void MainWindow::dropEvent(QDropEvent *event){
   // if(pageIndex == PAGE_ONE){
        if(page1){
            page1->dropEvent(event);
//            QPixmap pix(":/data/Placeholder lighter.png");
//            page1->tabUdisk->setPixmap(pix);
//            page1->urlIso->setText(page1->isoPath);
//            page1->urlIso1->setText(page1->isoPath);
//            page1->tabUdisk->show();
//            page1->topwidget1->hide();
//            page1->topwidget->show();
//            page1->top->hide();
  //      }
    }
    return QWidget::dropEvent(event);
}
//void MainWindow::eventFilter(QObject *watched, QEvent *event){
//    page1->eventFilter(watched,event);
//    return QWidget::eventFilter(watched,event);
//}
