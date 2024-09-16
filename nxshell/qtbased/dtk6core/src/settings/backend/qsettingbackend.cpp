// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settings/backend/qsettingbackend.h"

#include <QDebug>
#include <QMutex>
#include <QSettings>

DCORE_BEGIN_NAMESPACE

class QSettingBackendPrivate
{
public:
    QSettingBackendPrivate(QSettingBackend *parent) : q_ptr(parent) {}

    QSettings       *settings   = nullptr;
    QMutex          writeLock;

    QSettingBackend *q_ptr;
    Q_DECLARE_PUBLIC(QSettingBackend)
};

/*!
@~english
  @class Dtk::Core::QSettingBackend
  \inmodule dtkcore
  @brief Storage DSetttings to an QSettings.
 */

/*!
@~english
  @brief Save data to filepath with QSettings::NativeFormat format.
  \a filepath is path to storage data.
  \a parent
 */
QSettingBackend::QSettingBackend(const QString &filepath, QObject *parent) :
    DSettingsBackend(parent), d_ptr(new QSettingBackendPrivate(this))
{
    Q_D(QSettingBackend);

    d->settings = new QSettings(filepath, QSettings::NativeFormat, this);
    qDebug() << "create config" <<  d->settings->fileName();
}

QSettingBackend::~QSettingBackend()
{

}

/*!
@~english
  @brief List all keys of QSettings
  @return
 */
QStringList QSettingBackend::keys() const
{
    Q_D(const QSettingBackend);
    return d->settings->childGroups();
}

/*!
@~english
  @brief Get value of key from QSettings
  \a key
  @return
 */
QVariant QSettingBackend::getOption(const QString &key) const
{
    Q_D(const QSettingBackend);
    d->settings->beginGroup(key);
    auto value = d->settings->value("value");
    d->settings->endGroup();
    return value;
}

/*!
@~english
  @brief Set value of key to QSettings
  \a key
  \a value
 */
void QSettingBackend::doSetOption(const QString &key, const QVariant &value)
{
    Q_D(QSettingBackend);
    d->writeLock.lock();
    d->settings->beginGroup(key);
    auto oldValue = d->settings->value("value");
    if (oldValue != value) {
        d->settings->setValue("value", value);
    }
    d->settings->endGroup();
    d->settings->sync();
    d->writeLock.unlock();
}

/*!
@~english
  @brief Trigger DSettings to save option value to QSettings
 */
void QSettingBackend::doSync()
{
    Q_D(QSettingBackend);
    d->settings->sync();
}


DCORE_END_NAMESPACE
