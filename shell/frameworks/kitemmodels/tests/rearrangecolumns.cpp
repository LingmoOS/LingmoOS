/*
    SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: David Faure <david.faure@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KRearrangeColumnsProxyModel>
#include <QApplication>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

// Prepares one row for a QStandardItemModel
inline QList<QStandardItem *> makeStandardItems(const QStringList &texts)
{
    QList<QStandardItem *> items;
    items.reserve(texts.count());
    for (const QString &txt : texts) {
        items << new QStandardItem(txt);
    }
    return items;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QStandardItemModel source;
    source.insertRow(0, makeStandardItems({QStringLiteral("A0"), QStringLiteral("B0"), QStringLiteral("C0"), QStringLiteral("D0")}));
    source.insertRow(1, makeStandardItems({QStringLiteral("A1"), QStringLiteral("B1"), QStringLiteral("C1"), QStringLiteral("D1")}));
    source.insertRow(2, makeStandardItems({QStringLiteral("A2"), QStringLiteral("B2"), QStringLiteral("C2"), QStringLiteral("D2")}));
    source.setHorizontalHeaderLabels({QStringLiteral("H1"), QStringLiteral("H2"), QStringLiteral("H3"), QStringLiteral("H4")});

    KRearrangeColumnsProxyModel pm;
    pm.setSourceColumns(QList<int>{2, 3, 1, 0});
    pm.setSourceModel(&source);

    QTreeView treeView;
    treeView.setModel(&pm);
    treeView.show();

    QTimer::singleShot(500, &pm, [&]() {
        pm.setSourceColumns(QList<int>{2, 1, 0, 3});
    });

    return app.exec();
}
