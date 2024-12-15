// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sciexpressionbar.h"

#include "../utils.h"
#include "../globaldefine.h"
#include "../../3rdparty/core/settings.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QTimer>
#include <DGuiApplicationHelper>
#include <QRandomGenerator>
#include <QtGlobal>

const int SCIPREC = 31; //科学计算器精度
const int LIST_HEIGHT = 35; //输入栏上方表达式的高度
const int INPUTEDIT_HEIGHT = 55;

SciExpressionBar::SciExpressionBar(QWidget *parent)
    : DWidget(parent)
{
    m_listView = new SimpleListView(0, this);
    m_listDelegate = new SimpleListDelegate(0, this);
    m_listModel = new SimpleListModel(0, this);
    m_inputEdit = new InputEdit(this);
    m_evaluator = Evaluator::instance();
    m_isContinue = true;
    m_isAllClear = false;
    m_isResult = false;
    m_inputNumber = false;
    m_isUndo = false;
    m_meanexp = true;
    m_listView->setModel(m_listModel);
    m_listView->setItemDelegate(m_listDelegate);
    // init inputEdit attributes.
    m_listView->setFixedHeight(LIST_HEIGHT);
    m_inputEdit->setFixedHeight(INPUTEDIT_HEIGHT);
    m_inputEdit->setAlignment(Qt::AlignRight);
    m_inputEdit->setTextMargins(10, 0, 10, 6);

    DPalette pl1 = this->palette();
    pl1.setColor(DPalette::Button, Qt::transparent);
    pl1.setColor(DPalette::Highlight, Qt::transparent);
    pl1.setColor(DPalette::HighlightedText, Qt::blue);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(m_listView);
    layout->addWidget(m_inputEdit);
    layout->setMargin(0);
    layout->setSpacing(0);

    initConnect();

    Settings::instance()->angleUnit = 'd';
    m_funclist = {"arcsin", "arccos", "arctan", "arccot", "sin", "cos", "tan", "cot"
                  , "abs", "lg", "ln", "log", "mod", "sqrt", "cbrt", "yroot", "pi", "π", "e"
                 };
}

SciExpressionBar::~SciExpressionBar() {}

void SciExpressionBar::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

QPair<bool, QString> SciExpressionBar::getexpression()
{
    m_pair.second = m_expression;
    return m_pair;
}

Quantity SciExpressionBar::getanswer()
{
    if (m_pair.first == true)
        return m_listanswer;
    return Quantity();
}

void SciExpressionBar::enterNumberEvent(const QString &text)
{
    if (!judgeinput())
        return;
    if (m_inputNumber && m_isResult == true) {
        m_inputEdit->clear();
        m_isResult = false;
    }
    if (m_isResult) {
        m_inputEdit->clear();
        m_isResult = false;
    }
    if (!m_isContinue) {
        // the cursor position at the end, it will clear edit text.

        if (cursorPosAtEnd())
            m_inputEdit->clear();

        m_isContinue = true;
    }

    m_inputNumber = false;
    m_isUndo = false;

    // 20200401 修改symbolFaultTolerance执行位置
    replaceSelection(m_inputEdit->text());
    /* add 20200722
     * 当输入数字的前面是e和pi时，在前面补乘号防止直接出现表达式错误
     */
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    QString sRegNum1 = "[πe]";
    QRegExp rx1;
    rx1.setPattern(sRegNum1);
    if (curpos > 0 && rx1.exactMatch(exp.at(curpos - 1))) {
        m_inputEdit->insert("×" + text);
    } else
        m_inputEdit->insert(text);
//    m_inputEdit->insert(text);
    int nowcur = m_inputEdit->cursorPosition();
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    m_inputEdit->setText(pointFaultTolerance(m_inputEdit->text()));
    m_inputEdit->setCursorPosition(nowcur);
    emit clearStateChanged(false);
    addUndo();
}

void SciExpressionBar::enterSymbolEvent(const QString &text)
{
    if (!judgeinput())
        return;
    QString oldText = m_inputEdit->text();
    QString symbol = text;
    symbol.replace(EN_DIV, CN_DIV);
    m_isResult = false;
    m_isUndo = false;
    // 20200213统一被选中光标复位代码
    replaceSelection(m_inputEdit->text());
    if (m_inputEdit->text().isEmpty()) {
        if (symbol != "-") {
            m_inputEdit->setText(oldText);
        } else {
            m_inputEdit->insert(symbol);
        }
    } else {
        int curPos = m_inputEdit->cursorPosition();
        QString exp = m_inputEdit->text();
        if (cursorPosAtEnd()) {
            if (m_inputEdit->text() == "－") {
                m_inputEdit->setText(oldText);
            } else {
                QString lastStr = exp.right(1);
                if (isOperator(lastStr))
                    exp.chop(1);
                m_inputEdit->setText(exp + symbol);
            }
        } else if (curPos == 0) {
            QString firstStr = exp.left(1);
            if (firstStr == CN_MIN) {
                m_inputEdit->setText(oldText);
            } else {
                if (symbol == CN_MIN || symbol == EN_MIN)
                    m_inputEdit->insert(symbol);
            }
        } else {
            QString infront = exp.at(curPos - 1);
            QString behand = exp.at(curPos);
            if (isOperator(infront) || isOperator(behand)) {
                m_inputEdit->setText(oldText);
            } else
                m_inputEdit->insert(symbol);

            // 2020316修复添加符号后光标问题
            //添加符号后左侧数字不会多分隔符，只需考虑添加符号后输入框光标前的数字与添加前是否一致
            if (exp.mid(0, curPos).remove(QRegExp("[＋－×÷/,.%()E]")) ==
                    m_inputEdit->text().mid(0, curPos).remove(QRegExp("[＋－×÷/,.%()E]"))) {
                QString sRegNum = "[＋－×÷/]";
                QRegExp rx;
                rx.setPattern(sRegNum);
                rx.exactMatch(m_inputEdit->text().at(curPos))
                ? m_inputEdit->setCursorPosition(curPos + 1)
                : m_inputEdit->setCursorPosition(curPos);
            } else
                m_inputEdit->setCursorPosition(curPos - 1);
        }
    }
    m_isContinue = true;
    expressionCheck();
    if (!m_inputEdit->text().isEmpty()) {
        emit clearStateChanged(false);
    }
    addUndo();
}

void SciExpressionBar::enterPercentEvent()
{
    if (!judgeinput())
        return;
    if (m_inputEdit->text().isEmpty()) {
        return;
    }
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",");
    bool isAtEnd = cursorPosAtEnd();
    /*
     * 当光标位置的前一位是运算符时，在函数方法前面补0,当函数的运算优先级小于等于
     * 前一位运算符时，则补（0
     * %做特殊处理--0%无效，故不显示
     */
    int diff = 0; //补数字后光标位移的距离
    QString sRegNum = "[＋－×÷/(^!%E]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    if (curpos == 0 || rx.exactMatch(exp.at(curpos - 1))) {
        m_inputEdit->insert("");
        diff = -1;
    } else
        m_inputEdit->insert("%");
    // 20200401 symbolFaultTolerance
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    int newPro = m_inputEdit->text().count(",");
    m_isUndo = false;
    m_isResult = false;
    addUndo();

    if (!isAtEnd) {
        if (newPro < proNumber && exp.at(curpos) != ",") {
            m_inputEdit->setCursorPosition(curpos + diff);
        } else {
            m_inputEdit->setCursorPosition(curpos + 1 + diff);
        }
    } else {
        m_inputEdit->setCursorPosition(curpos + 1 + diff);
    }
}

void SciExpressionBar::enterPointEvent()
{
    if (!judgeinput())
        return;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    if (curpos == 0) {
        m_inputEdit->insert("0.");
    } else {
        if (exp.at(curpos - 1) == ".")
            return;
        QString sRegNum = "[0-9,]+";
        QRegExp rx;
        rx.setPattern(sRegNum);
        if (rx.exactMatch(exp.at(curpos - 1))) {
            int index = exp.indexOf(QRegExp("[^0-9,]"), curpos);
            QString cut = exp.mid(curpos, index - curpos);
            int aftercurpos = cut.count(",");
            int before = exp.count(",");
            m_inputEdit->insert(".");
            int after = m_inputEdit->text().count(",");
            if (before - aftercurpos == after) {
                m_inputEdit->setCursorPosition(curpos + 1);
            } else {
                m_inputEdit->setCursorPosition(curpos);
            }
        } else
            m_inputEdit->insert("0.");
    }
    exp = pointFaultTolerance(m_inputEdit->text());
    if (exp != m_inputEdit->text())
        m_inputEdit->setText(exp);
    m_isUndo = false;
    m_isResult = false;
    emit clearStateChanged(false);
    addUndo();
}

void SciExpressionBar::enterBackspaceEvent()
{
    QString sRegNum = "[a-z]"; //20200811去除大写字母，否则Ｅ将被看作函数
    QRegExp rx;
    rx.setPattern(sRegNum);
    SSelection selection = m_inputEdit->getSelection();
    if (!selection.selected.isEmpty()) {
        QString text = m_inputEdit->text();
        QString seloldtext = text;
        int removepos = 0; //被删除位置
        //光标不在开头且光标左侧是字母或者光标右侧是字母
        if ((selection.curpos > 0 &&
                rx.exactMatch(m_inputEdit->text().at(selection.curpos - 1)))
                || (selection.curpos + selection.selected.size() < m_inputEdit->text().size() && rx.exactMatch(m_inputEdit->text().at(selection.curpos + selection.selected.size())))) {
            int selleftfunlen = 0; //选中左侧一部分函数长度
            int funpos = -1;
            int rightfunpos = -1;
            int j;
            for (int i = 0; i < m_funclist.size(); i++) {
                //记录光标左侧离光标最近的函数位
                funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], selection.curpos - 1);
                if (funpos != -1 && (funpos <= selection.curpos) && (selection.curpos < funpos + m_funclist[i].length())) {
                    selleftfunlen = selection.curpos - funpos;
                    break; //光标在函数开头和函数结尾之间
                } else
                    funpos = -1;
            }
            for (j = 0; j < m_funclist.size(); j++) {
                //记录离光标最近的右侧函数位
                rightfunpos = m_inputEdit->text().lastIndexOf(m_funclist[j], selection.curpos + selection.selected.size() - 1);
                if (rightfunpos != -1 && (rightfunpos + m_funclist[j].length() > selection.curpos + selection.selected.size()))
                    break;
                else
                    rightfunpos = -1;
            }
            if (funpos != -1 || rightfunpos != -1) {
                if (funpos != -1 && rightfunpos == -1) {
                    removepos = funpos;
                    text.remove(funpos, selection.selected.size() + selleftfunlen); //仅左侧有函数
                } else if (rightfunpos != -1 && funpos == -1) {
                    removepos = selection.curpos;
                    text.remove(selection.curpos, rightfunpos + m_funclist[j].length() - selection.curpos); //仅右侧有函数
                } else {
                    removepos = funpos;
                    text.remove(funpos, rightfunpos + m_funclist[j].length() - funpos); //函数中或左右均有
                }
            } else {
                removepos = selection.curpos;
                text.remove(selection.curpos, selection.selected.size());
            }
        } else {
            removepos = selection.curpos;
            text.remove(selection.curpos, selection.selected.size());
        }

        m_inputEdit->setText(text); //输入栏删除被选中
        // 20200401 symbolFaultTolerance
        m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
        // 20200316选中部分光标置位问题修复
        if ((seloldtext.mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length()) ==
                m_inputEdit->text().mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length())
            m_inputEdit->setCursorPosition(removepos);
        else if ((seloldtext.mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length()) >
                 m_inputEdit->text().mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length())
            m_inputEdit->setCursorPosition(removepos + 1);
        else
            m_inputEdit->setCursorPosition(removepos - 1);
    } else {
        QString text = m_inputEdit->text();
        int cur = m_inputEdit->cursorPosition();
        int funpos = -1;
        int i;
        if (text.size() > 0 && cur > 0 && text[cur - 1] == ",") {
            text.remove(cur - 2, 2);
            m_inputEdit->setText(text);
            // 20200401 symbolFaultTolerance
            m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
            m_inputEdit->setCursorPosition(cur - 2);
        } else {
            //退函数
            //光标不在开头且光标左侧是字母
            if (m_inputEdit->cursorPosition() > 0 && rx.exactMatch(m_inputEdit->text().at(m_inputEdit->cursorPosition() - 1))) {
                for (i = 0; i < m_funclist.size(); i++) {
                    //记录光标左侧离光标最近的函数位
                    funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], m_inputEdit->cursorPosition() - 1);
                    if (funpos != -1 && (funpos <= m_inputEdit->cursorPosition())
                            && (m_inputEdit->cursorPosition() <= funpos + m_funclist[i].length()))
                        break; //光标在函数开头和函数结尾之间
                    else
                        funpos = -1;
                }
                if (funpos != -1) {
                    m_inputEdit->setText(m_inputEdit->text().remove(funpos, m_funclist[i].length()));
                    m_inputEdit->setCursorPosition(funpos);
                }
            } else {
                int proNumber = text.count(",");
                m_inputEdit->backspace();
                int separator = proNumber - m_inputEdit->text().count(",");
                // 20200401 symbolFaultTolerance
                m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
                int newPro = m_inputEdit->text().count(",");
                if (cur > 0) {
                    QString sRegNum1 = "[0-9]+";
                    QRegExp rx1;
                    rx1.setPattern(sRegNum1);
                    //退数字
                    if (rx1.exactMatch(text.at(cur - 1)) && proNumber > newPro) {
                        if (text.mid(cur, text.length() - cur) == m_inputEdit->text().mid(m_inputEdit->text().length() - (text.length() - cur), text.length() - cur)) {
                            m_inputEdit->setCursorPosition(cur - 2);
                        } else
                            m_inputEdit->setCursorPosition(cur - 1);
                    } else {
                        if (separator < 0) {
                            m_inputEdit->setCursorPosition(cur - 1 - separator);
                        } else {
                            m_inputEdit->setCursorPosition(cur - 1);
                        }
                    }
                    //退小数点
                    if (text.at(cur - 1) == ".") {
                        if (text.mid(0, cur).count(",") != m_inputEdit->text().mid(0, cur).count(","))
                            m_inputEdit->setCursorPosition(cur);
                        else
                            m_inputEdit->setCursorPosition(cur - 1);
                    }
                }
            }
        }
    }
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }

    if (pointFaultTolerance(m_inputEdit->text()) != m_inputEdit->text()) {
        QTimer::singleShot(5000, this, &SciExpressionBar::pointCheckAfterDelete);
    }

    m_isResult = false;
    m_isContinue = true;
    m_isUndo = false;
    addUndo();
}

void SciExpressionBar::enterClearEvent()
{
    bool need_addundo = !m_inputEdit->text().isEmpty();
    if (m_isAllClear) {
        m_listModel->clearItems();
        m_listView->reset();
        m_isAllClear = false;

        emit clearStateChanged(false);
    } else {
        if (m_listModel->rowCount(QModelIndex()) == 0)
            emit clearStateChanged(false);
        else
            emit clearStateChanged(true);

        m_inputEdit->clear();
        m_isAllClear = true;
    }
    m_isResult = false;
    m_isUndo = false;
    if (need_addundo)
        addUndo();
    m_FEisdown = false;
    emit fEStateChanged(false);
}

void SciExpressionBar::enterEqualEvent()
{
//    m_evaluator->setVariable(QLatin1String("e"), DMath::e(), Variable::BuiltIn);
    QString exp = m_inputEdit->text();
    if (m_inputEdit->text().isEmpty()) {
        m_pair.first = false;
        return;
    }
    const QString expression = formatExpression(m_inputEdit->expressionText());
    QString exp1 = symbolComplement(expression);
    m_evaluator->setExpression(exp1);
    Quantity ans = m_evaluator->evalUpdateAns();
    // 20200403 bug-18971 表达式错误时输数字加等于再重新输入表达式历史记录错误表达式未被替换
    // 20200407 超过16位小数未科学计数
    qDebug() << "m_evaluator->error()" << m_evaluator->error();
    qDebug() << "ans" << m_inputEdit->expressionText();
    if (m_evaluator->error().isEmpty() && (exp.indexOf(QRegExp("[a-z＋－×÷/.,%()πe^!]")) != -1)) {
        if (ans.isNan() && !m_evaluator->isUserFunctionAssign()) {
            m_pair.first = false;
            m_expression = exp + "＝" + tr("Expression error");
            m_listModel->updataList(m_expression, -1, true);
            return;
        }
        //edit 20200413 for bug--19653
        QString result;
        if (m_FEisdown)
            result = DMath::format(ans, Quantity::Format::Scientific() + Quantity::Format::Precision(SCIPREC));
        else
            result = DMath::format(ans, Quantity::Format::General() + Quantity::Format::Precision(SCIPREC));
        QString formatResult = Utils::formatThousandsSeparators(result);
        formatResult = formatResult.replace(CN_ADD, EN_ADD)
                       .replace(CN_MIN, EN_MIN)
                       .replace(CN_MUL, EN_MUL)
                       .replace(CN_DIV, EN_DIV);
        //.replace(QString::fromUtf8(","), "");

        //        QString tStr = m_inputEdit->text().replace(QString::fromUtf8(","), "");
        QString tStr = m_inputEdit->text();
        //edit 20200518 for bug-26628
        QString StrToComp = formatResult;
        StrToComp = StrToComp.replace(EN_ADD, CN_ADD)
                    .replace(EN_MIN, CN_MIN)
                    .replace(EN_MUL, CN_MUL)
                    .replace(EN_DIV, CN_DIV);
        if (StrToComp == exp) {
            m_pair.first = false;
            return;
        }
        //end edit 20200518 for bug-26628
        // 20200402 需求3.2.1.6当输入的数字中有千位符，点击等号视为前后一致，不计入表达式
        if (formatResult != tStr) {
//        m_listModel->updataList(m_inputEdit->text() + "＝" + formatResult, m_hisRevision);
            m_inputEdit->setAnswer(formatResult, ans);
        }
        m_isContinue = false;
        m_inputEdit->setText(formatResult);
        formatResult = formatResult.replace(EN_MIN, CN_MIN);

        m_pair.first = true;
        m_expression = exp + "＝" + formatResult;
        m_listModel->updataList(m_expression, -1, true);
        m_listanswer = ans;
    } else {
        m_pair.first = false;
        if (!m_evaluator->error().isEmpty()) {
            m_expression = exp + "＝" + tr("Expression error");
            m_listModel->updataList(m_expression, -1, true);
        } else {
            return;
        }
    }
    m_isResult = true;
    m_isUndo = false;
    addUndo();
}

/**
 * @brief SciExpressionBar::enterDegEvent
 * 弧度/角度/梯度切换
 * @param mod
 */
void SciExpressionBar::enterDegEvent(int mod)
{
    //mod=1,enter走rad运算
    emit turnDeg();
    if (mod == 1)
        Settings::instance()->angleUnit = 'r';
    else if (mod == 2) {
        Settings::instance()->angleUnit = 'g';
    } else {
        Settings::instance()->angleUnit = 'd';
    }
}

/**
 * @brief SciExpressionBar::enterFEEvent
 * 是否以科学计数法的方式输出计算结果，状态可切换，按清除键后还原。
 * 默认不以科学计数法显示
 * @param isdown
 * 是否被按下
 */
void SciExpressionBar::enterFEEvent(bool isdown)
{
    if (isdown) {
        m_FEisdown = false;
        emit fEStateChanged(false);
    } else {
        m_FEisdown = true;
        emit fEStateChanged(true);
    }
}

void SciExpressionBar::enterRandEvent()
{
    m_inputEdit->clear();
    m_isResult = false;
    m_isUndo = false;
    QString str;
    QRandomGenerator* randGen = QRandomGenerator::system();
    for (int i = 0; i < SCIPREC; i++) {
        int n = randGen->generate() % 10;
        str.append(QString::number(n));
    }
    str = "0." + str;
    m_isResult = false;
    m_inputEdit->setText(str);
    if (!m_inputEdit->text().isEmpty())
        emit clearStateChanged(false);
    addUndo();
}

void SciExpressionBar::enterOperatorEvent(const QString &text)
{
    if (!judgeinput())
        return;
    QString zerotext = "0" + text;
    QString brackettext = "(0" + text;
    int length = text.length();
    emit clearStateChanged(false);
    if (m_inputEdit->text().isEmpty()) {
        m_inputEdit->setText(zerotext);
        m_isResult = false;
        m_isUndo = false;
        addUndo();
        return;
    }
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",");
    bool isAtEnd = cursorPosAtEnd();
    /*
     * 当光标位置的前一位是运算符时，在函数方法前面补0,当函数的运算优先级小于等于
     * 前一位运算符时，则补（0
     */
    int diff = 0; //补数字后光标位移的距离
    QString sRegNum = "[＋－×÷/(^]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    if (curpos == 0) {
        m_inputEdit->insert(zerotext);
        diff = 1;
    } else if (rx.exactMatch(exp.at(curpos - 1))) {
        if (exp.at(curpos - 1) == "^") {
            m_inputEdit->insert(brackettext);
            diff = 2;
        } else {
            m_inputEdit->insert(zerotext);
            diff = 1;
        }
    } else
        m_inputEdit->insert(text);

    // 20200401 symbolFaultTolerance
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    int newPro = m_inputEdit->text().count(",");
    m_isUndo = false;

    if (!isAtEnd) {
        if (newPro < proNumber && exp.at(curpos) != ",") {
            m_inputEdit->setCursorPosition(curpos + length - 1 + diff);
        } else {
            m_inputEdit->setCursorPosition(curpos + length + diff);
        }
        //对于^2,^3类型的，在后面接数字的时候补乘号
        if (isnumber(text.back()) && isnumber(m_inputEdit->text().at(m_inputEdit->cursorPosition()))) {
            m_inputEdit->insert("×");
            m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() - 1);
        }
    } else {
        m_inputEdit->setCursorPosition(curpos + length + diff);
    }
    addUndo();
}

void SciExpressionBar::enterFunctionEvent(const QString &text)
{
    if (!judgeinput())
        return;
    QString function = text + "()";
    int length = text.length();
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",");
    bool isAtEnd = cursorPosAtEnd();
    m_inputEdit->insert(function);
    // 20200401 symbolFaultTolerance
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    int newPro = m_inputEdit->text().count(",");
    m_isUndo = false;
    addUndo();

    if (!isAtEnd) {
        if (newPro < proNumber && exp.at(curpos) != ",") {
            m_inputEdit->setCursorPosition(curpos + length);
        } else {
            m_inputEdit->setCursorPosition(curpos + length + 1);
        }
    } else {
        m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() - 1);
    }
    emit clearStateChanged(false);
}

void SciExpressionBar::enterConstantEvent(const QString &text)
{
    if (!judgeinput())
        return;
    m_isResult = false;
    QString multitext = "×" + text;
    int length = text.length();

    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",");
    bool isAtEnd = cursorPosAtEnd();
    /* add 20200722
     * 当e和pi前面是数字类型的字符时，在前面补乘号防止直接出现表达式错误
     */
    int multi = 0;//是否需要补乘号
    if (curpos > 0 && isnumber(exp.at(curpos - 1))) {
        m_inputEdit->insert(multitext);
        multi = 1;
    } else
        m_inputEdit->insert(text);
    // 20200401 symbolFaultTolerance
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    int newPro = m_inputEdit->text().count(",");
    m_isUndo = false;
    addUndo();

    if (!isAtEnd) {
        if (newPro < proNumber && exp.at(curpos) != ",") {
            m_inputEdit->setCursorPosition(curpos + length - 1 + multi);
        } else {
            m_inputEdit->setCursorPosition(curpos + length + multi);
        }
    }
    emit clearStateChanged(false);
}

void SciExpressionBar::enterBracketEvent(const int &type)
{
    if (!judgeinput())
        return;
    if (m_inputEdit->text().count("(") >= 100 && type == 1)
        return;
    if (m_inputEdit->text().count(")") >= 100 && type == 2)
        return;
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",");
    bool isAtEnd = cursorPosAtEnd();
    switch (type) {
    case 1:
        m_inputEdit->insert("(");
        break;
    default:
        m_inputEdit->insert(")");
        break;
    }
    // 20200401 symbolFaultTolerance
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    int newPro = m_inputEdit->text().count(",");
    m_isUndo = false;
    addUndo();

    if (!isAtEnd) {
        if (newPro < proNumber && exp.at(curpos) != ",") {
            m_inputEdit->setCursorPosition(curpos);
        } else {
            m_inputEdit->setCursorPosition(curpos + 1);
        }
    }
    emit clearStateChanged(false);
}

void SciExpressionBar::enterSpecialFuncEvent(const QString &text)
{
    if (!judgeinput())
        return;
    if (m_inputEdit->text().isEmpty()) {
        if (text == "abs(") {
            m_inputEdit->setText("abs(0)");
            m_isResult = false;
            m_isUndo = false;
            addUndo();
            emit clearStateChanged(false);
            return;
        }
        return;
    }
    emit clearStateChanged(false);
    QString exptext = m_inputEdit->text();//表达式
    // 20200316百分号选中部分格式替代
    replaceSelection(m_inputEdit->text());
    int curPos = m_inputEdit->cursorPosition();
    if (m_inputEdit->text() == QString()) {
        m_inputEdit->setText(text);
        m_isResult = false;
        m_isUndo = false;
        addUndo();
        return;
    }
    m_isResult = false;
    m_isUndo = false;
    if (!expressionInFunc(exptext)) {
        m_inputEdit->setText(exptext);
        m_inputEdit->setCursorPosition(curPos);
        return;
    } else {
        if (exptext.count("(") == exptext.count(")")) {
            m_inputEdit->setCursorPosition(curPos - exptext.length());
            if (text == "1/(" && curPos - exptext.length() > 0 && isnumber(m_inputEdit->text().at(curPos - exptext.length() - 1))) {
                m_inputEdit->insert("×1/(");
            } else
                m_inputEdit->insert(text);
            int afterinsertpos = m_inputEdit->cursorPosition();
            m_inputEdit->setCursorPosition(afterinsertpos + exptext.length());
            m_inputEdit->insert(")");
        }
    }
    addUndo();
}

void SciExpressionBar::copyResultToClipboard()
{
    if (m_inputEdit->text().isEmpty())
        return;

    SSelection selection = m_inputEdit->getSelection();
    QApplication::clipboard()->setText(selection.selected); //将选中项放入粘贴板
}

void SciExpressionBar::copyClipboard2Result()
{
    if (!judgeinput())
        return;
    QString oldText = m_inputEdit->text(); //未粘贴操作的text
    int curpos = m_inputEdit->cursorPosition(); //未粘贴操作的光标位
    replaceSelection(oldText);
    QString exp = m_inputEdit->text();
    QString text = Utils::toHalfWidth(QApplication::clipboard()->text());
    text = text.left(text.indexOf("="));
    text = text.replace(EN_ADD, CN_ADD)
           .replace(EN_MIN, CN_MIN)
           .replace(EN_Underscore, CN_MIN)
           .replace(EN_MUL, CN_MUL)
           .replace('x', CN_MUL)
           .replace('X', CN_MUL)
           .replace(CN_Asterisk, CN_MUL)
           .replace(CN_LBracket, EN_LBracket)
           .replace(CN_RBracket, EN_RBracket)
           .replace(CN_Underscore, CN_MIN)
           .replace(CN_Percent, EN_Percent)
           .replace(EN_DIV, CN_DIV)
           .replace(" ", ""); //对粘贴板中的内容进行英替中

    //匹配函数方法
    QStringList list = text.split(QRegExp("[0-9＋－×÷/()%^!E.,。]")); //正则表达式中为科学模式下可存在的非字母;函数中;无法被复制
    for (int i = 0; i < list.size(); i++) {
        QString item = list[i];
        for (int j = 0; j < m_funclist.size(); j++) {
            if (item.toLower().contains(m_funclist[j])) {
                item.replace(item, m_funclist[j]); //包含函数名的取出;item中若存在两个函数名，只可以替代最前面的函数名
                break;
            }
            if (j == m_funclist.size() - 1)
                item.replace(item, QString());
        }
        text.replace(list[i], item);
    }

    while (exp.count("(") + text.count("(") > 100) {
        text.remove(text.lastIndexOf("("), 1);
    }
    while (exp.count(")") + text.count(")") > 100) {
        text.remove(text.lastIndexOf(")"), 1);
    }

    m_inputEdit->insert(text);

    QString faulttolerance = pointFaultTolerance(m_inputEdit->text());
    faulttolerance = m_inputEdit->symbolFaultTolerance(faulttolerance);

    if (faulttolerance != m_inputEdit->text())
        m_inputEdit->setText(faulttolerance); //如果经过容错处理的表达式有改变，重新设置表达式，不设置光标
    if (m_inputEdit->text() == exp) {
        m_inputEdit->setText(oldText);
        m_inputEdit->setCursorPosition(curpos);
        qDebug() << "Invalid content"; //提示是否复制了无效内容,复制的内容全是字母等
    }

    if (!m_inputEdit->text().isEmpty())
        emit clearStateChanged(false);
    m_isResult = false;
    m_isUndo = false;
    addUndo();
}

void SciExpressionBar::allElection()
{
    m_inputEdit->selectAll();
    SSelection selection;
    selection.selected = m_inputEdit->text();
    m_inputEdit->setSelection(selection);
}

void SciExpressionBar::shear()
{
    QString sRegNum = "[a-z]"; //20200811去除大写字母，否则Ｅ将被看作函数
    QRegExp rx;
    rx.setPattern(sRegNum);
    SSelection selection = m_inputEdit->getSelection();
    QString selectText = m_inputEdit->selectedText();
    selectText = selectText.replace(",", "");
    QApplication::clipboard()->setText(selectText);
    int removepos = 0; //被删除位置
    QString text = m_inputEdit->text();
    QString seloldtext = text;
    //光标不在开头且光标左侧是字母或者光标右侧是字母
    if ((selection.curpos > 0 &&
            rx.exactMatch(m_inputEdit->text().at(selection.curpos - 1)))
            || (selection.curpos + selection.selected.size() < m_inputEdit->text().size() && rx.exactMatch(m_inputEdit->text().at(selection.curpos + selection.selected.size())))) {
        int selleftfunlen = 0; //选中左侧一部分函数长度
        int funpos = -1;
        int rightfunpos = -1;
        int j;
        for (int i = 0; i < m_funclist.size(); i++) {
            //记录光标左侧离光标最近的函数位
            funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], selection.curpos - 1);
            if (funpos != -1 && (funpos <= selection.curpos) && (selection.curpos < funpos + m_funclist[i].length())) {
                selleftfunlen = selection.curpos - funpos;
                break; //光标在函数开头和函数结尾之间
            } else
                funpos = -1;
        }
        for (j = 0; j < m_funclist.size(); j++) {
            //记录离光标最近的右侧函数位
            rightfunpos = m_inputEdit->text().lastIndexOf(m_funclist[j], selection.curpos + selection.selected.size() - 1);
            if (rightfunpos != -1 && (rightfunpos + m_funclist[j].length() > selection.curpos + selection.selected.size()))
                break;
            else
                rightfunpos = -1;
        }
        if (funpos != -1 || rightfunpos != -1) {
            if (funpos != -1 && rightfunpos == -1) {
                removepos = funpos;
                text.remove(funpos, selection.selected.size() + selleftfunlen); //仅左侧有函数
            } else if (rightfunpos != -1 && funpos == -1) {
                removepos = selection.curpos;
                text.remove(selection.curpos, rightfunpos + m_funclist[j].length() - selection.curpos); //仅右侧有函数
            } else {
                removepos = funpos;
                text.remove(funpos, rightfunpos + m_funclist[j].length() - funpos); //函数中或左右均有
            }
        } else {
            removepos = selection.curpos;
            text.remove(selection.curpos, selection.selected.size());
        }
    } else {
        removepos = selection.curpos;
        text.remove(selection.curpos, selection.selected.size());
    }

    m_inputEdit->setText(text); //输入栏删除被选中
    // 20200401 symbolFaultTolerance
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    // 20200316选中部分光标置位问题修复
    if ((seloldtext.mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length()) ==
            m_inputEdit->text().mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length())
        m_inputEdit->setCursorPosition(removepos);
    else if ((seloldtext.mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length()) >
             m_inputEdit->text().mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length())
        m_inputEdit->setCursorPosition(removepos + 1);
    else
        m_inputEdit->setCursorPosition(removepos - 1);
    // 20200401 选中部分左侧为分隔符按退格的光标处理
    if (pointFaultTolerance(m_inputEdit->text()) != m_inputEdit->text()) {
        QTimer::singleShot(5000, this, &SciExpressionBar::pointCheckAfterDelete);
    }
    // end fix
    m_isResult = false;
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
    m_isContinue = true;
    m_isUndo = false;
    addUndo();
}

/**
 * @brief 删除事件，选中删除
 */
void SciExpressionBar::deleteText()
{
    QString sRegNum = "[a-z]"; //20200811去除大写字母，否则Ｅ将被看作函数
    QRegExp rx;
    rx.setPattern(sRegNum);
    SSelection selection = m_inputEdit->getSelection();
    int removepos = 0; //被删除位置
    QString text = m_inputEdit->text();
    QString seloldtext = text;
    //光标不在开头且光标左侧是字母或者光标右侧是字母
    if ((selection.curpos > 0 &&
            rx.exactMatch(m_inputEdit->text().at(selection.curpos - 1)))
            || (selection.curpos + selection.selected.size() < m_inputEdit->text().size() && rx.exactMatch(m_inputEdit->text().at(selection.curpos + selection.selected.size())))) {
        int selleftfunlen = 0; //选中左侧一部分函数长度
        int funpos = -1;
        int rightfunpos = -1;
        int j;
        for (int i = 0; i < m_funclist.size(); i++) {
            //记录光标左侧离光标最近的函数位
            funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], selection.curpos - 1);
            if (funpos != -1 && (funpos <= selection.curpos) && (selection.curpos < funpos + m_funclist[i].length())) {
                selleftfunlen = selection.curpos - funpos;
                break; //光标在函数开头和函数结尾之间
            } else
                funpos = -1;
        }
        for (j = 0; j < m_funclist.size(); j++) {
            //记录离光标最近的右侧函数位
            rightfunpos = m_inputEdit->text().lastIndexOf(m_funclist[j], selection.curpos + selection.selected.size() - 1);
            if (rightfunpos != -1 && (rightfunpos + m_funclist[j].length() > selection.curpos + selection.selected.size()))
                break;
            else
                rightfunpos = -1;
        }
        if (funpos != -1 || rightfunpos != -1) {
            if (funpos != -1 && rightfunpos == -1) {
                removepos = funpos;
                text.remove(funpos, selection.selected.size() + selleftfunlen); //仅左侧有函数
            } else if (rightfunpos != -1 && funpos == -1) {
                removepos = selection.curpos;
                text.remove(selection.curpos, rightfunpos + m_funclist[j].length() - selection.curpos); //仅右侧有函数
            } else {
                removepos = funpos;
                text.remove(funpos, rightfunpos + m_funclist[j].length() - funpos); //函数中或左右均有
            }
        } else {
            removepos = selection.curpos;
            text.remove(selection.curpos, selection.selected.size());
        }
    } else {
        removepos = selection.curpos;
        text.remove(selection.curpos, selection.selected.size());
    }

    m_inputEdit->setText(text); //输入栏删除被选中
    // 20200401 symbolFaultTolerance
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    // 20200316选中部分光标置位问题修复
    if ((seloldtext.mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length()) ==
            m_inputEdit->text().mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length())
        m_inputEdit->setCursorPosition(removepos);
    else if ((seloldtext.mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length()) >
             m_inputEdit->text().mid(0, removepos).remove(QRegExp("[＋－×÷/,.%()E]")).length())
        m_inputEdit->setCursorPosition(removepos + 1);
    else
        m_inputEdit->setCursorPosition(removepos - 1);
    // 20200401 选中部分左侧为分隔符按退格的光标处理
    if (pointFaultTolerance(m_inputEdit->text()) != m_inputEdit->text()) {
        QTimer::singleShot(5000, this, &SciExpressionBar::pointCheckAfterDelete);
    }
    // end fix
    m_isResult = false;
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
    m_isContinue = true;
    m_isUndo = false;
    addUndo();
    return;
}

void SciExpressionBar::handleTextChanged(const QString &text)
{
    Q_UNUSED(text);
    m_isAllClear = false;
    m_isContinue = true;
}

void SciExpressionBar::pointCheckAfterDelete()
{
    int curpos = m_inputEdit->cursorPosition();
    m_inputEdit->setText(pointFaultTolerance(m_inputEdit->text()));
    m_inputEdit->setCursorPosition(curpos);
}

bool SciExpressionBar::cursorPosAtEnd()
{
    return m_inputEdit->cursorPosition() == m_inputEdit->text().length();
}

QString SciExpressionBar::formatExpression(const QString &text)
{
    return QString(text)
           .replace(CN_ADD, EN_ADD)
           .replace(CN_MIN, EN_MIN)
           .replace(CN_MUL, EN_MUL)
           .replace(CN_DIV, EN_DIV)
           .replace(EN_Comma, "")
           .replace(EN_PI, QLatin1String("pi"));
}

void SciExpressionBar::revisionResults(const QModelIndex &index)
{
//    clearLinkageCache(m_inputEdit->text(), false);
    QString text = index.data(SimpleListModel::ExpressionRole).toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList historic = text.split(QString("＝"), QString::SkipEmptyParts);
#else
    QStringList historic = text.split("＝", Qt::SkipEmptyParts);
#endif
    if (historic.size() != 2)
        return;
    QString expression = historic.at(0);
//    m_hisRevision = index.row();
    m_inputEdit->setText(expression);
//    m_Selected = m_hisRevision;
    m_isResult = false;
    // fix addundo for history revision
    m_isUndo = false;
    addUndo();

    emit clearStateChanged(false);
}

/**
 * @brief 点击右侧历史记录
 */
void SciExpressionBar::hisRevisionResults(const QModelIndex &index, Quantity ans)
{
    QString text = index.data(SimpleListModel::ExpressionWithOutTip).toString();
//    QStringList historic = text.split(QString("＝"), QString::SkipEmptyParts);
//    if (historic.size() != 2)
//        return;
    QString expression = DMath::format(ans, Quantity::Format::General() + Quantity::Format::Precision(SCIPREC));
//    m_hisRevision = index.row();
    m_inputEdit->setAnswer(expression, ans);
    m_listModel->updataList(text, -1, true);
//    m_Selected = m_hisRevision;
    m_isResult = false;
    // fix addundo for history revision
    m_isUndo = false;
    addUndo();
    m_inputEdit->setFocus();
    emit clearStateChanged(false); //清除按键为C
}

bool SciExpressionBar::judgeinput()
{
    QString sRegNum = "[A-Za-z]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    SSelection selection = m_inputEdit->getSelection();

    if (selection.selected != "") {
        //光标不在开头且光标左侧是字母或者光标右侧是字母
        if ((selection.curpos > 0 &&
                rx.exactMatch(m_inputEdit->text().at(selection.curpos - 1)))
                || (selection.curpos + selection.selected.size() < m_inputEdit->text().size() && rx.exactMatch(m_inputEdit->text().at(selection.curpos + selection.selected.size())))) {
            int funpos = -1;
            int rightfunpos = -1;
            for (int i = 0; i < m_funclist.size(); i++) {
                //记录光标左侧离光标最近的函数位
                funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], selection.curpos - 1);
                if (funpos != -1 && (funpos <= selection.curpos) && (selection.curpos < funpos + m_funclist[i].length())) {
                    return false; //选中左侧在函数中
                }
            }
            for (int j = 0; j < m_funclist.size(); j++) {
                //记录离光标最近的右侧函数位
                rightfunpos = m_inputEdit->text().lastIndexOf(m_funclist[j], selection.curpos + selection.selected.size() - 1);
                if (rightfunpos != -1 && (rightfunpos + m_funclist[j].length() > selection.curpos + selection.selected.size()))
                    return false; //选中右侧在函数中
            }
        }
        return true;
    } else {
        if (m_inputEdit->cursorPosition() > 0 && rx.exactMatch(m_inputEdit->text().at(m_inputEdit->cursorPosition() - 1))) {
            for (int i = 0; i < m_funclist.size(); i++) {
                //记录光标左侧离光标最近的函数位
                int funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], m_inputEdit->cursorPosition() - 1);
                if (funpos != -1 && (funpos < m_inputEdit->cursorPosition()) && (m_inputEdit->cursorPosition() < funpos + m_funclist[i].length()))
                    return false; //光标在函数中
            }
        }
        return true;
    }
}
void SciExpressionBar::initConnect()
{
    connect(m_listView, &SimpleListView::obtainingHistoricalSimple, this,
            &SciExpressionBar::revisionResults);
    connect(m_listView, &SimpleListView::obtainingHistoricalSimple, m_inputEdit,
            &InputEdit::hisexpression);
    connect(m_inputEdit, &InputEdit::textChanged, this, &SciExpressionBar::handleTextChanged);
    connect(m_inputEdit, &InputEdit::keyPress, this, &SciExpressionBar::keyPress);
    connect(m_inputEdit, &InputEdit::equal, this, &SciExpressionBar::enterEqualEvent);
    connect(m_inputEdit, &InputEdit::cut, this, &SciExpressionBar::shear);
    connect(m_inputEdit, &InputEdit::copy, this, &SciExpressionBar::copyResultToClipboard);
    connect(m_inputEdit, &InputEdit::paste, this, &SciExpressionBar::copyClipboard2Result);
    connect(m_inputEdit, &InputEdit::deleteText, this, &SciExpressionBar::deleteText);
    connect(m_inputEdit, &InputEdit::selectAllText, this, &SciExpressionBar::allElection);
    connect(m_inputEdit, &InputEdit::undo, this, &SciExpressionBar::Undo);
    connect(m_inputEdit, &InputEdit::redo, this, &SciExpressionBar::Redo);
    connect(m_inputEdit, &InputEdit::setResult, this, &SciExpressionBar::setResultFalse);
    connect(m_inputEdit, &InputEdit::separateChange, this, &SciExpressionBar::onSeparateChange);
}

QString SciExpressionBar::symbolComplement(const QString exp)
{
    QString text = exp;
    int index = text.indexOf("(", 0);
    while (index != -1) {
        if (index >= 1 && text.at(index - 1).isNumber()) {
            text.insert(index, "×");
            ++index;
        }
        ++index;
        index = text.indexOf("(", index);
    }
    index = text.indexOf(")", 0);
    while (index != -1) {
        if (index < text.length() - 1 && text.at(index + 1).isNumber()) {
            text.insert(index + 1, "×");
            ++index;
        }
        ++index;
        index = text.indexOf(")", index);
    }
    //20200811　fix bug-42274 e,π,lastans跟随函数表达式错误问题
    index = text.indexOf("e", 0);
    while (index != -1) {
        text.insert(index, "(");
        text.insert(index + 2, ")");
        index = text.indexOf("e", index + 3);
    }
    index = text.indexOf("pi", 0);
    while (index != -1) {
        text.insert(index, "(");
        text.insert(index + 3, ")");
        index = text.indexOf("pi", index + 4);
    }
    index = text.indexOf("lastans", 0);
    while (index != -1) {
        text.insert(index, "(");
        text.insert(index + 8, ")");
        index = text.indexOf("lastans", index + 9);
    }
    return text;
}

QString SciExpressionBar::pointFaultTolerance(const QString &text)
{
    QString oldText = text;
    // QString reformatStr = Utils::reformatSeparators(QString(text).remove(','));
    QString reformatStr = oldText.replace(EN_ADD, CN_ADD)
                          .replace(EN_MIN, CN_MIN)
                          .replace(EN_Underscore, CN_MIN)
                          .replace(EN_MUL, CN_MUL)
                          .replace('x', CN_MUL)
                          .replace('X', CN_MUL)
                          .replace(CN_Asterisk, CN_MUL)
                          .replace(CN_LBracket, EN_LBracket)
                          .replace(CN_RBracket, EN_RBracket)
                          .replace(CN_Underscore, CN_MIN)
                          .replace(CN_Percent, EN_Percent)
                          /*.replace('/', QString::fromUtf8("÷"))*/; //对内容进行英替中
    QStringList list = reformatStr.split(QRegExp("[＋－×÷/(]")); //20200717去掉),否则下方)小数点容错无法进入
    QStringList symbollist;
    for (int i = 0; i < reformatStr.size(); ++i) {
        if (QRegExp("[＋－×÷/(]").exactMatch(reformatStr.at(i)))
            symbollist << reformatStr.at(i);
    }
    reformatStr.clear();
    for (int i = 0; i < list.size(); ++i) {
        QString item = list[i];
        int firstPoint = item.indexOf(".");
        if (item.contains(QString::fromUtf8("。"))) {
            if (firstPoint >= 0)
                item.remove(QString::fromUtf8("。"));
            else
                item.replace(QString::fromUtf8("。"), ".");
            firstPoint = item.indexOf(".");
        }
        if (firstPoint == -1) {
            reformatStr += item;
            if (!symbollist.isEmpty()) {
                reformatStr += symbollist.first();
                symbollist.pop_front();
            }
            continue;
        }
        if (firstPoint == 0) {
            item.insert(firstPoint, "0"); //小数点在数字前，进行补0;例:.123->0.123;此处未对reformatStr进行操作，导致只有两个.时才会进行补0
            ++firstPoint;
            // oldText.replace(list[i], item);
        } else {
            if (item.at(firstPoint - 1) == ")" || item.at(firstPoint - 1) == "%") {
                item.remove(firstPoint, 1);
                item.insert(firstPoint, "0."); //20200717)及%后小数点补0;与小数点输入处理一致
            }
        }
        if (item.count(".") > 1) {
            item.remove(".");
            item.insert(firstPoint, ".");
        }
        reformatStr += item;
        if (!symbollist.isEmpty()) {
            reformatStr += symbollist.first();
            symbollist.pop_front();
        }
    }
    for (int i = 0; i < reformatStr.size(); ++i) {
        //20200811避免e,π后的小数点补0
        if (reformatStr[i] == "." && (i == 0 || (!reformatStr[i - 1].isNumber() && reformatStr[i - 1] != "e" && reformatStr[i - 1] != "π"))) {
            reformatStr.insert(i, "0"); //补0操作，例:1+.2->1+0.2
            ++i;
        }
    }

    return reformatStr;
}

void SciExpressionBar::expressionCheck()
{
    QString exp = m_inputEdit->text();
    int cur = m_inputEdit->cursorPosition();
    //光标前的分隔符
    int separator = 0;

    for (int i = 0; i < exp.size(); ++i) {
        if (exp[i] == ",") {
            exp.remove(i, 1);
            --i;
            if (i + 1 < cur) {
                ++separator;
                --cur;
            }
        }
    }
    for (int i = 0; i < exp.size(); ++i) {
        while (exp[i].isNumber()) {
            // fix for delete 0 behind "."
            if (exp[i] == "0" && exp[i + 1] != "." && (i == 0 || exp[i - 1] != ".") &&
                    (i == 0 || !exp[i - 1].isNumber()) && (exp.size() == 1 || exp[i + 1].isNumber())) {
                exp.remove(i, 1);
                --i;
                if (i + 1 < cur)
                    --cur;
            }
            ++i;
        }
        if (exp[i] == "." && (i == 0 || !exp[i - 1].isNumber())) {
            exp.insert(i, "0");
            ++i;
            if (i < cur)
                ++cur;
        }
    }
    m_inputEdit->setText(exp);
    m_inputEdit->setCursorPosition(cur + separator);
}

bool SciExpressionBar::isnumber(QChar a)
{
    if (a.isDigit() || a == "." || a == "," || a == EN_PI || a == EN_Eular)
        return true;
    else
        return false;
}

/**
 * 点击倒数、绝对值、相反数时包含的表达式
 */
bool SciExpressionBar::expressionInFunc(QString &text)
{
    QString oldText = text;
    int curPos = m_inputEdit->cursorPosition();
    int epos = m_inputEdit->text().indexOf("E");
    QString sRegNum = "[0-9,.E]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    if (curPos == 0) {
        return false;
    }
    if (m_inputEdit->text().length() > curPos && rx.exactMatch(m_inputEdit->text().at(curPos))) {
        return false;
    }
    if (epos > -1 && epos == curPos - 1) {
        return false;
    }
    // start edit for task-13519
    QString sRegNum1 = "[^0-9,.)πe]";
    QRegExp rx1;
    rx1.setPattern(sRegNum1);
    if (rx1.exactMatch(oldText.at(curPos - 1)))
        return false;
    else {
        QString newtext = m_inputEdit->text();
        int percentpos = m_inputEdit->cursorPosition();
        int operatorpos =
            newtext.lastIndexOf(QRegularExpression(QStringLiteral("[^0-9,.eπE]")), percentpos - 1);

        bool nooperator = false;
        if (operatorpos > 0 && newtext.at(operatorpos - 1) == "E")
            operatorpos =
                newtext.mid(0, operatorpos - 1)
                .lastIndexOf(QRegularExpression(QStringLiteral("[^0-9,.eπE]")), percentpos - 1);
        /*出现以下情况：
         * 1. 负号在表达式的开头，如-1,-120等，视为一个整体的负数
         * 2. 负号在左括号的后一位，如(-1, (-121等，也视为一个整体的负数
         * 其中，当出现(-12)时，光标在右括号右侧时则会优先取到 ")",只有在右括号左侧才满足条件2*/
        if ((operatorpos == 0 && newtext.at(0) == "－")
                || (operatorpos > 0 && newtext.at(operatorpos) == "－" && newtext.at(operatorpos - 1) == "("))
            operatorpos--;

        if (operatorpos < 0) {
            operatorpos++;
            nooperator = true;
        }
//        QString exptext;  //表达式
        if (newtext.at(percentpos - 1) == ')') {
            if (operatorpos > 0 && newtext.at(operatorpos - 1) == '(')
                return false;
            do {
                operatorpos = newtext.lastIndexOf('(', operatorpos - 1);
                if (operatorpos <= 0) {
                    break;
                }
            } while (newtext.mid(operatorpos, newtext.size() - operatorpos).count('(') !=
                     newtext.mid(operatorpos, percentpos - operatorpos).count(')'));

            //匹配到的(不在开头且(左侧是字母
            QString sRegNum2 = "[A-Za-z]";
            QRegExp latterrx;
            latterrx.setPattern(sRegNum2);
            if (operatorpos > 0 && latterrx.exactMatch(m_inputEdit->text().at(operatorpos - 1))) {
                int funpos = -1; //记录函数位
                int i;
                for (i = 0; i < m_funclist.size(); i++) {
                    //记录(左侧离(最近的函数位
                    funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], operatorpos - 1);
                    if (funpos != -1 && (funpos + m_funclist[i].length() == operatorpos))
                        break; //(在函数结尾
                    else
                        funpos = -1;
                }
                if (funpos != -1)
                    operatorpos = operatorpos - m_funclist[i].length(); //截取函数
            } else if (operatorpos > 1 && m_inputEdit->text().mid(operatorpos - 2, 2) == "1/") {
                operatorpos = operatorpos - 2; //截取倒数
            }

            text = newtext.mid(operatorpos,
                               percentpos - operatorpos);  //截取表达式
        } else {
            text = newtext.mid(operatorpos + (nooperator == true ? 0 : 1),
                               percentpos - operatorpos + (nooperator == true ? 1 : 0) - 1);
            //截取表达式
        }
    }
    return true;
}

void SciExpressionBar::Undo()
{
    if (m_undo.isEmpty())
        return;
    //    clearLinkageCache();
    m_redo.append(m_undo.last());
    m_inputEdit->setRedoAction(true);
    m_undo.removeLast();
    m_isUndo = true;
    //20200619 从结果撤销到上一次表达式按下数字键不清空输入框
    if (m_isResult)
        m_isResult = false;
    // 20200319修复选中某一数字按下相同数字无法清除选中内容的问题
    if (!m_undo.isEmpty()) {
        if (m_undo.size() > 1) {
            for (int i = m_undo.size() - 1; i > 0; i--) {
                if (m_undo.at(i) == m_inputEdit->text() && m_undo.at(i - 1) == m_inputEdit->text())
                    m_undo.pop_back();
            }
        }
        m_inputEdit->setText(m_undo.last());
    } else {
        m_inputEdit->clear();
        m_inputEdit->setUndoAction(false);
    }
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
}

void SciExpressionBar::addUndo()
{
    // 20200319修复选中某一数字按下相同数字无法清除选中内容的问题
    //    if (!m_undo.isEmpty() && m_inputEdit->text() == m_undo.last())
    //        return;
    m_undo.append(m_inputEdit->text());
    m_redo.clear();
    m_inputEdit->setRedoAction(false);
    m_inputEdit->setUndoAction(true);
    SSelection selection;
    m_inputEdit->setSelection(selection);
}

void SciExpressionBar::Redo()
{
    if (m_redo.isEmpty())
        return;
    //    clearLinkageCache();
    m_inputEdit->setText(m_redo.last());
    m_undo.append(m_inputEdit->text());
    m_redo.removeLast();
    m_inputEdit->setRedoAction(!m_redo.isEmpty());
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
}

void SciExpressionBar::initTheme(int type)
{
    //edit for bug-21476
    int typeIn = type;
    if (typeIn == 0) {
        typeIn = DGuiApplicationHelper::instance()->themeType();
    }
    m_listDelegate->setThemeType(typeIn);
    m_inputEdit->themetypechanged(typeIn);
}

void SciExpressionBar::setResultFalse()
{
    m_isResult = false;
}

void SciExpressionBar::replaceSelection(QString text)
{
    QString seloldtext = text;
    SSelection selection = m_inputEdit->getSelection();
    int selcurPos = m_inputEdit->cursorPosition();
    if (!selection.selected.isEmpty()) {
        text.remove(selection.curpos, selection.selected.size());
        m_inputEdit->setText(text);
        if (selcurPos > selection.curpos &&
                selcurPos <= selection.curpos + selection.selected.size())
            selcurPos = selection.curpos;
        // 20200313选中部分光标置位问题修复
        if (seloldtext.mid(0, selcurPos).remove(QRegExp("[,]")).length() ==
                m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[,]")).length())
            m_inputEdit->setCursorPosition(selcurPos);
        else if (seloldtext.mid(0, selcurPos).remove(QRegExp("[,]")).length() >
                 m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[,]")).length())
            m_inputEdit->setCursorPosition(selcurPos + 1);
        else
            m_inputEdit->setCursorPosition(selcurPos - 1);
    }
    // reset selection
    selection = SSelection();
    m_inputEdit->setSelection(selection);
}

bool SciExpressionBar::isOperator(const QString &text)
{
    if (text == CN_ADD || text == CN_MIN ||
            text == CN_MUL || text == CN_DIV) {
        return true;
    } else {
        return false;
    }
}

void SciExpressionBar::moveLeft()
{
    QString sRegNum = "[A-Za-z]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    if (m_inputEdit->cursorPosition() > 0 && rx.exactMatch(m_inputEdit->text().at(m_inputEdit->cursorPosition() - 1))) {
        int funpos = -1;
        int i;
        for (i = 0; i < m_funclist.size(); i++) {
            funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], m_inputEdit->cursorPosition() - 1);
            if (funpos != -1 && funpos + m_funclist[i].length() == m_inputEdit->cursorPosition())
                break;
            else
                funpos = -1;
        }
        if (funpos != -1) {
            m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() - m_funclist[i].length());
        } else {
            m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() - 1);
        }
    } else {
        m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() - 1);
    }
    m_inputEdit->setFocus();
}

void SciExpressionBar::moveRight()
{
    QString sRegNum = "[A-Za-z]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    if (!cursorPosAtEnd() && rx.exactMatch(m_inputEdit->text().at(m_inputEdit->cursorPosition()))) {
        int funpos = -1;
        int i;
        for (i = 0; i < m_funclist.size(); i++) {
            funpos = m_inputEdit->text().indexOf(m_funclist[i], m_inputEdit->cursorPosition());
            if (funpos != -1 && funpos == m_inputEdit->cursorPosition())
                break;
            else
                funpos = -1;
        }
        if (funpos != -1) {
            m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() + m_funclist[i].length());
        } else {
            m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() + 1);
        }
    } else {
        m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() + 1);
    }
    m_inputEdit->setFocus();
}
//20200414 bug20294鼠标点击取消focus
InputEdit *SciExpressionBar::getInputEdit()
{
    return m_inputEdit;
}

/**
 * @brief SciExpressionBar::separateChange
 * 数字间隔位数发生改变
 */
void SciExpressionBar::onSeparateChange()
{
   m_listModel->updataOfSeparate();
}
