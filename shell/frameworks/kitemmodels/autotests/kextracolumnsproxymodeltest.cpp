/*
    SPDX-FileCopyrightText: 2015 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: David Faure <david.faure@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QItemSelectionModel>
#include <QSignalSpy>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTest>
#include <QTreeView>

#include "dynamictreemodel.h"

#include "test_model_helpers.h"
#include <kextracolumnsproxymodel.h>
using namespace TestModelHelpers;

Q_DECLARE_METATYPE(QModelIndex)

class tst_KExtraColumnsProxyModel : public QObject
{
    Q_OBJECT

private:
    class NoExtraColumns : public KExtraColumnsProxyModel
    {
        QVariant extraColumnData(const QModelIndex &, int, int, int) const override
        {
            Q_ASSERT(0);
            return QVariant();
        }
    };

    class TwoExtraColumnsProxyModel : public KExtraColumnsProxyModel
    {
    public:
        TwoExtraColumnsProxyModel()
            : KExtraColumnsProxyModel()
            , m_extraColumnData('Z')
        {
            appendColumn(QStringLiteral("H5"));
            appendColumn(QStringLiteral("WRONG")); // removed two lines below, just to test removeColumn
            appendColumn(QStringLiteral("H6"));
            removeExtraColumn(1);
        }
        QVariant extraColumnData(const QModelIndex &, int row, int extraColumn, int role) const override
        {
            if (role != Qt::DisplayRole) {
                return QVariant();
            }
            switch (extraColumn) {
            case 0:
                return QString(m_extraColumnData);
            case 1:
                return QString::number(row);
            default:
                Q_ASSERT(0);
                return QVariant();
            }
        }
        bool setExtraColumnData(const QModelIndex &parent, int row, int extraColumn, const QVariant &data, int role) override
        {
            if (extraColumn == 0 && role == Qt::EditRole) {
                m_extraColumnData = data.toString().at(0);
                extraColumnDataChanged(QModelIndex(), 0, extraColumn, QList<int>() << Qt::EditRole);
                return true;
            }
            return KExtraColumnsProxyModel::setExtraColumnData(parent, row, extraColumn, data, role);
        }
        void changeExtraColumnData()
        {
            m_extraColumnData = '<';
            extraColumnDataChanged(QModelIndex(), 0, 0, QList<int>() << Qt::EditRole);
        }

    private:
        QChar m_extraColumnData;
    };

private Q_SLOTS:

    void initTestCase()
    {
        qRegisterMetaType<QModelIndex>();
    }

    void init()
    {
        // Prepare the source model to use later on
        mod.clear();
        mod.appendRow(makeStandardItems(QStringList() << QStringLiteral("A") << QStringLiteral("B") << QStringLiteral("C") << QStringLiteral("D")));
        mod.item(0, 0)->appendRow(makeStandardItems(QStringList() << QStringLiteral("m") << QStringLiteral("n") << QStringLiteral("o") << QStringLiteral("p")));
        mod.item(0, 0)->appendRow(makeStandardItems(QStringList() << QStringLiteral("q") << QStringLiteral("r") << QStringLiteral("s") << QStringLiteral("t")));
        mod.item(0, 0)->child(1, 0)->appendRow(
            makeStandardItems(QStringList() << QStringLiteral("u") << QStringLiteral("v") << QStringLiteral("w") << QStringLiteral("w")));
        mod.appendRow(makeStandardItems(QStringList() << QStringLiteral("E") << QStringLiteral("F") << QStringLiteral("G") << QStringLiteral("H")));
        mod.item(1, 0)->appendRow(makeStandardItems(QStringList() << QStringLiteral("x") << QStringLiteral("y") << QStringLiteral("z") << QStringLiteral(".")));
        mod.setHorizontalHeaderLabels(QStringList() << QStringLiteral("H1") << QStringLiteral("H2") << QStringLiteral("H3") << QStringLiteral("H4"));

        QCOMPARE(extractRowTexts(&mod, 0), QStringLiteral("ABCD"));
        QCOMPARE(extractRowTexts(&mod, 0, mod.index(0, 0)), QStringLiteral("mnop"));
        QCOMPARE(extractRowTexts(&mod, 1, mod.index(0, 0)), QStringLiteral("qrst"));
        QCOMPARE(extractRowTexts(&mod, 0, mod.index(1, 0, mod.index(0, 0))), QStringLiteral("uvww"));
        QCOMPARE(extractRowTexts(&mod, 1), QStringLiteral("EFGH"));
        QCOMPARE(extractRowTexts(&mod, 0, mod.index(1, 0)), QStringLiteral("xyz."));
        QCOMPARE(extractHorizontalHeaderTexts(&mod), QStringLiteral("H1H2H3H4"));
    }

    void shouldDoNothingIfNoExtraColumns()
    {
        // Given a extra-columns proxy
        NoExtraColumns pm;

        // When setting it to a source model
        pm.setSourceModel(&mod);

        // Then the proxy should show the same as the model
        QCOMPARE(pm.rowCount(), mod.rowCount());
        QCOMPARE(pm.columnCount(), mod.columnCount());

        QCOMPARE(pm.rowCount(pm.index(0, 0)), 2);
        QCOMPARE(pm.index(0, 0).parent(), QModelIndex());

        // (verify that the mapFromSource(mapToSource(x)) == x roundtrip works)
        for (int row = 0; row < pm.rowCount(); ++row) {
            for (int col = 0; col < pm.columnCount(); ++col) {
                QCOMPARE(pm.mapFromSource(pm.mapToSource(pm.index(row, col))), pm.index(row, col));
            }
        }

        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABCD"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(0, 0)), QStringLiteral("mnop"));
        QCOMPARE(extractRowTexts(&pm, 1, pm.index(0, 0)), QStringLiteral("qrst"));
        QCOMPARE(extractRowTexts(&pm, 1), QStringLiteral("EFGH"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(1, 0)), QStringLiteral("xyz."));
        QCOMPARE(extractHorizontalHeaderTexts(&pm), QStringLiteral("H1H2H3H4"));
    }

    void shouldShowExtraColumns()
    {
        // Given a extra-columns proxy with two extra columns
        TwoExtraColumnsProxyModel pm;

        // When setting it to a source model
        pm.setSourceModel(&mod);

        // Then the proxy should show the extra column
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABCDZ0"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(0, 0)), QStringLiteral("mnopZ0"));
        QCOMPARE(extractRowTexts(&pm, 1, pm.index(0, 0)), QStringLiteral("qrstZ1"));
        QCOMPARE(extractRowTexts(&pm, 1), QStringLiteral("EFGHZ1"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(1, 0)), QStringLiteral("xyz.Z0"));
        QCOMPARE(extractHorizontalHeaderTexts(&pm), QStringLiteral("H1H2H3H4H5H6"));

        // Verify tree structure of proxy
        const QModelIndex secondParent = pm.index(1, 0);
        QVERIFY(!secondParent.parent().isValid());
        QCOMPARE(indexToText(pm.index(0, 0, secondParent).parent()), indexToText(secondParent));
        QCOMPARE(indexToText(pm.index(0, 3, secondParent).parent()), indexToText(secondParent));
        QVERIFY(indexToText(pm.index(0, 4)).startsWith(QLatin1String("0,4,")));
        QCOMPARE(indexToText(pm.index(0, 4, secondParent).parent()), indexToText(secondParent));
        QVERIFY(indexToText(pm.index(0, 5)).startsWith(QLatin1String("0,5,")));
        QCOMPARE(indexToText(pm.index(0, 5, secondParent).parent()), indexToText(secondParent));

        QCOMPARE(pm.index(0, 0).sibling(0, 4).column(), 4);
        QCOMPARE(pm.index(0, 4).sibling(0, 1).column(), 1);

        QCOMPARE(pm.buddy(pm.index(0, 0)), pm.index(0, 0));
        QCOMPARE(pm.buddy(pm.index(0, 4)), pm.index(0, 4));

        QVERIFY(pm.hasChildren(pm.index(0, 0)));
        QVERIFY(!pm.hasChildren(pm.index(0, 4)));

        QVERIFY(!pm.canFetchMore(QModelIndex()));
    }

    void shouldHandleDataChanged()
    {
        // Given a extra-columns proxy, with two extra columns
        TwoExtraColumnsProxyModel pm;
        setup(pm);
        QSignalSpy dataChangedSpy(&pm, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // When a cell in a source model changes
        mod.item(0, 2)->setData("c", Qt::EditRole);

        // Then the change should be notified to the proxy
        QCOMPARE(dataChangedSpy.count(), 1);
        QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), pm.index(0, 2));
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABcDZ0"));
    }

    void shouldHandleDataChangedInExtraColumn()
    {
        // Given a extra-columns proxy, with two extra columns
        TwoExtraColumnsProxyModel pm;
        setup(pm);
        QSignalSpy dataChangedSpy(&pm, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // When the proxy wants to signal a change in an extra column
        pm.changeExtraColumnData();

        // Then the change should be available and notified
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABCD<0"));
        QCOMPARE(dataChangedSpy.count(), 1);
        QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), pm.index(0, 4));
    }

    void shouldHandleSetDataInNormalColumn()
    {
        // Given a extra-columns proxy, with two extra columns
        TwoExtraColumnsProxyModel pm;
        setup(pm);
        QSignalSpy dataChangedSpy(&pm, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // When editing a cell in the proxy
        QVERIFY(pm.setData(pm.index(0, 2), "c", Qt::EditRole));

        // Then the change should be available and notified
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABcDZ0"));
        QCOMPARE(dataChangedSpy.count(), 1);
        QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), pm.index(0, 2));
    }

    void shouldHandleSetDataInExtraColumn()
    {
        // Given a extra-columns proxy, with two extra columns
        TwoExtraColumnsProxyModel pm;
        setup(pm);
        QSignalSpy dataChangedSpy(&pm, SIGNAL(dataChanged(QModelIndex, QModelIndex)));

        // When editing a cell in the proxy
        QVERIFY(pm.setData(pm.index(0, 4), "-", Qt::EditRole));

        // Then the change should be available and notified
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABCD-0"));
        QCOMPARE(dataChangedSpy.count(), 1);
        QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), pm.index(0, 4));
    }

    void shouldHandleRowInsertion()
    {
        // Given a extra-columns proxy, with two extra columns
        TwoExtraColumnsProxyModel pm;
        setup(pm);

        QSignalSpy rowATBISpy(&pm, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy rowInsertedSpy(&pm, SIGNAL(rowsInserted(QModelIndex, int, int)));

        // When a source model inserts a new (child) row
        mod.item(1, 0)->appendRow(makeStandardItems(QStringList() << QStringLiteral("1") << QStringLiteral("2") << QStringLiteral("3") << QStringLiteral("4")));

        // Then the proxy should notify its users and show changes
        QCOMPARE(rowSpyToText(rowATBISpy), QStringLiteral("1,1"));
        QCOMPARE(rowSpyToText(rowInsertedSpy), QStringLiteral("1,1"));
        QCOMPARE(pm.rowCount(), 2);
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABCDZ0"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(0, 0)), QStringLiteral("mnopZ0"));
        QCOMPARE(extractRowTexts(&pm, 1, pm.index(0, 0)), QStringLiteral("qrstZ1"));
        QCOMPARE(extractRowTexts(&pm, 1), QStringLiteral("EFGHZ1"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(1, 0)), QStringLiteral("xyz.Z0"));
        QCOMPARE(extractRowTexts(&pm, 1, pm.index(1, 0)), QStringLiteral("1234Z1"));
        QCOMPARE(extractHorizontalHeaderTexts(&pm), QStringLiteral("H1H2H3H4H5H6"));
    }

    void shouldHandleColumnInsertion()
    {
        // Given a extra-columns proxy, with two extra columns
        TwoExtraColumnsProxyModel pm;
        setup(pm);

        QCOMPARE(pm.columnCount(), 6);
        QCOMPARE(mod.columnCount(), 4);

        QSignalSpy colATBISpy(&pm, SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)));
        QSignalSpy colInsertedSpy(&pm, SIGNAL(columnsInserted(QModelIndex, int, int)));

        // When a source model inserts a new column
        mod.setColumnCount(5); // like QStandardItem::setChild does
        QCOMPARE(mod.columnCount(), 5);
        // QStandardItemModel is quite dumb, it records the number of columns in each item
        for (int row = 0; row < mod.rowCount(); ++row) {
            mod.item(row, 0)->setColumnCount(5);
        }

        // Then the proxy should notify its users and show changes
        QCOMPARE(rowSpyToText(colATBISpy), QStringLiteral("4,4;4,4;4,4")); // QStandardItemModel emits it for each parent
        QCOMPARE(rowSpyToText(colInsertedSpy), QStringLiteral("4,4;4,4;4,4"));
        QCOMPARE(pm.columnCount(), 7);
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABCD Z0"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(0, 0)), QStringLiteral("mnop Z0"));
        QCOMPARE(extractRowTexts(&pm, 1, pm.index(0, 0)), QStringLiteral("qrst Z1"));
        QCOMPARE(extractRowTexts(&pm, 1), QStringLiteral("EFGH Z1"));
        QCOMPARE(extractRowTexts(&pm, 0, pm.index(1, 0)), QStringLiteral("xyz. Z0"));
        QCOMPARE(extractHorizontalHeaderTexts(&pm), QStringLiteral("H1H2H3H45H5H6")); // '5' was inserted in there
    }

    // row removal, layoutChanged, modelReset -> same thing, works via QIdentityProxyModel
    // missing: test for mapSelectionToSource
    // missing: test for moving a row in an underlying model. Problem: QStandardItemModel doesn't implement moveRow...

    void shouldHandleLayoutChanged()
    {
        // Given a extra-columns proxy, with two extra columns
        TwoExtraColumnsProxyModel pm;
        // And a QSFPM underneath
        QSortFilterProxyModel proxy;
        proxy.setSourceModel(&mod);
        QCOMPARE(proxy.columnCount(), 4);
        pm.setSourceModel(&proxy);
        QCOMPARE(pm.columnCount(), 6);
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("ABCDZ0"));
        // And a selection
        QItemSelectionModel selection(&pm);
        selection.select(pm.index(0, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        const QModelIndex grandChild = pm.index(0, 0, pm.index(1, 0, pm.index(0, 0)));
        QCOMPARE(grandChild.data().toString(), QStringLiteral("u"));
        selection.select(grandChild, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        const QModelIndexList lst = selection.selectedIndexes();
        QCOMPARE(lst.count(), 12);
        for (int col = 0; col < 6; ++col) {
            QCOMPARE(lst.at(col).row(), 0);
            QCOMPARE(lst.at(col).column(), col);
        }

        // When sorting
        pm.sort(0, Qt::DescendingOrder);

        // Then the proxy should be sorted
        QCOMPARE(extractRowTexts(&pm, 0), QStringLiteral("EFGHZ0"));
        QCOMPARE(extractRowTexts(&pm, 1), QStringLiteral("ABCDZ1"));
        // And the selection should be updated accordingly
        const QModelIndexList lstAfter = selection.selectedIndexes();
        QCOMPARE(lstAfter.count(), 12);
        for (int col = 0; col < 6; ++col) {
            QCOMPARE(lstAfter.at(col).row(), 1);
            QCOMPARE(lstAfter.at(col).column(), col);
        }
    }

    void persistIndexOnLayoutChange()
    {
        DynamicTreeModel model;

        ModelResetCommand resetCommand(&model);

        resetCommand.setInitialTree(
            " - 1"
            " - - 2"
            " - - - 3"
            " - - - - 4"
            " - - - - 5");
        resetCommand.doCommand();

        NoExtraColumns proxy;
        proxy.setSourceModel(&model);

        QPersistentModelIndex persistentIndex;

        QPersistentModelIndex persistentIndexToMove = model.match(model.index(0, 0), Qt::DisplayRole, "4", 1, Qt::MatchRecursive).first();
        QCOMPARE(persistentIndexToMove.row(), 0);
        QCOMPARE(persistentIndexToMove.parent(), model.match(model.index(0, 0), Qt::DisplayRole, "3", 1, Qt::MatchRecursive).first());

        QPersistentModelIndex sourcePersistentIndex = model.match(model.index(0, 0), Qt::DisplayRole, "5", 1, Qt::MatchRecursive).first();

        QCOMPARE(sourcePersistentIndex.data().toString(), QStringLiteral("5"));

        bool gotLayoutAboutToBeChanged = false;
        bool gotLayoutChanged = false;

        QObject::connect(&proxy, &QAbstractItemModel::layoutAboutToBeChanged, &proxy, [&proxy, &persistentIndex, &gotLayoutAboutToBeChanged] {
            gotLayoutAboutToBeChanged = true;
            persistentIndex = proxy.match(proxy.index(0, 0), Qt::DisplayRole, "5", 1, Qt::MatchRecursive).first();
            QCOMPARE(persistentIndex.row(), 1);
        });

        QObject::connect(&proxy, &QAbstractItemModel::layoutChanged, &proxy, [&proxy, &persistentIndex, &sourcePersistentIndex, &gotLayoutChanged] {
            gotLayoutChanged = true;
            QCOMPARE(QModelIndex(persistentIndex), proxy.mapFromSource(sourcePersistentIndex));
        });

        ModelMoveLayoutChangeCommand layoutChangeCommand(&model, nullptr);

        layoutChangeCommand.setAncestorRowNumbers({0, 0, 0});
        layoutChangeCommand.setStartRow(0);
        layoutChangeCommand.setEndRow(0);
        layoutChangeCommand.setDestAncestors({0, 0});
        layoutChangeCommand.setDestRow(1);

        layoutChangeCommand.doCommand();

        QCOMPARE(persistentIndex.row(), 0);

        QCOMPARE(persistentIndexToMove.row(), 1);
        QCOMPARE(persistentIndexToMove.parent(), model.match(model.index(0, 0), Qt::DisplayRole, "2", 1, Qt::MatchRecursive).first());

        QVERIFY(gotLayoutAboutToBeChanged);
        QVERIFY(gotLayoutChanged);
        QVERIFY(persistentIndex.isValid());
    }

private:
    void setup(KExtraColumnsProxyModel &pm)
    {
        pm.setSourceModel(&mod);
    }

    static QString indexToText(const QModelIndex &index)
    {
        if (!index.isValid()) {
            return QStringLiteral("invalid");
        }
        return QString::number(index.row()) + "," + QString::number(index.column()) + ","
            + QString::number(reinterpret_cast<qulonglong>(index.internalPointer()), 16) + " in "
            + QString::number(reinterpret_cast<qulonglong>(index.model()), 16);
    }

    QStandardItemModel mod;
};

QTEST_MAIN(tst_KExtraColumnsProxyModel)

#include "kextracolumnsproxymodeltest.moc"
