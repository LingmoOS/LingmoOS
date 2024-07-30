/*
    This file is part of KOrganizer.
    SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcoreconfigskeleton.h"
#include "kcoreconfigskeleton_p.h"

#include <QUrl>

#include <algorithm>

static QString obscuredString(const QString &str)
{
    QString result;
    const QChar *unicode = str.unicode();
    for (int i = 0; i < str.length(); ++i) {
        // yes, no typo. can't encode ' ' or '!' because
        // they're the unicode BOM. stupid scrambling. stupid.
        result += (unicode[i].unicode() <= 0x21) ? unicode[i] : QChar(0x1001F - unicode[i].unicode());
    }

    return result;
}

KConfigSkeletonItemPrivate::~KConfigSkeletonItemPrivate() = default;

KConfigSkeletonItem::KConfigSkeletonItem(const QString &_group, const QString &_key)
    : mGroup(_group)
    , mKey(_key)
    , d_ptr(new KConfigSkeletonItemPrivate)
{
}

KConfigSkeletonItem::KConfigSkeletonItem(KConfigSkeletonItemPrivate &dd, const QString &_group, const QString &_key)
    : mGroup(_group)
    , mKey(_key)
    , d_ptr(&dd)
{
}

KConfigSkeletonItem::~KConfigSkeletonItem()
{
    delete d_ptr;
}

void KConfigSkeletonItem::setGroup(const QString &_group)
{
    mGroup = _group;
}

void KConfigSkeletonItem::setGroup(const KConfigGroup &cg)
{
    Q_D(KConfigSkeletonItem);
    d->mConfigGroup = cg;
}

KConfigGroup KConfigSkeletonItem::configGroup(KConfig *config) const
{
    Q_D(const KConfigSkeletonItem);
    if (d->mConfigGroup.isValid()) {
        return d->mConfigGroup;
    }
    return KConfigGroup(config, mGroup);
}

QString KConfigSkeletonItem::group() const
{
    return mGroup;
}

void KConfigSkeletonItem::setKey(const QString &_key)
{
    mKey = _key;
}

QString KConfigSkeletonItem::key() const
{
    return mKey;
}

void KConfigSkeletonItem::setName(const QString &_name)
{
    mName = _name;
}

QString KConfigSkeletonItem::name() const
{
    return mName;
}

void KConfigSkeletonItem::setLabel(const QString &l)
{
    Q_D(KConfigSkeletonItem);
    d->mLabel = l;
}

QString KConfigSkeletonItem::label() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mLabel;
}

void KConfigSkeletonItem::setToolTip(const QString &t)
{
    Q_D(KConfigSkeletonItem);
    d->mToolTip = t;
}

QString KConfigSkeletonItem::toolTip() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mToolTip;
}

void KConfigSkeletonItem::setWhatsThis(const QString &w)
{
    Q_D(KConfigSkeletonItem);
    d->mWhatsThis = w;
}

QString KConfigSkeletonItem::whatsThis() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mWhatsThis;
}

void KConfigSkeletonItem::setWriteFlags(KConfigBase::WriteConfigFlags flags)
{
    Q_D(KConfigSkeletonItem);
    d->mWriteFlags = flags;
}

KConfigBase::WriteConfigFlags KConfigSkeletonItem::writeFlags() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mWriteFlags;
}

QVariant KConfigSkeletonItem::minValue() const
{
    return QVariant();
}

QVariant KConfigSkeletonItem::maxValue() const
{
    return QVariant();
}

bool KConfigSkeletonItem::isImmutable() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mIsImmutable;
}

bool KConfigSkeletonItem::isDefault() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mIsDefaultImpl();
}

bool KConfigSkeletonItem::isSaveNeeded() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mIsSaveNeededImpl();
}

QVariant KConfigSkeletonItem::getDefault() const
{
    Q_D(const KConfigSkeletonItem);
    return d->mGetDefaultImpl();
}

void KConfigSkeletonItem::readImmutability(const KConfigGroup &group)
{
    Q_D(KConfigSkeletonItem);
    d->mIsImmutable = group.isEntryImmutable(mKey);
}

void KConfigSkeletonItem::setIsDefaultImpl(const std::function<bool()> &impl)
{
    Q_D(KConfigSkeletonItem);
    d->mIsDefaultImpl = impl;
}

void KConfigSkeletonItem::setIsSaveNeededImpl(const std::function<bool()> &impl)
{
    Q_D(KConfigSkeletonItem);
    d->mIsSaveNeededImpl = impl;
}

void KConfigSkeletonItem::setGetDefaultImpl(const std::function<QVariant()> &impl)
{
    Q_D(KConfigSkeletonItem);
    d->mGetDefaultImpl = impl;
}

KPropertySkeletonItem::KPropertySkeletonItem(QObject *object, const QByteArray &propertyName, const QVariant &defaultValue)
    : KConfigSkeletonItem(*new KPropertySkeletonItemPrivate(object, propertyName, defaultValue), {}, {})
{
    setIsDefaultImpl([this] {
        Q_D(const KPropertySkeletonItem);
        return d->mReference == d->mDefaultValue;
    });
    setIsSaveNeededImpl([this] {
        Q_D(const KPropertySkeletonItem);
        return d->mReference != d->mLoadedValue;
    });
    setGetDefaultImpl([this] {
        Q_D(const KPropertySkeletonItem);
        return d->mDefaultValue;
    });
}

QVariant KPropertySkeletonItem::property() const
{
    Q_D(const KPropertySkeletonItem);
    return d->mReference;
}

void KPropertySkeletonItem::setProperty(const QVariant &p)
{
    Q_D(KPropertySkeletonItem);
    if (d->mReference == p) {
        return;
    }
    d->mReference = p;
    if (d->mNotifyFunction) {
        d->mNotifyFunction();
    }
}

bool KPropertySkeletonItem::isEqual(const QVariant &p) const
{
    Q_D(const KPropertySkeletonItem);
    return d->mReference == p;
}

void KPropertySkeletonItem::readConfig(KConfig *)
{
    Q_D(KPropertySkeletonItem);
    setProperty(d->mObject->property(d->mPropertyName.constData()));
    d->mLoadedValue = d->mReference;
}

void KPropertySkeletonItem::writeConfig(KConfig *)
{
    Q_D(KPropertySkeletonItem);
    d->mObject->setProperty(d->mPropertyName.constData(), d->mReference);
    d->mLoadedValue = d->mReference;
}

void KPropertySkeletonItem::readDefault(KConfig *)
{
    Q_D(KPropertySkeletonItem);
    setProperty(d->mConstDefaultValue);
}

void KPropertySkeletonItem::setDefault()
{
    Q_D(KPropertySkeletonItem);
    setProperty(d->mDefaultValue);
}

void KPropertySkeletonItem::swapDefault()
{
    Q_D(KPropertySkeletonItem);
    if (d->mReference == d->mDefaultValue) {
        return;
    }
    std::swap(d->mReference, d->mDefaultValue);
    if (d->mNotifyFunction) {
        d->mNotifyFunction();
    }
}

void KPropertySkeletonItem::setNotifyFunction(const std::function<void()> &impl)
{
    Q_D(KPropertySkeletonItem);
    d->mNotifyFunction = impl;
}

KCoreConfigSkeleton::ItemString::ItemString(const QString &_group, const QString &_key, QString &reference, const QString &defaultValue, Type type)
    : KConfigSkeletonGenericItem<QString>(_group, _key, reference, defaultValue)
    , mType(type)
{
}

void KCoreConfigSkeleton::ItemString::writeConfig(KConfig *config)
{
    if (mReference != mLoadedValue) { // WABA: Is this test needed?
        KConfigGroup cg = configGroup(config);
        if ((mDefault == mReference) && !cg.hasDefault(mKey)) {
            cg.revertToDefault(mKey, writeFlags());
        } else if (mType == Path) {
            cg.writePathEntry(mKey, mReference, writeFlags());
        } else if (mType == Password) {
            cg.writeEntry(mKey, obscuredString(mReference), writeFlags());
        } else {
            cg.writeEntry(mKey, mReference, writeFlags());
        }
        mLoadedValue = mReference;
    }
}

void KCoreConfigSkeleton::ItemString::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);

    if (mType == Path) {
        mReference = cg.readPathEntry(mKey, mDefault);
    } else if (mType == Password) {
        QString val = cg.readEntry(mKey, obscuredString(mDefault));
        mReference = obscuredString(val);
    } else {
        mReference = cg.readEntry(mKey, mDefault);
    }

    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemString::setProperty(const QVariant &p)
{
    mReference = p.toString();
}

bool KCoreConfigSkeleton::ItemString::isEqual(const QVariant &v) const
{
    return mReference == v.toString();
}

QVariant KCoreConfigSkeleton::ItemString::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemPassword::ItemPassword(const QString &_group, const QString &_key, QString &reference, const QString &defaultValue)
    : ItemString(_group, _key, reference, defaultValue, Password)
{
}

KCoreConfigSkeleton::ItemPath::ItemPath(const QString &_group, const QString &_key, QString &reference, const QString &defaultValue)
    : ItemString(_group, _key, reference, defaultValue, Path)
{
}

KCoreConfigSkeleton::ItemUrl::ItemUrl(const QString &_group, const QString &_key, QUrl &reference, const QUrl &defaultValue)
    : KConfigSkeletonGenericItem<QUrl>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemUrl::writeConfig(KConfig *config)
{
    if (mReference != mLoadedValue) { // WABA: Is this test needed?
        KConfigGroup cg = configGroup(config);
        if ((mDefault == mReference) && !cg.hasDefault(mKey)) {
            cg.revertToDefault(mKey, writeFlags());
        } else {
            cg.writeEntry<QString>(mKey, mReference.toString(), writeFlags());
        }
        mLoadedValue = mReference;
    }
}

void KCoreConfigSkeleton::ItemUrl::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);

    mReference = QUrl(cg.readEntry<QString>(mKey, mDefault.toString()));
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemUrl::setProperty(const QVariant &p)
{
    mReference = qvariant_cast<QUrl>(p);
}

bool KCoreConfigSkeleton::ItemUrl::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<QUrl>(v);
}

QVariant KCoreConfigSkeleton::ItemUrl::property() const
{
    return QVariant::fromValue<QUrl>(mReference);
}

KCoreConfigSkeleton::ItemProperty::ItemProperty(const QString &_group, const QString &_key, QVariant &reference, const QVariant &defaultValue)
    : KConfigSkeletonGenericItem<QVariant>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemProperty::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemProperty::setProperty(const QVariant &p)
{
    mReference = p;
}

bool KCoreConfigSkeleton::ItemProperty::isEqual(const QVariant &v) const
{
    // this might cause problems if the QVariants are not of default types
    return mReference == v;
}

QVariant KCoreConfigSkeleton::ItemProperty::property() const
{
    return mReference;
}

KCoreConfigSkeleton::ItemBool::ItemBool(const QString &_group, const QString &_key, bool &reference, bool defaultValue)
    : KConfigSkeletonGenericItem<bool>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemBool::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemBool::setProperty(const QVariant &p)
{
    mReference = p.toBool();
}

bool KCoreConfigSkeleton::ItemBool::isEqual(const QVariant &v) const
{
    return mReference == v.toBool();
}

QVariant KCoreConfigSkeleton::ItemBool::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemInt::ItemInt(const QString &_group, const QString &_key, qint32 &reference, qint32 defaultValue)
    : KConfigSkeletonGenericItem<qint32>(_group, _key, reference, defaultValue)
    , mHasMin(false)
    , mHasMax(false)
{
}

void KCoreConfigSkeleton::ItemInt::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    if (mHasMin) {
        mReference = qMax(mReference, mMin);
    }
    if (mHasMax) {
        mReference = qMin(mReference, mMax);
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemInt::setProperty(const QVariant &p)
{
    mReference = p.toInt();
}

bool KCoreConfigSkeleton::ItemInt::isEqual(const QVariant &v) const
{
    return mReference == v.toInt();
}

QVariant KCoreConfigSkeleton::ItemInt::property() const
{
    return QVariant(mReference);
}

QVariant KCoreConfigSkeleton::ItemInt::minValue() const
{
    if (mHasMin) {
        return QVariant(mMin);
    }
    return QVariant();
}

QVariant KCoreConfigSkeleton::ItemInt::maxValue() const
{
    if (mHasMax) {
        return QVariant(mMax);
    }
    return QVariant();
}

void KCoreConfigSkeleton::ItemInt::setMinValue(qint32 v)
{
    mHasMin = true;
    mMin = v;
}

void KCoreConfigSkeleton::ItemInt::setMaxValue(qint32 v)
{
    mHasMax = true;
    mMax = v;
}

KCoreConfigSkeleton::ItemLongLong::ItemLongLong(const QString &_group, const QString &_key, qint64 &reference, qint64 defaultValue)
    : KConfigSkeletonGenericItem<qint64>(_group, _key, reference, defaultValue)
    , mHasMin(false)
    , mHasMax(false)
{
}

void KCoreConfigSkeleton::ItemLongLong::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    if (mHasMin) {
        mReference = qMax(mReference, mMin);
    }
    if (mHasMax) {
        mReference = qMin(mReference, mMax);
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemLongLong::setProperty(const QVariant &p)
{
    mReference = p.toLongLong();
}

bool KCoreConfigSkeleton::ItemLongLong::isEqual(const QVariant &v) const
{
    return mReference == v.toLongLong();
}

QVariant KCoreConfigSkeleton::ItemLongLong::property() const
{
    return QVariant(mReference);
}

QVariant KCoreConfigSkeleton::ItemLongLong::minValue() const
{
    if (mHasMin) {
        return QVariant(mMin);
    }
    return QVariant();
}

QVariant KCoreConfigSkeleton::ItemLongLong::maxValue() const
{
    if (mHasMax) {
        return QVariant(mMax);
    }
    return QVariant();
}

void KCoreConfigSkeleton::ItemLongLong::setMinValue(qint64 v)
{
    mHasMin = true;
    mMin = v;
}

void KCoreConfigSkeleton::ItemLongLong::setMaxValue(qint64 v)
{
    mHasMax = true;
    mMax = v;
}

QString KCoreConfigSkeleton::ItemEnum::valueForChoice(const QString &name) const
{
    for (auto it = mChoices.cbegin(); it != mChoices.cend(); ++it) {
        if (it->name == name) {
            return it->value.isEmpty() ? it->name : it->value;
        }
    }
    return name;
}

void KCoreConfigSkeleton::ItemEnum::setValueForChoice(const QString &name, const QString &value)
{
    for (auto it = mChoices.begin(); it != mChoices.end(); ++it) {
        if (it->name == name) {
            it->value = value;
            return;
        }
    }
}

KCoreConfigSkeleton::ItemEnum::ItemEnum(const QString &_group, const QString &_key, qint32 &reference, const QList<Choice> &choices, qint32 defaultValue)
    : ItemInt(_group, _key, reference, defaultValue)
    , mChoices(choices)
{
}

void KCoreConfigSkeleton::ItemEnum::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    if (!cg.hasKey(mKey)) {
        mReference = mDefault;
    } else {
        int i = 0;
        mReference = -1;
        const QString entryString = cg.readEntry(mKey, QString());
        for (auto it = mChoices.cbegin(); it != mChoices.cend(); ++it, ++i) {
            QString choiceName = (*it).name;
            if (valueForChoice(choiceName).compare(entryString, Qt::CaseInsensitive) == 0) {
                mReference = i;
                break;
            }
        }
        if (mReference == -1) {
            mReference = cg.readEntry(mKey, mDefault);
        }
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemEnum::writeConfig(KConfig *config)
{
    if (mReference != mLoadedValue) { // WABA: Is this test needed?
        KConfigGroup cg = configGroup(config);
        if ((mDefault == mReference) && !cg.hasDefault(mKey)) {
            cg.revertToDefault(mKey, writeFlags());
        } else if ((mReference >= 0) && (mReference < mChoices.count())) {
            cg.writeEntry(mKey, valueForChoice(mChoices.at(mReference).name), writeFlags());
        } else {
            cg.writeEntry(mKey, mReference, writeFlags());
        }
        mLoadedValue = mReference;
    }
}

QList<KCoreConfigSkeleton::ItemEnum::Choice> KCoreConfigSkeleton::ItemEnum::choices() const
{
    return mChoices;
}

KCoreConfigSkeleton::ItemUInt::ItemUInt(const QString &_group, const QString &_key, quint32 &reference, quint32 defaultValue)
    : KConfigSkeletonGenericItem<quint32>(_group, _key, reference, defaultValue)
    , mHasMin(false)
    , mHasMax(false)
{
}

void KCoreConfigSkeleton::ItemUInt::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    if (mHasMin) {
        mReference = qMax(mReference, mMin);
    }
    if (mHasMax) {
        mReference = qMin(mReference, mMax);
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemUInt::setProperty(const QVariant &p)
{
    mReference = p.toUInt();
}

bool KCoreConfigSkeleton::ItemUInt::isEqual(const QVariant &v) const
{
    return mReference == v.toUInt();
}

QVariant KCoreConfigSkeleton::ItemUInt::property() const
{
    return QVariant(mReference);
}

QVariant KCoreConfigSkeleton::ItemUInt::minValue() const
{
    if (mHasMin) {
        return QVariant(mMin);
    }
    return QVariant();
}

QVariant KCoreConfigSkeleton::ItemUInt::maxValue() const
{
    if (mHasMax) {
        return QVariant(mMax);
    }
    return QVariant();
}

void KCoreConfigSkeleton::ItemUInt::setMinValue(quint32 v)
{
    mHasMin = true;
    mMin = v;
}

void KCoreConfigSkeleton::ItemUInt::setMaxValue(quint32 v)
{
    mHasMax = true;
    mMax = v;
}

KCoreConfigSkeleton::ItemULongLong::ItemULongLong(const QString &_group, const QString &_key, quint64 &reference, quint64 defaultValue)
    : KConfigSkeletonGenericItem<quint64>(_group, _key, reference, defaultValue)
    , mHasMin(false)
    , mHasMax(false)
{
}

void KCoreConfigSkeleton::ItemULongLong::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    if (mHasMin) {
        mReference = qMax(mReference, mMin);
    }
    if (mHasMax) {
        mReference = qMin(mReference, mMax);
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemULongLong::setProperty(const QVariant &p)
{
    mReference = p.toULongLong();
}

bool KCoreConfigSkeleton::ItemULongLong::isEqual(const QVariant &v) const
{
    return mReference == v.toULongLong();
}

QVariant KCoreConfigSkeleton::ItemULongLong::property() const
{
    return QVariant(mReference);
}

QVariant KCoreConfigSkeleton::ItemULongLong::minValue() const
{
    if (mHasMin) {
        return QVariant(mMin);
    }
    return QVariant();
}

QVariant KCoreConfigSkeleton::ItemULongLong::maxValue() const
{
    if (mHasMax) {
        return QVariant(mMax);
    }
    return QVariant();
}

void KCoreConfigSkeleton::ItemULongLong::setMinValue(quint64 v)
{
    mHasMin = true;
    mMin = v;
}

void KCoreConfigSkeleton::ItemULongLong::setMaxValue(quint64 v)
{
    mHasMax = true;
    mMax = v;
}

KCoreConfigSkeleton::ItemDouble::ItemDouble(const QString &_group, const QString &_key, double &reference, double defaultValue)
    : KConfigSkeletonGenericItem<double>(_group, _key, reference, defaultValue)
    , mHasMin(false)
    , mHasMax(false)
{
}

void KCoreConfigSkeleton::ItemDouble::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    if (mHasMin) {
        mReference = qMax(mReference, mMin);
    }
    if (mHasMax) {
        mReference = qMin(mReference, mMax);
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemDouble::setProperty(const QVariant &p)
{
    mReference = p.toDouble();
}

bool KCoreConfigSkeleton::ItemDouble::isEqual(const QVariant &v) const
{
    return mReference == v.toDouble();
}

QVariant KCoreConfigSkeleton::ItemDouble::property() const
{
    return QVariant(mReference);
}

QVariant KCoreConfigSkeleton::ItemDouble::minValue() const
{
    if (mHasMin) {
        return QVariant(mMin);
    }
    return QVariant();
}

QVariant KCoreConfigSkeleton::ItemDouble::maxValue() const
{
    if (mHasMax) {
        return QVariant(mMax);
    }
    return QVariant();
}

void KCoreConfigSkeleton::ItemDouble::setMinValue(double v)
{
    mHasMin = true;
    mMin = v;
}

void KCoreConfigSkeleton::ItemDouble::setMaxValue(double v)
{
    mHasMax = true;
    mMax = v;
}

KCoreConfigSkeleton::ItemRect::ItemRect(const QString &_group, const QString &_key, QRect &reference, const QRect &defaultValue)
    : KConfigSkeletonGenericItem<QRect>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemRect::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemRect::setProperty(const QVariant &p)
{
    mReference = p.toRect();
}

bool KCoreConfigSkeleton::ItemRect::isEqual(const QVariant &v) const
{
    return mReference == v.toRect();
}

QVariant KCoreConfigSkeleton::ItemRect::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemRectF::ItemRectF(const QString &_group, const QString &_key, QRectF &reference, const QRectF &defaultValue)
    : KConfigSkeletonGenericItem<QRectF>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemRectF::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemRectF::setProperty(const QVariant &p)
{
    mReference = p.toRectF();
}

bool KCoreConfigSkeleton::ItemRectF::isEqual(const QVariant &v) const
{
    return mReference == v.toRectF();
}

QVariant KCoreConfigSkeleton::ItemRectF::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemPoint::ItemPoint(const QString &_group, const QString &_key, QPoint &reference, const QPoint &defaultValue)
    : KConfigSkeletonGenericItem<QPoint>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemPoint::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemPoint::setProperty(const QVariant &p)
{
    mReference = p.toPoint();
}

bool KCoreConfigSkeleton::ItemPoint::isEqual(const QVariant &v) const
{
    return mReference == v.toPoint();
}

QVariant KCoreConfigSkeleton::ItemPoint::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemPointF::ItemPointF(const QString &_group, const QString &_key, QPointF &reference, const QPointF &defaultValue)
    : KConfigSkeletonGenericItem<QPointF>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemPointF::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemPointF::setProperty(const QVariant &p)
{
    mReference = p.toPointF();
}

bool KCoreConfigSkeleton::ItemPointF::isEqual(const QVariant &v) const
{
    return mReference == v.toPointF();
}

QVariant KCoreConfigSkeleton::ItemPointF::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemSize::ItemSize(const QString &_group, const QString &_key, QSize &reference, const QSize &defaultValue)
    : KConfigSkeletonGenericItem<QSize>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemSize::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemSize::setProperty(const QVariant &p)
{
    mReference = p.toSize();
}

bool KCoreConfigSkeleton::ItemSize::isEqual(const QVariant &v) const
{
    return mReference == v.toSize();
}

QVariant KCoreConfigSkeleton::ItemSize::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemSizeF::ItemSizeF(const QString &_group, const QString &_key, QSizeF &reference, const QSizeF &defaultValue)
    : KConfigSkeletonGenericItem<QSizeF>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemSizeF::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemSizeF::setProperty(const QVariant &p)
{
    mReference = p.toSizeF();
}

bool KCoreConfigSkeleton::ItemSizeF::isEqual(const QVariant &v) const
{
    return mReference == v.toSizeF();
}

QVariant KCoreConfigSkeleton::ItemSizeF::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemDateTime::ItemDateTime(const QString &_group, const QString &_key, QDateTime &reference, const QDateTime &defaultValue)
    : KConfigSkeletonGenericItem<QDateTime>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemDateTime::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemDateTime::setProperty(const QVariant &p)
{
    mReference = p.toDateTime();
}

bool KCoreConfigSkeleton::ItemDateTime::isEqual(const QVariant &v) const
{
    return mReference == v.toDateTime();
}

QVariant KCoreConfigSkeleton::ItemDateTime::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemStringList::ItemStringList(const QString &_group, const QString &_key, QStringList &reference, const QStringList &defaultValue)
    : KConfigSkeletonGenericItem<QStringList>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemStringList::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    if (!cg.hasKey(mKey)) {
        mReference = mDefault;
    } else {
        mReference = cg.readEntry(mKey, mDefault);
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemStringList::setProperty(const QVariant &p)
{
    mReference = p.toStringList();
}

bool KCoreConfigSkeleton::ItemStringList::isEqual(const QVariant &v) const
{
    return mReference == v.toStringList();
}

QVariant KCoreConfigSkeleton::ItemStringList::property() const
{
    return QVariant(mReference);
}

KCoreConfigSkeleton::ItemPathList::ItemPathList(const QString &_group, const QString &_key, QStringList &reference, const QStringList &defaultValue)
    : ItemStringList(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemPathList::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    if (!cg.hasKey(mKey)) {
        mReference = mDefault;
    } else {
        mReference = cg.readPathEntry(mKey, QStringList());
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemPathList::writeConfig(KConfig *config)
{
    if (mReference != mLoadedValue) { // WABA: Is this test needed?
        KConfigGroup cg = configGroup(config);
        if ((mDefault == mReference) && !cg.hasDefault(mKey)) {
            cg.revertToDefault(mKey, writeFlags());
        } else {
            QStringList sl = mReference;
            cg.writePathEntry(mKey, sl, writeFlags());
        }
        mLoadedValue = mReference;
    }
}

KCoreConfigSkeleton::ItemUrlList::ItemUrlList(const QString &_group, const QString &_key, QList<QUrl> &reference, const QList<QUrl> &defaultValue)
    : KConfigSkeletonGenericItem<QList<QUrl>>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemUrlList::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    if (!cg.hasKey(mKey)) {
        mReference = mDefault;
    } else {
        QStringList strList;
        for (const QUrl &url : std::as_const(mDefault)) {
            strList.append(url.toString());
        }
        mReference.clear();
        const QStringList readList = cg.readEntry<QStringList>(mKey, strList);
        for (const QString &str : readList) {
            mReference.append(QUrl(str));
        }
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemUrlList::writeConfig(KConfig *config)
{
    if (mReference != mLoadedValue) { // WABA: Is this test needed?
        KConfigGroup cg = configGroup(config);
        if ((mDefault == mReference) && !cg.hasDefault(mKey)) {
            cg.revertToDefault(mKey, writeFlags());
        } else {
            QStringList strList;
            for (const QUrl &url : std::as_const(mReference)) {
                strList.append(url.toString());
            }
            cg.writeEntry<QStringList>(mKey, strList, writeFlags());
        }
        mLoadedValue = mReference;
    }
}

void KCoreConfigSkeleton::ItemUrlList::setProperty(const QVariant &p)
{
    mReference = qvariant_cast<QList<QUrl>>(p);
}

bool KCoreConfigSkeleton::ItemUrlList::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<QList<QUrl>>(v);
}

QVariant KCoreConfigSkeleton::ItemUrlList::property() const
{
    return QVariant::fromValue<QList<QUrl>>(mReference);
}

KCoreConfigSkeleton::ItemIntList::ItemIntList(const QString &_group, const QString &_key, QList<int> &reference, const QList<int> &defaultValue)
    : KConfigSkeletonGenericItem<QList<int>>(_group, _key, reference, defaultValue)
{
}

void KCoreConfigSkeleton::ItemIntList::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);
    if (!cg.hasKey(mKey)) {
        mReference = mDefault;
    } else {
        mReference = cg.readEntry(mKey, mDefault);
    }
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KCoreConfigSkeleton::ItemIntList::setProperty(const QVariant &p)
{
    mReference = qvariant_cast<QList<int>>(p);
}

bool KCoreConfigSkeleton::ItemIntList::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<QList<int>>(v);
}

QVariant KCoreConfigSkeleton::ItemIntList::property() const
{
    return QVariant::fromValue<QList<int>>(mReference);
}

// static int kCoreConfigSkeletionDebugArea() { static int s_area = KDebug::registerArea("kdecore (KConfigSkeleton)"); return s_area; }

KCoreConfigSkeleton::KCoreConfigSkeleton(const QString &configname, QObject *parent)
    : QObject(parent)
    , d(new KCoreConfigSkeletonPrivate)
{
    // qDebug() << "Creating KCoreConfigSkeleton (" << (void *)this << ")";

    d->mConfig = KSharedConfig::openConfig(configname, KConfig::FullConfig);
}

KCoreConfigSkeleton::KCoreConfigSkeleton(KSharedConfig::Ptr pConfig, QObject *parent)
    : QObject(parent)
    , d(new KCoreConfigSkeletonPrivate)
{
    // qDebug() << "Creating KCoreConfigSkeleton (" << (void *)this << ")";
    d->mConfig = std::move(pConfig);
}

KCoreConfigSkeleton::~KCoreConfigSkeleton()
{
    delete d;
}

void KCoreConfigSkeleton::setCurrentGroup(const QString &group)
{
    d->mCurrentGroup = group;
}

QString KCoreConfigSkeleton::currentGroup() const
{
    return d->mCurrentGroup;
}

KConfig *KCoreConfigSkeleton::config()
{
    return d->mConfig.data();
}

const KConfig *KCoreConfigSkeleton::config() const
{
    return d->mConfig.data();
}

KSharedConfig::Ptr KCoreConfigSkeleton::sharedConfig() const
{
    return d->mConfig;
}

void KCoreConfigSkeleton::setSharedConfig(KSharedConfig::Ptr pConfig)
{
    d->mConfig = std::move(pConfig);
}

KConfigSkeletonItem::List KCoreConfigSkeleton::items() const
{
    return d->mItems;
}

bool KCoreConfigSkeleton::useDefaults(bool b)
{
    if (b == d->mUseDefaults) {
        return d->mUseDefaults;
    }

    d->mUseDefaults = b;
    for (auto *skelItem : std::as_const(d->mItems)) {
        skelItem->swapDefault();
    }

    usrUseDefaults(b);
    return !d->mUseDefaults;
}

void KCoreConfigSkeleton::setDefaults()
{
    for (auto *skelItem : std::as_const(d->mItems)) {
        skelItem->setDefault();
    }
    usrSetDefaults();
}

void KCoreConfigSkeleton::load()
{
    d->mConfig->reparseConfiguration();
    read();
}

void KCoreConfigSkeleton::read()
{
    for (auto *skelItem : std::as_const(d->mItems)) {
        skelItem->readConfig(d->mConfig.data());
    }
    usrRead();
}

bool KCoreConfigSkeleton::isDefaults() const
{
    return std::all_of(d->mItems.cbegin(), d->mItems.cend(), [](KConfigSkeletonItem *skelItem) {
        return skelItem->isDefault();
    });
}

bool KCoreConfigSkeleton::isSaveNeeded() const
{
    return std::any_of(d->mItems.cbegin(), d->mItems.cend(), [](KConfigSkeletonItem *skelItem) {
        return skelItem->isSaveNeeded();
    });
}

bool KCoreConfigSkeleton::save()
{
    // qDebug();
    for (auto *skelItem : std::as_const(d->mItems)) {
        skelItem->writeConfig(d->mConfig.data());
    }

    if (!usrSave()) {
        return false;
    }

    if (d->mConfig->isDirty()) {
        if (!d->mConfig->sync()) {
            return false;
        }
        Q_EMIT configChanged();
    }
    return true;
}

bool KCoreConfigSkeleton::usrUseDefaults(bool)
{
    return false;
}

void KCoreConfigSkeleton::usrSetDefaults()
{
}

void KCoreConfigSkeleton::usrRead()
{
}

bool KCoreConfigSkeleton::usrSave()
{
    return true;
}

void KCoreConfigSkeleton::addItem(KConfigSkeletonItem *item, const QString &name)
{
    if (d->mItems.contains(item)) {
        if (item->name() == name || (name.isEmpty() && item->name() == item->key())) {
            // nothing to do -> it is already in our collection
            // and the name isn't changing
            return;
        }

        d->mItemDict.remove(item->name());
    } else {
        d->mItems.append(item);
    }

    item->setName(name.isEmpty() ? item->key() : name);
    d->mItemDict.insert(item->name(), item);
    item->readDefault(d->mConfig.data());
    item->readConfig(d->mConfig.data());
}

void KCoreConfigSkeleton::removeItem(const QString &name)
{
    KConfigSkeletonItem *item = d->mItemDict.value(name);
    if (item) {
        d->mItems.removeAll(item);
        d->mItemDict.remove(item->name());
        delete item;
    }
}

void KCoreConfigSkeleton::clearItems()
{
    KConfigSkeletonItem::List items = d->mItems;
    d->mItems.clear();
    d->mItemDict.clear();
    qDeleteAll(items);
}

KCoreConfigSkeleton::ItemString *KCoreConfigSkeleton::addItemString(const QString &name, QString &reference, const QString &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemString *item;
    item = new KCoreConfigSkeleton::ItemString(d->mCurrentGroup, key.isEmpty() ? name : key, reference, defaultValue, KCoreConfigSkeleton::ItemString::Normal);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemPassword *
KCoreConfigSkeleton::addItemPassword(const QString &name, QString &reference, const QString &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemPassword *item;
    item = new KCoreConfigSkeleton::ItemPassword(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemPath *KCoreConfigSkeleton::addItemPath(const QString &name, QString &reference, const QString &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemPath *item;
    item = new KCoreConfigSkeleton::ItemPath(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemProperty *
KCoreConfigSkeleton::addItemProperty(const QString &name, QVariant &reference, const QVariant &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemProperty *item;
    item = new KCoreConfigSkeleton::ItemProperty(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemBool *KCoreConfigSkeleton::addItemBool(const QString &name, bool &reference, bool defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemBool *item;
    item = new KCoreConfigSkeleton::ItemBool(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemInt *KCoreConfigSkeleton::addItemInt(const QString &name, qint32 &reference, qint32 defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemInt *item;
    item = new KCoreConfigSkeleton::ItemInt(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemUInt *KCoreConfigSkeleton::addItemUInt(const QString &name, quint32 &reference, quint32 defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemUInt *item;
    item = new KCoreConfigSkeleton::ItemUInt(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemLongLong *KCoreConfigSkeleton::addItemLongLong(const QString &name, qint64 &reference, qint64 defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemLongLong *item;
    item = new KCoreConfigSkeleton::ItemLongLong(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemULongLong *KCoreConfigSkeleton::addItemULongLong(const QString &name, quint64 &reference, quint64 defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemULongLong *item;
    item = new KCoreConfigSkeleton::ItemULongLong(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemDouble *KCoreConfigSkeleton::addItemDouble(const QString &name, double &reference, double defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemDouble *item;
    item = new KCoreConfigSkeleton::ItemDouble(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemRect *KCoreConfigSkeleton::addItemRect(const QString &name, QRect &reference, const QRect &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemRect *item;
    item = new KCoreConfigSkeleton::ItemRect(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemRectF *KCoreConfigSkeleton::addItemRectF(const QString &name, QRectF &reference, const QRectF &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemRectF *item;
    item = new KCoreConfigSkeleton::ItemRectF(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemPoint *KCoreConfigSkeleton::addItemPoint(const QString &name, QPoint &reference, const QPoint &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemPoint *item;
    item = new KCoreConfigSkeleton::ItemPoint(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemPointF *KCoreConfigSkeleton::addItemPointF(const QString &name, QPointF &reference, const QPointF &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemPointF *item;
    item = new KCoreConfigSkeleton::ItemPointF(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemSize *KCoreConfigSkeleton::addItemSize(const QString &name, QSize &reference, const QSize &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemSize *item;
    item = new KCoreConfigSkeleton::ItemSize(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemSizeF *KCoreConfigSkeleton::addItemSizeF(const QString &name, QSizeF &reference, const QSizeF &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemSizeF *item;
    item = new KCoreConfigSkeleton::ItemSizeF(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemDateTime *
KCoreConfigSkeleton::addItemDateTime(const QString &name, QDateTime &reference, const QDateTime &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemDateTime *item;
    item = new KCoreConfigSkeleton::ItemDateTime(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemStringList *
KCoreConfigSkeleton::addItemStringList(const QString &name, QStringList &reference, const QStringList &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemStringList *item;
    item = new KCoreConfigSkeleton::ItemStringList(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KCoreConfigSkeleton::ItemIntList *
KCoreConfigSkeleton::addItemIntList(const QString &name, QList<int> &reference, const QList<int> &defaultValue, const QString &key)
{
    KCoreConfigSkeleton::ItemIntList *item;
    item = new KCoreConfigSkeleton::ItemIntList(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

bool KCoreConfigSkeleton::isImmutable(const QString &name) const
{
    KConfigSkeletonItem *item = findItem(name);
    return !item || item->isImmutable();
}

KConfigSkeletonItem *KCoreConfigSkeleton::findItem(const QString &name) const
{
    return d->mItemDict.value(name);
}

KConfigCompilerSignallingItem::KConfigCompilerSignallingItem(KConfigSkeletonItem *item,
                                                             QObject *object,
                                                             KConfigCompilerSignallingItem::NotifyFunction targetFunction,
                                                             quint64 userData)
    : KConfigSkeletonItem(item->group(), item->key())
    , mItem(item)
    , mTargetFunction(targetFunction)
    , mObject(object)
    , mUserData(userData)
{
    Q_ASSERT(mTargetFunction);
    Q_ASSERT(mItem);
    Q_ASSERT(mObject);

    setIsDefaultImpl([this] {
        return mItem->isDefault();
    });
    setIsSaveNeededImpl([this] {
        return mItem->isSaveNeeded();
    });
    setGetDefaultImpl([this] {
        return mItem->getDefault();
    });
}

KConfigCompilerSignallingItem::~KConfigCompilerSignallingItem()
{
}

bool KConfigCompilerSignallingItem::isEqual(const QVariant &p) const
{
    return mItem->isEqual(p);
}

QVariant KConfigCompilerSignallingItem::property() const
{
    return mItem->property();
}

QVariant KConfigCompilerSignallingItem::minValue() const
{
    return mItem->minValue();
}

QVariant KConfigCompilerSignallingItem::maxValue() const
{
    return mItem->maxValue();
}

void KConfigCompilerSignallingItem::readConfig(KConfig *c)
{
    QVariant oldValue = mItem->property();
    mItem->readConfig(c);
    // readConfig() changes mIsImmutable, update it here as well
    KConfigGroup cg = configGroup(c);
    readImmutability(cg);
    if (!mItem->isEqual(oldValue)) {
        invokeNotifyFunction();
    }
}

void KConfigCompilerSignallingItem::readDefault(KConfig *c)
{
    mItem->readDefault(c);
    // readDefault() changes mIsImmutable, update it here as well
    KConfigGroup cg = configGroup(c);
    readImmutability(cg);
}

void KConfigCompilerSignallingItem::writeConfig(KConfig *c)
{
    mItem->writeConfig(c);
}

void KConfigCompilerSignallingItem::setDefault()
{
    QVariant oldValue = mItem->property();
    mItem->setDefault();
    if (!mItem->isEqual(oldValue)) {
        invokeNotifyFunction();
    }
}

void KConfigCompilerSignallingItem::setProperty(const QVariant &p)
{
    if (!mItem->isEqual(p)) {
        mItem->setProperty(p);
        invokeNotifyFunction();
    }
}

void KConfigCompilerSignallingItem::swapDefault()
{
    QVariant oldValue = mItem->property();
    mItem->swapDefault();
    if (!mItem->isEqual(oldValue)) {
        invokeNotifyFunction();
    }
}

void KConfigCompilerSignallingItem::setWriteFlags(KConfigBase::WriteConfigFlags flags)
{
    mItem->setWriteFlags(flags);
}

KConfigBase::WriteConfigFlags KConfigCompilerSignallingItem::writeFlags() const
{
    return mItem->writeFlags();
}

void KConfigCompilerSignallingItem::setGroup(const KConfigGroup &cg)
{
    mItem->setGroup(cg);
}

KConfigGroup KConfigCompilerSignallingItem::configGroup(KConfig *config) const
{
    return mItem->configGroup(config);
}

#include "moc_kcoreconfigskeleton.cpp"
