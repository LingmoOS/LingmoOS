// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simplelistdelegate.h"

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <DGuiApplicationHelper>
#include <QtGlobal>

#include "dthememanager.h"
#include "simplelistmodel.h"
#include "simplelistview.h"
#include "../utils.h"

DWIDGET_USE_NAMESPACE

const int PADDING = 15; //历史记录区边距
const int NOHISTORYHEIGHT = 302; // 无历史记录rect高度
const int HISWIDTH = 451; //历史记录宽度

SimpleListDelegate::SimpleListDelegate(int mode, QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_mode = mode;
    m_selected = false;
    m_simpleListDelegate = this;
}

SimpleListDelegate::~SimpleListDelegate()
{
//    delete m_simpleListDelegate;
//    m_simpleListDelegate = NULL;
}

/**
 * @brief 设置联动项
 */
void SimpleListDelegate::setHisLink(const int link)
{
    m_linkItem.push_back(link);
}

/**
 * @brief 设置被联动项
 */
void SimpleListDelegate::setHisLinked(const int linked)
{
    m_linkedIten.push_back(linked);
}

/**
 * @brief SimpleListDelegate::删除联动（暂未使用）
 * @param link 联动项
 * @param linked 被联动项
 */
void SimpleListDelegate::removeLine(const int link, const int linked)
{
    for (int i = 0; i < m_linkItem.size(); ++i) {
        if (m_linkItem[i] == link)
            m_linkItem.remove(i);
    }
    for (int i = 0; i < m_linkedIten.size(); ++i) {
        if (m_linkedIten[i] == linked)
            m_linkedIten.remove(i);
    }
}

/**
 * @brief 删除单项联动
 */
void SimpleListDelegate::removeLine(int index)
{
    if (index < m_linkItem.size())
        m_linkItem.remove(index);
    if (index < m_linkedIten.size())
        m_linkedIten.remove(index);
}

/**
 * @brief 删除最后一项联动
 */
void SimpleListDelegate::removeHisLink()
{
    if (!m_linkItem.isEmpty())
        m_linkItem.removeLast();
}

/**
 * @brief 删除所有联动
 */
void SimpleListDelegate::removeAllLink()
{
    m_linkedIten.clear();
    m_linkItem.clear();
}

/**
 * @brief 删除最后一项被联动项
 */
void SimpleListDelegate::removeHisLinked()
{
    if (!m_linkedIten.isEmpty())
        m_linkedIten.removeLast();
}

void SimpleListDelegate::setThemeType(int type)
{
    m_type = type;
}

/**
 * @brief 为paint提供参数
 * @param index 行
 * @param state 当前状态0-normal 1-hover 2-press
 */
void SimpleListDelegate::paintback(const QModelIndex &index, int state)
{
    m_row = index.row();
    m_state = state;
}

void SimpleListDelegate::currentfocusindex(QModelIndex index)
{
    m_focusindex = index;
}

void SimpleListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    drawFocusStatus(painter, option); //焦点边框
    if (m_mode == 0) {
        const QString expression = index.data(SimpleListModel::ExpressionRole).toString();
        QRect rect(option.rect);
        rect.setRight(option.widget->width() - 13);
        QString errorFontColor;
        QString fontColor;
        QString linkColor;
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                                QPainter::SmoothPixmapTransform);

        QFont font;
        for (int i = 16; i > 12; --i) {  // edit for bug-17238
            font.setPixelSize(i);

            QFontMetrics fm(font);
            int fontWidth = fm.horizontalAdvance(expression);
            int editWidth = rect.width() - 24;

            if (fontWidth < editWidth)
                break;
        }
        painter->setFont(font);

        QStringList splitList = expression.split("＝");
        QString resultStr = splitList.last();
        int resultWidth = painter->fontMetrics().horizontalAdvance(resultStr);

        if (resultWidth > rect.width() / 1.4) {
            resultStr = painter->fontMetrics().elidedText(resultStr, Qt::ElideRight,
                                                          int(rect.width() / 1.4) + PADDING);
            resultWidth = painter->fontMetrics().horizontalAdvance(resultStr);
        }

        if (m_type == 1) {
            errorFontColor = "#FF5736";  //edit for bug-21508
            linkColor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();
            fontColor = "#838483";
        } else {
            errorFontColor = "#9A2F2F";  //edit for bug-21508
            linkColor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name();
            fontColor = "#838483";
        }

        // check result text is error.
        if (resultStr == tr("Expression error")) {
            painter->setPen(QColor(errorFontColor));
        } else {
            painter->setPen(QColor(fontColor));
            for (int i = 0; i < m_linkItem.size(); ++i) {
                if (m_linkItem[i] == index.row())
                    painter->setPen(QColor(linkColor));
            }
        }

        int equalStrWidth = painter->fontMetrics().horizontalAdvance(" ＝ ");
        QString expStr = painter->fontMetrics().elidedText(
                             splitList.first(), Qt::ElideLeft, rect.width() - resultWidth - PADDING * 2 - equalStrWidth);
        // QString expStr = splitList.first();

        if (m_state == 2 && m_row == index.row()) {
            //edit for bug--21508
            QRectF resultRect(rect.topRight().x() - resultWidth - PADDING - 2, rect.y() + 4.8, resultWidth + 7,
                              rect.height() - 9.6);
            QPainterPath path;
            path.addRoundedRect(resultRect, 4, 4);

            // Setting Gradient
            //        QLinearGradient linear(resultRect.topLeft(), resultRect.bottomRight());
            //        linear.setColorAt(0, QColor(0, 151, 231, 255));
            //        linear.setColorAt(1, QColor(0, 122, 219, 255));
            //        linear.setColorAt(0, QColor("#0081FF"));
            //        linear.setColorAt(1, QColor("#0081FF"));
            //        linear.setSpread(QGradient::PadSpread);
            //edit for bug--21476
            if (m_type == 1) {
                QBrush brush(QColor(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name()));
                painter->fillPath(path, brush);
            } else {
                QBrush brush(QColor(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name()));
                painter->fillPath(path, brush);
            }

            painter->setPen(QPen(QColor(Qt::white)));
//            m_simpleListDelegate->setSelect(false);
        }

        if (splitList.size() == 1) {
            // draw expression text;
            painter->setPen(QColor(fontColor));
            painter->drawText(
                QRect(rect.x() + PADDING, rect.y(), rect.width() - PADDING * 2, rect.height()),
                Qt::AlignVCenter | Qt::AlignRight, expStr);
        } else {
            // draw result text.
            painter->drawText(
                QRect(rect.x() + PADDING, rect.y(), rect.width() - PADDING * 2, rect.height()),
                Qt::AlignVCenter | Qt::AlignRight, resultStr);

            QString linkNum, exp;
            m_simpleListDelegate->cutApart(expStr, linkNum, exp); //剪切出可能被联动项
            exp = exp + " ＝ ";

            // draw expression text;
            painter->setPen(QColor(fontColor));
            painter->drawText(QRect(rect.x() + PADDING, rect.y(),
                                    rect.width() - resultWidth - PADDING * 2, rect.height()),
                              Qt::AlignVCenter | Qt::AlignRight, exp);

            painter->setPen(QColor(fontColor));
            for (int i = 0; i < m_linkedIten.size(); ++i) {
                if (m_linkedIten[i] == index.row() && resultStr != tr("Expression error")) // 错误表达式不联动
                    painter->setPen(QColor(linkColor)); //判断第一个数是否是被联动项,如果是，设置字体颜色为高亮
            }

            int expWidth = painter->fontMetrics().horizontalAdvance(exp);
            painter->drawText(QRect(rect.x() + PADDING, rect.y(),
                                    rect.width() - resultWidth - expWidth - PADDING * 2, rect.height()),
                              Qt::AlignVCenter | Qt::AlignRight, linkNum);
        }
    } else if (m_mode == 1) {
        const QString expression = index.data(SimpleListModel::ExpressionWithOutTip).toString();
        QRect rect(option.rect);
        rect.setRight(option.widget->width() - 5); //设置矩形右边缘,会导致rect宽度改变
        QString errorFontColor;
        QString fontColor;
        QString resultColor;
        QColor normalbackground, pressbackground, nohistory;
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                                QPainter::SmoothPixmapTransform);
        QTextOption textoption(Qt::AlignRight | Qt::AlignVCenter);
        textoption.setWrapMode(QTextOption::WrapAnywhere); //单词可在任意地方分行
        QFontMetrics fm1 = painter->fontMetrics();
        QFont font;
        font.setPixelSize(16);
        QFont fontresult; //结果字体
        fontresult.setPixelSize(30);
        painter->setFont(font);

        QStringList splitList = expression.split("＝");
        QString resultStr = splitList.last();
        QString exp = splitList.first() + " ＝ ";

        int expHeight;
        int expline = (painter->fontMetrics().horizontalAdvance(exp) % (rect.width() - PADDING * 2)) ?
                      (painter->fontMetrics().horizontalAdvance(exp) / (rect.width() - PADDING * 2) + 1) :
                      (painter->fontMetrics().horizontalAdvance(exp) / (rect.width() - PADDING * 2));
        expHeight = painter->fontMetrics().height() * expline;

        if (m_type == 1) {
            errorFontColor = "#FF5736";  //edit for bug-21508
            resultColor = "#303030";
            fontColor = "#838483";
            normalbackground = QColor(0, 0, 0);
            normalbackground.setAlphaF(0.05);
            pressbackground = QColor(0, 0, 0);
            pressbackground.setAlphaF(0.2);
            nohistory = QColor(85, 85, 85);
            nohistory.setAlphaF(0.4);
        } else {
            errorFontColor = "#9A2F2F";  //edit for bug-21508
            resultColor = "#B4B4B4";
            fontColor = "#838483";
            normalbackground = QColor(255, 255, 255);
            normalbackground.setAlphaF(0.05);
            pressbackground = QColor(255, 255, 255);
            pressbackground.setAlphaF(0.2);
            nohistory = QColor(192, 198, 212);
            nohistory.setAlphaF(0.4);
        }

        // check result text is error.
        painter->setPen(QColor(fontColor));
        if (splitList.size() == 1) {
            painter->setPen(QColor(nohistory));
            painter->drawText(
                QRectF(rect.x() + PADDING, rect.y(), rect.width() - PADDING * 2, NOHISTORYHEIGHT),
                expression, Qt::AlignHCenter | Qt::AlignVCenter);
        } else {
            // draw result text.
            painter->drawText(
                QRectF(rect.x() + PADDING, rect.y() + 10, rect.width() - PADDING * 2, expHeight),
                exp, textoption); //exp与上方空隙5pix
            painter->setFont(fontresult);
            int resultHeight;
            int resultline = (painter->fontMetrics().horizontalAdvance(resultStr) % (rect.width() - PADDING * 2 - 1)) ?
                             (painter->fontMetrics().horizontalAdvance(resultStr) / (rect.width() - PADDING * 2 - 1) + 1) :
                             (painter->fontMetrics().horizontalAdvance(resultStr) / (rect.width() - PADDING * 2 - 1)); //由于结果字体较大，暂以此避免
            resultHeight = painter->fontMetrics().height() * resultline;
            if (resultStr == tr("Expression error")) {
                painter->setPen(QColor(errorFontColor));
            } else {
                painter->setPen(QColor(resultColor));
            }
            painter->drawText(
                QRectF(rect.x() + PADDING, rect.y() + expHeight + 12, rect.width() - PADDING * 2, resultHeight),
                resultStr, textoption); //result与exp空隙12pix
            if (option.state & QStyle::State_MouseOver && m_state == 0) {
                painter->setBrush(normalbackground);
                painter->setPen(Qt::NoPen);
                painter->drawRect(rect.x(), rect.y(), rect.width() + 5, rect.height());
            } else if (m_state == 1 && m_row == index.row()) {
                painter->setBrush(normalbackground);
                painter->setPen(Qt::NoPen);
                painter->drawRect(rect.x(), rect.y(), rect.width() + 5, rect.height());
            }
            if (m_state == 2 && m_row == index.row()) {
                painter->setPen(Qt::NoPen);
                painter->setBrush(pressbackground);
                painter->drawRect(rect.x(), rect.y(), rect.width() + 5, rect.height());
            }
        }
    }

}
/**
 * @brief 绘制焦点状态
 */
void SimpleListDelegate::drawFocusStatus(QPainter *painter, const QStyleOptionViewItem &option) const
{
    SimpleListView *listview = qobject_cast<SimpleListView *>(option.styleObject);
    if (listview->hasFocus()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        QRectF itemrect(listview->visualRect(m_focusindex));
        QRectF frame(itemrect.left() + 1, itemrect.top() + 1, HISWIDTH - 2, itemrect.height() - 2);
        QPen pen;
        pen.setColor(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
        pen.setWidth(2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(frame, 8, 8); //focus边框
    }
}

/**
 * @brief 大小设置
 */
QSize SimpleListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option);
    if (m_mode == 1) {
        const QString expression = index.data(SimpleListModel::ExpressionWithOutTip).toString();
        const int rectwidth = 447; //paintevent设置右边缘后的宽度
        QStringList splitList = expression.split("＝");
        if (splitList.size() == 1)
            return QSize(-1, 302); //历史记录无内容时大小
        QString resultStr = splitList.last();
        QString exp = splitList.first() + "＝";
        QFont font;
        font.setPixelSize(16);
        QFontMetrics fmexp(font);
        QFont fontresult; //结果字体
        fontresult.setPixelSize(30);
        QFontMetrics fmresult(fontresult);
        int expHeight;
        int expline = (fmexp.horizontalAdvance(exp) % (rectwidth - PADDING * 2)) ?
                      (fmexp.horizontalAdvance(exp) / (rectwidth - PADDING * 2) + 1) :
                      (fmexp.horizontalAdvance(exp) / (rectwidth - PADDING * 2));
        expHeight = fmexp.height() * expline;
        int resultHeight;
        int resultline = (fmresult.horizontalAdvance(resultStr) % (rectwidth - PADDING * 2 - 1)) ?
                         (fmresult.horizontalAdvance(resultStr) / (rectwidth - PADDING * 2 - 1) + 1) :
                         (fmresult.horizontalAdvance(resultStr) / (rectwidth - PADDING * 2 - 1)); //由于结果字体较大，暂以此避免
        resultHeight = fmresult.height() * resultline;
        return QSize(HISWIDTH, expHeight + resultHeight + 25); //多出25pix空隙
    } else
        return QSize(-1, 33);
}

//bool SimpleListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
//                                     const QStyleOptionViewItem &option, const QModelIndex &index)
//{
//    Q_UNUSED(event);
//    Q_UNUSED(model);
//    Q_UNUSED(option);
//    m_selected = true;
//    if (m_mode == 0)
//        emit obtainingHistorical(index);
//    return true;
//}

/**
 * @brief 将等于号左侧表达式中被联动项与剩余部分分开
 * @param text 表达式
 * @param linkNum 第一个数字
 * @param expStr 剩余项
 */
void SimpleListDelegate::cutApart(const QString text, QString &linkNum, QString &expStr)
{
    QString exp = text;
    QStringList list;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    list = exp.split(QRegExp("[＋－×÷/()]"), QString::SkipEmptyParts);
#else
    list = exp.split(QRegularExpression("[＋－×÷/()]"), Qt::SkipEmptyParts);
#endif
    if (list.isEmpty() || list.size() == 1) {
        linkNum = "";
        expStr = exp;
        return;
    }
    if (exp.at(0) != "－") {
        int index = 0;
        while (exp.at(index) == "(") {
            ++index;
            linkNum.append("(");
            if (exp.at(index) == "－")
                linkNum.append("－");
        }
        linkNum.append(list.at(0));
    } else {
        linkNum.append("－");
        if (exp.at(1) == "(")
            linkNum.append("(");
        linkNum.append(list.at(0));
    }
    if (linkNum.at(linkNum.size() - 1) == "E")
        linkNum = linkNum + exp.at(exp.indexOf("E") + 1) + list.at(1);
    expStr = text.right(text.length() - linkNum.length());
}

/**
 * @brief 更新编辑器组件的大小
 */
void SimpleListDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
