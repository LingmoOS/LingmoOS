/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ItemBuilder.h"

class ItemIncubator : public QQmlIncubator
{
public:
    ItemIncubator(QQmlComponent *component, QQmlContext *context)
    {
        m_component = component;
        m_context = context;
    }

    void setStateCallback(std::function<void(QQuickItem*)> callback)
    {
        m_stateCallback = callback;
    }

    void setCompletedCallback(std::function<void(ItemIncubator*)> callback)
    {
        m_completedCallback = callback;
    }

    void create()
    {
        m_component->create(*this, m_context);
    }

    bool isFinished()
    {
        return m_finished;
    }

private:
    void setInitialState(QObject *object) override
    {
        auto item = qobject_cast<QQuickItem*>(object);
        if (item) {
            m_stateCallback(item);
        }
    }

    void statusChanged(QQmlIncubator::Status status) override
    {
        if (status == QQmlIncubator::Error) {
            qWarning() << "Could not create delegate in ItemBuilder";
            const auto e = errors();
            for (const auto &error : e) {
                qWarning() << error;
            }
            m_finished = true;
        }

        if (status == QQmlIncubator::Ready) {
            m_completedCallback(this);
            m_finished = true;
        }
    }

    QQmlComponent *m_component;
    QQmlContext *m_context;
    std::function<void(QQuickItem*)> m_stateCallback;
    std::function<void(ItemIncubator*)> m_completedCallback;
    bool m_finished = false;
};

ItemBuilder::ItemBuilder(QObject *parent)
    : QObject(parent)
{
}

ItemBuilder::~ItemBuilder()
{
    clear();
}

QQmlComponent *ItemBuilder::component() const
{
    return m_component;
}

void ItemBuilder::setComponent(QQmlComponent *newComponent)
{
    if (newComponent == m_component) {
        return;
    }

    m_component = newComponent;
    clear();
}

QQmlContext *ItemBuilder::context() const
{
    return m_context;
}

void ItemBuilder::setContext(QQmlContext *newContext)
{
    if (newContext == m_context) {
        return;
    }

    m_context = newContext;
    clear();
}


int ItemBuilder::count() const
{
    return m_count;
}

void ItemBuilder::setCount(int newCount)
{
    if (newCount == m_count) {
        return;
    }

    m_count = newCount;
    clear();
}

QQmlIncubator::IncubationMode ItemBuilder::incubationMode() const
{
    return m_incubationMode;
}

void ItemBuilder::setIncubationMode(QQmlIncubator::IncubationMode newIncubationMode)
{
    if (newIncubationMode == m_incubationMode) {
        return;
    }

    m_incubationMode = newIncubationMode;
}

QVariantMap ItemBuilder::initialProperties() const
{
    return m_initialProperties;
}

void ItemBuilder::setInitialProperties(const QVariantMap & newInitialProperties)
{
    if (newInitialProperties == m_initialProperties) {
        return;
    }

    m_initialProperties = newInitialProperties;
}

void ItemBuilder::build(QQuickItem *parent)
{
    if ((int(m_items.size()) == m_count && m_incubators.empty()) || !m_incubators.empty() || !m_component) {
        return;
    }

    m_incubators.reserve(m_count);
    std::fill_n(std::back_inserter(m_items), m_count, std::shared_ptr<QQuickItem>());

    for (int i = 0; i < m_count; ++i) {
        auto context = m_context ? m_context : qmlContext(m_component);
        auto incubator = std::make_unique<ItemIncubator>(m_component, context);

        incubator->setStateCallback([this, parent, i](QQuickItem *item) {
            item->setParentItem(parent);

            for (auto itr = m_initialProperties.keyValueBegin(); itr != m_initialProperties.keyValueEnd(); ++itr) {
                item->setProperty((*itr).first.toUtf8().data(), (*itr).second);
            }

            Q_EMIT beginCreate(i, item);
        });

        incubator->setCompletedCallback([this, i](ItemIncubator *incubator) {
            auto item = std::shared_ptr<QQuickItem>(qobject_cast<QQuickItem*>(incubator->object()));
            m_items[i] = item;

            Q_EMIT endCreate(i, item.get());

            m_completed++;
            if (m_completed == m_count) {
                QMetaObject::invokeMethod(this, [this]() {
                    m_incubators.clear();
                }, Qt::QueuedConnection);
                Q_EMIT finished();
            }
        });

        incubator->create();

        m_incubators.push_back(std::move(incubator));
    }
}

bool ItemBuilder::isFinished() const
{
    return m_completed == m_count;
}

std::vector<std::shared_ptr<QQuickItem>> ItemBuilder::items() const
{
    return m_items;
}

void ItemBuilder::clear()
{
    m_items.clear();

    if (m_incubators.size() > 0) {
        for (auto &incubator : m_incubators) {
            incubator->clear();
        }
    }
    m_incubators.clear();

    m_completed = 0;
}

#include "moc_ItemBuilder.cpp"
