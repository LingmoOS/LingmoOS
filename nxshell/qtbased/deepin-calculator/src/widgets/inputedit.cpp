// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inputedit.h"

#include "../../3rdparty/core/settings.h"
#include "../../3rdparty/math/floatconfig.h"
#include "../utils.h"
#include "../dsettings.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <DMenu>

const QString AtoF = "ABCDEF";

InputEdit::InputEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_ans(0)
    , m_ansStartPos(0)
    , m_ansLength(0)
    , m_ansVaild(false)
    , m_currentInAns(false)
    , m_currentOnAnsLeft(false)
    , m_oldText("")
    , m_lastPos(0)
    , m_memoryans(0)
{
    m_evaluator = Evaluator::instance();
    setAttribute(Qt::WA_InputMethodEnabled, false); //禁止中文输入法
    setAttribute(Qt::WA_TranslucentBackground); //设置窗口透明
    setFocusPolicy(Qt::StrongFocus); //接受Tab键和鼠标单击做焦点
    autoZoomFontSize();
    initAction();

    this->setFrame(false);
    this->setClearButtonEnabled(false); //不显示清除按钮
    this->setContextMenuPolicy(Qt::CustomContextMenu); //信号(customContextMenuRequested)槽完成右击菜单事件

    connect(this, &QLineEdit::textChanged, this, &InputEdit::handleTextChanged);
    connect(this, &QLineEdit::cursorPositionChanged, this, &InputEdit::handleCursorPositionChanged);
    connect(this, &QLineEdit::customContextMenuRequested, this, &InputEdit::showTextEditMenu); //右键菜单信号槽
    connect(this, &QLineEdit::selectionChanged, this, &InputEdit::selectionChangedSlot);
    connect(this, &QLineEdit::selectionChanged, [ = ] {
        int pos = this->cursorPosition();
        this->cursorPositionChanged(pos, pos);
    });

    connect(this, &QLineEdit::textChanged, this, &InputEdit::isExpressionEmpty);
    connect(this, &QLineEdit::textChanged, this, &InputEdit::getCurrentAns);

    connect(this, &InputEdit::swietThreeSeparate, this, &InputEdit::onSwietThreeSeparateClicked);
    connect(this, &InputEdit::swietFourSeparate, this, &InputEdit::onswietFourSeparateClicked);

    m_funclist = {"and",  "not", "xor", "nand", "nor", "mod", "or",
                  "shl", "shr", "sal", "sar", "rol", "ror", "rcl", "rcr"
                 };
}

InputEdit::~InputEdit() {}

/**
 * @brief 当对超过17位的数进行百分号处理时，保留超过精度的部分
 * @return precentans
 * 由于百分号逻辑改变，此函数暂未使用
 */
//QString InputEdit::expressionPercent(QString &str)
//{
//    QString t = str;
//    bool longnumber = false;

//    QString ans = DMath::format(m_ans, Quantity::Format::Fixed() + Quantity::Format::Precision(DECPRECISION));
//    m_evaluator->setVariable(QLatin1String("precentans"), m_ans, Variable::BuiltIn); //把ans当作precentans保存
//    /*
//     * 判断ans是否是长数字
//     */
//    if (ans.length() > 17) {
//        for (int i = 17; i < ans.length(); i++) {
//            if (ans.at(i) != "0") {
//                longnumber = true;
//                break;
//            }
//        }
//    }
//    /*
//     * 是长数字时返回ans
//     */
//    if (longnumber && m_lastPos == m_ansStartPos + m_ansLength + 1) {
//        t = QLatin1String("precentans") + str.back();
//        m_ispercentanswer = true;
//    }
//    //    if (m_ansVaild) {
//    //        QString ans = DMath::format(m_ans, Quantity::Format::Precision(DECPRECISION));
//    //        t.remove(m_ansStartPos, m_ansLength);
//    //        t.insert(m_ansStartPos, ans);
//    //    }
//    return t;
//}

/**
 * @brief 在点击等于号时使用此函数判断输入框中是否存在上一次结果的长数字
 * return 将有上次结果长数字的部分替换为lastans返回
 */
QString InputEdit::expressionText()
{
    QString t = text();
    //    t.remove(",");
    //edit for bug-19653 20200416  当数字长度超过精度范围时，保留小数点最后的数。
    bool longnumber = false;

    QString ans = DMath::format(m_ans, Quantity::Format::Fixed() + Quantity::Format::Precision(DECPRECISION));
    m_evaluator->setVariable(QLatin1String("lastans"), m_ans, Variable::BuiltIn);
    /*
     * 判断ans是否是长数字
     */
    if (ans.length() > 17) {
        for (int i = 17; i < ans.length(); i++) {
            if (ans.at(i) != "0") {
                longnumber = true;
                break;
            }
        }
    }
    /*
     * 是长数字并且输入框中包含ans时输入框中的可见ans替换为lastans按长数字计算
     */
    //20200619输入框是否包含ans，长数字等于后撤销退格一位按等于计算错误;不撤销退格一位按等于号计算错误
    if (m_ansVaild && longnumber && text().contains(m_strans)) {
        t.remove(m_ansStartPos, m_ansLength);
        if (m_ansLength != 0) {
            t.insert(m_ansStartPos, QLatin1String("lastans"));
        }
    }
    //    if (m_ansVaild) {
    //        QString ans = DMath::format(m_ans, Quantity::Format::Precision(DECPRECISION));
    //        t.remove(m_ansStartPos, m_ansLength);
    //        t.insert(m_ansStartPos, ans);
    //    }
    //    qDebug() << t;
    return t;
}

/**
 * @brief 点击等于号可以出正确结果时settext,且更新私有参数有关ans的值
 */
void InputEdit::setAnswer(const QString &str, const Quantity &ans)
{
    m_ans = ans;
    m_ansStartPos = 0;
    m_ansLength = str.length();
    m_oldText = "";
    setText(str);
    m_strans = str;
    m_strans = m_strans.replace('+', QString::fromUtf8("＋"))
               .replace('-', QString::fromUtf8("－"))
               .replace('*', QString::fromUtf8("×"))
               .replace(QString::fromUtf8("（"), "(")
               .replace(QString::fromUtf8("）"), ")");
}

/**
 * @brief InputEdit::setPercentAnswer
 * @param str1-完整表达式
 * @param str2-百分比计算结果
 * @param ans-百分比计算结果
 * @param Pos-光标位置
 * （由于百分号逻辑改变，暂未使用）
 */
//void InputEdit::setPercentAnswer(const QString &str1, const QString &str2, const Quantity &ans,
//                                 const int &Pos)
//{
//    if (m_ispercentanswer) {
//        m_ans = ans;
//        m_ansStartPos = Pos + ((Pos == 0) ? 0 : 1); //edit 20200416
//        m_ansLength = str2.length();
//        m_oldText = "";
//        setText(str1);
//        int ansEnd = m_ansStartPos + m_ansLength;
//        while (ansEnd > str1.length()) {
//            --ansEnd;
//        }
//        m_ansVaild = /*m_ansLength > 10 &&*/ m_ansLength > 0 && (m_ansStartPos == 0 || !str1[m_ansStartPos - 1].isDigit()) &&
//                                             (ansEnd == str1.length() || !str1[ansEnd].isDigit());
//    }
//    m_ispercentanswer = false;
//}

/**
 * @brief 清空输入框且更新ans参数
 */
void InputEdit::clear()
{
    m_ansLength = 0;
    setText("");
}

/**
 * @brief 由undo列表设置右键菜单中撤销状态
 */
void InputEdit::setUndoAction(bool state)
{
    m_undo->setEnabled(state);
}

/**
 * @brief 由redo列表设置右键菜单中撤销状态
 */
void InputEdit::setRedoAction(bool state)
{
    m_redo->setEnabled(state);
}

/**
 * @brief 跳过QLineEdit的物理键盘事件，由modul中handleEditKeyPress触发
 */
void InputEdit::keyPressEvent(QKeyEvent *e)
{
    if (hasFocus() && e->modifiers() == Qt::AltModifier && e->key() == Qt::Key_M) {
        showTextEditMenuByAltM();
    } else
        Q_EMIT keyPress(e);
    return;
}

/**
 * @brief 鼠标双击事件
 */
void InputEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    //fix bug-47162保持触摸屏双击输入框与其他应用一致
    QLineEdit::mouseDoubleClickEvent(e);
    selectAll();
    m_selected.selected = text();
    /*if (e->button() == Qt::LeftButton) {
        int position = this->cursorPositionAt(e->pos());
        int posBegin = findWordBeginPosition(position);
        int posEnd = findWordEndPosition(position);
        this->setSelection(posBegin, posEnd - posBegin + 1);
    }*/
}

void InputEdit::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        setFocus();
        emit setResult(); //expression中m_isResult置为false
        //        qDebug() << m_selected.selected;
    }
    QLineEdit::mousePressEvent(e);
}

void InputEdit::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        emit paste();
        return;
    }
    QLineEdit::mouseReleaseEvent(event);
}

/**
 * @brief 右键菜单action初始化
 */
void InputEdit::initAction()
{
    //fix bug-47321
    m_undo = new QAction(tr("Undo"), this);
    m_redo = new QAction(tr("Redo"), this);
    m_cut = new QAction(tr("Cut"), this);
    m_copy = new QAction(tr("Copy"), this);
    m_paste = new QAction(tr("Paste"), this);
    m_delete = new QAction(tr("Delete"), this);
    m_select = new QAction(tr("Select All"), this);
    m_threeSeparate = new QAction(tr("Use thousands separator"), this);
    m_fourSeparate = new QAction(tr("Use ten-thousands separator"), this);

    connect(m_undo, &QAction::triggered, this, &InputEdit::undo);
    connect(m_redo, &QAction::triggered, this, &InputEdit::redo);
    connect(m_cut, &QAction::triggered, this, &InputEdit::cut);
    connect(m_copy, &QAction::triggered, this, &InputEdit::copy);
    connect(m_paste, &QAction::triggered, this, &InputEdit::paste);
    connect(m_delete, &QAction::triggered, this, &InputEdit::deleteText);
    connect(m_select, &QAction::triggered, this, &InputEdit::selectAllText);
    connect(m_threeSeparate, &QAction::triggered, this, &InputEdit::swietThreeSeparate);
    connect(m_fourSeparate, &QAction::triggered, this, &InputEdit::swietFourSeparate);

    m_undo->setEnabled(false);
    m_redo->setEnabled(false);
    m_cut->setEnabled(false);
    m_copy->setEnabled(false);
    m_delete->setEnabled(false);
    m_select->setEnabled(false);
}

/**
 * 暂未使用
 */
//bool InputEdit::isSymbolCategoryChanged(int pos1, int pos2)
//{
//    QString str = text();
//    QChar::Category category1 = str.at(pos1).category();
//    QChar::Category category2 = str.at(pos2).category();

//    if (category1 == QChar::Number_DecimalDigit || category1 == QChar::Punctuation_Other) {
//        if (category2 == QChar::Number_DecimalDigit || category2 == QChar::Punctuation_Other) {
//            return false;
//        }
//    }

//    return true;
//}

/**
 * 暂未使用
 */
//int InputEdit::findWordBeginPosition(int pos)
//{
//    QString str = text();

//    if (0 >= pos) {
//        return 0;
//    }
//    while (pos > 0) {
//        pos--;
//        if (isSymbolCategoryChanged(pos, pos + 1)) {
//            return pos + 1;
//        }
//    }

//    return 0;
//}

/**
 * 暂未使用
 */
//int InputEdit::findWordEndPosition(int pos)
//{
//    QString str = text();

//    if (pos >= str.length()) {
//        return str.length() - 1;
//    }

//    while (pos < str.length() - 1) {
//        pos++;
//        if (isSymbolCategoryChanged(pos, pos - 1)) {
//            return pos - 1;
//        }
//    }

//    return str.length() - 1;
//}

/**
 * @brief InputEdit::输入框字号变化
 */
void InputEdit::autoZoomFontSize()
{
    QFont font;

    // the maximum font is 30, minimum font is 15.
    for (int i = 30; i > 16; --i) {
        font.setPixelSize(i);

        QFontMetrics fm(font);
        int fontWidth = fm.horizontalAdvance(text());
        int editWidth = width() - 45;

        if (fontWidth < editWidth)
            break;
    }

    setFont(font);
}

void InputEdit::themetypechanged(int type)
{
    DPalette pl = this->palette();
    //itemwidget字体颜色设置
    if (type == 1) {
        pl.setColor(DPalette::Text, QColor("#303030"));
    } else {
        pl.setColor(DPalette::Text, QColor("#B4B4B4"));
    }
    pl.setColor(DPalette::Button, Qt::transparent); //inputedit背景色
    pl.setColor(DPalette::Highlight, Qt::transparent); //边框高亮色
    pl.setColor(DPalette::HighlightedText, Qt::blue); //全选字体高亮色
    this->setPalette(pl);
}

/**
 * @brief InputEdit::valueChangeFromProSyskeypad
 * @param num:二进制数字
 * 点击位键盘时，对应修改光标位置的数字
 */
void InputEdit::valueChangeFromProSyskeypad(const QString num)
{
//    qDebug() << "change";
    QString text = this->text();
    int pos = this->cursorPosition();
    int numstart = pos, numend = pos;
    QString number = formatBinaryNumber(num);
//    qDebug() << "changenumber:" << number;
    if (text == QString()) {
        text = number;
    } else {
        while (numstart != 0 && (isNumber(text.at(numstart - 1)) ||
                                 (text.length() > 1 && numstart == 1 && isNumber(text.at(1)) && text.at(0) == QString::fromUtf8("－")))) {
            numstart--;
        }
        while (numend < text.length() && isNumber(text.at(numend))) {
            numend++;
        }
        if (numstart > 0 && !isNumber(text.at(numstart - 1)) && number.at(0) == "-") {
            text.remove(numstart, numend - numstart).insert(numstart, "(" + number);
            numstart += 2;
        } else
            text.remove(numstart, numend - numstart).insert(numstart, number);
    }
    this->setText(text);
    //重新找到numend
    numend = numstart < 0 ? 0 : numstart;
    while (numend < this->text().length() && (isNumber(this->text().at(numend)) ||
                                              ((numend == 0) && isNumber(text.at(1)) && text.at(0) == "-"))) {
        numend++;
    }
    this->setCursorPosition(numend);
}

/**
 * @brief 输入框textchange时触发
 */
void InputEdit::handleTextChanged(const QString &text)
{
    if (m_currentInAns) {
        m_ansLength = 0; //光标在ans中间且text改变，清空ans
    } else if (m_currentOnAnsLeft && m_oldText.length() != 0) {
        int textLength = text.length();
        int oldTextLength = m_oldText.length();
//        int minValue = std::min(textLength, oldTextLength);

//        int i = 1;
//        for (; i < minValue && text[textLength - i] == m_oldText[oldTextLength - i]; ++i)
//            ; //i=diff?不知为何又用到下侧int计算　i无用

//        int diff = (textLength - i) - (oldTextLength - i);
        int diff = textLength - oldTextLength;
        m_ansStartPos += diff;
    }
    if (text.indexOf("=") != -1) {
        QString exp = text;
        exp.remove(text.indexOf("="), 1);
        setText(exp);
        Q_EMIT equal(); //当前外界键盘及计算器键盘中=均不走此；猜测为了复制粘贴等于式；当前粘贴会去除=，也不会走此；暂不删除
        return;
    }

    int ansEnd = m_ansStartPos + m_ansLength;

    m_oldText = text;
    while (ansEnd > text.length()) {
        --ansEnd;
    }

    //ans是否有效；判断条件：ans未被改变,ans左侧右侧均不是小数点
    m_ansVaild = /*m_ansLength > 10 &&*/ m_ansLength > 0 && (m_ansStartPos == 0 || !text[m_ansStartPos - 1].isDigit()) &&
                                         (ansEnd == text.length() || !text[ansEnd].isDigit());
    int oldPosition = this->cursorPosition();
    QString reformatStr = QString();
    if (Settings::instance()->programmerBase == 0)
        reformatStr = Utils::reformatSeparators(QString(text).remove(','));
    else
        reformatStr = Utils::reformatSeparatorsPro(QString(text).remove(',').remove(" "), Settings::instance()->programmerBase);
    reformatStr = reformatStr.replace('+', QString::fromUtf8("＋"))
                  .replace('-', QString::fromUtf8("－"))
                  .replace("_", QString::fromUtf8("－"))
                  .replace('*', QString::fromUtf8("×"))
                  .replace(QString::fromUtf8("＊"), QString::fromUtf8("×"))
//                  .replace('/', QString::fromUtf8("÷"))
//                  .replace('x', QString::fromUtf8("×"))
                  .replace('X', QString::fromUtf8("×"))
                  .replace(QString::fromUtf8("（"), "(")
                  .replace(QString::fromUtf8("）"), ")")
                  .replace(QString::fromUtf8("——"), QString::fromUtf8("－"))
                  .replace(QString::fromUtf8("％"), "%");

    multipleArithmetic(reformatStr);
//    reformatStr.remove(QRegExp("[^0-9＋－×÷,.%()e]"));
    // reformatStr = pointFaultTolerance(reformatStr);
    //    reformatStr = symbolFaultTolerance(reformatStr);
    setText(reformatStr);
    autoZoomFontSize();

    // reformat text.
    int oldLength = text.length();
    int newLength = reformatStr.length();
    int pos;
    if (newLength > oldLength)
        pos = oldPosition + (newLength - oldLength);
    else
        pos = oldPosition;
    if (pos > newLength)
        pos = newLength;
    this->setCursorPosition(pos);

    m_selected.oldText = this->text(); //选中输入情况下清空被选部分
    m_selected.selected = selectedText();
    m_selected.curpos = selectionStart() < selectionEnd() ? selectionStart() : selectionEnd();
}

/**
 * @brief InputEdit::radixChanged
 * 当进制表切换时，输入栏中对应的值需要同步切换
 */
void InputEdit::radixChanged(int baseori, int basedest)
{
    this->setText(scanAndExec(baseori, basedest));
}

QString InputEdit::scanAndExec(int baseori, int basedest)
{
    m_numvec.clear();
    m_opvec.clear();
    m_textorder = QString();
    QString oldtext = this->text();
    oldtext.remove("，").remove(" ");
    for (int i = 0; i < oldtext.length();) {
        if (isNumber(oldtext.at(i))) {
            for (int j = 0; j < oldtext.length() - i; j++) {
                if (i + j == oldtext.length() - 1) {
                    if (isNumber(oldtext.at(i + j))) {
                        m_numvec.append(oldtext.mid(i, j + 1));
                        m_textorder += "0";
                        i += j + 1;
                    } else {
                        m_numvec.append(oldtext.mid(i, j));
                        m_textorder += "0";
                        i += j;
                    }
                    break;
                }
                if (!isNumber(oldtext.at(i + j))) {
                    m_numvec.append(oldtext.mid(i, j));
                    m_textorder += "0";
                    i += j;
                    break;
                }
            }
        } else {
            if (oldtext.at(i).isLower()) {
                if (oldtext.at(i) == 'n' && oldtext.at(i + 1) == 'a') {
                    m_opvec.append(oldtext.mid(i, 4));
                    m_textorder += "1";
                    i += 4;
                } else if (oldtext.at(i) == 'o') {
                    m_opvec.append(oldtext.mid(i, 2));
                    m_textorder += "1";
                    i += 2;
                } else {
                    m_opvec.append(oldtext.mid(i, 3));
                    m_textorder += "1";
                    i += 3;
                }
            } else if ((i == 0 || !isNumber(oldtext.at(i - 1))) && oldtext.at(i) == QString::fromUtf8("－")
                       && oldtext.length() > i + 1 && isNumber(oldtext.at(i + 1))) {
                i++;
                for (int j = 0; j < oldtext.length() - i; j++) {
                    if (!isNumber(oldtext.at(i + j))) {
                        m_numvec.append(oldtext.mid(i - 1, j + 1));
                        m_textorder += "0";
                        i += j;
                        break;
                    }
                    if (i + j == oldtext.length() - 1) {
                        m_numvec.append(oldtext.mid(i - 1, j + 2));
                        m_textorder += "0";
                        i += j + 1;
                        break;
                    }
                }
            } else {
                m_opvec.append(oldtext.at(i));
                m_textorder += "1";
                i++;
            }
        }
    }
    for (int i = 0; i < m_numvec.size(); i++) {
        QString num = formatExpression(baseori, m_numvec.at(i));
        Quantity ans(HNumber(num.toLatin1().data()));
        switch (basedest) {
        case 16:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
            break;
        case 8:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Octal()).remove("0o");
            break;
        case 2:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
            break;
        default:
            num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
            break;
        }
        m_numvec.replace(i, num);
    }
    QString newtext = QString();
    for (int i = 0; i < m_textorder.length(); i++) {
        if (m_textorder.at(i) == "0") {
            newtext.append(m_numvec.first());
            m_numvec.pop_front();
        } else {
            newtext.append(m_opvec.first());
            m_opvec.pop_front();
        }
    }
    return newtext;
}

/**
 * @brief 小数点容错处理，防止多小数点
 */
QString InputEdit::pointFaultTolerance(const QString &text)
{
    QString exp = text;
    QString oldText = text;
    QStringList list = exp.split(QRegExp("[＋－×÷/()]"));
    for (int i = 0; i < list.size(); ++i) {
        QString item = list[i];
        int firstPoint = item.indexOf(".");
        if (firstPoint == -1)
            continue;
        if (firstPoint == 0) {
            item.insert(firstPoint, "0"); //小数点在第一位补0操作，此处存在若无多小数点情况不会补零，虚在其余地方进行操作
            ++firstPoint;
            // oldText.replace(list[i], item);
        } else {
            if (item.at(firstPoint - 1) == ")" || item.at(firstPoint - 1) == "%") {
                item.remove(firstPoint, 1); //原定义)及%右侧不能添加.，现在小数点输入事件中进行过补0操作,不会进入此判断
                oldText.replace(list[i], item);
            }
        }
        if (item.count(".") > 1) { //多小数点只保留第一位小数点
            int cur = cursorPosition();
            item.remove(".");
            item.insert(firstPoint, ".");
            oldText.replace(list[i], item);
            setCursorPosition(cur);
        }
    }
    return oldText;
}

/**
 * @brief 保证数字中间的符号只有一个
 * 防止输入栏中只有符号可输入*+
 * e+/e-和数字间不可以插入非数字
 * 去除从e到下一个运算符中的小数点
 */
QString InputEdit::symbolFaultTolerance(const QString &text)
{
    if (text.isEmpty())
        return text;
    QString exp = text;
    QString newText;
    /*if (isSymbol(exp.at(0))) {
        if (exp.at(0) != QString::fromUtf8("－"))
            return "";
        else {
            for (int i = 0; i < exp.length(); ++i) {}
        }
    }*/
    QStringList symbolList;
    for (int i = 0; i < exp.length(); ++i) {
        /*if (isSymbol(exp.at(i)) && i < exp.length() - 2) {
            while (isSymbol(exp.at(i + 1)) && i < exp.length() - 2) {
                ++i;
            }
            newText.append(exp.at(i));
        } else {
            newText.append(exp.at(i));
        }*/
        if (!isSymbol(exp.at(i))) {
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
    //edit 20200526 for bug-28491
    int expPos = newText.indexOf("E");
    if (expPos > 0) {
        //e后非＋／－
        if (newText.length() > expPos + 1 && newText.at(expPos + 1) != QString::fromUtf8("－") && newText.at(expPos + 1) != QString::fromUtf8("＋")
                && newText.at(expPos + 1) != "-" && newText.at(expPos + 1) != "+") {
            while (newText.length() > expPos + 1 && (newText.at(expPos + 1) == "(" || newText.at(expPos + 1) == ")")) {
                newText.remove(expPos + 1, 1); //避免e后可输入()情况
            }
            return newText;
        }
        //e+/e-后有数字
        if (newText.length() > expPos + 2) {
            while (newText.length() > expPos + 2 && newText.at(expPos + 2).isNumber() == false) {
                newText.remove(expPos + 2, 1); //e+/e-和数字间不可以插入非数字
            }
            int nextsymbolpos = newText.indexOf(QRegExp("[＋－×÷/()]"), expPos + 2); //e+/e-右侧第一个符号
            for (int i = expPos; i < (nextsymbolpos == -1 ? newText.length() : nextsymbolpos); i++) {
                if (newText.at(i) == "." || newText.at(i) == QString::fromUtf8("。"))
                    newText.remove(i, 1); //去除从e到下一个运算符中的小数点
            }
        }
    }
    return newText;
}

/**
 * @brief 判断是否是加减乘除
 */
bool InputEdit::isSymbol(const QString &text)
{
    if (text == QString::fromUtf8("＋"))
        return true;
    else if (text == QString::fromUtf8("－"))
        return true;
    else if (text == QString::fromUtf8("×"))
        return true;
    else if (text == QString::fromUtf8("÷"))
        return true;
    else
        return false;
}

/**
 * @brief 判断光标在ans哪里
 */
void InputEdit::handleCursorPositionChanged(int oldPos, int newPos)
{
    Q_UNUSED(oldPos);

    int ansEnd = m_ansStartPos + m_ansLength;
    int selectStart = m_selected.curpos; //选中输入后选中部分被清空　不可用selectStart()
    int selectEnd = selectStart + m_selected.selected.length();

    if (newPos > m_ansStartPos && newPos < ansEnd) {
        m_currentInAns = true; //当前光标在ans开始后结束前
    } else if ((selectStart >= m_ansStartPos && selectStart < ansEnd && m_selected.selected.length() > 0) ||
               (selectEnd > m_ansStartPos && selectEnd <= ansEnd) ||
               (selectStart < m_ansStartPos && selectEnd > ansEnd)) { //选中->输入->selection为空->cursprchanged导致此处不会进入;删除hasSelectedText();
        m_currentInAns = true; //选中区与ans有交集
    } else if (newPos <= m_ansStartPos) {
        m_currentInAns = false;
        m_currentOnAnsLeft = true; //光标在ans左侧，保存以去计算ans开始位置
    } else {
        m_currentInAns = false;
        m_currentOnAnsLeft = false;
    }
    m_lastPos = newPos;
    getCurrentCursorPositionNumber(newPos);
}

/**
 * @brief 括号补全，暂未使用
 */
void InputEdit::BracketCompletion(QKeyEvent *e)
{
    Q_UNUSED(e);
    QString oldText = text();
    int curs = this->cursorPosition();
    int right = oldText.length() - curs;
    int leftLeftParen = oldText.left(curs).count("(");
    int leftRightParen = oldText.left(curs).count(")");
    int rightLeftParen = oldText.right(right).count("(");
    int rightrightParen = oldText.right(right).count(")");
    //左右括号总数是否相等
    if (oldText.count("(") != oldText.count(")")) {
        //光标左侧左括号大于右括号
        if (leftLeftParen > leftRightParen) {
            if (leftLeftParen - leftRightParen + (rightLeftParen - rightrightParen) > 0) {
                oldText.insert(curs, ")");
            } else if (leftLeftParen - leftRightParen + (rightLeftParen - rightrightParen) < 0) {
                oldText.insert(curs, "(");
            } else {
                oldText.insert(curs, "()");
            }
            //如果左侧左括号小于等于左侧右括号
        } else {
            //如果右侧左括号小于右括号
            if (rightLeftParen < rightrightParen) {
                oldText.insert(curs, "(");
            } else {
                oldText.insert(curs, "()");
            }
        }
        //相等则输入一对括号
    } else {
        oldText.insert(curs, "()");
    }
    this->setCursorPosition(curs);
    setText(oldText);
}

/**
 * @brief 猜测防止异常情况\n出现在)及%后补*,当前text中不会出现\n,故此函数无效
 */
void InputEdit::multipleArithmetic(QString &text)
{
    int index = text.indexOf("\n");
    if (index != -1) {
        int count = text.count("\n");
        for (int i = 0; i < count; ++i) {
            index = text.indexOf("\n", i);
            if (index == 0)
                continue;
            if (text.at(index - 1) == ")" || text.at(index - 1) == "%")
                text.replace(index, 1, "×");
        }
    }
}

/**
 * @brief 在鼠标处显示菜单
 */
void InputEdit::showTextEditMenu()
{
    DMenu *menu = new DMenu(this);
    menu->addAction(m_undo);
    menu->addAction(m_redo);
    menu->addAction(m_cut);
    menu->addAction(m_copy);
    menu->addAction(m_paste);
    menu->addAction(m_delete);
    menu->addSeparator();

    int separate = 3;
    switch (DSettingsAlt::instance()->getOption("mode").toInt()) {
        case 0: separate = DSettingsAlt::instance()->getStandardSeparate();
        break;
    case 1: separate = DSettingsAlt::instance()->getScientificSeparate();
        break;
    case 2:
        if (Settings::instance()->programmerBase == 10) {
            separate = DSettingsAlt::instance()->getProgrammerSeparate();
        } else {
            separate = -1;
        }
        break;
    }

    if (separate == 3) {
        menu->addAction(m_fourSeparate);
    } else if (separate == 4) {
        menu->addAction(m_threeSeparate);
    }

    menu->addAction(m_select);

    if (QApplication::clipboard()->text().isEmpty())
        m_paste->setEnabled(false);
    else
        m_paste->setEnabled(true);

    if (this->selectedText().isEmpty()) {
        m_cut->setEnabled(false);
        m_copy->setEnabled(false);
        m_delete->setEnabled(false);
    } else {
        m_cut->setEnabled(true);
        m_copy->setEnabled(true);
        m_delete->setEnabled(true);
    }

    //全选需要有内容
    if (this->text() != QString()) {
        m_select->setEnabled(true);
    } else {
        m_select->setEnabled(false);
    }

    menu->move(cursor().pos());
    menu->exec();
    menu->deleteLater();
}

/**
 * @brief 在光标处显示菜单
 */
void InputEdit::showTextEditMenuByAltM()
{
    DMenu *menu = new DMenu(this);
    menu->addAction(m_undo);
    menu->addAction(m_redo);
    menu->addAction(m_cut);
    menu->addAction(m_copy);
    menu->addAction(m_paste);
    menu->addAction(m_delete);
    menu->addSeparator();

    int separate = 3;
    switch (DSettingsAlt::instance()->getOption("mode").toInt()) {
        case 0: separate = DSettingsAlt::instance()->getStandardSeparate();
        break;
    case 1: separate = DSettingsAlt::instance()->getScientificSeparate();
        break;
    case 2:
        if (Settings::instance()->programmerBase == 10) {
            separate = DSettingsAlt::instance()->getProgrammerSeparate();
        } else {
            separate = -1;
        }
        break;
    }

    if (separate == 3) {
        menu->addAction(m_fourSeparate);
    } else if (separate == 4) {
        menu->addAction(m_threeSeparate);
    }

    menu->addAction(m_select);

    if (QApplication::clipboard()->text().isEmpty())
        m_paste->setEnabled(false);
    else
        m_paste->setEnabled(true);

    if (this->selectedText().isEmpty()) {
        m_cut->setEnabled(false);
        m_copy->setEnabled(false);
        m_delete->setEnabled(false);
    } else {
        m_cut->setEnabled(true);
        m_copy->setEnabled(true);
        m_delete->setEnabled(true);
    }

    //全选需要有内容
    if (this->text() != QString()) {
        m_select->setEnabled(true);
    } else {
        m_select->setEnabled(false);
    }

    menu->move(mapToGlobal(cursorRect().bottomRight()));
    menu->exec();
    menu->deleteLater();
}

/**
 * @brief InputEdit::formatAns
 * 按进制返回计算结果
 */
QString InputEdit::formatAns(const QString &text)
{
    QString num = formatExpression(2, text);
    Quantity ans(HNumber(num.toLatin1().data()));
    switch (Settings::instance()->programmerBase) {
    case 16:
        num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
        break;
    case 8:
        num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
        break;
    case 2:
        num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
        break;
    default:
        num = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
        break;
    }
    return num;
}

/**
 * @brief InputEdit::getCurrentAns
 * 输入时获取当前输入栏中的结果
 */
QPair<bool, Quantity> InputEdit::getCurrentAns()
{
    QPair<bool, Quantity> pair;
    QString expression;
    if (Settings::instance()->programmerBase == 8 || Settings::instance()->programmerBase == 16) {
        expression = InputEdit::formatExpression(2, scanAndExec(Settings::instance()->programmerBase, 2));
    } else {
        expression = InputEdit::formatExpression(Settings::instance()->programmerBase, text());
    }
    QString exp1 = symbolComplement(expression);
    m_evaluator->setExpression(exp1);
    Quantity ans = m_evaluator->evalUpdateAns();

    if (m_evaluator->error().isEmpty()) {
        if (ans.isNan() && !m_evaluator->isUserFunctionAssign()) {
            pair.first = false;
            pair.second = Quantity(0);
        } else {
            pair.first = true;
            pair.second = ans;
        }
    } else {
        pair.first = false;
        pair.second = Quantity(0);
    }
    emit prolistAns(pair);
    return pair;
}

/**
 * @brief 选中改变的槽
 */
void InputEdit::selectionChangedSlot()
{
    if (!hasFocus())
        return; //只有选中被改变情况下给m_selected赋值,选中输入不会改变;选中输入后优先级高于cursorchanged，去掉return无意义
    m_selected.oldText = text();
    m_selected.selected = selectedText();
    m_selected.curpos = selectionStart() < selectionEnd() ? selectionStart() : selectionEnd();
}

/**
 * @brief InputEdit::getMemoryAnswer
 * 点击m+,m-,ms时进行获取的计算结果
 * @return pair.first-是否可计算　pair.second-计算结果
 */
QPair<bool, Quantity> InputEdit::getMemoryAnswer()
{
    QPair<bool, Quantity> pair;
    QString expression;
    expression = symbolComplement(expressionText()).replace(QString::fromUtf8("＋"), "+")
                 .replace(QString::fromUtf8("－"), "-")
                 .replace(QString::fromUtf8("×"), "*")
                 .replace(QString::fromUtf8("÷"), "/")
                 .replace(QString::fromUtf8(","), "");
    m_evaluator->setExpression(expression);
    m_memoryans = m_evaluator->evalUpdateAns();
    if (m_evaluator->error().isEmpty()) {
        if (m_memoryans.isNan() && !m_evaluator->isUserFunctionAssign()) {
            pair.first = false;
            pair.second = Quantity(0);
            return pair;
        }
        pair.first = true;
        pair.second = m_memoryans;
        return pair;
    } else {
        pair.first = false;
        pair.second = Quantity(0);
        return pair;
    }
}

/**
 * @brief 输入栏是否为空，方便判断m-,m+,ms是否可用
 */
void InputEdit::isExpressionEmpty()
{
    if (text().isEmpty())
        emit emptyExpression(true);
    else
        emit emptyExpression(false);
}

/**
 * @brief 当前输入框为历史记录点击时，清空ans
 */
void InputEdit::hisexpression()
{
    m_ansLength = 0;
}

/**
 * @brief ExpressionBar::symbolComplement
 * 当计算((()))这种超过3个括号嵌套的表达式时有问题，手动补乘号进行规避
 * @param exp
 * @return
 */
QString InputEdit::symbolComplement(const QString exp)
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
    return text;
}

/**
 * @brief InputEdit::CurrentCursorPositionNumber
 * 用于修改位键盘
 */
QString InputEdit::CurrentCursorPositionNumber(const int pos, const int base)
{
    QString text = this->text();
    if (text.isEmpty()) {
        return "";
    }
    QString currentnum = QString();
    int numstart = pos, numend = pos;
    while (numstart != 0 && (isNumber(text.at(numstart - 1)) ||
                             (text.length() > 1 && numstart == 1 && isNumber(text.at(1)) && text.at(0) == QString::fromUtf8("－")))) {
        numstart--;
    }
    while (numend < text.length() && isNumber(text.at(numend))) {
        numend++;
    }
    currentnum = text.mid(numstart, numend - numstart);
    currentnum = formatExpression(Settings::instance()->programmerBase, currentnum);
    Quantity ans(HNumber(currentnum.toLatin1().data()));
    if (ans.isNan())
        return "";
    switch (base) {
    case 16:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
        break;
    case 8:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
        break;
    case 2:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
        break;
    default:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
        break;
    }
    return currentnum;
}

/**
 * @brief InputEdit::CurrentCursorPositionNumber
 * 获取当前光标前后的整个数字，光标前是符号则返回后面的数字
 */
QString InputEdit::CurrentCursorPositionNumber(const int pos)
{
    QString text = this->text();
    if (text.isEmpty()) {
        return "";
    }
    QString currentnum = QString();
    int numstart = pos - 1, numend = pos - 1;
    if (numstart >= 0 && isNumber(text.at(numstart))) {
        while (numstart > 0 && (isNumber(text.at(numstart - 1)) ||
                                ((numstart == 1) && isNumber(text.at(1)) && text.at(0) == QString::fromUtf8("－")))) {
            numstart--;
        }
        while (numend < text.length() && isNumber(text.at(numend))) {
            numend++;
        }
        currentnum = text.mid(numstart, numend - numstart);
    }
    currentnum = formatExpression(Settings::instance()->programmerBase, currentnum);
    Quantity ans(HNumber(currentnum.toLatin1().data()));
    if (ans.isNan())
        return "";
    switch (Settings::instance()->programmerBase) {
    case 16:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
        break;
    case 8:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
        break;
    case 2:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
        break;
    default:
        currentnum = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
        break;
    }
    return currentnum;
}

/**
 * @brief InputEdit::getCurrentCursorPositionNumber
 * @param pos:当前光标位置
 * 获取当前光标所在位置对应的数字，用于修改位键盘
 */
void InputEdit::getCurrentCursorPositionNumber(const int pos)
{
    QString currentnum = CurrentCursorPositionNumber(pos, 2);
    if (currentnum == "") {
        emit cursorPositionNumberChanged("0");
    } else {
        emit cursorPositionNumberChanged(currentnum);
    }
//    qDebug() << "currentnum" << currentnum;
}

bool InputEdit::isNumber(QChar a)
{
    if (a.isDigit() || a == " " || a == "," || AtoF.contains(a))
        return true;
    else
        return false;
}

QString InputEdit::formatBinaryNumber(const QString num)
{
    QString str;
    for (int i = 0; i < num.length(); i++) {
        if (num.at(i) == '1') {
            str = num.right(num.length() - i);
            str = formatAns(str);
            return str;
        }
        if (i == num.length() - 1 && num.at(num.length() - 1) != '1') {
            str = "0";
            return str;
        }
    }
    return "0";
}

/**
 * @brief InputEdit::formatExpression
 * @param text:格式化前的内容
 * @return 格式化后的text
 * 用于进制转换时和计算时插入进制标志
 */
QString InputEdit::formatExpression(const int &probase, const QString &text)
{
    QString formattext = text;
    formattext.replace(QString::fromUtf8("＋"), "+")
    .replace(QString::fromUtf8("－"), "-")
    .replace(QString::fromUtf8("×"), "*")
    .replace(QString::fromUtf8("÷"), "/")
    .replace(QString::fromUtf8(","), "")
    .replace(QString::fromUtf8(" "), "")
    .replace("%", "mod");

    QString base = QString();
    switch (probase) {
    case 16:
        base = "0x";
        break;
    case 8:
        base = "0o";
        break;
    case 2:
        base = "0b";
        break;
    default:
        break;
    }
    if (base != QString()) {
        for (int i = 0; i < formattext.length();) {
            if ((i == 0) && isNumber(formattext.at(i))) {
                formattext.insert(i, base);
                i += 3;
                continue;
            } else if ((i < formattext.length() - 1) && !isNumber(formattext.at(i)) && isNumber(formattext.at(i + 1))) {
                formattext.insert(i + 1, base);
                i += 3;
                continue;
            }
            i++;
        }
    }
    return formattext;
}

void InputEdit::focusInEvent(QFocusEvent *event)
{
    int curtemp = cursorPosition();
    QLineEdit::focusInEvent(event);
    if (event->reason() == Qt::TabFocusReason) {
        setCursorPosition(curtemp);
    }
}

/**
 * @brief onSwietThreeSeparateClicked
 * 切换为千分位菜单点击事件
 */
void InputEdit::onSwietThreeSeparateClicked()
{
    DSettingsAlt::instance()->setSeparate(3);
    handleTextChanged(m_oldText);   //更新input中的算式
    emit separateChange();  //发送更新信号
}

/**
 * @brief onswietFourSeparateClicked
 * 切换为万分位菜单点击事件
 */
void InputEdit::onswietFourSeparateClicked()
{
    DSettingsAlt::instance()->setSeparate(4);
    handleTextChanged(m_oldText);   //更新input中的算式
    emit separateChange();  //发送更新信号
}


