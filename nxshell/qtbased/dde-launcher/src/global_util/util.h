// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTIL_H
#define UTIL_H

#include "iteminfo.h"
#include "constants.h"

#include <DConfig>

#include <QtCore>
#include <QGSettings>

DCORE_USE_NAMESPACE

enum CacheType {
    TextType,
    ImageType
};

QString getCategoryNames(QString text);
const QPixmap loadSvg(const QString &fileName, const int size);
const QPixmap loadSvg(const QString &fileName, const QSize &size);
const QPixmap renderSVG(const QString &path, const QSize &size);
QGSettings *SettingsPtr(const QString &schema_id, const QByteArray &path = QByteArray(), QObject *parent = nullptr);
QGSettings *ModuleSettingsPtr(const QString &module, const QByteArray &path = QByteArray(), QObject *parent = nullptr);
QString qtify_name(const char *name);
QVariant SettingValue(const QString &schema_id, const QByteArray &path = QByteArray(), const QString &key = QString(), const QVariant &fallback = QVariant());
bool createCalendarIcon(const QString &fileName);
int perfectIconSize(const int size);
QString cacheKey(const ItemInfo_v1 &itemInfo);
bool getThemeIcon(QPixmap &pixmap, const ItemInfo_v1 &itemInfo, const int size);
QIcon getIcon(const QString &name);

class ConfigWorker : QObject
{
    Q_OBJECT
public:
    static DConfig *instance();

    static QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant());
    static void setValue(const QString &key, const QVariant &value = QVariant());

private:
    static DConfig *INSTANCE;
};
#endif // UTIL_H
