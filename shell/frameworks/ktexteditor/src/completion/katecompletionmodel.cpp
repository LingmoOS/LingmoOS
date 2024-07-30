/*
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecompletionmodel.h"

#include "kateargumenthintmodel.h"
#include "katecompletiontree.h"
#include "katecompletionwidget.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "kateview.h"
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

#include <KFuzzyMatcher>
#include <KLocalizedString>

#include <QApplication>
#include <QMultiMap>
#include <QTimer>
#include <QVarLengthArray>

using namespace KTextEditor;

/// A helper-class for handling completion-models with hierarchical grouping/optimization
class HierarchicalModelHandler
{
public:
    explicit HierarchicalModelHandler(CodeCompletionModel *model);
    void addValue(CodeCompletionModel::ExtraItemDataRoles role, const QVariant &value);
    // Walks the index upwards and collects all defined completion-roles on the way
    void collectRoles(const QModelIndex &index);
    void takeRole(const QModelIndex &index);

    CodeCompletionModel *model() const;

    // Assumes that index is a sub-index of the indices where role-values were taken
    QVariant getData(CodeCompletionModel::ExtraItemDataRoles role, const QModelIndex &index) const;

    bool hasHierarchicalRoles() const;

    int inheritanceDepth(const QModelIndex &i) const;

    QString customGroup() const
    {
        return m_customGroup;
    }

    int customGroupingKey() const
    {
        return m_groupSortingKey;
    }

private:
    typedef std::pair<CodeCompletionModel::ExtraItemDataRoles, QVariant> RoleAndValue;
    typedef std::vector<std::pair<CodeCompletionModel::ExtraItemDataRoles, QVariant>> RoleMap;
    RoleMap m_roleValues;
    QString m_customGroup;
    int m_groupSortingKey;
    CodeCompletionModel *m_model;
};

CodeCompletionModel *HierarchicalModelHandler::model() const
{
    return m_model;
}

bool HierarchicalModelHandler::hasHierarchicalRoles() const
{
    return !m_roleValues.empty();
}

void HierarchicalModelHandler::collectRoles(const QModelIndex &index)
{
    if (index.parent().isValid()) {
        collectRoles(index.parent());
    }
    if (m_model->rowCount(index) != 0) {
        takeRole(index);
    }
}

int HierarchicalModelHandler::inheritanceDepth(const QModelIndex &i) const
{
    return getData(CodeCompletionModel::InheritanceDepth, i).toInt();
}

void HierarchicalModelHandler::takeRole(const QModelIndex &index)
{
    QVariant v = index.data(CodeCompletionModel::GroupRole);
    if (v.isValid() && v.canConvert<int>()) {
        QVariant value = index.data(v.toInt());
        if (v.toInt() == Qt::DisplayRole) {
            m_customGroup = index.data(Qt::DisplayRole).toString();
            QVariant sortingKey = index.data(CodeCompletionModel::InheritanceDepth);
            if (sortingKey.canConvert<int>()) {
                m_groupSortingKey = sortingKey.toInt();
            }
        } else {
            auto role = (CodeCompletionModel::ExtraItemDataRoles)v.toInt();
            addValue(role, value);
        }
    } else {
        qCDebug(LOG_KTE) << "Did not return valid GroupRole in hierarchical completion-model";
    }
}

QVariant HierarchicalModelHandler::getData(CodeCompletionModel::ExtraItemDataRoles role, const QModelIndex &index) const
{
    auto it = std::find_if(m_roleValues.begin(), m_roleValues.end(), [role](const RoleAndValue &v) {
        return v.first == role;
    });
    if (it != m_roleValues.end()) {
        return it->second;
    } else {
        return index.data(role);
    }
}

HierarchicalModelHandler::HierarchicalModelHandler(CodeCompletionModel *model)
    : m_groupSortingKey(-1)
    , m_model(model)
{
}

void HierarchicalModelHandler::addValue(CodeCompletionModel::ExtraItemDataRoles role, const QVariant &value)
{
    auto it = std::find_if(m_roleValues.begin(), m_roleValues.end(), [role](const RoleAndValue &v) {
        return v.first == role;
    });
    if (it != m_roleValues.end()) {
        it->second = value;
    } else {
        m_roleValues.push_back({role, value});
    }
}

KateCompletionModel::KateCompletionModel(KateCompletionWidget *parent)
    : ExpandingWidgetModel(parent)
    , m_ungrouped(new Group({}, 0, this))
    , m_argumentHints(new Group(i18n("Argument-hints"), -1, this))
    , m_bestMatches(new Group(i18n("Best matches"), BestMatchesProperty, this))
    , m_emptyGroups({m_ungrouped, m_argumentHints, m_bestMatches})
{
    m_updateBestMatchesTimer = new QTimer(this);
    m_updateBestMatchesTimer->setSingleShot(true);
    connect(m_updateBestMatchesTimer, &QTimer::timeout, this, &KateCompletionModel::updateBestMatches);

    m_groupHash.insert(0, m_ungrouped);
    m_groupHash.insert(-1, m_argumentHints);
    m_groupHash.insert(BestMatchesProperty, m_argumentHints);

    createGroups();
}

KateCompletionModel::~KateCompletionModel()
{
    clearCompletionModels();
    delete m_argumentHints;
    delete m_ungrouped;
    delete m_bestMatches;
}

QTreeView *KateCompletionModel::treeView() const
{
    return view()->completionWidget()->treeView();
}

QVariant KateCompletionModel::data(const QModelIndex &index, int role) const
{
    if (!hasCompletionModel() || !index.isValid()) {
        return QVariant();
    }

    if (role == InternalRole::IsNonEmptyGroup) {
        auto group = groupForIndex(index);
        return group && !group->isEmpty;
    }

    // groupOfParent returns a group when the index is a member of that group, but not the group head/label.
    if (!hasGroups() || groupOfParent(index)) {
        if (role == Qt::TextAlignmentRole) {
            int c = 0;
            for (const auto &list : m_columnMerges) {
                if (size_t(index.column()) < c + list.size()) {
                    c += list.size();
                    continue;
                } else if (list.size() == 1 && list.front() == CodeCompletionModel::Scope) {
                    return Qt::AlignRight;
                } else {
                    return QVariant();
                }
            }
        }

        // Merge text for column merging
        if (role == Qt::DisplayRole) {
            QString text;
            for (int column : m_columnMerges[index.column()]) {
                QModelIndex sourceIndex = mapToSource(createIndex(index.row(), column, index.internalPointer()));
                text.append(sourceIndex.data(role).toString());
            }

            return text;
        }

        if (role == CodeCompletionModel::HighlightingMethod) {
            // Return that we are doing custom-highlighting of one of the sub-strings does it. Unfortunately internal highlighting does not work for the other
            // substrings.
            for (int column : m_columnMerges[index.column()]) {
                QModelIndex sourceIndex = mapToSource(createIndex(index.row(), column, index.internalPointer()));
                QVariant method = sourceIndex.data(CodeCompletionModel::HighlightingMethod);
                if (method.userType() == QMetaType::Int && method.toInt() == CodeCompletionModel::CustomHighlighting) {
                    return QVariant(CodeCompletionModel::CustomHighlighting);
                }
            }
            return QVariant();
        }
        if (role == CodeCompletionModel::CustomHighlight) {
            // Merge custom highlighting if multiple columns were merged
            QStringList strings;

            // Collect strings
            const auto &columns = m_columnMerges[index.column()];
            strings.reserve(columns.size());
            for (int column : columns) {
                strings << mapToSource(createIndex(index.row(), column, index.internalPointer())).data(Qt::DisplayRole).toString();
            }

            QList<QVariantList> highlights;

            // Collect custom-highlightings
            highlights.reserve(columns.size());
            for (int column : columns) {
                highlights << mapToSource(createIndex(index.row(), column, index.internalPointer())).data(CodeCompletionModel::CustomHighlight).toList();
            }

            return mergeCustomHighlighting(strings, highlights, 0);
        }

        QVariant v = mapToSource(index).data(role);
        if (v.isValid()) {
            return v;
        } else {
            return ExpandingWidgetModel::data(index, role);
        }
    }

    // Returns a nonzero group if this index is the head of a group(A Label in the list)
    Group *g = groupForIndex(index);

    if (g && (!g->isEmpty)) {
        switch (role) {
        case Qt::DisplayRole:
            if (!index.column()) {
                return g->title;
            }
            break;

        case Qt::FontRole:
            if (!index.column()) {
                QFont f = view()->renderer()->currentFont();
                f.setBold(true);
                return f;
            }
            break;

        case Qt::ForegroundRole:
            return QApplication::palette().toolTipText().color();
        case Qt::BackgroundRole:
            return QApplication::palette().toolTipBase().color();
        }
    }

    return QVariant();
}

int KateCompletionModel::contextMatchQuality(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }
    Group *g = groupOfParent(index);
    if (!g || g->filtered.size() < (size_t)index.row()) {
        return 0;
    }

    return contextMatchQuality(g->filtered[index.row()].sourceRow());
}

int KateCompletionModel::contextMatchQuality(const ModelRow &source) const
{
    QModelIndex realIndex = source.second;

    int bestMatch = -1;
    // Iterate through all argument-hints and find the best match-quality
    for (const Item &item : m_argumentHints->filtered) {
        const ModelRow &row(item.sourceRow());
        if (realIndex.model() != row.first) {
            continue; // We can only match within the same source-model
        }

        QModelIndex hintIndex = row.second;

        QVariant depth = hintIndex.data(CodeCompletionModel::ArgumentHintDepth);
        if (!depth.isValid() || depth.userType() != QMetaType::Int || depth.toInt() != 1) {
            continue; // Only match completion-items to argument-hints of depth 1(the ones the item will be given to as argument)
        }

        hintIndex.data(CodeCompletionModel::SetMatchContext);

        QVariant matchQuality = realIndex.data(CodeCompletionModel::MatchQuality);
        if (matchQuality.isValid() && matchQuality.userType() == QMetaType::Int) {
            int m = matchQuality.toInt();
            if (m > bestMatch) {
                bestMatch = m;
            }
        }
    }

    if (m_argumentHints->filtered.empty()) {
        QVariant matchQuality = realIndex.data(CodeCompletionModel::MatchQuality);
        if (matchQuality.isValid() && matchQuality.userType() == QMetaType::Int) {
            int m = matchQuality.toInt();
            if (m > bestMatch) {
                bestMatch = m;
            }
        }
    }

    return bestMatch;
}

Qt::ItemFlags KateCompletionModel::flags(const QModelIndex &index) const
{
    if (!hasCompletionModel() || !index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (!hasGroups() || groupOfParent(index)) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsEnabled;
}

KateCompletionWidget *KateCompletionModel::widget() const
{
    return static_cast<KateCompletionWidget *>(QObject::parent());
}

KTextEditor::ViewPrivate *KateCompletionModel::view() const
{
    return widget()->view();
}

int KateCompletionModel::columnCount(const QModelIndex &) const
{
    return 3;
}

KateCompletionModel::ModelRow KateCompletionModel::modelRowPair(const QModelIndex &index)
{
    return qMakePair(static_cast<CodeCompletionModel *>(const_cast<QAbstractItemModel *>(index.model())), index);
}

bool KateCompletionModel::hasChildren(const QModelIndex &parent) const
{
    if (!hasCompletionModel()) {
        return false;
    }

    if (!parent.isValid()) {
        if (hasGroups()) {
            return true;
        }

        return !m_ungrouped->filtered.empty();
    }

    if (parent.column() != 0) {
        return false;
    }

    if (!hasGroups()) {
        return false;
    }

    if (Group *g = groupForIndex(parent)) {
        return !g->filtered.empty();
    }

    return false;
}

QModelIndex KateCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column >= columnCount(QModelIndex())) {
        return QModelIndex();
    }

    if (parent.isValid() || !hasGroups()) {
        if (parent.isValid() && parent.column() != 0) {
            return QModelIndex();
        }

        Group *g = groupForIndex(parent);

        if (!g) {
            return QModelIndex();
        }

        if (row >= (int)g->filtered.size()) {
            // qCWarning(LOG_KTE) << "Invalid index requested: row " << row << " beyond individual range in group " << g;
            return QModelIndex();
        }

        // qCDebug(LOG_KTE) << "Returning index for child " << row << " of group " << g;
        return createIndex(row, column, g);
    }

    if (size_t(row) >= m_rowTable.size()) {
        // qCWarning(LOG_KTE) << "Invalid index requested: row " << row << " beyond group range.";
        return QModelIndex();
    }

    // qCDebug(LOG_KTE) << "Returning index for group " << m_rowTable[row];
    return createIndex(row, column, quintptr(0));
}

bool KateCompletionModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column >= columnCount(QModelIndex())) {
        return false;
    }

    if (parent.isValid() || !hasGroups()) {
        if (parent.isValid() && parent.column() != 0) {
            return false;
        }

        Group *g = groupForIndex(parent);

        if (row >= (int)g->filtered.size()) {
            return false;
        }

        return true;
    }

    if (size_t(row) >= m_rowTable.size()) {
        return false;
    }

    return true;
}

QModelIndex KateCompletionModel::indexForRow(Group *g, int row) const
{
    if (row < 0 || row >= (int)g->filtered.size()) {
        return QModelIndex();
    }

    return createIndex(row, 0, g);
}

QModelIndex KateCompletionModel::indexForGroup(Group *g) const
{
    if (!hasGroups()) {
        return QModelIndex();
    }

    auto it = std::find(m_rowTable.begin(), m_rowTable.end(), g);
    if (it == m_rowTable.end()) {
        return QModelIndex();
    }
    int row = std::distance(m_rowTable.begin(), it);
    return createIndex(row, 0, quintptr(0));
}

void KateCompletionModel::clearGroups()
{
    m_ungrouped->clear();
    m_argumentHints->clear();
    m_bestMatches->clear();

    // Don't bother trying to work out where it is
    m_rowTable.erase(std::remove_if(m_rowTable.begin(),
                                    m_rowTable.end(),
                                    [this](Group *g) {
                                        return (g == m_ungrouped) || (g == m_argumentHints) || (g == m_bestMatches);
                                    }),
                     m_rowTable.end());

    m_emptyGroups.erase(std::remove_if(m_emptyGroups.begin(),
                                       m_emptyGroups.end(),
                                       [this](Group *g) {
                                           return (g == m_ungrouped) || (g == m_argumentHints) || (g == m_bestMatches);
                                       }),
                        m_emptyGroups.end());

    qDeleteAll(m_rowTable);
    qDeleteAll(m_emptyGroups);
    m_rowTable.clear();
    m_emptyGroups.clear();
    m_groupHash.clear();
    m_customGroupHash.clear();

    m_emptyGroups.insert(m_emptyGroups.end(), {m_ungrouped, m_argumentHints, m_bestMatches});

    m_groupHash.insert(0, m_ungrouped);
    m_groupHash.insert(-1, m_argumentHints);
    m_groupHash.insert(BestMatchesProperty, m_bestMatches);
}

KateCompletionModel::GroupSet KateCompletionModel::createItems(const HierarchicalModelHandler &_handler, const QModelIndex &i, bool notifyModel)
{
    HierarchicalModelHandler handler(_handler);
    GroupSet ret;
    QAbstractItemModel *model = handler.model();

    if (model->rowCount(i) == 0) {
        // Leaf node, create an item
        ret.insert(createItem(handler, i, notifyModel));
    } else {
        // Non-leaf node, take the role from the node, and recurse to the sub-nodes
        handler.takeRole(i);
        for (int a = 0; a < model->rowCount(i); a++) {
            ret.merge(createItems(handler, model->index(a, 0, i), notifyModel));
        }
    }

    return ret;
}

KateCompletionModel::GroupSet KateCompletionModel::deleteItems(const QModelIndex &i)
{
    GroupSet ret;

    if (i.model()->rowCount(i) == 0) {
        // Leaf node, delete the item
        Group *g = groupForIndex(mapFromSource(i));
        ret.insert(g);
        g->removeItem(ModelRow(const_cast<CodeCompletionModel *>(static_cast<const CodeCompletionModel *>(i.model())), i));
    } else {
        // Non-leaf node
        for (int a = 0; a < i.model()->rowCount(i); a++) {
            ret.merge(deleteItems(i.model()->index(a, 0, i)));
        }
    }

    return ret;
}

void KateCompletionModel::createGroups()
{
    beginResetModel();
    // After clearing the model, it has to be reset, else we will be in an invalid state while inserting
    // new groups.
    clearGroups();

    bool has_groups = false;
    GroupSet groups;
    for (CodeCompletionModel *sourceModel : std::as_const(m_completionModels)) {
        has_groups |= sourceModel->hasGroups();
        for (int i = 0; i < sourceModel->rowCount(); ++i) {
            groups.merge(createItems(HierarchicalModelHandler(sourceModel), sourceModel->index(i, 0)));
        }
    }

    // since notifyModel = false above, we just appended the data as is,
    // we sort it now
    for (auto g : groups) {
        // no need to sort prefiltered, it is just the raw dump of everything
        // filtered is what gets displayed
        //         std::sort(g->prefilter.begin(), g->prefilter.end());
        std::sort(g->filtered.begin(), g->filtered.end(), [this](const Item &l, const Item &r) {
            return l.lessThan(this, r);
        });
    }

    m_hasGroups = has_groups;

    // debugStats();

    for (Group *g : m_rowTable) {
        hideOrShowGroup(g);
    }

    for (Group *g : m_emptyGroups) {
        hideOrShowGroup(g);
    }

    makeGroupItemsUnique();

    updateBestMatches();
    endResetModel();
}

KateCompletionModel::Group *KateCompletionModel::createItem(const HierarchicalModelHandler &handler, const QModelIndex &sourceIndex, bool notifyModel)
{
    // QModelIndex sourceIndex = sourceModel->index(row, CodeCompletionModel::Name, QModelIndex());

    int completionFlags = handler.getData(CodeCompletionModel::CompletionRole, sourceIndex).toInt();

    int argumentHintDepth = handler.getData(CodeCompletionModel::ArgumentHintDepth, sourceIndex).toInt();

    Group *g;
    if (argumentHintDepth) {
        g = m_argumentHints;
    } else {
        QString customGroup = handler.customGroup();
        if (!customGroup.isNull() && m_hasGroups) {
            if (m_customGroupHash.contains(customGroup)) {
                g = m_customGroupHash[customGroup];
            } else {
                g = new Group(customGroup, 0, this);
                g->customSortingKey = handler.customGroupingKey();
                m_emptyGroups.push_back(g);
                m_customGroupHash.insert(customGroup, g);
            }
        } else {
            g = fetchGroup(completionFlags, handler.hasHierarchicalRoles());
        }
    }

    Item item = Item(g != m_argumentHints, this, handler, ModelRow(handler.model(), sourceIndex));

    if (g != m_argumentHints) {
        item.match(this);
    }

    g->addItem(item, notifyModel);

    return g;
}

void KateCompletionModel::slotRowsInserted(const QModelIndex &parent, int start, int end)
{
    HierarchicalModelHandler handler(static_cast<CodeCompletionModel *>(sender()));
    if (parent.isValid()) {
        handler.collectRoles(parent);
    }

    GroupSet affectedGroups;
    for (int i = start; i <= end; ++i) {
        affectedGroups.merge(createItems(handler, handler.model()->index(i, 0, parent), /* notifyModel= */ true));
    }

    for (auto g : affectedGroups) {
        hideOrShowGroup(g, true);
    }
}

void KateCompletionModel::slotRowsRemoved(const QModelIndex &parent, int start, int end)
{
    CodeCompletionModel *source = static_cast<CodeCompletionModel *>(sender());

    GroupSet affectedGroups;
    for (int i = start; i <= end; ++i) {
        QModelIndex index = source->index(i, 0, parent);
        affectedGroups.merge(deleteItems(index));
    }

    for (auto g : affectedGroups) {
        hideOrShowGroup(g, true);
    }
}

KateCompletionModel::Group *KateCompletionModel::fetchGroup(int attribute, bool forceGrouping)
{
    Q_UNUSED(forceGrouping);

    ///@todo use forceGrouping
    if (!hasGroups()) {
        return m_ungrouped;
    }

    int groupingAttribute = groupingAttributes(attribute);
    // qCDebug(LOG_KTE) << attribute << " " << groupingAttribute;

    if (m_groupHash.contains(groupingAttribute)) {
        return m_groupHash.value(groupingAttribute);
    }

    QString st;
    QString at;
    QString it;
    QString title;

    if (attribute & KTextEditor::CodeCompletionModel::GlobalScope) {
        st = QStringLiteral("Global");
    } else if (attribute & KTextEditor::CodeCompletionModel::NamespaceScope) {
        st = QStringLiteral("Namespace");
    } else if (attribute & KTextEditor::CodeCompletionModel::LocalScope) {
        st = QStringLiteral("Local");
    }

    title = st;

    if (attribute & KTextEditor::CodeCompletionModel::Public) {
        at = QStringLiteral("Public");
    } else if (attribute & KTextEditor::CodeCompletionModel::Protected) {
        at = QStringLiteral("Protected");
    } else if (attribute & KTextEditor::CodeCompletionModel::Private) {
        at = QStringLiteral("Private");
    }

    if (!at.isEmpty()) {
        if (!title.isEmpty()) {
            title.append(QLatin1String(", "));
        }

        title.append(at);
    }

    Group *ret = new Group(title, attribute, this);
    ret->scope = QString();

    m_emptyGroups.push_back(ret);
    m_groupHash.insert(groupingAttribute, ret);

    return ret;
}

KateCompletionModel::Group *KateCompletionModel::groupForIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        if (!hasGroups()) {
            return m_ungrouped;
        } else {
            return nullptr;
        }
    }

    if (groupOfParent(index)) {
        return nullptr;
    }

    if (size_t(index.row()) >= m_rowTable.size()) {
        return m_ungrouped;
    }

    return m_rowTable[index.row()];
}

QModelIndex KateCompletionModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    if (Group *g = groupOfParent(index)) {
        if (!hasGroups()) {
            Q_ASSERT(g == m_ungrouped);
            return QModelIndex();
        }

        auto it = std::find(m_rowTable.begin(), m_rowTable.end(), g);
        if (it == m_rowTable.end()) {
            qCWarning(LOG_KTE) << "Couldn't find parent for index" << index;
            return QModelIndex();
        }
        int row = std::distance(m_rowTable.begin(), it);
        return createIndex(row, 0, quintptr(0));
    }

    return QModelIndex();
}

int KateCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (hasGroups()) {
            // qCDebug(LOG_KTE) << "Returning row count for toplevel " << m_rowTable.count();
            return m_rowTable.size();
        } else {
            // qCDebug(LOG_KTE) << "Returning ungrouped row count for toplevel " << m_ungrouped->filtered.count();
            return m_ungrouped->filtered.size();
        }
    }

    if (parent.column() > 0) {
        // only the first column has children
        return 0;
    }

    Group *g = groupForIndex(parent);

    // This is not an error, seems you don't have to check hasChildren()
    if (!g) {
        return 0;
    }

    // qCDebug(LOG_KTE) << "Returning row count for group " << g << " as " << g->filtered.count();
    return g->filtered.size();
}

QModelIndex KateCompletionModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid()) {
        return QModelIndex();
    }

    if (Group *g = groupOfParent(proxyIndex)) {
        if (std::find(m_rowTable.begin(), m_rowTable.end(), g) == m_rowTable.end()) {
            qWarning() << Q_FUNC_INFO << "Stale proxy index for which there is no group";
            return {};
        }

        if (proxyIndex.row() >= 0 && proxyIndex.row() < (int)g->filtered.size()) {
            ModelRow source = g->filtered[proxyIndex.row()].sourceRow();
            return source.second.sibling(source.second.row(), proxyIndex.column());
        } else {
            qCDebug(LOG_KTE) << "Invalid proxy-index";
        }
    }

    return QModelIndex();
}

QModelIndex KateCompletionModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) {
        return QModelIndex();
    }

    if (!hasGroups()) {
        return index(m_ungrouped->rowOf(modelRowPair(sourceIndex)), sourceIndex.column(), QModelIndex());
    }

    for (Group *g : m_rowTable) {
        int row = g->rowOf(modelRowPair(sourceIndex));
        if (row != -1) {
            return index(row, sourceIndex.column(), indexForGroup(g));
        }
    }

    // Copied from above
    for (Group *g : m_emptyGroups) {
        int row = g->rowOf(modelRowPair(sourceIndex));
        if (row != -1) {
            return index(row, sourceIndex.column(), indexForGroup(g));
        }
    }

    return QModelIndex();
}

void KateCompletionModel::setCurrentCompletion(QMap<KTextEditor::CodeCompletionModel *, QString> currentMatch)
{
    beginResetModel();

    m_currentMatch = currentMatch;

    if (!hasGroups()) {
        changeCompletions(m_ungrouped);
    } else {
        for (Group *g : m_rowTable) {
            if (g != m_argumentHints) {
                changeCompletions(g);
            }
        }
        for (Group *g : m_emptyGroups) {
            if (g != m_argumentHints) {
                changeCompletions(g);
            }
        }
    }

    // NOTE: best matches are also updated in resort
    resort();

    endResetModel();
}

QString KateCompletionModel::commonPrefixInternal(const QString &forcePrefix) const
{
    QString commonPrefix; // isNull() = true

    std::vector<Group *> groups = m_rowTable;
    groups.push_back(m_ungrouped);

    for (Group *g : groups) {
        for (const Item &item : g->filtered) {
            uint startPos = currentCompletion(item.sourceRow().first).length();
            const QString candidate = item.name().mid(startPos);

            if (!candidate.startsWith(forcePrefix)) {
                continue;
            }

            if (commonPrefix.isNull()) {
                commonPrefix = candidate;

                // Replace QString() prefix with QString(), so we won't initialize it again
                if (commonPrefix.isNull()) {
                    commonPrefix = QString(); // isEmpty() = true, isNull() = false
                }
            } else {
                commonPrefix.truncate(candidate.length());

                for (int a = 0; a < commonPrefix.length(); ++a) {
                    if (commonPrefix[a] != candidate[a]) {
                        commonPrefix.truncate(a);
                        break;
                    }
                }
            }
        }
    }

    return commonPrefix;
}

QString KateCompletionModel::commonPrefix(QModelIndex selectedIndex) const
{
    QString commonPrefix = commonPrefixInternal(QString());

    if (commonPrefix.isEmpty() && selectedIndex.isValid()) {
        Group *g = m_ungrouped;
        if (hasGroups()) {
            g = groupOfParent(selectedIndex);
        }

        if (g && selectedIndex.row() < (int)g->filtered.size()) {
            // Follow the path of the selected item, finding the next non-empty common prefix
            Item item = g->filtered[selectedIndex.row()];
            int matchLength = currentCompletion(item.sourceRow().first).length();
            commonPrefix = commonPrefixInternal(item.name().mid(matchLength).left(1));
        }
    }

    return commonPrefix;
}

void KateCompletionModel::changeCompletions(Group *g)
{
    // This code determines what of the filtered items still fit
    // don't notify the model. The model is notified afterwards through a reset().
    g->filtered.clear();
    std::remove_copy_if(g->prefilter.begin(), g->prefilter.end(), std::back_inserter(g->filtered), [this](Item &item) {
        return !item.match(this);
    });

    hideOrShowGroup(g, /*notifyModel=*/false);
}

int KateCompletionModel::Group::orderNumber() const
{
    if (this == model->m_ungrouped) {
        return 700;
    }

    if (customSortingKey != -1) {
        return customSortingKey;
    }

    if (attribute & BestMatchesProperty) {
        return 1;
    }

    if (attribute & KTextEditor::CodeCompletionModel::LocalScope) {
        return 100;
    } else if (attribute & KTextEditor::CodeCompletionModel::Public) {
        return 200;
    } else if (attribute & KTextEditor::CodeCompletionModel::Protected) {
        return 300;
    } else if (attribute & KTextEditor::CodeCompletionModel::Private) {
        return 400;
    } else if (attribute & KTextEditor::CodeCompletionModel::NamespaceScope) {
        return 500;
    } else if (attribute & KTextEditor::CodeCompletionModel::GlobalScope) {
        return 600;
    }

    return 700;
}

bool KateCompletionModel::Group::orderBefore(Group *other) const
{
    return orderNumber() < other->orderNumber();
}

void KateCompletionModel::hideOrShowGroup(Group *g, bool notifyModel)
{
    if (g == m_argumentHints) {
        Q_EMIT argumentHintsChanged();
        m_updateBestMatchesTimer->start(200); // We have new argument-hints, so we have new best matches
        return; // Never show argument-hints in the normal completion-list
    }

    if (!g->isEmpty) {
        if (g->filtered.empty()) {
            // Move to empty group list
            g->isEmpty = true;
            auto it = std::find(m_rowTable.begin(), m_rowTable.end(), g);

            if (it != m_rowTable.end()) {
                int row = std::distance(m_rowTable.begin(), it);
                if (hasGroups() && notifyModel) {
                    beginRemoveRows(QModelIndex(), row, row);
                }
                m_rowTable.erase(it);
                if (hasGroups() && notifyModel) {
                    endRemoveRows();
                }
                m_emptyGroups.push_back(g);
            } else {
                qCWarning(LOG_KTE) << "Group " << g << " not found in row table!!";
            }
        }

    } else {
        if (!g->filtered.empty()) {
            // Move off empty group list
            g->isEmpty = false;

            int row = 0; // Find row where to insert
            for (size_t a = 0; a < m_rowTable.size(); a++) {
                if (g->orderBefore(m_rowTable[a])) {
                    row = a;
                    break;
                }
                row = a + 1;
            }

            if (notifyModel) {
                if (hasGroups()) {
                    beginInsertRows(QModelIndex(), row, row);
                } else {
                    beginInsertRows(QModelIndex(), 0, g->filtered.size());
                }
            }
            m_rowTable.insert(m_rowTable.begin() + row, g);
            if (notifyModel) {
                endInsertRows();
            }
            m_emptyGroups.erase(std::remove(m_emptyGroups.begin(), m_emptyGroups.end(), g), m_emptyGroups.end());
        }
    }
}

bool KateCompletionModel::indexIsItem(const QModelIndex &index) const
{
    if (!hasGroups()) {
        return true;
    }

    if (groupOfParent(index)) {
        return true;
    }

    return false;
}

void KateCompletionModel::slotModelReset()
{
    createGroups();

    // debugStats();
}

void KateCompletionModel::debugStats()
{
    if (!hasGroups()) {
        qCDebug(LOG_KTE) << "Model groupless, " << m_ungrouped->filtered.size() << " items.";
    } else {
        qCDebug(LOG_KTE) << "Model grouped (" << m_rowTable.size() << " groups):";
        for (Group *g : m_rowTable) {
            qCDebug(LOG_KTE) << "Group" << g << "count" << g->filtered.size();
        }
    }
}

bool KateCompletionModel::hasCompletionModel() const
{
    return !m_completionModels.empty();
}

int KateCompletionModel::translateColumn(int sourceColumn) const
{
    if (m_columnMerges.empty()) {
        return sourceColumn;
    }

    /* Debugging - dump column merge list

    QString columnMerge;
    for (const QList<int> &list : m_columnMerges) {
      columnMerge += '[';
      for (int column : list) {
        columnMerge += QString::number(column) + QLatin1Char(' ');
      }
      columnMerge += "] ";
    }

    qCDebug(LOG_KTE) << k_funcinfo << columnMerge;*/

    int c = 0;
    for (const auto &list : m_columnMerges) {
        for (int column : list) {
            if (column == sourceColumn) {
                return c;
            }
        }
        c++;
    }
    return -1;
}

int KateCompletionModel::groupingAttributes(int attribute) const
{
    int ret = 0;

    if (countBits(attribute & ScopeTypeMask) > 1) {
        qCWarning(LOG_KTE) << "Invalid completion model metadata: more than one scope type modifier provided.";
    }
    if (attribute & KTextEditor::CodeCompletionModel::GlobalScope) {
        ret |= KTextEditor::CodeCompletionModel::GlobalScope;
    } else if (attribute & KTextEditor::CodeCompletionModel::NamespaceScope) {
        ret |= KTextEditor::CodeCompletionModel::NamespaceScope;
    } else if (attribute & KTextEditor::CodeCompletionModel::LocalScope) {
        ret |= KTextEditor::CodeCompletionModel::LocalScope;
    }

    if (countBits(attribute & AccessTypeMask) > 1) {
        qCWarning(LOG_KTE) << "Invalid completion model metadata: more than one access type modifier provided.";
    }
    if (attribute & KTextEditor::CodeCompletionModel::Public) {
        ret |= KTextEditor::CodeCompletionModel::Public;
    } else if (attribute & KTextEditor::CodeCompletionModel::Protected) {
        ret |= KTextEditor::CodeCompletionModel::Protected;
    } else if (attribute & KTextEditor::CodeCompletionModel::Private) {
        ret |= KTextEditor::CodeCompletionModel::Private;
    }

    return ret;
}

int KateCompletionModel::countBits(int value)
{
    int count = 0;
    for (int i = 1; i; i <<= 1) {
        if (i & value) {
            count++;
        }
    }

    return count;
}

KateCompletionModel::Item::Item(bool doInitialMatch, KateCompletionModel *m, const HierarchicalModelHandler &handler, ModelRow sr)
    : m_sourceRow(sr)
    , matchCompletion(StartsWithMatch)
    , m_haveExactMatch(false)
{
    inheritanceDepth = handler.getData(CodeCompletionModel::InheritanceDepth, m_sourceRow.second).toInt();
    m_unimportant = handler.getData(CodeCompletionModel::UnimportantItemRole, m_sourceRow.second).toBool();

    QModelIndex nameSibling = sr.second.sibling(sr.second.row(), CodeCompletionModel::Name);
    m_nameColumn = nameSibling.data(Qt::DisplayRole).toString();

    if (doInitialMatch) {
        match(m);
    }
}

bool KateCompletionModel::Item::lessThan(KateCompletionModel *model, const Item &rhs) const
{
    int ret = 0;

    // qCDebug(LOG_KTE) << c1 << " c/w " << c2 << " -> " << (model->isSortingReverse() ? ret > 0 : ret < 0) << " (" << ret << ")";

    if (m_unimportant && !rhs.m_unimportant) {
        return false;
    }

    if (!m_unimportant && rhs.m_unimportant) {
        return true;
    }

    if (matchCompletion < rhs.matchCompletion) {
        // enums are ordered in the order items should be displayed
        return true;
    }
    if (matchCompletion > rhs.matchCompletion) {
        return false;
    }

    ret = inheritanceDepth - rhs.inheritanceDepth;

    if (ret == 0) {
        auto it = model->m_currentMatch.constFind(rhs.m_sourceRow.first);
        if (it != model->m_currentMatch.cend()) {
            const QString &filter = it.value();
            bool thisStartWithFilter = m_nameColumn.startsWith(filter, Qt::CaseSensitive);
            bool rhsStartsWithFilter = rhs.m_nameColumn.startsWith(filter, Qt::CaseSensitive);

            if (thisStartWithFilter && !rhsStartsWithFilter) {
                return true;
            }
            if (rhsStartsWithFilter && !thisStartWithFilter) {
                return false;
            }
        }
    }

    if (ret == 0) {
        // Do not use localeAwareCompare, because it is simply too slow for a list of about 1000 items
        ret = QString::compare(m_nameColumn, rhs.m_nameColumn, Qt::CaseInsensitive);
    }

    if (ret == 0) {
        // FIXME need to define a better default ordering for multiple model display
        ret = m_sourceRow.second.row() - rhs.m_sourceRow.second.row();
    }

    return ret < 0;
}

void KateCompletionModel::Group::addItem(const Item &i, bool notifyModel)
{
    if (isEmpty) {
        notifyModel = false;
    }

    QModelIndex groupIndex;
    if (notifyModel) {
        groupIndex = model->indexForGroup(this);
    }

    if (notifyModel) {
        auto comp = [this](const Item &left, const Item &right) {
            return left.lessThan(model, right);
        };
        prefilter.insert(std::upper_bound(prefilter.begin(), prefilter.end(), i, comp), i);
    } else {
        prefilter.push_back(i);
    }

    if (i.isVisible()) {
        if (notifyModel) {
            auto comp = [this](const Item &left, const Item &right) {
                return left.lessThan(model, right);
            };
            auto it = std::upper_bound(filtered.begin(), filtered.end(), i, comp);
            const auto rowNumber = it - filtered.begin();
            model->beginInsertRows(groupIndex, rowNumber, rowNumber);
            filtered.insert(it, i);
        } else {
            // we will sort it later
            filtered.push_back(i);
        }
    }

    if (notifyModel) {
        model->endInsertRows();
    }
}

bool KateCompletionModel::Group::removeItem(const ModelRow &row)
{
    for (size_t pi = 0; pi < prefilter.size(); ++pi) {
        if (prefilter[pi].sourceRow() == row) {
            int index = rowOf(row);
            if (index != -1) {
                model->beginRemoveRows(model->indexForGroup(this), index, index);
                filtered.erase(filtered.begin() + index);
            }

            prefilter.erase(prefilter.begin() + pi);

            if (index != -1) {
                model->endRemoveRows();
            }

            return index != -1;
        }
    }

    Q_ASSERT(false);
    return false;
}

KateCompletionModel::Group::Group(const QString &title, int attribute, KateCompletionModel *m)
    : model(m)
    , attribute(attribute)
    // ugly hack to add some left margin
    , title(QLatin1Char(' ') + title)
    , isEmpty(true)
    , customSortingKey(-1)
{
    Q_ASSERT(model);
}

void KateCompletionModel::Group::resort()
{
    auto comp = [this](const Item &left, const Item &right) {
        return left.lessThan(model, right);
    };
    std::stable_sort(filtered.begin(), filtered.end(), comp);
    model->hideOrShowGroup(this);
}

void KateCompletionModel::resort()
{
    for (Group *g : m_rowTable) {
        g->resort();
    }

    for (Group *g : m_emptyGroups) {
        g->resort();
    }

    // call updateBestMatches here, so they are moved to the top again.
    updateBestMatches();
}

void KateCompletionModel::Group::clear()
{
    prefilter.clear();
    filtered.clear();
    isEmpty = true;
}

uint KateCompletionModel::filteredItemCount() const
{
    uint ret = 0;
    for (Group *group : m_rowTable) {
        ret += group->filtered.size();
    }

    return ret;
}

bool KateCompletionModel::shouldMatchHideCompletionList() const
{
    // @todo Make this faster

    bool doHide = false;
    CodeCompletionModel *hideModel = nullptr;

    for (Group *group : m_rowTable) {
        for (const Item &item : group->filtered) {
            if (item.haveExactMatch()) {
                KTextEditor::CodeCompletionModelControllerInterface *iface3 =
                    qobject_cast<KTextEditor::CodeCompletionModelControllerInterface *>(item.sourceRow().first);
                bool hide = false;
                if (!iface3) {
                    hide = true;
                }
                if (iface3
                    && iface3->matchingItem(item.sourceRow().second) == KTextEditor::CodeCompletionModelControllerInterface::HideListIfAutomaticInvocation) {
                    hide = true;
                }
                if (hide) {
                    doHide = true;
                    hideModel = item.sourceRow().first;
                }
            }
        }
    }

    if (doHide) {
        // Check if all other visible items are from the same model
        for (Group *group : m_rowTable) {
            for (const Item &item : group->filtered) {
                if (item.sourceRow().first != hideModel) {
                    return false;
                }
            }
        }
    }

    return doHide;
}

static inline QChar toLower(QChar c)
{
    return c.isLower() ? c : c.toLower();
}

bool KateCompletionModel::matchesAbbreviation(const QString &word, const QString &typed, int &score)
{
    // A mismatch is very likely for random even for the first letter,
    // thus this optimization makes sense.

    // We require that first letter must match before we do fuzzy matching.
    // Not sure how well this well it works in practice, but seems ok so far.
    // Also, 0 might not be the first letter. Some sources add a space or a marker
    // at the beginning. So look for first letter
    const int firstLetter = [&word] {
        for (auto it = word.cbegin(); it != word.cend(); ++it) {
            if (it->isLetter())
                return int(it - word.cbegin());
        }
        return 0;
    }();

    QStringView wordView = word;
    wordView = wordView.mid(firstLetter);

    if (toLower(wordView.at(0)) != toLower(typed.at(0))) {
        return false;
    }

    const auto res = KFuzzyMatcher::match(typed, wordView);
    score = res.score;
    return res.matched;
}

static inline bool containsAtWordBeginning(const QString &word, const QString &typed)
{
    if (typed.size() > word.size()) {
        return false;
    }

    for (int i = 1; i < word.size(); i++) {
        // The current position is a word beginning if the previous character was an underscore
        // or if the current character is uppercase. Subsequent uppercase characters do not count,
        // to handle the special case of UPPER_CASE_VARS properly.
        const QChar c = word.at(i);
        const QChar prev = word.at(i - 1);
        if (!(prev == QLatin1Char('_') || (c.isUpper() && !prev.isUpper()))) {
            continue;
        }
        if (QStringView(word).mid(i).startsWith(typed, Qt::CaseInsensitive)) {
            return true;
        }

        // If we do not have enough string left, return early
        if (word.size() - i < typed.size()) {
            return false;
        }
    }
    return false;
}

KateCompletionModel::Item::MatchType KateCompletionModel::Item::match(KateCompletionModel *model)
{
    const QString match = model->currentCompletion(m_sourceRow.first);

    m_haveExactMatch = false;

    // Hehe, everything matches nothing! (ie. everything matches a blank string)
    if (match.isEmpty()) {
        return PerfectMatch;
    }
    if (m_nameColumn.isEmpty()) {
        return NoMatch;
    }

    matchCompletion = (m_nameColumn.startsWith(match) ? StartsWithMatch : NoMatch);

    if (matchCompletion == NoMatch && !m_nameColumn.isEmpty() && !match.isEmpty()) {
        // if still no match, try abbreviation matching
        int score = 0;
        if (matchesAbbreviation(m_nameColumn, match, score)) {
            inheritanceDepth -= score;
            matchCompletion = AbbreviationMatch;
        }
    }

    if (matchCompletion == NoMatch) {
        // if no match, try for "contains"
        // Only match when the occurrence is at a "word" beginning, marked by
        // an underscore or a capital. So Foo matches BarFoo and Bar_Foo, but not barfoo.
        // Starting at 1 saves looking at the beginning of the word, that was already checked above.
        if (containsAtWordBeginning(m_nameColumn, match)) {
            matchCompletion = ContainsMatch;
        }
    }

    if (matchCompletion && match.length() == m_nameColumn.length()) {
        matchCompletion = PerfectMatch;
        m_haveExactMatch = true;
    }

    return matchCompletion;
}

bool KateCompletionModel::Item::isVisible() const
{
    return matchCompletion;
}

const KateCompletionModel::ModelRow &KateCompletionModel::Item::sourceRow() const
{
    return m_sourceRow;
}

QString KateCompletionModel::currentCompletion(KTextEditor::CodeCompletionModel *model) const
{
    return m_currentMatch.value(model);
}

void KateCompletionModel::addCompletionModel(KTextEditor::CodeCompletionModel *model)
{
    if (m_completionModels.contains(model)) {
        return;
    }

    m_completionModels.push_back(model);

    connect(model, &KTextEditor::CodeCompletionModel::rowsInserted, this, &KateCompletionModel::slotRowsInserted);
    connect(model, &KTextEditor::CodeCompletionModel::rowsRemoved, this, &KateCompletionModel::slotRowsRemoved);
    connect(model, &KTextEditor::CodeCompletionModel::modelReset, this, &KateCompletionModel::slotModelReset);

    // This performs the reset
    createGroups();
}

void KateCompletionModel::setCompletionModel(KTextEditor::CodeCompletionModel *model)
{
    clearCompletionModels();
    addCompletionModel(model);
}

void KateCompletionModel::setCompletionModels(const QList<KTextEditor::CodeCompletionModel *> &models)
{
    // if (m_completionModels == models)
    // return;

    clearCompletionModels();

    m_completionModels = models;

    for (KTextEditor::CodeCompletionModel *model : models) {
        connect(model, &KTextEditor::CodeCompletionModel::rowsInserted, this, &KateCompletionModel::slotRowsInserted);
        connect(model, &KTextEditor::CodeCompletionModel::rowsRemoved, this, &KateCompletionModel::slotRowsRemoved);
        connect(model, &KTextEditor::CodeCompletionModel::modelReset, this, &KateCompletionModel::slotModelReset);
    }

    // This performs the reset
    createGroups();
}

QList<KTextEditor::CodeCompletionModel *> KateCompletionModel::completionModels() const
{
    return m_completionModels;
}

void KateCompletionModel::removeCompletionModel(CodeCompletionModel *model)
{
    if (!model || !m_completionModels.contains(model)) {
        return;
    }

    bool willCreateGroups = (m_completionModels.size() - 1) > 0;

    if (!willCreateGroups) {
        beginResetModel();
    }
    m_currentMatch.remove(model);

    clearGroups();

    model->disconnect(this);

    m_completionModels.removeAll(model);
    if (!willCreateGroups) {
        endResetModel();
    }

    if (willCreateGroups) {
        // This performs the reset
        createGroups();
    }
}

void KateCompletionModel::makeGroupItemsUnique(bool onlyFiltered)
{
    struct FilterItems {
        FilterItems(KateCompletionModel &model, const QList<KTextEditor::CodeCompletionModel *> &needShadowing)
            : m_model(model)
            , m_needShadowing(needShadowing)
        {
        }

        QHash<QString, CodeCompletionModel *> had;
        KateCompletionModel &m_model;
        const QList<KTextEditor::CodeCompletionModel *> &m_needShadowing;

        void filter(std::vector<Item> &items)
        {
            std::vector<Item> temp;
            temp.reserve(items.size());
            for (const Item &item : items) {
                auto it = had.constFind(item.name());
                if (it != had.constEnd() && *it != item.sourceRow().first && m_needShadowing.contains(item.sourceRow().first)) {
                    continue;
                }

                had.insert(item.name(), item.sourceRow().first);
                temp.push_back(item);
            }
            items.swap(temp);
        }

        void filter(Group *group, bool onlyFiltered)
        {
            if (group->prefilter.size() == group->filtered.size()) {
                // Filter only once
                filter(group->filtered);
                if (!onlyFiltered) {
                    group->prefilter = group->filtered;
                }
            } else {
                // Must filter twice
                filter(group->filtered);
                if (!onlyFiltered) {
                    filter(group->prefilter);
                }
            }

            if (group->filtered.empty()) {
                m_model.hideOrShowGroup(group);
            }
        }
    };

    QList<KTextEditor::CodeCompletionModel *> needShadowing;
    for (KTextEditor::CodeCompletionModel *model : std::as_const(m_completionModels)) {
        KTextEditor::CodeCompletionModelControllerInterface *v4 = qobject_cast<KTextEditor::CodeCompletionModelControllerInterface *>(model);
        if (v4 && v4->shouldHideItemsWithEqualNames()) {
            needShadowing.push_back(model);
        }
    }

    if (needShadowing.isEmpty()) {
        return;
    }

    FilterItems filter(*this, needShadowing);

    filter.filter(m_ungrouped, onlyFiltered);

    for (Group *group : m_rowTable) {
        filter.filter(group, onlyFiltered);
    }
}

// Updates the best-matches group
void KateCompletionModel::updateBestMatches()
{
    // We cannot do too many operations here, because they are all executed
    // whenever a character is added. Would be nice if we could split the
    // operations up somewhat using a timer.
    int maxMatches = 300;

    m_updateBestMatchesTimer->stop();
    // Maps match-qualities to ModelRows paired together with the BestMatchesCount returned by the items.
    typedef QMultiMap<int, QPair<int, ModelRow>> BestMatchMap;
    BestMatchMap matches;

    if (!hasGroups()) {
        // If there is no grouping, just change the order of the items, moving the best matching ones to the front
        QMultiMap<int, int> rowsForQuality;

        int row = 0;
        for (const Item &item : m_ungrouped->filtered) {
            ModelRow source = item.sourceRow();

            QVariant v = source.second.data(CodeCompletionModel::BestMatchesCount);

            if (v.userType() == QMetaType::Int && v.toInt() > 0) {
                int quality = contextMatchQuality(source);
                if (quality > 0) {
                    rowsForQuality.insert(quality, row);
                }
            }

            ++row;
            --maxMatches;
            if (maxMatches < 0) {
                break;
            }
        }

        if (!rowsForQuality.isEmpty()) {
            // Rewrite m_ungrouped->filtered in a new order
            QSet<int> movedToFront;
            std::vector<Item> newFiltered;
            newFiltered.reserve(rowsForQuality.size());
            movedToFront.reserve(rowsForQuality.size());
            for (auto it = rowsForQuality.constBegin(); it != rowsForQuality.constEnd(); ++it) {
                newFiltered.push_back(m_ungrouped->filtered[it.value()]);
                movedToFront.insert(it.value());
            }
            std::reverse(newFiltered.begin(), newFiltered.end());

            int size = m_ungrouped->filtered.size();
            for (int a = 0; a < size; ++a) {
                if (!movedToFront.contains(a)) {
                    newFiltered.push_back(m_ungrouped->filtered[a]);
                }
            }
            m_ungrouped->filtered.swap(newFiltered);
        }
        return;
    }

    ///@todo Cache the CodeCompletionModel::BestMatchesCount
    for (Group *g : m_rowTable) {
        if (g == m_bestMatches) {
            continue;
        }
        for (int a = 0; a < (int)g->filtered.size(); a++) {
            ModelRow source = g->filtered[a].sourceRow();

            QVariant v = source.second.data(CodeCompletionModel::BestMatchesCount);

            if (v.userType() == QMetaType::Int && v.toInt() > 0) {
                // Return the best match with any of the argument-hints

                int quality = contextMatchQuality(source);
                if (quality > 0) {
                    matches.insert(quality, qMakePair(v.toInt(), g->filtered[a].sourceRow()));
                }
                --maxMatches;
            }

            if (maxMatches < 0) {
                break;
            }
        }
        if (maxMatches < 0) {
            break;
        }
    }

    // Now choose how many of the matches will be taken. This is done with the rule:
    // The count of shown best-matches should equal the average count of their BestMatchesCounts
    int cnt = 0;
    int matchesSum = 0;
    BestMatchMap::const_iterator it = matches.constEnd();
    while (it != matches.constBegin()) {
        --it;
        ++cnt;
        matchesSum += (*it).first;
        if (cnt > matchesSum / cnt) {
            break;
        }
    }

    m_bestMatches->filtered.clear();

    it = matches.constEnd();

    while (it != matches.constBegin() && cnt > 0) {
        --it;
        --cnt;

        m_bestMatches->filtered.push_back(Item(true, this, HierarchicalModelHandler((*it).second.first), (*it).second));
    }

    hideOrShowGroup(m_bestMatches);
}

void KateCompletionModel::rowSelected(const QModelIndex & /*row*/) const
{
    ///@todo delay this
    int rc = widget()->argumentHintModel()->rowCount(QModelIndex());
    if (rc == 0) {
        return;
    }

    // For now, simply update the whole column 0
    QModelIndex start = widget()->argumentHintModel()->index(0, 0);
    QModelIndex end = widget()->argumentHintModel()->index(rc - 1, 0);

    widget()->argumentHintModel()->emitDataChanged(start, end);
}

void KateCompletionModel::clearCompletionModels()
{
    if (m_completionModels.empty()) {
        return;
    }

    beginResetModel();
    for (CodeCompletionModel *model : std::as_const(m_completionModels)) {
        model->disconnect(this);
    }

    m_completionModels.clear();

    m_currentMatch.clear();

    clearGroups();
    endResetModel();
}

#include "moc_katecompletionmodel.cpp"
