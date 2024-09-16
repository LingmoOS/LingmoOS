// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCONFIGFILE_H
#define DCONFIGFILE_H

#include <dtkcore_global.h>
#include <DObject>
#include <QStringList>
#include <QFlags>
#include <QVariant>
#include <QLocale>
#include <QJsonDocument>
#include <QDebug>

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

DCORE_BEGIN_NAMESPACE

class DConfigMeta;
class DConfigCache;
class DConfigFilePrivate;
class LIBDTKCORESHARED_EXPORT DConfigFile : public DObject{
    D_DECLARE_PRIVATE(DConfigFile)
public:
    enum Flag {
        NoOverride = 1 << 0,
        Global = 1 << 1,
        UserPublic = 1 << 2
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    enum Permissions {
        ReadOnly,
        ReadWrite
    };

    enum Visibility {
        Private,
        Public
    };

    struct Version {
        quint16 major;
        quint16 minor;
    };

    static constexpr Version supportedVersion();

    explicit DConfigFile(const QString &appId, const QString &name,
                         const QString &subpath = QString());
    explicit DConfigFile(const DConfigFile &other);

    bool load(const QString &localPrefix = QString());
    bool load(QIODevice *meta, const QList<QIODevice*> &overrides);

    bool save(const QString &localPrefix = QString(), QJsonDocument::JsonFormat format = QJsonDocument::Indented,
              bool sync = false) const;

    bool isValid() const;
    QVariant value(const QString &key, DConfigCache *userCache = nullptr) const;
    QVariant cacheValue(DConfigCache *userCache, const QString &key) const;
    bool setValue(const QString &key, const QVariant &value, const QString &callerAppid,
                  DConfigCache *userCache = nullptr);

    DConfigCache *createUserCache(const uint uid);
    DConfigCache *globalCache() const;

    DConfigMeta *meta();

protected:
    friend QDebug operator<<(QDebug, const DConfigFile &);
};

class LIBDTKCORESHARED_EXPORT DConfigMeta {
public:
    virtual ~DConfigMeta();
    virtual DConfigFile::Version version() const = 0;
    virtual void setVersion(quint16 major, quint16 minor) = 0;

    virtual bool load(const QString &localPrefix = QString()) = 0;

    virtual bool load(QIODevice *meta, const QList<QIODevice*> &overrides) = 0;

    virtual QStringList keyList() const = 0;
    virtual DConfigFile::Flags flags(const QString &key) const = 0;
    virtual DConfigFile::Permissions permissions(const QString &key) const = 0;
    virtual DConfigFile::Visibility visibility(const QString &key) const = 0;
    virtual int serial(const QString &key) const = 0;

    virtual QString displayName(const QString &key, const QLocale &locale) = 0;
    virtual QString description(const QString &key, const QLocale &locale) = 0;

    virtual QString metaPath(const QString &localPrefix = QString(), bool *useAppId = nullptr) const = 0;
    virtual QStringList allOverrideDirs(const bool useAppId, const QString &prefix = QString()) const = 0;

    virtual QVariant value(const QString &key) const = 0;
    static QStringList genericMetaDirs(const QString &localPrefix = QString());
    static QStringList applicationMetaDirs(const QString &localPrefix, const QString &appId);
};

class LIBDTKCORESHARED_EXPORT DConfigCache {
public:
    virtual ~DConfigCache();

    virtual bool load(const QString &localPrefix = QString()) = 0;
    virtual bool save(const QString &localPrefix = QString(),
                      QJsonDocument::JsonFormat format = QJsonDocument::Indented, bool sync = false) = 0;
    virtual bool isGlobal() const = 0;

    virtual void remove(const QString &key) = 0;
    virtual QStringList keyList() const = 0;
    virtual bool setValue(const QString &key, const QVariant &value, const int serial,
                          const uint uid, const QString &callerAppid) = 0;
    virtual QVariant value(const QString &key) const = 0;
    virtual int serial(const QString &key) const = 0;
    virtual uint uid() const = 0;

    virtual void setCachePathPrefix(const QString &prefix) = 0;
};

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const DConfigFile &);
#endif

Q_DECLARE_OPERATORS_FOR_FLAGS(DConfigFile::Flags)

DCORE_END_NAMESPACE

#endif // DCONFIGFILE_H
