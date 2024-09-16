// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settings/backend/gsettingsbackend.h"

//#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#if QT_HAS_INCLUDE(<QGSettings/QGSettings>)
#include <QGSettings/QGSettings>
#else
#include <QGSettings>
#endif

#include <DSettings>

DCORE_BEGIN_NAMESPACE

QString unqtifyName(const QString &name)
{
    QString ret;
    for (auto p : name) {
        const QChar c(p);
        if (c.isUpper()) {
            ret.append("-");
            ret.append(c.toLower().toLatin1());
        } else {
            ret.append(c);
        }
    }
    return ret;
}

QString qtifyName(const QString &key)
{
    return QString(key).replace(".", "-").replace("_", "-");
}


class GSettingsBackendPrivate
{
public:
    GSettingsBackendPrivate(GSettingsBackend *parent) : q_ptr(parent) {}

    QGSettings *gsettings;
    QMap<QString, QString> keyMap;

    GSettingsBackend *q_ptr;
    Q_DECLARE_PUBLIC(GSettingsBackend)
};

/*!
@~english
  @class Dtk::Core::GSettingsBackend
  \inmodule dtkcore
  @brief Storage backend of DSettings use gsettings.
  
  You should generate gsetting schema with /usr/lib/x86_64-linux-gnu/libdtk-$$VERSION/DCore/bin/dtk-settings.
  
  You can find this tool from libdtkcore-bin. use /usr/lib/x86_64-linux-gnu/libdtk-$$VERSION/DCore/bin/dtk-settings -h for help.
 */

GSettingsBackend::GSettingsBackend(DSettings *settings, QObject *parent) :
    DSettingsBackend(parent), d_ptr(new GSettingsBackendPrivate(this))
{
    Q_D(GSettingsBackend);

    QJsonObject settingsMeta = settings->meta();
    auto gsettingsMeta = settingsMeta.value("gsettings").toObject();
    auto id = gsettingsMeta.value("id").toString();
    auto path = gsettingsMeta.value("path").toString();

    for (QString key : settings->keys()) {
        auto gsettingsKey = QString(key).replace(".", "-").replace("_", "-");
        d->keyMap.insert(gsettingsKey, key);
    }

    d->gsettings = new QGSettings(id.toUtf8(), path.toUtf8(), this);

    connect(d->gsettings, &QGSettings::changed, this, [ = ](const QString & key) {
        auto dk = d->keyMap.value(unqtifyName(key));
//        qDebug() << "gsetting change" << key << d->gsettings->get(key);
        Q_EMIT optionChanged(dk, d->gsettings->get(key));
    });

}

GSettingsBackend::~GSettingsBackend()
{

}

/*!
@~english
  @brief List all gsettings keys.
  @return Return all gsettings keys.
 */
QStringList GSettingsBackend::keys() const
{
    Q_D(const GSettingsBackend);
    return d->gsettings->keys();
}

/*!
@~english
  @brief Get value of key.
  @return Return the value of the given \a key.
 */
QVariant GSettingsBackend::getOption(const QString &key) const
{
    Q_D(const GSettingsBackend);
    return d->gsettings->get(qtifyName(key));
}

/*!
@~english
  @brief Set value to gsettings
  Use the \a key to save the \a value.
 */
void GSettingsBackend::doSetOption(const QString &key, const QVariant &value)
{
    Q_D(GSettingsBackend);
    if (value != d->gsettings->get(qtifyName(key))) {
//        qDebug() << "doSetOption" << key << d->gsettings->get(qtifyName(key));
        d->gsettings->set(qtifyName(key), value);
    }
}

/*!
@~english
  @brief Trigger DSettings to sync option to storage.
 */
void GSettingsBackend::doSync()
{
    Q_EMIT sync();
}

DCORE_END_NAMESPACE
