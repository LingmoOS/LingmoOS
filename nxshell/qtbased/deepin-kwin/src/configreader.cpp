#include "configreader.h"

#define MAXTIMES 256
namespace KWin
{
std::unique_ptr<ConfigManager> ConfigManager::m_configManager;

ConfigMonitor::ConfigMonitor(QString service, QString path)
{
    QDBusConnection::sessionBus().connect(service, path, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                          "sa{sv}as", this, SLOT(slotUpdateProperty(QDBusMessage)));
}

ConfigMonitor::~ConfigMonitor()
{
}

void ConfigMonitor::inhibit()
{
    m_inhibitCount++;
}

void ConfigMonitor::uninhibit()
{
    m_inhibitCount--;
}

void ConfigMonitor::addReader(ConfigReader *r)
{
    if (!m_readerList.contains(r))
        m_readerList.append(r);
}

void ConfigMonitor::removeReader(ConfigReader *r)
{
    inhibit();
    if (m_readerList.contains(r))
        m_readerList.removeOne(r);
    uninhibit();
}

void ConfigMonitor::slotUpdateProperty(QDBusMessage msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count()) {
        return;
    }
    if (m_inhibitCount != 0) {
        return;
    }

    QString interfaceName = msg.arguments().at(0).toString();
    for (auto *r : m_readerList) {
        if (r && r->getInterface() == interfaceName) {
            QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
            QStringList keys = changedProps.keys();
            for (int i = 0; i < keys.size(); ++i) {
                if (keys.at(i) == r->getPropertyName()) {
                    r->updateProperty(changedProps.value(keys.at(i)));
                    break;
                }
            }
        }
    }
}

ConfigManager *ConfigManager::instance()
{
    if (!m_configManager) {
        m_configManager.reset(new ConfigManager());
    }
    return m_configManager.get();
}

ConfigManager::ConfigManager()
{
}

ConfigManager::~ConfigManager()
{
}

ConfigMonitor *ConfigManager::getMonitor(QString service, QString path)
{
    if (m_managerList.contains(service)) {
        return m_managerList[service].get();
    } else {
        m_managerList[service] = std::make_shared<ConfigMonitor>(service, path);
    }
    return m_managerList[service].get();
}

ConfigReaderThread::ConfigReaderThread(const QString &service, const QString &path, const QString &interface, const QString &propertyName)
    : m_service(service), m_path(path), m_interface(interface), m_propertyName(propertyName)
{
}

ConfigReaderThread::~ConfigReaderThread()
{
}

void ConfigReaderThread::run()
{
    bool found = false;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    for (int i = 1; i < MAXTIMES; i *= 2) {
        auto sleepfunc = [&i]{
            QThread::sleep(i);
        };
        if (!sessionBus.interface()->isServiceRegistered(m_service)) {
            sleepfunc();
            continue;
        }
        QDBusInterface dbus(m_service, m_path, m_interface);
        dbus.setTimeout(300);
        if (!dbus.isValid()) {
            sleepfunc();
            continue;
        }
        QVariant property = dbus.property(m_propertyName.toStdString().c_str());

        if (property.isValid()) {
            Q_EMIT propertyFound(property);
            found = true;
            break;
        }

        sleepfunc();
    }

    if (!found) {
        qWarning() << "cannot get dbus property";
    }
}

ConfigReader::ConfigReader(const QString &service, const QString &path, const QString &interface, const QString &propertyName)
    : m_service(service), m_interface(interface), m_propertyName(propertyName)
{
    ConfigReaderThread *thread = new ConfigReaderThread(service, path, interface, propertyName);
    connect(thread, SIGNAL(propertyFound(QVariant)), this, SLOT(slotSetProperty(QVariant)));

    thread->start();

    m_monitor = ConfigManager::instance()->getMonitor(service, path);
    if (m_monitor)
        m_monitor->addReader(this);
}

ConfigReader::~ConfigReader()
{
    if (m_monitor)
        m_monitor->removeReader(this);
}

void ConfigReader::updateProperty(QVariant variant)
{
    m_property = variant;
    Q_EMIT sigPropertyChanged(variant);
}

void ConfigReader::slotSetProperty(QVariant property)
{
    m_property = property;
}

}
