// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "expressionbar.h"

#include "../utils.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QTimer>
#include <DGuiApplicationHelper>
#include <QtGlobal>

const int STANDPREC = 15;
const int WIDGET_FIXHEIGHT = 147;
const int INPUTEDIT_HEIGHT = 55;
const int HISTORYLINKAGE_MAXSIZE = 10;

ExpressionBar::ExpressionBar(QWidget *parent)
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
    m_hisRevision = -1;
    m_linkageIndex = -1;
    m_isLinked = false;
    m_isUndo = false;
    m_Selected = -1;
    m_meanexp = true;
    // init inputEdit attributes.
    m_inputEdit->setFixedHeight(INPUTEDIT_HEIGHT);
    m_inputEdit->setAlignment(Qt::AlignRight);
    m_inputEdit->setTextMargins(10, 0, 10, 6);
//    m_inputEdit->setFocus();

    m_listView->setModel(m_listModel);
    m_listView->setItemDelegate(m_listDelegate);
//    DPalette pl = this->palette();
//    pl.setColor(DPalette::Light, QColor(0, 0, 0, 0));
//    pl.setColor(DPalette::Dark, QColor(0, 0, 0, 0));
//    pl.setColor(DPalette::Base, QColor(0, 0, 0, 0));
//    this->setPalette(pl);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_listView);
    layout->addWidget(m_inputEdit);
    layout->setMargin(0);
    layout->setSpacing(0);

    setFixedHeight(WIDGET_FIXHEIGHT);
    initConnect();
}

ExpressionBar::~ExpressionBar() {}

void ExpressionBar::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief ExpressionBar::enterNumberEvent
 * 输入0-9的数字
 * @param text
 * 对应的数字
 */
void ExpressionBar::enterNumberEvent(const QString &text)
{
    //    if (m_isLinked)
    //        clearLinkageCache();
    if (m_inputNumber && m_hisRevision == -1) {
        m_inputEdit->clear();
        m_isResult = false;
    }
    /*当直接在点击等于号后出的结果后面进行输入，则替换结果*/
    //20200615 按等于号后退格结果再输入十位数十位消失问题
    if (m_isResult) {
        m_inputEdit->clear();
        m_isResult = false;
        //        clearLinkageCache();
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
    m_inputEdit->insert(text);
    int nowcur = m_inputEdit->cursorPosition();
    m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
    m_inputEdit->setText(pointFaultTolerance(m_inputEdit->text()));
    m_inputEdit->setCursorPosition(nowcur);
    emit clearStateChanged(false);
    addUndo();
}

/**
 * @brief ExpressionBar::enterSymbolEvent
 * 输入四则运算符
 * @param text
 * 对应的运算符
 */
void ExpressionBar::enterSymbolEvent(const QString &text)
{
    QString symbol = text;
    symbol.replace('/', QString::fromUtf8("÷"));
    QString oldText = m_inputEdit->text();
    if (!m_hisLink.isEmpty() && m_hisLink.last().linkedItem == -1) {
        m_hisLink.last().linkedItem = m_listModel->rowCount(QModelIndex());
        m_hisLink.last().isLink = true;
        m_listDelegate->setHisLinked(m_hisLink.last().linkedItem);
        m_isLinked = false;
        if (m_hisLink.size() > HISTORYLINKAGE_MAXSIZE - 1) {
            m_hisLink.removeFirst();
            m_listDelegate->removeLine(0);
        }
    }
    //    clearSelectSymbol();   //fix for bug-14117
    if (m_isUndo) {
        int row = m_listModel->rowCount(QModelIndex());
        if (row != 0) {
            QString text =
                m_listModel->index(row - 1).data(SimpleListModel::ExpressionRole).toString();
            QString result = text.split("＝").last();
            if (result == m_inputEdit->text()) {
                historicalLinkageIndex his;
                his.linkageTerm = row - 1;
                his.linkageValue = result;
                his.linkedItem = row;
                m_hisLink.append(his);
                m_listDelegate->setHisLink(row - 1);
                m_listDelegate->setHisLinked(row);
            }
        }
    }
    m_isResult = false;
    m_isUndo = false;
    // QString oldText = m_inputEdit->text();
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
            if (exp.mid(0, curPos).remove(QRegExp("[＋－×÷,.%()E]")) ==
                    m_inputEdit->text().mid(0, curPos).remove(QRegExp("[＋－×÷,.%()E]"))) {
                QString sRegNum = "[＋－×÷]";
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

/**
 * @brief ExpressionBar::enterPointEvent
 * 输入小数点
 */
void ExpressionBar::enterPointEvent()
{
    //    if (m_isLinked)
    //        clearLinkageCache();
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    if (curpos == 0) {
        m_inputEdit->insert("0.");
    } else {
        if (exp.at(curpos - 1) == ".")
            return;
        //20200716修改在)及%后小数点不可输入问题
//        if (exp.at(curpos - 1) != ")" && exp.at(curpos - 1) != "%") {
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
//        }
    }
    exp = pointFaultTolerance(m_inputEdit->text());
    exp = m_inputEdit->symbolFaultTolerance(exp);
    if (exp != m_inputEdit->text())
        m_inputEdit->setText(exp);
    m_isUndo = false;
    m_isResult = false;
    emit clearStateChanged(false);
    addUndo();
}

/**
 * @brief ExpressionBar::enterBackspaceEvent
 * 输入退格键
 */
void ExpressionBar::enterBackspaceEvent()
{
    //    if (m_isResult)
    //        clearLinkageCache();
    // m_inputEdit->backspace();
    SSelection selection = m_inputEdit->getSelection();
    int selcurPos = m_inputEdit->cursorPosition();
    if (selection.selected != "") {
        QString text = m_inputEdit->text();
        QString seloldtext = text;
        text.remove(selection.curpos, selection.selected.size());
        m_inputEdit->setText(text);
        // 20200401 symbolFaultTolerance
        m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
        if (selcurPos > selection.curpos &&
                selcurPos <= selection.curpos + selection.selected.size())
            selcurPos = selection.curpos;
        // 20200316选中部分光标置位问题修复
        if (seloldtext.mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length() ==
                m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length())
            m_inputEdit->setCursorPosition(selcurPos);
        else if (seloldtext.mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length() >
                 m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length())
            m_inputEdit->setCursorPosition(selcurPos + 1);
        else
            m_inputEdit->setCursorPosition(selcurPos - 1);
    } else {
        QString text = m_inputEdit->text();
        int cur = m_inputEdit->cursorPosition();
        if (text.size() > 0 && cur > 0 && text[cur - 1] == ",") {
            text.remove(cur - 2, 2);
            m_inputEdit->setText(text);
            // 20200401 symbolFaultTolerance
            m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
            m_inputEdit->setCursorPosition(cur - 2);
        } else {
            int proNumber = text.count(",");
            m_inputEdit->backspace();
            int separator = proNumber - m_inputEdit->text().count(",");
            // 20200401 symbolFaultTolerance
            m_inputEdit->setText(m_inputEdit->symbolFaultTolerance(m_inputEdit->text()));
            int newPro = m_inputEdit->text().count(",");
            if (cur > 0) {
                QString sRegNum = "[0-9]+";
                QRegExp rx;
                rx.setPattern(sRegNum);
                //退数字
                if (rx.exactMatch(text.at(cur - 1)) && proNumber > newPro) {
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
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }

    QTimer::singleShot(5000, this, [ = ] {
        int curpos = m_inputEdit->cursorPosition();
        m_inputEdit->setText(pointFaultTolerance(m_inputEdit->text()));
        m_inputEdit->setCursorPosition(curpos);
    });

    m_isContinue = true;
    m_isUndo = false;
    m_isResult = false;
    addUndo();
}

/**
 * @brief ExpressionBar::enterClearEvent
 * C/AC的清空事件
 */
void ExpressionBar::enterClearEvent()
{
    bool need_addundo = !m_inputEdit->text().isEmpty();
    if (m_isAllClear) {
        m_listModel->clearItems();
        m_listView->reset();
        m_isAllClear = false;
        m_isLinked = false;    //20200619 清空历史记录时将联动参数置为false
        m_isAutoComputation = false;
        m_hisRevision = -1;
        m_hisLink.clear();
        m_listDelegate->removeAllLink();

        emit clearStateChanged(false);
    } else {
        if (m_listModel->rowCount(QModelIndex()) == 0)
            emit clearStateChanged(false);
        else
            emit clearStateChanged(true);

        m_inputEdit->clear();
        m_isAllClear = true;
        //        clearLinkageCache();
    }
    m_isResult = false;

    m_isUndo = false;
    m_Selected = -1;
    if (need_addundo)
        addUndo();
}

/**
 * @brief ExpressionBar::enterEqualEvent
 * 输入栏中表达式的求值，以及求值后的格式化
 */
void ExpressionBar::enterEqualEvent()
{
    QString oldtext = m_inputEdit->text();
    if (m_inputEdit->text().isEmpty())
        return;
    if (!m_isLinked)
        clearLinkageCache(m_inputEdit->text(), true);
    if (m_hisRevision == -1) {
        const QString expression = formatExpression(m_inputEdit->expressionText());
        QString exp = symbolComplement(expression);
        m_evaluator->setExpression(exp);
    } else {
        const QString expression = formatExpression(m_inputEdit->text());
        QString exp = symbolComplement(expression);
        m_evaluator->setExpression(exp);
    }
    Quantity ans = m_evaluator->evalUpdateAns();
    QString newResult;
    // 20200403 bug-18971 表达式错误时输数字加等于再重新输入表达式历史记录错误表达式未被替换
    // 20200407 超过16位小数未科学计数
    if (m_evaluator->error().isEmpty() && (oldtext.indexOf(QRegExp("[＋－×÷.,%()E]")) != -1)) {
        if (ans.isNan() && !m_evaluator->isUserFunctionAssign())
            return;
        //edit 20200413 for bug--19653
        const QString result = DMath::format(ans, Quantity::Format::General() + Quantity::Format::Precision(STANDPREC));
        QString formatResult = Utils::formatThousandsSeparators(result);
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
        if (StrToComp == oldtext)
            return;
        //end edit 20200518 for bug-26628
        // 20200402 需求3.2.1.6当输入的数字中有千位符，点击等号视为前后一致，不计入表达式
        if (formatResult != tStr) {
            m_listModel->updataList(m_inputEdit->text() + "＝" + formatResult, m_hisRevision);
            m_inputEdit->setAnswer(formatResult, ans);
            newResult = formatResult;
        }
        m_isContinue = false;
    } else {
        // 20200403 bug-18971 表达式错误时输数字加等于再重新输入表达式历史记录错误表达式未被替换
        if (!m_evaluator->error().isEmpty()) {
            m_listModel->updataList(m_inputEdit->text() + "＝" + tr("Expression error"),
                                    m_hisRevision);
            m_meanexp = false; // 20200409 输入错误表达式后，修改历史记录,再次输入表达式后高亮未取消
        } else {
            m_meanexp = false;
            return;
        }
        if (m_hisRevision == -1)
            m_hisRevision = m_listModel->rowCount(QModelIndex()) - 1;
        else {
            for (int i = 0; i < m_hisLink.size(); ++i) {
                if (m_hisLink[i].linkageTerm == m_hisRevision) {
                    // m_listDelegate->removeLine(m_hisLink[i].linkageTerm,
                    // m_hisLink[i].linkedItem);
                    m_listDelegate->removeLine(i);
                    m_hisLink.removeAt(i);
                    --i;
                }
            }
        }
    }

    QString selectedresult = QString();
    if (m_isLinked) {
        if (m_hisRevision == -1) {
            m_isLinked = false;
            m_listView->scrollToBottom();
            m_isResult = true;
            m_isUndo = false;
            addUndo();
            return;
        }
        for (int i = 0; i < m_hisLink.size(); ++i) {
            if (m_hisRevision == m_hisLink[i].linkageTerm) {
                m_hisRevision = m_hisLink[i].linkedItem;
                if (m_hisRevision == -1) {
                    m_hisLink.remove(i);
                    m_listDelegate->removeHisLink();
                    break;
                }
                QString text = m_listModel->index(m_hisRevision)
                               .data(SimpleListModel::ExpressionRole)
                               .toString();
                QString linkedExp = text.split("＝").first();
                int length = m_hisLink[i].linkageValue.length();
                if (linkedExp.left(length) != m_hisLink[i].linkageValue ||
                        (!isOperator(linkedExp.at(length)) && (linkedExp.at(length) != "("))) {
                    m_hisLink.remove(i);
                    m_listDelegate->removeHisLink();
                    break;
                }
                if (newResult.isEmpty())
                    break;
                newResult = newResult.replace("+", QString::fromUtf8("＋"))
                            .replace("-", QString::fromUtf8("－"));
                m_hisLink[i].linkageValue = newResult;
                QString newText = newResult + linkedExp.right(linkedExp.length() - length);
                m_inputEdit->setText(newText);
                settingLinkage();
            }
        }
        if (m_Selected != -1 && m_evaluator->error().isEmpty()) {
            selectedresult = m_listModel->index(m_Selected)
                             .data(SimpleListModel::ExpressionRole)
                             .toString()
                             .split("＝")
                             .last();
            m_inputEdit->setText(selectedresult);
        }
        // 20200403 表达式错误重新输入新表达式输入框无结果
        //        if (m_evaluator->error().isEmpty())
        //            m_inputEdit->clear();
    }
    // 20200403 bug-18971 表达式错误时输数字加等于再重新输入表达式历史记录错误表达式未被替换
    if (m_evaluator->error().isEmpty() && (oldtext.indexOf(QRegExp("[＋－×÷,.%()E]")) != -1))
        m_hisRevision = -1;
    m_listView->scrollToBottom();
    m_isLinked = false;
    m_isResult = true;
    m_isUndo = false;
    addUndo();
}

/**
 * @brief ExpressionBar::enterPercentEvent
 * 当前使用的输入百分号的事件
 */
void ExpressionBar::enterPercentEvent()
{
    if (m_inputEdit->text().isEmpty()) {
//        m_inputEdit->setText("0%");
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

/**
 * @brief ExpressionBar::enterBracketsEvent
 * 数字键盘中点击左右括号的事件，直接显示左右括号，光标位于中间
 */
void ExpressionBar::enterBracketsEvent()
{
    if (!m_hisLink.isEmpty() && m_hisLink.last().linkedItem == -1) {
        m_hisLink.last().linkedItem = m_listModel->rowCount(QModelIndex());
        m_hisLink.last().isLink = true;
        m_listDelegate->setHisLinked(m_hisLink.last().linkedItem);
        m_isLinked = false;
        if (m_hisLink.size() > HISTORYLINKAGE_MAXSIZE - 1) {
            m_hisLink.removeFirst();
            m_listDelegate->removeLine(0);
        }
    }
    //    clearSelectSymbol();   //fix for bug-14117
    if (m_isUndo) {
        int row = m_listModel->rowCount(QModelIndex());
        if (row != 0) {
            QString text =
                m_listModel->index(row - 1).data(SimpleListModel::ExpressionRole).toString();
            QString result = text.split("＝").last();
            if (result == m_inputEdit->text()) {
                historicalLinkageIndex his;
                his.linkageTerm = row - 1;
                his.linkageValue = result;
                his.linkedItem = row;
                m_hisLink.append(his);
                m_listDelegate->setHisLink(row - 1);
                m_listDelegate->setHisLinked(row);
            }
        }
    }
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString oldText = m_inputEdit->text();
    int currentPos = m_inputEdit->cursorPosition();
    m_inputEdit->insert("()");
    // 20200401 symbolFaultTolerance
    QString formatexp = m_inputEdit->symbolFaultTolerance(m_inputEdit->text());
    if (oldText.mid(0, currentPos).count(",") == m_inputEdit->text().mid(0, currentPos).count(","))
        m_inputEdit->setCursorPosition(currentPos + 1);
    else
        m_inputEdit->setCursorPosition(currentPos);
    if (formatexp == oldText)
        m_inputEdit->setText(formatexp);
    m_isUndo = false;
    emit clearStateChanged(false);
    addUndo();
}

/**
 * @brief ExpressionBar::enterLeftBracketsEvent
 * 键盘左括号输入的事件，rightbrackets为右括号(数字键盘无法输入单个的左右括号)
 */
void ExpressionBar::enterLeftBracketsEvent()
{
    if (!m_hisLink.isEmpty() && m_hisLink.last().linkedItem == -1) {
        m_hisLink.last().linkedItem = m_listModel->rowCount(QModelIndex());
        m_hisLink.last().isLink = true;
        m_listDelegate->setHisLinked(m_hisLink.last().linkedItem);
        m_isLinked = false;
        if (m_hisLink.size() > HISTORYLINKAGE_MAXSIZE - 1) {
            m_hisLink.removeFirst();
            m_listDelegate->removeLine(0);
        }
    }
    //    clearSelectSymbol();   //fix for bug-14117
    if (m_isUndo) {
        int row = m_listModel->rowCount(QModelIndex());
        if (row != 0) {
            QString text =
                m_listModel->index(row - 1).data(SimpleListModel::ExpressionRole).toString();
            QString result = text.split("＝").last();
            if (result == m_inputEdit->text()) {
                historicalLinkageIndex his;
                his.linkageTerm = row - 1;
                his.linkageValue = result;
                his.linkedItem = row;
                m_hisLink.append(his);
                m_listDelegate->setHisLink(row - 1);
                m_listDelegate->setHisLinked(row);
            }
        }
    }
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",");
    bool isAtEnd = cursorPosAtEnd();
    m_inputEdit->insert("(");
    // 20200401 symbolFaultTolerance
    QString formatexp = m_inputEdit->symbolFaultTolerance(m_inputEdit->text());
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
    if (formatexp == exp)
        m_inputEdit->setText(formatexp);
    emit clearStateChanged(false);
}

void ExpressionBar::enterRightBracketsEvent()
{
    if (!m_hisLink.isEmpty() && m_hisLink.last().linkedItem == -1) {
        m_hisLink.last().linkedItem = m_listModel->rowCount(QModelIndex());
        m_hisLink.last().isLink = true;
        m_listDelegate->setHisLinked(m_hisLink.last().linkedItem);
        m_isLinked = false;
        if (m_hisLink.size() > HISTORYLINKAGE_MAXSIZE - 1) {
            m_hisLink.removeFirst();
            m_listDelegate->removeLine(0);
        }
    }
    //    clearSelectSymbol();   //fix for bug-14117
    if (m_isUndo) {
        int row = m_listModel->rowCount(QModelIndex());
        if (row != 0) {
            QString text =
                m_listModel->index(row - 1).data(SimpleListModel::ExpressionRole).toString();
            QString result = text.split("＝").last();
            if (result == m_inputEdit->text()) {
                historicalLinkageIndex his;
                his.linkageTerm = row - 1;
                his.linkageValue = result;
                his.linkedItem = row;
                m_hisLink.append(his);
                m_listDelegate->setHisLink(row - 1);
                m_listDelegate->setHisLinked(row);
            }
        }
    }
    m_isResult = false;
    replaceSelection(m_inputEdit->text());
    QString exp = m_inputEdit->text();
    int curpos = m_inputEdit->cursorPosition();
    int proNumber = m_inputEdit->text().count(",");
    bool isAtEnd = cursorPosAtEnd();
    m_inputEdit->insert(")");
    // 20200401 symbolFaultTolerance
    QString formatexp = m_inputEdit->symbolFaultTolerance(m_inputEdit->text());
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
    if (formatexp == exp)
        m_inputEdit->setText(formatexp);
    emit clearStateChanged(false);
}

/**
 * @brief ExpressionBar::copyResultToClipboard
 * 将输入栏中的内容复制到剪贴板
 */
void ExpressionBar::copyResultToClipboard()
{
//    m_evaluator->unsetVariable(QLatin1String("e"), true);
    if (m_inputEdit->text().isEmpty())
        return;
    // QApplication::clipboard()->setText(m_inputEdit->selectedText());

    //edit for bug-37850 非全选不复制计算结果->输入栏复制的内容不是表达式的结果，而是选中的表达式
    SSelection selection = m_inputEdit->getSelection();
    QApplication::clipboard()->setText(selection.selected); //将选中项放入粘贴板
//    SSelection selection = m_inputEdit->getSelection();
//    if (selection.selected != "" && selection.selected != m_inputEdit->text()) {
//        QApplication::clipboard()->setText(selection.selected);
//        return;
//    }

//    const QString expression = formatExpression(m_inputEdit->text());
//    m_evaluator->setExpression(expression);
//    Quantity ans = m_evaluator->evalUpdateAns();

//    if (m_evaluator->error().isEmpty()) {
//        if (ans.isNan() && !m_evaluator->isUserFunctionAssign())
//            return;

//        //edit 20200413 for bug--19653
//        const QString result = DMath::format(ans, Quantity::Format::General() + Quantity::Format::Precision(STANDPREC));
//        QString formatResult = Utils::formatThousandsSeparators(result);
//        formatResult = formatResult.replace('-', "－").replace('+', "＋");
//        // m_inputEdit->setAnswer(formatResult, ans);

//        QApplication::clipboard()->setText(formatResult);
//    } else {
//        QApplication::clipboard()->setText(m_inputEdit->text());
//    }
}

/**
 * @brief ExpressionBar::copyClipboard2Result
 * 将剪贴板的内容复制到输入栏
 */
void ExpressionBar::copyClipboard2Result()
{
    if (!m_hisLink.isEmpty() && m_hisLink.last().linkedItem == -1) {
        m_hisLink.last().linkedItem = m_listModel->rowCount(QModelIndex());
        m_hisLink.last().isLink = true;
        m_listDelegate->setHisLinked(m_hisLink.last().linkedItem);
        m_isLinked = false;
        if (m_hisLink.size() > HISTORYLINKAGE_MAXSIZE - 1) {
            m_hisLink.removeFirst();
            m_listDelegate->removeLine(0);
        }
    }
    //    clearSelectSymbol();   //fix for bug-14117
    if (m_isUndo) {
        int row = m_listModel->rowCount(QModelIndex());
        if (row != 0) {
            QString text =
                m_listModel->index(row - 1).data(SimpleListModel::ExpressionRole).toString();
            QString result = text.split("＝").last();
            if (result == m_inputEdit->text()) {
                historicalLinkageIndex his;
                his.linkageTerm = row - 1;
                his.linkageValue = result;
                his.linkedItem = row;
                m_hisLink.append(his);
                m_listDelegate->setHisLink(row - 1);
                m_listDelegate->setHisLinked(row);
            }
        }
    }

    QString oldText = m_inputEdit->text(); //未粘贴操作的text
    int curpos = m_inputEdit->cursorPosition(); //未粘贴操作的光标位
    replaceSelection(oldText);
    QString exp = m_inputEdit->text();
    QString text = Utils::toHalfWidth(QApplication::clipboard()->text());
    text = text.left(text.indexOf("="));
    text = text.replace('+', QString::fromUtf8("＋"))
           .replace('-', QString::fromUtf8("－"))
           .replace("_", QString::fromUtf8("－"))
           .replace('*', QString::fromUtf8("×"))
           .replace('x', QString::fromUtf8("×"))
           .replace('X', QString::fromUtf8("×"))
           .replace(QString::fromUtf8("＊"), QString::fromUtf8("×"))
           .replace(QString::fromUtf8("（"), "(")
           .replace(QString::fromUtf8("）"), ")")
           .replace(QString::fromUtf8("——"), QString::fromUtf8("－"))
           .replace(QString::fromUtf8("％"), "%")
           .replace('/', QString::fromUtf8("÷")); //对粘贴板中的内容进行英替中
    text.remove(QRegExp("[^0-9＋－×÷,.%()。]"));
//    text = pasteFaultTolerance(text);
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

/**
 * @brief ExpressionBar::allElection
 * 全选事件
 */
void ExpressionBar::allElection()
{
    m_inputEdit->selectAll();
    SSelection selection;
    selection.selected = m_inputEdit->text();
    m_inputEdit->setSelection(selection);
}

/**
 * @brief ExpressionBar::shear
 * 剪切事件
 */
void ExpressionBar::shear()
{
    QString text = m_inputEdit->text();
    int selcurPos = m_inputEdit->cursorPosition();
    QString selectText = m_inputEdit->selectedText();
    selectText = selectText.replace(",", "");
    QApplication::clipboard()->setText(selectText);
    int start = m_inputEdit->selectionStart();
    int length = m_inputEdit->selectionLength();
    text.remove(start, length);
    m_inputEdit->setText(text);
    selcurPos -= length;
    m_isResult = false;
    addUndo();
    m_isUndo = false;
    //设置剪切后光标位置
    if (text.mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length() ==
            m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length())
        m_inputEdit->setCursorPosition(selcurPos);
    else if (text.mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length() >
             m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length())
        m_inputEdit->setCursorPosition(selcurPos + 1);
    else
        m_inputEdit->setCursorPosition(selcurPos - 1);

    //发送C/AC切换信号
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
}

/**
 * @brief 删除事件，选中删除
 */
void ExpressionBar::deleteText()
{
    QString text = m_inputEdit->text();
    int selcurPos = m_inputEdit->cursorPosition();
    int start = m_inputEdit->selectionStart();
    int length = m_inputEdit->selectionLength();
    text.remove(start, length);
    m_inputEdit->setText(text);
    selcurPos -= length;
    addUndo();
    m_isUndo = false;
    m_isResult = false;
    //设置删除后光标位置
    if (text.mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length() ==
            m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length())
        m_inputEdit->setCursorPosition(selcurPos);
    else if (text.mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length() >
             m_inputEdit->text().mid(0, selcurPos).remove(QRegExp("[＋－×÷,.%()E]")).length())
        m_inputEdit->setCursorPosition(selcurPos + 1);
    else
        m_inputEdit->setCursorPosition(selcurPos - 1);

    //发送C/AC切换信号
    if (m_inputEdit->text().isEmpty() && m_listModel->rowCount(QModelIndex()) != 0) {
        emit clearStateChanged(true);
        m_isAllClear = true;
    } else {
        emit clearStateChanged(false);
        m_isAllClear = false;
    }
}

/**
 * @brief ExpressionBar::handleTextChanged
 * 输入栏中为上一次计算的结果时，如果被修改，则输入数字时不进行整体的替换
 * @param text
 */
void ExpressionBar::handleTextChanged(const QString &text)
{
    Q_UNUSED(text);
    m_isAllClear = false;
    m_isContinue = true;
}

bool ExpressionBar::cursorPosAtEnd()
{
    return m_inputEdit->cursorPosition() == m_inputEdit->text().length();
}

/**
 * @brief ExpressionBar::formatExpression
 * 在进行计算前替换中文符号
 * @param text
 * 需要计算的内容
 * @return
 * 替换后的表达式
 */
QString ExpressionBar::formatExpression(const QString &text)
{
    return QString(text)
           .replace(QString::fromUtf8("＋"), "+")
           .replace(QString::fromUtf8("－"), "-")
           .replace(QString::fromUtf8("×"), "*")
           .replace(QString::fromUtf8("÷"), "/")
           .replace(QString::fromUtf8(","), "");
}

/**
 * @brief ExpressionBar::revisionResults
 * 点击历史记录后将表达式显示在输入栏
 * @param index
 * 点击的行号/索引
 */
void ExpressionBar::revisionResults(const QModelIndex &index)
{
    clearLinkageCache(m_inputEdit->text(), false);
    QString text = index.data(SimpleListModel::ExpressionRole).toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QStringList historic = text.split(QString("＝"), QString::SkipEmptyParts);
#else
    QStringList historic = text.split("＝", Qt::SkipEmptyParts);
#endif
    if (historic.size() != 2)
        return;
    QString expression = historic.at(0);
    m_hisRevision = index.row();
    m_inputEdit->setText(expression);
    m_Selected = m_hisRevision;
    m_isResult = false;
    // fix addundo for history revision
    m_isUndo = false;
    addUndo();

    emit clearStateChanged(false);
}

/**
 * @brief ExpressionBar::completedBracketsCalculation
 * 自动补全右括号，暂未用到
 * @param text
 * @return
 */
//QString ExpressionBar::completedBracketsCalculation(QString &text)
//{
//    int leftBrack = text.count("(");
//    int rightBrack = text.count(")");
//    QString newText = text;
//    if (leftBrack > rightBrack) {
//        for (int i = 0; i < leftBrack - rightBrack; i++)
//            newText += ")";
//    }

//    return newText;
//}

// edit 20200318 for fix cleanlinkcache
/**
 * @brief ExpressionBar::clearLinkageCache
 * 判断是否要清除最后一个联动项，如果联动项大于0,则判断表达式的第一位数字是否
 * 和上一列历史记录的结果相等，不等则清除联动
 * @param text
 * @param isequal
 * 区分是点击历史记录还是对表达式进行计算的事件
 */
void ExpressionBar::clearLinkageCache(const QString &text, bool isequal)
{
    if (m_hisLink.isEmpty())
        return;
    QString linkedExp = text.split("＝").first();
    if (m_hisLink.count() > 0 && isequal == true) {
        int length = m_hisLink.last().linkageValue.length();
        if (linkedExp.left(length) != m_hisLink.last().linkageValue ||
                (linkedExp.length() > length && !isOperator(linkedExp.at(length)) &&
                 (linkedExp.at(length) != "("))) {
            m_hisLink.removeLast();
            m_isLinked = false;
            m_listDelegate->removeHisLink();
            m_listDelegate->removeHisLinked();
            //            m_isResult = false;
        }
    } else {
        if (m_hisLink.last().linkedItem == -1) {
            m_hisLink.removeLast();
            m_isLinked = false;
            m_listDelegate->removeHisLink();
//            m_listDelegate->removeHisLinked();
            m_isResult = false;
        }
    }
}

/**
 * @brief ExpressionBar::settingLinkage
 * 点击等于号后的事件，进行历史记录联动的判断
 */
void ExpressionBar::settingLinkage()
{
//    m_evaluator->unsetVariable(QLatin1String("e"), true);
    const int hisRecision = m_hisRevision;
    if (hisRecision != -1) {
        // edit 20200403 删除错误表达式
        if (m_Selected != m_hisRevision && m_hisRevision < m_hisLink.count() &&
                m_hisLink[m_hisRevision].linkageValue.count(tr("Expression error")) > 0) {
            m_hisLink.removeAt(m_hisRevision);
            m_listModel->deleteItem(m_hisRevision);
        }
        for (int i = 0; i < m_hisLink.size(); i++) {
            if (m_hisLink[i].linkedItem == hisRecision) {
                if (cancelLink(i + 1) && i != 0)
                    --i;
                if (m_hisLink.size() == 0)
                    break;
            }
            if (m_hisLink[i].linkageTerm == hisRecision) {
                m_isLinked = true;
                enterEqualEvent();
                return;
            }
        }
        historicalLinkageIndex hisIndex;
        hisIndex.linkageTerm = hisRecision;
        // hisIndex.isLink = true;
        enterEqualEvent();
        QString exp =
            m_listModel->index(hisRecision).data(SimpleListModel::ExpressionRole).toString();
        hisIndex.linkageValue = exp.split("＝").last();
        m_hisLink.push_back(hisIndex);
    } else {
        // 20200402 需求3.2.1.6当输入的数字中有千位符，点击等号视为前后一致，不计入表达式
        //        if (!m_hisLink.isEmpty() && m_hisLink.last().linkedItem == -1)
        //            return;
        // judgeLinkageAgain();
        enterEqualEvent();
        m_isLinked = true;
        historicalLinkageIndex hisIndex;
        // hisIndex.isLink = true;
        hisIndex.linkageTerm = m_listModel->rowCount(QModelIndex()) - 1;
        QString exp = m_listModel->index(hisIndex.linkageTerm)
                      .data(SimpleListModel::ExpressionRole)
                      .toString();
        hisIndex.linkageValue = exp.split("＝").last();
        m_hisLink.push_back(hisIndex);
    }
    if (m_meanexp == false) {
        m_hisLink.pop_back();
        m_meanexp = true;
    } else {
        m_listDelegate->setHisLink(m_hisLink.last().linkageTerm);
    }
}

void ExpressionBar::initConnect()
{
    connect(m_listView, &SimpleListView::obtainingHistoricalSimple, this,
            &ExpressionBar::revisionResults);
    connect(m_listView, &SimpleListView::obtainingHistoricalSimple, m_inputEdit,
            &InputEdit::hisexpression);
    connect(m_inputEdit, &InputEdit::textChanged, this, &ExpressionBar::handleTextChanged);
    connect(m_inputEdit, &InputEdit::keyPress, this, &ExpressionBar::keyPress);
    connect(m_inputEdit, &InputEdit::equal, this, &ExpressionBar::enterEqualEvent);
    connect(m_inputEdit, &InputEdit::cut, this, &ExpressionBar::shear);
    connect(m_inputEdit, &InputEdit::copy, this, &ExpressionBar::copyResultToClipboard);
    connect(m_inputEdit, &InputEdit::paste, this, &ExpressionBar::copyClipboard2Result);
    connect(m_inputEdit, &InputEdit::deleteText, this, &ExpressionBar::deleteText);
    connect(m_inputEdit, &InputEdit::selectAllText, this, &ExpressionBar::allElection);
    connect(m_inputEdit, &InputEdit::undo, this, &ExpressionBar::Undo);
    connect(m_inputEdit, &InputEdit::redo, this, &ExpressionBar::Redo);
    connect(m_inputEdit, &InputEdit::setResult, this, &ExpressionBar::setResultFalse);
    connect(m_inputEdit, &InputEdit::separateChange, this, &ExpressionBar::onSeparateChange);
}

/**
 * @brief ExpressionBar::symbolComplement
 * 当计算((()))这种超过3个括号嵌套的表达式时有问题，手动补乘号进行规避
 * @param exp
 * @return
 */
QString ExpressionBar::symbolComplement(const QString exp)
{
    QString text = exp;
    int index = text.indexOf("(", 0);
    while (index != -1) {
        if (index >= 1 && (text.at(index - 1).isNumber() || text.mid(index - 3, 3) == "ans")) {
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
    return text;
}

/**
 * @brief 粘贴容错（20200716，粘贴时不再使用此函数）
 * @param exp 复制的内容
 * @return 粘贴后的text
 */
//QString ExpressionBar::pasteFaultTolerance(QString exp)
//{
//    exp = m_inputEdit->text().insert(m_inputEdit->cursorPosition(), exp);
//    exp = pointFaultTolerance(exp); //避免粘贴后的text有两个.
//    if (exp[0] == ",")
//        exp.remove(0, 1);
//    for (int i = 0; i < exp.size(); ++i) {
//        while (exp[i].isNumber()) {
//            if (exp[i] == "0"
//                    && exp[i + 1] != "."
//                    && (i == 0 || (exp[i - 1] != "," && exp[i - 1] != "." && !exp[i - 1].isNumber()))
//                    && (i == 0 || !exp[i - 1].isNumber())
//                    && (exp.size() == 1 || exp[i + 1].isNumber())) {
//                exp.remove(i, 1); //0的容错处理，例:将0123的0去除 //在输入符号时也会进行此操作
//                --i;
//            }
//            ++i;
//        }
//        if (exp[i] == "." && (i == 0 || !exp[i - 1].isNumber())) {
//            exp.insert(i, "0"); //补0操作，例:1+.2->1+0.2 //小数点容错处理会进行此操作
//            ++i;
//        }
//    }
//    return exp;
//}

/**
 * @brief ExpressionBar::pointFaultTolerance
 * 符号的容错处理
 * @param text
 * @return
 */
QString ExpressionBar::pointFaultTolerance(const QString &text)
{
    QString oldText = text;
    // QString reformatStr = Utils::reformatSeparators(QString(text).remove(','));
    QString reformatStr = oldText.replace('+', QString::fromUtf8("＋"))
                          .replace('-', QString::fromUtf8("－"))
                          .replace("_", QString::fromUtf8("－"))
                          .replace('*', QString::fromUtf8("×"))
                          .replace(QString::fromUtf8("＊"), QString::fromUtf8("×"))
                          .replace('/', QString::fromUtf8("÷"))
                          .replace('x', QString::fromUtf8("×"))
                          .replace('X', QString::fromUtf8("×"))
                          .replace(QString::fromUtf8("（"), "(")
                          .replace(QString::fromUtf8("）"), ")")
                          .replace(QString::fromUtf8("——"), QString::fromUtf8("－"))
                          .replace(QString::fromUtf8("％"), "%");
    QStringList list = reformatStr.split(QRegExp("[＋－×÷(]")); //20200717去掉),否则下方)小数点容错无法进入; //此处可不考虑多符号问题
    QStringList symbollist;
    for (int i = 0; i < reformatStr.size(); ++i) {
        if (QRegExp("[＋－×÷(]").exactMatch(reformatStr.at(i)))
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
        if (reformatStr[i] == "." && (i == 0 || !reformatStr[i - 1].isNumber())) {
            reformatStr.insert(i, "0"); //补0操作，例:1+.2->1+0.2
            ++i;
        }
    }

    return reformatStr;
}

void ExpressionBar::expressionCheck()
{
    QString exp = m_inputEdit->text();
    int cur = m_inputEdit->cursorPosition();
    int n = m_inputEdit->cursorPosition();
    int length = exp.length() - n;//原表达式光标右长度
    int sum = 0;

    for (int i = 0; i < exp.size(); ++i) {
        if (exp[i] == ",") {
            exp.remove(i, 1);
            --i;
            if (i + 1 < cur) {
                --cur;
            }
        }
    }
    for (int i = 0; i < exp.size(); ++i) {
        int num = 0;
        int a = 0;
        while (exp[i].isNumber()) {
            // fix for delete 0 behind "."
            if (exp[i] == "0" && exp[i + 1] != "." && (i == 0 || exp[i - 1] != ".") &&
                    (i == 0 || !exp[i - 1].isNumber()) && (exp.size() == 1 || exp[i + 1].isNumber())) {
                exp.remove(i, 1);
                --i;
                if (i + 1 < cur)
                    --cur;
                else {
                    a++;//光标后每个数字段被去掉0的个数
                }
            }
            ++i;
            if (i >= cur)
                num++;//光标后每个数字段长度
        }
        sum = sum + ((num - 1) / 3 - (num - a - 1) / 3) + a;
        if (exp[i] == "." && (i == 0 || !exp[i - 1].isNumber())) {
            exp.insert(i, "0");
            ++i;
            if (i < cur)
                ++cur;
        }
    }
    m_inputEdit->setText(exp);
//    m_inputEdit->setCursorPosition(cur + separator);
    m_inputEdit->setCursorPosition(m_inputEdit->text().length() - (length - sum)); //20200525数字前有0时容错删除0光标错位
}

/**
 * @brief ExpressionBar::Undo
 * 撤销
 */
void ExpressionBar::Undo()
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

/**
 * @brief ExpressionBar::addUndo
 * 将当前的内容存入撤销的容器中
 */
void ExpressionBar::addUndo()
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

void ExpressionBar::Redo()
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

/**
 * @brief ExpressionBar::initTheme
 * 向历史记录的listview中传递当前应用主题
 * @param type
 * 主题类型
 */
void ExpressionBar::initTheme(int type)
{
    //edit for bug-21476
    int typeIn = type;
    if (typeIn == 0) {
        typeIn = DGuiApplicationHelper::instance()->themeType();
    }
    m_listDelegate->setThemeType(typeIn);
    m_inputEdit->themetypechanged(typeIn);
}

//void ExpressionBar::setSelection()
//{
//    SSelection select = m_inputEdit->getSelection();
//    if (select.selected.isEmpty())
//        return;
//    if (m_inputEdit->text() == select.clearText)
//        m_inputEdit->setText(select.oldText);
//    select.selected = "";
//}

//void ExpressionBar::getSelection()
//{
//    if (!m_inputEdit->selectedText().isEmpty()) {
//        int start = m_inputEdit->selectionStart();
//        QString exp = m_inputEdit->text();
//        exp = exp.remove(start, m_inputEdit->selectionLength());
//        m_inputEdit->setText(exp);
//        m_inputEdit->setCursorPosition(start);
//    }
//}

void ExpressionBar::setResultFalse()
{
    m_isResult = false;
}

/**
 * @brief ExpressionBar::replaceSelection
 * 清除选中的内容并设置光标位置
 * @param text
 */
void ExpressionBar::replaceSelection(QString text)
{
    QString seloldtext = text;
    SSelection selection = m_inputEdit->getSelection();
    int selcurPos = m_inputEdit->cursorPosition();
    if (selection.selected != "") {
        //删除选中部分
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

bool ExpressionBar::cancelLink(int index)
{
    bool isRemove = false;
    for (int i = 0; i < index; ++i) {
        if (m_hisLink[i].linkedItem == m_hisRevision) {
            QString exp = m_inputEdit->text();
            exp = exp.replace(",", "");
            QStringList list = exp.split(QRegExp("[＋－×÷()]"));
            if (exp[0] == "－")
                list[0] = "－" + list[1];
            QString linkvalue = m_hisLink[i].linkageValue;
            linkvalue = linkvalue.replace(",", "");
            //            if (list.at(0) != m_hisLink[i].linkageValue) {
            if (list.at(0) != linkvalue) {
                // m_listDelegate->removeLine(m_hisLink[i].linkageTerm,
                // m_hisLink[i].linkedItem);
                m_listDelegate->removeLine(i);
                m_hisLink.remove(i);
                isRemove = true;
            }
        }
    }
    return isRemove;
}

//void ExpressionBar::settingLinkage(const QModelIndex &index)
//{
//    int row = index.row();
//    for (int i = 0; i < m_hisLink.size(); i++) {
//        if (m_hisLink[i].linkageTerm == row) {
//            m_hisLink[i].isLink = true;
//            m_inputEdit->clear();
//        }
//    }
//}

bool ExpressionBar::isOperator(const QString &text)
{
    if (text == QString::fromUtf8("＋") || text == QString::fromUtf8("－") ||
            text == QString::fromUtf8("×") || text == QString::fromUtf8("÷")) {
        return true;
    } else {
        return false;
    }
}

void ExpressionBar::moveLeft()
{
    m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() - 1);
    m_inputEdit->setFocus();
}

void ExpressionBar::moveRight()
{
    m_inputEdit->setCursorPosition(m_inputEdit->cursorPosition() + 1);
    m_inputEdit->setFocus();
}
//20200414 bug20294鼠标点击取消focus
InputEdit *ExpressionBar::getInputEdit()
{
    return m_inputEdit;
}

/**
 * @brief ExpressionBar::separateChange
 * 数字间隔位数发生改变
 */
void ExpressionBar::onSeparateChange()
{
   m_listModel->updataOfSeparate();
}
