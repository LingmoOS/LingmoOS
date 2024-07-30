/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kwalletfreedesktopattributes.h"

#include "kwalletd.h"
#include "kwalletd_debug.h"
#include "kwalletfreedesktopcollection.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>

KWalletFreedesktopAttributes::KWalletFreedesktopAttributes(const QString &walletName)
{
    const QString writeLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kwalletd");
    _path = writeLocation + QChar::fromLatin1('/') + KWalletD::encodeWalletName(walletName) + QStringLiteral("_attributes.json");

    read();

    if (!_params.contains(FDO_KEY_CREATED)) {
        const auto currentTime = QString::number(QDateTime::currentSecsSinceEpoch());
        _params[FDO_KEY_CREATED] = currentTime;
        _params[FDO_KEY_MODIFIED] = currentTime;
    }
}

void KWalletFreedesktopAttributes::read()
{
    QByteArray content;
    {
        QFile file(_path);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        if (!file.isOpen()) {
            qCDebug(KWALLETD_LOG) << "Can't read attributes file " << _path;
            return;
        }
        content = file.readAll();
    }

    const auto jsonDoc = QJsonDocument::fromJson(content);
    if (jsonDoc.isObject()) {
        _params = jsonDoc.object();
    } else {
        qCWarning(KWALLETD_LOG) << "Can't read attributes: the root element must be an JSON-object: " << _path;
        _params = QJsonObject();
    }
}

void KWalletFreedesktopAttributes::write()
{
    if (_params.empty()) {
        QFile::remove(_path);
        return;
    }

    updateLastModified();

    QSaveFile sf(_path);
    if (!sf.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qCWarning(KWALLETD_LOG) << "Can't write attributes file: " << _path;
        return;
    }
    sf.setPermissions(QSaveFile::ReadUser | QSaveFile::WriteUser);

    const QJsonDocument saveDoc(_params);

    const QByteArray jsonBytes = saveDoc.toJson();
    if (sf.write(jsonBytes) != jsonBytes.size()) {
        sf.cancelWriting();
        qCWarning(KWALLETD_LOG) << "Cannot write attributes file " << _path;
        return;
    }
    if (!sf.commit()) {
        qCWarning(KWALLETD_LOG) << "Cannot commit attributes file " << _path;
    }
}

static QString entryLocationToStr(const EntryLocation &entryLocation)
{
    return entryLocation.folder + QChar::fromLatin1('/') + entryLocation.key;
}

static EntryLocation splitToEntryLocation(const QString &entryLocation)
{
    const int slashPos = entryLocation.indexOf(QChar::fromLatin1('/'));
    if (slashPos == -1) {
        qCWarning(KWALLETD_LOG) << "Entry location '" << entryLocation << "' has no slash '/'";
        return {};
    } else {
        return {entryLocation.left(slashPos), entryLocation.right((entryLocation.size() - slashPos) - 1)};
    }
}

void KWalletFreedesktopAttributes::remove(const EntryLocation &entryLocation)
{
    _params.remove(entryLocationToStr(entryLocation));
    if (_params.empty()) {
        QFile::remove(_path);
    } else {
        write();
    }
}

void KWalletFreedesktopAttributes::deleteFile()
{
    QFile::remove(_path);
}

void KWalletFreedesktopAttributes::renameLabel(const EntryLocation &oldLocation, const EntryLocation &newLocation)
{
    const QString oldLoc = entryLocationToStr(oldLocation);

    const auto found = _params.find(oldLoc);
    if (found == _params.end() || !found->isObject()) {
        qCWarning(KWALLETD_LOG) << "Can't rename label (!?)";
        return;
    }
    const auto obj = found->toObject();
    _params.erase(found);
    _params.insert(entryLocationToStr(newLocation), obj);

    write();
}

void KWalletFreedesktopAttributes::renameWallet(const QString &newName)
{
    const QString writeLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kwalletd");
    const QString newPath = writeLocation + QChar::fromLatin1('/') + newName + QStringLiteral("_attributes.json");

    QFile::rename(_path, newPath);
    _path = newPath;
}

void KWalletFreedesktopAttributes::newItem(const EntryLocation &entryLocation)
{
    _params[entryLocationToStr(entryLocation)] = QJsonObject();
}

QList<EntryLocation> KWalletFreedesktopAttributes::matchAttributes(const StrStrMap &attributes) const
{
    QList<EntryLocation> items;

    for (auto i = _params.constBegin(); i != _params.constEnd(); ++i) {
        if (!i->isObject()) {
            continue;
        }

        bool match = true;
        const auto itemParams = i->toObject();
        const auto foundItemAttribs = itemParams.find(QStringLiteral("attributes"));
        if (foundItemAttribs == itemParams.end() || !foundItemAttribs->isObject()) {
            continue;
        }
        const auto itemAttribs = foundItemAttribs->toObject();

        for (auto i = attributes.constBegin(); i != attributes.constEnd(); ++i) {
            const auto foundKey = itemAttribs.find(i.key());
            if (foundKey == itemAttribs.end() || !foundKey->isString() || foundKey->toString() != i.value()) {
                match = false;
                break;
            }
        }

        if (match) {
            items += splitToEntryLocation(i.key());
        }
    }

    return items;
}

void KWalletFreedesktopAttributes::setAttributes(const EntryLocation &entryLocation, const StrStrMap &attributes)
{
    QJsonObject jsonAttrs;
    for (auto i = attributes.constBegin(); i != attributes.constEnd(); ++i) {
        jsonAttrs.insert(i.key(), i.value());
    }

    const QString strLocation = entryLocationToStr(entryLocation);

    const auto foundParams = _params.find(strLocation);
    QJsonObject params;
    if (foundParams != _params.end() && foundParams->isObject()) {
        params = foundParams->toObject();
    } else {
        return;
    }

    if (jsonAttrs.empty()) {
        params.remove(QStringLiteral("attributes"));
    } else {
        params[QStringLiteral("attributes")] = jsonAttrs;
    }

    _params[strLocation] = params;

    write();
}

StrStrMap KWalletFreedesktopAttributes::getAttributes(const EntryLocation &entryLocation) const
{
    const auto foundObj = _params.find(entryLocationToStr(entryLocation));
    if (foundObj == _params.end() || !foundObj->isObject()) {
        return StrStrMap();
    }
    const auto jsonParams = foundObj->toObject();

    const auto foundAttrs = jsonParams.find(QStringLiteral("attributes"));
    if (foundAttrs == jsonParams.end() || !foundAttrs->isObject()) {
        return StrStrMap();
    }
    const auto jsonAttrs = foundAttrs->toObject();

    StrStrMap itemAttrs;

    for (auto i = jsonAttrs.constBegin(); i != jsonAttrs.constEnd(); ++i) {
        if (i.value().isString()) {
            itemAttrs.insert(i.key(), i.value().toString());
        }
    }

    return itemAttrs;
}

QString KWalletFreedesktopAttributes::getStringParam(const EntryLocation &entryLocation, const QString &paramName, const QString &defaultParam) const
{
    const auto foundParams = _params.find(entryLocationToStr(entryLocation));
    if (foundParams == _params.end() || !foundParams->isObject()) {
        return defaultParam;
    }
    const auto params = foundParams->toObject();

    const auto foundParam = params.find(paramName);
    if (foundParam == params.end() || !foundParam->isString()) {
        return defaultParam;
    }

    return foundParam->toString();
}

qulonglong KWalletFreedesktopAttributes::getULongLongParam(const EntryLocation &entryLocation, const QString &paramName, qulonglong defaultParam) const
{
    const auto str = getStringParam(entryLocation, paramName, QString::number(defaultParam));
    bool ok = false;
    const auto result = str.toULongLong(&ok);
    return ok ? result : defaultParam;
}

void KWalletFreedesktopAttributes::setParam(const EntryLocation &entryLocation, const QString &paramName, const QString &param)
{
    const auto entryLoc = entryLocationToStr(entryLocation);
    const auto foundParams = _params.find(entryLoc);
    if (foundParams == _params.end() || !foundParams->isObject()) {
        return;
    }

    auto params = foundParams->toObject();

    params[paramName] = param;
    _params[entryLoc] = params;

    write();
}

void KWalletFreedesktopAttributes::setParam(const EntryLocation &entryLocation, const QString &paramName, qulonglong param)
{
    setParam(entryLocation, paramName, QString::number(param));
}

void KWalletFreedesktopAttributes::remove(const FdoUniqueLabel &itemUniqLabel)
{
    remove(itemUniqLabel.toEntryLocation());
}

void KWalletFreedesktopAttributes::setAttributes(const FdoUniqueLabel &itemUniqLabel, const StrStrMap &attributes)
{
    setAttributes(itemUniqLabel.toEntryLocation(), attributes);
}

StrStrMap KWalletFreedesktopAttributes::getAttributes(const FdoUniqueLabel &itemUniqLabel) const
{
    return getAttributes(itemUniqLabel.toEntryLocation());
}

QString KWalletFreedesktopAttributes::getStringParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, const QString &defaultParam) const
{
    return getStringParam(itemUniqLabel.toEntryLocation(), paramName, defaultParam);
}

qulonglong KWalletFreedesktopAttributes::getULongLongParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, qulonglong defaultParam) const
{
    return getULongLongParam(itemUniqLabel.toEntryLocation(), paramName, defaultParam);
}

void KWalletFreedesktopAttributes::setParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, const QString &param)
{
    setParam(itemUniqLabel.toEntryLocation(), paramName, param);
}

void KWalletFreedesktopAttributes::setParam(const FdoUniqueLabel &itemUniqLabel, const QString &paramName, qulonglong param)
{
    setParam(itemUniqLabel.toEntryLocation(), paramName, param);
}

QList<EntryLocation> KWalletFreedesktopAttributes::listItems() const
{
    QList<EntryLocation> items;
    for (auto i = _params.constBegin(); i != _params.constEnd(); ++i) {
        if (i->isObject()) {
            items.push_back(splitToEntryLocation(i.key()));
        }
    }
    return items;
}

qulonglong KWalletFreedesktopAttributes::lastModified() const
{
    auto found = _params.constFind(FDO_KEY_MODIFIED);
    if (found == _params.constEnd()) {
        return 0;
    }
    return found->toString().toULongLong();
}

qulonglong KWalletFreedesktopAttributes::birthTime() const
{
    auto found = _params.constFind(FDO_KEY_CREATED);
    if (found == _params.constEnd()) {
        return 0;
    }
    return found->toString().toULongLong();
}

void KWalletFreedesktopAttributes::updateLastModified()
{
    _params[FDO_KEY_MODIFIED] = QString::number(QDateTime::currentSecsSinceEpoch());
}
