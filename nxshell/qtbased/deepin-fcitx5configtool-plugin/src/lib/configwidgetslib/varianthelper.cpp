/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "varianthelper.h"
#include <QDBusArgument>

namespace fcitx {
namespace kcm {

QVariantMap toMap(const QVariant &variant) {
    QVariantMap map;
    if (variant.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(variant);
        argument >> map;
    }
    if (variant.canConvert<QVariantMap>()) {
        map = variant.toMap();
    }
    return map;
}

QString valueFromVariantMapByPath(const QVariantMap &map,
                                  const QStringList &path, int depth) {
    auto iter = map.find(path[depth]);
    if (iter == map.end()) {
        return QString();
    }
    if (depth + 1 == path.size()) {
        if (iter->canConvert<QString>()) {
            return iter->toString();
        }
    } else {
        QVariantMap map = toMap(*iter);

        if (!map.isEmpty()) {
            return valueFromVariantMapByPath(map, path, depth + 1);
        }
    }
    return QString();
}

QVariant valueFromVariantHelper(const QVariant &value,
                                const QStringList &pathList, int depth) {
    if (depth == pathList.size()) {
        return value;
    }
    auto map = toMap(value);
    // Make it finishes faster.
    if (map.isEmpty() || !map.contains(pathList[depth])) {
        return {};
    }
    return valueFromVariantHelper(map[pathList[depth]], pathList, depth + 1);
}

QVariant readVariant(const QVariant &value, const QString &path) {
    auto pathList = path.split("/");
    return valueFromVariantHelper(toMap(value), pathList, 0);
}

QString readString(const QVariantMap &map, const QString &path) {
    auto pathList = path.split("/");
    if (pathList.empty()) {
        return QString();
    }
    return valueFromVariantMapByPath(map, pathList, 0);
}

bool readBool(const QVariantMap &map, const QString &path) {
    return readString(map, path) == "True";
}

void writeVariantHelper(QVariantMap &map, const QStringList &path,
                        const QVariant &value, int depth) {
    if (depth + 1 == path.size()) {
        map[path[depth]] = value;
    } else {
        auto iter = map.find(path[depth]);
        if (iter == map.end()) {
            iter = map.insert(path[depth], QVariantMap());
        }

        if (iter->type() != QVariant::Map) {
            auto oldValue = *iter;
            *iter = QVariantMap({{"", oldValue}});
        }

        auto &nextMap = *static_cast<QVariantMap *>(iter->data());
        writeVariantHelper(nextMap, path, value, depth + 1);
    }
}

void writeVariant(QVariantMap &map, const QString &path,
                  const QVariant &value) {
    auto pathList = path.split("/");
    if (pathList.empty()) {
        return;
    }
    writeVariantHelper(map, pathList, value, 0);
}

} // namespace kcm
} // namespace fcitx
