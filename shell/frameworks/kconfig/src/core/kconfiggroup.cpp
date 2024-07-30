/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfiggroup.h"
#include "kconfiggroup_p.h"

#include "kconfig.h"
#include "kconfig_core_log_settings.h"
#include "kconfig_p.h"
#include "kconfigdata_p.h"
#include "ksharedconfig.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QPoint>
#include <QRect>
#include <QSharedData>
#include <QString>
#include <QTextStream>
#include <QUrl>
#include <QUuid>

#include <algorithm>
#include <array>
#include <math.h>
#include <stdlib.h>

class KConfigGroupPrivate : public QSharedData
{
public:
    KConfigGroupPrivate(KConfig *owner, bool isImmutable, bool isConst, const QString &name)
        : mOwner(owner)
        , mName(name)
        , bImmutable(isImmutable)
        , bConst(isConst)
    {
        if (Q_UNLIKELY(!mOwner->name().isEmpty() && mOwner->accessMode() == KConfigBase::NoAccess)) {
            qCWarning(KCONFIG_CORE_LOG) << "Created a KConfigGroup on an inaccessible config location" << mOwner->name() << name;
        }
    }

    KConfigGroupPrivate(const KSharedConfigPtr &owner, const QString &name)
        : sOwner(owner)
        , mOwner(sOwner.data())
        , mName(name)
        , bImmutable(name.isEmpty() ? owner->isImmutable() : owner->isGroupImmutable(name))
        , bConst(false)
    {
        if (Q_UNLIKELY(!mOwner->name().isEmpty() && mOwner->accessMode() == KConfigBase::NoAccess)) {
            qCWarning(KCONFIG_CORE_LOG) << "Created a KConfigGroup on an inaccessible config location" << mOwner->name() << name;
        }
    }

    KConfigGroupPrivate(KConfigGroup *parent, bool isImmutable, bool isConst, const QString &name)
        : sOwner(parent->d->sOwner)
        , mOwner(parent->d->mOwner)
        , mName(name)
        , bImmutable(isImmutable)
        , bConst(isConst)
    {
        if (!parent->d->mName.isEmpty()) {
            mParent = parent->d;
        }
    }

    KConfigGroupPrivate(const KConfigGroupPrivate *other, bool isImmutable, const QString &name)
        : sOwner(other->sOwner)
        , mOwner(other->mOwner)
        , mName(name)
        , bImmutable(isImmutable)
        , bConst(other->bConst)
    {
        if (!other->mName.isEmpty()) {
            mParent = const_cast<KConfigGroupPrivate *>(other);
        }
    }

    KSharedConfig::Ptr sOwner;
    KConfig *mOwner;
    QExplicitlySharedDataPointer<KConfigGroupPrivate> mParent;
    QString mName;

    /* bitfield */
    const bool bImmutable : 1; // is this group immutable?
    const bool bConst : 1; // is this group read-only?

    QString fullName() const
    {
        if (!mParent) {
            return name();
        }
        return mParent->fullName(mName);
    }

    QString name() const
    {
        if (mName.isEmpty()) {
            return QStringLiteral("<default>");
        }
        return mName;
    }

    QString fullName(const QString &aGroup) const
    {
        if (mName.isEmpty()) {
            return aGroup;
        }
        return fullName() + QLatin1Char('\x1d') + aGroup;
    }

    static QExplicitlySharedDataPointer<KConfigGroupPrivate> create(KConfigBase *master, const QString &name, bool isImmutable, bool isConst)
    {
        QExplicitlySharedDataPointer<KConfigGroupPrivate> data;
        if (dynamic_cast<KConfigGroup *>(master)) {
            data = new KConfigGroupPrivate(static_cast<KConfigGroup *>(master), isImmutable, isConst, name);
        } else {
            data = new KConfigGroupPrivate(dynamic_cast<KConfig *>(master), isImmutable, isConst, name);
        }
        return data;
    }

    static QByteArray serializeList(const QList<QByteArray> &list);
    static QStringList deserializeList(const QString &data);
};

QByteArray KConfigGroupPrivate::serializeList(const QList<QByteArray> &list)
{
    QByteArray value;

    if (!list.isEmpty()) {
        auto it = list.cbegin();
        const auto end = list.cend();

        value = QByteArray(*it).replace('\\', QByteArrayLiteral("\\\\")).replace(',', QByteArrayLiteral("\\,"));

        while (++it != end) {
            // In the loop, so it is not done when there is only one element.
            // Doing it repeatedly is a pretty cheap operation.
            value.reserve(4096);

            value += ',';
            value += QByteArray(*it).replace('\\', QByteArrayLiteral("\\\\")).replace(',', QByteArrayLiteral("\\,"));
        }

        // To be able to distinguish an empty list from a list with one empty element.
        if (value.isEmpty()) {
            value = QByteArrayLiteral("\\0");
        }
    }

    return value;
}

QStringList KConfigGroupPrivate::deserializeList(const QString &data)
{
    if (data.isEmpty()) {
        return QStringList();
    }
    if (data == QLatin1String("\\0")) {
        return QStringList(QString());
    }
    QStringList value;
    QString val;
    val.reserve(data.size());
    bool quoted = false;
    for (int p = 0; p < data.length(); p++) {
        if (quoted) {
            val += data[p];
            quoted = false;
        } else if (data[p].unicode() == '\\') {
            quoted = true;
        } else if (data[p].unicode() == ',') {
            val.squeeze(); // release any unused memory
            value.append(val);
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    value.append(val);
    return value;
}

static QVarLengthArray<int, 8> asIntList(QByteArrayView string)
{
    int start = 0;
    int next = start;
    QVarLengthArray<int, 8> ret;
    while ((next = string.indexOf(',', start)) != -1) {
        ret.push_back(string.sliced(start, next - start).toInt());
        start = next + 1;
    }
    ret.push_back(string.sliced(start, string.size() - start).toInt());
    return ret;
}

static QVarLengthArray<qreal, 8> asRealList(QByteArrayView string)
{
    int start = 0;
    int next = start;
    QVarLengthArray<qreal, 8> ret;
    while ((next = string.indexOf(',', start)) != -1) {
        ret.push_back(string.sliced(start, next - start).toDouble());
        start = next + 1;
    }
    ret.push_back(string.sliced(start, string.size() - start).toDouble());
    return ret;
}

static QString errString(const char *pKey, const QByteArray &value, const QVariant &aDefault)
{
    return QStringLiteral("\"%1\" - conversion of \"%3\" to %2 failed")
        .arg(QString::fromLatin1(pKey), QString::fromLatin1(aDefault.typeName()), QString::fromLatin1(value));
}

static QString formatError(int expected, int got)
{
    return QStringLiteral(" (wrong format: expected %1 items, got %2)").arg(expected).arg(got);
}

QVariant KConfigGroup::convertToQVariant(const char *pKey, const QByteArray &value, const QVariant &aDefault)
{
    // if a type handler is added here you must add a QVConversions definition
    // to kconfigconversioncheck_p.h, or KConfigConversionCheck::to_QVariant will not allow
    // readEntry<T> to convert to QVariant.
    switch (static_cast<QMetaType::Type>(aDefault.userType())) {
    case QMetaType::UnknownType:
        return QVariant();
    case QMetaType::QString:
        // this should return the raw string not the dollar expanded string.
        // imho if processed string is wanted should call
        // readEntry(key, QString) not readEntry(key, QVariant)
        return QString::fromUtf8(value);
    case QMetaType::QUuid:
        return QUuid::fromString(value);
    case QMetaType::QVariantList:
    case QMetaType::QStringList:
        return KConfigGroupPrivate::deserializeList(QString::fromUtf8(value));
    case QMetaType::QByteArray:
        return value;
    case QMetaType::Bool: {
        static const std::array<const char *, 4> negatives = {"false", "no", "off", "0"};

        return std::all_of(negatives.begin(), negatives.end(), [value](const char *negativeString) {
            return value.compare(negativeString, Qt::CaseInsensitive) != 0;
        });
    }
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong: {
        QVariant tmp = value;
        if (!tmp.convert(aDefault.metaType())) {
            tmp = aDefault;
        }
        return tmp;
    }
    case QMetaType::QPoint: {
        const auto list = asIntList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        return QPoint(list.at(0), list.at(1));
    }
    case QMetaType::QPointF: {
        const auto list = asRealList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        return QPointF(list.at(0), list.at(1));
    }
    case QMetaType::QRect: {
        const auto list = asIntList(value);

        if (list.count() != 4) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(4, list.count());
            return aDefault;
        }
        const QRect rect(list.at(0), list.at(1), list.at(2), list.at(3));
        if (!rect.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return rect;
    }
    case QMetaType::QRectF: {
        const auto list = asRealList(value);

        if (list.count() != 4) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(4, list.count());
            return aDefault;
        }
        const QRectF rect(list.at(0), list.at(1), list.at(2), list.at(3));
        if (!rect.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return rect;
    }
    case QMetaType::QSize: {
        const auto list = asIntList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        const QSize size(list.at(0), list.at(1));
        if (!size.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return size;
    }
    case QMetaType::QSizeF: {
        const auto list = asRealList(value);

        if (list.count() != 2) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(2, list.count());
            return aDefault;
        }
        const QSizeF size(list.at(0), list.at(1));
        if (!size.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return size;
    }
    case QMetaType::QDateTime: {
        const auto list = asRealList(value);
        if (list.count() < 6) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(6, list.count());
            return aDefault;
        }
        const QDate date(list.at(0), list.at(1), list.at(2));
        const qreal totalSeconds = list.at(5);
        qreal seconds;
        const qreal fractional = modf(totalSeconds, &seconds);
        const qreal milliseconds = round(fractional * 1000.0);
        const QTime time(list.at(3), list.at(4), seconds, milliseconds);
        const QDateTime dt(date, time);
        if (!dt.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return dt;
    }
    case QMetaType::QDate: {
        auto list = asIntList(value);
        // list.count == 6 -> don't break config files that stored QDate as QDateTime
        if (list.count() != 3 && list.count() != 6) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault) << formatError(3, list.count());
            return aDefault;
        }
        const QDate date(list.at(0), list.at(1), list.at(2));
        if (!date.isValid()) {
            qCWarning(KCONFIG_CORE_LOG) << errString(pKey, value, aDefault);
            return aDefault;
        }
        return date;
    }
    case QMetaType::QColor:
    case QMetaType::QFont:
        qCWarning(KCONFIG_CORE_LOG) << "KConfigGroup::readEntry was passed GUI type '" << aDefault.typeName()
                                    << "' but KConfigGui isn't linked! If it is linked to your program, "
                                       "this is a platform bug. Please inform the KDE developers";
        break;
    case QMetaType::QUrl:
        return QUrl(QString::fromUtf8(value));

    default:
        break;
    }

    qCWarning(KCONFIG_CORE_LOG) << "unhandled type " << aDefault.typeName();
    return QVariant();
}

static bool cleanHomeDirPath(QString &path, const QString &homeDir)
{
#ifdef Q_OS_WIN // safer
    if (!QDir::toNativeSeparators(path).startsWith(QDir::toNativeSeparators(homeDir))) {
        return false;
    }
#else
    if (!path.startsWith(homeDir)) {
        return false;
    }
#endif

    int len = homeDir.length();
    // replace by "$HOME" if possible
    if (len && (path.length() == len || path[len] == QLatin1Char('/'))) {
        path.replace(0, len, QStringLiteral("$HOME"));
        return true;
    }

    return false;
}

static QString translatePath(QString path) // krazy:exclude=passbyvalue
{
    if (path.isEmpty()) {
        return path;
    }

    // only "our" $HOME should be interpreted
    path.replace(QLatin1Char('$'), QLatin1String("$$"));

    const bool startsWithFile = path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive);
    path = startsWithFile ? QUrl(path).toLocalFile() : path;

    if (QDir::isRelativePath(path)) {
        return path;
    }

    // Use the same thing as what expandString() will do, to keep data intact
#ifdef Q_OS_WIN
    const QString homeDir = QDir::homePath();
#else
    const QString homeDir = QFile::decodeName(qgetenv("HOME"));
#endif
    (void)cleanHomeDirPath(path, homeDir);

    if (startsWithFile) {
        path = QUrl::fromLocalFile(path).toString();
    }

    return path;
}

KConfigGroup::KConfigGroup()
    : d()
{
}

bool KConfigGroup::isValid() const
{
    return bool(d);
}

KConfigGroupGui _kde_internal_KConfigGroupGui;
static inline bool readEntryGui(const QByteArray &data, const char *key, const QVariant &input, QVariant &output)
{
    if (_kde_internal_KConfigGroupGui.readEntryGui) {
        return _kde_internal_KConfigGroupGui.readEntryGui(data, key, input, output);
    }
    return false;
}

static inline bool writeEntryGui(KConfigGroup *cg, const char *key, const QVariant &input, KConfigGroup::WriteConfigFlags flags)
{
    if (_kde_internal_KConfigGroupGui.writeEntryGui) {
        return _kde_internal_KConfigGroupGui.writeEntryGui(cg, key, input, flags);
    }
    return false;
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QString &_group)
    : d(KConfigGroupPrivate::create(master, _group, master->isGroupImmutable(_group), false))
{
}

KConfigGroup::KConfigGroup(const KConfigBase *master, const QString &_group)
    : d(KConfigGroupPrivate::create(const_cast<KConfigBase *>(master), _group, master->isGroupImmutable(_group), true))
{
}

KConfigGroup::KConfigGroup(const KSharedConfigPtr &master, const QString &_group)
    : d(new KConfigGroupPrivate(master, _group))
{
}

KConfigGroup &KConfigGroup::operator=(const KConfigGroup &rhs)
{
    d = rhs.d;
    return *this;
}

KConfigGroup::KConfigGroup(const KConfigGroup &rhs)
    : d(rhs.d)
{
}

KConfigGroup::~KConfigGroup()
{
    d.reset();
}

KConfigGroup KConfigGroup::groupImpl(const QString &aGroup)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::groupImpl", "accessing an invalid group");
    Q_ASSERT_X(!aGroup.isEmpty(), "KConfigGroup::groupImpl", "can not have an unnamed child group");

    KConfigGroup newGroup;

    newGroup.d = new KConfigGroupPrivate(this, isGroupImmutableImpl(aGroup), d->bConst, aGroup);

    return newGroup;
}

const KConfigGroup KConfigGroup::groupImpl(const QString &aGroup) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::groupImpl", "accessing an invalid group");
    Q_ASSERT_X(!aGroup.isEmpty(), "KConfigGroup::groupImpl", "can not have an unnamed child group");

    KConfigGroup newGroup;

    newGroup.d = new KConfigGroupPrivate(const_cast<KConfigGroup *>(this), isGroupImmutableImpl(aGroup), true, aGroup);

    return newGroup;
}

KConfigGroup KConfigGroup::parent() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::parent", "accessing an invalid group");

    KConfigGroup parentGroup;

    if (d->mParent) {
        parentGroup.d = d->mParent;
    } else {
        parentGroup.d = new KConfigGroupPrivate(d->mOwner, d->mOwner->isImmutable(), d->bConst, QString());
        // make sure we keep the refcount up on the KConfig object
        parentGroup.d->sOwner = d->sOwner;
    }

    return parentGroup;
}

void KConfigGroup::deleteGroup(WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::deleteGroup", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::deleteGroup", "deleting a read-only group");

    config()->deleteGroup(d->fullName(), flags);
}

QString KConfigGroup::name() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::name", "accessing an invalid group");

    return d->name();
}

bool KConfigGroup::exists() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::exists", "accessing an invalid group");

    return config()->hasGroup(d->fullName());
}

bool KConfigGroup::sync()
{
    Q_ASSERT_X(isValid(), "KConfigGroup::sync", "accessing an invalid group");

    if (!d->bConst) {
        return config()->sync();
    }

    return false;
}

QMap<QString, QString> KConfigGroup::entryMap() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::entryMap", "accessing an invalid group");

    return config()->entryMap(d->fullName());
}

KConfig *KConfigGroup::config()
{
    Q_ASSERT_X(isValid(), "KConfigGroup::config", "accessing an invalid group");

    return d->mOwner;
}

const KConfig *KConfigGroup::config() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::config", "accessing an invalid group");

    return d->mOwner;
}

bool KConfigGroup::isEntryImmutable(const char *key) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::isEntryImmutable", "accessing an invalid group");

    return (isImmutable() || !config()->d_func()->canWriteEntry(d->fullName(), key, config()->readDefaults()));
}

bool KConfigGroup::isEntryImmutable(const QString &key) const
{
    return isEntryImmutable(key.toUtf8().constData());
}

QString KConfigGroup::readEntryUntranslated(const QString &pKey, const QString &aDefault) const
{
    return readEntryUntranslated(pKey.toUtf8().constData(), aDefault);
}

QString KConfigGroup::readEntryUntranslated(const char *key, const QString &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntryUntranslated", "accessing an invalid group");

    QString result = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchFlags(), nullptr);
    if (result.isNull()) {
        return aDefault;
    }
    return result;
}

QString KConfigGroup::readEntry(const char *key, const char *aDefault) const
{
    return readEntry(key, QString::fromUtf8(aDefault));
}

QString KConfigGroup::readEntry(const QString &key, const char *aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QString KConfigGroup::readEntry(const char *key, const QString &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    bool expand = false;

    // read value from the entry map
    QString aValue = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized, &expand);
    if (aValue.isNull()) {
        aValue = aDefault;
    }

    if (expand) {
        return KConfigPrivate::expandString(aValue);
    }

    return aValue;
}

QString KConfigGroup::readEntry(const QString &key, const QString &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readEntry(const char *key, const QStringList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    const QString data = readEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    return KConfigGroupPrivate::deserializeList(data);
}

QStringList KConfigGroup::readEntry(const QString &key, const QStringList &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QVariant KConfigGroup::readEntry(const char *key, const QVariant &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    const QByteArray data = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized);
    if (data.isNull()) {
        return aDefault;
    }

    QVariant value;
    if (!readEntryGui(data, key, aDefault, value)) {
        return convertToQVariant(key, data, aDefault);
    }

    return value;
}

QVariant KConfigGroup::readEntry(const QString &key, const QVariant &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QVariantList KConfigGroup::readEntry(const char *key, const QVariantList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readEntry", "accessing an invalid group");

    const QString data = readEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    const auto &list = KConfigGroupPrivate::deserializeList(data);

    QVariantList value;
    value.reserve(list.count());
    for (const QString &v : list) {
        value << v;
    }

    return value;
}

QVariantList KConfigGroup::readEntry(const QString &key, const QVariantList &aDefault) const
{
    return readEntry(key.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readXdgListEntry(const QString &key, const QStringList &aDefault) const
{
    return readXdgListEntry(key.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readXdgListEntry(const char *key, const QStringList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readXdgListEntry", "accessing an invalid group");

    const QString data = readEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    QStringList value;
    QString val;
    val.reserve(data.size());
    // XXX List serialization being a separate layer from low-level parsing is
    // probably a bug. No affected entries are defined, though.
    bool quoted = false;
    for (int p = 0; p < data.length(); p++) {
        if (quoted) {
            val += data[p];
            quoted = false;
        } else if (data[p] == QLatin1Char('\\')) {
            quoted = true;
        } else if (data[p] == QLatin1Char(';')) {
            value.append(val);
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    if (!val.isEmpty()) {
        value.append(val);
    }
    return value;
}

QString KConfigGroup::readPathEntry(const QString &pKey, const QString &aDefault) const
{
    return readPathEntry(pKey.toUtf8().constData(), aDefault);
}

QString KConfigGroup::readPathEntry(const char *key, const QString &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readPathEntry", "accessing an invalid group");

    bool expand = false;

    QString aValue = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized, &expand);
    if (aValue.isNull()) {
        aValue = aDefault;
    }

    return KConfigPrivate::expandString(aValue);
}

QStringList KConfigGroup::readPathEntry(const QString &pKey, const QStringList &aDefault) const
{
    return readPathEntry(pKey.toUtf8().constData(), aDefault);
}

QStringList KConfigGroup::readPathEntry(const char *key, const QStringList &aDefault) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::readPathEntry", "accessing an invalid group");

    const QString data = readPathEntry(key, QString());
    if (data.isNull()) {
        return aDefault;
    }

    return KConfigGroupPrivate::deserializeList(data);
}

void KConfigGroup::writeEntry(const char *key, const QString &value, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    writeEntry(key, value.toUtf8(), flags);
}

void KConfigGroup::writeEntry(const QString &key, const QString &value, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), value, flags);
}

void KConfigGroup::writeEntry(const QString &key, const char *value, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    writeEntry(key.toUtf8().constData(), QVariant(QString::fromLatin1(value)), pFlags);
}

void KConfigGroup::writeEntry(const char *key, const char *value, WriteConfigFlags pFlags)
{
    writeEntry(key, QVariant(QString::fromLatin1(value)), pFlags);
}

void KConfigGroup::writeEntry(const char *key, const QByteArray &value, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    config()->d_func()->putData(d->fullName(), key, value.isNull() ? QByteArray("") : value, flags);
}

void KConfigGroup::writeEntry(const QString &key, const QByteArray &value, WriteConfigFlags pFlags)
{
    writeEntry(key.toUtf8().constData(), value, pFlags);
}

void KConfigGroup::writeEntry(const char *key, const QStringList &list, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    QList<QByteArray> balist;
    balist.reserve(list.count());

    for (const QString &entry : list) {
        balist.append(entry.toUtf8());
    }

    writeEntry(key, KConfigGroupPrivate::serializeList(balist), flags);
}

void KConfigGroup::writeEntry(const QString &key, const QStringList &list, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), list, flags);
}

void KConfigGroup::writeEntry(const char *key, const QVariantList &list, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    QList<QByteArray> data;
    data.reserve(list.count());

    for (const QVariant &v : list) {
        if (v.userType() == QMetaType::QByteArray) {
            data << v.toByteArray();
        } else {
            data << v.toString().toUtf8();
        }
    }

    writeEntry(key, KConfigGroupPrivate::serializeList(data), flags);
}

void KConfigGroup::writeEntry(const char *key, const QVariant &value, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeEntry", "writing to a read-only group");

    if (writeEntryGui(this, key, value, flags)) {
        return; // GUI type that was handled
    }

    QByteArray data;
    // if a type handler is added here you must add a QVConversions definition
    // to kconfigconversioncheck_p.h, or KConfigConversionCheck::to_QVariant will not allow
    // writeEntry<T> to convert to QVariant.
    switch (static_cast<QMetaType::Type>(value.userType())) {
    case QMetaType::UnknownType:
        data = "";
        break;
    case QMetaType::QByteArray:
        data = value.toByteArray();
        break;
    case QMetaType::QString:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Bool:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        data = value.toString().toUtf8();
        break;
    case QMetaType::QVariantList:
        if (!value.canConvert<QStringList>()) {
            qCWarning(KCONFIG_CORE_LOG) << "not all types in \"" << key
                                        << "\" can convert to QString,"
                                           " information will be lost";
        }
        Q_FALLTHROUGH();
    case QMetaType::QStringList:
        writeEntry(key, value.toList(), flags);
        return;
    case QMetaType::QPoint: {
        const QPoint rPoint = value.toPoint();

        const QVariantList list{rPoint.x(), rPoint.y()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QPointF: {
        const QPointF point = value.toPointF();

        const QVariantList list{point.x(), point.y()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QRect: {
        const QRect rRect = value.toRect();

        const QVariantList list{rRect.left(), rRect.top(), rRect.width(), rRect.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QRectF: {
        const QRectF rRectF = value.toRectF();

        const QVariantList list{rRectF.left(), rRectF.top(), rRectF.width(), rRectF.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QSize: {
        const QSize rSize = value.toSize();

        const QVariantList list{rSize.width(), rSize.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QUuid: {
        writeEntry(key, value.toString(), flags);
        return;
    }
    case QMetaType::QSizeF: {
        const QSizeF rSizeF = value.toSizeF();

        const QVariantList list{rSizeF.width(), rSizeF.height()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QDate: {
        const QDate date = value.toDate();

        const QVariantList list{date.year(), date.month(), date.day()};

        writeEntry(key, list, flags);
        return;
    }
    case QMetaType::QDateTime: {
        const QDateTime rDateTime = value.toDateTime();

        const QTime time = rDateTime.time();
        const QDate date = rDateTime.date();

        const QVariantList list{
            date.year(),
            date.month(),
            date.day(),

            time.hour(),
            time.minute(),
            time.second() + time.msec() / 1000.0,
        };

        writeEntry(key, list, flags);
        return;
    }

    case QMetaType::QColor:
    case QMetaType::QFont:
        qCWarning(KCONFIG_CORE_LOG) << "KConfigGroup::writeEntry was passed GUI type '" << value.typeName()
                                    << "' but KConfigGui isn't linked! If it is linked to your program, this is a platform bug. "
                                       "Please inform the KDE developers";
        break;
    case QMetaType::QUrl:
        data = QUrl(value.toUrl()).toString().toUtf8();
        break;
    default:
        qCWarning(KCONFIG_CORE_LOG) << "KConfigGroup::writeEntry - unhandled type" << value.typeName() << "in group" << name();
    }

    writeEntry(key, data, flags);
}

void KConfigGroup::writeEntry(const QString &key, const QVariant &value, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), value, flags);
}

void KConfigGroup::writeEntry(const QString &key, const QVariantList &list, WriteConfigFlags flags)
{
    writeEntry(key.toUtf8().constData(), list, flags);
}

void KConfigGroup::writeXdgListEntry(const QString &key, const QStringList &value, WriteConfigFlags pFlags)
{
    writeXdgListEntry(key.toUtf8().constData(), value, pFlags);
}

void KConfigGroup::writeXdgListEntry(const char *key, const QStringList &list, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writeXdgListEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writeXdgListEntry", "writing to a read-only group");

    QString value;
    value.reserve(4096);

    // XXX List serialization being a separate layer from low-level escaping is
    // probably a bug. No affected entries are defined, though.
    for (QString val : list) { // clazy:exclude=range-loop
        val.replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(QLatin1Char(';'), QLatin1String("\\;"));
        value += val + QLatin1Char(';');
    }

    writeEntry(key, value, flags);
}

void KConfigGroup::writePathEntry(const QString &pKey, const QString &path, WriteConfigFlags pFlags)
{
    writePathEntry(pKey.toUtf8().constData(), path, pFlags);
}

void KConfigGroup::writePathEntry(const char *pKey, const QString &path, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writePathEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writePathEntry", "writing to a read-only group");

    config()->d_func()->putData(d->fullName(), pKey, translatePath(path).toUtf8(), pFlags, true);
}

void KConfigGroup::writePathEntry(const QString &pKey, const QStringList &value, WriteConfigFlags pFlags)
{
    writePathEntry(pKey.toUtf8().constData(), value, pFlags);
}

void KConfigGroup::writePathEntry(const char *pKey, const QStringList &value, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::writePathEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::writePathEntry", "writing to a read-only group");

    QList<QByteArray> list;
    list.reserve(value.length());
    for (const QString &path : value) {
        list << translatePath(path).toUtf8();
    }

    config()->d_func()->putData(d->fullName(), pKey, KConfigGroupPrivate::serializeList(list), pFlags, true);
}

void KConfigGroup::deleteEntry(const char *key, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::deleteEntry", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::deleteEntry", "deleting from a read-only group");

    config()->d_func()->putData(d->fullName(), key, QByteArray(), flags);
}

void KConfigGroup::deleteEntry(const QString &key, WriteConfigFlags flags)
{
    deleteEntry(key.toUtf8().constData(), flags);
}

void KConfigGroup::revertToDefault(const char *key, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::revertToDefault", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::revertToDefault", "writing to a read-only group");

    config()->d_func()->revertEntry(d->fullName(), key, flags);
}

void KConfigGroup::revertToDefault(const QString &key, WriteConfigFlags flags)
{
    revertToDefault(key.toUtf8().constData(), flags);
}

bool KConfigGroup::hasDefault(const char *key) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::hasDefault", "accessing an invalid group");

    KEntryMap::SearchFlags flags = KEntryMap::SearchDefaults | KEntryMap::SearchLocalized;

    return !config()->d_func()->lookupData(d->fullName(), key, flags).isNull();
}

bool KConfigGroup::hasDefault(const QString &key) const
{
    return hasDefault(key.toUtf8().constData());
}

bool KConfigGroup::hasKey(const char *key) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::hasKey", "accessing an invalid group");

    KEntryMap::SearchFlags flags = KEntryMap::SearchLocalized;
    if (config()->readDefaults()) {
        flags |= KEntryMap::SearchDefaults;
    }

    return !config()->d_func()->lookupData(d->fullName(), key, flags).isNull();
}

bool KConfigGroup::hasKey(const QString &key) const
{
    return hasKey(key.toUtf8().constData());
}

bool KConfigGroup::isImmutable() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::isImmutable", "accessing an invalid group");

    return d->bImmutable;
}

QStringList KConfigGroup::groupList() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::groupList", "accessing an invalid group");

    return config()->d_func()->groupList(d->fullName());
}

QStringList KConfigGroup::keyList() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::keyList", "accessing an invalid group");

    return config()->d_func()->usedKeyList(d->fullName());
}

void KConfigGroup::markAsClean()
{
    Q_ASSERT_X(isValid(), "KConfigGroup::markAsClean", "accessing an invalid group");

    config()->markAsClean();
}

KConfigGroup::AccessMode KConfigGroup::accessMode() const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::accessMode", "accessing an invalid group");

    return config()->accessMode();
}

bool KConfigGroup::hasGroupImpl(const QString &b) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::hasGroupImpl", "accessing an invalid group");

    return config()->hasGroup(d->fullName(b));
}

void KConfigGroup::deleteGroupImpl(const QString &b, WriteConfigFlags flags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::deleteGroupImpl", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::deleteGroupImpl", "deleting from a read-only group");

    config()->deleteGroup(d->fullName(b), flags);
}

bool KConfigGroup::isGroupImmutableImpl(const QString &groupName) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::isGroupImmutableImpl", "accessing an invalid group");

    if (!hasGroupImpl(groupName)) { // group doesn't exist yet
        return d->bImmutable; // child groups are immutable if the parent is immutable.
    }

    return config()->isGroupImmutable(d->fullName(groupName));
}

void KConfigGroup::copyTo(KConfigBase *other, WriteConfigFlags pFlags) const
{
    Q_ASSERT_X(isValid(), "KConfigGroup::copyTo", "accessing an invalid group");
    Q_ASSERT(other != nullptr);

    if (KConfigGroup *otherGroup = dynamic_cast<KConfigGroup *>(other)) {
        config()->d_func()->copyGroup(d->fullName(), otherGroup->d->fullName(), otherGroup, pFlags);
    } else if (KConfig *otherConfig = dynamic_cast<KConfig *>(other)) {
        KConfigGroup newGroup = otherConfig->group(d->fullName());
        otherConfig->d_func()->copyGroup(d->fullName(), d->fullName(), &newGroup, pFlags);
    } else {
        Q_ASSERT_X(false, "KConfigGroup::copyTo", "unknown type of KConfigBase");
    }
}

void KConfigGroup::reparent(KConfigBase *parent, WriteConfigFlags pFlags)
{
    Q_ASSERT_X(isValid(), "KConfigGroup::reparent", "accessing an invalid group");
    Q_ASSERT_X(!d->bConst, "KConfigGroup::reparent", "reparenting a read-only group");
    Q_ASSERT_X(!d->bImmutable, "KConfigGroup::reparent", "reparenting an immutable group");
    Q_ASSERT(parent != nullptr);

    KConfigGroup oldGroup(*this);

    d = KConfigGroupPrivate::create(parent, d->mName, false, false);
    oldGroup.copyTo(this, pFlags);
    oldGroup.deleteGroup(); // so that the entries with the old group name are deleted on sync
}

void KConfigGroup::moveValue(const char *key, KConfigGroup &other, WriteConfigFlags pFlags)
{
    const QString groupName = name();
    const auto entry = config()->d_ptr->lookupInternalEntry(groupName, key, KEntryMap::SearchLocalized);

    // Only write the entry if it is not null, if it is a global enry there is no point in moving it
    if (!entry.mValue.isNull() && !entry.bGlobal) {
        deleteEntry(key, pFlags);
        KEntryMap::EntryOptions options = KEntryMap::EntryOption::EntryDirty;
        if (entry.bDeleted) {
            options |= KEntryMap::EntryDeleted;
        }

        if (entry.bExpand) {
            options |= KEntryMap::EntryExpansion;
        }

        other.config()->d_ptr->setEntryData(other.name(), key, entry.mValue, options);
    }
}

void KConfigGroup::moveValuesTo(const QList<const char *> &keys, KConfigGroup &other, WriteConfigFlags pFlags)
{
    Q_ASSERT(isValid());
    Q_ASSERT(other.isValid());

    for (const auto key : keys) {
        moveValue(key, other, pFlags);
    }
}

void KConfigGroup::moveValuesTo(KConfigGroup &other, WriteConfigFlags pFlags)
{
    Q_ASSERT(isValid());
    Q_ASSERT(other.isValid());

    const QStringList keys = keyList();
    for (const QString &key : keys) {
        moveValue(key.toUtf8().constData(), other, pFlags);
    }
}
