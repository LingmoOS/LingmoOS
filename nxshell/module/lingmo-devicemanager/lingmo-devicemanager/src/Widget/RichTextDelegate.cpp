// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "RichTextDelegate.h"
#include "PageBoardInfo.h"

// Dtk头文件
#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>

// Qt库文件
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QLoggingCategory>
#include <DFontSizeManager>
#include <QPainterPath>

// 其它头文件
#include "DetailTreeView.h"

DWIDGET_USE_NAMESPACE

RichTextDelegate::RichTextDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void RichTextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;

    if (opt.features & QStyleOptionViewItem::Alternate) {
        background = palette.color(cg, DPalette::ItemBackground);
    } else {
        background = palette.color(cg, DPalette::Base);
    }

    QRect rect = opt.rect;
    QPainterPath path;
    QRect rectpath = rect;

    // 确定绘制区域的形状，单元格
    rectpath.setWidth(rect.width() - 1);

    DWidget *par = dynamic_cast<DWidget *>(this->parent());
    if (!par)
        return;
    if (rectpath.y() > 0) {

        // 高度不超过表格高度
        if (rectpath.y() + rectpath.height() < 40 * (par->height() / 40 - 1)) {
            path.addRect(rectpath);
        } else {
            // 单元格超过表格下边框
            QRect tmpRect = rectpath;
            tmpRect.setHeight(40 * (par->height() / 40 - 1) - rectpath.y());

            path.addRoundedRect(tmpRect, 8, 8);

            if (index.column() == 0) {

                // 填充左上，右侧
                // 左上空白
                QRect rect1(tmpRect.topLeft().x(), tmpRect.topLeft().y(), 8, tmpRect.height() - 8);

                // 第一列后移一个像素
                rect1.setX(rect1.x() + 1);

                QPainterPath pathLeftTop;
                pathLeftTop.addRect(rect1);

                // 左下角绘制区域
                pathLeftTop = pathLeftTop.subtracted(path);
                painter->fillPath(pathLeftTop, background);

                // 右侧空白
                QRect rect2(tmpRect.topRight().x() - 7, tmpRect.topRight().y(), 8, tmpRect.height());

                QPainterPath pathRight;
                pathRight.addRect(rect2);

                // 右侧绘制区域
                pathRight = pathRight.subtracted(path);
                painter->fillPath(pathRight, background);
            }

            if (index.column() == 1) {

                //填充左侧,右上
                // 左侧空白
                QRect rect1(tmpRect.topLeft().x(), tmpRect.topLeft().y(), 8, tmpRect.height());

                QPainterPath pathLeft;
                pathLeft.addRect(rect1);

                // 左侧绘制区域
                pathLeft = pathLeft.subtracted(path);
                painter->fillPath(pathLeft, background);

                // 右上空白
                QRect rect2(tmpRect.topRight().x() - 7, tmpRect.topRight().y(), 8, tmpRect.height() - 8);

                QPainterPath pathRightTop;
                pathRightTop.addRect(rect2);

                // 右上绘制区域
                pathRightTop = pathRightTop.subtracted(path);
                painter->fillPath(pathRightTop, background);
            }

        }
    }

    if (rectpath.y() <= 0) {
        QRect tmpRect = rectpath;
        tmpRect.setY(0);

        // 高度超过表格下边框
        if (rectpath.y() + rectpath.height() > 40 * (par->height() / 40 - 1) - rectpath.y()) {
            tmpRect.setHeight(40 * (par->height() / 40 - 1));
            path.addRect(rectpath);
        } else {

            path.addRoundedRect(tmpRect, 8, 8);
            // 填充第一列空白
            if (index.column() == 0) {

                // 左下空白
                QRect rect1(tmpRect.bottomLeft().x(), tmpRect.bottomLeft().y() - 7, 8, 8);

                // 第一列后移一个像素
                rect1.setX(rect1.x() + 1);

                QPainterPath pathLeftBottom;
                pathLeftBottom.addRect(rect1);

                // 左下角绘制区域
                pathLeftBottom = pathLeftBottom.subtracted(path);
                painter->fillPath(pathLeftBottom, background);

                // 右侧空白
                QRect rect2(tmpRect.topRight().x() - 7, tmpRect.topRight().y() + 1, 8, tmpRect.height());

                QPainterPath pathRight;
                pathRight.addRect(rect2);

                // 右侧绘制区域
                pathRight = pathRight.subtracted(path);
                painter->fillPath(pathRight, background);
            }

            // 填充第二列空白
            if (index.column() == 1) {
                // 左侧空白
                QRect rect1(tmpRect.topLeft().x(), tmpRect.topLeft().y() + 1, 8, tmpRect.height());

                QPainterPath pathLeft;
                pathLeft.addRect(rect1);

                // 左侧绘制区域
                pathLeft = pathLeft.subtracted(path);
                painter->fillPath(pathLeft, background);

                // 右下角
                QRect rect2(tmpRect.bottomRight().x() - 7, tmpRect.bottomRight().y() - 7, 8, 8);

                QPainterPath pathRightBottom;
                pathRightBottom.addRect(rect2);

                // 左侧绘制区域
                pathRightBottom = pathRightBottom.subtracted(path);
                painter->fillPath(pathRightBottom, background);
            }

        }
    }


    painter->fillPath(path, background);

    QStringList lstStr = opt.text.split("\n");
    if (lstStr.size() > 1) {
        QTextDocument  textDoc;
        //设置文字居中显示
        textDoc.setTextWidth(option.rect.width() - 6);//设置文本左边空6px的位置。1. 文本长度减6
        //设置文本内容
        QDomDocument doc;
        getDocFromLst(doc, lstStr);
        textDoc.setHtml(doc.toString());

        // bug111063中 社区版与专业版使用同一代码，主板界面展示效果不同
        // PageBoardInfo 中计算行高方式与html中计算行高方式不同，导致每行下方出现截断或空白
        // 此处获取html整体高度后再对PageBoardInfo设置行高，则不会再出现截断或空白
        dynamic_cast<PageSingleInfo *>(this->parent())->setRowHeight(index.row(), textDoc.size().toSize().height() + 6);

        QAbstractTextDocumentLayout::PaintContext   paintContext;
        paintContext.palette.setCurrentColorGroup(cg);
        QRect  textRect = style->subElementRect(QStyle::SE_ItemViewItemText,  &opt);
        textRect.setWidth(textRect.size().width() - 6);//设置文本左边空6px的位置。2. 显示矩形减6
        textRect.setHeight(textRect.size().height() - 3);
        QPoint point(QPoint(option.rect.x() + 6, option.rect.y() + 3));//设置文本左边空6px的位置。1. 文本长度减6
        painter->save();
        painter->translate(point);
        painter->setClipRect(textRect.translated(-point));
        textDoc.documentLayout()->draw(painter, paintContext);
        painter->restore();
    } else {
        QTextDocument  textDoc;
        //设置文字居中显示
        textDoc.setTextWidth(option.rect.width() - 6);//设置文本左边空6px的位置。1. 文本长度减6
        //设置文本内容
        QDomDocument doc;
        QDomElement p = doc.createElement("p");
        p.setAttribute("width", "100%");
        p.setAttribute("border", "0");
        p.setAttribute("style", "text-align:left;");
        p.setAttribute("style", "font-weight:504;");
        QDomText nameText = doc.createTextNode(opt.text);
        p.appendChild(nameText);
        doc.appendChild(p);
        textDoc.setHtml(doc.toString());

        QAbstractTextDocumentLayout::PaintContext   paintContext;
        paintContext.palette.setCurrentColorGroup(cg);
        QRect  textRect = style->subElementRect(QStyle::SE_ItemViewItemText,  &opt);
        textRect.setWidth(textRect.size().width() - 6);//设置文本左边空6px的位置。2. 显示矩形减6
        textRect.setHeight(textRect.size().height() - 6);
        QPoint point(QPoint(option.rect.x() + 6, option.rect.y() + 6));//设置文本左边空6px的位置。3. 显示矩形位置加6
        painter->save();
        painter->translate(point);
        painter->setClipRect(textRect.translated(-point));
        textDoc.documentLayout()->draw(painter, paintContext);
        painter->restore();
    }

    painter->restore();
}

QWidget *RichTextDelegate::createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return nullptr;
}

QSize RichTextDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(40, size.height()));

    if (index.column()) {
        size.setWidth(std::max(150, size.width()));
    }
    return size;
}

void RichTextDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->showDecorationSelected = true;
    bool ok = false;
    if (index.data(Qt::TextAlignmentRole).isValid()) {
        uint value = index.data(Qt::TextAlignmentRole).toUInt(&ok);
        option->displayAlignment = static_cast<Qt::Alignment>(value);
    }

    if (!ok)
        option->displayAlignment = Qt::AlignLeft | Qt::AlignTop;
    option->textElideMode = Qt::ElideRight;
    option->features = QStyleOptionViewItem::HasDisplay;
    if (index.row() % 2 == 0)
        option->features |= QStyleOptionViewItem::Alternate;
    if (index.data(Qt::DisplayRole).isValid())
        option->text = index.data().toString();
}

void RichTextDelegate::getDocFromLst(QDomDocument &doc, const QStringList &lst)const
{
    QDomElement table = doc.createElement("table");
    table.setAttribute("style", "border-collapse: collapse;border-spacing: 0;");

    int rowWidth = 120;
    QFontMetrics fm(DFontSizeManager::instance()->t8());
    // 计算出第一列宽度
    foreach (auto kv, lst) {
        QStringList keyValue = kv.split(":");
        if (keyValue.size() != 2) {
            return;
        }
        int curWidth = fm.width(keyValue[0] + ":") + 10;
        if (rowWidth < curWidth) rowWidth = curWidth;
    }

    foreach (auto kv, lst) {
        QStringList keyValue = kv.split(":");
        QPair<QString, QString> pair;
        pair.first = keyValue[0];
        pair.second = keyValue[1];

        // 添加一行
        addRow(doc, table, pair, rowWidth);
    }
    // 添加该表格到doc
    doc.appendChild(table);
}

void RichTextDelegate::addRow(QDomDocument &doc, QDomElement &table, const QPair<QString, QString> &pair, const int &rowWidth)const
{
    QDomElement tr = doc.createElement("tr");
//    tr.setAttribute("style", "line-height:100;height:100;");

    // 该行的第一列
    QString nt = pair.first.isEmpty() ? "" : pair.first + ":";
    addTd1(doc, tr, nt, rowWidth);

    // 该行的第二列
    // 如果该列的内容很多则分行显示
    QStringList strList = pair.second.split("  /  \t\t");
    if (strList.size() > 2) {

        QStringList::iterator it = strList.begin();
        addTd2(doc, tr, *it);
        ++it;
        for (; it != strList.end(); ++it) {
            QPair<QString, QString> tempPair;
            tempPair.first = "";
            tempPair.second = *it;
            addRow(doc, tr, tempPair, rowWidth);
        }
    } else {
        addTd2(doc, tr, pair.second);
    }

    table.appendChild(tr);
}

void RichTextDelegate::addTd1(QDomDocument &doc, QDomElement &tr, const QString &value, const int &rowWidth)const
{
    QDomElement td = doc.createElement("td");
    td.setAttribute("width", QString("%1").arg(rowWidth));
    int px = DFontSizeManager::instance()->t8().pixelSize();
    QString fontSize = QString("text-align:left;font-weight:504;font-size:%1px;").arg(px);
    td.setAttribute("style", fontSize);

    QDomText valueText = doc.createTextNode(value);
    td.appendChild(valueText);

    tr.appendChild(td);
}

void RichTextDelegate::addTd2(QDomDocument &doc, QDomElement &tr, const QString &value)const
{
    QDomElement td = doc.createElement("td");
    int px = DFontSizeManager::instance()->t8().pixelSize();
    QString fontSize = QString("font-size:%1px;").arg(px);
    td.setAttribute("style", fontSize);

    QDomText valueText = doc.createTextNode(value);
    td.appendChild(valueText);

    tr.appendChild(td);
}
