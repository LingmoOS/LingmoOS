// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgethandler.h"
#include <QSettings>
#include <QDebug>
#include <QCoreApplication>

WIDGETS_FRAME_BEGIN_NAMESPACE
QString WidgetHandlerImpl::id() const { return m_id;}

QSize WidgetHandlerImpl::size() const
{
    return size(m_type, true);
}

int WidgetHandlerImpl::roundedCornerRadius() const
{
    return UI::RoundedRectRadius;
}

QSize WidgetHandlerImpl::size(const IWidget::Type type, const bool instance)
{
    switch (type) {
    case IWidget::Small:
        return instance ? UI::Widget::SmallSize : UI::Widget::StoreSmallSize;
    case IWidget::Middle:
        return instance ? UI::Widget::MiddleSize : UI::Widget::StoreMiddleSize;
    case IWidget::Large:
        return instance ? UI::Widget::LargeSize : UI::Widget::StoreLargeSize;
    case IWidget::Custom:
        return instance ? UI::Widget::CustomSize : UI::Widget::StoreCustomSize;
    }
    return QSize();
}

QString WidgetHandlerImpl::typeString() const
{
    return typeString(m_type);
}

void WidgetHandlerImpl::clear()
{
    if (unavailableDS())
        return;

    m_dataStore->beginGroup(m_id);
    m_dataStore->remove("");
    m_dataStore->endGroup();
}

bool WidgetHandlerImpl::unavailableDS() const
{
    return !m_isUserAreaInstance;
}

WidgetHandlerImpl *WidgetHandlerImpl::get(WidgetHandler *handler)
{
    return dynamic_cast<WidgetHandlerImpl *>(handler);
}

IWidgetPlugin::Type WidgetHandlerImpl::pluginType() const
{
    return m_pluginType;
}

bool WidgetHandlerImpl::isFixted() const
{
    return m_pluginType == IWidgetPlugin::Resident || m_pluginType == IWidgetPlugin::Alone;
}

bool WidgetHandlerImpl::isCustom() const
{
    return m_pluginType == IWidgetPlugin::Alone;
}

void WidgetHandlerImpl::setDataStore(DataStore *store)
{
    m_dataStore = store;
}

WidgetHandlerImpl::WidgetHandlerImpl()
    : WidgetHandler()
{
}

WidgetHandlerImpl::~WidgetHandlerImpl()
{
}

QVariant WidgetHandlerImpl::value(const QString &key, const QVariant &defaultValue) const
{
    if (unavailableDS())
        return defaultValue;

    m_dataStore->beginGroup(m_id);
    const auto &v = m_dataStore->value(key, defaultValue);
    m_dataStore->endGroup();
    qDebug(dwLog()) << "value: " << key << v;
    return v;
}

void WidgetHandlerImpl::resetValue(const QString &key)
{
    if (unavailableDS())
        return;

    qDebug(dwLog()) << "resetValue: " << key;
    m_dataStore->beginGroup(m_id);
    m_dataStore->setValue(key, QVariant());
    m_dataStore->endGroup();
}

void WidgetHandlerImpl::setValue(const QString &key, const QVariant &value)
{
    if (unavailableDS())
        return;

    qDebug(dwLog()) << "setValue: " << key << value;
    m_dataStore->beginGroup(m_id);
    m_dataStore->setValue(key, value);
    m_dataStore->endGroup();
}

void WidgetHandlerImpl::removeValue(const QString &key)
{
    if (unavailableDS())
        return;

    qDebug(dwLog()) << "removeValue: " << key;
    m_dataStore->beginGroup(m_id);
    m_dataStore->remove(key);
    m_dataStore->endGroup();
}

bool WidgetHandlerImpl::containsValue(const QString &key)
{
    if (unavailableDS())
        return false;

    m_dataStore->beginGroup(m_id);
    const bool res = m_dataStore->contains(key);
    m_dataStore->endGroup();
    return res;
}

QString WidgetHandlerImpl::typeString(const Widgets::IWidget::Type type)
{
    Q_ASSERT(type >= IWidget::Invalid && type <= IWidget::Large);
    static const QVector<QString > types{
        qApp->translate("WidgetHandler", "Invalid"),
        qApp->translate("WidgetHandler", "Small"),
        qApp->translate("WidgetHandler", "Middle"),
        qApp->translate("WidgetHandler", "Large")};

    return types[type];
}
WIDGETS_FRAME_END_NAMESPACE
