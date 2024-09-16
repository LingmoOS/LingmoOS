// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AptConfigMessage.h"
#include "utils/utils.h"

#include <DGuiApplicationHelper>
#include <DRecentManager>
#include <DTitlebar>

#include <QGuiApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>

AptConfigMessage *AptConfigMessage::aptConfig = nullptr;

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

AptConfigMessage::AptConfigMessage(QWidget *parent)
    : DMainWindow(parent)
{
    initControl();         // 初始化控件
    initAccessibleName();  // 自动化测试，为控件添加AccessibleName
    initUI();              // 初始化UI界面
    initTitlebar();        // 初始化标题栏，隐藏标题栏的各种按钮
    initTabOrder();        // 初始化按钮的焦点切换顺序
    connect(m_pushbutton, &QPushButton::clicked, this, &AptConfigMessage::dealInput);  // 按钮按下，处理输入的内容并发送到安装程序
}

/**
 * @brief AptConfigMessage::initTabOrder 初始化tab切换焦点的顺序。
 * 设置焦点在输入框和按钮之间循环
 */
void AptConfigMessage::initTabOrder()
{
    QWidget::setTabOrder(m_pushbutton, m_inputEdit->lineEdit());  // 设置焦点的切换顺序从按钮到输入框
    QWidget::setTabOrder(m_inputEdit->lineEdit(), m_pushbutton);  // 设置焦点的切换顺序从输入框到按钮
}

/**
 * @brief AptConfigMessage::initTitlebar 初始化标题栏
 */
void AptConfigMessage::initTitlebar()
{
    DTitlebar *tb = titlebar();
    if (tb) {
        tb->setIcon(QIcon::fromTheme("deepin-deb-installer"));  // 设置图标
        tb->setTitle("");
        tb->setVisible(false);
        tb->setMenuVisible(false);         // 设置标题栏菜单按钮不可见
        tb->setAutoFillBackground(false);  // 填充标题栏背景
    }
}

/**
 * @brief AptConfigMessage::initControl 初始化各个控件
 */
void AptConfigMessage::initControl()
{
    this->setFocusPolicy(Qt::NoFocus);  // 设置自身无焦点

    // 初始化 配置信息展示框的样式
    m_textEdit = new InstallProcessInfoView(360, 196);
    m_textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_textEdit->setTextFontSize(12, QFont::Medium);
    m_textEdit->setMinimumSize(360, 196);
    m_textEdit->setFocusPolicy(Qt::NoFocus);

    // 初始化输入框
    m_inputEdit = new DLineEdit();
    m_inputEdit->setMinimumSize(220, 36);

    // 设置输入框只接受两个数字，配置的选项在99个以内（1-99）
    // 兼容有些包（mysql-community-server）配置时需要输入密码，取消对输入框的限制
    //     QRegExp regExp("[0-9]{1,2}");
    //     m_inputEdit->setValidator(new QRegExpValidator(regExp, this));

    // 初始化提示信息lable
    m_pQuestionLabel = new DLabel(tr("Enter the number to configure: "));
    m_pQuestionLabel->setMaximumWidth(360);
    m_pQuestionLabel->setFocusPolicy(Qt::NoFocus);

    // 初始化提交信息按钮
    m_pushbutton = new DSuggestButton(tr("OK", "button"));
    m_pushbutton->setDefault(true);
    m_pushbutton->setMinimumSize(130, 36);

    // 焦点在信息输入框时，按回车触发提交信息。
    connect(m_inputEdit, &DLineEdit::returnPressed, m_pushbutton, &DPushButton::click);
}

/**
 * @brief AptConfigMessage::initUI 整体布局的初始化
 *
 */
void AptConfigMessage::initUI()
{
    setFixedSize(380, 332);           // 固定配置框的大小
    setTitlebarShadowEnabled(false);  // 设置标题栏无阴影

    // 建立最大的整体布局
    QVBoxLayout *centralLayout = new QVBoxLayout();
    centralLayout->addStretch(10);               // 设置最小间距为10px
    centralLayout->addWidget(m_textEdit);        // 添加配置信息展示框
    centralLayout->addWidget(m_pQuestionLabel);  // 添加提示信息label
    centralLayout->addStretch(10);               // 添加弹簧最小间距为10px

    // 输入框和按钮小布局
    QHBoxLayout *pInputLayout = new QHBoxLayout();
    pInputLayout->addWidget(m_inputEdit);   // 添加输入框
    m_inputEdit->setFocus();                // 输入框默认启动时带有焦点。
    pInputLayout->addStretch(10);           // 添加间距
    pInputLayout->addWidget(m_pushbutton);  // 添加按钮

    centralLayout->addLayout(pInputLayout);            // 把输入框和按钮的布局放到整体布局中
    centralLayout->addStretch(10);                     // 增加下边距
    centralLayout->setContentsMargins(10, 0, 10, 10);  // 设置整体的上下左右的边距

    // 增加一个widget 放置上述布局，并将此widget设置为中心窗口
    QWidget *wrapWidget = new QWidget();
    wrapWidget->setLayout(centralLayout);
    wrapWidget->setFocusPolicy(Qt::NoFocus);  // 此widget无焦点
    setCentralWidget(wrapWidget);

    setWindowIcon(QIcon::fromTheme("deepin-deb-installer"));                 // 给程序添加图标文件
    move(qApp->primaryScreen()->geometry().center() - geometry().center());  // 移动此窗口到屏幕中间。
}

/**
 * @brief AptConfigMessage::appendTextEdit  向配置信息展示窗口添加配置信息的数据
 * @param processInfo           配置信息数据
 */
void AptConfigMessage::appendTextEdit(QString processInfo)
{
    // 保证焦点在输入框上
    m_inputEdit->lineEdit()->setFocus();
    // 如果添加的数据是空的或者只有换行，则不添加
    qInfo() << processInfo;
    if (processInfo.isEmpty() || processInfo == "\\r\\n")
        return;

    QString configMessage;
    configMessage = processInfo.replace("  ", "     ");

    // 移除多余的“"”
    configMessage.remove(QChar('\"'), Qt::CaseInsensitive);
    configMessage.remove("\\r");
    // 获取配置的第一行的最后一个字符的下标，用于判断当前是否还有信息需要展示
    int num = configMessage.indexOf("\\n");
    // 下标为-1 表明此时只有一行数据需要展示，则直接添加
    if (num == -1) {
        m_textEdit->appendText(processInfo);
        //        m_textEdit->appendText("\n");
        return;
    }
    int messageSize = configMessage.size();
    while (num != -1) {
        num = configMessage.indexOf("\\n");  // 获取第一行的下标

        QString strFilter;                      // 存放第一行的数据
        strFilter = configMessage.mid(0, num);  // 截取第一行

        // 从原始数据中删除第一行
        // num +2 是为了去掉换行，size-num-3是保证 数据的长度不超过原本text的长度
        // size-1 是本身的长度 num+2 是第一行的长度  size-1-(num +2) = size-num-3
        configMessage = configMessage.mid(num + 2, messageSize - num - 3);

        if (strFilter[0] == '\t')
            strFilter.remove(0, 1);         // 如果第一行的第一个数据是tab，去掉
        m_textEdit->appendText(strFilter);  // 添加数据
        qDebug() << "strFilter" << strFilter;

        // 如果当前已经是最后一行。此时text的数据长度大于0且text已经不包含任何的换行则退出，说明信息获取完成。
        if (num == -1 && configMessage.size() > 0 && !configMessage.contains("\n")) {
            break;
        }
    }
}

/**
 * @brief AptConfigMessage::dealInput
 * 向工作线程传递输入的数据
 */
void AptConfigMessage::dealInput()
{
    // 如果当前输入框中的信息是空的 或者输入了00 则不提交，并清除信息
    // PS:dpkg 规定如果输入00 配置会结束
    if (m_inputEdit->text().isEmpty() || m_inputEdit->text() == "" || m_inputEdit->text() == "00") {
        m_inputEdit->clear();  // 每次提交输入信息后，输入框清除。
        return;
    }
    QString str = m_inputEdit->text();            // 获取输入框的输入信息
    str.remove(QChar('"'), Qt::CaseInsensitive);  // 去除输入框中多余的“"”
    emit AptConfigInputStr(str);                  // 提交信息到配置安装程序
    m_inputEdit->clear();                         // 清除输入框的内容
}

/**
 * @brief AptConfigMessage::paintEvent
 * @param event
 *
 * 去除界面上的 最小化 最大化 关闭 菜单 按钮
 */
void AptConfigMessage::paintEvent(QPaintEvent *event)
{
    DMainWindow::paintEvent(event);
    QLayout *layout = titlebar()->layout();
    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget != nullptr && QString(widget->metaObject()->className()) == "QWidget") {
            QLayout *widgetLayout = widget->layout();
            for (int j = 0; j < widgetLayout->count(); ++j) {
                QWidget *topwidget = widgetLayout->itemAt(j)->widget();
                if (topwidget != nullptr && QString(topwidget->metaObject()->className()) == "QWidget") {
                    QLayout *wLayout = topwidget->layout();
                    for (int k = 0; k < wLayout->count(); ++k) {
                        QWidget *bottomWidget = wLayout->itemAt(k)->widget();
                        if (bottomWidget != nullptr && QString(bottomWidget->metaObject()->className()).contains("Button")) {
                            bottomWidget->setFocusPolicy(Qt::NoFocus);  // 设置标题栏所有的控件无焦点
                            bottomWidget->setVisible(false);            // 隐藏标题栏所有的控件。
                        }
                    }
                }
            }
        }
    }
}

void AptConfigMessage::closeEvent(QCloseEvent *event)
{
    // bug121131  右键dock栏软件包安装器，选择“关闭所有”，关闭配置项弹窗
    // bug121123  禁用Alt+f4组合键关闭 配置包窗口
    // Alt+F4按键是由系统驱动实现的，根本无法捕获这个事件
    // Alt+F4 触发关闭事件，则忽略关闭事件即可
    event->ignore();
}

/**
 * @brief AptConfigMessage::clearTexts  清除输入框和信息框的内容
 */
void AptConfigMessage::clearTexts()
{
    m_textEdit->clearText();  // 清除信息框的内容
    m_inputEdit->clear();     // 清除输入框的内容
}

/**
 * @brief AptConfigMessage::initAccessibleName 初始化AccessibleName
 */
void AptConfigMessage::initAccessibleName()
{
    this->setObjectName("AptConfigMessage");
    this->setAccessibleName("AptConfigMessage");

    m_textEdit->setObjectName("InstallInfoEdit");
    m_textEdit->setAccessibleName("InstallInfoEdit");

    m_inputEdit->setObjectName("ConfigInputEdit");
    m_inputEdit->setAccessibleName("ConfigInputEdit");

    m_pushbutton->setObjectName("ConfigConfirmButton");
    m_pushbutton->setAccessibleName("ConfigConfirmButton");
}

AptConfigMessage::~AptConfigMessage()
{
    delete m_textEdit;
    delete m_inputEdit;
    delete m_pushbutton;
    delete m_pQuestionLabel;
    delete aptConfig;
}
