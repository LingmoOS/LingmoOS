// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsettings.h"

#include <QMap>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>
#include <QDebug>

#include "dsettingsoption.h"
#include "dsettingsgroup.h"
#include "dsettingsbackend.h"

DCORE_BEGIN_NAMESPACE

class DSettingsPrivate
{
public:
    DSettingsPrivate(DSettings *parent) : q_ptr(parent) {}

    DSettingsBackend            *backend = nullptr;
    QJsonObject                 meta;
    QMap <QString, OptionPtr>   options;

    QMap<QString, GroupPtr>     childGroups;
    QList<QString>              childGroupKeys;

    DSettings *q_ptr;
    Q_DECLARE_PUBLIC(DSettings)
};


/*!
@~english
   @class Dtk::Core::DSettingsBackend
   \inmodule dtkcore
   @brief DSettingsBackend is interface of DSettings storage class.

   Simaple example:

@code
{
    "groups": [{
        "key": "base",
        "name": "Basic settings",
        "groups": [{
                "key": "open_action",
                "name": "Open Action",
                "options": [{
                        "key": "alway_open_on_new",
                        "type": "checkbox",
                        "text": "Always Open On New Windows",
                        "default": true
                    },
                    {
                        "key": "open_file_action",
                        "name": "Open File:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            },
            {
                "key": "new_tab_windows",
                "name": "New Tab & Window",
                "options": [{
                        "key": "new_window_path",
                        "name": "New Window Open:",
                        "type": "combobox",
                        "default": ""
                    },
                    {
                        "key": "new_tab_path",
                        "name": "New Tab Open:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            }
        ]
    }]
}
@endcode

    How to read/write key and value:

    @code
    // init a storage backend
    QTemporaryFile tmpFile;
    tmpFile.open();
    auto backend = new Dtk::Core::QSettingBackend(tmpFile.fileName());

    // read settings from json
    auto settings = Dtk::Core::DSettings::fromJsonFile(":/resources/data/dfm-settings.json");
    settings->setBackend(backend);

    // read value
    auto opt = settings->option("base.new_tab_windows.new_window_path");
    qDebug() << opt->value();

    // modify value
    opt->setValue("Test")
    qDebug() << opt->value();
   @endcode
   @sa Dtk::Core::DSettingsOption
   @sa Dtk::Core::DSettingsGroup
   @sa Dtk::Core::DSettingsBackend
   @sa Dtk::Widget::DSettingsWidgetFactory
   @sa Dtk::Widget::DSettingsDialog
 */

/*!
@~english
  @fn virtual QStringList DSettingsBackend::keys() const = 0;
  @brief return all key of storage.
 */
/*!
@~english
  @fn virtual QVariant DSettingsBackend::getOption(const QString &key) const = 0;
  @brief get value by \a key.
 */
/*!
@~english
  @fn virtual void DSettingsBackend::doSync() = 0;
  @brief do the real sync action.
 */
/*!
@~english
  @fn virtual void DSettingsBackend::doSetOption(const QString &key, const QVariant &value) = 0;
  @brief write \a key / \a value to storage.
 */
/*!
@~english
  @fn void DSettingsBackend::optionChanged(const QString &key, const QVariant &value);
  @brief emitted when option \a value changed.
 */
/*!
@~english
  @fn void DSettingsBackend::sync();
  @brief private signal, please do not use it.
 */
/*!
@~english
  @fn void DSettingsBackend::setOption(const QString &key, const QVariant &value);
  @brief private signal, please do not use it.

  \internal
  \a key \a value
 */

/*!
@~english
 @class Dtk::Core::DSettings
 \inmodule dtkcore
 @brief DSettings is the basic library that provides unified configuration storage and interface generation tools for Dtk applications.

 DSetting uses json as the description file for the application configuration. The configuration of application query is divided into two basic levels: group / key value.
 The standard Dtk configuration control generally contains only three levels of group / subgroup / key values. For keys with more than three levels, you can use the.
 DSettings's API interface reads and writes, but cannot be displayed on standard DSettingsDialogs.

 
 Simple configuration file：
@code
{
    "groups": [{
        "key": "base",
        "name": "Basic settings",
        "groups": [{
                "key": "open_action",
                "name": "Open Action",
                "options": [{
                        "key": "alway_open_on_new",
                        "type": "checkbox",
                        "text": "Always Open On New Windows",
                        "default": true
                    },
                    {
                        "key": "open_file_action",
                        "name": "Open File:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            },
            {
                "key": "new_tab_windows",
                "name": "New Tab & Window",
                "options": [{
                        "key": "new_window_path",
                        "name": "New Window Open:",
                        "type": "combobox",
                        "default": ""
                    },
                    {
                        "key": "new_tab_path",
                        "name": "New Tab Open:",
                        "type": "combobox",
                        "default": ""
                    }
                ]
            }
        ]
    }]
}
@endcode

The group contains a root group of base with two subgroups: open_action/new_tab_windows, each of which contains several options.
The complete access id for the configuration "New Window Open:" is base.new_tab_windows.new_window_path.

How to read/write key and value:
@code
    // init a storage backend
    QTemporaryFile tmpFile;
    tmpFile.open();
    auto backend = new Dtk::Core::QSettingBackend(tmpFile.fileName());

    // read settings from json
    auto settings = Dtk::Core::DSettings::fromJsonFile(":/resources/data/dfm-settings.json");
    settings->setBackend(backend);

    // read value
    auto opt = settings->option("base.new_tab_windows.new_window_path");
    qDebug() << opt->value();

    // modify value
    opt->setValue("Test")
    qDebug() << opt->value();
@endcode
@sa Dtk::Core::DSettingsOption
@sa Dtk::Core::DSettingsGroup
@sa Dtk::Core::DSettingsBackend
@sa Dtk::Widget::DSettingsWidgetFactory
@sa Dtk::Widget::DSettingsDialog
*/

DSettings::DSettings(QObject *parent) :
    QObject(parent), dd_ptr(new DSettingsPrivate(this))
{
}

DSettings::~DSettings()
{

}

void DSettings::setBackend(DSettingsBackend *backend)
{
    Q_D(DSettings);
    if (nullptr == backend) {
        return;
    }

    if (d->backend != nullptr) {
        qWarning() << "set backend to exist " << d->backend;
    }

    d->backend = backend;


    auto backendWriteThread = new QThread;
    d->backend->moveToThread(backendWriteThread);

    connect(d->backend, &DSettingsBackend::optionChanged,
    this, [ = ](const QString & key, const QVariant & value) {
        option(key)->setValue(value);
    });
    // exit and delete thread
    connect(this, &DSettings::destroyed, this, [backendWriteThread](){
        if (backendWriteThread->isRunning()) {
            backendWriteThread->quit();
            backendWriteThread->wait();
        }
        backendWriteThread->deleteLater();
    });

    backendWriteThread->start();

    // load form backend
    loadValue();
}

/*!
@~english
   @brief Get DSettings from \a json. The returned data needs to be manually released after use.
 */
QPointer<DSettings> DSettings::fromJson(const QByteArray &json)
{
    auto settingsPtr = QPointer<DSettings>(new DSettings);
    settingsPtr->parseJson(json);
    return settingsPtr;
}

QPointer<DSettings> DSettings::fromJsonFile(const QString &filepath)
{
    QFile jsonFile(filepath);
    jsonFile.open(QIODevice::ReadOnly);
    auto jsonData = jsonFile.readAll();
    jsonFile.close();

    return DSettings::fromJson(jsonData);
}

QJsonObject DSettings::meta() const
{
    Q_D(const DSettings);
    return d->meta;
}

QStringList DSettings::keys() const
{
    Q_D(const DSettings);
    return d->options.keys();
}

QPointer<DSettingsOption> DSettings::option(const QString &key) const
{
    Q_D(const DSettings);
    return d->options.value(key);
}

QVariant DSettings::value(const QString &key) const
{
    Q_D(const DSettings);
    auto opt = d->options.value(key);
    if (opt.isNull()) {
        return QVariant();
    }

    return opt->value();
}

QStringList DSettings::groupKeys() const
{
    Q_D(const DSettings);
    return d->childGroupKeys;
}

QList<QPointer<DSettingsGroup> > DSettings::groups() const
{
    Q_D(const DSettings);
    return d->childGroups.values();
}
/*!
@~english
  @brief DSettings::group will recurrence find childGroup
  \a key
  @return
 */
QPointer<DSettingsGroup> DSettings::group(const QString &key) const
{
    Q_D(const DSettings);
    auto childKeylist = key.split(".");
    if (0 >= childKeylist.length()) {
        return nullptr;
    }

    auto mainGroupKey = childKeylist.value(0);
    if (1 >= childKeylist.length()) {
        return d->childGroups.value(mainGroupKey);
    }

    return d->childGroups.value(mainGroupKey)->childGroup(key);
}

QList<QPointer<DSettingsOption> > DSettings::options() const
{
    Q_D(const DSettings);
    return d->options.values();
}

QVariant DSettings::getOption(const QString &key) const
{
    QPointer<DSettingsOption> optionPointer = option(key);
    if (optionPointer) {
        return optionPointer->value();
    }
    return QVariant();
}

void DSettings::setOption(const QString &key, const QVariant &value)
{
    option(key)->setValue(value);
}

void DSettings::sync()
{
    Q_D(DSettings);
    if (!d->backend) {
        qWarning() << "backend was not setted..!";
        return;
    }

    d->backend->doSync();
}

void DSettings::reset()
{
    Q_D(DSettings);

    for (auto option : d->options) {
        if (option->canReset()) {
            setOption(option->key(), option->defaultValue());
        }
    }

    if (!d->backend) {
        qWarning() << "backend was not setted..!";
        return;
    }

    d->backend->sync();
}

void DSettings::parseJson(const QByteArray &json)
{
    Q_D(DSettings);

    auto jsonDoc = QJsonDocument::fromJson(json);
    d->meta = jsonDoc.object();
    auto mainGroups = d->meta.value("groups");
    for (auto groupJson : mainGroups.toArray()) {
        auto group = DSettingsGroup::fromJson("", groupJson.toObject());
        group->setParent(this);
        for (auto option : group->options()) {
            d->options.insert(option->key(), option);
        }
        d->childGroupKeys << group->key();
        d->childGroups.insert(group->key(), group);
    }

    for (auto option :  d->options.values()) {
        d->options.insert(option->key(), option);
        connect(option.data(), &DSettingsOption::valueChanged,
        this, [ = ](QVariant value) {
            if (d->backend) {
                Q_EMIT d->backend->setOption(option->key(), value);
            } else {
                qWarning() << "backend was not setted..!";
            }
            Q_EMIT valueChanged(option->key(), value);
        });
    }
}

void DSettings::loadValue()
{
    Q_D(DSettings);
    if (!d->backend) {
        qWarning() << "backend was not setted..!";
        return;
    }

    for (auto key : d->backend->keys()) {
        auto value = d->backend->getOption(key);
        auto opt = option(key);
        if (!value.isValid() || opt.isNull()) {
            continue;
        }

        opt->blockSignals(true);
        opt->setValue(value);
        opt->blockSignals(false);
    }
}

DCORE_END_NAMESPACE
