// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsettingsgroup.h"

#include <QMap>
#include <QJsonObject>
#include <QJsonArray>

DCORE_BEGIN_NAMESPACE

class DSettingsGroupPrivate
{
public:
    DSettingsGroupPrivate(DSettingsGroup *parent) : q_ptr(parent) {}

    QString key;
    QString name;
    bool    hide = false;

    QMap<QString, OptionPtr>    options;

    QPointer<DSettingsGroup>             parent;
    QMap<QString, OptionPtr>    childOptions;
    QList<QString>              childOptionKeys;

    QMap<QString, GroupPtr>     childGroups;
    QList<QString>              childGroupKeys;

    void parseJson(const QString &prefixKey, const QJsonObject &group);

    DSettingsGroup *q_ptr;
    Q_DECLARE_PUBLIC(DSettingsGroup)
};

/*!
@~english
  @class Dtk::Core::DSettingsGroup
  \inmodule dtkcore
  @brief A group of DSettingsOption and DSettingsGroup.
  DSettingsGroup can contain a lost option and subgroup.
 */


DSettingsGroup::DSettingsGroup(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsGroupPrivate(this))
{

}

DSettingsGroup::~DSettingsGroup()
{

}

/*!
@~english
  @brief Get direct parent group of this group.
  @return
 */
QPointer<DSettingsGroup> DSettingsGroup::parentGroup() const
{
    Q_D(const DSettingsGroup);
    return d->parent;
}

/*!
@~english
  @brief Change the direct \a parentGroup of this group.
 */
void DSettingsGroup::setParentGroup(QPointer<DSettingsGroup> parentGroup)
{
    Q_D(DSettingsGroup);
    d->parent = parentGroup;
}

/*!
@~english
  @brief Return the full key of this group, include all parent.
  @return eturn the full key of this group, include all parent.
 */
QString DSettingsGroup::key() const
{
    Q_D(const DSettingsGroup);
    return d->key;
}

/*!
@~english
  @brief Get display name of this group, it may be translated.
  @return Get display name of this group.
 */
QString DSettingsGroup::name() const
{
    Q_D(const DSettingsGroup);
    return d->name;
}

/*!
@~english
  @brief Check this group will show on DSettings dialog.
  @return true indicates that this option group will be displayed。
 */
bool DSettingsGroup::isHidden() const
{
    Q_D(const DSettingsGroup);
    return d->hide;
}

/*!
@~english
  @brief Get the child group of groupKey
  \a groupKey is child group key

  @return Returns a pointer to a subgroup.
 */
QPointer<DSettingsGroup> DSettingsGroup::childGroup(const QString &groupKey) const
{
    Q_D(const DSettingsGroup);
    return d->childGroups.value(groupKey);
}

/*!
@~english
  @brief Get the child option of key
  \a key is child option key

  @return Returns a pointer to the child option of key.
 */
QPointer<DSettingsOption> DSettingsGroup::option(const QString &key) const
{
    Q_D(const DSettingsGroup);
    return d->childOptions.value(key);
}

/*!
@~english
  @brief Enum all direct child group of this group

  @return Returns a list of all subgroup Pointers.
 */
QList<QPointer<DSettingsGroup> > DSettingsGroup::childGroups() const
{
    Q_D(const DSettingsGroup);
    QList<QPointer<DSettingsGroup> > grouplist;
    for (auto groupKey : d->childGroupKeys) {
        grouplist << d->childGroups.value(groupKey);
    }
    return grouplist;
}

/*!
@~english
  @brief Enum all direct child option with the raw order in json description file.
  @return Returns a list of all suboption Pointers.
 */
QList<QPointer<DSettingsOption> > DSettingsGroup::childOptions() const
{
    Q_D(const DSettingsGroup);
    QList<QPointer<DSettingsOption> > optionlist;
    for (auto optionKey : d->childOptionKeys) {
        optionlist << d->childOptions.value(optionKey);
    }
    return optionlist;
}

/*!
@~english
  @brief Enum all direct child option of this group.
  @return Returns a list of all option Pointers.
 */
QList<QPointer<DSettingsOption> > DSettingsGroup::options() const
{
    Q_D(const DSettingsGroup);
    return d->options.values();
}

/*!
@~english
  @brief Convert QJsonObject to DSettingsGroup.
  \a prefixKey instead parse prefix key from parent.
  \a group is an QJsonObejct instance.
  @return Returns a group pointer after parsing json.

  @sa QPointer Dtk::Core::DSettingsOption
 */
QPointer<DSettingsGroup> DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &group)
{
    auto groupPtr = QPointer<DSettingsGroup>(new DSettingsGroup);
    groupPtr->parseJson(prefixKey, group);
    return groupPtr;
}

/*!
@~english
  @brief Parse QJsonObject to DSettingsGroup.
  \a prefixKey instead parse prefix key from parent.
  \a json is an QJsonObejct instance.
  @sa QPointer<DSettingsOption> Dtk::Core::DSettingsGroup::fromJson(const QString &prefixKey, const QJsonObject &json)
 */
void DSettingsGroup::parseJson(const QString &prefixKey, const QJsonObject &group)
{
    Q_D(DSettingsGroup);
    d->parseJson(prefixKey, group);
}

void DSettingsGroupPrivate::parseJson(const QString &prefixKey, const QJsonObject &group)
{
    Q_Q(DSettingsGroup);
    key = group.value("key").toString();
    Q_ASSERT(!key.isEmpty());
    key = prefixKey.isEmpty() ? key : prefixKey + "." + key;
    name = group.value("name").toString();
    hide = group.value("hide").toBool();

    for (auto optionJson :  group.value("options").toArray()) {
        auto optionObject = optionJson.toObject();
        auto option = DSettingsOption::fromJson(key, optionObject);
        option->setParent(q);
        options.insert(option->key(), option);
        childOptions.insert(option->key(), option);
        childOptionKeys << option->key();
    }

    auto subGroups = group.value("groups").toArray();
    for (auto subGroup : subGroups) {
        auto child = DSettingsGroup::fromJson(key, subGroup.toObject());
        child->setParent(q);
        child->setParentGroup(q);
        childGroups.insert(child->key(), child);
        childGroupKeys << child->key();

        for (auto option : child->options()) {
            options.insert(option->key(), option);
        }
    }

}

DCORE_END_NAMESPACE
