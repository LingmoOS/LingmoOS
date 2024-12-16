// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCMANAGER_H
#define DCCMANAGER_H
#include "oceanuiapp.h"
#include "oceanuiobject.h"

#include <DConfig>
#include <LSysInfo>

#include <QDBusContext>
#include <QDBusMessage>
#include <QObject>

QT_BEGIN_NAMESPACE
class QWindow;
class QQmlApplicationEngine;
class QAbstractItemModel;
QT_END_NAMESPACE

namespace oceanuiV25 {
class NavigationModel;
class SearchModel;
class PluginManager;

class OceanUIManager : public OceanUIApp, protected QDBusContext
{
    Q_OBJECT
public:
    explicit OceanUIManager(QObject *parent = nullptr);
    ~OceanUIManager() override;

    static bool installTranslator(const QString &name);
    void init();
    QQmlApplicationEngine *engine();
    void setMainWindow(QWindow *window);
    void loadModules(bool async, const QStringList &dirs);

    int width() const override;
    int height() const override;

    inline OceanUIObject *root() const override { return m_root; }

    inline OceanUIObject *activeObject() const override { return m_activeObject; }

    Q_INVOKABLE inline const QVector<OceanUIObject *> &currentObjects() const { return m_currentObjects; }

    Q_INVOKABLE OceanUIApp::UosEdition uosEdition() const;

    Q_INVOKABLE bool isTreeland() const;

    inline const QSet<QString> &hideModule() const { return m_hideModule; }

public Q_SLOTS:
    OceanUIObject *object(const QString &name) override;
    void addObject(OceanUIObject *obj) override;
    void removeObject(OceanUIObject *obj) override;
    void removeObject(const QString &name) override;
    void showPage(const QString &url) override;
    void showPage(OceanUIObject *obj);
    void showPage(OceanUIObject *obj, const QString &cmd) override;
    void toBack();
    QWindow *mainWindow() const override;
    QAbstractItemModel *navModel() const override;
    QSortFilterProxyModel *searchModel() const override;

    void show();
    void showHelp();
    // DBus Search
    QString search(const QString &json);
    bool stop(const QString &json);
    bool action(const QString &json);
    QString GetAllModule();

Q_SIGNALS:
    void activeItemChanged(QQuickItem *item);
    void hideModuleChanged(const QSet<QString> &hideModule);

private:
    void initConfig();
    bool contains(const QSet<QString> &urls, const OceanUIObject *obj);
    bool isEqual(const QString &url, const OceanUIObject *obj);
    OceanUIObject *findObject(const QString &url, bool onlyRoot = false);
    OceanUIObject *findParent(const OceanUIObject *obj);

private Q_SLOTS:
    void waitShowPage(const QString &url, const QDBusMessage message);
    void doShowPage(OceanUIObject *obj, const QString &cmd);
    void updateModuleConfig(const QString &key);
    void onVisible(bool visible);
    void onObjectAoceand(OceanUIObject *obj);
    void onObjectRemoved(OceanUIObject *obj);
    void onObjectDisplayChanged();
    bool addObjectToParent(OceanUIObject *obj);
    bool removeObjectFromParent(OceanUIObject *obj);
    void onQuit();
    void waitLoadFinished() const;
    void doGetAllModule(const QDBusMessage message) const;

private:
    OceanUIObject *m_root;
    OceanUIObject *m_activeObject;    // 当前定位的项
    OceanUIObject *m_hideObjects;     // 隐藏的项
    OceanUIObject *m_noAddObjects;    // 未找到父对象的
    OceanUIObject *m_noParentObjects; // 没有父对象的

    QVector<OceanUIObject *> m_currentObjects;

    PluginManager *m_plugins;
    QWindow *m_window;
    Dtk::Core::DConfig *m_dconfig;
    QSet<QString> m_hideModule;
    QSet<QString> m_disableModule;
    QQmlApplicationEngine *m_engine;
    NavigationModel *m_navModel;
    SearchModel *m_searchModel;
};
} // namespace oceanuiV25
#endif // DCCMANAGER_H
