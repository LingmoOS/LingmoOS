// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "oceanuiobject.h"

#include "oceanuiobject_p.h"

#include <QLoggingCategory>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

namespace oceanuiV25 {
static Q_LOGGING_CATEGORY(oceanuiLog, "ocean.oceanui.object");

OceanUIObject::Private *OceanUIObject::Private::FromObject(const OceanUIObject *obj)
{
    return obj ? obj->p_ptr : nullptr;
}

OceanUIObject::Private::Private(OceanUIObject *obj)
    : m_badge(0)
    , m_pageType(Menu)
    , m_weight(-1)
    , m_flags(0)
    , q_ptr(obj)
    , m_parent(nullptr)
    , m_currentObject(nullptr)
    , m_page(nullptr)
    , m_sectionItem(nullptr)
    , m_parentItem(nullptr)
{
}

OceanUIObject::Private::~Private()
{
    if (m_sectionItem) {
        m_parentItem = nullptr;
        delete m_sectionItem;
        m_sectionItem = nullptr;
    }
    if (m_page && (!m_page->parent() || m_page->parent() == q_ptr)) {
        delete m_page;
        m_page = nullptr;
    }
    if (m_parent) {
        m_parent->p_ptr->removeChild(q_ptr);
    }
    while (!m_children.isEmpty()) {
        OceanUIObject *child = m_children.first();
        removeChild(0);
        if (child->parent() == q_ptr) {
            delete child;
        }
    }
}

bool OceanUIObject::Private::getFlagState(uint32_t flag) const
{
    return (m_flags & flag);
}

bool OceanUIObject::Private::setFlagState(uint32_t flag, bool state)
{
    if (getFlagState(flag) != state) {
        bool hioceann = getFlagState(DCC_ALL_HIOCEANN);
        bool disabled = getFlagState(DCC_ALL_DISABLED);
        if (state)
            m_flags |= flag;
        else
            m_flags &= (~flag);
        if (hioceann != getFlagState(DCC_ALL_HIOCEANN)) {
            if (!hioceann) {
                deleteSectionItem();
            }
            Q_EMIT q_ptr->visibleToAppChanged(hioceann);
        }
        bool allDisabled = getFlagState(DCC_ALL_DISABLED);
        if (disabled != allDisabled) {
            if (m_parentItem) {
                m_parentItem->setEnabled(!allDisabled);
            }
            Q_EMIT q_ptr->enabledToAppChanged(disabled);
        }
        return true;
    }
    return false;
}

uint32_t OceanUIObject::Private::getFlag() const
{
    return m_flags;
}

bool OceanUIObject::Private::addChild(OceanUIObject::Private *child)
{
    return addChild(child->q_ptr);
}

bool OceanUIObject::Private::addChild(OceanUIObject *child, bool updateParent)
{
    if (!child) {
        return false;
    }
    int index = 0;
    for (auto &&it = m_children.cbegin(); it != m_children.cend(); it++) {
        if (*it == child)
            return false;
        if (child->weight() >= (*it)->weight()) {
            index++;
        }
    }

    Q_EMIT q_ptr->childAboutToBeAoceand(q_ptr, index);
    m_children.insert(m_children.cbegin() + index, child);
    OceanUIObject::Private::FromObject(child)->SetParent(q_ptr);
    if (updateParent) {
        child->setParent(q_ptr);
    }
    Q_EMIT q_ptr->childAoceand(child);
    Q_EMIT q_ptr->childrenChanged(m_children);
    return true;
}

void OceanUIObject::Private::removeChild(int index)
{
    if (index < 0 || index >= m_children.size()) {
        // Q_ASSERT(false);
        return;
    }

    OceanUIObject *child = m_children.at(index);
    Q_EMIT q_ptr->childAboutToBeRemoved(q_ptr, index);
    m_children.erase(m_children.cbegin() + index);
    OceanUIObject::Private::FromObject(child)->SetParent(nullptr);
    Q_EMIT q_ptr->childRemoved(child);
    Q_EMIT q_ptr->childrenChanged(m_children);
}

void OceanUIObject::Private::removeChild(OceanUIObject *child)
{
    int pos = getChildIndex(child);
    if (pos >= 0)
        removeChild(pos);
}

void OceanUIObject::Private::updatePos(OceanUIObject *child)
{
    int oldPos = -1;
    int modelPos = 0;
    int index = 0;
    for (auto &&it = m_children.cbegin(); it != m_children.cend(); it++, index++) {
        if (*it == child) {
            oldPos = index;
        } else if (child->weight() >= (*it)->weight()) {
            modelPos = index + 1;
        }
    }

    int newPos = oldPos < modelPos ? modelPos - 1 : modelPos;
    if (oldPos < 0 || oldPos == newPos) {
        return;
    }
    Q_EMIT q_ptr->childAboutToBeMoved(q_ptr, modelPos, oldPos);
    m_children.move(oldPos, newPos);
    Q_EMIT q_ptr->childMoved(child);
    Q_EMIT q_ptr->childrenChanged(m_children);
}

const QVector<OceanUIObject *> &OceanUIObject::Private::getChildren() const
{
    return m_children;
}

int OceanUIObject::Private::getIndex() const
{
    return m_parent ? m_parent->p_ptr->getChildren().indexOf(q_ptr) : -1;
}

OceanUIObject *OceanUIObject::Private::getChild(int childPos) const
{
    return (childPos >= 0 && childPos < m_children.size() ? m_children[childPos] : nullptr);
}

int OceanUIObject::Private::getChildIndex(const OceanUIObject *child) const
{
    return m_children.indexOf(const_cast<OceanUIObject *>(child));
}

void OceanUIObject::Private::deleteSectionItem()
{
    if (m_sectionItem) {
        QQuickItem *item = m_sectionItem.get();
        m_sectionItem = nullptr;
        q_ptr->setParentItem(nullptr);
        for (auto &&child : m_children) {
            if (child->p_ptr->m_sectionItem) {
                Q_EMIT child->deactive();
            }
        }
        // 延时delete等动画完成
        QTimer::singleShot(500, item, [item]() {
            item->deleteLater();
        });
    }
}

void OceanUIObject::Private::data_append(QQmlListProperty<QObject> *data, QObject *o)
{
    if (!o)
        return;
    OceanUIObject::Private *that = reinterpret_cast<OceanUIObject::Private *>(data->data);
    that->m_data.append(o);
    o->setParent(that->q_ptr);

    if (OceanUIObject *obj = qobject_cast<OceanUIObject *>(o)) {
        OceanUIObject::Private *parent = reinterpret_cast<OceanUIObject::Private *>(data->data);
        parent->m_objects.append(obj);
    }
}

qsizetype OceanUIObject::Private::data_count(QQmlListProperty<QObject> *data)
{
    return reinterpret_cast<OceanUIObject::Private *>(data->data)->m_data.count();
}

QObject *OceanUIObject::Private::data_at(QQmlListProperty<QObject> *data, qsizetype i)
{
    QObjectList &d = reinterpret_cast<OceanUIObject::Private *>(data->data)->m_data;
    if (i < 0 || i >= d.size())
        return nullptr;
    return d.at(i);
}

void OceanUIObject::Private::data_clear(QQmlListProperty<QObject> *data)
{
    reinterpret_cast<OceanUIObject::Private *>(data->data)->m_data.clear();
    reinterpret_cast<OceanUIObject::Private *>(data->data)->m_objects.clear();
}

///////////////////////////////////////////////////////
OceanUIObject::OceanUIObject(QObject *parent)
    : QObject(parent)
    , p_ptr(new OceanUIObject::Private(this))
{
    connect(this, &OceanUIObject::deactive, this, [this]() {
        p_ptr->deleteSectionItem();
    });
}

OceanUIObject::~OceanUIObject()
{
    delete p_ptr;
}

QString OceanUIObject::name() const
{
    return objectName();
}

void OceanUIObject::setName(const QString &name)
{
    setObjectName(name);
    Q_EMIT nameChanged(name);
}

QString OceanUIObject::parentName() const
{
    return p_ptr->m_parentName;
}

void OceanUIObject::setParentName(const QString &parentName)
{
    if (p_ptr->m_parentName != parentName) {
        p_ptr->m_parentName = parentName;
        Q_EMIT parentNameChanged(p_ptr->m_parentName);
    }
}

quint32 OceanUIObject::weight() const
{
    return p_ptr->m_weight;
}

void OceanUIObject::setWeight(quint32 weight)
{
    if (p_ptr->m_weight != weight) {
        p_ptr->m_weight = weight;
        if (p_ptr->m_parent) {
            p_ptr->m_parent->p_ptr->updatePos(this);
        }
        Q_EMIT weightChanged(p_ptr->m_weight);
    }
}

QString OceanUIObject::displayName() const
{
    return p_ptr->m_displayName;
}

void OceanUIObject::setDisplayName(const QString &displayName)
{
    if (p_ptr->m_displayName != displayName) {
        p_ptr->m_displayName = displayName;
        Q_EMIT displayNameChanged(p_ptr->m_displayName);
    }
}

QString OceanUIObject::description() const
{
    return p_ptr->m_description;
}

void OceanUIObject::setDescription(const QString &description)
{
    if (p_ptr->m_description != description) {
        p_ptr->m_description = description;
        Q_EMIT descriptionChanged(p_ptr->m_description);
    }
}

QString OceanUIObject::icon() const
{
    return p_ptr->m_icon;
}

void OceanUIObject::setIcon(const QString &icon)
{
    if (p_ptr->m_icon != icon) {
        p_ptr->m_icon = icon;
        if (!icon.isEmpty()) {
            QQmlContext *context = qmlContext(this);
            p_ptr->m_iconSource = context ? context->resolvedUrl(icon) : icon;
        } else {
            p_ptr->m_iconSource.clear();
        }
        Q_EMIT iconChanged(p_ptr->m_icon);
        Q_EMIT iconSourceChanged(p_ptr->m_iconSource);
    }
}

QUrl OceanUIObject::iconSource() const
{
    return p_ptr->m_iconSource;
}

qint8 OceanUIObject::badge() const
{
    return p_ptr->m_badge;
}

void OceanUIObject::setBadge(qint8 badge)
{
    if (p_ptr->m_badge != badge) {
        p_ptr->m_badge = badge;
        Q_EMIT badgeChanged(p_ptr->m_badge);
    }
}

bool OceanUIObject::isVisible() const
{
    return !p_ptr->getFlagState(DCC_HIOCEANN);
}

void OceanUIObject::setVisible(bool isVisible)
{
    if (p_ptr->setFlagState(DCC_HIOCEANN, !isVisible)) {
        Q_EMIT visibleChanged(isVisible);
    }
}

bool OceanUIObject::isVisibleToApp() const
{
    return !p_ptr->getFlagState(DCC_ALL_HIOCEANN);
}

bool OceanUIObject::isEnabled() const
{
    return !p_ptr->getFlagState(DCC_DISABLED);
}

void OceanUIObject::setEnabled(bool enabled)
{
    if (p_ptr->setFlagState(DCC_DISABLED, !enabled)) {
        Q_EMIT enabledChanged(enabled);
    }
}

bool OceanUIObject::isEnabledToApp() const
{
    return !p_ptr->getFlagState(DCC_ALL_DISABLED);
}

bool OceanUIObject::canSearch() const
{
    return !p_ptr->getFlagState(DCC_CANSEARCH);
}

void OceanUIObject::setCanSearch(bool canSearch)
{
    if (p_ptr->setFlagState(DCC_CANSEARCH, !canSearch)) {
        Q_EMIT canSearchChanged(canSearch);
    }
}

OceanUIObject::BackgroundTypes OceanUIObject::backgroundType() const
{
    return BackgroundTypes(p_ptr->m_flags & DCC_MASK_BGTYPE);
}

void OceanUIObject::setBackgroundType(BackgroundTypes type)
{
    if (backgroundType() != type) {
        p_ptr->m_flags &= DCC_MASK_NOBGTYPE;
        p_ptr->m_flags |= type;
        Q_EMIT backgroundTypeChanged(type);
    }
}

OceanUIObject *OceanUIObject::currentObject()
{
    return p_ptr->m_currentObject;
}

void OceanUIObject::setCurrentObject(OceanUIObject *obj)
{
    if (p_ptr->m_currentObject != obj) {
        if (p_ptr->m_currentObject) {
            Q_EMIT p_ptr->m_currentObject->deactive();
        }
        p_ptr->m_currentObject = obj;
        Q_EMIT currentObjectChanged(p_ptr->m_currentObject);
    }
}

quint8 OceanUIObject::pageType() const
{
    return p_ptr->m_pageType;
}

void OceanUIObject::setPageType(quint8 type)
{
    if (p_ptr->m_pageType != type) {
        p_ptr->m_pageType = type;
        Q_EMIT pageTypeChanged(p_ptr->m_pageType);
    }
}

QQuickItem *OceanUIObject::getSectionItem(QObject *parent)
{
    p_ptr->deleteSectionItem();
    if (p_ptr->m_page) {
        QQmlContext *creationContext = p_ptr->m_page->creationContext();
        QQmlContext *context = new QQmlContext(creationContext);
        context->setContextProperty("oceanuiObj", this);
#if 0
        QObject *nobj = p_ptr->m_page->beginCreate(context);
        if (nobj) {
            p_ptr->m_sectionItem = qobject_cast<QQuickItem *>(nobj);
            if (!p_ptr->m_sectionItem) {
                delete nobj;
            } else {
                if (qFuzzyIsNull(p_ptr->m_sectionItem->z()))
                    p_ptr->m_sectionItem->setZ(2);
                // QQml_setParent_noEvent(sectionItem, contentItem);
                if (QQuickItem *p = qobject_cast<QQuickItem *>(parent)) {
                    p_ptr->m_sectionItem->setParentItem(p);
                    p_ptr->m_sectionItem->setParent(this);
                } else if (QQuickWindow *p = qobject_cast<QQuickWindow *>(parent)) {
                    p_ptr->m_sectionItem->setParent(this);
                } else {
                    p_ptr->m_sectionItem->setParent(this);
                }
            }
            // sections are not controlled by FxListItemSG, so apply attached properties here
        } else {
            qCWarning(oceanuiLog()) << "create page error:" << p_ptr->m_page->errorString();
            delete context;
        }
        p_ptr->m_page->completeCreate();
#else
        p_ptr->m_sectionItem = qobject_cast<QQuickItem *>(p_ptr->m_page->create(context));
        if (p_ptr->m_sectionItem) {
            p_ptr->m_sectionItem->setParent(this);
        } else {
            qCWarning(oceanuiLog()) << "create page error:" << p_ptr->m_page->errorString();
            delete context;
        }
#endif
    }
    return p_ptr->m_sectionItem.get();
}

QQuickItem *OceanUIObject::parentItem()
{
    return p_ptr->m_parentItem.get();
}

void OceanUIObject::setParentItem(QQuickItem *item)
{
    if (item != p_ptr->m_parentItem.get()) {
        p_ptr->m_parentItem = item;
        if (p_ptr->m_parentItem) {
            p_ptr->m_parentItem->setEnabled(isEnabledToApp());
        }
        Q_EMIT parentItemChanged(p_ptr->m_parentItem.get());
    }
}

QQmlComponent *OceanUIObject::page() const
{
    return p_ptr->m_page;
}

void OceanUIObject::setPage(QQmlComponent *page)
{
    if (p_ptr->m_page.get() != page) {
        p_ptr->m_page = page;
        if (p_ptr->m_page && !p_ptr->m_page->parent()) {
            p_ptr->m_page->setParent(this);
        }
        Q_EMIT pageChanged(p_ptr->m_page.get());
    }
}

QQmlListProperty<QObject> OceanUIObject::data()
{
    return QQmlListProperty<QObject>(this, p_ptr, &OceanUIObject::Private::data_append, &OceanUIObject::Private::data_count, &OceanUIObject::Private::data_at, &OceanUIObject::Private::data_clear);
}

const QVector<OceanUIObject *> &OceanUIObject::getChildren() const
{
    return p_ptr->getChildren();
}

} // namespace oceanuiV25
