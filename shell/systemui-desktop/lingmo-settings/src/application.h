#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QDBusConnection>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "include/interface/moduleinterface.h"

class Application : public QApplication {
  Q_OBJECT

 public:
  explicit Application(int &argc, char **argv);
  void addPage(QString title, QString name, QString page, QString iconSource,
               QString iconColor, QString category);
  void switchToPage(const QString &name);

 private:
  void insertPlugin();
  QQmlApplicationEngine m_engine;
};

#endif  // APPLICATION_H
