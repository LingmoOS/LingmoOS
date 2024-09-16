// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchlistdelegate.h"
#include "grandsearchlistview.h"
#include "global/builtinsearch.h"

#include <DGuiApplicationHelper>

#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QStyleOptionViewItem>
#include <QTextCursor>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QToolTip>

#define ListItemSpace             10       // 列表图标与文本间间距
#define ListItemHeight            36       // 列表行高
#define ListIconSize              24       // 列表图标大小
#define ListRowWidth              740      // 列表行宽
#define ListIconMargin            10       // 列表图标边距
#define TailDataMargin            10       // 拖尾信息显示间隔
#define TailMaxWidth              150      // 拖尾信息最大显示宽度

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GrandSearch;

GrandSearchListDelegate::GrandSearchListDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

GrandSearchListDelegate::~GrandSearchListDelegate()
{

}

void GrandSearchListDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    DStyledItemDelegate::initStyleOption(option, index);
}

bool GrandSearchListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event)
    Q_UNUSED(model)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return true;
}

void GrandSearchListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    // 绘制选中状态(UI设计图暂无选择外框线）
    drawSelectState(painter, option, index);

    // 绘制匹配结果文本
    drawSearchResultText(painter, option, index);
}

QSize GrandSearchListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(ListRowWidth, ListItemHeight);
}

static void hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

bool GrandSearchListDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(Qt::ToolTipRole).toString();

        if (tooltip.isEmpty()) {
            hideTooltipImmediately();
        } else {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
            QToolTip::showText(event->globalPos(), strtooltip, view);
        }

        return true;
    }

    return DStyledItemDelegate::helpEvent(event, view, option, index);
}

void GrandSearchListDelegate::drawSelectState(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    if (option.state & QStyle::State_Selected)
        return;

    if (option.state & QStyle::State_MouseOver) {
        painter->save();
        painter->setPen(Qt::NoPen);

        QColor hovertColor;
        const GrandSearchListView *listview = qobject_cast<const GrandSearchListView *>(option.widget);
        if ((listview->getThemeType() == DGuiApplicationHelper::DarkType)
                || (index.isValid() && index == listview->currentIndex())) {
            hovertColor = QColor(255, 255, 255, static_cast<int>(255 * 0.05));
        } else {
            hovertColor = QColor(0, 0, 0, static_cast<int>(255*0.05));
        }
        painter->setBrush(hovertColor);
        QRect selecteColorRect = option.rect.adjusted(0, 0, 0, 0);
        painter->drawRoundedRect(selecteColorRect, 8, 8);
        painter->restore();
    }

}

void GrandSearchListDelegate::drawSearchResultText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor nameTextColor;
    const GrandSearchListView *listview = qobject_cast<const GrandSearchListView *>(option.widget);

    if ((listview->getThemeType() == DGuiApplicationHelper::DarkType)
            || (index.isValid() && index == listview->currentIndex())) {
        nameTextColor = QColor("#FFFFFF");
    } else {
        nameTextColor = QColor("#000000");
    }

    // 设置字体
    QFont nameFont = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    nameFont.setWeight(QFont::Medium);

    const QString &name = index.data(DATA_ROLE).value<MatchedItem>().name;
    const QString &searcher = index.data(DATA_ROLE).value<MatchedItem>().searcher;

    int listItemTextMaxWidth = option.rect.width() - ListIconMargin - ListIconSize - ListItemSpace;
    QFontMetrics nameFontMetrics(nameFont);
    QString elidedName = nameFontMetrics.elidedText(name, Qt::ElideRight, listItemTextMaxWidth);
    if (elidedName != name && GRANDSEARCH_CLASS_WEB_STATICTEXT == searcher) {
        // 如果存在截断显示，且该项属于web搜索，则截断时需要保留尾部的双引号
        // 根据文档工程师给出的翻译规范，双引号会被翻译，需要使用翻译后的字符计算宽度
        static const QString markStr = name.right(1);
        static const int markWidth = nameFontMetrics.size(Qt::TextSingleLine, markStr).width();
        elidedName = nameFontMetrics.elidedText(name.left(name.count() - 1), Qt::ElideRight, listItemTextMaxWidth - markWidth);
        elidedName.append(markStr);
    }

    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;
    QStyle *pStyle = viewOption.widget ? viewOption.widget->style() : QApplication::style();
    viewOption.text = "";
    pStyle->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

    QTextDocument nameDocument;
    nameDocument.setDocumentMargin(0);
    nameDocument.setPlainText(elidedName);
    QTextCursor nameCursor(&nameDocument);

    nameCursor.beginEditBlock();
    nameCursor.select(QTextCursor::LineUnderCursor);
    QTextCharFormat nameFmt;
    nameFmt.setForeground(nameTextColor);
    nameFmt.setFont(nameFont);
    nameCursor.mergeCharFormat(nameFmt);
    nameCursor.clearSelection();
    nameCursor.movePosition(QTextCursor::EndOfLine);
    nameCursor.endEditBlock();

    // 设置文字边距，保证绘制文字居中
    QAbstractTextDocumentLayout::PaintContext paintContext;
    int nameMargin = static_cast<int>((option.rect.height() - nameFontMetrics.height()) / 2);
    int actualNameWidth = nameFontMetrics.size(Qt::TextSingleLine, elidedName).width();
    int actualStartX = ListIconMargin + ListIconSize + ListItemSpace;
    QRect nameTextRect(actualStartX , option.rect.y() + nameMargin, actualNameWidth, option.rect.height());

    painter->save();
    painter->translate(nameTextRect.topLeft());
    painter->setClipRect(nameTextRect.translated(-nameTextRect.topLeft()));
    nameDocument.documentLayout()->draw(painter, paintContext);
    painter->restore();

    // 未显示预览窗口时，需要显示拖尾数据
    if (!listview->isPreviewItem()) {
        drawTailText(painter, option, index, option.rect.width(), actualStartX + nameTextRect.width());
    }

}

void GrandSearchListDelegate::drawTailText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, int textMaxWidth, int actualStartX) const
{
    QFont tailFont = DFontSizeManager::instance()->get(DFontSizeManager::T8);
    QFontMetrics tailFontMetrics(tailFont);

    // 计算拖尾间隔符号显示宽度
    static QString separator("— ");
    int separatorWidth = tailFontMetrics.size(Qt::TextSingleLine, separator).width();

    // 计算是否有空间显示拖尾信息
    int totalTailWidth = textMaxWidth - actualStartX;
    // 显示拖尾信息前，必须先显示分隔符
    totalTailWidth = totalTailWidth - TailDataMargin - separatorWidth;

    // 存在剩余空间来显示拖尾信息
    if (totalTailWidth > 0) {

        // 提取拖尾数据
        const QVariant &extra = index.data(DATA_ROLE).value<MatchedItem>().extra;
        QStringList tailStringList = extra.toHash().value(GRANDSEARCH_PROPERTY_ITEM_TAILER, QStringList()).toStringList();
        if (!tailStringList.isEmpty()) {

            // 根据剩余宽度，计算能够显示的拖尾数量
            int tailCount = tailStringList.count();
            int tailAverageWidth = totalTailWidth;
            calcTailShowInfo(totalTailWidth, tailCount, tailAverageWidth);
            const QMap<int, QString> &tailMap = calcTailShowData(tailStringList, tailCount, tailAverageWidth, tailFontMetrics);

            if (tailMap.isEmpty())
                return;

            // 设置拖尾字体属性
            QColor tailTextColor;
            const GrandSearchListView *listview = qobject_cast<const GrandSearchListView *>(option.widget);

            if ((listview->getThemeType() == DGuiApplicationHelper::DarkType)
                    || (index.isValid() && index == listview->currentIndex())) {
                tailTextColor = QColor("#FFFFFF");
            } else {
                tailTextColor = QColor("#000000");
            }

            tailTextColor.setAlphaF(0.5);

            // 先绘制分隔符,与名称间隔10个像素
            actualStartX += TailDataMargin;
            drawTailDetailedInfo(painter, option, separator, tailTextColor, tailFont, tailFontMetrics, actualStartX);

            // 再逐个绘制拖尾信息
            for (auto index : tailMap.keys()) {

                if (0 != index) {
                    // 如果不是第一个拖尾数据，则需要保留必要的间隔空间
                    actualStartX += TailDataMargin;
                }

                const QString &showTailInfo = tailMap.value(index);
                drawTailDetailedInfo(painter, option, showTailInfo, tailTextColor, tailFont, tailFontMetrics, actualStartX);
            }
        }
    }
}

void GrandSearchListDelegate::drawTailDetailedInfo(QPainter *painter, const QStyleOptionViewItem &option, const QString &text, const QColor &color, const QFont &font, const QFontMetrics &fontMetrics, int &startX) const
{
    QTextDocument document;
    document.setDocumentMargin(0);
    document.setPlainText(text);
    QTextCursor cursor(&document);

    cursor.beginEditBlock();
    cursor.select(QTextCursor::LineUnderCursor);
    QTextCharFormat fmt;
    fmt.setForeground(color);
    fmt.setFont(font);
    cursor.mergeCharFormat(fmt);
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::EndOfLine);
    cursor.endEditBlock();

    int margin = static_cast<int>((option.rect.height() - fontMetrics.height()) / 2);
    int showWidth = fontMetrics.size(Qt::TextSingleLine, text).width();
    QRect rect(startX, option.rect.y() + margin, showWidth, option.rect.height());

    QAbstractTextDocumentLayout::PaintContext paintContext;
    painter->save();
    painter->translate(rect.topLeft());
    painter->setClipRect(rect.translated(-rect.topLeft()));
    document.documentLayout()->draw(painter, paintContext);
    painter->restore();
    // 更新绘制开始位置
    startX += showWidth;
}

void GrandSearchListDelegate::calcTailShowInfo(const int totalTailWidth, int &tailCount, int &averageWidth) const
{
    Q_ASSERT(tailCount >= 1);

    while (1 != tailCount) {
        // 计算每个拖尾信息平均可显示宽度(每个拖尾信息之间显示间隔10个像素)
        averageWidth = (totalTailWidth - (tailCount - 1) * TailDataMargin) / tailCount;
        if (averageWidth > 0)
            break;
        // 宽度不够显示 (tailCount - 1) * TailDataMargin 个间隔，则减少拖尾数量继续计算
        tailCount--;
        averageWidth = totalTailWidth;
    }
}

QMap<int, QString> GrandSearchListDelegate::calcTailShowData(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const
{
    if (averageWidth >= TailMaxWidth) {
        averageWidth = TailMaxWidth;

        // 平均宽度大于最大限制，则每个项可用的最大宽度均为限制宽度，此时不需要计算剩余空间，再累加给截断显示项进行使用
        return calcTailShowDataByMaxWidth(strings, tailCount, averageWidth, fontMetrics);
    }

    // 平均宽度小于最大限制，则需要针对每个项，计算使用后的剩余空间，并累加后提供给被截断显示的项使用
    return calcTailShowDataByOptimalWidth(strings, tailCount, averageWidth, fontMetrics);
}

QMap<int, QString> GrandSearchListDelegate::calcTailShowDataByMaxWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const
{
    QString tailString;             // 拖尾数据
    QString elidedTailText;         // 实际显示的拖尾数据
    QMap<int, QString> tailMap;     // 最终显示的数据--<拖尾序号，拖尾数据>

    for (int i = 0; i < tailCount; i++) {
        tailString = strings.takeFirst();
        elidedTailText = fontMetrics.elidedText(tailString, Qt::ElideLeft, averageWidth);
        if (!elidedTailText.isEmpty())
            tailMap.insert(i, elidedTailText);
    }

    return tailMap;
}

QMap<int, QString> GrandSearchListDelegate::calcTailShowDataByOptimalWidth(QStringList &strings, const int &tailCount, int averageWidth, const QFontMetrics &fontMetrics) const
{
    QString tailString;             // 拖尾数据
    QMap<int, QString> tailMap;     // 最终显示的数据--<拖尾序号，拖尾数据>
    QMap<int, QString> pendMap;     // 推迟计算的拖尾数据(仅使用平均空间时会发生截断的拖尾数据)
    int unUsedWidth = 0;            // 未使用空间
    // 提取能够全量显示的拖尾信息
    for (int i = 0; i < tailCount ; i++) {
        // 从前向后，逐个取出数据进行计算
        tailString = strings.takeFirst();
        int currentTailWidth = fontMetrics.size(Qt::TextSingleLine, tailString).width();
        if (currentTailWidth < averageWidth) {
            // 该项拖尾数据能够全量显示，则保存，并累加剩余空间
            unUsedWidth += averageWidth - currentTailWidth;
            if (!tailString.isEmpty())
                tailMap.insert(i, tailString);
            continue;
        }
        pendMap.insert(i, tailString);
    }

    // 提取被截断显示的拖尾信息
    int availableWidth;             // 当前项可利用的最大宽度
    QString pendString;             // 待处理的原始信息
    QString elidedTailText;         // 截断显示的拖尾数据
    for (auto index : pendMap.keys()) {

        availableWidth = averageWidth;
        if (unUsedWidth > 0) {
            availableWidth += unUsedWidth;
            unUsedWidth = 0;

            if (availableWidth > TailMaxWidth) {
                // 退还超出限制的空间
                unUsedWidth = availableWidth - TailMaxWidth;
                availableWidth = TailMaxWidth;
            }
        }

        pendString = pendMap.value(index);
        elidedTailText = fontMetrics.elidedText(pendString, Qt::ElideLeft, availableWidth);
        if (elidedTailText == pendString) {
            // 未截断显示，说明设置的可用空间还有剩余，需要退还给未使用空间
            int elidedTailWidth = fontMetrics.size(Qt::TextSingleLine, pendString).width();
            unUsedWidth += availableWidth - elidedTailWidth;
        }

        if (!elidedTailText.isEmpty())
            tailMap.insert(index, elidedTailText);
        pendMap.remove(index);
    }

    return tailMap;
}

