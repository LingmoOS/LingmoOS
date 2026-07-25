#include "lingmoui.h"

#ifdef _WIN32
    #include "windowhelper.h"
    #include "shadowhelper/windowshadow.h"
#elif __APPLE__
    #include "windowhelper.h"
    #include "shadowhelper/windowshadow.h"
#elif __linux__
    #ifdef KF_WINDOWSYSTEM_FOUND
        #include "windowhelper.h"
        #include "shadowhelper/windowshadow.h"
    #endif
#endif

#include "blurhelper/windowblur.h"
#include "iconthemeprovider.h"
#include "newiconitem.h"
#include "qqmlsortfilterproxymodel.h"
#include "thememanager.h"
#include "wheelhandler.h"

#include "desktop/menupopupwindow.h"

#include <QDebug>
#include <QFontDatabase>
#include <QQmlEngine>
#include <QQuickStyle>

QUrl componentUrl(const QString& fileName)
{
    return QUrl(QStringLiteral("qrc:/lingmoui/kit/compatible_module/LingmoUI/CompatibleModule/Controls/") + fileName);
}

void LingmoUI::initializeEngine(QQmlEngine* engine, const char* uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("LingmoUI.CompatibleModule"));

    int fontId = QFontDatabase::addApplicationFont(":/lingmoui/kit/compatible_module/LingmoUI/CompatibleModule/images/fonticons/FluentSystemIcons-Regular.ttf");
    QStringList fontFamilies;
    fontFamilies << QFontDatabase::applicationFontFamilies(fontId);

    // Set base URL to the plugin URL
    engine->setBaseUrl(baseUrl());

    engine->addImageProvider(QStringLiteral("icontheme"), new IconThemeProvider());
}

void LingmoUI::registerTypes(const char* uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("LingmoUI.CompatibleModule"));

    qmlRegisterSingletonType<ThemeManager>("LingmoUI.CompatibleModule.Core", 3, 0, "ThemeManager", [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return new ThemeManager;
    });

#ifdef _WIN32
    qmlRegisterType<WindowShadow>(uri, 3, 0, "WindowShadow");
    qmlRegisterType<WindowHelper>(uri, 3, 0, "WindowHelper");
#elif __APPLE__
    qmlRegisterType<WindowShadow>(uri, 3, 0, "WindowShadow");
    qmlRegisterType<WindowHelper>(uri, 3, 0, "WindowHelper");
#elif __linux__
    #ifdef KF_WINDOWSYSTEM_FOUND
        qmlRegisterType<WindowShadow>(uri, 3, 0, "WindowShadow");
        qmlRegisterType<WindowHelper>(uri, 3, 0, "WindowHelper");
    #endif
#endif
    qmlRegisterType<WindowBlur>(uri, 3, 0, "WindowBlur");
    qmlRegisterType<NewIconItem>(uri, 3, 0, "IconItem");
    qmlRegisterType<MenuPopupWindow>(uri, 3, 0, "MenuPopupWindow");
    qmlRegisterType<WheelHandler>(uri, 3, 0, "WheelHandler");
    qmlRegisterType<QQmlSortFilterProxyModel>(uri, 3, 0, "SortFilterProxyModel");

    qmlRegisterSingletonType(componentUrl(QStringLiteral("Theme.qml")), uri, 3, 0, "Theme");
    qmlRegisterSingletonType(componentUrl(QStringLiteral("Units.qml")), uri, 3, 0, "Units");

    qmlRegisterType(componentUrl(QStringLiteral("AboutDialog.qml")), uri, 3, 0, "AboutDialog");
    qmlRegisterType(componentUrl(QStringLiteral("ActionTextField.qml")), uri, 3, 0, "ActionTextField");
    qmlRegisterType(componentUrl(QStringLiteral("BusyIndicator.qml")), uri, 3, 0, "BusyIndicator");
    qmlRegisterType(componentUrl(QStringLiteral("Icon.qml")), uri, 3, 0, "Icon");
    qmlRegisterType(componentUrl(QStringLiteral("PopupTips.qml")), uri, 3, 0, "PopupTips");
    qmlRegisterType(componentUrl(QStringLiteral("RoundedRect.qml")), uri, 3, 0, "RoundedRect");
    qmlRegisterType(componentUrl(QStringLiteral("TabBar.qml")), uri, 3, 0, "TabBar");
    qmlRegisterType(componentUrl(QStringLiteral("TabButton.qml")), uri, 3, 0, "TabButton");
    qmlRegisterType(componentUrl(QStringLiteral("TabCloseButton.qml")), uri, 3, 0, "TabCloseButton");
    qmlRegisterType(componentUrl(QStringLiteral("TabView.qml")), uri, 3, 0, "TabView");
    qmlRegisterType(componentUrl(QStringLiteral("Toast.qml")), uri, 3, 0, "Toast");
    qmlRegisterType(componentUrl(QStringLiteral("Window.qml")), uri, 3, 0, "Window");
    qmlRegisterType(componentUrl(QStringLiteral("RoundImageButton.qml")), uri, 3, 0, "RoundImageButton");
    qmlRegisterType(componentUrl(QStringLiteral("DesktopMenu.qml")), uri, 3, 0, "DesktopMenu");

    qmlProtectModule(uri, 1);
}