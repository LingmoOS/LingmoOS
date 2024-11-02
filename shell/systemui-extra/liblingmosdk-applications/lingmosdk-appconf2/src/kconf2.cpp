#include "kconf2.h"
#include <QDebug>
#include <string>
#include <iostream>

namespace kdk {

enum KCONF2TYPE {
    KCONF2_TYPE_BOOLEAN       = 'b',
    KCONF2_TYPE_BYTE          = 'y',
    KCONF2_TYPE_INT16         = 'n',
    KCONF2_TYPE_UINT16        = 'q',
    KCONF2_TYPE_INT32         = 'i',
    KCONF2_TYPE_UINT32        = 'u',
    KCONF2_TYPE_INT64         = 'x',
    KCONF2_TYPE_UINT64        = 't',
    KCONF2_TYPE_HANDLE        = 'h',
    KCONF2_TYPE_DOUBLE        = 'd',
    KCONF2_TYPE_STRING        = 's',
    KCONF2_TYPE_OBJECT_PATH   = 'o',
    KCONF2_TYPE_SIGNATURE     = 'g',
    KCONF2_TYPE_VARIANT       = 'v',
    KCONF2_TYPE_MAYBE         = 'm',
    KCONF2_TYPE_ARRAY         = 'a',
    KCONF2_TYPE_TUPLE         = '(',
    KCONF2_TYPE_DICT_ENTRY    = '{'
};

class KConf2Private : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KConf2)

public:
    KConf2Private(KConf2* parent);
    QVariant typesToQvariant(const char *key, const char *value) const;
private:
    KConf2 *q_ptr;
    KSettings* m_pGsettings = nullptr;
    long signal_handler_id;
};

void k_free(char **str)
{
    for (int i = 0; str[i]; i++)
        free(str[i]);
}

QString qtifyName(const char *name)
{
    bool next_cap = false;
    QString result;

    while (*name) {
        if (*name == '-') {
            next_cap = true;
        } else if (next_cap) {
            result.append(QChar(*name).toUpper().toLatin1());
            next_cap = false;
        } else {
            result.append(*name);
        }

        name++;
    }

    return result;
}

static void settingChanged(KSettings *lingmostyle_settings, const char *key, void *user_data)
{
    KConf2 *self = (KConf2 *)user_data;

    QMetaObject::invokeMethod(self, "changed", Qt::QueuedConnection, Q_ARG(QString, qtifyName(key)));
}

KConf2::KConf2(const QByteArray &schema_id)
    :d_ptr(new KConf2Private(this))
{
    Q_D(KConf2);
    if (!d->m_pGsettings)
        d->m_pGsettings = kdk_conf2_new(schema_id, NULL);

    d->signal_handler_id = kdk_conf2_connect_signal(d->m_pGsettings, "changed",(KCallBack)settingChanged, this);
}

KConf2::~KConf2()
{
    Q_D(KConf2);
    kdk_conf2_ksettings_destroy(d->m_pGsettings);
}

QVariant KConf2::get(const QString &key) const
{
    Q_D(const KConf2);

    char *value = kdk_conf2_get_value(d->m_pGsettings, key.toUtf8().data());

    if(value == NULL )
    {
        qWarning()<<"There is no value for key";
        free(value);
        return QVariant();
    }
    else
    {
        QVariant qvalue = d->typesToQvariant(key.toUtf8().data(), value);
        free(value);
        return qvalue;
    }
}

void KConf2::set(const QString &key, const QVariant &value)
{
    if (!this->trySet(key, value))
        qWarning("unable to set key '%s' to value '%s'", key.toUtf8().constData(), value.toString().toUtf8().constData());
}

bool KConf2::trySet(const QString &key, const QVariant &value)
{
    Q_D(KConf2);
    if (!d->m_pGsettings)
        return false;

    bool success = false;

    success = kdk_conf2_set_value(d->m_pGsettings, key.toUtf8().data(), value.value<QString>().toUtf8().data());
    return success;
}

QStringList KConf2::keys() const
{
    Q_D(const KConf2);
    QStringList list;

    char **keys = kdk_conf2_list_keys(d->m_pGsettings);
    for (int i = 0; keys[i]; i++)
        list.append(qtifyName(keys[i]));

    k_free(keys);

    return list;
}

void KConf2::reset(const QString &qkey)
{
    Q_D(KConf2);
    if (!d->m_pGsettings)
        return;

    kdk_conf2_reset(d->m_pGsettings, qkey.toUtf8().data());
}

bool KConf2::isSettingsAvailable(const QString &schema_id)
{
    bool result = false;
    if (kdk_conf2_new(schema_id.toUtf8().data(), NULL)) {
        result = true;
    }

    return result;
}

QStringList KConf2::schemas(const QString &app, const QString &version) const
{
    Q_D(const KConf2);
    QStringList list;

    char **schemas = kdk_conf2_list_schemas(app.toUtf8().data(),version.toUtf8().data());
    if(schemas == NULL)
    {
        qWarning()<<"The obtained list schemas is empty";
    }
    else
    {
        for (int i = 0; schemas[i]; i++)
            list.append(qtifyName(schemas[i]));

        k_free(schemas);
    }
    return list;
}

QString KConf2::getDescrition(QString &key)
{
    Q_D(KConf2);
    char *value = kdk_conf2_get_descrition(d->m_pGsettings, key.toUtf8().data());

    QString str = QString::fromUtf8(value);

    free(value);
    return str;
}

QString KConf2::getSummary(QString &key)
{
    Q_D(KConf2);
    char *value = kdk_conf2_get_summary(d->m_pGsettings, key.toUtf8().data());

    QString str = QString::fromUtf8(value);

    free(value);

    return str;
}

QString KConf2::getType(QString &key)
{
    Q_D(KConf2);
    char *value = kdk_conf2_get_type(d->m_pGsettings, key.toUtf8().data());

    QString str = QString::fromUtf8(value);

    free(value);
    return str;
}

bool KConf2::isWritable(QString &key)
{
    Q_D(KConf2);
    if (!d->m_pGsettings)
        return false;

    bool success = false;

    success = kdk_conf2_is_writable(d->m_pGsettings, key.toUtf8().data());
    return success;
}

QString KConf2::getId()
{
    Q_D(KConf2);
    char *value = kdk_conf2_get_id(d->m_pGsettings);
    QString str = QString::fromUtf8(value);

    free(value);
    return str;
}

QString KConf2::getVersion()
{
    Q_D(KConf2);
    char *value = kdk_conf2_get_version(d->m_pGsettings);
    QString str = QString::fromUtf8(value);

    free(value);
    return str;
}

KSettings *KConf2::getChild(QString &name)
{
    Q_D(KConf2);
    KSettings *settings = kdk_conf2_get_child(d->m_pGsettings, name.toUtf8().data());

    return settings;
}

QStringList KConf2::children() const
{
    Q_D(const KConf2);
    QStringList list;

    char **keys = kdk_conf2_list_children(d->m_pGsettings);
    for (int i = 0; keys[i]; i++)
        list.append(qtifyName(keys[i]));

    k_free(keys);

    return list;
}

KConf2Private::KConf2Private(KConf2 *parent)
    :q_ptr(parent)
{
    setParent(parent);
}

QVariant KConf2Private::typesToQvariant(const char *key, const char *value) const
{
    Q_Q(const KConf2);
    char *type = kdk_conf2_get_type(m_pGsettings, key);
        switch (*type)
        {
        case KCONF2_TYPE_BOOLEAN:
            return QVariant(value);

        case KCONF2_TYPE_BYTE:
            return QVariant(QString(value));

        case KCONF2_TYPE_INT16:
            return QVariant(QString(value).toShort());

        case KCONF2_TYPE_UINT16:
            return QVariant(QString(value).toUShort());

        case KCONF2_TYPE_INT32:
            return QVariant(QString(value).toInt());

        case KCONF2_TYPE_UINT32:
            return QVariant(QString(value).toUInt());

        case KCONF2_TYPE_INT64:
            return QVariant(QString(value).toLongLong());

        case KCONF2_TYPE_UINT64:
            return QVariant(QString(value).toULongLong());

        case KCONF2_TYPE_DOUBLE:
            return QVariant(QString(value).toDouble());

        case KCONF2_TYPE_STRING:
            return QVariant(QString(value));

        case KCONF2_TYPE_ARRAY:
        default:
            return QVariant();
        }
}

}

#include "kconf2.moc"
#include "moc_kconf2.cpp"
