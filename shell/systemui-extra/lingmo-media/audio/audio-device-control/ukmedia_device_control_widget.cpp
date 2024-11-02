/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ukmedia_device_control_widget.h"

QList<pa_device_port_info> devsInfo;
QList<pa_device_port_info> devsInputInfo;

/*
 *  Class:          UkmediaDevControlWidget
 *  description:    输出设备的子类
 */
//主窗口布局
UkmediaDevControlWidget::UkmediaDevControlWidget(QWidget *parent): KWidget(parent)
{
    this->setIcon(QIcon::fromTheme("lingmo-control-center"));
    this->setWidgetName(tr("Sound Equipment Control"));
    this->setWindowFlags(Qt::Dialog);
    this->setContentsMargins(0,0,0,0);

    initDevControlWidget();
}

void UkmediaDevControlWidget::initDevControlWidget()
{
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    m_pOutputAndInputBar = new KNavigationBar(this);

    QStandardItem * outputItem = new QStandardItem(QIcon::fromTheme("audio-volume-overamplified-symbolic"),tr("Output Devices"));
    QStandardItem * inputItem = new QStandardItem(QIcon::fromTheme("lingmo-microphone-on-symbolic"),tr("Input Devices"));

    // 设置项的样式
    QString itemStyle = "QStandardItem { min-width: 17px; max-width: 17px; min-height: 36px; max-height: 36px; }";
    outputItem->setData(itemStyle, Qt::UserRole);

#define ITEM_FLAG 2
    outputItem->setData(OutputItem, Qt::UserRole+ITEM_FLAG);
    inputItem->setData(InputItem, Qt::UserRole+ITEM_FLAG);

    QList<QStandardItem *> list2;
    list2<<outputItem<<inputItem;

    m_pOutputAndInputBar->addItem(outputItem);
    m_pOutputAndInputBar->addItem(inputItem);

    m_pOutputAndInputBar->setContentsMargins(0,0,0,0);
    m_pOutputAndInputBar->setFixedSize(188, 520);

    vLayout->addWidget(m_pOutputAndInputBar);
    sideBar()->setLayout(vLayout);

    //管理显示界面
    //声明一个输出设备显示窗口
    outputDevWidget = new OutputDevWidget;
    inputDevWidget  = new InputDevWidget;

    // 创建QStackedWidget用于管理不同的界面
    QStackedWidget* stackedWidget = new QStackedWidget();
    stackedWidget->setContentsMargins(0,0,0,0);
    stackedWidget->setFixedSize(560, 518);
    stackedWidget->addWidget(outputDevWidget);
    stackedWidget->addWidget(inputDevWidget);

    QStandardItemModel *m_mode = m_pOutputAndInputBar->model();
    m_pOutputAndInputBar->listview()->setCurrentIndex(m_mode->item(0,0)->index());

    //添加右侧设备窗口布局
    QVBoxLayout* vLayout2 = new QVBoxLayout;
    vLayout2->addWidget(stackedWidget);
    baseBar()->setLayout(vLayout2);

    //获取被点击的item的数据，跳转到对应的页面
    connect(m_pOutputAndInputBar->listview(),&QListView::clicked,this,[=](const QModelIndex &index){
        int flag = index.data(Qt::UserRole+ITEM_FLAG).toInt();
        if (flag == OutputItem) {
            stackedWidget->setCurrentWidget(outputDevWidget);
        }
        else if(flag == InputItem){
            stackedWidget->setCurrentWidget(inputDevWidget);
        }
    });
    this->setLayoutType(HorizontalType);

    //设备初始化操作
    outputDevWidget->updateOutputDev();
    inputDevWidget->updateInputDev();
}

UkmediaDevControlWidget::~UkmediaDevControlWidget()
{
}

/*
 *  Class:          OutputDevWidget
 *  description:    输出设备界面的显示、设备罗列、设备禁用与开启功能等功能实现
 */

//输出设备窗口
OutputDevWidget::OutputDevWidget(QWidget *parent)
{
    initOutputDevUi();
}

void OutputDevWidget::initOutputDevUi()
{
    m_pOutputDevWidget = new QFrame(this);

    m_pOutputDevWidget->setFrameShape(QFrame::Shape::Box);
    m_pOutputDevWidget->setContentsMargins(0, 0, 0, 0);
    //设置大小
    m_pOutputDevWidget->setFixedSize(560,490);

    //标签：输出设备
    m_pOutputDevLabel = new TitleLabel(m_pOutputDevWidget);
    m_pOutputDevLabel->setText(tr("Output Devices"));
    m_pOutputDevLabel->setFixedSize(480,24);
    m_pOutputDevLabel->setStyleSheet("QLabel{color: palette(windowText);}");

    // 创建水平布局并添加标签和占位符小部件
    QHBoxLayout* labelLayout = new QHBoxLayout();
    labelLayout->addWidget(m_pOutputDevLabel);
    labelLayout->addStretch(); // 添加占位符小部件
    // 设置水平布局的外边距
    QMargins labelMargins(16, 0, 0, 0); // 设置左边外边距为 16px
    labelLayout->setContentsMargins(labelMargins);

    //设备窗口布局
    //所有输出设备widget
    outputDevArea = new QScrollArea(this);
    outputDevArea->setFixedSize(512,410);
    outputDevArea->setFrameShape(QFrame::NoFrame);//bjc去掉outputDevArea的边框
    outputDevArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outputDevArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);//滚动条背景透明

    displayOutputDevWidget = new QFrame(outputDevArea);
    displayOutputDevWidget->setFixedWidth(512);
//    displayOutputDevWidget->setAttribute(Qt::WA_TranslucentBackground);
    outputDevArea->setWidget(displayOutputDevWidget);

    m_pSubDevVlayout = new QVBoxLayout(displayOutputDevWidget);
    displayOutputDevWidget->setLayout(m_pSubDevVlayout);

    //设备窗口添加左右两边的间隙
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0,0,0,0);
    hlayout->addItem(new QSpacerItem(10,410,QSizePolicy::Fixed));
    hlayout->addWidget(outputDevArea);
    hlayout->addItem(new QSpacerItem(23,410,QSizePolicy::Fixed));
    hlayout->setSpacing(0);

    // 创建确定按钮
    m_pOutputConfirmBtn = new QPushButton(tr("Confirm"), this);
    m_pOutputConfirmBtn->setFixedSize(96, 36);
    m_pOutputConfirmBtn->setProperty("isImportant", true);

    // 创建水平布局并添加确定按钮和占位符小部件
    QHBoxLayout* m_pHLayout = new QHBoxLayout();
    m_pHLayout->addStretch(); // 添加占位符小部件
    m_pHLayout->addWidget(m_pOutputConfirmBtn);
    m_pHLayout->addSpacing(24); // 添加右边框间距

    //进行整体布局
    QBoxLayout *m_pVlayout = new QVBoxLayout(m_pOutputDevWidget);
    m_pVlayout->setContentsMargins(0,0,0,0);
    m_pVlayout->addLayout(labelLayout);
//    m_pVlayout->addWidget(m_pOutputDevLabel);
    m_pVlayout->addItem(new QSpacerItem(16,8,QSizePolicy::Fixed));
    m_pVlayout->addLayout(hlayout);
    // 将确定按钮添加到布局中
    m_pVlayout->addLayout(m_pHLayout);
    m_pVlayout->addSpacing(30); // 添加下边框间距

    m_pOutputDevWidget->setLayout(m_pVlayout);
    m_pOutputDevWidget->layout()->setContentsMargins(0,0,0,0);

    //确定按钮，关闭窗口
    connect(m_pOutputConfirmBtn, &QPushButton::clicked, this, &OutputDevWidget::closeWindow);
    //监听dbus 移除输出设备信号
    QDBusConnection::sessionBus().connect(QString(), "/org/lingmo/media", "org.lingmo.media","updateDevSignal",this,SLOT(updateOutputDev()));
\
}

void OutputDevWidget::closeWindow()
{
    QWidget* currentWindow = window(); // 获取当前窗口
    if (currentWindow)
        currentWindow->close(); // 关闭窗口
}

//移除子设备窗口
void OutputDevWidget::updateOutputDev()
{
    //将设备窗口中的子窗口删除
    qDeleteAll(subDevWidgets);
    subDevWidgets.clear();

    // 清空列表
    devsInfo.clear();

    //将设备窗口中的子窗重新刷新一遍
    initOutputDevWidget();
}

//初始化子设备窗口
void OutputDevWidget::initOutputDevWidget()
{
    //获取设备函数
    getPaDevInfo();

    int outputCount = 0;
    int inputCount  = 0;

    for (const pa_device_port_info &info : devsInfo)
    {
        if(info.direction == 1)
            outputCount++;
        else if(info.direction == 2)
            inputCount++;
    }

    for (const pa_device_port_info &info : devsInfo)
    {
        if(info.direction == 1)
        {
            qDebug() << "card:" << info.card
                     << "name:" << info.name
                     << "inOrOut" << info.direction
                     << "dsc:" << info.description
                     << "ava:" << info.available
                     << "plugged_stauts" << info.plugged_stauts
                     << "device_description:" << info.device_description
                     << "device_product_name:" << info.device_product_name;

            QString devLabel = info.description + " (" + info.device_description + ")";

            //添加子设备到输出设备窗口
            addSubOutputDevWidget(info.card, info.name, devLabel, info.available, outputCount);
        }
    }
}

//添加子设备窗口
void OutputDevWidget::addSubOutputDevWidget(int card, QString portName, QString DeviceName, int Available, int itemNum)
{

    //声明一个dev item类，写入声卡ID、设备名、label
    subDevWidget = new SubDevWidget();

    //widget显示应用音量
    QWidget *subDevItemwidget = new QWidget(displayOutputDevWidget);
    subDevItemwidget->setFixedSize(512,60);
    subDevItemwidget->setObjectName("subDevItemwidget");

    subDevWidget->m_pDeviceNameLabel = new FixLabel(DeviceName, subDevItemwidget);
    subDevWidget->m_pDevDisableButton = new KSwitchButton();
    subDevWidget->m_pDevDisableButton->setObjectName("DevDisableButton");

    // 设置字符串属性
    subDevWidget->setProperty("card", card);
    subDevWidget->setProperty("DeviceName", DeviceName);
    subDevWidget->m_pDevDisableButton->setProperty("card", card);
    subDevWidget->m_pDevDisableButton->setProperty("portName", portName);
    subDevWidget->m_pDevDisableButton->setProperty("DeviceName", DeviceName);

    //判断设备的可用状态来初始化 button 的选中状态
    if(Available == 3){
        subDevWidget->m_pDevDisableButton->setChecked(false);
    }else{
        subDevWidget->m_pDevDisableButton->setChecked(true);
    }

    QHBoxLayout *outputDevControlLayout = new QHBoxLayout(subDevItemwidget);
    outputDevControlLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    outputDevControlLayout->addWidget(subDevWidget->m_pDeviceNameLabel);
    outputDevControlLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    outputDevControlLayout->addWidget(subDevWidget->m_pDevDisableButton);
    outputDevControlLayout->addItem(new QSpacerItem(16, 60, QSizePolicy::Fixed));
    outputDevControlLayout->setSpacing(0);
    subDevItemwidget->setLayout(outputDevControlLayout);
    subDevItemwidget->layout()->setContentsMargins(0,0,0,0);

    //设置设备子窗口样式
    QString themeName = LINGMO_THEME_DEFAULT;
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings *nThemeSetting = new QGSettings("org.lingmo.style");
        if (nThemeSetting->keys().contains("styleName")) {
            themeName = nThemeSetting->get("style-name").toString();
        }
    }
    // 设置自定义属性
    subDevWidget->m_pDevDisableButton->setProperty("customBackground", true);

    if (themeName == LINGMO_THEME_DEFAULT || themeName == LINGMO_THEME_LIGHT)
        subDevItemwidget->setStyleSheet("#subDevItemwidget{border-radius: 6px; background-color: #F5F5F5;}");
    else if (themeName == LINGMO_THEME_DARK)
        subDevItemwidget->setStyleSheet("#subDevItemwidget{border-radius: 6px; background-color: #333333;}");

    m_pSubDevVlayout->addWidget(subDevItemwidget);
    displayOutputDevWidget->resize(512, itemNum*60);
    m_pSubDevVlayout->setSpacing(2);
    m_pSubDevVlayout->setContentsMargins(0,0,0,0);
    displayOutputDevWidget->setLayout(m_pSubDevVlayout);
    m_pSubDevVlayout->update();

    subDevItemwidget->setProperty("label",DeviceName);
    subDevWidgets.append(subDevItemwidget);

    // 连接按钮的信号与槽
    connectSubDevButton(subDevWidget->m_pDevDisableButton);
}

//将每个子设备窗口的button进行设备关联
void OutputDevWidget::connectSubDevButton(KSwitchButton *button)
{
    connect(button, &KSwitchButton::stateChanged, [=](bool state) {

        int card = button->property("card").toInt();
        QString portName = button->property("portName").toString();
        qDebug() << "connectSubDevButton  button" << state << card << portName;

#ifdef PA_PORT_AVAILABLE_DISABLE
        int outputdevstate = state? PA_PORT_AVAILABLE_YES : PA_PORT_AVAILABLE_DISABLE;
#else
        int outputdevstate = PA_PORT_AVAILABLE_YES;
#endif
        setDevDisable(card, portName, outputdevstate);

    });
}

//通过DBUS传输具体设备给pulseaudio来禁用或开启设备
void OutputDevWidget::setDevDisable(int cardId, QString portName, int devOutputState)
{
    // 创建DBus连接
    QDBusConnection connection = QDBusConnection::sessionBus();
    // 创建DBus接口
    QDBusInterface interface("org.PulseAudio.DeviceControl", "/org/pulseaudio/device_control","org.PulseAudio.DeviceControl", connection);

    qDebug() << " setDevDisable()  " << cardId << portName << devOutputState;
    // 调用DBus方法
    QDBusReply<QString> response = interface.call("SetDeviceStatus", cardId, portName, devOutputState);

    qDebug() << response;
}

//通过DBUS从pulseaudio中获取设备信息
void OutputDevWidget::getPaDevInfo()
{
    // 注册自定义结构体类型
    qRegisterMetaType<pa_device_port_info>("pa_device_port_info");
    qDBusRegisterMetaType<pa_device_port_info>();
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qDebug() << "Cannot connect to D-Bus.";
        return ;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.PulseAudio.DeviceControl",
                                                          "/org/pulseaudio/device_control",
                                                          "org.PulseAudio.DeviceControl",
                                                          "GetAllDeviceInfo");
    QDBusMessage reply = bus.call(message);

    const QDBusArgument &dbusArgs = reply.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        pa_device_port_info info;
        dbusArgs.beginStructure();
        dbusArgs >> info.card;
        dbusArgs >> info.direction;
        dbusArgs >> info.available;
        dbusArgs >> info.plugged_stauts;
        dbusArgs >> info.name;
        dbusArgs >> info.description;
        dbusArgs >> info.device_description;
//        dbusArgs >> info.device_product_name;
        //过滤掉不可用端口
        if(info.available == PA_PORT_AVAILABLE_NO || info.plugged_stauts == PORT_STAUTS_UNPLUGGED)
        {
            dbusArgs.endStructure();
            continue;
        }
        qDebug() << " getPaDevInfo()" << info.available;
//        dbusArgs >> info;
        devsInfo.push_back(info);
        dbusArgs.endStructure();
    }
    dbusArgs.endArray();
}

QFrame* OutputDevWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

OutputDevWidget::~OutputDevWidget()
{
}

/*
 *  Class:          InputDevWidget
 *  description:    输入设备界面的显示、设备罗列、设备禁用与开启功能等功能实现
 */

//输出设备窗口
InputDevWidget::InputDevWidget(QWidget *parent)
{
    initInputDevUi();
}

void InputDevWidget::initInputDevUi()
{
    m_pInputDevWidget = new QFrame(this);

    m_pInputDevWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputDevWidget->setContentsMargins(0, 0, 0, 0);
    //设置大小
    m_pInputDevWidget->setFixedSize(560, 490);

    //标签：输出设备
    m_pInputDevLabel = new TitleLabel(m_pInputDevWidget);
    m_pInputDevLabel->setText(tr("Input Devices"));
    m_pInputDevLabel->setFixedSize(480, 24);
    m_pInputDevLabel->setStyleSheet("QLabel{color: palette(windowText);}");

    // 创建水平布局并添加标签和占位符小部件
    QHBoxLayout* labelLayout = new QHBoxLayout();
    labelLayout->addWidget(m_pInputDevLabel);
    labelLayout->addStretch(); // 添加占位符小部件
    // 设置水平布局的外边距
    QMargins labelMargins(16, 0, 0, 0); // 设置左边外边距为 16px
    labelLayout->setContentsMargins(labelMargins);

    //设备窗口布局
    //所有输出设备widget
    inputDevArea = new QScrollArea(this);
    inputDevArea->setFixedSize(512, 410);
    inputDevArea->setFrameShape(QFrame::NoFrame);//bjc去掉inputDevArea的边框
    inputDevArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    inputDevArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);//滚动条背景透明

    displayInputDevWidget = new QFrame(inputDevArea);
    displayInputDevWidget->setFixedWidth(512);
//    displayInputDevWidget->setAttribute(Qt::WA_TranslucentBackground);
    inputDevArea->setWidget(displayInputDevWidget);

    m_pSubDevVlayout = new QVBoxLayout(displayInputDevWidget);
    displayInputDevWidget->setLayout(m_pSubDevVlayout);

    //设备窗口添加左右两边的间隙
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0,0,0,0);
    hlayout->addItem(new QSpacerItem(10, 410, QSizePolicy::Fixed));
    hlayout->addWidget(inputDevArea);
    hlayout->addItem(new QSpacerItem(23, 410, QSizePolicy::Fixed));
    hlayout->setSpacing(0);

    // 创建确定按钮
    m_pInputConfirmBtn = new QPushButton(tr("Confirm"), this);
    m_pInputConfirmBtn->setFixedSize(96, 36);
    m_pInputConfirmBtn->setProperty("isImportant", true);

    // 创建水平布局并添加确定按钮和占位符小部件
    QHBoxLayout* m_pHLayout = new QHBoxLayout();
    m_pHLayout->addStretch(); // 添加占位符小部件
    m_pHLayout->addWidget(m_pInputConfirmBtn);
    m_pHLayout->addSpacing(24); // 添加右边框间距

    //进行整体布局
    QBoxLayout *m_pVlayout = new QVBoxLayout(m_pInputDevWidget);
    m_pVlayout->setContentsMargins(0,0,0,0);
    m_pVlayout->addLayout(labelLayout);
//    m_pVlayout->addWidget(m_pInputDevLabel);
    m_pVlayout->addItem(new QSpacerItem(16, 8, QSizePolicy::Fixed));
    m_pVlayout->addLayout(hlayout);
    // 将确定按钮添加到布局中
    m_pVlayout->addLayout(m_pHLayout);
    m_pVlayout->addSpacing(30); // 添加下边框间距

    m_pInputDevWidget->setLayout(m_pVlayout);
    m_pInputDevWidget->layout()->setContentsMargins(0,0,0,0);

    //确定按钮，关闭窗口
    connect(m_pInputConfirmBtn, &QPushButton::clicked, this, &InputDevWidget::closeWindow);
    //监听dbus 移除输出设备信号
    QDBusConnection::sessionBus().connect(QString(), "/org/lingmo/media", "org.lingmo.media", "updateDevSignal", this, SLOT(updateInputDev()));
}

void InputDevWidget::closeWindow()
{
    QWidget* currentWindow = window(); // 获取当前窗口
    if (currentWindow)
        currentWindow->close(); // 关闭窗口
}

//移除子设备窗口
void InputDevWidget::updateInputDev()
{
    //将设备窗口中的子窗口删除
    qDeleteAll(subDevWidgets);
    subDevWidgets.clear();

    // 清空列表
    devsInputInfo.clear();

    //将设备窗口中的子窗重新刷新一遍
    initInputDevWidget();
}

//初始化子设备窗口
void InputDevWidget::initInputDevWidget()
{
    //获取设备函数
    getPaDevInfo();

    int inputCount = 0;

    for (const pa_device_port_info &info : devsInputInfo)
    {
        if(info.direction == 2)
            inputCount++;
    }

    for (const pa_device_port_info &info : devsInputInfo)
    {
        if(info.direction == 2)
        {
            qDebug() << "card:" << info.card
                     << "name:" << info.name
                     << "inOrOut" << info.direction
                     << "dsc:" << info.description
                     << "ava:" << info.available
                     << "plugged_stauts" << info.plugged_stauts
                     << "device_description:" << info.device_description
                     << "device_product_name:" << info.device_product_name;

            QString devLabel = info.description + " (" + info.device_description + ")";

            //添加子设备到输出设备窗口
            addSubInputDevWidget(info.card,info.name,devLabel,info.available,inputCount);
        }
    }
}

//添加子设备窗口
void InputDevWidget::addSubInputDevWidget(int card, QString portName, QString DeviceName, int Available, int itemNum)
{
    //声明一个dev item类，写入声卡ID、设备名、label
    subDevWidget = new SubDevWidget();

    //widget显示应用音量
    QWidget *subDevItemwidget = new QWidget(displayInputDevWidget);
    subDevItemwidget->setFixedSize(512, 60);
    subDevItemwidget->setObjectName("subDevItemwidget");

    subDevWidget->m_pDeviceNameLabel = new FixLabel(DeviceName, subDevItemwidget);
    subDevWidget->m_pDevDisableButton = new KSwitchButton();
    subDevWidget->m_pDevDisableButton->setObjectName("DevDisableButton");

    // 设置字符串属性
    subDevWidget->setProperty("card", card);
    subDevWidget->setProperty("DeviceName", DeviceName);
    subDevWidget->m_pDevDisableButton->setProperty("card", card);
    subDevWidget->m_pDevDisableButton->setProperty("portName", portName);
    subDevWidget->m_pDevDisableButton->setProperty("DeviceName", DeviceName);

    //判断设备的可用状态来初始化 button 的选中状态
    if(Available == 3){
        subDevWidget->m_pDevDisableButton->setChecked(false);
    }else{
        subDevWidget->m_pDevDisableButton->setChecked(true);
    }

    QHBoxLayout *inputDevControlLayout = new QHBoxLayout(subDevItemwidget);
    inputDevControlLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    inputDevControlLayout->addWidget(subDevWidget->m_pDeviceNameLabel);
    inputDevControlLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Expanding));
    inputDevControlLayout->addWidget(subDevWidget->m_pDevDisableButton);
    inputDevControlLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    inputDevControlLayout->setSpacing(0);
    subDevItemwidget->setLayout(inputDevControlLayout);
    subDevItemwidget->layout()->setContentsMargins(0,0,0,0);

    //设置设备子窗体颜色
    QString themeName = LINGMO_THEME_DEFAULT;
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings *nThemeSetting = new QGSettings("org.lingmo.style");
        if (nThemeSetting->keys().contains("styleName")) {
            themeName = nThemeSetting->get("style-name").toString();
        }
    }
    if (themeName == LINGMO_THEME_DEFAULT || themeName == LINGMO_THEME_LIGHT)
        subDevItemwidget->setStyleSheet("#subDevItemwidget{border-radius: 6px; background-color: #F5F5F5;}");
    else if (themeName == LINGMO_THEME_DARK)
        subDevItemwidget->setStyleSheet("#subDevItemwidget{border-radius: 6px; background-color: #333333;}");

    //设置窗体圆角
    subDevItemwidget->setStyleSheet("QWidget { border-radius: 60px; }");

    m_pSubDevVlayout->addWidget(subDevItemwidget);
    displayInputDevWidget->resize(512, itemNum*60);
    m_pSubDevVlayout->setSpacing(2);
    m_pSubDevVlayout->setContentsMargins(0,0,0,0);
    displayInputDevWidget->setLayout(m_pSubDevVlayout);
    m_pSubDevVlayout->update();

    subDevItemwidget->setProperty("label",DeviceName);
    subDevWidgets.append(subDevItemwidget);

    // 连接按钮的信号与槽
    connectSubDevButton(subDevWidget->m_pDevDisableButton);
}

//将每个子设备窗口的button进行设备关联
void InputDevWidget::connectSubDevButton(KSwitchButton *button)
{
    connect(button, &KSwitchButton::stateChanged, [=](bool state) {

        int card = button->property("card").toInt();
        QString portName = button->property("portName").toString();
        qDebug() << "connectSubDevButton  button" << state << card << portName;

#ifdef PA_PORT_AVAILABLE_DISABLE
        int inputDevState = state? PA_PORT_AVAILABLE_YES : PA_PORT_AVAILABLE_DISABLE;
#else
        int inputDevState = PA_PORT_AVAILABLE_YES;
#endif
        setDevDisable(card, portName, inputDevState);

    });
}

//通过DBUS传输具体设备给pulseaudio来禁用或开启设备
void InputDevWidget::setDevDisable(int cardId, QString portName, int inputDevState)
{
    // 创建DBus连接
    QDBusConnection connection = QDBusConnection::sessionBus();
    // 创建DBus接口
    QDBusInterface interface("org.PulseAudio.DeviceControl", "/org/pulseaudio/device_control", "org.PulseAudio.DeviceControl", connection);

    qDebug() << " setDevDisable  " << cardId << portName << inputDevState;
    // 调用DBus方法
    QDBusReply<QString> response = interface.call("SetDeviceStatus", cardId, portName, inputDevState);

    qDebug() << response;
}

//通过DBUS从pulseaudio中获取设备信息
void InputDevWidget::getPaDevInfo()
{
    // 注册自定义结构体类型
    qRegisterMetaType<pa_device_port_info>("pa_device_port_info");
    qDBusRegisterMetaType<pa_device_port_info>();
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qDebug() << "Cannot connect to D-Bus.";
        return ;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.PulseAudio.DeviceControl",
                                                          "/org/pulseaudio/device_control",
                                                          "org.PulseAudio.DeviceControl",
                                                          "GetAllDeviceInfo");
    QDBusMessage reply = bus.call(message);

    const QDBusArgument &dbusArgs = reply.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>();
//    QList<pa_device_port_info> devsInputInfo;

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        pa_device_port_info info;
        dbusArgs.beginStructure();
        dbusArgs >> info.card;
        dbusArgs >> info.direction;
        dbusArgs >> info.available;
        dbusArgs >> info.plugged_stauts;
        dbusArgs >> info.name;
        dbusArgs >> info.description;
        dbusArgs >> info.device_description;
        dbusArgs >> info.device_product_name;
        //过滤掉不可用端口
        if(info.available == PA_PORT_AVAILABLE_NO || info.plugged_stauts == PORT_STAUTS_UNPLUGGED)
        {
            dbusArgs.endStructure();
            continue;
        }
//        dbusArgs >> info;
        devsInputInfo.push_back(info);
        dbusArgs.endStructure();
    }
    dbusArgs.endArray();
}

QFrame* InputDevWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

InputDevWidget::~InputDevWidget()
{
}


/*
 *  Class:          SubDevWidget
 *  description:    单个设备的子类，其中会记录声卡ID、端口名、label名
 */
SubDevWidget::SubDevWidget(QWidget *parent)
{

}

void SubDevWidget::setLabelText(QString deviceLabel)
{
    this->m_pDeviceNameLabel->setText(deviceLabel);
}

SubDevWidget::~SubDevWidget()
{

}
