/*
    SPDX-FileCopyrightText: 2015 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: David Faure <david.faure@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TEST_MODEL_HELPERS_H
#define TEST_MODEL_HELPERS_H

#include <QSignalSpy>
#include <QStandardItem>
#include <QString>

namespace TestModelHelpers
{
// Prepares one row for a QStandardItemModel
inline QList<QStandardItem *> makeStandardItems(const QStringList &texts)
{
    QList<QStandardItem *> items;
    for (const QString &txt : texts) {
        items << new QStandardItem(txt);
    }
    return items;
}

// Extracts a full row from a model as a string
// Works best if every cell contains only one character
inline QString extractRowTexts(QAbstractItemModel *model, int row, const QModelIndex &parent = QModelIndex())
{
    QString result;
    const int colCount = model->columnCount();
    for (int col = 0; col < colCount; ++col) {
        const QString txt = model->index(row, col, parent).data().toString();
        result += txt.isEmpty() ? QStringLiteral(" ") : txt;
    }
    return result;
}

// Extracts all headers
inline QString extractHorizontalHeaderTexts(QAbstractItemModel *model)
{
    QString result;
    const int colCount = model->columnCount();
    for (int col = 0; col < colCount; ++col) {
        const QString txt = model->headerData(col, Qt::Horizontal).toString();
        result += txt.isEmpty() ? QStringLiteral(" ") : txt;
    }
    return result;
}

inline QString rowSpyToText(const QSignalSpy &spy)
{
    if (!spy.isValid()) {
        return QStringLiteral("THE SIGNALSPY IS INVALID!");
    }
    QString str;
    for (int i = 0; i < spy.count(); ++i) {
        str += spy.at(i).at(1).toString() + QLatin1Char(',') + spy.at(i).at(2).toString();
        if (i + 1 < spy.count()) {
            str += QLatin1Char(';');
        }
    }
    return str;
}

}
#endif
