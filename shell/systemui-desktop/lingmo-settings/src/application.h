#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QDBusConnection>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <memory>
#include <qguiapplication.h>
#include <qobject.h>

#include "include/interface/moduleinterface.h"

class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(std::shared_ptr<QQmlApplicationEngine> engine);
    void addPage(QString title, QString name, QString page, QString iconSource,
        QString iconColor, QString category);
    void switchToPage(const QString& name);

private:
    void insertPlugin();

    void initLogger();

    std::shared_ptr<QQmlApplicationEngine> m_engine;
};

#endif // APPLICATION_H
