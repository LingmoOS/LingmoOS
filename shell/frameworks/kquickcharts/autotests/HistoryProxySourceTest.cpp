/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QTest>

#include "datasource/HistoryProxySource.h"
#include "datasource/ModelSource.h"
#include "datasource/SingleValueSource.h"

#define qs QStringLiteral

class TestModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int minimum READ minimum NOTIFY minimumChanged)
    Q_PROPERTY(int maximum READ maximum NOTIFY maximumChanged)

public:
    TestModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
    {
        m_items.fill(1, 10);
    }

    int rowCount(const QModelIndex &parent) const override
    {
        if (parent.isValid()) {
            return 0;
        } else {
            return 10;
        }
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        Q_UNUSED(role)

        if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
            qDebug() << "index invalid";
            return QVariant{};
        }

        return m_items.at(index.row());
    }

    void set(int item, int value)
    {
        if (item < 0 || item >= 10) {
            return;
        }

        m_items[item] = value;
        Q_EMIT dataChanged(index(item, 0), index(item, 0));
        Q_EMIT minimumChanged();
        Q_EMIT maximumChanged();
    }

    int minimum()
    {
        return *std::min_element(m_items.cbegin(), m_items.cend());
    }

    int maximum()
    {
        return *std::max_element(m_items.cbegin(), m_items.cend());
    }

    Q_SIGNAL void minimumChanged();
    Q_SIGNAL void maximumChanged();

private:
    QList<int> m_items;
};

class HistoryProxySourceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreate()
    {
        // Basic creation should create an empty source.
        auto source = new HistoryProxySource{};

        QCOMPARE(source->itemCount(), 0);
        QCOMPARE(source->item(0), QVariant{});
        QCOMPARE(source->minimum(), QVariant{});
        QCOMPARE(source->maximum(), QVariant{});
        QCOMPARE(source->source(), nullptr);
    }

    void testWithValue()
    {
        auto valueSource = std::make_unique<SingleValueSource>();

        // Simple case first, using default values for maximum history and fill
        // mode. This should lead to 10 history items with the most recent value
        // being the first item.
        auto historySource = std::make_unique<HistoryProxySource>();
        historySource->setSource(valueSource.get());
        historySource->setMaximumHistory(10);
        historySource->setFillMode(HistoryProxySource::DoNotFill);

        for (int i = 0; i < 15; i++) {
            valueSource->setValue(i);
            QCOMPARE(historySource->itemCount(), std::min(i + 1, 10));

            for (int item = 0; item < historySource->itemCount(); ++item) {
                QCOMPARE(historySource->item(item), i - item);
            }
        }

        // Minimum and maximum should be minimum and maximum of current history
        // items.
        QCOMPARE(historySource->minimum(), 5);
        QCOMPARE(historySource->maximum(), 14);

        // Clearing will reset the history.
        QCOMPARE(historySource->itemCount(), 10);
        historySource->clear();
        QCOMPARE(historySource->itemCount(), 0);

        valueSource->setValue(5);

        // With an interval, we should be getting maximumHistory items of the
        // same value if we simply wait for at least interval * 10 milliseconds.
        historySource->setInterval(50);

        QTest::qWait(750);

        QCOMPARE(historySource->itemCount(), 10);
        for (int i = 0; i < historySource->itemCount(); ++i) {
            QCOMPARE(historySource->item(i), 5);
        }

        historySource->setInterval(0);

        // Increasing history size does not delete existing history.
        historySource->setMaximumHistory(20);

        QCOMPARE(historySource->itemCount(), 10);

        // Changing fill mode resets history and should now report
        // maximumHistory number of items. Empty items should use the same
        // QVariant type as the value source does.

        historySource->setFillMode(HistoryProxySource::FillFromStart);

        QCOMPARE(historySource->item(0), QVariant{QMetaType(QMetaType::Int)});
        QCOMPARE(historySource->itemCount(), 20);
        QCOMPARE(historySource->item(19), QVariant{QMetaType(QMetaType::Int)});

        // Using the FillFromEnd fill mode adds items to the end while itemCount
        // is less than maximumHistory.

        historySource->setFillMode(HistoryProxySource::FillFromEnd);
        QCOMPARE(historySource->itemCount(), 20);

        for (int i = 0; i < 19; ++i) {
            valueSource->setValue(i);
            QCOMPARE(historySource->item(0), QVariant{QMetaType(QMetaType::Int)});
            for (int item = 19; item > 1; --item) {
                if ((item - 19) + i < 0) {
                    QCOMPARE(historySource->item(item), QVariant{QMetaType(QMetaType::Int)});
                } else {
                    QCOMPARE(historySource->item(item), 19 - item);
                }
            }
        }
    }

    void testWithModel()
    {
        auto model = std::make_unique<TestModel>();
        auto modelSource = std::make_unique<ModelSource>();
        modelSource->setModel(model.get());
        modelSource->setRole(Qt::DisplayRole);

        auto historySource = std::make_unique<HistoryProxySource>();
        historySource->setSource(modelSource.get());
        historySource->setMaximumHistory(10);
        historySource->setFillMode(HistoryProxySource::DoNotFill);

        // When there is a source with an itemCount > 1, we can use setItem to
        // indicate which item should be watched.
        historySource->setItem(2);

        // When we then change the model, the history source should record an
        // item.
        for (int i = 0; i < 10; ++i) {
            model->set(2, i);
            QCOMPARE(historySource->itemCount(), i + 1);
            for (int item = 0; item < historySource->itemCount(); ++item) {
                QCOMPARE(historySource->item(item), i - item);
            }
        }

        historySource->clear();

        model->set(2, 5);
        model->set(2, 2);

        // When the model has a minimum and a maximum property, those will be
        // used for the history source's minimum and maximum rather than
        // calculating it based on the items in the history.

        QCOMPARE(historySource->minimum(), 1);
        QCOMPARE(historySource->maximum(), 2);
    }
};

QTEST_GUILESS_MAIN(HistoryProxySourceTest)

#include "HistoryProxySourceTest.moc"
