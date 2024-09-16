// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qmlplugin_plugin.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "dquickwindow.h"
#include "dquickitemviewport.h"
#include "dquickblitframebuffer.h"
#include "private/dhandlecontextmenuwindow_p.h"
#include "private/dquickglow_p.h"
#include "private/dquickinwindowblur_p.h"
#include "private/dquickrectangle_p.h"
#include "private/dquickbehindwindowblur_p.h"
#include "private/dquickopacitymask_p.h"
#include "private/dquickkeylistener_p.h"

#include "private/dqmlglobalobject_p.h"
#include "private/dconfigwrapper_p.h"
#include "private/dquickiconimage_p.h"
#include "private/dquickdciiconimage_p.h"
#include "private/dquickiconlabel_p.h"
#include "private/dsettingscontainer_p.h"
#include "private/dmessagemanager_p.h"
#include "private/dpopupwindowhandle_p.h"
#include "private/dobjectmodelproxy_p.h"
#include "private/dquickwaterprogressattribute_p.h"
#include "private/dquickarrowboxpath_p.h"
#include "private/dquickcoloroverlay_p.h"
#include "private/dquickapploaderitem_p.h"
#endif

#include "private/dquickimageprovider_p.h"
#include "private/dquickcontrolpalette_p.h"

#include <DFontManager>
#include <DPlatformHandle>

#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <qqml.h>

DGUI_USE_NAMESPACE

DQUICK_BEGIN_NAMESPACE


static const QString softwareBackend = "software";

template<typename T>
inline void dtkRegisterType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName) {
    qmlRegisterType<T>(uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterType<T>(uri2, versionMajor, versionMinor, qmlName);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template<typename T>
inline void dtkRegisterAnonymousType(const char *uri1, const char *uri2, int versionMajor) {
#if (QT_VERSION > QT_VERSION_CHECK(5, 14, 0))
    qmlRegisterAnonymousType<T>(uri1, versionMajor);
    if (uri2)
        qmlRegisterAnonymousType<T>(uri2, versionMajor);
#else
    Q_UNUSED(uri1)
    Q_UNUSED(uri2)
    Q_UNUSED(versionMajor)
    qmlRegisterType<T>();
#endif
}
template<typename T>
inline void dtkRegisterUncreatableType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName, const QString& reason) {
    qmlRegisterUncreatableType<T>(uri1, versionMajor, versionMinor, qmlName, reason);
    if (uri2)
        qmlRegisterUncreatableType<T>(uri2, versionMajor, versionMinor, qmlName, reason);
}
template <typename T>
inline void dtkRegisterSingletonType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName,
                                     QObject *(*callback)(QQmlEngine *, QJSEngine *)) {
    qmlRegisterSingletonType<T>(uri1, versionMajor, versionMinor, qmlName, callback);
    if (uri2)
        qmlRegisterSingletonType<T>(uri2, versionMajor, versionMinor, qmlName, callback);
}
inline void dtkSettingsRegisterType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName) {
    static QString urlTemplate = QStringLiteral("qrc:/dtk/declarative/qml/settings/%1.qml");
    const QUrl url(urlTemplate.arg(qmlName));
    qmlRegisterType(url, uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterType(url, uri2, versionMajor, versionMinor, qmlName);
}
#endif

inline void dtkRegisterType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName, const char *subdir = "") {
    static QString urlTemplate = QStringLiteral("qrc:/dtk/declarative/qml/%1%2.qml");
    const QUrl url(urlTemplate.arg(subdir).arg(qmlName));
    qmlRegisterType(url, uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterType(url, uri2, versionMajor, versionMinor, qmlName);
}

inline void dtkRegisterTypeAlias(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlFileName, const char *alias, const char *subdir = "") {
    static QString urlTemplate = QStringLiteral("qrc:/dtk/declarative/qml/%1%2.qml");
    const QUrl url(urlTemplate.arg(subdir).arg(qmlFileName));
    qmlRegisterType(url, uri1, versionMajor, versionMinor, alias);
    if (uri2)
        qmlRegisterType(url, uri2, versionMajor, versionMinor, alias);
}

inline void dtkRegisterTypeOverridable(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlFileName) {
    dtkRegisterTypeAlias(uri1, uri2, versionMajor, versionMinor, qmlFileName, qmlFileName, "overridable/");
}

inline void dtkStyleRegisterSingletonType(const char *uri1, const char *uri2, int versionMajor, int versionMinor, const char *qmlName) {
    static QString urlTemplate = QStringLiteral("qrc:/dtk/declarative/qml/style/%1.qml");
    QUrl url(urlTemplate.arg(qmlName));
    qmlRegisterSingletonType(url, uri1, versionMajor, versionMinor, qmlName);
    if (uri2)
        qmlRegisterSingletonType(url, uri2, versionMajor, versionMinor, qmlName);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
static QVariant quickColorTypeConverter(const QString &data)
{
    return QVariant::fromValue(DQuickControlColor(QColor(data)));
}

static QVariant dcolorTypeConverter(const QString &data)
{
    return QVariant::fromValue(DColor(QColor(data)));
}
#endif

template <typename ReturnType>
ReturnType convertColorToQuickColorType(const QColor &value)
{
    return DQuickControlColor(value);
}

template <typename ReturnType>
ReturnType convertColorToDColorType(const QColor &value)
{
    return DColor(value);
}

template <typename ReturnType>
ReturnType convertDColorToQuickColorType(const DColor &value)
{
    return DQuickControlColor(value);
}

void QmlpluginPlugin::registerTypes(const char *uri)
{
    // @uri org.deepin.dtk
    qmlRegisterModule(uri, 1, 0);
    // @uri org.deepin.dtk.style
    const QByteArray styleUri = QByteArray(uri).append(".style");
    qmlRegisterModule(styleUri.constData(), 1, 0);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QByteArray implUri;
    dtkRegisterTypeOverridable(uri, nullptr, 1, 0, "InWindowBlur");
#else
    // @uri org.deepin.dtk.impl
    const QByteArray implUri = QByteArray(uri).append(".impl");
    qmlRegisterModule(implUri.constData(), 1, 0);
    // @uri org.deepin.dtk.private
    const QByteArray privateUri = QByteArray(uri).append(".private");
    qmlRegisterModule(privateUri.constData(), 1, 0);
    // @uri org.deepin.dtk.controls
    const QByteArray controlsUri = QByteArray(uri).append(".controls");
    qmlRegisterModule(controlsUri.constData(), 1, 0);
    // @uri org.deepin.dtk.settings
    const QByteArray settingsUri = QByteArray(uri) + ".settings";
    qmlRegisterModule(settingsUri, 1, 0);

    // for org.deepin.dtk and org.deepin.dtk.impl
    dtkRegisterType<DQuickIconImage>(uri, implUri, 1, 0, "QtIcon");
    dtkRegisterType<DQuickIconLabel>(uri, implUri, 1, 0, "IconLabel");
    dtkRegisterType<DQuickGlow>(uri, implUri, 1, 0, "GlowEffect");
    dtkRegisterType<DQUICK_NAMESPACE::DQuickDciIconImage>(uri, implUri, 1, 0, "DciIcon");
    dtkRegisterType<DQuickItemViewport>(uri, implUri, 1, 0, "ItemViewport");
    dtkRegisterType<DFontManager>(uri, implUri, 1, 0, "FontManager");
    dtkRegisterType<DHandleContextMenuWindow>(uri, implUri, 1, 0, "ContextMenuWindow");
    dtkRegisterType<DQuickBlitFramebuffer>(uri, implUri, 1, 0, "BlitFramebuffer");
    dtkRegisterType<DConfigWrapper>(uri, implUri, 1, 0, "Config");
    dtkRegisterType<DQuickInWindowBlur>(uri, implUri, 1, 0, "InWindowBlur");
    dtkRegisterType<DQuickControlPalette>(uri, implUri, 1, 0, "Palette");
    dtkRegisterType<FloatingMessageContainer>(uri, implUri, 1, 0, "FloatingMessageContainer");
    dtkRegisterUncreatableType<MessageManager>(uri, implUri, 1, 0, "MessageManager", "Window Attached");
    dtkRegisterType<DQuickRectangle>(uri, implUri, 1, 0, "RoundRectangle");
    dtkRegisterType<DQuickBehindWindowBlur>(uri, implUri, 1, 0, "BehindWindowBlur");
    dtkRegisterType<ObjectModelProxy>(uri, implUri, 1, 0, "ObjectModelProxy");
    dtkRegisterType<DQuickOpacityMask>(uri, implUri, 1, 0, "SoftwareOpacityMask");
    dtkRegisterType<DQuickKeyListener>(uri, implUri, 1, 0, "KeySequenceListener");
    dtkRegisterType<DQuickWaterProgressAttribute>(uri, implUri, 1, 0, "WaterProgressAttribute");
    dtkRegisterType<DQuickArrowBoxPath>(uri, implUri, 1, 0, "ArrowBoxPath");
    dtkRegisterType<DQuickAppLoaderItem>(uri, implUri, 1, 0, "AppLoader");
    dtkRegisterType<DQuickColorOverlay>(uri, implUri, 1, 0, "SoftwareColorOverlay");

    dtkRegisterAnonymousType<DQUICK_NAMESPACE::DQuickDciIcon>(uri, implUri, 1);
    dtkRegisterAnonymousType<DQuickControlColor>(uri, implUri, 1);
    dtkRegisterAnonymousType<DDciIconPalette>(uri, implUri, 1);
    dtkRegisterAnonymousType<WaterPopAttribute>(uri, implUri, 1);

    dtkRegisterUncreatableType<DQuickWindow>(uri, implUri, 1, 0, "DWindow", "DQuickWindow Attached");
    dtkRegisterUncreatableType<DQuickControlColorSelector>(uri, implUri, 1, 0, "ColorSelector",
                                                           QStringLiteral("ColorSelector is only available as an attached property."));
    dtkRegisterUncreatableType<DColor>(uri, implUri, 1, 0, "Color",
                                       QStringLiteral("Color is only available as enums."));
    dtkRegisterUncreatableType<DPopupWindowHandle>(uri, implUri, 1, 0, "PopupHandle", "PopupWindow Attached");
    dtkRegisterUncreatableType<DPlatformHandle>(uri, implUri, 1, 0, "PlatformHandle", "PlatformHandle");

    qRegisterMetaType<DQUICK_NAMESPACE::DQuickDciIcon>();
    qRegisterMetaType<DQuickControlColor>("ControlColor");
    qRegisterMetaType<DColor::Type>();
    qRegisterMetaType<DGUI_NAMESPACE::DDciIconPalette>();

    //DQMLGlobalObject 依赖 DWindowManagerHelper中枚举的定义，所以需要先注册
    dtkRegisterSingletonType<DWindowManagerHelper>(uri, implUri, 1, 0, "WindowManagerHelper",
                                                   [](QQmlEngine *, QJSEngine *) -> QObject * {
        auto helper = DWindowManagerHelper::instance();
        QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
        return helper;
    });
    dtkRegisterSingletonType<DGuiApplicationHelper>(uri, implUri, 1, 0, "ApplicationHelper",
                                                    [](QQmlEngine *, QJSEngine *) -> QObject * {
        auto helper = DGuiApplicationHelper::instance();
        QQmlEngine::setObjectOwnership(helper, QQmlEngine::CppOwnership);
        return helper;
    });
    qRegisterMetaType<DGuiApplicationHelper::ColorType>("Dtk::Gui::DGuiApplicationHelper::ColorType");
    dtkRegisterSingletonType<DQMLGlobalObject>(uri, implUri, 1, 0, "DTK",
                                               [](QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject * {
        return DQMLGlobalObject::create(qmlEngine, jsEngine);
    });

    // 自定义的 QML 控件可以通过把 QML 文件注册到环境中的方式来实现
    // for org.deepin.dtk
    dtkRegisterType(uri, nullptr, 1, 0, "RectangularShadow");
    dtkRegisterType(uri, nullptr, 1, 0, "FocusBoxBorder");
    dtkRegisterType(uri, nullptr, 1, 0, "CicleSpreadAnimation");
    dtkRegisterType(uri, nullptr, 1, 0, "BoxShadow");
    dtkRegisterType(uri, nullptr, 1, 0, "BoxInsetShadow");
    dtkRegisterType(uri, nullptr, 1, 0, "InsideBoxBorder");
    dtkRegisterType(uri, nullptr, 1, 0, "OutsideBoxBorder");
    dtkRegisterType(uri, nullptr, 1, 0, "SortFilterModel");

    // for org.deepin.dtk.controls
    // QtQuick Controls
    dtkRegisterType(uri, controlsUri, 1, 0, "ApplicationWindow");
    dtkRegisterType(uri, controlsUri, 1, 0, "Button");
    dtkRegisterType(uri, controlsUri, 1, 0, "Menu");
    dtkRegisterType(uri, controlsUri, 1, 0, "MenuItem");
    dtkRegisterType(uri, controlsUri, 1, 0, "MenuSeparator");
    dtkRegisterType(uri, controlsUri, 1, 0, "ScrollIndicator");
    dtkRegisterType(uri, controlsUri, 1, 0, "Popup");
    dtkRegisterType(uri, controlsUri, 1, 0, "ScrollBar");
    // DTK Controls
    dtkRegisterType(uri, controlsUri, 1, 0, "LineEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "SearchEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "PasswordEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "WarningButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "RecommandButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "ToolButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "IconButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "FloatingButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "ButtonBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "TitleBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "DialogTitleBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "WindowButtonGroup");
    dtkRegisterType(uri, controlsUri, 1, 0, "DialogWindow");
    dtkRegisterType(uri, controlsUri, 1, 0, "AboutDialog");
    dtkRegisterType(uri, controlsUri, 1, 0, "HelpAction");
    dtkRegisterType(uri, controlsUri, 1, 0, "AboutAction");
    dtkRegisterType(uri, controlsUri, 1, 0, "QuitAction");
    dtkRegisterType(uri, controlsUri, 1, 0, "ThemeMenu");
    dtkRegisterType(uri, controlsUri, 1, 0, "AlertToolTip");
    dtkRegisterType(uri, controlsUri, 1, 0, "FlowStyle");
    dtkRegisterType(uri, controlsUri, 1, 0, "Control");
    dtkRegisterType(uri, controlsUri, 1, 0, "SpinBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "TextField");
    dtkRegisterType(uri, controlsUri, 1, 0, "TextArea");
    dtkRegisterType(uri, controlsUri, 1, 0, "PlusMinusSpinBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "FloatingPanel");
    dtkRegisterType(uri, controlsUri, 1, 0, "FloatingMessage");
    dtkRegisterType(uri, controlsUri, 1, 0, "HighlightPanel");
    dtkRegisterType(uri, controlsUri, 1, 0, "ArrowListView");
    dtkRegisterType(uri, controlsUri, 1, 0, "StyledBehindWindowBlur");
    dtkRegisterType(uri, controlsUri, 1, 0, "PopupWindow");
    dtkRegisterType(uri, controlsUri, 1, 0, "Switch");
    dtkRegisterType(uri, controlsUri, 1, 0, "Slider");
    dtkRegisterType(uri, controlsUri, 1, 0, "ComboBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "ActionButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "ItemDelegate");
    dtkRegisterType(uri, controlsUri, 1, 0, "CheckDelegate");
    dtkRegisterType(uri, controlsUri, 1, 0, "TipsSlider");
    dtkRegisterType(uri, controlsUri, 1, 0, "SliderTipItem");
    dtkRegisterType(uri, controlsUri, 1, 0, "CheckBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "IpV4LineEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "StyledArrowShapeWindow");
    // TODO(xiaoyaobing): software rendering has not been completed
    dtkRegisterType(uri, controlsUri, 1, 0, "ArrowShapePopup");

    dtkRegisterType(uri, controlsUri, 1, 0, "Action");
    dtkRegisterType(uri, controlsUri, 1, 0, "ActionGroup");
    dtkRegisterType(uri, controlsUri, 1, 0, "Label");
    dtkRegisterType(uri, controlsUri, 1, 0, "Container");
    dtkRegisterType(uri, controlsUri, 1, 0, "ButtonGroup");
    dtkRegisterType(uri, controlsUri, 1, 0, "ScrollView");
    dtkRegisterType(uri, controlsUri, 1, 0, "MenuBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "AbstractButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "TabBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "SwipeDelegate");
    dtkRegisterType(uri, controlsUri, 1, 0, "SwipeView");
    dtkRegisterType(uri, controlsUri, 1, 0, "Drawer");
    dtkRegisterType(uri, controlsUri, 1, 0, "Dial");
    dtkRegisterType(uri, controlsUri, 1, 0, "Dialog");
    dtkRegisterType(uri, controlsUri, 1, 0, "DelayButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "RoundButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "RadioButton");
    dtkRegisterType(uri, controlsUri, 1, 0, "ToolTip");
    dtkRegisterType(uri, controlsUri, 1, 0, "DialogButtonBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "Frame");
    dtkRegisterType(uri, controlsUri, 1, 0, "GroupBox");
    dtkRegisterType(uri, controlsUri, 1, 0, "PageIndicator");
    dtkRegisterType(uri, controlsUri, 1, 0, "Pane");
    dtkRegisterType(uri, controlsUri, 1, 0, "StackView");
    dtkRegisterType(uri, controlsUri, 1, 0, "BusyIndicator");
    dtkRegisterType(uri, controlsUri, 1, 0, "ProgressBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "BoxPanel");
    dtkRegisterType(uri, controlsUri, 1, 0, "KeySequenceEdit");
    dtkRegisterType(uri, controlsUri, 1, 0, "ButtonIndicator");
    dtkRegisterType(uri, controlsUri, 1, 0, "EmbeddedProgressBar");
    dtkRegisterType(uri, controlsUri, 1, 0, "WaterProgressBar");

    // for org.deepin.dtk.settings
    dtkRegisterType<SettingsOption>(settingsUri, implUri, 1, 0, "SettingsOption");
    dtkRegisterType<SettingsGroup>(settingsUri, implUri, 1, 0, "SettingsGroup");
    dtkRegisterType<SettingsContainer>(settingsUri, implUri, 1, 0, "SettingsContainer");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "SettingsDialog");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "OptionDelegate");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "CheckBox");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "LineEdit");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "ComboBox");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "NavigationTitle");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "ContentTitle");
    dtkSettingsRegisterType(settingsUri, nullptr, 1, 0, "ContentBackground");

    // for org.deepin.dtk.private
    dtkRegisterType(privateUri, implUri, 1, 0, "ButtonPanel");
#endif

    dtkRegisterType<QSortFilterProxyModel>(uri, implUri, 1, 0, "SortFilterProxyModel");

    // for org.deepin.dtk.style(allowed to override)
    dtkStyleRegisterSingletonType(uri, styleUri, 1, 0, "Style");

    if (softwareBackend == QQuickWindow::sceneGraphBackend()) {
        dtkRegisterTypeAlias(uri, implUri, 1, 0, "SoftwareColorOverlay", "ColorOverlay", "private/");
    } else {
        dtkRegisterType(uri, implUri, 1, 0, "ColorOverlay", "private/");
    }

    if (softwareBackend == QQuickWindow::sceneGraphBackend()) {
        dtkRegisterTypeAlias(uri, implUri, 1, 0, "SoftwareOpacityMask", "OpacityMask", "private/");
    } else {
        dtkRegisterType(uri, implUri, 1, 0, "OpacityMask", "private/");
    }

    // for custom type
    QMetaType::registerConverter<QColor, DQuickControlColor>(convertColorToQuickColorType<DQuickControlColor>);
    QMetaType::registerConverter<QColor, DColor>(convertColorToDColorType<DColor>);
    QMetaType::registerConverter<DColor, DQuickControlColor>(convertDColorToQuickColorType<DQuickControlColor>);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QQmlMetaType::registerCustomStringConverter(qMetaTypeId<DQuickControlColor>(), quickColorTypeConverter);
    QQmlMetaType::registerCustomStringConverter(qMetaTypeId<DColor>(), dcolorTypeConverter);
#endif
}

void QmlpluginPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(DGuiApplicationHelper::instance());
    engine->addImageProvider("dtk.icon", new DQuickIconProvider);
    engine->addImageProvider("dtk.dci.icon", new DQuickDciIconProvider);
    engine->addImageProvider("dtk.shadow", new DQuickShadowProvider);
    QQmlExtensionPlugin::initializeEngine(engine, uri);
    DQMLGlobalObject::loadTranslator();
}

DQUICK_END_NAMESPACE
