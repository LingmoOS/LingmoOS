// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

#include "PatientModel.h"

#include <chrono>

#include <QDebug>
#include <QMetaMethod>

using namespace std::chrono_literals;

PatientModel::PatientModel(const QMetaObject &mo, QObject *parent)
    : QAbstractListModel(parent)
{
    initRoleNames(mo);
}

QHash<int, QByteArray> PatientModel::roleNames() const
{
    return m_roles;
}

int PatientModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent); // this is a flat list we decidedly don't care about the parent
    return m_objects.count();
}

QVariant PatientModel::data(const QModelIndex &index, int role) const
{
    if (!hasIndex(index.row(), index.column())) {
        return {};
    }
    // return QVariant::fromValue((QObject *)0x1);
    QObject *obj = m_objects.at(index.row());
    switch ((ItemRole)role) {
    case ObjectRole:
        return QVariant::fromValue(obj);
    case IndexRole:
        return QVariant::fromValue(index.row());
    }
    const QByteArray prop = m_objectProperties.value(role);
    if (prop.isEmpty()) {
        return {};
    }
    return obj->property(prop.constData());
}

bool PatientModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!hasIndex(index.row(), index.column())) {
        return false;
    }
    QObject *obj = m_objects.at(index.row());
    if (role == ObjectRole) {
        return false; // cannot set object!
    }
    const QByteArray prop = m_objectProperties.value(role);
    if (prop.isEmpty()) {
        return false;
    }
    return obj->setProperty(prop.constData(), value);
}

int PatientModel::role(const QByteArray &roleName) const
{
    return m_roles.key(roleName, -1);
}

void PatientModel::addObject(std::unique_ptr<QObject> patient)
{
    const int index = m_objects.size();
    beginInsertRows(QModelIndex(), index, index);

    QObject *object = patient.release();
    object->setParent(this);

    m_objects.append(object);
    Q_ASSERT(!m_roles.isEmpty());

    const QMetaObject *mo = object->metaObject();
    // We have all the data changed notify signals already stored, let's connect all changed signals to our property change wrapper.
    const QList<int> signalIndices = m_signalIndexToProperties.keys();
    for (const auto &signalMethodIndex : signalIndices) {
        const QMetaMethod meth = mo->method(signalMethodIndex);
        // Since we dynamically connect all relevant change signals through QMetaMethods we also need the slot to be a QMM. Unfortunate since this is
        // a bit fiddly.
        connect(object, meth, this, propertyChangedMetaMethod());
    }

    endInsertRows();
}

QMetaMethod PatientModel::propertyChangedMetaMethod() const
{
    // This function purely exists because we meta-program a connection between a generic signal of the type QMetaMethod.
    // There is no connect() overload for (obj, QMetaMethod, obj, lambda) so we need a QMetaMethod to connect to!

    const auto mo = metaObject();
    // The function serves no purpose other than compile-time asserting the signature.
    // Mind that if the slot signature changes you must reflect this here and ensure that the connect calls stay valid WRT argument compatibility!
    const std::function<void(PatientModel &)> function = &PatientModel::propertyChanged;
    Q_ASSERT(function);
    const int methodIndex = mo->indexOfMethod("propertyChanged()");
    Q_ASSERT(methodIndex != -1);
    return mo->method(methodIndex);
}

void PatientModel::propertyChanged()
{
    // Property index and role index are the same so we only need to map the signal index to the property index.
    const int role = m_signalIndexToProperties.value(senderSignalIndex(), -1);
    Q_ASSERT(role != -1);
    const int index = m_objects.indexOf(sender());
    Q_ASSERT(index != -1);
    qDebug() << "PROPERTY CHANGED (" << index << ") :: " << role << roleNames().value(role);
    Q_EMIT dataChanged(createIndex(index, 0), createIndex(index, 0), {role});
}

int PatientModel::initRoleNames(const QMetaObject &mo)
{
    m_roles[ObjectRole] = QByteArrayLiteral("modelObject");
    m_roles[IndexRole] = QByteArrayLiteral("modelIndex");

    int maxEnumValue = ObjectRole;
    Q_ASSERT(maxEnumValue != -1);
    for (int i = 0; i < mo.propertyCount(); ++i) {
        const QMetaProperty property = mo.property(i);
        m_roles[++maxEnumValue] = QByteArray("ROLE_") + property.name();
        m_objectProperties.insert(maxEnumValue, property.name());
        if (!property.hasNotifySignal()) {
            continue;
        }
        m_signalIndexToProperties.insert(property.notifySignalIndex(), maxEnumValue);
    }
    return maxEnumValue;
}

void PatientModel::addDynamicRoleNames(int maxEnumValue, QObject *object)
{
    const auto dynamicPropertyNames = object->dynamicPropertyNames();
    for (const auto &dynProperty : dynamicPropertyNames) {
        m_roles[++maxEnumValue] = dynProperty;
        m_objectProperties.insert(maxEnumValue, dynProperty);
    }
}

bool PatientModel::ready() const
{
    return m_ready;
}

void PatientModel::setReady(bool ready)
{
    m_ready = ready;
    Q_EMIT readyChanged();
}

#include "moc_PatientModel.cpp"
