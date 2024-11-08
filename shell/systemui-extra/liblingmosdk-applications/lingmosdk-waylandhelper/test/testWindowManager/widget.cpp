/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "widget.h"
#include <unistd.h>
#include <QWindow>
#include <QBoxLayout>
#include <QDebug>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QTimer>
#include <kwindowsystem.h>
using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent),
      m_subWidget(nullptr)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(2000);
    setWindowTitle("windowtitle");
    connect(WindowManager::self(),&WindowManager::outputInfoChanged,this,[=](OutputInfo* info){
            qDebug() << "output info changed!name:"<<info->outputName() <<" usablearea:"<<info->usableArea();
    });

    connect(WindowManager::self(),&WindowManager::iconChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "icon changed";
    });
    connect(WindowManager::self(),&WindowManager::maximizedChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "maximizedChanged";
    });
    connect(WindowManager::self(),&WindowManager::minimizedChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "minimizedChanged";
    });
    connect(WindowManager::self(),&WindowManager::fullscreenChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "fullscreenChanged";
    });
    connect(WindowManager::self(),&WindowManager::keepAboveChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "keepAboveChanged";
    });
    connect(WindowManager::self(),&WindowManager::skipSwitcherChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "skipSwitcherChanged";
    });
    connect(WindowManager::self(),&WindowManager::skipTaskbarChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "skipTaskbarChanged";
    });
    connect(WindowManager::self(),&WindowManager::activeChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "activeChanged";
    });
    connect(WindowManager::self(),&WindowManager::titleChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "titleChanged";
    });
    connect(WindowManager::self(),&WindowManager::geometryChanged,this,[=](const WindowId& windowId){
//        if(m_lastestWindowId == windowId)
//            qDebug() << "geometryChanged";
    });
    connect(WindowManager::self(),&WindowManager::demandsAttentionChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "demandsAttentionChanged";
    });
    connect(WindowManager::self(),&WindowManager::onAllDesktopsChanged,this,[=](const WindowId& windowId){
        if(m_lastestWindowId == windowId)
            qDebug() << "jnihbj,bhmjbhgbhmvgv";
    });
    connect(WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId){

        /*注意：
         * 最新创建的窗体被设置为操作窗体，此demo中每个按钮对应一个接口函数调用，所有接口函数操作的窗口都是该最新创建的窗体
         */
//    qDebug()<<"窗口添加:"<<WindowManager::getWindowTitle(windowId)<<" "<<windowId;
//    qDebug()<<"窗口添加:icon:"<<WindowManager::getWindowIcon(windowId);
    if((quint32)getpid() == WindowManager::getPid(windowId))
    {
        m_lastestWindowId = windowId;
    }

//        qDebug()<<WindowManager::getWindowIcon(windowId);
//        qDebug()<<WindowManager::getPid(windowId);

    });
    connect(WindowManager::self(),&WindowManager::windowRemoved,this,[=](const WindowId& windowId){
        qDebug()<<"窗口移除:"<<windowId;
    });

    connect(WindowManager::self(),&WindowManager::windowChanged,this,[=](const WindowId& windowId){
        //该信号会被频繁触发，测试时可根据实际需求 选择是否添加打印
        if(m_lastestWindowId == windowId)
            qDebug()<<"窗口属性改变:"<<windowId;
    });
    connect(WindowManager::self(),&WindowManager::activeWindowChanged,this,[=](const WindowId& windowId){
       qDebug()<<"活动窗口改变:"<<windowId;
    });
    connect(WindowManager::self(),&WindowManager::currentDesktopChanged,this,[=](){
       qDebug()<<"当前桌面改变:"<<WindowManager::currentDesktop();
       qDebug()<<"被测窗体是否在当前桌面："<<WindowManager::isOnCurrentDesktop(m_lastestWindowId);
    });
    initUI();
    initConnection();
}

Widget::~Widget()
{

}

void Widget::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* hLayout1 = new QHBoxLayout();
    m_label = new QLabel(this);
    m_label->setText("注意：最新创建的窗体被设置为操作窗体，此demo中每个按钮对应一个接口函数调用，所有接口函数操作的窗口都是该最新创建的窗体");
    m_btnMinimize = new QPushButton("最小化窗口",this);
    m_btnMaximum = new QPushButton("最大化窗口",this);
    m_btnActivate = new QPushButton("激活窗口",this);
    m_btnStayOnTop = new QPushButton("置顶窗口",this);
    m_btnClose = new QPushButton("关闭窗口",this);
    m_btnShowDesktop = new QPushButton("显示桌面",this);
    m_shwWindowTitle = new QPushButton("获取窗口标题",this);
    m_shwWindowIcon = new QPushButton("获取窗口图标",this);
    m_btnShowGroup = new QPushButton("获取窗口所属组",this);
    m_btnprintList = new QPushButton("获取窗口列表",this);
    m_btnSetGeometry = new QPushButton("设置窗口大小",this);
    hLayout1->addWidget(m_btnMinimize);
    hLayout1->addWidget(m_btnMaximum);
    hLayout1->addWidget(m_btnActivate);
    hLayout1->addWidget(m_btnStayOnTop);
    hLayout1->addWidget(m_btnClose);


    QLabel *label = new QLabel(this);
    label->setText("依次输入起点x,起点y,宽度w,高度h");
    m_posX = new QSpinBox(this);
    m_posY = new QSpinBox(this);
    m_width = new QSpinBox(this);
    m_height = new QSpinBox(this);

    m_posX->setRange(0,2000);
    m_posY->setRange(0,2000);
    m_width->setRange(1,2000);
    m_height->setRange(1,2000);
    m_posX->setValue(0);
    m_posY->setValue(0);
    m_width->setValue(800);
    m_height->setValue(600);
    QHBoxLayout *hLayout5 = new QHBoxLayout;
    hLayout5->addWidget(m_posX);
    hLayout5->addWidget(m_posY);
    hLayout5->addWidget(m_width);
    hLayout5->addWidget(m_height);

    QHBoxLayout* hLayout2 = new QHBoxLayout();
    m_shwWdgbtn = new QPushButton("打开测试窗口",this);
    hLayout2->addWidget(m_shwWdgbtn);
    hLayout2->addWidget(m_btnSetGeometry);
    hLayout2->addWidget(new QLabel("设置窗口类型",this));
    m_setTypeBox = new QComboBox(this);
    QStringList l;
    l<<"Widget"<<"Window"<<"Dialog"<<"Sheet"<<"Drawer"<<"Popup"<<"Tool"<<"ToolTip"<<
       "SplashScreen"<<"Desktop"<<"SubWindow"<<"ForeignWindow"<<"CoverWindow";
    m_setTypeBox->addItems(l);
    hLayout2->addWidget(m_setTypeBox);

    QHBoxLayout* hLayout4 = new QHBoxLayout();
    hLayout4->addWidget(m_btnShowDesktop);
    hLayout4->addWidget(m_shwWindowTitle);
    hLayout4->addWidget(m_shwWindowIcon);
    hLayout4->addWidget(m_btnShowGroup);
    hLayout4->addWidget(m_btnprintList);

    QHBoxLayout* hLayout3 = new QHBoxLayout();
    m_btnChangeIcon = new QPushButton("更改图标",this);
    m_btnChangeTitle = new QPushButton("更改标题",this);
    m_btnGetType = new QPushButton("获取窗口类型",this);

    hLayout3->addWidget(m_btnChangeIcon);
    hLayout3->addWidget(m_btnChangeTitle);
    hLayout3->addWidget(m_btnGetType);

    QGroupBox *box2 = new QGroupBox("2.0需求接口",this);
    QHBoxLayout *hLayout6 = new QHBoxLayout();
    m_skipTaskbarBtn = new QPushButton("跳过任务栏",this);
    m_skipSwitcherBtn = new QPushButton("跳过窗口选择器",this);
    hLayout6->addWidget(m_skipTaskbarBtn);
    hLayout6->addWidget(m_skipSwitcherBtn);

    QHBoxLayout *hLayout7 = new QHBoxLayout();
    m_showOnAllDesktop = new QPushButton("在所有桌面中显示",this);
    m_isDesktopShowing = new QPushButton("获取桌面是否处于显示状态",this);

    hLayout7->addWidget(m_showOnAllDesktop);
    hLayout7->addWidget(m_isDesktopShowing);

    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->addLayout(hLayout6);
    vlayout->addLayout(hLayout7);
    box2->setLayout(vlayout);

    QGroupBox *box3 = new QGroupBox("2.1需求接口",this);
    QVBoxLayout *vLayout = new QVBoxLayout();
    QHBoxLayout *hLayout8 = new QHBoxLayout();
    m_isOnCurrentDesktop = new QPushButton("被测窗体是否在当前桌面",this);
    hLayout8->addStretch();
    hLayout8->addWidget(m_isOnCurrentDesktop);
    vLayout->addLayout(hLayout8);
    box3->setLayout(vLayout);

    QGroupBox *box4 = new QGroupBox("2.3需求接口",this);
    m_btnPanelWidget = new QPushButton("显示panel窗口",this);
    m_chbPanel = new QCheckBox(this);
    m_chbPanel->setText("设置panel窗体获取焦点");
    QVBoxLayout *vLayout2 = new QVBoxLayout();
    QHBoxLayout *hLayout9 = new QHBoxLayout();
    hLayout9->addWidget(m_chbPanel);
    hLayout9->addWidget(m_btnPanelWidget);
    vLayout2->addLayout(hLayout9);
    box4->setLayout(vLayout2);


    m_demandBtn = new QPushButton("demandAttention",this);
    m_fullscreenBtn = new QPushButton("全屏",this);
    m_processName = new QPushButton("打印进程名",this);
    hLayout9->addWidget(m_demandBtn);
    hLayout9->addWidget(m_fullscreenBtn);
    hLayout9->addWidget(m_processName);
    vLayout2->addLayout(hLayout9);
    box4->setLayout(vLayout2);

    QGroupBox *box5 = new QGroupBox("2.5需求接口",this);
    m_getGeometryBtn = new QPushButton("getGeometry",this);
    m_setGrabKeyboardBtn = new QPushButton("setGrabKeyboard" ,this);
    m_setSwitcherLayer = new QPushButton("setSwitcherLayer" ,this);
    m_setHighlightBtn = new QPushButton("setHighlight", this);
    m_unsetHighlightBtn = new QPushButton("unsetHighlight", this);
    m_setIconNameBtn = new QPushButton("setIconName",this);
    m_unsetIconNameBtn = new QPushButton("unsetIconName",this);

    QHBoxLayout* layout10 = new QHBoxLayout();
    layout10->addWidget(m_getGeometryBtn);
    layout10->addWidget(m_setGrabKeyboardBtn);
    layout10->addWidget(m_setSwitcherLayer);
    layout10->addWidget(m_setHighlightBtn);
    layout10->addWidget(m_setIconNameBtn);
    layout10->addWidget(m_unsetIconNameBtn);

    QVBoxLayout* vLayout3 = new QVBoxLayout();
    vLayout3->addLayout(layout10);

    m_openUnderCursorBtn = new QPushButton("openUnderCursor",this);

    m_openUnderCursorWithOffsetBtn = new QPushButton("openUnderCursorWithOffset",this);
    m_xOffset = new QSpinBox(this);
    m_xOffset->setRange(-1920*2,1920*2);
    m_xOffset->setValue(10);
    m_yOffset = new QSpinBox(this);
    m_yOffset->setValue(10);
    m_yOffset->setRange(-1080*2,1080*2);
    QHBoxLayout* layout11 = new QHBoxLayout();
    layout11->addWidget(m_unsetHighlightBtn);
    layout11->addWidget(m_openUnderCursorBtn);
    layout11->addWidget(m_openUnderCursorWithOffsetBtn);
    layout11->addWidget(m_xOffset);
    layout11->addWidget(m_yOffset);
    layout11->addWidget(m_setIconNameBtn);
    layout11->addWidget(m_unsetIconNameBtn);
    layout11->addStretch();
    vLayout3->addLayout(layout11);

    QHBoxLayout* layout12 = new QHBoxLayout();
    m_getCurrentOutputBtn = new QPushButton(tr("GetCurrentOutput"), this);
    m_getOutputsInfoBtn = new QPushButton(tr("GetOutputsInfo"), this);
    layout12->addWidget(m_getCurrentOutputBtn);
    layout12->addWidget(m_getOutputsInfoBtn);
    layout12->addStretch();
    vLayout3->addLayout(layout12);

    QHBoxLayout* layout13 = new QHBoxLayout();
    m_getWindowIdByTitleBtn = new QPushButton("Search",this);
    m_getWindowIdByTitleEdit = new QLineEdit(this);
    m_getWindowIdByTitleEdit->setPlaceholderText("get window id by title");
    m_getWindowIdByPidBtn = new QPushButton("Search",this);
    m_getWindowIdByPidEdit = new QLineEdit(this);
    m_getWindowIdByPidEdit->setPlaceholderText("get window id by pid");
    layout13->addWidget(m_getWindowIdByTitleEdit);
    layout13->addWidget(m_getWindowIdByTitleBtn);
    layout13->addWidget(m_getWindowIdByPidEdit);
    layout13->addWidget(m_getWindowIdByPidBtn);
    vLayout3->addLayout(layout13);

    box5->setLayout(vLayout3);


    mainLayout->addWidget(m_label);
    mainLayout->addLayout(hLayout1);
    mainLayout->addWidget(label);
    mainLayout->addLayout(hLayout5);
    mainLayout->addLayout(hLayout2);
    mainLayout->addLayout(hLayout3);
    mainLayout->addLayout(hLayout4);
    mainLayout->addWidget(box2);
    mainLayout->addWidget(box3);
    mainLayout->addWidget(box4);
    mainLayout->addWidget(box5);
    m_subWidget = new QWidget();
    m_subWidget->setWindowIcon(QIcon::fromTheme("lingmo-music"));
    m_subWidget->setWindowTitle("widget");

    m_panelWidget = new QWidget();
    WindowManager::setWindowLayer(m_panelWidget->windowHandle(), WindowLayer::Panel);
}

void Widget::initConnection()
{   
    connect(m_getWindowIdByPidBtn, &QPushButton::clicked,this,[=](){
        qDebug()<<QString("windows id of %1:").arg(m_getWindowIdByPidEdit->text())
               <<WindowManager::getWindowIdByPid(m_getWindowIdByPidEdit->text().toInt());
    });

    connect(m_getWindowIdByTitleBtn, &QPushButton::clicked,this,[=](){
        qDebug()<<QString("windows id of %1:").arg(m_getWindowIdByTitleEdit->text())
               <<WindowManager::getWindowIdByTtile(m_getWindowIdByTitleEdit->text());
    });

    connect(m_getCurrentOutputBtn,&::QPushButton::clicked, this, [=](){
        qDebug() << "current output name:" << WindowManager::currentOutputName();
        qDebug() << "current seat name:" << WindowManager::currentSeatName();
    });

    connect(m_getOutputsInfoBtn,&::QPushButton::clicked, this, [=](){
        QList<OutputInfo*> infos = WindowManager::outputsInfo();
        for(auto info : infos)
        {
            qDebug() << "output name:" << info->outputName();
            qDebug() << "output usable area:" << info->usableArea();;
        }
    });
    connect(m_getOutputsInfoBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setIconName(m_subWidget->windowHandle(), "lingmo-music");
    });
    connect(m_setIconNameBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setIconName(m_subWidget->windowHandle(), "lingmo-music");
    });

    connect(m_unsetIconNameBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setIconName(m_subWidget->windowHandle(), "");
    });

    connect(m_openUnderCursorWithOffsetBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setOpenUnderCursor(m_subWidget->windowHandle(), m_xOffset->value(), m_yOffset->value());
    });


    connect(m_openUnderCursorBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setOpenUnderCursor(m_subWidget->windowHandle());
    });

    connect(m_setHighlightBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setHighlight(m_lastestWindowId, true);
    });

    connect(m_unsetHighlightBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setHighlight(m_lastestWindowId, false);
    });

    connect(m_setSwitcherLayer,&::QPushButton::clicked, this, [=](){
        WindowManager::setWindowLayer(m_subWidget->windowHandle(), WindowLayer::Switcher);
    });

    connect(m_setGrabKeyboardBtn,&::QPushButton::clicked, this, [=](){
        WindowManager::setGrabKeyboard(m_subWidget->windowHandle(),true);
    });

    connect(m_getGeometryBtn,&QPushButton::clicked,this,[=](){
        qDebug()<<WindowManager::windowGeometry(m_lastestWindowId);
    });
    connect(m_btnPanelWidget,&QPushButton::clicked,this,[=](){
        if(m_panelWidget->isVisible())
        {   m_panelWidget->hide();
            m_btnPanelWidget->setText("显示panel窗口");
        }
        else
        {
            m_panelWidget->show();
            m_btnPanelWidget->setText("隐藏panel窗口");
        }
        WindowManager::setWindowLayer(m_panelWidget->windowHandle(), WindowLayer::Panel);
        WindowManager::setPanelTakefocus(m_panelWidget->windowHandle(), m_chbPanel->isChecked());
    });

    connect(m_setTypeBox,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,[=](int index)
    {
        switch (index) {
        case 0:
            m_subWidget->setWindowFlags(Qt::Widget);
            break;
        case 1:
            m_subWidget->setWindowFlags(Qt::Window);
            break;
        case 2:
        m_subWidget->setWindowFlags(Qt::Dialog);
            break;
        case 3:
            m_subWidget->setWindowFlags(Qt::Sheet);
            break;
        case 4:
            m_subWidget->setWindowFlags(Qt::Drawer);
            break;
        case 5:
            m_subWidget->setWindowFlags(Qt::Popup);
            break;
        case 6:
            m_subWidget->setWindowFlags(Qt::Tool);
            break;
        case 7:
            m_subWidget->setWindowFlags(Qt::ToolTip);
            break;
        case 8:
            m_subWidget->setWindowFlags(Qt::SplashScreen);
            break;
        case 9:
            m_subWidget->setWindowFlags(Qt::Desktop);
            break;
        case 10:
            m_subWidget->setWindowFlags(Qt::SubWindow);
            break;
        case 11:
            m_subWidget->setWindowFlags(Qt::ForeignWindow);
            break;
        case 12:
            m_subWidget->setWindowFlags(Qt::CoverWindow);
            break;
        default:
            break;
        }
    });
    connect(WindowManager::self(),&WindowManager::activeWindowChanged,this,[=](const WindowId&wid){
//       qDebug()<<"活动窗口改变:"<<wid;
//       qDebug()<<"当前活动窗口为:"<<WindowManager::currentActiveWindow();
    });

    connect(WindowManager::self(),&WindowManager::isShowingDesktopChanged,this,[=](){
       qDebug()<<"桌面显示状态改变，桌面处于显示状态："<<WindowManager::isShowingDesktop();
    });
    connect(m_btnMinimize,&QPushButton::clicked,this,[=](){
           WindowManager::minimizeWindow(m_lastestWindowId);
    });

    connect(m_btnMaximum,&QPushButton::clicked,this,[=](){
           WindowManager::maximizeWindow(m_lastestWindowId);
    });

    connect(m_btnActivate,&QPushButton::clicked,this,[=](){
           WindowManager::activateWindow(m_lastestWindowId);
    });

    connect(m_btnStayOnTop,&QPushButton::clicked,this,[=](){
            WindowManager::keepWindowAbove(m_lastestWindowId);
    });

    connect(m_btnClose,&QPushButton::clicked,this,[=](){
           WindowManager::closeWindow(m_lastestWindowId);
    });

    connect(m_btnShowDesktop,&QPushButton::clicked,this,[=](){
            WindowManager::showDesktop();
            m_timer->start();
            connect(m_timer,&QTimer::timeout,this,[=](){
                WindowManager::hideDesktop();
            });
    });

    connect(m_btnShowGroup,&QPushButton::clicked,this,[=](){
            qDebug()<<"窗口所属组:"<<WindowManager::getWindowGroup(m_lastestWindowId);
    });

    connect(m_shwWdgbtn,&QPushButton::clicked,this,[=](){
            m_subWidget->show();
    });

    connect(m_btnChangeTitle,&QPushButton::clicked,this,[=](){
        if(m_subWidget)
        {
            if(m_subWidget->windowTitle()!= QString("originTitle"))
                m_subWidget->setWindowTitle("originTitle");
            else
                m_subWidget->setWindowTitle("changedTitle");
        }
    });

    connect(m_btnChangeIcon,&QPushButton::clicked,this,[=](){
        if(m_subWidget)
        {
            if(!m_subWidget->windowIcon().name().contains("lingmo-music"))
                m_subWidget->setWindowIcon(QIcon::fromTheme("lingmo-music"));
            else
                m_subWidget->setWindowIcon(QIcon::fromTheme("lingmo-network"));
        }
    });

    connect(m_shwWindowTitle,&QPushButton::clicked,this,[=](){
        if(m_subWidget)
            qDebug() << "窗口标题为："<< WindowManager::getWindowTitle(m_lastestWindowId);
    });
    connect(m_shwWindowIcon,&QPushButton::clicked,this,[=](){
        if(m_subWidget)
        {
            qDebug() <<"窗口图标为："<<WindowManager::getWindowIcon(m_lastestWindowId);
        }

    });
    connect(m_btnprintList,&QPushButton::clicked,this,[=](){
        QList<WindowId> lists = WindowManager::windows();
        qDebug()<<"打印窗口列表：";
        for(auto id : lists)
        {
            qDebug()<<id<<"title:"<<WindowManager::getWindowTitle(id)
                   <<"pid:"<<WindowManager::getPid(id);
        }
    });
    connect(m_btnSetGeometry,&QPushButton::clicked,this,[=](){
        m_subWidget->show();
        QRect rect(m_posX->value(),m_posY->value(),m_width->value(),m_height->value());
        WindowManager::setGeometry(m_subWidget->windowHandle(),rect);
    });
    connect(m_btnGetType,&QPushButton::clicked,this,[=](){
        /*
        *Unknown  = -1,
        *Normal   = 0
        *Desktop  = 1,
        *Dock     = 2,
        *Toolbar  = 3,
        *Menu     = 4,
        *Dialog   = 5,
        *Override = 6, // NON STANDARD
        *TopMenu  = 7, // NON STANDARD
        *Utility  = 8,
        *Splash   = 9,
        *DropdownMenu = 10,
        *PopupMenu = 11,
        *Tooltip = 12,
        *Notification = 13,
        *ComboBox = 14,
        *DNDIcon = 15,
        *OnScreenDisplay = 16,
        *CriticalNotification = 17
        */
       switch (WindowManager::getWindowType(m_subWidget->winId()))
       {
       case NET::WindowType::Unknown:
           qDebug()<<"Unknown";
           break;
       case NET::WindowType::Normal:
           qDebug()<<"normal";
           break;
       case NET::WindowType::Desktop:
           qDebug()<<"Desktop";
           break;
       case NET::WindowType::Dock:
           qDebug()<<"Dock";
           break;
       case NET::WindowType::Toolbar:
           qDebug()<<"Toolbar";
           break;
       case NET::WindowType::Menu:
           qDebug()<<"Menu";
           break;
       case NET::WindowType::Dialog:
           qDebug()<<"Dialog";
           break;
       case NET::WindowType::Override:
           qDebug()<<"Override";
           break;
       case NET::WindowType::TopMenu:
           qDebug()<<"TopMenu";
           break;
       case NET::WindowType::Splash:
           qDebug()<<"Splash";
           break;
       case NET::WindowType::Utility:
           qDebug()<<"Utility";
           break;
       case NET::WindowType::DropdownMenu:
           qDebug()<<"DropdownMenu";
           break;
       case NET::WindowType::PopupMenu:
           qDebug()<<"PopupMenu";
           break;
       case NET::WindowType::Tooltip:
           qDebug()<<"Tooltip";
           break;
       case NET::WindowType::Notification:
           qDebug()<<"Notification";
           break;
       case NET::WindowType::ComboBox:
           qDebug()<<"ComboBox";
           break;
       case NET::WindowType::DNDIcon:
           qDebug()<<"DNDIcon";
           break;
       case NET::WindowType::OnScreenDisplay:
           qDebug()<<"OnScreenDisplay";
           break;
       case NET::WindowType::CriticalNotification:
           qDebug()<<"CriticalNotification";
           break;
       default:
           qDebug()<<WindowManager::getWindowType(m_subWidget->winId());
           break;
       } ;
    });

    connect(m_skipTaskbarBtn,&QPushButton::clicked,this,[=](){;
        WindowManager::setSkipTaskBar(m_subWidget->windowHandle(),true);
        QTimer::singleShot(1000,this,[=](){
            qDebug()<<"跳过任务栏："<<WindowManager::skipTaskBar(m_lastestWindowId);
        });

    });
    connect(m_skipSwitcherBtn,&QPushButton::clicked,this,[=](){
        WindowManager::setSkipSwitcher(m_subWidget->windowHandle(),true);
        QTimer::singleShot(1000,this,[=](){
            qDebug()<<"跳过窗口选择器"<<WindowManager::skipSwitcher(m_lastestWindowId);
        });
    });
    connect(m_showOnAllDesktop,&QPushButton::clicked,this,[=](){
        WindowManager::setOnAllDesktops(m_lastestWindowId);
        QTimer::singleShot(1000,this,[=](){
        qDebug()<<"设置窗口在所有桌面中显示:"<<WindowManager::isOnAllDesktops(m_lastestWindowId);
        });
    });
    connect(m_isDesktopShowing,&QPushButton::clicked,this,[=](){
        qDebug()<<"桌面处于显示状态："<<WindowManager::isShowingDesktop();
    });
    connect(m_isOnCurrentDesktop,&QPushButton::clicked,this,[=](){
        qDebug()<<m_lastestWindowId;
        qDebug()<<"被测窗体是否在当前桌面："<<WindowManager::isOnCurrentDesktop(m_lastestWindowId);
    });

    connect(m_demandBtn,&QPushButton::clicked,this,[=](){
        QTimer::singleShot(2000,this,[=](){
            WindowManager::demandAttention(m_lastestWindowId);
        });

    });
    connect(m_fullscreenBtn,&QPushButton::clicked,this,[=](){
        if(!m_subWidget)
            return;
        if(m_subWidget->isFullScreen())
        {
            m_subWidget->resize(640,480);
            m_subWidget->showNormal();
        }
        else
            m_subWidget->showFullScreen();
    });
    connect(m_processName,&QPushButton::clicked,this,[=](){
        if(!m_subWidget)
            return;
        qDebug()<<WindowManager::getProcessName(m_lastestWindowId);
    });
}
