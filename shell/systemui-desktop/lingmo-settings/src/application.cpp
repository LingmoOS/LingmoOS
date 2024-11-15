#define LOG_TAG "Settings::Application"

#include "application.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
// #include <QtWebEngine>

#include <elog.h>

#include "about.h"
#include "hostname.h"
#include "accessibility.h"
#include "appearance.h"
#include "background.h"
#include "battery.h"
#include "batteryhistorymodel.h"
#include "brightness.h"
#include "cursor/cursorthememodel.h"
#include "cursor/mouse.h"
#include "datetime/time.h"
#include "datetime/timezonemap.h"
#include "defaultapplications.h"
#include "fonts/fonts.h"
#include "fontsmodel.h"
#include "language.h"
#include "networkproxy.h"
#include "notifications.h"
#include "password.h"
#include "powermanager.h"
#include "settingsuiadaptor.h"
#include "touchpad.h"
// #include "updatorhelper.h"
// #include "upgradeablemodel.h"

const QString ModuleDirectory = "/usr/lib/lingmo-settings/modules";

static QObject* passwordSingleton(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    Password* object = new Password();
    return object;
}

Application::Application(std::shared_ptr<QQmlApplicationEngine> engine)
    : m_engine(engine)
{

    new SettingsUIAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/SettingsUI"),
        this);
    // QML
    const char* uri = "Lingmo.Settings";
    // @uri Lingmo.Settings
    qmlRegisterType<Appearance>(uri, 1, 0, "Appearance");
    qmlRegisterType<FontsModel>(uri, 1, 0, "FontsModel");
    qmlRegisterType<Brightness>(uri, 1, 0, "Brightness");
    qmlRegisterType<Battery>(uri, 1, 0, "Battery");
    qmlRegisterType<BatteryHistoryModel>(uri, 1, 0, "BatteryHistoryModel");
    qmlRegisterType<CursorThemeModel>(uri, 1, 0, "CursorThemeModel");
    qmlRegisterType<About>(uri, 1, 0, "About");
    qmlRegisterType<HostNameChanger>(uri, 1, 0, "HostNameChanger");
    qmlRegisterType<Background>(uri, 1, 0, "Background");
    qmlRegisterType<Language>(uri, 1, 0, "Language");
    qmlRegisterType<Fonts>(uri, 1, 0, "Fonts");
    qmlRegisterType<PowerManager>(uri, 1, 0, "PowerManager");
    qmlRegisterType<Mouse>(uri, 1, 0, "Mouse");
    qmlRegisterType<Time>(uri, 1, 0, "Time");
    qmlRegisterType<TimeZoneMap>(uri, 1, 0, "TimeZoneMap");
    qmlRegisterType<Touchpad>(uri, 1, 0, "Touchpad");
    qmlRegisterType<NetworkProxy>(uri, 1, 0, "NetworkProxy");
    qmlRegisterType<Notifications>(uri, 1, 0, "Notifications");
    qmlRegisterType<DefaultApplications>(uri, 1, 0, "DefaultApplications");
    qmlRegisterType<Accessibility>(uri, 1, 0, "Accessibility");
    qmlRegisterSingletonType<Password>(uri, 1, 0, "Password", passwordSingleton);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<QAbstractItemModel>();
#else
    qmlRegisterAnonymousType<QAbstractItemModel>(uri, 1);
#endif

    initLogger();

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm")
                             .arg("/usr/share/lingmo-settings/translations/")
                             .arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator* translator = new QTranslator(QApplication::instance());
        if (translator->load(qmFilePath)) {
            QApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    m_engine->addImportPath(QStringLiteral("qrc:/"));
    m_engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

}

void Application::initLogger()
{
    /* initialize EasyLogger */
    elog_init();
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    /* start EasyLogger */
    elog_start();
}

void Application::insertPlugin()
{
    QDir moduleDir(ModuleDirectory);
    if (!moduleDir.exists()) {
        log_d("module directory not exists");
        return;
    }
    auto moduleList = moduleDir.entryInfoList();
    for (auto i : moduleList) {
        QString path = i.absoluteFilePath();

        if (!QLibrary::isLibrary(path))
            continue;

        log_d("loading module: %s", i.absoluteFilePath().toStdString().c_str());
        QElapsedTimer et;
        et.start();
        QPluginLoader loader(path);

        QObject* instance = loader.instance();
        if (!instance) {
            qDebug() << loader.errorString();
            continue;
        }

        instance->setParent(this);

        auto* module = qobject_cast<ModuleInterface*>(instance);
        if (!module) {
            return;
        }
        log_d("load plugin Name: %s %s", module->name().toStdString().c_str(), module->title().toStdString().c_str());
        log_d("load this plugin using time: %d ms", et.elapsed());

        if (module->enabled()) {
            addPage(module->title(), module->name(), module->qmlPath(),
                module->iconPath(), module->iconColor().name(),
                module->category());
        }
    }
}

void Application::addPage(QString title, QString name, QString page,
    QString iconSource, QString iconColor,
    QString category)
{
    QObject* mainObject = m_engine->rootObjects().first();

    if (mainObject) {
        QMetaObject::invokeMethod(
            mainObject, "addPage", Q_ARG(QVariant, title), Q_ARG(QVariant, name),
            Q_ARG(QVariant, page), Q_ARG(QVariant, iconSource),
            Q_ARG(QVariant, iconColor), Q_ARG(QVariant, category));
    }
}

void Application::switchToPage(const QString& name)
{
    QObject* mainObject = m_engine->rootObjects().first();

    if (mainObject) {
        QMetaObject::invokeMethod(mainObject, "switchPageFromName",
            Q_ARG(QVariant, name));
    }
}
