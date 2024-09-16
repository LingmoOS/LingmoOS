// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "proexpressionbar.h"

#include "../utils.h"
#include "../../3rdparty/core/settings.h"

#include <QVBoxLayout>
#include <QDebug>
#include <DGuiApplicationHelper>

const int LIST_HEIGHT = 35; //输入栏上方表达式的高度
const int INPUTEDIT_HEIGHT = 55;

#define BASE_TAG(basetag) \
    if (Settings::instance()->programmerBase == 16) \
        basetag = "0x"; \
    else if (Settings::instance()->programmerBase == 8) \
        basetag = "0o"; \
    else if (Settings::instance()->programmerBase == 2) \
        basetag = "0b"; \

ProExpressionBar::ProExpressionBar(QWidget *parent)
    : DWidget(parent)
{
    m_evaluator = Evaluator::instance();
    m_listView = new SimpleListView(0, this);
    m_listDelegate = new SimpleListDelegate(0, this);
    m_listModel = new SimpleListModel(0, this);
    m_inputEdit = new InputEdit(this);
    m_isContinue = true;
    m_isAllClear = false;
    m_isResult = false;
    m_inputNumber = false;
    m_isUndo = false;

    m_listView->setModel(m_listModel);
    m_listView->setItemDelegate(m_listDelegate);
    m_listView->setFixedHeight(LIST_HEIGHT);
    m_inputEdit->setFixedHeight(INPUTEDIT_HEIGHT);
    m_inputEdit->setAlignment(Qt::AlignRight);
    m_inputEdit->setTextMargins(10, 0, 10, 6);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_listView);
    layout->addWidget(m_inputEdit);
    layout->setMargin(0);
    layout->setSpacing(0);
    initConnect();

    m_funclist = {"nand",  "not", "xor", "and", "nor", "ror", "or",
                  "shl", "shr", "sal", "sar", "rol", "rcl", "rcr"
                 };
}

ProExpressionBar::~ProExpressionBar()
{

}

void ProExpressionBar::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

InputEdit *ProExpressionBar::getInputEdit()
{
    return m_inputEdit;
}

bool ProExpressionBar::isnumber(QChar a)
{
    return m_inputEdit->isNumber(a);
}

/**
 * @brief ProExpressionBar::judgeinput
 * @return 是否正常输入
 * 用于判断当前输入光标是否在函数间，不允许在函数中插入任何内容
 */
bool ProExpressionBar::judgeinput()
{
    QString sRegNum = "[a-z]";
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

void ProExpressionBar::enterNumberEvent(const QString &text)
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
//        m_listView->setFocus();
    }

    m_inputNumber = false;
    m_isUndo = false;

    //超过输入范围的不允许输入
    QString oldtext = m_inputEdit->text();
    int oldcurpos = m_inputEdit->cursorPosition();
    // 20200401 修改symbolFaultTolerance执行位置
    if (isNumberOutOfRange(text)) {
        m_inputEdit->setText(oldtext);
        m_inputEdit->setCursorPosition(oldcurpos);
    } else {
        replaceSelection(m_inputEdit->text());
        m_inputEdit->insert(text);
        int nowcur = m_inputEdit->cursorPosition();
        m_inputEdit->setText(symbolFaultTolerance(m_inputEdit->text()));
        //    m_inputEdit->setText(pointFaultTolerance(m_inputEdit->text()));
        m_inputEdit->setCursorPosition(nowcur);
    }
    addUndo();
    emit clearStateChanged(false);
}

void ProExpressionBar::enterSymbolEvent(const QString &text)
{
    if (!judgeinput())
        return;
    QString oldText = m_inputEdit->text();
    QString symbol = text;
    symbol.replace('/', QString::fromUtf8("÷"));
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
            if (firstStr == QString::fromUtf8("－")) {
                m_inputEdit->setText(oldText);
            } else {
                if (symbol == QString::fromUtf8("－") || symbol == "-")
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
            if (exp.mid(0, curPos).remove(QRegExp("[＋－×÷/,%()\\s]")) ==
                    m_inputEdit->text().mid(0, curPos).remove(QRegExp("[＋－×÷/,%()\\s]"))) {
                QString sRegNum = "[＋－×÷/]";
                QRegExp rx;
                rx.setPattern(sRegNum);
                rx.exactMatch(m_inputEdit->text().at(curPos))
                ? m_inputEdit->setCursorPosition(curPos + 1)
                : m_inputEdit->setCursorPosition(curPos);
                qDebug() << curPos;
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

void ProExpressionBar::enterBackspaceEvent()
{
    QString sRegNum = "[a-z]"; //20200811去除大写字母，否则Ｅ将被看作函数
    QRegExp rx;
    rx.setPattern(sRegNum);
    SSelection selection = m_inputEdit->getSelection();
    if (selection.selected != "") {
        selectedPartDelete(rx);
    } else {
        QString text = m_inputEdit->text();
        int cur = m_inputEdit->cursorPosition();
        int funpos = -1;
        int i;
        int Sepold = text.count(",") + text.count(" ");
        if (text.size() > 0 && cur > 1 && (text[cur - 1] == "," || (text[cur - 1] == " " && !text[cur - 2].isLower()))) {
            text.remove(cur - 2, 2);
            m_inputEdit->setText(text);
            // 20200401 symbolFaultTolerance
            m_inputEdit->setText(symbolFaultTolerance(m_inputEdit->text()));
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
                    int Sepnew = m_inputEdit->text().count(",") + m_inputEdit->text().count(" ");
                    m_inputEdit->setCursorPosition(funpos + Sepnew - Sepold + 1);
                }
            } else if (m_inputEdit->cursorPosition() > 1 && (text[cur - 1] == " " && text[cur - 2].isLower())) {
                for (i = 0; i < m_funclist.size(); i++) {
                    //记录光标左侧离光标最近的函数位
                    funpos = m_inputEdit->text().lastIndexOf(m_funclist[i], m_inputEdit->cursorPosition() - 2);
                    if (funpos != -1 && (funpos <= m_inputEdit->cursorPosition())
                            && (m_inputEdit->cursorPosition() <= funpos + m_funclist[i].length() + 1))
                        break; //光标在函数开头和函数结尾之间
                    else
                        funpos = -1;
                }
                if (funpos != -1) {
                    m_inputEdit->setText(m_inputEdit->text().remove(funpos, m_funclist[i].length()));
                    int Sepnew = m_inputEdit->text().count(",") + m_inputEdit->text().count(" ");
                    m_inputEdit->setCursorPosition(funpos + Sepnew - Sepold + 1);
                }
            } else {
                int proNumber = text.count(",") + text.count(" ");
                m_inputEdit->backspace();
                int separator = proNumber - m_inputEdit->text().count(",") - m_inputEdit->text().count(" ");
                // 20200401 symbolFaultTolerance
                m_inputEdit->setText(symbolFaultTolerance(m_inputEdit->text()));
                int newPro = m_inputEdit->text().count(",") + m_inputEdit->text().count(" ");
                if (cur > 0) {
                    QString sRegNum1 = "[0-9A-F]+";
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

    m_isResult = false;
    m_isContinue = true;
    m_isUndo = false;
    addUndo();
}

void ProExpressionBar::enterClearEvent()
{
    Settings::instance()->proRotateCarry = "00";
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
//    m_Selected = -1;
    if (need_addundo)
        addUndo();
}

void ProExpressionBar::enterEqualEvent()
{
    QString exp = m_inputEdit->text();
    if (m_inputEdit->text().isEmpty()) {
        return;
    }
    QString expression = QString();
    if (Settings::instance()->programmerBase == 8 || Settings::instance()->programmerBase == 16) {
        expression = InputEdit::formatExpression(2, m_inputEdit->scanAndExec(Settings::instance()->programmerBase, 2));
    } else {
        expression = InputEdit::formatExpression(Settings::instance()->programmerBase, m_inputEdit->text());
    }
    QString exp1 = symbolComplement(expression);
    m_evaluator->setExpression(exp1);
    Quantity ans = m_evaluator->evalUpdateAns();
    Settings::instance()->proRotateCarry.replace(1, 1, Settings::instance()->proRotateCarry.front());
    // 20200403 bug-18971 表达式错误时输数字加等于再重新输入表达式历史记录错误表达式未被替换
    // 20200407 超过16位小数未科学计数
    qDebug() << "m_evaluator->error()" << m_evaluator->error();
    qDebug() << "ans" << m_inputEdit->expressionText();
    if (m_evaluator->error().isEmpty() && (exp.indexOf(QRegExp("[a-z＋－×÷,%()\\s]")) != -1)) {
        if (ans.isNan() && !m_evaluator->isUserFunctionAssign()) {
            m_expression = exp + "＝" + tr("Expression error");
            m_listModel->updataList(m_expression, -1, true);
            return;
        }
        //edit 20200413 for bug--19653
        QString result;
        switch (Settings::instance()->programmerBase) {
        case 16:
            result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
            break;
        case 8:
            result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
            break;
        case 2:
            result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
            break;
        default:
            result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
            break;
        }
        while (result.startsWith('0') && result.length() > 1) {
            result.remove(0, 1);
        }
        QString formatResult = Utils::formatThousandsSeparatorsPro(result, Settings::instance()->programmerBase);
        formatResult = formatResult.replace(QString::fromUtf8("＋"), "+")
                       .replace(QString::fromUtf8("－"), "-")
                       .replace(QString::fromUtf8("×"), "*")
                       .replace(QString::fromUtf8("÷"), "/");
        //.replace(QString::fromUtf8(","), "");

        //        QString tStr = m_inputEdit->text().replace(QString::fromUtf8(","), "");
        QString tStr = m_inputEdit->text();
        //edit 20200518 for bug-26628
        QString StrToComp = formatResult;
        StrToComp = StrToComp.replace("+", QString::fromUtf8("＋"))
                    .replace("-", QString::fromUtf8("－"))
                    .replace("*", QString::fromUtf8("×"))
                    .replace("/", QString::fromUtf8("÷"));
        if (StrToComp == exp) {
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
        formatResult = formatResult.replace(QString::fromUtf8("-"), "－");

        m_expression = exp + "＝" + formatResult;
        m_listModel->updataList(m_expression, -1, true);
    } else {
        if (!m_evaluator->error().isEmpty()) {
            m_expression = exp + "＝" + tr("Expression error");
            m_listModel->updataList(m_expression, -1, true);
        } else {
            return;
        }
    }
    m_isResult = true;
    m_isUndo = false;
    m_inputEdit->getCurrentCursorPositionNumber(m_inputEdit->cursorPosition());
    addUndo();
}

void ProExpressionBar::enterNotEvent()
{
    if (!judgeinput())
        return;
    emit clearStateChanged(false);
    if (m_inputEdit->text().isEmpty()) {
        m_inputEdit->setText("not(0)");
        m_isResult = false;
        m_isUndo = false;
        addUndo();
        return;
    }
    bool hasselect = (m_inputEdit->getSelection().selected != "");
    QString oldText = m_inputEdit->text();
    QString exp = m_inputEdit->text();
    // 20200316百分号选中部分格式替代
    replaceSelection(m_inputEdit->text());
    int curPos = m_inputEdit->cursorPosition();
    if (m_inputEdit->text() == QString()) {
        m_inputEdit->setText("not(");
        m_isResult = false;
        m_isUndo = false;
        addUndo();
        return;
    }
    if (curPos == 0 && hasselect == false) {
        return;
    }
    if ((curPos == 0 && hasselect == true) || curposInNumber(curPos)) {
        return;
    }
    // start edit for task-13519
    //        QString sRegNum1 = "[^0-9,.×÷)]";
    QString sRegNum1 = "[^A-F^0-9,\\s)]";
    QRegExp rx1;
    rx1.setPattern(sRegNum1);
    if (rx1.exactMatch(exp.at(curPos - 1)))
        m_inputEdit->setText(oldText);
    else {
        QString newtext = m_inputEdit->text();
        int percentpos = m_inputEdit->cursorPosition();
        int operatorpos =
            newtext.lastIndexOf(QRegularExpression(QStringLiteral("[^A-F^0-9,\\s]")), percentpos - 1);

        /*出现以下情况：
         * 1. 负号在表达式的开头，如-1,-120等，视为一个整体的负数
         * 2. 负号在左括号的后一位，如(-1, (-121等，也视为一个整体的负数
         * 其中，当出现(-12)时，光标在右括号右侧时则会优先取到 ")",只有在右括号左侧才满足条件2*/
        if ((operatorpos == 0 && newtext.at(0) == "－")
                || (operatorpos > 0 && newtext.at(operatorpos) == "－" && newtext.at(operatorpos - 1) == "("))
            operatorpos--;

        bool nooperator = false;
        if (operatorpos < 0) {
            operatorpos++;
            nooperator = true;
        }
        QString exptext;  //表达式
        if (newtext.at(percentpos - 1) == ')') {
            if (operatorpos > 0 && newtext.at(operatorpos - 1) == '(') {
                m_inputEdit->setText(oldText);
                m_inputEdit->setCursorPosition(percentpos);
                return;
            }
            do {
                operatorpos = newtext.lastIndexOf('(', operatorpos - 1);
                if (operatorpos <= 0) {
                    break;
                }
            } while (newtext.mid(operatorpos, newtext.size() - operatorpos).count('(') !=
                     newtext.mid(operatorpos, percentpos - operatorpos).count(')'));

            //匹配到的(不在开头且(左侧是字母
            QString sRegNum2 = "[a-z]";
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

            exptext = newtext.mid(operatorpos,
                                  percentpos - operatorpos);  //截取表达式
        } else {
            exptext = newtext.mid(operatorpos + (nooperator == true ? 0 : 1),
                                  percentpos - operatorpos + (nooperator == true ? 1 : 0) - 1);
            //截取表达式
        }
//        QString express = symbolComplement(exptext);
        if (exptext.count("(") == exptext.count(")")) {
            m_inputEdit->setCursorPosition(curPos - exptext.length());
            m_inputEdit->insert("not(");
            int afterinsertpos = m_inputEdit->cursorPosition();
            m_inputEdit->setCursorPosition(afterinsertpos + exptext.length());
            m_inputEdit->insert(")");
        }
    }
    m_isResult = false;
    m_isUndo = false;
    addUndo();
}

/**
 * @brief ProExpressionBar::enterOperatorEvent
 * @param text:输入的操作符名称
 * 包含了程序员计算器除not以外的所有逻辑、位运算操作符的输入
 */
void ProExpressionBar::enterOperatorEvent(const QString &text)
{
    if (!judgeinput())
        return;
    emit clearStateChanged(false);
    QString zerotext = "0" + text;
    bool isreplace = false;
    int length = text.length();
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
    int proNumber = m_inputEdit->text().left(curpos).count(",") + m_inputEdit->text().left(curpos).count(" ");
    /*
     * 当光标位置的前一位是运算符时，在函数方法前面补0,当函数的运算优先级小于等于
     * 前一位运算符时，则补（0
     */
    int diff = 0; //补数字后光标位移的距离
    QString sRegNum = "[＋－×÷(]";
    QRegExp rx;
    rx.setPattern(sRegNum);
    if (curpos == 0 || rx.exactMatch(exp.at(curpos - 1))) {
        m_inputEdit->insert(zerotext);
        diff = 1;
    } else if (exp.at(curpos - 1).isLower()) {
        while (diff <= curpos - 1 && exp.at(curpos - 1 - diff).isLower()) {
            diff++;
        }
        exp.replace(curpos - diff, diff, text);
        m_inputEdit->setText(exp);
        isreplace = true;
    } else if (curpos > 1 && exp.at(curpos - 1) == " " && exp.at(curpos - 2).isLower()) {
        while (diff <= curpos - 2 && exp.at(curpos - 2 - diff).isLower()) {
            diff++;
        }
        exp.replace(curpos - 1 - diff, diff, text);
        m_inputEdit->setText(exp);
        isreplace = true;
    } else if (exp.at(curpos - 1) == "%") {
        diff = 1;
        exp.replace(curpos - diff, diff, text);
        m_inputEdit->setText(exp);
        isreplace = true;
    } else
        m_inputEdit->insert(text);

    // 20200401 symbolFaultTolerance
    bool isAtEnd = cursorPosAtEnd();
    m_inputEdit->setText(symbolFaultTolerance(m_inputEdit->text()));
    m_isUndo = false;
    m_isResult = false;
    addUndo();

    //计算光标位移的距离，区别是否是替换前面的。只考虑左边的分隔符变化的情况，如果为替换的情况，需要减去替换带来的长度差。
    if (isreplace) {
        int newPro = m_inputEdit->text().left(curpos + length - diff).count(",") + m_inputEdit->text().left(curpos + length - diff).count(" ");
        diff += proNumber - newPro;
        m_inputEdit->setCursorPosition(curpos + length - diff);
    } else {
        int newPro = m_inputEdit->text().left(curpos + length).count(",") + m_inputEdit->text().left(curpos + length).count(" ");
        diff += newPro - proNumber;
        if (!isAtEnd) {
            if (newPro < proNumber && exp.at(curpos) != "," && exp.at(curpos) != " ") {
                m_inputEdit->setCursorPosition(curpos + length - 1 + diff);
            } else {
                m_inputEdit->setCursorPosition(curpos + length + diff);
            }
        } else {
            m_inputEdit->setCursorPosition(curpos + length + diff);
        }
    }
}

void ProExpressionBar::enterOppositeEvent()
{
    if (!judgeinput())
        return;
    if (m_inputEdit->text().isEmpty()) {
        return;
    }
    emit clearStateChanged(false);
    m_isResult = false;
    m_isUndo = false;
    bool hasselect = (m_inputEdit->getSelection().selected != "");
    QString oldText = m_inputEdit->text();
    QString exp = m_inputEdit->text();
    // 20200316百分号选中部分格式替代
    replaceSelection(m_inputEdit->text());
    int curPos = m_inputEdit->cursorPosition();
    if (m_inputEdit->text() == QString()) {
        m_inputEdit->setText("(-");
        m_isUndo = false;
        m_isResult = false;
        addUndo();
        return;
    }
    if (curPos == 0 && hasselect == false) {
        return;
    }
    if ((curPos == 0 && hasselect == true) || curposInNumber(curPos)) {
        return;
    }
    // start edit for task-13519
    //        QString sRegNum1 = "[^0-9,.×÷)]";
    QString sRegNum1 = "[^A-F^0-9,\\s)]";
    QString sRegNum2 = "[A-F0-9,]";
    QRegExp rx1, rx2;
    rx1.setPattern(sRegNum1);
    rx2.setPattern(sRegNum2);
    if (rx1.exactMatch(exp.at(curPos - 1)))
        return;
    else if (exp.at(curPos - 1) == "0" && (curPos <= 1 || !rx2.exactMatch(exp.at(curPos - 2)))) {
        return;
    } else if (curPos > 1 && exp.at(curPos - 1) == " " && exp.at(curPos - 2).isLower()) {
        return;
    } else {
        QString newtext = m_inputEdit->text();
        int percentpos = m_inputEdit->cursorPosition();
        int operatorpos =
            newtext.lastIndexOf(QRegularExpression(QStringLiteral("[^A-F^0-9,\\s]")), percentpos - 1);

        bool nooperator = false;
        if (operatorpos < 0) {
            operatorpos++;
            nooperator = true;
        }
        QString exptext;  //表达式
        if (newtext.at(percentpos - 1) == ')') {
            if (operatorpos > 0 && newtext.at(operatorpos - 1) == '(') {
                m_inputEdit->setText(oldText);
                m_inputEdit->setCursorPosition(percentpos);
                return;
            }
            do {
                operatorpos = newtext.lastIndexOf('(', operatorpos - 1);
                if (operatorpos <= 0) {
                    break;
                }
            } while (newtext.mid(operatorpos, newtext.size() - operatorpos).count('(') !=
                     newtext.mid(operatorpos, percentpos - operatorpos).count(')'));

            //匹配到的(不在开头且(左侧是字母
            QString sRegNum3 = "[a-z]";
            QRegExp latterrx;
            latterrx.setPattern(sRegNum3);
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
            }

            exptext = newtext.mid(operatorpos,
                                  percentpos - operatorpos);  //截取表达式
        } else {
            exptext = newtext.mid(operatorpos + (nooperator == true ? 0 : 1),
                                  percentpos - operatorpos + (nooperator == true ? 1 : 0) - 1);
            //截取表达式
        }
        if (exptext.count("(") == exptext.count(")")) {
            m_inputEdit->setCursorPosition(curPos - exptext.length());
            if (Settings::instance()->programmerBase == 10) {
                m_inputEdit->insert("(-");
                int afterinsertpos = m_inputEdit->cursorPosition();
                m_inputEdit->setCursorPosition(afterinsertpos + exptext.length());
                m_inputEdit->insert(")");
            } else {
                QString basetag = QString();
                BASE_TAG(basetag);
                m_evaluator->setExpression("0-" + basetag + exptext);
                Quantity ans = m_evaluator->evalUpdateAns();
                if (m_evaluator->error().isEmpty()) {
                    if (ans.isNan() && !m_evaluator->isUserFunctionAssign())
                        return;
                    //edit 20200413 for bug--19653
                    QString result;
                    switch (Settings::instance()->programmerBase) {
                    case 16:
                        result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
                        break;
                    case 8:
                        result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
                        break;
                    case 2:
                        result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
                        break;
                    default:
                        return;
                    }
                    result = Utils::formatThousandsSeparatorsPro(result, Settings::instance()->programmerBase);
                    QString text = m_inputEdit->text();
                    text.remove(curPos - exptext.length(), exptext.length());
                    text.insert(curPos - exptext.length(), result);
                    m_inputEdit->setText(text);
                    m_inputEdit->setCursorPosition(curPos - exptext.length() + result.length());
                } else {
                    return;
                }
            }
        }
    }
    m_isUndo = false;
    m_isResult = false;
    addUndo();
}

void ProExpressionBar::enterLeftBracketsEvent()
{
    if (!judgeinput())
        return;
    if (m_inputEdit->text().count("(") >= 100)
        return;
    emit clearStateChanged(false);
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",") + m_inputEdit->text().count(" ");
    m_inputEdit->insert("(");
    // 20200401 symbolFaultTolerance
    bool isAtEnd = cursorPosAtEnd();
    m_inputEdit->setText(symbolFaultTolerance(m_inputEdit->text()));
    int newPro = m_inputEdit->text().count(",") + m_inputEdit->text().count(" ");
    m_isUndo = false;
    m_isResult = false;
    addUndo();

    if (!isAtEnd) {
        if (newPro < proNumber && exp.at(curpos) != "," && exp.at(curpos) != " ") {
            m_inputEdit->setCursorPosition(curpos);
        } else {
            m_inputEdit->setCursorPosition(curpos + 1);
        }
    }
}

void ProExpressionBar::enterRightBracketsEvent()
{
    if (!judgeinput())
        return;
    if (m_inputEdit->text().count(")") >= 100)
        return;
    emit clearStateChanged(false);
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",") + m_inputEdit->text().count(" ");
    m_inputEdit->insert(")");
    // 20200401 symbolFaultTolerance
    bool isAtEnd = cursorPosAtEnd();
    m_inputEdit->setText(symbolFaultTolerance(m_inputEdit->text()));
    int newPro = m_inputEdit->text().count(",") + m_inputEdit->text().count(" ");
    m_isUndo = false;
    m_isResult = false;
    addUndo();

    if (!isAtEnd) {
        if (newPro < proNumber && exp.at(curpos) != "," && exp.at(curpos) != " ") {
            m_inputEdit->setCursorPosition(curpos);
        } else {
            m_inputEdit->setCursorPosition(curpos + 1);
        }
    }
}

void ProExpressionBar::moveLeft()
{
    QString sRegNum = "[a-z]";
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

void ProExpressionBar::moveRight()
{
    QString sRegNum = "[a-z]";
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

void ProExpressionBar::initTheme(int type)
{
    m_listDelegate->setThemeType(type);
    m_inputEdit->themetypechanged(type);
}

void ProExpressionBar::revisionResults(const QModelIndex &index)
{
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

void ProExpressionBar::addUndo()
{
    m_undo.append(m_inputEdit->text());
    m_redo.clear();
    m_inputEdit->setRedoAction(false);
    m_inputEdit->setUndoAction(true);
    SSelection selection;
    m_inputEdit->setSelection(selection);
}

void ProExpressionBar::copyResultToClipboard()
{
    if (m_inputEdit->text().isEmpty())
        return;

    SSelection selection = m_inputEdit->getSelection();
    QApplication::clipboard()->setText(selection.selected); //将选中项放入粘贴板
}

void ProExpressionBar::copyClipboard2Result()
{
    if (!judgeinput())
        return;
    QString oldText = m_inputEdit->text(); //未粘贴操作的text
    int curpos = m_inputEdit->cursorPosition(); //未粘贴操作的光标位
    QString text = Utils::toHalfWidth(QApplication::clipboard()->text());
    text = text.left(text.indexOf("="));
    text = text.toUpper();
    text = text.replace('+', QString::fromUtf8("＋"))
           .replace('-', QString::fromUtf8("－"))
           .replace("_", QString::fromUtf8("－"))
           .replace('*', QString::fromUtf8("×"))
           .replace('X', QString::fromUtf8("×"))
           .replace(QString::fromUtf8("＊"), QString::fromUtf8("×"))
           .replace(QString::fromUtf8("（"), "(")
           .replace(QString::fromUtf8("）"), ")")
           .replace(QString::fromUtf8("——"), QString::fromUtf8("－"))
           .replace('/', QString::fromUtf8("÷")); //对粘贴板中的内容进行英替中

    //匹配函数方法
    QStringList list;
    //根据不同进制，对应不同的筛选规则
    switch (Settings::instance()->programmerBase) {
    case 16:
        text.remove(QRegExp("[G-Z]"));
        list = text.split(QRegExp("[A-F0-9＋－×÷()%\\s]"));
        break;
    case 8:
        text.remove(QRegExp("[8-9]"));
        list = text.split(QRegExp("[0-7＋－×÷()%\\s]"));
        break;
    case 2:
        text.remove(QRegExp("[2-9]"));
        list = text.split(QRegExp("[0-1＋－×÷()%\\s]"));
        break;
    default:
        list = text.split(QRegExp("[0-9＋－×÷(),%\\s]"));
        break;
    }
    for (int i = 0; i < list.size(); i++) {
        QString item = list[i];
        for (int j = 0; j < m_funclist.size(); j++) {
            if (item.toLower().contains(m_funclist[j])) {
                item.replace(item, m_funclist[j]); //包含函数名的取出;item中若存在两个函数名，只可以替代最前面的函数名
                break;
            }
            if (j == m_funclist.size() - 1) {
                item.replace(item, QString());
            }
        }
        text.replace(list[i], item);
    }
    if (isNumberOutOfRange(text))
        return;
    replaceSelection(oldText);
    QString exp = m_inputEdit->text();

    while (exp.count("(") + text.count("(") > 100) {
        text.remove(text.lastIndexOf("("), 1);
    }
    while (exp.count(")") + text.count(")") > 100) {
        text.remove(text.lastIndexOf(")"), 1);
    }
    m_inputEdit->insert(text);

    QString faulttolerance = symbolFaultTolerance(m_inputEdit->text());
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

void ProExpressionBar::allElection()
{
    m_inputEdit->selectAll();
    SSelection selection;
    selection.selected = m_inputEdit->text();
    m_inputEdit->setSelection(selection);
}

void ProExpressionBar::shear()
{
    QString sRegNum = "[a-z]"; //20200811去除大写字母，否则Ｅ将被看作函数
    QRegExp rx;
    rx.setPattern(sRegNum);
    QString text = m_inputEdit->text();
    QString selectText = m_inputEdit->selectedText();
    selectText = selectText.replace(",", "").remove(" ");
    QApplication::clipboard()->setText(selectText);
    selectedPartDelete(rx);
    addUndo();
    m_isUndo = false;
    m_isResult = false;
    m_isContinue = true;
    //发送C/AC切换信号
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
}

void ProExpressionBar::deleteText()
{
    QString sRegNum = "[a-z]"; //20200811去除大写字母，否则Ｅ将被看作函数
    QRegExp rx;
    rx.setPattern(sRegNum);
    selectedPartDelete(rx);

    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
    m_isResult = false;
    m_isContinue = true;
    m_isUndo = false;
    addUndo();
}

void ProExpressionBar::Undo()
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

void ProExpressionBar::Redo()
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

void ProExpressionBar::setResultFalse()
{
    m_isResult = false;
}

void ProExpressionBar::initConnect()
{
    connect(m_listView, &SimpleListView::obtainingHistoricalSimple, this,
            &ProExpressionBar::revisionResults);
    connect(m_listView, &SimpleListView::obtainingHistoricalSimple, m_inputEdit,
            &InputEdit::hisexpression);
    connect(m_inputEdit, &InputEdit::textChanged, this, &ProExpressionBar::handleTextChanged);
    connect(m_inputEdit, &InputEdit::keyPress, this, &ProExpressionBar::keyPress);
    connect(m_inputEdit, &InputEdit::equal, this, &ProExpressionBar::enterEqualEvent);
    connect(m_inputEdit, &InputEdit::cut, this, &ProExpressionBar::shear);
    connect(m_inputEdit, &InputEdit::copy, this, &ProExpressionBar::copyResultToClipboard);
    connect(m_inputEdit, &InputEdit::paste, this, &ProExpressionBar::copyClipboard2Result);
    connect(m_inputEdit, &InputEdit::deleteText, this, &ProExpressionBar::deleteText);
    connect(m_inputEdit, &InputEdit::selectAllText, this, &ProExpressionBar::allElection);
    connect(m_inputEdit, &InputEdit::undo, this, &ProExpressionBar::Undo);
    connect(m_inputEdit, &InputEdit::redo, this, &ProExpressionBar::Redo);
    connect(m_inputEdit, &InputEdit::setResult, this, &ProExpressionBar::setResultFalse);
    connect(m_inputEdit, &InputEdit::separateChange, this, &ProExpressionBar::onSeparateChange);
}

void ProExpressionBar::replaceSelection(QString text)
{
    QString seloldtext = text;
    SSelection selection = m_inputEdit->getSelection();
    int selcurPos = m_inputEdit->cursorPosition();
    if (selection.selected != "") {
        text.remove(selection.curpos, selection.selected.size());
        m_inputEdit->setText(text);
        if (selcurPos > selection.curpos &&
                selcurPos <= selection.curpos + selection.selected.size())
            selcurPos = selection.curpos;
        // 20200313选中部分光标置位问题修复
        if (seloldtext.mid(0, selcurPos).remove(QRegExp("[,\\s]")).length() ==
                m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[,\\s]")).length())
            m_inputEdit->setCursorPosition(selcurPos);
        else if (seloldtext.mid(0, selcurPos).remove(QRegExp("[,\\s]")).length() >
                 m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[,\\s]")).length())
            m_inputEdit->setCursorPosition(selcurPos + 1);
        else
            m_inputEdit->setCursorPosition(selcurPos - 1);
    }
    // reset selection
    selection = SSelection();
    m_inputEdit->setSelection(selection);
}

QString ProExpressionBar::symbolComplement(const QString exp)
{
    QString text = exp;
    int index = text.indexOf("(", 0);
    while (index != -1) {
        if (index >= 1 && isnumber(text.at(index - 1))) {
            text.insert(index, "×");
            ++index;
        }
        ++index;
        index = text.indexOf("(", index);
    }
    index = text.indexOf(")", 0);
    while (index != -1) {
        if (index < text.length() - 1 && isnumber(text.at(index + 1))) {
            text.insert(index + 1, "×");
            ++index;
        }
        ++index;
        index = text.indexOf(")", index);
    }
    //20200811　fix bug-42274 e,π,lastans跟随函数表达式错误问题
//    index = text.indexOf("e", 0);
//    while (index != -1) {
//        text.insert(index, "(");
//        text.insert(index + 2, ")");
//        index = text.indexOf("e", index + 3);
//    }
//    index = text.indexOf("pi", 0);
//    while (index != -1) {
//        text.insert(index, "(");
//        text.insert(index + 3, ")");
//        index = text.indexOf("pi", index + 4);
//    }
    index = text.indexOf("lastans", 0);
    while (index != -1) {
        text.insert(index, "(");
        text.insert(index + 8, ")");
        index = text.indexOf("lastans", index + 9);
    }
    return text;
}

bool ProExpressionBar::cursorPosAtEnd()
{
    return m_inputEdit->cursorPosition() == m_inputEdit->text().length();
}

/**
 * @brief ProExpressionBar::isOperator
 * @param text
 * @return 是否为有效的四则运算符
 */
bool ProExpressionBar::isOperator(const QString &text)
{
    if (text == QString::fromUtf8("＋") || text == QString::fromUtf8("－") ||
            text == QString::fromUtf8("×") || text == QString::fromUtf8("÷")) {
        return true;
    } else {
        return false;
    }
}

void ProExpressionBar::expressionCheck()
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
        while (isnumber(exp[i])) {
            if (exp[i] == "0" && (i == 0 || !isnumber(exp[i - 1])) && (exp.size() == 1 || isnumber(exp[i + 1]))) {
                exp.remove(i, 1);
                --i;
                if (i + 1 < cur)
                    --cur;
            }
            ++i;
        }
    }
    m_inputEdit->setText(exp);
    m_inputEdit->setCursorPosition(cur + separator);
}

/**
 * @brief ProExpressionBar::symbolFaultTolerance
 * @param text:输入栏中的内容
 * @return 修改后的内容
 * 处理当有多个四则运算符时，只保留最后输入的符号
 */
QString ProExpressionBar::symbolFaultTolerance(const QString &text)
{
    if (text.isEmpty())
        return text;
    QString exp = text;
    QString newText;
    QStringList symbolList;
    for (int i = 0; i < exp.length(); ++i) {
        if (!isOperator(exp.at(i))) {
            if (!symbolList.isEmpty()) {
                if (!newText.isEmpty())
                    newText.append(symbolList.last()); //保证数字中间的符号只有一个，去除多余符号
                if (newText.isEmpty() && symbolList.last() == "－")
                    newText.append(symbolList.last()); //保存负数的-号
            }
            newText.append(exp.at(i));
            symbolList.clear();
        } else {
            symbolList.append(exp.at(i));
            continue;
        }
    }
//    qDebug() << symbolList << " a " << newText;
    if (!symbolList.isEmpty() /*&& !newText.isEmpty()*/) { //防止输入栏中只有符号可输入*/+;暂未屏蔽%
        if ((newText.isEmpty() && symbolList.contains("－"))) {
            newText.append("－");
        } else if (!newText.isEmpty()) {
            newText.append(symbolList.last());
        }
    }
    return newText;
}

bool ProExpressionBar::isNumberOutOfRange(const QString &text)
{
    QString curtext = m_inputEdit->text();
    int pos = m_inputEdit->cursorPosition();
    SSelection selection = m_inputEdit->getSelection();
    if (selection.selected != "") {
        curtext.remove(selection.curpos, selection.selected.size());
        pos = selection.curpos;
    }
    curtext.insert(pos, text);

    m_numvec.clear();
    curtext.remove("，").remove(" ");
    for (int i = 0; i < curtext.length();) {
        if (isnumber(curtext.at(i))) {
            for (int j = 0; j < curtext.length() - i; j++) {
                if (i + j == curtext.length() - 1) {
                    if (isnumber(curtext.at(i + j))) {
                        m_numvec.append(curtext.mid(i, j + 1));
                        i += j + 1;
                    } else {
                        m_numvec.append(curtext.mid(i, j));
                        i += j;
                    }
                    break;
                }
                if (!isnumber(curtext.at(i + j))) {
                    m_numvec.append(curtext.mid(i, j));
                    i += j;
                    break;
                }
            }
        } else {
            if (curtext.at(i).isLower()) {
                if (curtext.at(i) == 'n' && curtext.at(i + 1) == 'a') {
                    i += 4;
                } else if (curtext.at(i) == 'o') {
                    i += 2;
                } else {
                    i += 3;
                }
            } else if ((i == 0 || !isnumber(curtext.at(i - 1))) && curtext.at(i) == QString::fromUtf8("－")
                       && curtext.length() > i + 1 && isnumber(curtext.at(i + 1))) {
                i++;
                for (int j = 0; j < curtext.length() - i; j++) {
                    if (!isnumber(curtext.at(i + j))) {
                        m_numvec.append(curtext.mid(i - 1, j + 1));
                        i += j;
                        break;
                    }
                    if (i + j == curtext.length() - 1) {
                        m_numvec.append(curtext.mid(i - 1, j + 2));
                        i += j + 1;
                        break;
                    }
                }
            } else {
                i++;
            }
        }
    }
    for (int i = 0; i < m_numvec.size(); i++) {
        QString num = InputEdit::formatExpression(Settings::instance()->programmerBase, m_numvec.at(i));
        Quantity ans(HNumber(num.toLatin1().data(), true));
        if (ans.isNan())
            return true;
        num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Binary() + Quantity::Format::NCut()).remove("0b");
        if (Settings::instance()->programmerBase == 10) {
            Quantity posans;
            Quantity negans;
            switch (Settings::instance()->proBitLength) {
            case 8:
                posans = ans - Quantity(128);
                negans = ans + Quantity(129);
                if (!posans.isNegative() || !negans.isPositive())
                    return true;
                else
                    break;
            case 16:
                posans = ans - Quantity(32768);
                negans = ans + Quantity(32769);
                if (!posans.isNegative() || !negans.isPositive())
                    return true;
                else
                    break;
            case 32:
                posans = ans - Quantity(HNumber("2147483648"));
                negans = ans + Quantity(HNumber("2147483649"));
                if (!posans.isNegative() || !negans.isPositive())
                    return true;
                else
                    break;
            case 64:
                posans = ans - Quantity(HNumber("9223372036854775808"));
                negans = ans + Quantity(HNumber("9223372036854775809"));
                if (!posans.isNegative() || !negans.isPositive())
                    return true;
                else
                    break;
            }
        } else {
            if (num.length() > Settings::instance()->proBitLength)
                return true;
        }
    }
    return false;
}

/**
 * @brief ProExpressionBar::selectedPartDelete
 * 选中部分的删除处理
 */
void ProExpressionBar::selectedPartDelete(const QRegExp &rx)
{
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
    m_inputEdit->setText(symbolFaultTolerance(m_inputEdit->text()));
    // 20200316选中部分光标置位问题修复
    // 选中部分删除后重新设置光标位置，用删除前的光标位置左侧（分隔符新增的只会影响左侧的数字）的数字部分去除分隔符后减去删除后的光标位置左侧数字部分的长度，为光标位置的差。
    int diff = (seloldtext.mid(0, removepos).remove(QRegExp("[＋－×÷/,%()\\s]")).length()) - m_inputEdit->text().mid(0, removepos).remove(QRegExp("[＋－×÷/,%()\\s]")).length();
    if (0 == diff) {
        m_inputEdit->setCursorPosition(removepos);
    } else {
        m_inputEdit->setCursorPosition(removepos + diff);
    }
}

/**
 * @brief ProExpressionBar::curposInNumber
 * @param curpos:当前光标
 * 判断当前光标是否在数字前或数字内，用于输入not，+/-时禁止输入
 */
bool ProExpressionBar::curposInNumber(int curpos)
{
    if (m_inputEdit->text().length() <= curpos) {
        return false;
    }
    if (isnumber(m_inputEdit->text().at(curpos))) {
        if (curpos < m_inputEdit->text().length() - 1
                && m_inputEdit->text().at(curpos) == " " && !isnumber(m_inputEdit->text().at(curpos + 1))) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}

void ProExpressionBar::handleTextChanged()
{
    m_isAllClear = false;
    m_isContinue = true;
}

/**
 * @brief ProExpressionBar::separateChange
 * 数字间隔位数发生改变
 */
void ProExpressionBar::onSeparateChange()
{
   m_listModel->updataOfSeparate();
}
