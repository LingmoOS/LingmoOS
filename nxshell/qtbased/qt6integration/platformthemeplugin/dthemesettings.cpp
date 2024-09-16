/*
 * SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "dthemesettings.h"

#include <QFile>
#include <QFileInfo>
#include <QTimer>

#include <DFileWatcherManager>

#define ICON_THEME_NAME QStringLiteral("IconThemeName")
#define F_ICON_THEME_NAME QStringLiteral("FallBackIconThemeName")
#define STYLE_NAMES QStringLiteral("StyleNames")
#define SYSTEM_FONT QStringLiteral("Font")
#define SYSTEM_FIXED_FONT QStringLiteral("MonoFont")
#define SYSTEM_FONT_POINT_SIZE QStringLiteral("FontSize")
// 在某些情况下，触摸滚动视图的操作可能和其它的touch move行为冲突（例如文件管理器的视图滚动和文件内容多选）
// 只能在时间上控制让两个冲突的逻辑错开。此处定义此时间（单位：ms），表示从touch begin到touch move比较在
// 此时间段内完成，否则不应该认为这个一个触摸滚动操作
#define TOUCH_FLICK_BEGIN_MOVE_DELAY QStringLiteral("TouchFlickBeginMoveDelay")
#define SCREEN_SCALE_FACTORS QStringLiteral("ScreenScaleFactors")
#define SCALE_FACTOR QStringLiteral("ScaleFactor")
#define SCALE_LOGICAL_DPI QStringLiteral("ScaleLogicalDpi")
// 当窗口从一个屏幕移动到另一个屏幕后，自动根据新的屏幕缩放比例更新窗口实际大小
#define AUTO_SCALE_WINDOW QStringLiteral("AutoScaleWindow")

DCORE_USE_NAMESPACE

DThemeSettings::DThemeSettings(bool watchFile, QObject *parent)
    : QObject(parent)
    , settings(makeSettings())
{
    if (!watchFile)
        return;

    QStringList list;

    list << settings->fileName();
    list << QSettings(QSettings::IniFormat,
                      QSettings::SystemScope,
                      "deepin", "qt-theme").fileName();

    DFileWatcherManager *watcher = new DFileWatcherManager(this);

    for (const QString &path : list) {
        QFile file(path);

        if (!file.exists()) {
            file.open(QFile::WriteOnly);
            file.close();
        }

        if (QFile::exists(path))
            watcher->add(path);
    }

    connect(watcher, &DFileWatcherManager::fileModified, this, &DThemeSettings::onConfigChanged);
}

QSettings * DThemeSettings::makeSettings()
{
    QString saveConfigPath;

    do {
        // 需要自定义读取主题相关配置的存储路径，未定义时默认为 ~/.config
        // 适用于这样的情况：一个使用pkexec使用root权限启动的UI应用，需要跟随启动时的普通
        // 用户的字体、缩放等设置，可通过在QCoreApplication构造之前设置此环境变量指定使用
        // 某用户的主题配置文件。
        static QByteArray theme_config_path = qgetenv("D_QT_THEME_CONFIG_PATH");

        if (theme_config_path.isEmpty()) {
            break;
        }

        const QString suffix("/deepin/qt-theme.ini");
        QFileInfo info(theme_config_path + suffix);

        // 忽略不存在的文件或软连接
        if (!info.exists() || info.isSymLink()) {
            break;
        }

        // 先创建一个对象，用于获取默认配置文件的路径
        QSettings s(QSettings::IniFormat, QSettings::UserScope, "deepin", "qt-theme");
        QString file_path = s.fileName();

        // 必须以此路径结尾，去除此路径的剩余部分为配置文件路径
        if (!file_path.endsWith(suffix)) {
            break;
        }

        saveConfigPath = file_path.left(file_path.size() - suffix.size());

        if (saveConfigPath.isEmpty()) {
            break;
        }

        // 设置自定义的主题配置文件存储目录
        QSettings::setPath(s.format(), s.scope(), QString::fromLocal8Bit(theme_config_path));
    } while (false);

    QSettings *s = new QSettings(QSettings::IniFormat,
                                 QSettings::UserScope,
                                 "deepin", "qt-theme");

    // 恢复原本的配置目录
    if (!saveConfigPath.isEmpty()) {
        QSettings::setPath(s->format(), s->scope(), saveConfigPath);
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    s->setIniCodec("utf-8");
#endif
    s->beginGroup("Theme");

    return s;
}

bool DThemeSettings::contains(const QString &key) const
{
    return settings->contains(key);
}

QVariant DThemeSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return settings->value(key, defaultValue);
}

bool DThemeSettings::isSetIconThemeName() const
{
    return contains(ICON_THEME_NAME);
}

QString DThemeSettings::iconThemeName() const
{
    return value(ICON_THEME_NAME).toString();
}

bool DThemeSettings::isSetFallbackIconThemeName() const
{
    return contains(F_ICON_THEME_NAME);
}

QString DThemeSettings::fallbackIconThemeName() const
{
    return value(F_ICON_THEME_NAME).toString();
}

bool DThemeSettings::isSetSystemFont() const
{
    return contains(SYSTEM_FONT);
}

QString DThemeSettings::systemFont() const
{
    return value(SYSTEM_FONT).toString();
}

bool DThemeSettings::isSetStyleNames() const
{
    return contains(STYLE_NAMES);
}

QStringList DThemeSettings::styleNames() const
{
    return value(STYLE_NAMES).toStringList();
}

bool DThemeSettings::isSetSystemFontPixelSize() const
{
    return contains(SYSTEM_FONT_POINT_SIZE);
}

qreal DThemeSettings::systemFontPointSize() const
{
    return value(SYSTEM_FONT_POINT_SIZE, 9.0).toDouble();
}

bool DThemeSettings::isSetSystemFixedFont() const
{
    return contains(SYSTEM_FIXED_FONT);
}

QString DThemeSettings::systemFixedFont() const
{
    return value(SYSTEM_FIXED_FONT).toString();
}

int DThemeSettings::touchFlickBeginMoveDelay() const
{
    return value(TOUCH_FLICK_BEGIN_MOVE_DELAY, 300).toInt();
}

qreal DThemeSettings::scaleFactor() const
{
    return value(SCALE_FACTOR).toReal();
}

QByteArray DThemeSettings::screenScaleFactors() const
{
    return value(SCREEN_SCALE_FACTORS).toByteArray();
}

// 从配置文件中获取dpi相关数据，文件中存储的格式为 ScaleLogicalDpi=x,y
// 会被QSettings解析为QStringList，此处需要将其转换为QPair<qreal,qreal>
static DDpi takePair(const QVariant &value)
{
    if (!value.isValid()) {
        return qMakePair(0.0, 0.0);
    }

    const QStringList &l = value.toStringList();

    if (l.count() < 2) {
        return qMakePair(0.0, 0.0);
    }

    DDpi ret;

    ret.first = l.first().toDouble();
    ret.second = l.at(1).toDouble();

    return ret;
}

DDpi DThemeSettings::scaleLogicalDpi() const
{
    return takePair(value(SCALE_LOGICAL_DPI));
}

bool DThemeSettings::autoScaleWindow() const
{
    return value(AUTO_SCALE_WINDOW, true).toBool();
}

void DThemeSettings::onConfigChanged()
{
    QVariantMap config;

    for (const QString &v : settings->allKeys()) {
        config[v] = settings->value(v);
    }

    settings->sync();

    for (const QString &v : settings->allKeys()) {
        const QVariant &old_value = config.value(v);
        const QVariant &new_value = settings->value(v);

        if (old_value != new_value) {
            if (v == ICON_THEME_NAME)
                emit iconThemeNameChanged(new_value.toString());
            else if (v == F_ICON_THEME_NAME)
                emit fallbackIconThemeNameChanged(new_value.toString());
            else if (v == SYSTEM_FONT)
                emit systemFontChanged(new_value.toString());
            else if (v == STYLE_NAMES)
                emit styleNamesChanged(new_value.toStringList());
            else if (v == SYSTEM_FIXED_FONT)
                emit systemFixedFontChanged(new_value.toString());
            else if (v == SYSTEM_FONT_POINT_SIZE)
                emit systemFontPointSizeChanged(new_value.toInt());
            else if (v == TOUCH_FLICK_BEGIN_MOVE_DELAY)
                emit touchFlickBeginMoveDelayChanged(new_value.toInt());
            else if (v == SCREEN_SCALE_FACTORS)
                emit screenScaleFactorsChanged(new_value.toByteArray());
            else if (v == SCALE_FACTOR)
                emit scaleFactorChanged(new_value.toReal());
            else if (v == SCALE_LOGICAL_DPI)
                emit scaleLogicalDpiChanged(takePair(new_value));

            emit valueChanged(v, old_value, new_value);
        }
    }
}
