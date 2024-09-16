// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scientificmodule.h"

#include "dthememanager.h"
#include "../utils.h"
#include "../../3rdparty/math/quantity.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtGlobal>

const int EXPRESSIONBAR_HEIGHT = 100;

scientificModule::scientificModule(QWidget *parent)
    : DWidget(parent)
{
    m_stackWidget = new QStackedWidget(this);
    m_sciexpressionBar = new SciExpressionBar(this);
    m_memhiskeypad = new MemHisKeypad(this);
    m_scikeypadwidget = new ScientificKeyPad(this);
    m_memhiswidget = new MemHisWidget(this);
    m_memCalbtn = false;
    m_memRCbtn = false;
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_memoryPublic = MemoryPublic::instance();
    m_sciexpressionBar->setFixedHeight(EXPRESSIONBAR_HEIGHT);
    layout->addWidget(m_sciexpressionBar);
    layout->addWidget(m_memhiskeypad);
    m_stackWidget->addWidget(m_memhiswidget);
    m_stackWidget->addWidget(m_scikeypadwidget);
    m_stackWidget->setCurrentWidget(m_scikeypadwidget);
    layout->addWidget(m_stackWidget);
    layout->setSpacing(0);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    setMouseTracking(true); //默认false，默认下至少一个鼠标按键被按下才跟踪鼠标
    setFocus();

    m_sciexpressionBar->initTheme(DGuiApplicationHelper::instance()->themeType());
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            m_sciexpressionBar, &SciExpressionBar::initTheme);

    connect(m_memhiswidget->findChild<SimpleListView *>(), &SimpleListView::obtainingHistorical, this, &scientificModule::clickListView); //点击切换界面的历史记录
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
            &scientificModule::initTheme);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            m_scikeypadwidget, &ScientificKeyPad::buttonThemeChanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            m_memhiswidget, &MemHisWidget::themeChanged);
    connect(m_sciexpressionBar, &SciExpressionBar::keyPress, this, &scientificModule::handleEditKeyPress);
    connect(m_sciexpressionBar, &SciExpressionBar::clearStateChanged, this,
            &scientificModule::handleClearStateChanged); //判断C/AC
    connect(m_sciexpressionBar, &SciExpressionBar::turnDeg, this, &scientificModule::handleDegChanged); //点击deg按键
    connect(m_sciexpressionBar, &SciExpressionBar::fEStateChanged, this, &scientificModule::handleFEStateChanged);
    connect(this, &scientificModule::changedeg, m_scikeypadwidget, &ScientificKeyPad::getdeg); //切换deg切图
    connect(m_scikeypadwidget, &ScientificKeyPad::buttonPressed, this,
            &scientificModule::handleKeypadButtonPress);
    connect(m_scikeypadwidget, &ScientificKeyPad::buttonPressedbySpace, this, &scientificModule::handleKeypadButtonPressByspace);
    connect(m_memhiskeypad, &MemHisKeypad::buttonPressed, this,
            &scientificModule::handleKeypadButtonPress);
    connect(m_memhiskeypad, &MemHisKeypad::buttonPressedbySpace, this, &scientificModule::handleKeypadButtonPressByspace);
    connect(m_memhiswidget, &MemHisWidget::hisIsFilled, [ = ](bool hisisfilled) {
        if (hisisfilled) {
            MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
            btn2->setEnabled(true);
            m_havail = true;
        } else {
            m_havail = false;
        }
    });
    connect(m_memhiswidget->findChild<MemoryWidget *>(), &MemoryWidget::widgetplus, this, [ = ](int row) {
        //点击键盘按键上的m+,m-是先进行计算，若有计算结果放入内存中
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::widgetplus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second, row);
    });
    connect(m_memhiswidget->findChild<MemoryWidget *>(), &MemoryWidget::widgetminus, this, [ = ](int row) {
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::widgetminus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second, row);
    });
    connect(m_memoryPublic, &MemoryPublic::memorycleanSig, this, &scientificModule::mUnAvailableEvent);
    connect(m_memoryPublic, &MemoryPublic::generateDataSig, this, &scientificModule::mAvailableEvent);
    connect(m_memhiswidget->findChild<MemoryWidget *>(), &MemoryWidget::itemclick, this, [ = ](const QPair<QString, Quantity> p) {
        //内存界面点击item
        QString str = p.first;
        m_sciexpressionBar->getInputEdit()->setAnswer(str.remove("\n"), p.second);
        m_sciexpressionBar->getInputEdit()->setFocus();
        //点击item清除键状态改变
        this->handleClearStateChanged(false);
        m_stackWidget->setCurrentWidget(m_scikeypadwidget);
        m_memhiskeypad->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        m_sciexpressionBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
        btn->setEnabled(true);
        btn->setbuttongray(false);
        MemoryButton *btn1 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
        btn1->setEnabled(true);
        btn1->setbuttongray(false);
        MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mplus));
        btn2->setEnabled(true);
        btn2->setbuttongray(false);
        MemoryButton *btn3 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mmin));
        btn3->setEnabled(true);
        btn3->setbuttongray(false);
        MemoryButton *btn4 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MS));
        btn4->setEnabled(true);
        btn4->setbuttongray(false);
        MemoryButton *btn5 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
        btn5->setEnabled(true);
        btn5->setbtnlight(false);
        TextButton *btn6 = static_cast<TextButton *>(m_memhiskeypad->button(MemHisKeypad::Key_FE));
        btn6->setEnabled(true);
        m_memRCbtn = true;
        m_memCalbtn = true;
        m_isallgray = false;
    });
    connect(m_memhiswidget, &MemHisWidget::hideWidget, this, &scientificModule::hideMemHisWidget);
    connect(m_sciexpressionBar->getInputEdit(), &InputEdit::emptyExpression, this, [ = ](bool b) {
        //输入栏为空m+,m-,ms置灰；否则可用
        if (b == false) {
            MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mplus));
            btn2->setEnabled(true);
            MemoryButton *btn3 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mmin));
            btn3->setEnabled(true);
            MemoryButton *btn4 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MS));
            btn4->setEnabled(true);
            m_memCalbtn = true;
            m_memhiswidget->getMemoryWiget()->expressionempty(b);
        } else {
            MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mplus));
            btn2->setEnabled(false);
            MemoryButton *btn3 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mmin));
            btn3->setEnabled(false);
            MemoryButton *btn4 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MS));
            btn4->setEnabled(false);
            m_memCalbtn = false;
            m_memhiswidget->getMemoryWiget()->expressionempty(b);
        }
    });

    if (!m_memoryPublic->isEmpty())
        mAvailableEvent();
    else
        mUnAvailableEvent();
//    setScientificTabOrder();
}

scientificModule::~scientificModule() {}

void scientificModule::initTheme(int type)
{
    m_sciexpressionBar->initTheme(type);
    update();
}

/**
 * @brief 物理键盘事件
 */
void scientificModule::handleEditKeyPress(QKeyEvent *e)
{
    const bool isPressCtrl = e->modifiers() == Qt::ControlModifier; //ctrl是否按下
    const bool isPressShift = e->modifiers() == Qt::ShiftModifier; //shift是否按下

    switch (e->key()) {
    case Qt::Key_0:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_0); //键盘按下动画效果
        m_sciexpressionBar->enterNumberEvent("0");
        break;
    case Qt::Key_1:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_1);
        m_sciexpressionBar->enterNumberEvent("1");
        break;
    case Qt::Key_2:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_2);
        m_sciexpressionBar->enterNumberEvent("2");
        break;
    case Qt::Key_3:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_3);
        m_sciexpressionBar->enterNumberEvent("3");
        break;
    case Qt::Key_4:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_4);
        m_sciexpressionBar->enterNumberEvent("4");
        break;
    case Qt::Key_5:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_5);
        m_sciexpressionBar->enterNumberEvent("5");
        break;
    case Qt::Key_6:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_6);
        m_sciexpressionBar->enterNumberEvent("6");
        break;
    case Qt::Key_7:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_7);
        m_sciexpressionBar->enterNumberEvent("7");
        break;
    case Qt::Key_8:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_8);
        m_sciexpressionBar->enterNumberEvent("8");
        break;
    case Qt::Key_9:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_9);
        m_sciexpressionBar->enterNumberEvent("9");
        break;
    case Qt::Key_Plus:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Plus);
        m_sciexpressionBar->enterSymbolEvent("+");
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Min);
        m_sciexpressionBar->enterSymbolEvent("-");
        break;
    case Qt::Key_Asterisk:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Mult);
        m_sciexpressionBar->enterSymbolEvent("*");
        break;
    case Qt::Key_Slash:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Div);
        m_sciexpressionBar->enterSymbolEvent("/");
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Equal:
        m_sciexpressionBar->enterEqualEvent();
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Equals);
        if (m_sciexpressionBar->getexpression().first) {
            m_memhiswidget->findChild<SimpleListModel *>()->updataList(m_sciexpressionBar->getanswer(), m_sciexpressionBar->getexpression().second, 0);
            m_memhiswidget->historyfilled();
            m_memhiswidget->findChild<SimpleListView *>()->scrollToTop();
        }
        break;
    case Qt::Key_Backspace:
        m_sciexpressionBar->enterBackspaceEvent();
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Backspace);
        break;
    case Qt::Key_Period:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Point);
        m_sciexpressionBar->enterPointEvent();
        break;
    case Qt::Key_Escape:
        m_sciexpressionBar->enterClearEvent();
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Clear);
        break;
    case Qt::Key_ParenLeft:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_left);
        m_sciexpressionBar->enterBracketEvent(1);
        break;
    case Qt::Key_ParenRight:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_right);
        m_sciexpressionBar->enterBracketEvent(2);
        break;
    case Qt::Key_Percent:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Percent);
        m_sciexpressionBar->enterPercentEvent();
        break;
    case Qt::Key_AsciiCircum:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_xy);
        m_sciexpressionBar->enterOperatorEvent("^");
        break;
    case Qt::Key_C:
        if (isPressCtrl) {
            m_sciexpressionBar->copyResultToClipboard();
            setFocus();
        }
        break;
    case Qt::Key_V:
        if (isPressCtrl) {
            m_sciexpressionBar->copyClipboard2Result();
        } else {
            m_sciexpressionBar->enterFEEvent(m_FEisdown);
//            m_memhiskeypad->animate(MemHisKeypad::Key_FE);
        }
        break;
    case Qt::Key_A:
        if (isPressCtrl) {
            m_sciexpressionBar->allElection();
        }
        break;
    case Qt::Key_X:
        if (isPressCtrl) {
            m_sciexpressionBar->shear();
        } else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_Mult);
            m_sciexpressionBar->enterSymbolEvent("*");
        }
        break;
    case Qt::Key_Delete:
        m_sciexpressionBar->enterClearEvent();
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Clear);
        break;
    case Qt::Key_Left:
        m_sciexpressionBar->moveLeft();
        break;
    case Qt::Key_Right:
        m_sciexpressionBar->moveRight();
        break;
    case Qt::Key_Z:
        if (isPressCtrl)
            m_sciexpressionBar->Undo();
        break;
    case Qt::Key_Y:
        if (isPressCtrl)
            m_sciexpressionBar->Redo();
        else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_xy);
            m_sciexpressionBar->enterOperatorEvent("^");
        }
        break;
    case Qt::Key_L:
        if (isPressCtrl && m_memRCbtn && !m_isallgray) { //CTRL+L,MC
            m_memhiskeypad->animate(MemHisKeypad::Key_MC);
            QTimer::singleShot(100, this, [ = ] {
                m_memhiswidget->memoryFunctions(MemHisWidget::memoryclean); //延迟，让动画效果显示
            });
        } else if (!isPressCtrl) {
            if (isPressShift) {
                m_scikeypadwidget->animate(ScientificKeyPad::Key_logyx);
                m_sciexpressionBar->enterOperatorEvent("log");
            } else {
                m_scikeypadwidget->animate(ScientificKeyPad::Key_log);
                m_sciexpressionBar->enterFunctionEvent("lg");
            }
        }
        break;
    case Qt::Key_R:
        if (isPressCtrl && m_memRCbtn && !m_isallgray) { //CTRL+R,MR
            m_memhiskeypad->animate(MemHisKeypad::Key_MR);
            m_sciexpressionBar->getInputEdit()->setAnswer(m_memhiswidget->findChild<MemoryWidget *>()->getfirstnumber().first
                                                          , m_memhiswidget->findChild<MemoryWidget *>()->getfirstnumber().second);
            //点击item清除键状态改变
            this->handleClearStateChanged(false);
        } else if (!isPressCtrl) {
            if (isPressShift) {
                m_scikeypadwidget->animate(ScientificKeyPad::Key_Rand);
                m_sciexpressionBar->enterRandEvent();
            } else {
                m_scikeypadwidget->animate(ScientificKeyPad::Key_Derivative);
                m_sciexpressionBar->enterSpecialFuncEvent("1/(");
            }
        }
        break;
    case Qt::Key_P:
        if (isPressCtrl && m_memCalbtn && !m_isallgray) { //CTRL+P,M+
            m_sciexpressionBar->enterEqualEvent();
            m_memhiskeypad->animate(MemHisKeypad::Key_Mplus);
            if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first) //如果输入栏中可计算出结果
                m_memhiswidget->memoryFunctions(MemHisWidget::memoryplus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        } else if (!isPressCtrl) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_PI);
            m_sciexpressionBar->enterConstantEvent(QString::fromUtf8("π"));
        }
        break;
    case Qt::Key_Q:
        if (isPressCtrl && m_memCalbtn && !m_isallgray) { //CTRL+Q,M-
            m_sciexpressionBar->enterEqualEvent();
            m_memhiskeypad->animate(MemHisKeypad::Key_Mmin);
            if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first) //如果输入栏中可计算出结果
                m_memhiswidget->memoryFunctions(MemHisWidget::memoryminus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        } else if (!isPressCtrl) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_x2);
            m_sciexpressionBar->enterOperatorEvent("^2");
        }
        break;
    case Qt::Key_M:
        if (isPressCtrl && m_memCalbtn && !m_isallgray) { //ctrl+m,MS
            m_sciexpressionBar->enterEqualEvent();
            m_memhiskeypad->animate(MemHisKeypad::Key_MS);
            if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first) //如果输入栏中可计算出结果
                m_memhiswidget->memoryFunctions(MemHisWidget::generateData, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        } else if (!isPressCtrl) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_Mod);
            m_sciexpressionBar->enterOperatorEvent("mod");
        }
        break;
    case Qt::Key_NumberSign: //#
        m_scikeypadwidget->animate(ScientificKeyPad::Key_x3);
        m_sciexpressionBar->enterOperatorEvent("^3");
        break;
    case Qt::Key_N:
        if (isPressCtrl) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_ex);
            m_sciexpressionBar->enterConstantEvent("e^");
        } else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_ln);
            m_sciexpressionBar->enterFunctionEvent("ln");
        }
        break;
    case Qt::Key_At: //@
        m_scikeypadwidget->animate(ScientificKeyPad::Key_sqrt2);
        m_sciexpressionBar->enterFunctionEvent("sqrt");
        break;
    case Qt::Key_B:
        m_scikeypadwidget->animate(ScientificKeyPad::Key_sqrt3);
        m_sciexpressionBar->enterFunctionEvent("cbrt");
        break;
    case Qt::Key_Bar: //|,shift+'\'
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Modulus);
        m_sciexpressionBar->enterSpecialFuncEvent("abs(");
        break;
    case Qt::Key_E:
        if (isPressShift) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_e);
            m_sciexpressionBar->enterConstantEvent(QString::fromUtf8("e"));
        } else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_exp);
            m_sciexpressionBar->enterOperatorEvent(".E＋");
        }
        break;
    case Qt::Key_S:
        if (isPressShift) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_arcsin);
            m_sciexpressionBar->enterFunctionEvent("arcsin");
        } else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_sin);
            m_sciexpressionBar->enterFunctionEvent("sin");
        }
        break;
    case Qt::Key_O:
        if (isPressShift) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_arccos);
            m_sciexpressionBar->enterFunctionEvent("arccos");
        } else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_cos);
            m_sciexpressionBar->enterFunctionEvent("cos");
        }
        break;
    case Qt::Key_T:
        if (isPressShift) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_arctan);
            m_sciexpressionBar->enterFunctionEvent("arctan");
        } else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_tan);
            m_sciexpressionBar->enterFunctionEvent("tan");
        }
        break;
    case Qt::Key_J:
        if (isPressShift) {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_arccot);
            m_sciexpressionBar->enterFunctionEvent("arccot");
        } else {
            m_scikeypadwidget->animate(ScientificKeyPad::Key_cot);
            m_sciexpressionBar->enterFunctionEvent("cot");
        }
        break;
    case Qt::Key_Exclam: //'!'
        m_scikeypadwidget->animate(ScientificKeyPad::Key_Factorials);
        m_sciexpressionBar->enterOperatorEvent("!");
        break;
    default:
        break;
    }
    if (!m_scikeypadwidget->buttonHasFocus())
        m_sciexpressionBar->getInputEdit()->setFocus(); //edit 20200417 for bug--21146
    int left = 0;
    int right = 0;
    QString text = m_sciexpressionBar->getInputEdit()->text();
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == "(")
            left ++;
        else if (text[i] == ")") {
            if (left > 0)
                left--;
            else
                right++;
        }
    }
    m_scikeypadwidget->bracketsNum(0, QString::number(left)); //写入左右括号不匹配数
    m_scikeypadwidget->bracketsNum(1, QString::number(right));
}

/**
 * @brief 按钮事件
 */
void scientificModule::handleKeypadButtonPress(int key)
{
    bool pagefocus = false;
    m_scikeypadwidget->update();
    //20200414 bug20294鼠标点击取消focus
    switch (key) {
    case ScientificKeyPad::Key_0:
        m_sciexpressionBar->enterNumberEvent("0");
        break;
    case ScientificKeyPad::Key_1:
        m_sciexpressionBar->enterNumberEvent("1");
        break;
    case ScientificKeyPad::Key_2:
        m_sciexpressionBar->enterNumberEvent("2");
        break;
    case ScientificKeyPad::Key_3:
        m_sciexpressionBar->enterNumberEvent("3");
        break;
    case ScientificKeyPad::Key_4:
        m_sciexpressionBar->enterNumberEvent("4");
        break;
    case ScientificKeyPad::Key_5:
        m_sciexpressionBar->enterNumberEvent("5");
        break;
    case ScientificKeyPad::Key_6:
        m_sciexpressionBar->enterNumberEvent("6");
        break;
    case ScientificKeyPad::Key_7:
        m_sciexpressionBar->enterNumberEvent("7");
        break;
    case ScientificKeyPad::Key_8:
        m_sciexpressionBar->enterNumberEvent("8");
        break;
    case ScientificKeyPad::Key_9:
        m_sciexpressionBar->enterNumberEvent("9");
        break;
    case ScientificKeyPad::Key_Plus:
        m_sciexpressionBar->enterSymbolEvent("+");
        break;
    case ScientificKeyPad::Key_Min:
        m_sciexpressionBar->enterSymbolEvent("-");
        break;
    case ScientificKeyPad::Key_Mult:
        m_sciexpressionBar->enterSymbolEvent("*");
        break;
    case ScientificKeyPad::Key_Div:
        m_sciexpressionBar->enterSymbolEvent("/");
        break;
    case ScientificKeyPad::Key_Percent:
        m_sciexpressionBar->enterPercentEvent();
        break;
    case ScientificKeyPad::Key_Equals:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getexpression().first) {
            m_memhiswidget->findChild<SimpleListModel *>()->updataList(m_sciexpressionBar->getanswer(), m_sciexpressionBar->getexpression().second, 0);
            m_memhiswidget->historyfilled();
            m_memhiswidget->findChild<SimpleListView *>()->scrollToTop();
        }
        break;
    case ScientificKeyPad::Key_Clear:
        m_sciexpressionBar->enterClearEvent();
        break;
    case ScientificKeyPad::Key_Backspace:
        m_sciexpressionBar->enterBackspaceEvent();
        break;
    case ScientificKeyPad::Key_Point:
        m_sciexpressionBar->enterPointEvent();
        break;
    case ScientificKeyPad::Key_left:
        m_sciexpressionBar->enterBracketEvent(1);
        break;
    case ScientificKeyPad::Key_right:
        m_sciexpressionBar->enterBracketEvent(2);
        break;
    case MemHisKeypad::Key_MS:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::generateData, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        break;
    case MemHisKeypad::Key_MC:
        m_memhiswidget->memoryFunctions(MemHisWidget::memoryclean);
        break;
    case MemHisKeypad::Key_Mplus:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::memoryplus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        break;
    case MemHisKeypad::Key_Mmin:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::memoryminus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        break;
    case MemHisKeypad::Key_MR:
        m_sciexpressionBar->getInputEdit()->setAnswer(m_memhiswidget->findChild<MemoryWidget *>()->getfirstnumber().first
                                                      , m_memhiswidget->findChild<MemoryWidget *>()->getfirstnumber().second);
        //点击item清除键状态改变
        this->handleClearStateChanged(false);
        break;
    case MemHisKeypad::Key_MHlist:
        showMemHisWidget();
        m_memhiswidget->focusOnButtonbox(Qt::MouseFocusReason);
        m_sciexpressionBar->setAttribute(Qt::WA_TransparentForMouseEvents); //鼠标穿透
        m_memhiskeypad->setAttribute(Qt::WA_TransparentForMouseEvents); //鼠标穿透
        pagefocus = true;
        break;
    case ScientificKeyPad::Key_deg:
        m_sciexpressionBar->enterDegEvent(m_deg);
        break;
    case ScientificKeyPad::Key_sin:
        m_sciexpressionBar->enterFunctionEvent("sin");
        break;
    case MemHisKeypad::Key_FE:
        m_sciexpressionBar->enterFEEvent(m_FEisdown);
        break;
    case ScientificKeyPad::Key_page:
        handlePageStateChanged();
        break;
    case ScientificKeyPad::Key_PI:
        m_sciexpressionBar->enterConstantEvent(QString::fromUtf8("π"));
        break;
    case ScientificKeyPad::Key_e:
        m_sciexpressionBar->enterConstantEvent(QString::fromUtf8("e"));
        break;
    case ScientificKeyPad::Key_Mod:
        m_sciexpressionBar->enterOperatorEvent("mod");
        break;
    case ScientificKeyPad::Key_x2:
        m_sciexpressionBar->enterOperatorEvent("^2");
        break;
    case ScientificKeyPad::Key_Derivative:
        m_sciexpressionBar->enterSpecialFuncEvent("1/(");
        break;
    case ScientificKeyPad::Key_Factorials:
        m_sciexpressionBar->enterOperatorEvent("!");
        break;
    case ScientificKeyPad::Key_exp:
        m_sciexpressionBar->enterOperatorEvent(".E＋");
        break;
    case ScientificKeyPad::Key_cos:
        m_sciexpressionBar->enterFunctionEvent("cos");
        break;
    case ScientificKeyPad::Key_x3:
        m_sciexpressionBar->enterOperatorEvent("^3");
        break;
    case ScientificKeyPad::Key_tan:
        m_sciexpressionBar->enterFunctionEvent("tan");
        break;
    case ScientificKeyPad::Key_xy:
        m_sciexpressionBar->enterOperatorEvent("^");
        break;
    case ScientificKeyPad::Key_cot:
        m_sciexpressionBar->enterFunctionEvent("cot");
        break;
    case ScientificKeyPad::Key_10x:
        m_sciexpressionBar->enterConstantEvent("10^");
        break;
    case ScientificKeyPad::Key_Modulus:
        m_sciexpressionBar->enterSpecialFuncEvent("abs(");
        break;
    case ScientificKeyPad::Key_log:
        m_sciexpressionBar->enterFunctionEvent("lg");
        break;
    case ScientificKeyPad::Key_Rand:
        m_sciexpressionBar->enterRandEvent();
        break;
    case ScientificKeyPad::Key_ln:
        m_sciexpressionBar->enterFunctionEvent("ln");
        break;
    case ScientificKeyPad::Key_arcsin:
        m_sciexpressionBar->enterFunctionEvent("arcsin");
        break;
    case ScientificKeyPad::Key_sqrt2:
        m_sciexpressionBar->enterFunctionEvent("sqrt");
        break;
    case ScientificKeyPad::Key_arccos:
        m_sciexpressionBar->enterFunctionEvent("arccos");
        break;
    case ScientificKeyPad::Key_sqrt3:
        m_sciexpressionBar->enterFunctionEvent("cbrt");
        break;
    case ScientificKeyPad::Key_arctan:
        m_sciexpressionBar->enterFunctionEvent("arctan");
        break;
    case ScientificKeyPad::Key_ysqrtx:
        m_sciexpressionBar->enterOperatorEvent("yroot");
        break;
    case ScientificKeyPad::Key_arccot:
        m_sciexpressionBar->enterFunctionEvent("arccot");
        break;
    case ScientificKeyPad::Key_2x:
        m_sciexpressionBar->enterConstantEvent("2^");
        break;
    case ScientificKeyPad::Key_logyx:
        m_sciexpressionBar->enterOperatorEvent("log");
        break;
    case ScientificKeyPad::Key_ex:
        m_sciexpressionBar->enterConstantEvent("e^");
        break;
    case ScientificKeyPad::Key_opposite:
        m_sciexpressionBar->enterSpecialFuncEvent("(-");
        break;
    default:
        break;
    }
//    if (m_stackWidget->currentWidget() == m_scikeypadwidget)
//        m_sciexpressionBar->getInputEdit()->setFocus();
    if (!pagefocus)
        m_sciexpressionBar->getInputEdit()->setFocus();
    int left = 0;
    int right = 0;
    QString text = m_sciexpressionBar->getInputEdit()->text();
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == "(")
            left ++;
        else if (text[i] == ")") {
            if (left > 0)
                left--;
            else
                right++;
        }
    }
    m_scikeypadwidget->bracketsNum(0, QString::number(left)); //写入左右括号不匹配数
    m_scikeypadwidget->bracketsNum(1, QString::number(right));
}

/**
 * @brief 焦点空格事件
 */
void scientificModule::handleKeypadButtonPressByspace(int key)
{
    m_scikeypadwidget->update();
    m_memhiskeypad->update();
    if (key > 52 && key < 58)
        m_memhiskeypad->animate(MemHisKeypad::Buttons(key), true);
    else if (key < 52)
        m_scikeypadwidget->animate(ScientificKeyPad::Buttons(key), true);
    //20200414 bug20294鼠标点击取消focus
    switch (key) {
    case ScientificKeyPad::Key_0:
        m_sciexpressionBar->enterNumberEvent("0");
        break;
    case ScientificKeyPad::Key_1:
        m_sciexpressionBar->enterNumberEvent("1");
        break;
    case ScientificKeyPad::Key_2:
        m_sciexpressionBar->enterNumberEvent("2");
        break;
    case ScientificKeyPad::Key_3:
        m_sciexpressionBar->enterNumberEvent("3");
        break;
    case ScientificKeyPad::Key_4:
        m_sciexpressionBar->enterNumberEvent("4");
        break;
    case ScientificKeyPad::Key_5:
        m_sciexpressionBar->enterNumberEvent("5");
        break;
    case ScientificKeyPad::Key_6:
        m_sciexpressionBar->enterNumberEvent("6");
        break;
    case ScientificKeyPad::Key_7:
        m_sciexpressionBar->enterNumberEvent("7");
        break;
    case ScientificKeyPad::Key_8:
        m_sciexpressionBar->enterNumberEvent("8");
        break;
    case ScientificKeyPad::Key_9:
        m_sciexpressionBar->enterNumberEvent("9");
        break;
    case ScientificKeyPad::Key_Plus:
        m_sciexpressionBar->enterSymbolEvent("+");
        break;
    case ScientificKeyPad::Key_Min:
        m_sciexpressionBar->enterSymbolEvent("-");
        break;
    case ScientificKeyPad::Key_Mult:
        m_sciexpressionBar->enterSymbolEvent("*");
        break;
    case ScientificKeyPad::Key_Div:
        m_sciexpressionBar->enterSymbolEvent("/");
        break;
    case ScientificKeyPad::Key_Percent:
        m_sciexpressionBar->enterPercentEvent();
        break;
    case ScientificKeyPad::Key_Equals:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getexpression().first) {
            m_memhiswidget->findChild<SimpleListModel *>()->updataList(m_sciexpressionBar->getanswer(), m_sciexpressionBar->getexpression().second, 0);
            m_memhiswidget->historyfilled();
            m_memhiswidget->findChild<SimpleListView *>()->scrollToTop();
        }
        break;
    case ScientificKeyPad::Key_Clear:
        m_sciexpressionBar->enterClearEvent();
        break;
    case ScientificKeyPad::Key_Backspace:
        m_sciexpressionBar->enterBackspaceEvent();
        break;
    case ScientificKeyPad::Key_Point:
        m_sciexpressionBar->enterPointEvent();
        break;
    case ScientificKeyPad::Key_left:
        m_sciexpressionBar->enterBracketEvent(1);
        break;
    case ScientificKeyPad::Key_right:
        m_sciexpressionBar->enterBracketEvent(2);
        break;
    case MemHisKeypad::Key_MS:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::generateData, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        break;
    case MemHisKeypad::Key_MC:
        m_memhiswidget->memoryFunctions(MemHisWidget::memoryclean);
        break;
    case MemHisKeypad::Key_Mplus:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::memoryplus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        break;
    case MemHisKeypad::Key_Mmin:
        m_sciexpressionBar->enterEqualEvent();
        if (m_sciexpressionBar->getInputEdit()->getMemoryAnswer().first)
            m_memhiswidget->memoryFunctions(MemHisWidget::memoryminus, m_sciexpressionBar->getInputEdit()->getMemoryAnswer().second);
        break;
    case MemHisKeypad::Key_MR:
        m_sciexpressionBar->getInputEdit()->setAnswer(m_memhiswidget->findChild<MemoryWidget *>()->getfirstnumber().first
                                                      , m_memhiswidget->findChild<MemoryWidget *>()->getfirstnumber().second);
        //点击item清除键状态改变
        this->handleClearStateChanged(false);
        break;
    case MemHisKeypad::Key_MHlist:
        m_memhiskeypad->animate(MemHisKeypad::Key_MHlist, true);
        showMemHisWidget();
        m_memhiswidget->focusOnButtonbox(Qt::TabFocusReason);
        m_sciexpressionBar->setAttribute(Qt::WA_TransparentForMouseEvents); //鼠标穿透
        m_memhiskeypad->setAttribute(Qt::WA_TransparentForMouseEvents); //鼠标穿透
        break;
    case ScientificKeyPad::Key_deg:
        m_sciexpressionBar->enterDegEvent(m_deg);
        break;
    case ScientificKeyPad::Key_sin:
        m_sciexpressionBar->enterFunctionEvent("sin");
        break;
    case MemHisKeypad::Key_FE:
        m_sciexpressionBar->enterFEEvent(m_FEisdown);
        break;
    case ScientificKeyPad::Key_page:
        handlePageStateChanged();
        break;
    case ScientificKeyPad::Key_PI:
        m_sciexpressionBar->enterConstantEvent(QString::fromUtf8("π"));
        break;
    case ScientificKeyPad::Key_e:
        m_sciexpressionBar->enterConstantEvent(QString::fromUtf8("e"));
        break;
    case ScientificKeyPad::Key_Mod:
        m_sciexpressionBar->enterOperatorEvent("mod");
        break;
    case ScientificKeyPad::Key_x2:
        m_sciexpressionBar->enterOperatorEvent("^2");
        break;
    case ScientificKeyPad::Key_Derivative:
        m_sciexpressionBar->enterSpecialFuncEvent("1/(");
        break;
    case ScientificKeyPad::Key_Factorials:
        m_sciexpressionBar->enterOperatorEvent("!");
        break;
    case ScientificKeyPad::Key_exp:
        m_sciexpressionBar->enterOperatorEvent(".E＋");
        break;
    case ScientificKeyPad::Key_cos:
        m_sciexpressionBar->enterFunctionEvent("cos");
        break;
    case ScientificKeyPad::Key_x3:
        m_sciexpressionBar->enterOperatorEvent("^3");
        break;
    case ScientificKeyPad::Key_tan:
        m_sciexpressionBar->enterFunctionEvent("tan");
        break;
    case ScientificKeyPad::Key_xy:
        m_sciexpressionBar->enterOperatorEvent("^");
        break;
    case ScientificKeyPad::Key_cot:
        m_sciexpressionBar->enterFunctionEvent("cot");
        break;
    case ScientificKeyPad::Key_10x:
        m_sciexpressionBar->enterConstantEvent("10^");
        break;
    case ScientificKeyPad::Key_Modulus:
        m_sciexpressionBar->enterSpecialFuncEvent("abs(");
        break;
    case ScientificKeyPad::Key_log:
        m_sciexpressionBar->enterFunctionEvent("lg");
        break;
    case ScientificKeyPad::Key_Rand:
        m_sciexpressionBar->enterRandEvent();
        break;
    case ScientificKeyPad::Key_ln:
        m_sciexpressionBar->enterFunctionEvent("ln");
        break;
    case ScientificKeyPad::Key_arcsin:
        m_sciexpressionBar->enterFunctionEvent("arcsin");
        break;
    case ScientificKeyPad::Key_sqrt2:
        m_sciexpressionBar->enterFunctionEvent("sqrt");
        break;
    case ScientificKeyPad::Key_arccos:
        m_sciexpressionBar->enterFunctionEvent("arccos");
        break;
    case ScientificKeyPad::Key_sqrt3:
        m_sciexpressionBar->enterFunctionEvent("cbrt");
        break;
    case ScientificKeyPad::Key_arctan:
        m_sciexpressionBar->enterFunctionEvent("arctan");
        break;
    case ScientificKeyPad::Key_ysqrtx:
        m_sciexpressionBar->enterOperatorEvent("yroot");
        break;
    case ScientificKeyPad::Key_arccot:
        m_sciexpressionBar->enterFunctionEvent("arccot");
        break;
    case ScientificKeyPad::Key_2x:
        m_sciexpressionBar->enterConstantEvent("2^");
        break;
    case ScientificKeyPad::Key_logyx:
        m_sciexpressionBar->enterOperatorEvent("log");
        break;
    case ScientificKeyPad::Key_ex:
        m_sciexpressionBar->enterConstantEvent("e^");
        break;
    case ScientificKeyPad::Key_opposite:
        m_sciexpressionBar->enterSpecialFuncEvent("(-");
        break;
    default:
        break;
    }
    int left = 0;
    int right = 0;
    QString text = m_sciexpressionBar->getInputEdit()->text();
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == "(")
            left ++;
        else if (text[i] == ")") {
            if (left > 0)
                left--;
            else
                right++;
        }
    }
    m_scikeypadwidget->bracketsNum(0, QString::number(left)); //写入左右括号不匹配数
    m_scikeypadwidget->bracketsNum(1, QString::number(right));
}

/**
 * @brief 判断C/AC
 */
void scientificModule::handleClearStateChanged(bool isAllClear)
{
    TextButton *btn = static_cast<TextButton *>(m_scikeypadwidget->button(ScientificKeyPad::Key_Clear));

    if (isAllClear) {
        btn->setText("AC");
    } else {
        btn->setText("C");
    }
}

/**
 * @brief 切换deg状态并发送deg切图信号
 */
void scientificModule::handleDegChanged()
{
    emit changedeg(m_deg);
    if (m_deg == 1) {
        m_deg = 2;
    } else if (m_deg == 2) {
        m_deg = 3;
    } else if (m_deg == 3) {
        m_deg = 1;
    }
}

/**
 * @brief FE是否被按下
 */
void scientificModule::handleFEStateChanged(bool isdown)
{
    TextButton *btn = static_cast<TextButton *>(m_memhiskeypad->button(MemHisKeypad::Key_FE));
    if (isdown) {
        m_FEisdown = true;
    } else {
        m_FEisdown = false;
    }
    btn->setButtonDown(m_FEisdown);
}

/**
 * @brief 切换2nd状态
 */
void scientificModule::handlePageStateChanged()
{
    TextButton *btn = static_cast<TextButton *>(m_scikeypadwidget->button(ScientificKeyPad::Key_page));
    if (m_Pageisdown) {
        m_Pageisdown = false;
    } else {
        m_Pageisdown = true;
    }
    btn->setButtonDown(m_Pageisdown);
}

/**
 * @brief m_stackWidget切换至m_memhiswidget
 */
void scientificModule::showMemHisWidget()
{
    m_stackWidget->setCurrentWidget(m_memhiswidget);
    m_memhiswidget->getSimpleListModel()->updataOfSeparate();
    m_memhiswidget->getMemoryWiget()->resetLabelBySeparator();
    if (static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist))->hasFocus()) {
        //当前界面无历史focus到membuttonboxbutton
        if (!m_memhiswidget->findChild<IconButton *>()->isHidden())
            m_memhiswidget->findChild<QStackedWidget *>()->currentWidget()->setFocus();
        else {
            m_memhiswidget->findChild<DButtonBoxButton *>("mButtonBoxButton")->setFocus();
        }
    } else {
        m_memhiswidget->setFocus();
    }
    MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mplus));
    btn2->setbuttongray(true);
    btn2->setEnabled(false);
    MemoryButton *btn3 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mmin));
    btn3->setbuttongray(true);
    btn3->setEnabled(false);
    MemoryButton *btn4 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MS));
    btn4->setbuttongray(true);
    btn4->setEnabled(false);
    MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
    btn->setbuttongray(true);
    btn->setEnabled(false);
    MemoryButton *btn1 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
    btn1->setbuttongray(true);
    btn1->setEnabled(false);
    MemoryButton *btn5 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
    btn5->setbtnlight(true);
    btn5->setEnabled(false);
    TextButton *btn6 = static_cast<TextButton *>(m_memhiskeypad->button(MemHisKeypad::Key_FE));
    btn6->setEnabled(false);
    m_isallgray = true;
}

/**
 * @brief m_stackWidget切换至m_scikeypadwidget
 */
void scientificModule::hideMemHisWidget()
{
    m_stackWidget->setCurrentWidget(m_scikeypadwidget);
    m_isallgray = false;
    m_sciexpressionBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_memhiskeypad->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_sciexpressionBar->getInputEdit()->setFocus();
    MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mplus));
    btn2->setbuttongray(false);
    btn2->setEnabled(true);
    MemoryButton *btn3 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mmin));
    btn3->setbuttongray(false);
    btn3->setEnabled(true);
    MemoryButton *btn4 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MS));
    btn4->setbuttongray(false);
    btn4->setEnabled(true);
    MemoryButton *btn5 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
    btn5->setbtnlight(false);
    btn5->setEnabled(true);
    TextButton *btn6 = static_cast<TextButton *>(m_memhiskeypad->button(MemHisKeypad::Key_FE));
    btn6->setEnabled(true);
    if (m_avail == true) {
        MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
        btn->setbuttongray(false);
        btn->setEnabled(true);
        MemoryButton *btn7 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
        btn7->setbuttongray(false);
        btn7->setEnabled(true);
        m_memRCbtn = true;
    } else {
        MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
        btn->setbuttongray(true);
        btn->setEnabled(false);
        MemoryButton *btn1 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
        btn->setbuttongray(true);
        btn1->setEnabled(false);
        if (m_havail == false) {
            MemoryButton *btn8 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
            btn8->setEnabled(false);
        }
        m_memRCbtn = false;
    }
    m_sciexpressionBar->getInputEdit()->isExpressionEmpty(); //确认输入栏是否有内容，发送信号M+,M-,MS是否置灰
}

/**
 * @brief 点击历史记录
 */
void scientificModule::clickListView(const QModelIndex &index)
{
    QString text = index.data(SimpleListModel::ExpressionWithOutTip).toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList historic = text.split(QString("＝"), QString::SkipEmptyParts);
#else
    QStringList historic = text.split("＝", Qt::SkipEmptyParts);
#endif
    //历史记录中无内容不继续执行
    if (historic.size() != 2)
        return;
    m_sciexpressionBar->hisRevisionResults(index, m_memhiswidget->findChild<SimpleListModel *>()->getAnswer(index.row()));
    hideMemHisWidget();
}

/**
 * @brief 焦点不在scientificmodul时也触发keypress
 */
void scientificModule::setKeyPress(QKeyEvent *e)
{
    handleEditKeyPress(e);
}

/**
 * @brief memoryitem增加时mc,mr可用
 */
void scientificModule::mAvailableEvent()
{
    MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
    btn->setEnabled(true);
    MemoryButton *btn1 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
    btn1->setEnabled(true);
    MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
    btn2->setEnabled(true);
    m_memRCbtn = true;
    m_avail = true;
}

/**
 * @brief mc后mc,mr不可用
 */
void scientificModule::mUnAvailableEvent()
{
    MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
    btn->setEnabled(false);
    MemoryButton *btn1 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
    btn1->setEnabled(false);
    if (m_stackWidget->currentWidget() == m_scikeypadwidget && m_havail == false) {
        MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
        btn2->setEnabled(false);
    }
    m_memRCbtn = false;
    m_avail = false;
}

/**
 * @brief 切到标准模式时在初始化进行判断m+,m-,ms是否可用
 */
void scientificModule::checkLineEmpty()
{
    m_sciexpressionBar->getInputEdit()->isExpressionEmpty();
}

/**
 * @brief 鼠标点击切换stackwidget
 */
void scientificModule::mousePressEvent(QMouseEvent *event)
{
    //内存界面显示时，点击内存界面以外部分切换内存界面为键盘界面
    if (m_stackWidget->currentWidget() == m_memhiswidget && !m_stackWidget->geometry().contains(event->pos())) {
        m_stackWidget->setCurrentWidget(m_scikeypadwidget);
        MemoryButton *btn2 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mplus));
        btn2->setbuttongray(false);
        btn2->setEnabled(true);
        MemoryButton *btn3 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_Mmin));
        btn3->setbuttongray(false);
        btn3->setEnabled(true);
        MemoryButton *btn4 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MS));
        btn4->setbuttongray(false);
        btn4->setEnabled(true);
        MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
        btn->setbuttongray(false);
        btn->setEnabled(true);
        MemoryButton *btn1 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
        btn1->setbuttongray(false);
        btn1->setEnabled(true);
        MemoryButton *btn5 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
        btn5->setbtnlight(false);
        btn5->setEnabled(true);
        TextButton *btn6 = static_cast<TextButton *>(m_memhiskeypad->button(MemHisKeypad::Key_FE));
        btn6->setEnabled(true);
        m_isallgray = false;
        m_sciexpressionBar->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        m_memhiskeypad->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        m_sciexpressionBar->getInputEdit()->setFocus();
    }

    if (m_avail == true) {
        MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
        btn->setEnabled(true);
        MemoryButton *btn4 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
        btn4->setEnabled(true);
        m_memRCbtn = true;
    } else {
        MemoryButton *btn = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MC));
        btn->setEnabled(false);
        MemoryButton *btn1 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MR));
        btn1->setEnabled(false);
        if (m_havail == false) {
            MemoryButton *btn5 = static_cast<MemoryButton *>(m_memhiskeypad->button(MemHisKeypad::Key_MHlist));
            btn5->setEnabled(false);
        }
        m_memRCbtn = false;
    }
    m_sciexpressionBar->getInputEdit()->isExpressionEmpty(); //确认输入栏是否有内容，发送信号M+,M-,MS是否置灰
    QWidget::mousePressEvent(event);
}

/**
 * @brief 防止下拉菜单时鼠标点击拖拽可移动窗口
 */
void scientificModule::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

