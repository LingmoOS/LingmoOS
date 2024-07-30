/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ITEMBUILDER_H
#define ITEMBUILDER_H

#include <vector>
#include <memory>

#include <QObject>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QQuickItem>

#include "quickcharts_export.h"

class ItemIncubator;

/**
 * A helper class that instantiates QML items from QML components.
 *
 * Effectively this is a C++ version of Repeater.
 */
class QUICKCHARTS_EXPORT ItemBuilder : public QObject
{
    Q_OBJECT

public:
    ItemBuilder(QObject *parent = nullptr);
    ~ItemBuilder() override;

    QQmlComponent *component() const;
    void setComponent(QQmlComponent *newComponent);

    QQmlContext *context() const;
    void setContext(QQmlContext *newContext);

    int count() const;
    void setCount(int newCount);

    QQmlIncubator::IncubationMode incubationMode() const;
    void setIncubationMode(QQmlIncubator::IncubationMode newIncubationMode);

    QVariantMap initialProperties() const;
    void setInitialProperties(const QVariantMap &newInitialProperties);

    void build(QQuickItem *parent);

    Q_SIGNAL void beginCreate(int index, QQuickItem *item);
    Q_SIGNAL void endCreate(int index, QQuickItem *item);

    bool isFinished() const;
    Q_SIGNAL void finished();

    std::vector<std::shared_ptr<QQuickItem>> items() const;

    void clear();

private:
    QQmlComponent *m_component = nullptr;
    QQmlContext *m_context = nullptr;
    int m_count = 0;
    int m_completed = 0;
    QQmlIncubator::IncubationMode m_incubationMode = QQmlIncubator::IncubationMode::AsynchronousIfNested;
    QVariantMap m_initialProperties;

    std::vector<std::unique_ptr<ItemIncubator>> m_incubators;
    std::vector<std::shared_ptr<QQuickItem>> m_items;
};

#endif // ITEMBUILDER_H
