/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "expandingwidgetmodel.h"

#include <QApplication>
#include <QBrush>
#include <QModelIndex>
#include <QTextEdit>
#include <QTreeView>

#include <KColorUtils>
#include <ktexteditor/codecompletionmodel.h>

#include "katepartdebug.h"

using namespace KTextEditor;

inline QModelIndex firstColumn(const QModelIndex &index)
{
    return index.sibling(index.row(), 0);
}

ExpandingWidgetModel::ExpandingWidgetModel(QWidget *parent)
    : QAbstractItemModel(parent)
{
}

ExpandingWidgetModel::~ExpandingWidgetModel()
{
}

static QColor doAlternate(const QColor &color)
{
    QColor background = QApplication::palette().window().color();
    return KColorUtils::mix(color, background, 0.15);
}

uint ExpandingWidgetModel::matchColor(const QModelIndex &index) const
{
    int matchQuality = contextMatchQuality(index.sibling(index.row(), 0));

    if (matchQuality > 0) {
        bool alternate = index.row() & 1;

        QColor badMatchColor(0xff00aa44); // Blue-ish green
        QColor goodMatchColor(0xff00ff00); // Green

        QColor background = treeView()->palette().light().color();

        QColor totalColor = KColorUtils::mix(badMatchColor, goodMatchColor, ((float)matchQuality) / 10.0);

        if (alternate) {
            totalColor = doAlternate(totalColor);
        }

        const qreal dynamicTint = 0.2;
        const qreal minimumTint = 0.2;
        qreal tintStrength = (dynamicTint * matchQuality) / 10;
        if (tintStrength != 0.0) {
            tintStrength += minimumTint; // Some minimum tinting strength, else it's not visible any more
        }

        return KColorUtils::tint(background, totalColor, tintStrength).rgb();
    } else {
        return 0;
    }
}

QVariant ExpandingWidgetModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::BackgroundRole: {
        if (index.column() == 0) {
            // Highlight by match-quality
            uint color = matchColor(index);
            if (color) {
                return QBrush(color);
            }
        }
    }
    }
    return QVariant();
}

QList<QVariant> mergeCustomHighlighting(int leftSize, const QList<QVariant> &left, int rightSize, const QList<QVariant> &right)
{
    QList<QVariant> ret = left;
    if (left.isEmpty()) {
        ret << QVariant(0);
        ret << QVariant(leftSize);
        ret << QTextFormat(QTextFormat::CharFormat);
    }

    if (right.isEmpty()) {
        ret << QVariant(leftSize);
        ret << QVariant(rightSize);
        ret << QTextFormat(QTextFormat::CharFormat);
    } else {
        QList<QVariant>::const_iterator it = right.constBegin();
        while (it != right.constEnd()) {
            {
                QList<QVariant>::const_iterator testIt = it;
                for (int a = 0; a < 2; a++) {
                    ++testIt;
                    if (testIt == right.constEnd()) {
                        qCWarning(LOG_KTE) << "Length of input is not multiple of 3";
                        break;
                    }
                }
            }

            ret << QVariant((*it).toInt() + leftSize);
            ++it;
            ret << QVariant((*it).toInt());
            ++it;
            ret << *it;
            if (!(*it).value<QTextFormat>().isValid()) {
                qCDebug(LOG_KTE) << "Text-format is invalid";
            }
            ++it;
        }
    }
    return ret;
}

// It is assumed that between each two strings, one space is inserted
QList<QVariant> mergeCustomHighlighting(QStringList strings, QList<QVariantList> highlights, int grapBetweenStrings)
{
    if (strings.isEmpty()) {
        qCWarning(LOG_KTE) << "List of strings is empty";
        return QList<QVariant>();
    }

    if (highlights.isEmpty()) {
        qCWarning(LOG_KTE) << "List of highlightings is empty";
        return QList<QVariant>();
    }

    if (strings.count() != highlights.count()) {
        qCWarning(LOG_KTE) << "Length of string-list is " << strings.count() << " while count of highlightings is " << highlights.count() << ", should be same";
        return QList<QVariant>();
    }

    // Merge them together
    QString totalString = strings[0];
    QVariantList totalHighlighting = highlights[0];

    strings.pop_front();
    highlights.pop_front();

    while (!strings.isEmpty()) {
        totalHighlighting = mergeCustomHighlighting(totalString.length(), totalHighlighting, strings[0].length(), highlights[0]);
        totalString += strings[0];

        for (int a = 0; a < grapBetweenStrings; a++) {
            totalString += QLatin1Char(' ');
        }

        strings.pop_front();
        highlights.pop_front();
    }
    // Combine the custom-highlightings
    return totalHighlighting;
}
