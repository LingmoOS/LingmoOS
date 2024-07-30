/*
    SPDX-FileCopyrightText: 2016 Sune Vuorela <sune@debian.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kdescendantsproxymodel.h"

#include <QAbstractItemModelTester>
#include <QAbstractListModel>
#include <QIdentityProxyModel>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>

struct Node {
    ~Node()
    {
        qDeleteAll(children);
    }

    QString label;
    Node *parent = nullptr;
    QList<Node *> children;
    int knownChildren = 0;
};

class SimpleObjectModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SimpleObjectModel(QObject *parent = nullptr, bool incremental = false);
    ~SimpleObjectModel() override;

    QModelIndex index(int, int, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool insert(const QModelIndex &parent, int row, const QString &text);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow) override;

    Node *getRootNode() const
    {
        return m_root;
    }

private:
    Node *m_root;
    // simulate a model that loads in new rows with fetchMore()
    bool m_incremental;
};

SimpleObjectModel::SimpleObjectModel(QObject *parent, bool incremental)
    : QAbstractListModel(parent)
    , m_incremental(incremental)
{
    m_root = new Node;
}

SimpleObjectModel::~SimpleObjectModel()
{
    delete m_root;
}

QModelIndex SimpleObjectModel::index(int row, int col, const QModelIndex &parent) const
{
    Node *parentItem;
    if (!parent.isValid()) {
        parentItem = static_cast<Node *>(m_root);
    } else {
        parentItem = static_cast<Node *>(parent.internalPointer());
    }

    if (row < 0 || parentItem->children.size() <= row) {
        return QModelIndex();
    }
    Node *childItem = parentItem->children[row];

    return createIndex(row, col, childItem);
}

QModelIndex SimpleObjectModel::parent(const QModelIndex &index) const
{
    Node *childItem = static_cast<Node *>(index.internalPointer());
    if (!childItem) {
        return QModelIndex();
    }

    Node *parent = childItem->parent;
    Node *grandParent = parent->parent;

    int childRow = 0;
    if (grandParent) {
        childRow = grandParent->children.indexOf(parent);
    }

    if (parent == m_root) {
        return QModelIndex();
    }
    return createIndex(childRow, 0, parent);
}

int SimpleObjectModel::rowCount(const QModelIndex &index) const
{
    Node *item = static_cast<Node *>(index.internalPointer());
    if (!item) {
        item = m_root;
    }

    if (m_incremental) {
        return item->knownChildren;
    }
    return item->children.count();
}

bool SimpleObjectModel::hasChildren(const QModelIndex &index) const
{
    Node *item = static_cast<Node *>(index.internalPointer());
    if (!item) {
        item = m_root;
    }

    return !item->children.isEmpty();
}

QVariant SimpleObjectModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    Node *node = static_cast<Node *>(index.internalPointer());
    if (!node) {
        return QVariant();
    }
    return node->label;
}

bool SimpleObjectModel::insert(const QModelIndex &index, int row, const QString &text)
{
    if (row < 0) {
        return false;
    }

    Node *parent = static_cast<Node *>(index.internalPointer());
    if (!parent) {
        parent = m_root;
    }

    if (row > parent->children.count()) {
        return false;
    }

    beginInsertRows(index, row, row);
    Node *child = new Node;
    child->parent = parent;
    child->label = text;
    parent->children.insert(row, child);
    endInsertRows();

    return true;
}

bool SimpleObjectModel::removeRows(int row, int count, const QModelIndex &index)
{
    if (row < 0) {
        return false;
    }

    Node *parent = static_cast<Node *>(index.internalPointer());
    if (!parent) {
        parent = m_root;
    }

    const int last = row + count - 1;

    if (last >= parent->children.count()) {
        return false;
    }

    beginRemoveRows(index, row, last);
    for (int i = last; i >= row; i--) {
        Node *child = parent->children.takeAt(i);
        delete child;
    }
    endRemoveRows();

    return true;
}

bool SimpleObjectModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow)
{
    Node *sourceNode = static_cast<Node *>(sourceParent.internalPointer());
    if (!sourceNode) {
        sourceNode = m_root;
    }
    Node *destinationNode = static_cast<Node *>(destinationParent.internalPointer());
    if (!destinationNode) {
        destinationNode = m_root;
    }

    const int sourceLast = sourceRow + count - 1;

    if (sourceNode != destinationNode) {
        if (count <= 0 || sourceRow < 0 || sourceRow >= sourceNode->children.count() || destinationRow < 0
            || destinationRow > destinationNode->children.count()) {
            return false;
        }

        if (!beginMoveRows(sourceParent, sourceRow, sourceLast, destinationParent, destinationRow)) {
            return false;
        }

        Node *child = sourceNode->children.takeAt(sourceRow);
        child->parent = destinationNode;
        destinationNode->children.insert(destinationRow, child);

        endMoveRows();
        return true;
    }

    if (count <= 0 || sourceRow == destinationRow || sourceRow < 0 || sourceRow >= destinationNode->children.count() || destinationRow < 0
        || destinationRow > destinationNode->children.count() || count - destinationRow > destinationNode->children.count() - sourceRow) {
        return false;
    }

    // beginMoveRows wants indexes before the source rows are removed from the old order
    if (!beginMoveRows(sourceParent, sourceRow, sourceLast, destinationParent, destinationRow)) {
        return false;
    }

    if (sourceRow < destinationRow) {
        for (int i = count - 1; i >= 0; --i) {
            destinationNode->children.move(sourceRow + i, destinationRow - count + i);
        }
    } else {
        for (int i = 0; i < count; ++i) {
            destinationNode->children.move(sourceRow + i, destinationRow + i);
        }
    }

    endMoveRows();
    return true;
}

class tst_KDescendantProxyModel : public QObject
{
    Q_OBJECT
    std::unique_ptr<QStandardItemModel> createTree(const QString &prefix)
    {
        /*
         * |- parent1
         * |  |- child1
         * |  `- child2
         * `- parent2
         *    |- child1
         *    `- child2
         */
        auto model = std::make_unique<QStandardItemModel>(this);
        for (int i = 0; i < 2; i++) {
            QStandardItem *item = new QStandardItem();
            item->setData(QString(prefix + QString::number(i)), Qt::DisplayRole);
            for (int j = 0; j < 2; j++) {
                QStandardItem *child = new QStandardItem();
                child->setData(QString(prefix + QString::number(i) + "-" + QString::number(j)), Qt::DisplayRole);
                item->appendRow(child);
            }
            model->appendRow(item);
        }
        return model;
    }

private Q_SLOTS:
    void testResetModelContent();
    void testSourceModelReset();
    void testChangeSeparator();
    void testChangeInvisibleSeparator();
    void testRemoveSeparator();

    void testResetCollapsedModelContent();
    void testInsertInCollapsedModel();
    void testRemoveInCollapsedModel();
    void testMoveInsideCollapsed();
    void testExpandInsideCollapsed();
    void testEmptyModel();
    void testEmptyChild();
};

/// Tests that replacing the source model results in data getting changed
void tst_KDescendantProxyModel::testResetModelContent()
{
    auto model1 = createTree("FirstModel");
    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy);
    proxy.setSourceModel(model1.get());
    QCOMPARE(proxy.rowCount(), 6);

    {
        // clang-format off
        const QStringList results {
            "FirstModel0",
            "FirstModel0-0",
            "FirstModel0-1",
            "FirstModel1",
            "FirstModel1-0",
            "FirstModel1-1"
        };
        // clang-format on
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
    auto model2 = createTree("SecondModel");
    {
        proxy.setSourceModel(model2.get());
        // clang-format off
        const QStringList results {
            "SecondModel0",
            "SecondModel0-0",
            "SecondModel0-1",
            "SecondModel1",
            "SecondModel1-0",
            "SecondModel1-1"
        };
        // clang-format on
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
}

/// Tests that a reset in the source model results in data getting changed
void tst_KDescendantProxyModel::testSourceModelReset()
{
    auto model1 = createTree("FirstModel");
    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy); // ensure no nested signals
    QIdentityProxyModel identityProxy;
    identityProxy.setSourceModel(model1.get());
    proxy.setSourceModel(&identityProxy);
    QCOMPARE(proxy.rowCount(), 6);

    {
        // clang-format off
        const QStringList results {
            "FirstModel0",
            "FirstModel0-0",
            "FirstModel0-1",
            "FirstModel1",
            "FirstModel1-0",
            "FirstModel1-1"
        };
        // clang-format on
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }

    auto model2 = createTree("SecondModel");
    {
        identityProxy.setSourceModel(model2.get()); // This makes QIdentityProxyModel emit a reset
        // clang-format off
        const QStringList results {
            "SecondModel0",
            "SecondModel0-0",
            "SecondModel0-1",
            "SecondModel1",
            "SecondModel1-0",
            "SecondModel1-1"
        };
        // clang-format on
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
}

/// tests that change separator works, as well as emits the relevant data changed signals
void tst_KDescendantProxyModel::testChangeSeparator()
{
    auto model1 = createTree("FirstModel");
    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy);
    proxy.setSourceModel(model1.get());
    proxy.setDisplayAncestorData(true);
    QSignalSpy dataChangedSpy(&proxy, &QAbstractItemModel::dataChanged);
    QCOMPARE(proxy.rowCount(), 6);
    {
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0 / FirstModel0-0"
                                            << "FirstModel0 / FirstModel0-1"
                                            << "FirstModel1"
                                            << "FirstModel1 / FirstModel1-0"
                                            << "FirstModel1 / FirstModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
    proxy.setAncestorSeparator("LOL");
    QCOMPARE(dataChangedSpy.count(), 1);
    {
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0LOLFirstModel0-0"
                                            << "FirstModel0LOLFirstModel0-1"
                                            << "FirstModel1"
                                            << "FirstModel1LOLFirstModel1-0"
                                            << "FirstModel1LOLFirstModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
}

/// tests that change separator that is not shown does not change the content and does not
/// emit data changed signals, since the data isn't changed
void tst_KDescendantProxyModel::testChangeInvisibleSeparator()
{
    auto model1 = createTree("FirstModel");
    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy);
    proxy.setSourceModel(model1.get());
    QSignalSpy dataChangedSpy(&proxy, &QAbstractItemModel::dataChanged);
    QCOMPARE(proxy.rowCount(), 6);
    {
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0-0"
                                            << "FirstModel0-1"
                                            << "FirstModel1"
                                            << "FirstModel1-0"
                                            << "FirstModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
    proxy.setAncestorSeparator("LOL");
    QCOMPARE(dataChangedSpy.count(), 0);
    {
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0-0"
                                            << "FirstModel0-1"
                                            << "FirstModel1"
                                            << "FirstModel1-0"
                                            << "FirstModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
}

/// tests that data is properly updated when separator is removed/hidden
/// and data changed signal is emitted
void tst_KDescendantProxyModel::testRemoveSeparator()
{
    auto model1 = createTree("FirstModel");
    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy);
    proxy.setSourceModel(model1.get());
    QSignalSpy dataChangedSpy(&proxy, &QAbstractItemModel::dataChanged);
    proxy.setDisplayAncestorData(true);
    QCOMPARE(dataChangedSpy.count(), 1);
    dataChangedSpy.clear();
    QCOMPARE(proxy.rowCount(), 6);
    {
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0 / FirstModel0-0"
                                            << "FirstModel0 / FirstModel0-1"
                                            << "FirstModel1"
                                            << "FirstModel1 / FirstModel1-0"
                                            << "FirstModel1 / FirstModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
    proxy.setDisplayAncestorData(false);
    QCOMPARE(dataChangedSpy.count(), 1);
    {
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0-0"
                                            << "FirstModel0-1"
                                            << "FirstModel1"
                                            << "FirstModel1-0"
                                            << "FirstModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
}

void tst_KDescendantProxyModel::testResetCollapsedModelContent()
{
    auto model1 = createTree("FirstModel");
    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy);
    proxy.setExpandsByDefault(false);
    proxy.setSourceModel(model1.get());
    QCOMPARE(proxy.rowCount(), 2);

    {
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
    {
        QModelIndex idx = model1->index(0, 0);
        proxy.expandSourceIndex(idx);
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0-0"
                                            << "FirstModel0-1"
                                            << "FirstModel1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
    {
        QModelIndex idx = model1->index(1, 0);
        proxy.expandSourceIndex(idx);
        QStringList results = QStringList() << "FirstModel0"
                                            << "FirstModel0-0"
                                            << "FirstModel0-1"
                                            << "FirstModel1"
                                            << "FirstModel1-0"
                                            << "FirstModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }

    auto model2 = createTree("SecondModel");
    {
        proxy.setSourceModel(model2.get());
        QModelIndex idx = model2->index(0, 0);
        proxy.expandSourceIndex(idx);
        idx = model2->index(1, 0);
        proxy.expandSourceIndex(idx);
        QStringList results = QStringList() << "SecondModel0"
                                            << "SecondModel0-0"
                                            << "SecondModel0-1"
                                            << "SecondModel1"
                                            << "SecondModel1-0"
                                            << "SecondModel1-1";
        QCOMPARE(proxy.rowCount(), results.count());
        for (int i = 0; i < proxy.rowCount(); i++) {
            QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
        }
    }
}

void tst_KDescendantProxyModel::testInsertInCollapsedModel()
{
    auto model1 = createTree("Model");
    KDescendantsProxyModel proxy;
    proxy.setExpandsByDefault(false);
    proxy.setSourceModel(model1.get());
    QCOMPARE(proxy.rowCount(), 2);

    QSignalSpy insertSpy(&proxy, &QAbstractItemModel::rowsInserted);
    QCOMPARE(insertSpy.count(), 0);

    QStandardItem *parent = model1->item(0, 0);
    QVERIFY(parent);

    QStandardItem *child = new QStandardItem();
    child->setData(QString(QStringLiteral("Model") + QString::number(0) + "-" + QString::number(2)), Qt::DisplayRole);
    parent->appendRow(child);

    // Adding a child to the collapsed parent doesn't have an effect to the proxy
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(insertSpy.count(), 0);

    // If we expand everything inserted should be here
    QModelIndex idx = model1->index(0, 0);
    proxy.expandSourceIndex(idx);

    QCOMPARE(proxy.rowCount(), 5);
    QCOMPARE(insertSpy.count(), 1);

    QCOMPARE(proxy.index(3, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Model0-2"));

    // Add another child to the expanded node, now the proxy is affected immediately
    child = new QStandardItem();
    child->setData(QString(QStringLiteral("Model") + QString::number(0) + "-" + QString::number(3)), Qt::DisplayRole);
    parent->appendRow(child);

    QCOMPARE(proxy.rowCount(), 6);
    QCOMPARE(insertSpy.count(), 2);
}

void tst_KDescendantProxyModel::testRemoveInCollapsedModel()
{
    auto model1 = createTree("Model");
    KDescendantsProxyModel proxy;
    proxy.setExpandsByDefault(false);
    proxy.setSourceModel(model1.get());
    QCOMPARE(proxy.rowCount(), 2);

    QSignalSpy removeSpy(&proxy, &QAbstractItemModel::rowsRemoved);
    QCOMPARE(removeSpy.count(), 0);

    QStandardItem *parent = model1->item(0, 0);
    QVERIFY(parent);

    parent->removeRow(0);

    // Adding a child to the collapsed parent doesn't have an effect to the proxy
    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(removeSpy.count(), 0);

    // If we expand everything inserted should be here
    QModelIndex idx = model1->index(0, 0);
    proxy.expandSourceIndex(idx);

    QCOMPARE(proxy.rowCount(), 3);

    QCOMPARE(proxy.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("Model0-1"));
    parent->removeRow(0);

    QCOMPARE(proxy.rowCount(), 2);
    QCOMPARE(removeSpy.count(), 1);

    idx = model1->index(1, 0);
    proxy.expandSourceIndex(idx);
    QCOMPARE(proxy.rowCount(), 4);
}

void tst_KDescendantProxyModel::testMoveInsideCollapsed()
{
    SimpleObjectModel *model = new SimpleObjectModel(this);
    model->insert(QModelIndex(), 0, QStringLiteral("Model0"));
    model->insert(QModelIndex(), 1, QStringLiteral("Model1"));
    model->insert(QModelIndex(), 2, QStringLiteral("Model2"));

    model->insert(model->index(0, 0, QModelIndex()), 0, QStringLiteral("Model0-0"));
    model->insert(model->index(1, 0, QModelIndex()), 0, QStringLiteral("Model1-0"));

    QCOMPARE(model->rowCount(), 3);

    KDescendantsProxyModel proxy;
    proxy.setExpandsByDefault(false);
    proxy.setSourceModel(model);
    QCOMPARE(proxy.rowCount(), 3);

    QSignalSpy removeSpy(&proxy, &QAbstractItemModel::rowsRemoved);
    QCOMPARE(removeSpy.count(), 0);
    QSignalSpy insertSpy(&proxy, &QAbstractItemModel::rowsInserted);
    QCOMPARE(insertSpy.count(), 0);

    model->moveRows(QModelIndex(), 2, 1, model->index(0, 0, QModelIndex()), 1);

    QCOMPARE(removeSpy.count(), 1);

    QVERIFY(!removeSpy.first()[0].value<QModelIndex>().isValid());
    QCOMPARE(removeSpy.first()[1].toInt(), 2);
    QCOMPARE(removeSpy.first()[2].toInt(), 2);

    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(proxy.rowCount(), 2);

    model->moveRows(model->index(0, 0, QModelIndex()), 0, 1, QModelIndex(), 1);
    QCOMPARE(insertSpy.count(), 1);
    QCOMPARE(model->rowCount(), 3);
    QCOMPARE(proxy.rowCount(), 3);

    QVERIFY(!insertSpy.first()[0].value<QModelIndex>().isValid());
    QCOMPARE(insertSpy.first()[1].toInt(), 1);
    QCOMPARE(insertSpy.first()[2].toInt(), 1);

    QModelIndex idx = model->index(0, 0);
    proxy.expandSourceIndex(idx);
    idx = model->index(1, 0);
    proxy.expandSourceIndex(idx);
    idx = model->index(2, 0);
    proxy.expandSourceIndex(idx);
    QStringList results = QStringList() << "Model0"
                                        << "Model2"
                                        << "Model0-0"
                                        << "Model1"
                                        << "Model1-0";
    QCOMPARE(proxy.rowCount(), results.count());
    for (int i = 0; i < proxy.rowCount(); i++) {
        QCOMPARE(proxy.index(i, 0).data(Qt::DisplayRole).toString(), results[i]);
    }
}

void tst_KDescendantProxyModel::testExpandInsideCollapsed()
{
    SimpleObjectModel *model = new SimpleObjectModel(this);
    model->insert(QModelIndex(), 0, QStringLiteral("Model0"));
    model->insert(QModelIndex(), 1, QStringLiteral("Model1"));
    model->insert(QModelIndex(), 2, QStringLiteral("Model2"));

    auto parentIndex = model->index(0, 0, QModelIndex());
    model->insert(parentIndex, 0, QStringLiteral("Model0-0"));
    auto childIndex = model->index(0, 0, parentIndex);
    model->insert(childIndex, 0, QStringLiteral("Model0-0-0"));

    QCOMPARE(model->rowCount(), 3);

    KDescendantsProxyModel proxy;
    proxy.setExpandsByDefault(false);
    proxy.setSourceModel(model);
    QCOMPARE(proxy.rowCount(), 3);

    proxy.expandSourceIndex(childIndex);
    QVERIFY(proxy.isSourceIndexExpanded(childIndex));
    QVERIFY(!proxy.isSourceIndexExpanded(parentIndex));
    QCOMPARE(proxy.rowCount(), 3);
    proxy.expandSourceIndex(parentIndex);
    QCOMPARE(proxy.rowCount(), 5);
}

void tst_KDescendantProxyModel::testEmptyModel()
{
    SimpleObjectModel *model = new SimpleObjectModel(this, true);
    model->insert(QModelIndex(), 0, QStringLiteral("Row0"));
    model->insert(QModelIndex(), 0, QStringLiteral("Row1"));
    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy);
    proxy.setSourceModel(model);
    QCOMPARE(proxy.rowCount(), 0);
}

void tst_KDescendantProxyModel::testEmptyChild()
{
    SimpleObjectModel *model = new SimpleObjectModel(this, true);
    model->insert(QModelIndex(), 0, QStringLiteral("Row0"));
    auto parentIndex = model->index(0, 0, QModelIndex());
    model->insert(parentIndex, 0, QStringLiteral("Row0-0"));
    model->insert(parentIndex, 0, QStringLiteral("Row0-1"));
    model->insert(QModelIndex(), 0, QStringLiteral("Row1"));

    // simulate that the row count for the root node is known because it was listed
    model->getRootNode()->knownChildren = 2;

    KDescendantsProxyModel proxy;
    new QAbstractItemModelTester(&proxy);
    proxy.setSourceModel(model);
    proxy.setExpandsByDefault(false);
    QCOMPARE(proxy.rowCount(), 2);

    // remove the row that has children but a rowCount of 0
    model->removeRows(0, 1, QModelIndex());
    QCOMPARE(proxy.rowCount(), 1);
}

QTEST_MAIN(tst_KDescendantProxyModel)

#include "kdescendantsproxymodeltest.moc"
