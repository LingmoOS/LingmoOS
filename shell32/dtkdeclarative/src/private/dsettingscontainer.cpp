// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsettingscontainer_p.h"

#include <QQueue>
#include <QQuickItem>
#include "private/qquickpositioners_p.h"
#include <private/qqmlchangeset_p.h>

#include "dconfigwrapper_p.h"

DCORE_USE_NAMESPACE;

DQUICK_BEGIN_NAMESPACE
static constexpr char const *settingsOptionObjectName = "_d_settings_option";
static constexpr char const *settingsGroupObjectName = "_d_settings_group";

// get group by key, e.g:
//     qml: SettingsGroup { key: group1; children: [SettingsGroup { key: group2 } ] }
//     key: 'group1.group2'
static SettingsGroup *groupByKey(const QList<SettingsGroup *> groups, const QString &key)
{
    for (auto group : qAsConst(groups)) {
        if (key == group->key()) {
            return group;
        }
        if (key.startsWith(group->key())) {
            const auto & childGroup = *static_cast<QList<SettingsGroup *>*>( group->children().data);

            if (auto res = groupByKey(childGroup, key.mid(group->key().size() + 1))) {
                return res;
            }
        }
    }
    return nullptr;
}

SettingsContainer::SettingsContainer(QObject *parent)
    : QObject(parent)
    , m_navigationModel(new SettingsNavigationModel(this))
    , m_contentModel(new SettingsContentModel(this))
{
}

SettingsContainer::~SettingsContainer()
{
    qDeleteAll(m_groups);
    m_groups.clear();

    m_navigationModel->deleteLater();
    m_navigationModel = nullptr;
    m_contentModel->deleteLater();
    m_contentModel = nullptr;
}

QQmlListProperty<SettingsGroup> SettingsContainer::groups()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return QQmlListProperty<SettingsGroup>(this, &m_groups);
#else
    return QQmlListProperty<SettingsGroup>(this, m_groups);
#endif
}

SettingsContentModel *SettingsContainer::contentModel() const
{
    return m_contentModel;
}

SettingsNavigationModel *SettingsContainer::navigationModel() const
{
    return m_navigationModel;
}

QQmlComponent *SettingsContainer::contentTitle() const
{
    return m_contentTitle;
}

QQmlComponent *SettingsContainer::navigationTitle() const
{
    return m_navigationTitle;
}

void SettingsContainer::setNavigationTitle(QQmlComponent *delegate) {
    m_navigationTitle = delegate;
}

void SettingsContainer::setContentTitle(QQmlComponent *delegate) {
    m_contentTitle = delegate;
}

void SettingsContainer::componentComplete()
{
    for (auto group : m_groups) {
        group->setLevel(0);
        group->setParentGroup(nullptr);
        if (m_config) {
            group->setConfig(m_config);
        }
    }

    m_contentModel->updateModel();
    m_navigationModel->updateModel();

    for (auto group : groupList()) {
        connect(group, &SettingsGroup::visibleChanged, this, &SettingsContainer::onGroupVisibleChanged);
    }
}

void SettingsContainer::classBegin()
{
}

QVector<SettingsGroup *> SettingsContainer::groupList() const
{
    QVector<SettingsGroup *> list;
    QStack<SettingsGroup*> stack;
    for (auto group : qAsConst(m_groups)) {
        stack.push_back(group);
        while (!stack.isEmpty()) {
            auto group = stack.pop();

            auto children = *static_cast<QList<SettingsGroup*>*>(group->children().data);
            // keep order when it's declaration.
            std::reverse(children.begin(), children.end());
            for (auto childGroup : qAsConst(children)) {
                stack.push(childGroup);
            }
            list.push_back(group);
        }
    }
    return list;
}

QQmlContext *SettingsContainer::creationContext()
{
    return qmlContext(this);
}

QQmlComponent *SettingsContainer::contentBackground() const
{
    return m_contentBackground;
}

void SettingsContainer::setGroupVisible(const QString &key, bool visible)
{
    if (auto group = groupByKey(m_groups, key)) {
        SettingsGroup *parentGroup = group->parentGroup();
        while (parentGroup) {
            if (!parentGroup->visible()) {
                return;
            }
            parentGroup = parentGroup->parentGroup();
        }
        group->setVisible(visible);
    }
}

bool SettingsContainer::groupVisible(const QString &key) const
{
    if (auto group = groupByKey(m_groups, key)) {
        return group->visible();
    }
    return false;
}

void SettingsContainer::resetSettings()
{
    for (auto group : qAsConst(m_groups)) {
        QList<SettingsGroup *> gs;
        gs.append(group);

        while (!gs.isEmpty()) {
            auto g = gs.takeFirst();
            // Push child groups into the list.
            auto childrenGroups = *static_cast<QList<SettingsGroup*>*>(g->children().data);
            gs.append(childrenGroups);
            // Reset all child options.
            auto childrenOptions = *static_cast<QList<SettingsOption*>*>(g->options().data);
            for (auto opt : childrenOptions) {
                opt->resetValue();
            }
        }
    }
}

void SettingsContainer::setContentBackground(QQmlComponent *contentBackground)
{
    if (m_contentBackground == contentBackground)
        return;

    m_contentBackground = contentBackground;
    Q_EMIT contentBackgroundChanged();
}

void SettingsContainer::onGroupVisibleChanged(bool visible)
{
    if (auto group = qobject_cast<SettingsGroup*>(sender())) {

        if (visible) {
            m_contentModel->insert(group);
            m_navigationModel->insert(group);
        } else {
            m_contentModel->remove(group);
            m_navigationModel->remove(group);
        }
    }
}

DConfigWrapper *SettingsContainer::config() const
{
    return m_config;
}

void SettingsContainer::setConfig(DConfigWrapper *config)
{
    if (m_config == config)
        return;

    m_config = config;
    Q_EMIT configChanged();
}

SettingsOption::SettingsOption(QObject *parent)
    : QObject(parent)
{
}

SettingsOption::~SettingsOption()
{
}

QString SettingsOption::key() const
{
    return m_key;
}

QString SettingsOption::name() const
{
    return m_name;
}

QVariant SettingsOption::value()
{
    if (!m_valueInitialized) {
        if (m_config->isValid()) {
            m_value = m_config->value(m_key);
            m_valueInitialized = true;
        }
    }
    return m_value;
}

QQmlComponent *SettingsOption::delegate() const
{
    return m_delegate;
}

// QMetaObject::indexOfProperty would be createProperty automatically if not existing;
static int indexOfProperty(const QObject * obj, const QString &name)
{
    const auto mo = obj->metaObject();
    for (int i = 0; i < mo->propertyCount(); ++i) {
        if (mo->property(i).name() == name)
            return i;
    }
    return -1;
}

void SettingsOption::setConfig(DConfigWrapper *config)
{
    m_config = config;
    int propertyIndex = indexOfProperty(m_config, m_key);
    if (propertyIndex < 0) {
        connect(m_config, &DConfigWrapper::valueChanged, this, [this](const QString &key){
            if (key == m_key) {
                setValue(m_config->value(key));
                m_valueInitialized = true;
            }
        });
    } else {
        // valueChanged is not emitted when the key of Config defined in qml
        const auto mo = m_config->metaObject();
        if (mo->property(propertyIndex).hasNotifySignal()) {
            static const int propertyChangedIndex = metaObject()->indexOfSlot("onConfigValueChanged()");
            QMetaObject::connect(m_config, mo->property(propertyIndex).notifySignalIndex(), this, propertyChangedIndex);
        }
    }
}

SettingsOption *SettingsOption::qmlAttachedProperties(QObject *object)
{
    auto item = qobject_cast<QQuickItem *>(object);
    while (item) {
        auto option = item->property(settingsOptionObjectName);
        if (!option.isNull())
            return option.value<SettingsOption *>();
        item = item->parentItem();
    }
    return nullptr;
}

void SettingsOption::onConfigValueChanged()
{
    setValue(m_config->value(m_key));
    m_valueInitialized = true;
}

void SettingsOption::setValue(QVariant value)
{
    if (value == m_value)
        return;

    m_value = value;
    if (m_config)
        m_config->setValue(m_key, value);

    Q_EMIT valueChanged(value);
}

void SettingsOption::resetValue()
{
    m_config->resetValue(m_key);
}

void SettingsOption::setKey(QString key)
{
    if (m_key == key)
        return;

    m_key = key;
    Q_EMIT keyChanged(m_key);
}

void SettingsOption::setName(QString name)
{
    if (m_name == name)
        return;

    m_name = name;
    Q_EMIT nameChanged(m_name);
}

void SettingsOption::setDelegate(QQmlComponent *delegate)
{
    if (m_delegate == delegate)
        return;

    m_delegate = delegate;
    Q_EMIT delegateChanged();
}

SettingsGroup::SettingsGroup(QObject *parent)
    : QObject(parent)
{
}

SettingsGroup::~SettingsGroup()
{
    qDeleteAll(m_options);
    m_options.clear();

    qDeleteAll(m_children);
    m_children.clear();
}

QQmlListProperty<SettingsOption> SettingsGroup::options()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return QQmlListProperty<SettingsOption>(this, &m_options);
#else
    return QQmlListProperty<SettingsOption>(this, m_options);
#endif
}

QQmlListProperty<SettingsGroup> SettingsGroup::children()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return QQmlListProperty<SettingsGroup>(this, &m_children);
#else
    return QQmlListProperty<SettingsGroup>(this, m_children);
#endif
}

QQmlComponent *SettingsGroup::background() const
{
    return m_background;
}

void SettingsGroup::setBackground(QQmlComponent *background)
{
    if (m_background == background)
        return;

    m_background = background;
    Q_EMIT backgroundChanged();
}

void SettingsGroup::setConfig(DConfigWrapper *config)
{
    for (auto childGroup : qAsConst(m_children)) {
        childGroup->setConfig(config);
    }
    for (auto option : qAsConst(m_options)) {
        option->setConfig(config);
    }
}

SettingsGroup *SettingsGroup::parentGroup() const
{
    return m_parentGroup;
}

void SettingsGroup::setParentGroup(SettingsGroup *parentGroup)
{
    m_parentGroup = parentGroup;

    for (auto group : m_children) {
        group->setParentGroup(this);
    }
}

int SettingsGroup::index() const
{
    return m_index;
}

void SettingsGroup::setIndex(const int index)
{
    if (m_index == index)
        return;

    m_index = index;
    Q_EMIT indexChanged(m_index);
}

SettingsGroup *SettingsGroup::qmlAttachedProperties(QObject *object)
{
    auto item = qobject_cast<QQuickItem *>(object);
    while (item) {
        auto group = item->property(settingsGroupObjectName);
        if (!group.isNull())
            return group.value<SettingsGroup *>();
        item = item->parentItem();
    }
    return nullptr;
}

QString SettingsGroup::key() const
{
    return m_key;
}

QString SettingsGroup::name() const
{
    return m_name;
}

bool SettingsGroup::visible() const
{
    return m_visible;
}

void SettingsGroup::setVisible(bool visible)
{
    if (m_visible == visible)
        return;

    m_visible = visible;
    Q_EMIT visibleChanged(visible);

    for (auto item : m_children) {
        item->setVisible(visible);
    }
}

int SettingsGroup::level() const
{
    return m_level;
}

void SettingsGroup::setKey(const QString &key)
{
    if (m_key == key)
        return;

    m_key = key;
    Q_EMIT keyChanged(m_key);
}

void SettingsGroup::setName(QString name)
{
    if (m_name == name)
        return;

    m_name = name;
    Q_EMIT nameChanged(m_name);
}

void SettingsGroup::setLevel(int level)
{
    m_level = level;
    for (auto childGroup : m_children) {
        childGroup->setLevel(m_level + 1);
    }
}

class SettingsCompositor
{
public:
    using OriginIndex = int;
    using CurrentIndex = int;

    ~SettingsCompositor() {}

    CurrentIndex itemIndex(QObject *item) const {
        const OriginIndex groupIndex = groupItems.indexOf(item);
        if (groupIndex != -1) {
            return currentGroups.indexOf(groups[groupIndex]);
        }
        return -1;
    }

    inline QObject *object(SettingsGroup *group) const {
        return groupItems[originIndex(group)];
    }
    inline QObject *object(CurrentIndex index) const {
        return groupItems[originIndex(index)];
    }
    inline const QVector<SettingsGroup *> &objects() const {
        return currentGroups;
    }
    inline void setObject(CurrentIndex index, QObject *obj) {
        groupItems[originIndex(index)] = obj;
    }

    int reset(const QVector<SettingsGroup *> &now)
    {
        currentGroups.clear();
        groups = now;
        groupItems.resize(groups.count());
        for (int i = 0; i < groupItems.count(); i++)
            groupItems[i] = nullptr;

        for (auto group : qAsConst(groups)) {
            if (group->visible()) {
                currentGroups.push_back(group);
            }
        }
        return currentGroups.count();
    }

    CurrentIndex insert(SettingsGroup *group)
    {
        const OriginIndex index = originIndex(group);
        for (int i = 0; i < currentGroups.count(); ++i)
        {
            // find insert position by origin order.
            if (index < originIndex(currentGroups[i])) {
                currentGroups.insert(i, group);
                return i;
            }
        }
        return -1;
    }

    CurrentIndex remove(SettingsGroup *group)
    {
        const CurrentIndex index = currentGroups.indexOf(group);
        if (index != -1) {
            currentGroups.remove(index);
        }
        return index;
    }

private:
    inline OriginIndex originIndex(CurrentIndex index) const {
        return originIndex(currentGroups[index]);
    }
    inline OriginIndex originIndex(SettingsGroup *group) const {
        return groups.indexOf(group);
    }

    // origin data
    QVector<SettingsGroup *> groups;
    // QuickItem for each origin data
    QVector<QObject *> groupItems;
    // display data
    QVector<SettingsGroup *> currentGroups;
};

class SettingsInstanceModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(SettingsInstanceModel)
public:
    explicit SettingsInstanceModelPrivate(SettingsContainer *container)
        : container(container)
    {
    }
    virtual ~SettingsInstanceModelPrivate() override;

    const QVector<SettingsGroup *> &groups() const
    {
        return compositor.objects();
    }

    void insert(SettingsGroup *group) {
        const int index = compositor.insert(group);
        if (index != -1) {
            if (auto item = qobject_cast<QQuickItem*>(compositor.object(group))) {
                item->setVisible(true);
            }

            updateIndexes(index);

            QQmlChangeSet changeSet;
            changeSet.insert(index, 1);
            Q_Q(SettingsInstanceModel);

            Q_EMIT q->modelUpdated(changeSet, false);
            Q_EMIT q->countChanged();
        }
    }

    void reset()
    {
        const int count = compositor.reset(container->groupList());

        if (count > 0) {
            updateIndexes(0);

            QQmlChangeSet changeSet;
            changeSet.insert(0, count);
            Q_Q(SettingsInstanceModel);
            Q_EMIT q->modelUpdated(changeSet, true);
            Q_EMIT q->countChanged();
        }
    }

    void remove(SettingsGroup *group) {
        const int index = compositor.remove(group);

        if (index != -1) {
            if (auto item = qobject_cast<QQuickItem*>(compositor.object(group))) {
                item->setVisible(false);
            }

            updateIndexes(index);

            Q_Q(SettingsInstanceModel);
            QQmlChangeSet changeSet;
            changeSet.remove(index, 1);
            Q_EMIT q->modelUpdated(changeSet, false);
            Q_EMIT q->countChanged();
        }
    }

    void updateIndexes(const int startIndex)
    {
        const auto &groups = compositor.objects();
        for (int i = startIndex; i < groups.count(); i++) {
            groups.at(i)->setIndex(i);
        }
    }

    SettingsContainer *container;
    SettingsCompositor compositor;
};
SettingsInstanceModelPrivate::~SettingsInstanceModelPrivate() {}

SettingsInstanceModel::SettingsInstanceModel(SettingsContainer *container)
    : QQmlInstanceModel(*(new SettingsInstanceModelPrivate(container)), container)
{
}

int SettingsInstanceModel::count() const
{
    Q_D(const SettingsInstanceModel);

    return d->groups().count();
}
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
QVariant SettingsInstanceModel::variantValue(int index, const QString &name)
{
    Q_D(SettingsInstanceModel);

    const auto &groups = d->groups();
    if (index < 0 || index >= groups.count())
        return QString();
    return QQmlEngine::contextForObject(groups.at(index))->contextProperty(name);
}

QQmlInstanceModel::ReleaseFlags SettingsInstanceModel::release(QObject *object, ReusableFlag reusableFlag)
{
    Q_UNUSED(reusableFlag);
    Q_D(const SettingsInstanceModel);

    if (d->compositor.itemIndex(object) >= 0) {
        return QQmlInstanceModel::Referenced;
    }
    return ReleaseFlags();
}
#else
QString SettingsInstanceModel::stringValue(int index, const QString &name)
{
    Q_D(SettingsInstanceModel);

    const auto &groups = d->groups();
    if (index < 0 || index >= groups.count())
        return QString();
    return QQmlEngine::contextForObject(groups.at(index))->contextProperty(name).toString();
}

QQmlInstanceModel::ReleaseFlags SettingsInstanceModel::release(QObject *object)
{
    Q_D(const SettingsInstanceModel);

    if (d->compositor.itemIndex(object) >= 0) {
        return QQmlInstanceModel::Referenced;
    }
    return nullptr;
}
#endif

void SettingsInstanceModel::setWatchedRoles(const QList<QByteArray> &roles)
{
    Q_UNUSED(roles);
}

QQmlIncubator::Status SettingsInstanceModel::incubationStatus(int index)
{
    Q_D(SettingsInstanceModel);
    if (d->compositor.object(index)) {
        return QQmlIncubator::Ready;
    }
    return QQmlIncubator::Loading;
}

int SettingsInstanceModel::indexOf(QObject *object, QObject *objectContext) const
{
    Q_UNUSED(objectContext)

    Q_D(const SettingsInstanceModel);
    return d->compositor.itemIndex(object);
}

void SettingsInstanceModel::updateModel()
{
    Q_D(SettingsInstanceModel);
    d->reset();
}

void SettingsInstanceModel::insert(SettingsGroup *group)
{
    Q_D(SettingsInstanceModel);
    d->insert(group);
}

void SettingsInstanceModel::remove(SettingsGroup *group)
{
    Q_D(SettingsInstanceModel);
    d->remove(group);
}

SettingsContentModel::SettingsContentModel(SettingsContainer *container)
    : SettingsInstanceModel (container)
{
}

bool SettingsContentModel::isValid() const
{
    Q_D(const SettingsInstanceModel);

    return d->container->contentTitle() != nullptr;
}

QObject *SettingsContentModel::object(int index, QQmlIncubator::IncubationMode incubationMode)
{
    Q_UNUSED(incubationMode)

    Q_D(SettingsInstanceModel);

    if (auto obj = d->compositor.object(index)) {
        return obj;
    }

    SettingsGroup *group = d->groups()[index];

    auto groupContext = d->container->creationContext();

    QQmlComponent groupCom(d->container->contentTitle()->creationContext()->engine());
    groupCom.setData(groupComponentData(), QUrl());
    auto groupItem = qobject_cast<QQuickItem*>(groupCom.beginCreate(groupContext));

    d->compositor.setObject(index, groupItem);

    Q_EMIT initItem(index, groupItem);

    groupCom.completeCreate();

    auto titleItem = qobject_cast<QQuickItem*>(d->container->contentTitle()->beginCreate(groupContext));
    titleItem->setParentItem(groupItem);
    titleItem->setProperty(settingsGroupObjectName, QVariant::fromValue(group));
    d->container->contentTitle()->completeCreate();

    const auto options = static_cast<QList<SettingsOption*>*>(group->options().data);
    if (options->count() > 0) {
        QQmlComponent columnCom(groupContext->engine());
        columnCom.setData(optionsLayoutComponentData(), QUrl());

        QQuickItem *columnItem = qobject_cast<QQuickItem*>(columnCom.beginCreate(groupContext));
        columnItem->setParentItem(groupItem);
        for (auto option: qAsConst(*options)) {

            if (!option->delegate())
                continue;

            auto optionContext = option->delegate()->creationContext();
            auto optionItem = option->delegate()->beginCreate(optionContext);

            QQuickItem *item = qobject_cast<QQuickItem*>(optionItem);

            item->setProperty(settingsOptionObjectName, QVariant::fromValue(option));
            item->setParentItem(columnItem);
            option->delegate()->completeCreate();
        }
        columnCom.completeCreate();

        if (auto backgroundCom = contentBackgroundByGroup(group)) {
            auto backgroundItem = qobject_cast<QQuickItem*>(backgroundCom->beginCreate(groupContext));
            backgroundItem->setProperty(settingsGroupObjectName, QVariant::fromValue(group));

            backgroundItem->setParentItem(groupItem);
            columnItem->setParentItem(backgroundItem);
            backgroundCom->completeCreate();
        }
    }

    Q_EMIT createdItem(index, groupItem);

    return groupItem;
}

QQmlComponent *SettingsContentModel::contentBackgroundByGroup(const SettingsGroup *group)
{
    Q_D(SettingsInstanceModel);
    const SettingsGroup *parent = group;
    while(parent) {
        if (parent->background()) {
            return parent->background();
        }
        parent = parent->parentGroup();
    }
    return d->container->contentBackground();
}

const QByteArray SettingsContentModel::groupComponentData()
{
    const QByteArray comData("import QtQuick 2.11\n"
                             "Column {\n"
                             "  anchors {\n"
                             "      left: parent.left\n"
                             "      right: parent.right\n"
                             "  }\n"
                             "}\n");
    return comData;
}

const QByteArray SettingsContentModel::optionsLayoutComponentData()
{
    const QByteArray comData("import QtQuick 2.11\n"
                             "Column {\n"
                             "  padding: 10\n"
                             "  spacing: 10\n"
                             "  width: parent.width\n"
                             "}\n");
    return comData;
}

SettingsNavigationModel::SettingsNavigationModel(SettingsContainer *container)
    : SettingsInstanceModel (container)
{
}

bool SettingsNavigationModel::isValid() const
{
    Q_D(const SettingsInstanceModel);

    return d->container->navigationTitle() != nullptr;
}

QObject *SettingsNavigationModel::object(int index, QQmlIncubator::IncubationMode incubationMode)
{
    Q_UNUSED(incubationMode)
    Q_D(SettingsInstanceModel);

    if (auto obj = d->compositor.object(index)) {
        return obj;
    }

    SettingsGroup *group = d->groups()[index];

    auto titleDelegate = d->container->navigationTitle();
    auto titleItem = qobject_cast<QQuickItem*>(titleDelegate->beginCreate(titleDelegate->creationContext()));
    titleItem->setProperty(settingsGroupObjectName, QVariant::fromValue(group));

    d->compositor.setObject(index, titleItem);

    Q_EMIT initItem(index, titleItem);
    titleDelegate->completeCreate();
    Q_EMIT createdItem(index, titleItem);

    return titleItem;
}

DQUICK_END_NAMESPACE
