/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecompletiondelegate.h"

#include <ktexteditor/codecompletionmodel.h>

#include "katecompletionmodel.h"
#include "katepartdebug.h"

#include <QApplication>
#include <QPainter>

KateCompletionDelegate::KateCompletionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

static void paintItemViewText(QPainter *p, const QString &text, const QStyleOptionViewItem &options, QList<QTextLayout::FormatRange> formats)
{
    // set formats
    QTextLayout textLayout(text, options.font, p->device());
    auto fmts = textLayout.formats();
    formats.append(fmts);
    textLayout.setFormats(formats);

    // set alignment, rtls etc
    QTextOption textOption;
    textOption.setTextDirection(options.direction);
    textOption.setAlignment(QStyle::visualAlignment(options.direction, options.displayAlignment));
    textLayout.setTextOption(textOption);

    // layout the text
    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();
    if (!line.isValid())
        return;

    const int lineWidth = options.rect.width();
    line.setLineWidth(lineWidth);
    line.setPosition(QPointF(0, 0));

    textLayout.endLayout();

    int y = QStyle::alignedRect(Qt::LayoutDirectionAuto, options.displayAlignment, textLayout.boundingRect().size().toSize(), options.rect).y();

    // draw the text
    const auto pos = QPointF(options.rect.x(), y);
    textLayout.draw(p, pos);
}

void KateCompletionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &o, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = o;
    initStyleOption(&opt, index);
    QString text = opt.text;

    if (text.isEmpty()) {
        QStyledItemDelegate::paint(painter, o, index);
        return;
    }

    auto *style = opt.widget->style() ? opt.widget->style() : qApp->style();

    opt.text.clear();

    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);

    const bool isGroup = index.data(KateCompletionModel::IsNonEmptyGroup).toBool();
    if (!isGroup && !opt.features.testFlag(QStyleOptionViewItem::HasDecoration)) {
        // 3 because 2 margins for the icon, and one left margin for the text
        int hMargins = style->pixelMetric(QStyle::PM_FocusFrameHMargin) * 3;
        textRect.adjust(hMargins + opt.decorationSize.width(), 0, 0, 0);
    }

#if 0
    auto p = painter->pen();
    painter->setPen(Qt::yellow);
    painter->drawRect(opt.rect);

    painter->setPen(Qt::red);
    painter->drawRect(textRect);
    painter->setPen(p);
#endif

    auto highlightings = createHighlighting(index);
    opt.rect = textRect;
    opt.displayAlignment = m_alignTop ? Qt::AlignTop : Qt::AlignVCenter;
    paintItemViewText(painter, text, opt, std::move(highlightings));
}

QSize KateCompletionDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().toString().isEmpty()) {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (!index.data(Qt::DecorationRole).isNull()) {
        return size;
    }
    const int hMargins = option.widget->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) * 3;
    size.rwidth() += option.decorationSize.width() + hMargins;
    return size;
}

static QList<QTextLayout::FormatRange> highlightingFromVariantList(const QList<QVariant> &customHighlights)
{
    QList<QTextLayout::FormatRange> ret;

    for (int i = 0; i + 2 < customHighlights.count(); i += 3) {
        if (!customHighlights[i].canConvert<int>() || !customHighlights[i + 1].canConvert<int>() || !customHighlights[i + 2].canConvert<QTextFormat>()) {
            qCWarning(LOG_KTE) << "Unable to convert triple to custom formatting.";
            continue;
        }

        QTextLayout::FormatRange format;
        format.start = customHighlights[i].toInt();
        format.length = customHighlights[i + 1].toInt();
        format.format = customHighlights[i + 2].value<QTextFormat>().toCharFormat();

        if (!format.format.isValid()) {
            qCWarning(LOG_KTE) << "Format is not valid";
        }

        ret << format;
    }
    return ret;
}

QList<QTextLayout::FormatRange> KateCompletionDelegate::createHighlighting(const QModelIndex &index)
{
    QVariant highlight = index.data(KTextEditor::CodeCompletionModel::HighlightingMethod);

    // TODO: config enable specifying no highlight as default
    int highlightMethod = KTextEditor::CodeCompletionModel::InternalHighlighting;
    if (highlight.canConvert<int>()) {
        highlightMethod = highlight.toInt();
    }

    if (highlightMethod & KTextEditor::CodeCompletionModel::CustomHighlighting) {
        return highlightingFromVariantList(index.data(KTextEditor::CodeCompletionModel::CustomHighlight).toList());
    }

    return {};
}
