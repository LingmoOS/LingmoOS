/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "musicsearchlistdelegate.h"
#include "musicsearchlistview.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QStyleOptionViewItem>
#include <QTextCursor>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>

#include "UI/base/widgetstyle.h"

QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

MusicSearchListDelegate::MusicSearchListDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}

MusicSearchListDelegate::~MusicSearchListDelegate()
{

}

void MusicSearchListDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}

bool MusicSearchListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return true;
}

void MusicSearchListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //获取当前行信息
    const MusicSearchListview *listview = qobject_cast<const MusicSearchListview *>(option.widget);
    QString searchText = listview->getSearchText();

    QColor textColor;
    QColor lightColor;

    if (WidgetStyle::themeColor == 1) {
        textColor = QColor("#FFFFFF");
    } else if (WidgetStyle::themeColor == 0) {
        textColor = QColor("#000000");
    }


    QPalette pa = option.palette;
    QBrush selectBrush = pa.brush(QPalette::Active, QPalette::Highlight);
    QColor selectColor = selectBrush.color();
    lightColor = selectColor;

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

//    QFont fontT6;
//    fontT6.setPixelSize(14);

    // 通过字体宽度来对齐左右间隔
    int allFontWidth = 232 - 28 -28;

    if (listview->getSearchType() == SearchType::TitleType) {
        // 绘制歌曲
        musicDataStruct metaPtr = index.data(Qt::UserRole + SearchType::TitleType).value<musicDataStruct>();

        QString mtext;
        mtext = metaPtr.title + "-" + metaPtr.singer;
        QFontMetricsF fontWidth(listview->font());
        mtext = fontWidth.elidedText(mtext, Qt::ElideRight, allFontWidth);

        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        if (option.state.testFlag(QStyle::State_HasFocus))
            viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
        QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
        viewOption.text = "";
        pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

        QTextDocument document;
        // 设置文字边距，保证绘制文字居中
        document.setDocumentMargin(0);
        document.setPlainText(mtext);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setForeground(lightColor);

        // 搜索字体高亮
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        fmt.setForeground(textColor);
        fmt.setFont(listview->font());
        testcursor.mergeCharFormat(fmt);
        testcursor.clearSelection();
        testcursor.movePosition(QTextCursor::EndOfLine);

        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
            highlight_cursor = document.find(searchText, highlight_cursor);
            if (!highlight_cursor.isNull()) {
                if (!found) {
                    found = true;
                }
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();

        QAbstractTextDocumentLayout::PaintContext paintContext;

        int margin = static_cast<int>(((option.rect.height() - fontWidth.height()) / 2));
//        QRect textRect(0+28, option.rect.y() + margin, 287, option.rect.height());
        // 0+28 为左缩进28px
        QRect textRect(0+28, option.rect.y() + margin, allFontWidth, option.rect.height());
        painter->save();
        painter->translate(textRect.topLeft());
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        document.documentLayout()->draw(painter, paintContext);
        painter->restore();

    } else if (listview->getSearchType() == SearchType::SingerType) {
        // 绘制歌曲
        musicDataStruct metaPtr = index.data(Qt::UserRole + SearchType::SingerType).value<musicDataStruct>();

        QString mtext;
        mtext = metaPtr.title + "-" + metaPtr.singer;
        QFontMetricsF fontWidth(listview->font());
        mtext = fontWidth.elidedText(mtext, Qt::ElideRight, allFontWidth);
        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        if (option.state.testFlag(QStyle::State_HasFocus))
            viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
        QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
        viewOption.text = "";
        pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

        QTextDocument document;
        // 设置文字边距，保证绘制文字居中
        document.setDocumentMargin(0);
        document.setPlainText(mtext);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setForeground(lightColor);

        // 搜索字体高亮
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        fmt.setForeground(textColor);
        fmt.setFont(listview->font());
        testcursor.mergeCharFormat(fmt);
        testcursor.clearSelection();
        testcursor.movePosition(QTextCursor::EndOfLine);

        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
            highlight_cursor = document.find(searchText, highlight_cursor);
            if (!highlight_cursor.isNull()) {
                if (!found)
                    found = true;
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();

        QAbstractTextDocumentLayout::PaintContext paintContext;

        int margin = static_cast<int>(((option.rect.height() - fontWidth.height()) / 2));
        QRect textRect(0+28, option.rect.y() + margin, allFontWidth, option.rect.height());
        painter->save();
        painter->translate(textRect.topLeft());
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        document.documentLayout()->draw(painter, paintContext);
        painter->restore();

    } else if (listview->getSearchType() == SearchType::AlbumType) {
        // 绘制歌曲
        musicDataStruct metaPtr = index.data(Qt::UserRole + SearchType::AlbumType).value<musicDataStruct>();

        QString mtext;
        mtext = metaPtr.album + "-" + metaPtr.singer;
        QFontMetricsF fontWidth(listview->font());
        mtext = fontWidth.elidedText(mtext, Qt::ElideRight, allFontWidth);
        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        if (option.state.testFlag(QStyle::State_HasFocus))
            viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
        QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
        viewOption.text = "";
        pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

        QTextDocument document;
        // 设置文字边距，保证绘制文字居中
        document.setDocumentMargin(0);
        document.setPlainText(mtext);
        bool found = false;
        QTextCursor highlight_cursor(&document);
        QTextCursor cursor(&document);

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setForeground(lightColor);

        // 搜索字体高亮
        QTextCursor testcursor(&document);
        testcursor.select(QTextCursor::LineUnderCursor);
        QTextCharFormat fmt;
        fmt.setForeground(textColor);
        fmt.setFont(listview->font());
        testcursor.mergeCharFormat(fmt);
        testcursor.clearSelection();
        testcursor.movePosition(QTextCursor::EndOfLine);

        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
            highlight_cursor = document.find(searchText, highlight_cursor);
            if (!highlight_cursor.isNull()) {
                if (!found)
                    found = true;
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();

        QAbstractTextDocumentLayout::PaintContext paintContext;

        int margin = static_cast<int>(((option.rect.height() - fontWidth.height()) / 2));
        QRect textRect(0+28, option.rect.y() + margin, allFontWidth, option.rect.height());
        painter->save();
        painter->translate(textRect.topLeft());
        painter->setClipRect(textRect.translated(-textRect.topLeft()));
        document.documentLayout()->draw(painter, paintContext);
        painter->restore();
    }
}

QSize MusicSearchListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(126, 36);
}
