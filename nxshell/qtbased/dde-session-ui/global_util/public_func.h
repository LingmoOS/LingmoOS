// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PUBLIC_FUNC_H
#define PUBLIC_FUNC_H

#include <QPixmap>
#include <QApplication>
#include <QIcon>
#include <QImageReader>
#include <QSettings>
#include <QString>

QPixmap loadPixmap(const QString &file);

template <typename T>
T findValueByQSettings(const QStringList &configFiles,
                       const QString &group,
                       const QString &key,
                       const QVariant &failback)
{
    for (const QString &path : configFiles) {
        QSettings settings(path, QSettings::IniFormat);
        if (!group.isEmpty()) {
            settings.beginGroup(group);
        }

        const QVariant& v = settings.value(key);
        if (v.isValid()) {
            T t = v.value<T>();
            return t;
        }
    }

    return failback.value<T>();
}

#endif // PUBLIC_FUNC_H
