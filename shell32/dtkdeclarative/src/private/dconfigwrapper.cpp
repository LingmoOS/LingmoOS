// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfigwrapper_p.h"

#include <QDebug>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QQmlEngine>
#include <QTimer>
#include <private/qqmlopenmetaobject_p.h>

#include <DConfig>

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(cfLog, "dtk.dsg.config" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(cfLog, "dtk.dsg.config");
#endif

DCORE_USE_NAMESPACE;

// the properties and previous values.
using DefalutProperties = QMap<QByteArray, QVariant>;
static DefalutProperties propertyAndValues(const QObject* obj)
{
    QMap<QByteArray, QVariant> properties;
    const QMetaObject *mo = obj->metaObject();
    const int offset = mo->propertyOffset();
    const int count = mo->propertyCount();
    static const QStringList ReservedPropertyNames {
        "name",
        "subpath"
    };

    for (int i = offset; i < count; ++i) {
        const QMetaProperty &property = mo->property(i);
        if (ReservedPropertyNames.contains(property.name())) {
            qCWarning(cfLog()) << property.name() << " is keyword for Config." << property.isUser();
            continue;
        }

        const QVariant &previousValue = property.read(obj);
        properties[property.name()] = previousValue;
    }
    return properties;
}

class DConfigWrapperMetaObject : public QQmlOpenMetaObject {
public:
    DConfigWrapperMetaObject(DConfigWrapper *o, QQmlOpenMetaObjectType *type)
        : QQmlOpenMetaObject(o, type)
        , owner(o)
    {
    }
    ~DConfigWrapperMetaObject() override;

    DConfigWrapper *owner;
    // QQmlOpenMetaObject interface
protected:
    virtual QVariant propertyWriteValue(int index, const QVariant &value) override
    {
        const QByteArray &proName = name(index);
        qCDebug(cfLog) << "propertyWriteValue" << proName << value;
        owner->impl->setValue(proName, value);
        // Pre judgment returns the set value first.
        // If the value is different, `valueChanged` will be triggered again to update the value,
        // there are problems when the service is unavailable.
        // But If `impl->value(proName)` is returned, blocking has a performance problem.
        return value;
    }
    int metaCall(QObject *o, QMetaObject::Call _c, int _id, void **_a) override
    {
        if (_c == QMetaObject::ResetProperty) {
            owner->impl->reset(name(_id - type()->propertyOffset()));
        }

        return QQmlOpenMetaObject::metaCall(o, _c, _id, _a);
    }
};

DConfigWrapperMetaObject::~DConfigWrapperMetaObject()
{
}

/*!
    \class DTK::Quick::DConfigWrapper
    \inmodule dtkdeclarative

    \brief Import some function about reading and writing DSG configuration.
*/
DConfigWrapper::DConfigWrapper(QObject *parent)
    : QObject(parent)
    , impl(nullptr)
{
}

DConfigWrapper::~DConfigWrapper()
{
}

/*!
 * \brief \sa DConfig name()
 * \return
 */
QString DConfigWrapper::name() const
{
    return m_name;
}

void DConfigWrapper::setName(const QString &name)
{
    if (!m_name.isEmpty()) {
        qWarning() << "name is existed." << m_name;
        return;
    }

    m_name = name;
}

/*!
 * \brief \sa DConfig subpath()
 * \return
 */
QString DConfigWrapper::subpath() const
{
    return m_subpath;
}

void DConfigWrapper::setSubpath(const QString &subpath)
{
    if (!m_subpath.isEmpty()) {
        qWarning() << "subpath is existed." << m_subpath;
        return;
    }

    m_subpath = subpath;
}

/*!
 * \brief \sa DConfig keyList()
 * \return
 */
QStringList DConfigWrapper::keyList() const
{
    if (!impl)
        return QStringList();

    return impl->keyList();
}

/*!
 * \brief \sa DConfig isValid()
 * \return
 */
bool DConfigWrapper::isValid() const
{
    if (!impl)
        return false;

    return impl->isValid();
}

/*!
 * \brief \sa DConfig value()
 * \return
 */
QVariant DConfigWrapper::value(const QString &key, const QVariant &fallback) const
{
    if (!impl)
        return fallback;

    return impl->value(key, fallback);
}

/*!
 * \brief \sa DConfig setValue()
 * \return
 */
void DConfigWrapper::setValue(const QString &key, const QVariant &value)
{
    if (!impl)
        return;

    impl->setValue(key, value);
}

void DConfigWrapper::resetValue(const QString &key)
{
    if (!impl)
        return;

    impl->reset(key);
}

void DConfigWrapper::classBegin()
{

}

/*!
    \brief Initialize `DConfig` and redirect method of property's get and set.
    `DConfig` can only be initialized after \property name and \property subpath initialization
    is completed.
    \return
 */
void DConfigWrapper::componentComplete()
{
    impl = new DTK_CORE_NAMESPACE::DConfig(m_name, m_subpath, this);

    if (!impl->isValid()) {
        qCWarning(cfLog) << QString("create dconfig failed, valid:%1, name:%2, subpath:%3, backend:%4")
                      .arg(impl->isValid())
                      .arg(impl->name())
                      .arg(impl->subpath())
                      .arg(impl->backendName());
        impl->deleteLater();
        impl = nullptr;
        return;
    }

    qInfo() << QString("create dconfig successful, valid:%1, name:%2, subpath:%3, backend:%4")
               .arg(impl->isValid())
               .arg(impl->name())
               .arg(impl->subpath())
               .arg(impl->backendName());

    // Get the dynamic properties and previous values defined in qml.
    const DefalutProperties &properties = propertyAndValues(this);
    qCDebug(cfLog) << "properties" << properties;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto objectType = new QQmlOpenMetaObjectType(&DConfigWrapper::staticMetaObject, qmlEngine(this));
#else
    auto objectType = new QQmlOpenMetaObjectType(&DConfigWrapper::staticMetaObject);
#endif
    auto mo = new DConfigWrapperMetaObject(this, objectType);
    mo->setCached(true);

    for (auto iter = properties.begin(); iter != properties.end(); iter++) {
        // it's need to emit signal, because other qml object maybe read the old value
        // when binding the property before the component completed, also it has a performance problem.
        // sync backend's value to `Wrapper`, we only use Wrapper's value(defined in qml) as fallback value.
        mo->setValue(iter.key(), impl->value(iter.key(), iter.value()));
    }

     // Using QueuedConnection because impl->setValue maybe emit sync signal in `propertyWriteValue`.
    connect(impl, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this, mo, properties](const QString &key){
        const QByteArray &proName = key.toLocal8Bit();
        if (properties.contains(proName)) {
            qCDebug(cfLog) << "update value from DConfig by 'valueChanged', key:" << proName;
            mo->setValue(proName, impl->value(proName, properties.value(proName)));
        }
        Q_EMIT valueChanged(key);
    }, Qt::QueuedConnection);
}
