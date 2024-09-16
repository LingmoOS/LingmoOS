// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGUIAPPLICATIONHELPER_H
#define DGUIAPPLICATIONHELPER_H

#include <dtkgui_global.h>
#include <DPalette>
#include <DObject>

#include <QGuiApplication>
#include <QObject>

DGUI_BEGIN_NAMESPACE

class DPlatformTheme;
class DFontManager;
class DGuiApplicationHelperPrivate;
class DGuiApplicationHelper : public QObject, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DGuiApplicationHelper)

    Q_PROPERTY(ColorType themeType READ themeType NOTIFY themeTypeChanged)
    Q_PROPERTY(ColorType paletteType READ paletteType WRITE setPaletteType NOTIFY paletteTypeChanged)
    Q_PROPERTY(bool hasUserManual READ hasUserManual)

public:
    enum ColorType {
        UnknownType,
        LightType,
        DarkType
    };
    Q_ENUM(ColorType)

    enum SingleScope {
        UserScope,
        GroupScope,
        WorldScope
    };
    Q_ENUM(SingleScope)

    enum SizeMode {
        NormalMode,
        CompactMode
    };
    Q_ENUM(SizeMode)

    enum Attribute {
        UseInactiveColorGroup    = 1 << 0,
        ColorCompositing         = 1 << 1,
        DontSaveApplicationTheme = 1 << 2,

        /* readonly flag */
        ReadOnlyLimit            = 1 << 22,
        IsDeepinPlatformTheme    = ReadOnlyLimit << 0,
        IsDXcbPlatform           = ReadOnlyLimit << 1,
        IsXWindowPlatform        = ReadOnlyLimit << 2,
        IsTableEnvironment       = ReadOnlyLimit << 3,
        IsDeepinEnvironment      = ReadOnlyLimit << 4,
        IsSpecialEffectsEnvironment = ReadOnlyLimit << 5
    };
    Q_ENUM(Attribute)
    Q_DECLARE_FLAGS(Attributes, Attribute)

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    typedef DGuiApplicationHelper *(*HelperCreator)();
    D_DECL_DEPRECATED static void registerInstanceCreator(HelperCreator creator);
#endif
    static DGuiApplicationHelper *instance();
    ~DGuiApplicationHelper();

    static QColor adjustColor(const QColor &base, qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat,
                              qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat);
    static QImage adjustColor(const QImage &base, qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat,
                              qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat);
    static QColor blendColor(const QColor &substrate, const QColor &superstratum);
    static DPalette standardPalette(ColorType type);
    static void generatePaletteColor(DPalette &base, QPalette::ColorRole role, ColorType type);
    static void generatePaletteColor(DPalette &base, DPalette::ColorType role, ColorType type);
    static void generatePalette(DPalette &base, ColorType type = UnknownType);
    static DPalette fetchPalette(const DPlatformTheme *theme);
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    Q_DECL_DEPRECATED_X("Use UseInactiveColorGroup enum with setAttribute.") static void setUseInactiveColorGroup(bool on);
    Q_DECL_DEPRECATED_X("Use ColorCompositing enum with setAttribute.") static void setColorCompositingEnabled(bool on);
#endif
    static bool isXWindowPlatform();
    static bool isTabletEnvironment();
    static bool isSpecialEffectsEnvironment();
    static void setAttribute(Attribute attribute, bool enable);
    static bool testAttribute(Attribute attribute);

    DPlatformTheme *systemTheme() const;
    DPlatformTheme *applicationTheme() const;
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    D_DECL_DEPRECATED DPlatformTheme *windowTheme(QWindow *window) const;
#endif

    DPalette applicationPalette() const;
    DPalette applicationPalette(ColorType paletteType) const;
    void setApplicationPalette(const DPalette &palette);
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    D_DECL_DEPRECATED DPalette windowPalette(QWindow *window) const;
#endif

    const DFontManager *fontManager() const;

    static ColorType toColorType(const QColor &color);
    static ColorType toColorType(const QPalette &palette);
    ColorType themeType() const;
    ColorType paletteType() const;

    static bool setSingleInstance(const QString &key, SingleScope singleScope = UserScope);
    static void setSingleInstanceInterval(int interval = 3000);
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    D_DECL_DEPRECATED static void setSingelInstanceInterval(int interval = 3000);
#endif

    // 获取帮助手册目录
    static QStringList userManualPaths(const QString &appName);
    bool hasUserManual() const;

    static bool loadTranslator(const QString &fileName, const QList<QString> &translateDirs, const QList<QLocale> &localeFallback);
    static bool loadTranslator(const QList<QLocale> &localeFallback = QList<QLocale>() << QLocale::system());

public Q_SLOTS:
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    D_DECL_DEPRECATED_X("Plase use setPaletteType") void setThemeType(ColorType themeType);
#endif
    void setPaletteType(ColorType paletteType);
    void handleHelpAction();
    static void openUrl(const QString &url);

    DGuiApplicationHelper::SizeMode sizeMode() const;
    void setSizeMode(const DGuiApplicationHelper::SizeMode mode);
    void resetSizeMode();
    static inline bool isCompactMode()
    {
        return instance()->sizeMode() == DGuiApplicationHelper::CompactMode;
    }

Q_SIGNALS:
    void themeTypeChanged(ColorType themeType);
    void paletteTypeChanged(ColorType paletteType);
    void newProcessInstance(qint64 pid, const QStringList &arguments);
    void fontChanged(const QFont &font);
    void applicationPaletteChanged();
    void sizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);

protected:
    explicit DGuiApplicationHelper();
    virtual void initialize();

private:
    D_PRIVATE_SLOT(void _q_initApplicationTheme(bool))
    D_PRIVATE_SLOT(void _q_sizeModeChanged(int))
    friend class _DGuiApplicationHelper;
};

DGUI_END_NAMESPACE

#endif // DGUIAPPLICATIONHELPER_H
