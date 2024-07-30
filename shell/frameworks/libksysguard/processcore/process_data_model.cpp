/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process_data_model.h"
#include "formatter.h"

#include "processcore/extended_process_list.h"
#include "processcore/process.h"
#include "processcore/process_attribute.h"
#include "processcore/process_attribute_model.h"
#include "processcore/process_data_provider.h"

#include <QMetaEnum>
#include <QTimer>

using namespace KSysGuard;

class Q_DECL_HIDDEN KSysGuard::ProcessDataModel::Private
{
public:
    Private(ProcessDataModel *q);
    void beginInsertRow(KSysGuard::Process *parent);
    void endInsertRow();
    void beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent);
    void endMoveProcess();
    void beginRemoveRow(KSysGuard::Process *process);
    void endRemoveRow();

    void update();
    QModelIndex getQModelIndex(Process *process, int column) const;

    ProcessDataModel *q;
    KSysGuard::Process *m_rootProcess;
    QSharedPointer<ExtendedProcesses> m_processes;
    QTimer *m_timer;
    ProcessAttributeModel *m_attributeModel = nullptr;
    const int m_updateInterval = 2000;
    bool m_flatList = true;
    KSysGuard::Process *m_removingRowFor = nullptr;

    QHash<QString, KSysGuard::ProcessAttribute *> m_availableAttributes;
    QList<KSysGuard::ProcessAttribute *> m_enabledAttributes;
};

ProcessDataModel::ProcessDataModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new ProcessDataModel::Private(this))
{
}

ProcessDataModel::~ProcessDataModel()
{
}

ProcessDataModel::Private::Private(ProcessDataModel *_q)
    : q(_q)
    , m_processes(KSysGuard::ExtendedProcesses::instance())
    , m_timer(new QTimer(_q))
{
    m_rootProcess = m_processes->getProcess(-1);
    connect(m_processes.get(), &KSysGuard::Processes::beginAddProcess, q, [this](KSysGuard::Process *process) {
        beginInsertRow(process);
    });
    connect(m_processes.get(), &KSysGuard::Processes::endAddProcess, q, [this]() {
        endInsertRow();
    });
    connect(m_processes.get(), &KSysGuard::Processes::beginMoveProcess, q, [this](KSysGuard::Process *process, KSysGuard::Process *new_parent) {
        beginMoveProcess(process, new_parent);
    });
    connect(m_processes.get(), &KSysGuard::Processes::endMoveProcess, q, [this]() {
        endMoveProcess();
    });
    connect(m_processes.get(), &KSysGuard::Processes::beginRemoveProcess, q, [this](KSysGuard::Process *process) {
        beginRemoveRow(process);
    });
    connect(m_processes.get(), &KSysGuard::Processes::endRemoveProcess, q, [this]() {
        endRemoveRow();
    });

    const auto attributes = m_processes->attributes();
    m_availableAttributes.reserve(attributes.count());
    for (auto attr : attributes) {
        m_availableAttributes[attr->id()] = attr;
    }

    connect(m_timer, &QTimer::timeout, q, [this]() {
        update();
    });
    m_timer->setInterval(m_updateInterval);
    m_timer->start();
}

QVariant ProcessDataModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }

    const int attr = index.column();
    auto attribute = d->m_enabledAttributes[attr];
    switch (role) {
    case Qt::DisplayRole:
    case FormattedValue: {
        KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *>(index.internalPointer());
        const QVariant value = attribute->data(process);
        return KSysGuard::Formatter::formatValue(value, attribute->unit());
    }
    case Value: {
        KSysGuard::Process *process = reinterpret_cast<KSysGuard::Process *>(index.internalPointer());
        const QVariant value = attribute->data(process);
        return value;
    }
    case Attribute: {
        return attribute->id();
    }
    case Minimum: {
        return attribute->min();
    }
    case Maximum: {
        return attribute->max();
    }
    case ShortName: {
        if (!attribute->shortName().isEmpty()) {
            return attribute->shortName();
        }
        return attribute->name();
    }
    case Name: {
        return attribute->name();
    }
    case Unit: {
        return attribute->unit();
    }
    case UpdateInterval: {
        return d->m_updateInterval;
    }
    }
    return QVariant();
}

int ProcessDataModel::rowCount(const QModelIndex &parent) const
{
    if (d->m_flatList) {
        if (parent.isValid()) {
            return 0;
        } else {
            return d->m_processes->processCount();
        }
    }

    if (!parent.isValid()) {
        return d->m_rootProcess->children().count();
    } else if (parent.column() != 0) {
        return 0;
    }

    KSysGuard::Process *proc = reinterpret_cast<KSysGuard::Process *>(parent.internalPointer());
    Q_ASSERT(proc);
    return proc->children().count();
}

QModelIndex ProcessDataModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    if (d->m_flatList || !index.isValid()) {
        return QModelIndex();
    }

    KSysGuard::Process *proc = reinterpret_cast<KSysGuard::Process *>(index.internalPointer());
    Q_ASSERT(proc);
    return d->getQModelIndex(proc->parent(), 0);
}

QStringList ProcessDataModel::availableAttributes() const
{
    return d->m_availableAttributes.keys();
}

QStringList ProcessDataModel::enabledAttributes() const
{
    QStringList rc;
    rc.reserve(d->m_enabledAttributes.size());
    for (auto attr : d->m_enabledAttributes) {
        rc << attr->id();
    }
    return rc;
}

void ProcessDataModel::setEnabledAttributes(const QStringList &enabledAttributes)
{
    beginResetModel();

    QList<ProcessAttribute *> unusedAttributes = d->m_enabledAttributes;
    d->m_enabledAttributes.clear();

    for (auto attributeId : enabledAttributes) {
        auto attribute = d->m_availableAttributes[attributeId];
        if (!attribute) {
            continue;
        }
        unusedAttributes.removeOne(attribute);
        d->m_enabledAttributes << attribute;
        int columnIndex = d->m_enabledAttributes.count() - 1;

        // reconnect as using the columnIndex in the lambda makes everything super fast
        disconnect(attribute, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
        connect(attribute, &KSysGuard::ProcessAttribute::dataChanged, this, [this, columnIndex](KSysGuard::Process *process) {
            if (process->pid() != -1) {
                const QModelIndex index = d->getQModelIndex(process, columnIndex);
                if (index.isValid() && process != d->m_removingRowFor) {
                    Q_EMIT dataChanged(index, index);
                }
            }
        });
    }

    for (auto *unusedAttribute : std::as_const(unusedAttributes)) {
        disconnect(unusedAttribute, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
    }

    endResetModel();
    d->update();

    Q_EMIT enabledAttributesChanged();
}

bool ProcessDataModel::enabled() const
{
    return d->m_timer->isActive();
}

void ProcessDataModel::setEnabled(bool newEnabled)
{
    if (newEnabled == d->m_timer->isActive()) {
        return;
    }

    if (newEnabled) {
        d->m_timer->start();
    } else {
        d->m_timer->stop();
    }

    Q_EMIT enabledChanged();
}

bool ProcessDataModel::flatList() const
{
    return d->m_flatList;
}

void ProcessDataModel::setFlatList(bool flat)
{
    if (d->m_flatList == flat) {
        return;
    }
    beginResetModel();
    // NOTE: layoutAboutToBeChanged doesn't play well with TableView delegate recycling
    // Q_EMIT layoutAboutToBeChanged();

    d->m_flatList = flat;
    endResetModel();
    Q_EMIT flatListChanged();
}

QModelIndex ProcessDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column >= columnCount()) {
        return QModelIndex();
    }

    // Flat List
    if (d->m_flatList) {
        if (parent.isValid()) {
            return QModelIndex();
        }

        if (d->m_processes->processCount() <= row) {
            return QModelIndex();
        }

        return createIndex(row, column, d->m_processes->getAllProcesses().at(row));
    }

    // Tree mode
    KSysGuard::Process *process;

    if (parent.isValid()) {
        process = reinterpret_cast<KSysGuard::Process *>(parent.internalPointer());
    } else {
        process = d->m_rootProcess;
    }

    if (row >= process->children().count()) {
        return QModelIndex();
    } else {
        return createIndex(row, column, process->children()[row]);
    }
}

void ProcessDataModel::Private::beginInsertRow(KSysGuard::Process *process)
{
    Q_ASSERT(process);

    // Flat List
    if (m_flatList) {
        const int row = m_processes->processCount();
        q->beginInsertRows(QModelIndex(), row, row);
        return;
    }

    // Tree mode
    const int row = process->parent()->children().count();
    q->beginInsertRows(getQModelIndex(process->parent(), 0), row, row);
}

void ProcessDataModel::Private::endInsertRow()
{
    q->endInsertRows();
}

void ProcessDataModel::Private::beginRemoveRow(KSysGuard::Process *process)
{
    Q_ASSERT(process);
    Q_ASSERT(!m_removingRowFor);

    m_removingRowFor = process;
    int row = process->parent()->children().indexOf(process);
    Q_ASSERT(row >= 0);
    if (m_flatList) {
        q->beginRemoveRows(QModelIndex(), process->index(), process->index());
    } else {
        q->beginRemoveRows(getQModelIndex(process->parent(), 0), row, row);
    }
}

void ProcessDataModel::Private::endRemoveRow()
{
    m_removingRowFor = nullptr;
    q->endRemoveRows();
}

void ProcessDataModel::Private::beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent)
{
    if (m_flatList) {
        return; // We don't need to move processes when in simple mode
    }

    int current_row = process->parent()->children().indexOf(process);
    Q_ASSERT(current_row != -1);
    int new_row = new_parent->children().count();
    QModelIndex sourceParent = getQModelIndex(process->parent(), 0);
    QModelIndex destinationParent = getQModelIndex(new_parent, 0);
    q->beginMoveRows(sourceParent, current_row, current_row, destinationParent, new_row);
}

void ProcessDataModel::Private::endMoveProcess()
{
    if (m_flatList) {
        return; // We don't need to move processes when in simple mode
    }

    q->endMoveRows();
}

void ProcessDataModel::Private::update()
{
    Processes::UpdateFlags flags;
    for (auto attribute : std::as_const(m_enabledAttributes)) {
        flags |= attribute->requiredUpdateFlags();
    }

    m_processes->updateAllProcesses(m_updateInterval, flags);
}

QModelIndex ProcessDataModel::Private::getQModelIndex(KSysGuard::Process *process, int column) const
{
    if (!process || process->pid() == -1) {
        return QModelIndex(); // pid -1 is our fake process meaning the very root (never drawn).  To represent that, we return QModelIndex() which also means
                              // the top element
    }

    int row;

    if (m_flatList) {
        row = process->index();
    } else {
        row = process->parent()->children().indexOf(process);
    }

    Q_ASSERT(row != -1);
    return q->createIndex(row, column, process);
}

ProcessAttributeModel *ProcessDataModel::attributesModel()
{
    // lazy load
    if (!d->m_attributeModel) {
        d->m_attributeModel = new KSysGuard::ProcessAttributeModel(d->m_availableAttributes.values().toVector(), this);
    }
    return d->m_attributeModel;
}

int ProcessDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->m_enabledAttributes.count();
}

QHash<int, QByteArray> ProcessDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    const QMetaEnum e = QMetaEnum::fromType<AdditionalRoles>();

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant ProcessDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QVariant();
    }

    if (section < 0 || section >= columnCount()) {
        return QVariant();
    }

    auto attribute = d->m_enabledAttributes[section];

    switch (role) {
    case Qt::DisplayRole:
    case ShortName: {
        if (!attribute->shortName().isEmpty()) {
            return attribute->shortName();
        }
        return attribute->name();
    }
    case Name:
        return attribute->name();
    case Value:
    case Attribute: {
        return attribute->id();
    }
    case Unit: {
        auto attribute = d->m_enabledAttributes[section];
        return attribute->unit();
    }
    case Minimum: {
        return attribute->min();
    }
    case Maximum: {
        return attribute->max();
    }
    case UpdateInterval: {
        return d->m_updateInterval;
    }
    default:
        break;
    }

    return QVariant();
}
