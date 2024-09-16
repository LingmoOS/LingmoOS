#ifndef KWIN_CONFIGREADER_H
#define KWIN_CONFIGREADER_H

#include <kwinglobals.h>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusArgument>
#include <QThread>
#include <QDebug>
#include <QList>
#include <QHash>

#include "wayland_server.h"

namespace KWin
{
class ConfigReader;

class ConfigMonitor : public QObject
{
    Q_OBJECT
public:
    ConfigMonitor(QString service, QString path);
    ~ConfigMonitor();

    void addReader(ConfigReader *);
    void removeReader(ConfigReader *);
    void inhibit();
    void uninhibit();

public Q_SLOTS:
    void slotUpdateProperty(QDBusMessage msg);

private:
    QList<ConfigReader *> m_readerList;
    int                   m_inhibitCount = 0;
};

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    ConfigManager();
    ~ConfigManager();

    ConfigMonitor *getMonitor(QString service, QString path);

    static ConfigManager *instance();

private:
    static std::unique_ptr<ConfigManager> m_configManager;
    QHash<QString, std::shared_ptr<ConfigMonitor>> m_managerList;
};

class ConfigReaderThread : public QThread
{
    Q_OBJECT
public:
    ConfigReaderThread(const QString &service, const QString &path, const QString &interface, const QString &propertyName);
    ~ConfigReaderThread();

Q_SIGNALS:
    void propertyFound(QVariant);

protected:
    void run() override;

private:
    QString m_service;
    QString m_path;
    QString m_interface;
    QString m_propertyName;
};


class KWIN_EXPORT ConfigReader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant m_property READ getProperty WRITE setProperty)
public:
    ConfigReader(const QString &service, const QString &path, const QString &interface, const QString &propertyName);
    ~ConfigReader();

    QVariant getProperty() const {
        return m_property;
    }

    void setProperty(const QVariant &property) {
        m_property = property;
    }

    QString getPropertyName() { return m_propertyName; }
    QString getInterface() { return m_interface; }
    void updateProperty(QVariant);

Q_SIGNALS:
    void sigPropertyChanged(QVariant);

public Q_SLOTS:
    void slotSetProperty(QVariant property);

private:
    QString m_interface;
    QString m_propertyName;
    QString m_service;
    QVariant m_property;
    ConfigMonitor *m_monitor = nullptr;
};

}

#endif