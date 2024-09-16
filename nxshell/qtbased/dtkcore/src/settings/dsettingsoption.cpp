// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsettingsoption.h"

#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>

DCORE_BEGIN_NAMESPACE

class DSettingsOptionPrivate
{
public:
    DSettingsOptionPrivate(DSettingsOption *parent) : q_ptr(parent) {}

    void parseJson(const QString &prefixKey, const QJsonObject &option);

    QPointer<DSettingsGroup>             parent;

    QString     key;
    QString     name;
    QString     viewType;
    QVariant    defalutValue;
    QVariant    value;
    QVariantMap datas;
    bool        canReset;
    bool        hidden;

    DSettingsOption *q_ptr;
    Q_DECLARE_PUBLIC(DSettingsOption)
};

/*!
@~english
  @class Dtk::Core::DSettingsOption
  \inmodule dtkcore
  @brief DSettingsOption is the base key/value item of DSettings.
 */

/*!
@~english
  @fn void DSettingsOption::valueChanged(QVariant value);
  @brief Emit when option value change.

  \a value Changed data.
 */

/*!
@~english
  @fn void DSettingsOption::dataChanged(const QString &dataType, QVariant value);
  @brief Emit when option data change.

  \a dataType Changed data type, \a value Changed data.
 */

/*!
@~english
  @property Dtk::Core::DSettingsOption::value
  @brief Current value of this option.
 */

DSettingsOption::DSettingsOption(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsOptionPrivate(this))
{
}

DSettingsOption::~DSettingsOption()
{

}

/*!
@~english
  @brief Get direct parent group of this option.
  @return Returns the direct parent group of the this option.
 */
QPointer<DSettingsGroup> DSettingsOption::parentGroup() const
{
    Q_D(const DSettingsOption);
    return d->parent;
}

/*!
@~english
  @brief Change the direct parent group of this option.

  \a parentGroup parent group.
 */
void DSettingsOption::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsOption);
    d->parent = parentGroup;
}

/*!
@~english
  @brief Return the full key of this option, include all parent.
  @return Return the full key of this option, include all parent.
 */
QString DSettingsOption::key() const
{
    Q_D(const DSettingsOption);
    return d->key;
}

/*!
@~english
  @brief Get display name of the option, it may be translated.
  @return Return the name of the option.
 */
QString DSettingsOption::name() const
{
    Q_D(const DSettingsOption);
    return d->name;
}

/*!
@~english
  @brief Check this option can be reset to default value. if false, reset action will not take effect.

  @return true if can be reset.
 */
bool DSettingsOption::canReset() const
{
    Q_D(const DSettingsOption);
    return d->canReset;
}

/*!
@~english
  @brief Default value of this option, must config in this json desciption file.

  @return Return the default value of this option.
 */
QVariant DSettingsOption::defaultValue() const
{
    Q_D(const DSettingsOption);
    return d->defalutValue;
}

/*!
@~english
  @brief Get current value of option.

  @return Return the current value of this option.
 */
QVariant DSettingsOption::value() const
{
    Q_D(const DSettingsOption);
    return (!d->value.isValid() || d->value.isNull()) ? d->defalutValue : d->value;
}

/*!
@~english
  @brief Custom data of option, like QObject::property.
  \a dataType Data type.

  @return Data type Indicates the data.
  @sa QObject::property
  @sa Dtk::Core::DSettingsOption::setData
 */
QVariant DSettingsOption::data(const QString &dataType) const
{
    Q_D(const DSettingsOption);
    return d->datas.value(dataType);
}

/*!
@~english
  @brief UI widget type of this option.

  @return Returns the UI widget type of the option.
  @sa Dtk::Widget::DSettingsWidgetFactory
 */
QString DSettingsOption::viewType() const
{
    Q_D(const DSettingsOption);
    return d->viewType;
}

/*!
@~english
  @brief Check this option will show on DSettings dialog.

  @return true if option not bind to ui element.
 */
bool DSettingsOption::isHidden() const
{
    Q_D(const DSettingsOption);
    return d->hidden;
}

/*!
@~english
  @brief Convert QJsonObject to DSettingsOption.

  \a prefixKey instead parse prefix key from parent.
  \a json is an QJsonObejct instance.

  @return Return the option data after parsing.
 */
QPointer<DSettingsOption> DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &json)
{
    auto optionPtr = QPointer<DSettingsOption>(new DSettingsOption);
    optionPtr->parseJson(prefixKey, json);
    return optionPtr;
}

/*!
@~english
  @brief Set current value of option.

  \a value Current value of option.
 */
void DSettingsOption::setValue(QVariant value)
{
    Q_D(DSettingsOption);

    // 默认没有设置value时比较默认值。防止reset时出现所有的option都发射valueChanged
    if (this->value() == value) {
        return;
    }

    d->value = value;
    Q_EMIT valueChanged(value);
}

///*!
// * @brief Override default value of json
// * \a value
// */
//void DSettingsOption::setDefault(QVariant value)
//{
//    Q_D(DSettingsOption);
//    d->defalutValue = value;
//}

/*!
@~english
  @brief Set custom data.
  \a dataType is data id, just a unique string.
  \a value of the data id.
  @sa Dtk::Core::DSettingsOption::data
 */
void DSettingsOption::setData(const QString &dataType, QVariant value)
{
    Q_D(DSettingsOption);

    if (d->datas.value(dataType) == value) {
        return;
    }

    d->datas.insert(dataType, value);

    Q_EMIT dataChanged(dataType, value);
}

/*!
@~english
  @brief Parse QJsonObject to DSettingsOption.
  \a prefixKey instead parse prefix key from parent.
  \a option is an QJsonObejct instance.
  @sa QPointer<DSettingsOption> Dtk::Core::DSettingsOption::fromJson(const QString &prefixKey, const QJsonObject &json)
 */
void DSettingsOption::parseJson(const QString &prefixKey, const QJsonObject &option)
{
    Q_D(DSettingsOption);
    d->parseJson(prefixKey, option);
}

void DSettingsOptionPrivate::parseJson(const QString &prefixKey, const QJsonObject &option)
{
//    Q_Q(Option);
    key = option.value("key").toString();
    Q_ASSERT(!key.isEmpty());
    Q_ASSERT(!prefixKey.isEmpty());
    key = prefixKey + "." + key;
    name = option.value("name").toString();

    canReset = !option.contains("reset") ? true : option.value("reset").toBool();
    defalutValue = option.value("default").toVariant();
    hidden = !option.contains("hide") ? false : option.value("hide").toBool();
    viewType = option.value("type").toString();

    QStringList revserdKeys;
    revserdKeys << "key" << "name" << "reset"
                << "default" << "hide" << "type";

    auto allKeys = option.keys();
    for (auto key : revserdKeys) {
        allKeys.removeAll(key);
    }

    for (auto key : allKeys) {
        auto value = option.value(key);
        if (value.isArray()) {
            QStringList stringlist;
            for (auto va : value.toArray()) {
                stringlist << QString("%1").arg(va.toString());
            }
            datas.insert(key, stringlist);
        } else {
            datas.insert(key, value.toVariant());
        }
    }
}

DCORE_END_NAMESPACE


