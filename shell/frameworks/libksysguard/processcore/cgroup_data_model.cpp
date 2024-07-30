/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "cgroup_data_model.h"

#include "Formatter.h"
#include "cgroup.h"
#include "extended_process_list.h"
#include "process_attribute.h"
#include "process_data_model.h"

#include <KLocalizedString>

#include <QDebug>
#include <QDir>
#include <QMetaEnum>
#include <QTimer>

#include <algorithm>
#include <filesystem>

using namespace KSysGuard;

class KSysGuard::CGroupDataModelPrivate
{
public:
    QList<KSysGuard::Process *> processesFor(CGroup *app);

    QSharedPointer<ExtendedProcesses> m_processes;
    QTimer *m_updateTimer;
    ProcessAttributeModel *m_attributeModel = nullptr;
    QHash<QString, KSysGuard::ProcessAttribute *> m_availableAttributes;
    QList<KSysGuard::ProcessAttribute *> m_enabledAttributes;

    bool m_available = false;
    QString m_root;
    QScopedPointer<CGroup> m_rootGroup;

    QList<CGroup *> m_cGroups; // an ordered list of unfiltered cgroups from our root
    QHash<QString, CGroup *> m_cgroupMap; // all known cgroups from our root
    QHash<QString, CGroup *> m_oldGroups;
    QHash<CGroup *, QList<Process *>> m_processMap; // cached mapping of cgroup to list of processes of that group
};

class GroupNameAttribute : public ProcessAttribute
{
public:
    GroupNameAttribute(QObject *parent)
        : KSysGuard::ProcessAttribute(QStringLiteral("menuId"), i18nc("@title", "Desktop ID"), parent)
    {
    }
    QVariant cgroupData(CGroup *app, const QList<KSysGuard::Process *> &processes) const override
    {
        Q_UNUSED(processes)
        return app->service()->menuId();
    }
};

class AppIconAttribute : public KSysGuard::ProcessAttribute
{
public:
    AppIconAttribute(QObject *parent)
        : KSysGuard::ProcessAttribute(QStringLiteral("iconName"), i18nc("@title", "Icon"), parent)
    {
    }
    QVariant cgroupData(CGroup *app, const QList<KSysGuard::Process *> &processes) const override
    {
        Q_UNUSED(processes)
        return app->service()->icon();
    }
};

class AppNameAttribute : public KSysGuard::ProcessAttribute
{
public:
    AppNameAttribute(QObject *parent)
        : KSysGuard::ProcessAttribute(QStringLiteral("appName"), i18nc("@title", "Name"), parent)
    {
    }
    QVariant cgroupData(CGroup *app, const QList<KSysGuard::Process *> &processes) const override
    {
        Q_UNUSED(processes)
        return app->service()->name();
    }
};

CGroupDataModel::CGroupDataModel(QObject *parent)
    : CGroupDataModel(QStringLiteral("/"), parent)
{
}

CGroupDataModel::CGroupDataModel(const QString &root, QObject *parent)
    : QAbstractItemModel(parent)
    , d(new CGroupDataModelPrivate)
{
    d->m_updateTimer = new QTimer(this);
    d->m_processes = ExtendedProcesses::instance();

    QList<ProcessAttribute *> attributes = d->m_processes->attributes();
    attributes.reserve(attributes.count() + 3);
    attributes.append(new GroupNameAttribute(this));
    attributes.append(new AppNameAttribute(this));
    attributes.append(new AppIconAttribute(this));
    for (auto attr : std::as_const(attributes)) {
        d->m_availableAttributes[attr->id()] = attr;
    }

    if (CGroup::cgroupSysBasePath().isEmpty()) {
        return;
    }

    connect(d->m_updateTimer, &QTimer::timeout, this, [this]() {
        update();
    });
    d->m_updateTimer->setInterval(2000);
    d->m_updateTimer->start();

    // updateAllProcesses will delete processes that no longer exist, a method that
    // can be called by any user of the shared Processes
    // so clear out our cache of cgroup -> process whenever anything gets removed
    connect(d->m_processes.data(), &Processes::beginRemoveProcess, this, [this]() {
        d->m_processMap.clear();
    });

    setRoot(root);
}

CGroupDataModel::~CGroupDataModel()
{
}

int CGroupDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->m_cGroups.count();
}

QModelIndex CGroupDataModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= d->m_cGroups.count()) {
        return QModelIndex();
    }
    if (parent.isValid()) {
        return QModelIndex();
    }
    return createIndex(row, column, d->m_cGroups.at(row));
}

QModelIndex CGroupDataModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int CGroupDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->m_enabledAttributes.count();
}

QStringList CGroupDataModel::availableAttributes() const
{
    return d->m_availableAttributes.keys();
}

QStringList CGroupDataModel::enabledAttributes() const
{
    QStringList rc;
    rc.reserve(d->m_enabledAttributes.size());
    for (auto attr : std::as_const(d->m_enabledAttributes)) {
        rc << attr->id();
    }
    return rc;
}

void CGroupDataModel::setEnabledAttributes(const QStringList &enabledAttributes)
{
    beginResetModel();

    QList<ProcessAttribute *> unusedAttributes = d->m_enabledAttributes;
    d->m_enabledAttributes.clear();

    for (auto attribute : enabledAttributes) {
        auto attr = d->m_availableAttributes.value(attribute, nullptr);
        if (!attr) {
            qWarning() << "Could not find attribute" << attribute;
            continue;
        }
        unusedAttributes.removeOne(attr);
        d->m_enabledAttributes << attr;
        int columnIndex = d->m_enabledAttributes.count() - 1;

        // reconnect as using the attribute in the lambda makes everything super fast
        disconnect(attr, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
        connect(attr, &KSysGuard::ProcessAttribute::dataChanged, this, [this, columnIndex](KSysGuard::Process *process) {
            auto cgroup = d->m_cgroupMap.value(process->cGroup());
            if (!cgroup) {
                return;
            }
            const QModelIndex index = getQModelIndex(cgroup, columnIndex);
            Q_EMIT dataChanged(index, index);
        });
    }

    for (auto unusedAttr : std::as_const(unusedAttributes)) {
        disconnect(unusedAttr, &KSysGuard::ProcessAttribute::dataChanged, this, nullptr);
    }

    endResetModel();

    Q_EMIT enabledAttributesChanged();
}

QModelIndex CGroupDataModel::getQModelIndex(CGroup *cgroup, int column) const
{
    Q_ASSERT(cgroup);
    int row = d->m_cGroups.indexOf(cgroup);
    return index(row, column, QModelIndex());
}

QHash<int, QByteArray> CGroupDataModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = ProcessDataModel::staticMetaObject.enumerator(ProcessDataModel::staticMetaObject.indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant CGroupDataModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }
    int attr = index.column();
    auto attribute = d->m_enabledAttributes[attr];
    switch (role) {
    case Qt::DisplayRole:
    case ProcessDataModel::FormattedValue: {
        KSysGuard::CGroup *app = reinterpret_cast<KSysGuard::CGroup *>(index.internalPointer());
        const QVariant value = attribute->cgroupData(app, d->processesFor(app));
        return KSysGuard::Formatter::formatValue(value, attribute->unit());
    }
    case ProcessDataModel::Value: {
        KSysGuard::CGroup *app = reinterpret_cast<KSysGuard::CGroup *>(index.internalPointer());
        const QVariant value = attribute->cgroupData(app, d->processesFor(app));
        return value;
    }
    case ProcessDataModel::Attribute: {
        return attribute->id();
    }
    case ProcessDataModel::Minimum: {
        return attribute->min();
    }
    case ProcessDataModel::Maximum: {
        return attribute->max();
    }
    case ProcessDataModel::ShortName: {
        if (!attribute->shortName().isEmpty()) {
            return attribute->shortName();
        }
        return attribute->name();
    }
    case ProcessDataModel::Name: {
        return attribute->name();
    }
    case ProcessDataModel::Unit: {
        return attribute->unit();
    }
    case ProcessDataModel::PIDs: {
        const auto pids = static_cast<KSysGuard::CGroup *>(index.internalPointer())->pids();
        QVariantList result;
        std::transform(pids.begin(), pids.end(), std::back_inserter(result), [](pid_t pid) {
            return int(pid);
        });
        return result;
    }
    }
    return QVariant();
}

QVariant CGroupDataModel::headerData(int section, Qt::Orientation orientation, int role) const
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
    case ProcessDataModel::ShortName: {
        if (!attribute->shortName().isEmpty()) {
            return attribute->shortName();
        }
        return attribute->name();
    }
    case ProcessDataModel::Name:
        return attribute->name();
    case ProcessDataModel::Value:
    case ProcessDataModel::Attribute: {
        return attribute->id();
    }
    case ProcessDataModel::Unit: {
        auto attribute = d->m_enabledAttributes[section];
        return attribute->unit();
    }
    case ProcessDataModel::Minimum: {
        return attribute->min();
    }
    case ProcessDataModel::Maximum: {
        return attribute->max();
    }
    default:
        break;
    }

    return QVariant();
}

ProcessAttributeModel *CGroupDataModel::attributesModel()
{
    // lazy load
    if (!d->m_attributeModel) {
        d->m_attributeModel = new KSysGuard::ProcessAttributeModel(d->m_availableAttributes.values().toVector(), this);
    }
    return d->m_attributeModel;
}

bool CGroupDataModel::isEnabled() const
{
    return d->m_updateTimer->isActive();
}

void CGroupDataModel::setEnabled(bool enabled)
{
    if (enabled) {
        d->m_updateTimer->start();
        QMetaObject::invokeMethod(
            this,
            [this] {
                update();
            },
            Qt::QueuedConnection);
    } else {
        d->m_updateTimer->stop();
    }
}

QString CGroupDataModel::root() const
{
    return d->m_root;
}

void CGroupDataModel::setRoot(const QString &root)
{
    if (root == d->m_root) {
        return;
    }
    d->m_root = root;
    Q_EMIT rootChanged();
    QMetaObject::invokeMethod(
        this,
        [this] {
            update();
        },
        Qt::QueuedConnection);

    const QString path = CGroup::cgroupSysBasePath() + root;
    bool available = QFile::exists(path);

    if (available) {
        d->m_rootGroup.reset(new CGroup(root));
    } else {
        d->m_rootGroup.reset();
    }

    if (available != d->m_available) {
        d->m_available = available;
        Q_EMIT availableChanged();
    }
}

void CGroupDataModel::update()
{
    if (!d->m_rootGroup) {
        return;
    }

    d->m_oldGroups = d->m_cgroupMap;

    Processes::UpdateFlags flags;
    for (auto attribute : std::as_const(d->m_enabledAttributes)) {
        flags |= attribute->requiredUpdateFlags();
    }

    // In an ideal world we would only the relevant process
    // but Ksysguard::Processes doesn't handle that very well
    d->m_processes->updateAllProcesses(d->m_updateTimer->interval(), flags);

    update(d->m_rootGroup.data());

    for (auto c : std::as_const(d->m_oldGroups)) {
        int row = d->m_cGroups.indexOf(c);
        if (row >= 0) {
            beginRemoveRows(QModelIndex(), row, row);
            d->m_cGroups.removeOne(c);
            endRemoveRows();
        }
        d->m_cgroupMap.remove(c->id());
        delete c;
    }
}

bool CGroupDataModel::filterAcceptsCGroup(const QString &id)
{
    return id.endsWith(QLatin1String(".service")) || id.endsWith(QLatin1String(".scope"));
}

void CGroupDataModel::update(CGroup *node)
{
    namespace fs = std::filesystem;
    const QString path = CGroup::cgroupSysBasePath() + node->id();

    // Update our own stat info
    // This may trigger some dataChanged
    node->requestPids(this, [this, node](QList<pid_t> pids) {
        auto row = d->m_cGroups.indexOf(node);
        if (row >= 0) {
            d->m_cGroups[row]->setPids(pids);
            d->m_processMap.remove(d->m_cGroups[row]);
            Q_EMIT dataChanged(index(row, 0, QModelIndex()), index(row, columnCount() - 1, QModelIndex()));
        }
    });

    std::error_code error;
    const fs::directory_iterator iterator(path.toUtf8().data(), error);
    if (error) {
        return;
    }
    for (const auto &entry : iterator) {
        if (!entry.is_directory()) {
            continue;
        }
        const QString childId = node->id() % QLatin1Char('/') % QString::fromUtf8(entry.path().filename().c_str());
        CGroup *childNode = d->m_cgroupMap[childId];
        if (!childNode) {
            childNode = new CGroup(childId);
            d->m_cgroupMap[childNode->id()] = childNode;

            if (filterAcceptsCGroup(childId)) {
                int row = d->m_cGroups.count();
                beginInsertRows(QModelIndex(), row, row);
                d->m_cGroups.append(childNode);
                endInsertRows();
            }
        }
        update(childNode);
        d->m_oldGroups.remove(childId);
    }
}

bool CGroupDataModel::isAvailable() const
{
    return d->m_available;
}

QList<Process *> CGroupDataModelPrivate::processesFor(CGroup *app)
{
    if (m_processMap.contains(app)) {
        return m_processMap.value(app);
    }

    QList<Process *> result;
    const auto pids = app->pids();
    std::for_each(pids.begin(), pids.end(), [this, &result](pid_t pid) {
        auto process = m_processes->getProcess(pid);
        if (process) {
            result.append(process);
        }
    });

    m_processMap.insert(app, result);

    return result;
}
