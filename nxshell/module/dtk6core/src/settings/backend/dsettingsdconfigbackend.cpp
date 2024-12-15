// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settings/backend/dsettingsdconfigbackend.h"

#include <QDebug>
#include <QMutex>
#include <DConfig>

DCORE_BEGIN_NAMESPACE

class DSettingsDConfigBackendPrivate
{
public:
    explicit DSettingsDConfigBackendPrivate(DSettingsDConfigBackend *parent) : q_ptr(parent) {}

    DConfig       *dConfig   = nullptr;
    QMutex          writeLock;

    DSettingsDConfigBackend *q_ptr;
    Q_DECLARE_PUBLIC(DSettingsDConfigBackend)
};

/*!
@~english
  @class Dtk::Core::DSettingsDConfigBackend
  \inmodule dtkcore
  @brief Storage DSetttings to an DConfig.
 */

/*!
@~english
  @brief Save data to configure file name with DConfig.
  \a name configure file name.
  \a subpath subdirectory of configure file name.
  \a parent
 */
DSettingsDConfigBackend::DSettingsDConfigBackend(const QString &name, const QString &subpath, QObject *parent) :
    DSettingsBackend(parent), d_ptr(new DSettingsDConfigBackendPrivate(this))
{
    Q_D(DSettingsDConfigBackend);

    d->dConfig = new DConfig(name, subpath, this);
}

DSettingsDConfigBackend::~DSettingsDConfigBackend()
{

}

/*!
@~english
  @brief List all keys of DConfig
  @return
 */
QStringList DSettingsDConfigBackend::keys() const
{
    Q_D(const DSettingsDConfigBackend);
    return d->dConfig->keyList();
}

/*!
@~english
  @brief Get value of key from DConfig
  \a key
  @return
 */
QVariant DSettingsDConfigBackend::getOption(const QString &key) const
{
    Q_D(const DSettingsDConfigBackend);
    return d->dConfig->value(key);
}

/*!
@~english
  @brief Set value of key to DConfig
  \a key
  \a value
 */
void DSettingsDConfigBackend::doSetOption(const QString &key, const QVariant &value)
{
    Q_D(DSettingsDConfigBackend);
    d->writeLock.lock();
    d->dConfig->setValue(key, value);
    d->writeLock.unlock();
}

/*!
@~english
  @brief Trigger DSettings to save option value to DConfig
 */
void DSettingsDConfigBackend::doSync()
{
    Q_D(DSettingsDConfigBackend);

    // TODO
}


DCORE_END_NAMESPACE
