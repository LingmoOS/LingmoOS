/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#include "result-view-delegate.h"
#include <QPainterPath>
#include <QApplication>
#include <QPen>
#include <QStyleFactory>
#include <QTreeView>
#include "search-result-model.h"
using namespace LingmoUISearch;
static ResultItemStyle *global_instance_of_item_style = nullptr;

ResultViewDelegate::ResultViewDelegate(QObject *parent) : QStyledItemDelegate(parent),
    m_textDoc(new QTextDocument(this)),
    m_hightLightEffectHelper(new HightLightEffectHelper(this))
{
    m_textDoc->setDefaultFont(QApplication::font());
    connect(qApp, &QApplication::fontChanged, m_textDoc, &QTextDocument::setDefaultFont);
}

void ResultViewDelegate::setSearchKeyword(const QString &regFindKeyWords)
{
    m_hightLightEffectHelper->setExpression(regFindKeyWords);
}

void ResultViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.displayAlignment = QApplication::isLeftToRight() ?
            Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter) : Qt::Alignment(Qt::AlignRight|Qt::AlignVCenter);

    QString originalText = opt.text;
    opt.text = QString();

    QStyle *style = opt.widget->style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget); //绘制非文本区域内容

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);
    QFontMetrics fontMetrics(opt.font);
    QString text = fontMetrics.elidedText(originalText, Qt::ElideRight, textRect.width() - m_textDoc->documentMargin() - 1); //富余5px的宽度

    //无需显示tooltip时，将状态位写回model
    auto view = qobject_cast<QTreeView *>(this->parent());
    if(text == originalText && originalText == index.data(Qt::ToolTipRole).toString()) {
        if(view) {
            view->model()->setData(index, false, AdditionalRoles::ShowToolTip);
        }
    } else {
        if(view) {
            view->model()->setData(index, true, AdditionalRoles::ShowToolTip);
        }
    }
    opt.text = text;


    painter->save();

    if(opt.state & QStyle::State_Selected) {
        QVariant selectPen = opt.widget->property("textSelectPen");
        if(selectPen.isValid() && selectPen.canConvert<QPen>()) {
            m_hightLightEffectHelper->setTextColor(selectPen.value<QPen>().brush());
        } else {
            m_hightLightEffectHelper->setTextColor(QBrush(opt.palette.highlightedText().color()));
        }
    } else {
        m_hightLightEffectHelper->setTextColor(QBrush(opt.palette.text().color()));
    }

    m_textDoc->setPlainText(text);
    m_hightLightEffectHelper->setDocument(m_textDoc);
    m_hightLightEffectHelper->rehighlight();

    int textRectX = QApplication::isLeftToRight() ? textRect.x() : textRect.width() - fontMetrics.horizontalAdvance(text) - m_textDoc->documentMargin();
    painter->translate(textRectX, textRect.y() + (textRect.height() - fontMetrics.height()) / 2 - m_textDoc->documentMargin());
    m_textDoc->drawContents(painter);
    painter->restore();
}

ResultItemStyle *ResultItemStyle::getStyle()
{
    if (!global_instance_of_item_style) {
        global_instance_of_item_style = new ResultItemStyle;
    }
    return global_instance_of_item_style;
}

ResultItemStyle::ResultItemStyle()
{
    connect(qApp, &QApplication::paletteChanged, this, [&](){
        auto style = QStyleFactory::create("lingmo");
        if(style) {
            setBaseStyle(style);
            Q_EMIT baseStyleChanged();
        }
    });
}

int ResultItemStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_ItemView_ActivateItemOnSingleClick:
        return false;
    default:
        return qApp->style()->styleHint(hint, option, widget, returnData);
    }
}

HightLightEffectHelper::HightLightEffectHelper(QObject *parent) : QSyntaxHighlighter(parent)
{
    m_expression.setCaseSensitivity(Qt::CaseInsensitive);
    m_expression.setPatternSyntax(QRegExp::FixedString);
}

void HightLightEffectHelper::setExpression(const QString &text)
{
    m_expression.setPattern(text);
}

void HightLightEffectHelper::setTextColor(const QBrush &brush)
{
    m_textCharFormat.setForeground(brush);
}

void HightLightEffectHelper::highlightBlock(const QString &text)
{
    setFormat(0, text.length(), m_textCharFormat);
    m_textCharFormat.setFontWeight(QFont::Bold);
    int index = text.indexOf(m_expression);
    while(index >= 0){
        int length = m_expression.matchedLength();
        setFormat(index, length, m_textCharFormat);
        index = text.indexOf(m_expression, index+length);
    }
    m_textCharFormat.setFontWeight(QFont::Normal);
}
