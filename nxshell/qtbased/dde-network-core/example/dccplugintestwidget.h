// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCCPLUGINTESTWIDGET_H
#define DCCPLUGINTESTWIDGET_H

#include <QWidget>
#include <QStack>

//#include <interface/frameproxyinterface.h>
#include <interface/moduleobject.h>

#include <DMainWindow>

class QPushButton;
class DCCNetworkModule;
class QHBoxLayout;
class FourthColWidget;
class QMouseEvent;
class QPaintEvent;
class QStandardItemModel;
class QTranslator;
class QAbstractItemView;
class QBoxLayout;

DWIDGET_BEGIN_NAMESPACE
class DBackgroundGroup;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

using namespace DCC_NAMESPACE;

namespace DCC_NAMESPACE {


class MainModulePrivate;

class MainModule : public ModuleObject
{
    Q_OBJECT

public:
    explicit MainModule(QObject *parent = nullptr);
    MainModule(const QString &name, const QString &displayName = QString(), QObject *parent = nullptr);
    MainModule(const QString &name, const QStringList &contentText, QObject *parent = nullptr);
    MainModule(const QString &name, const QString &displayName, const QStringList &contentText, QObject *parent = nullptr);
    MainModule(const QString &name, const QString &displayName, const QVariant &icon, QObject *parent = nullptr);
    MainModule(const QString &name, const QString &displayName, const QString &description, QObject *parent = nullptr);
    MainModule(const QString &name, const QString &displayName, const QString &description, const QVariant &icon, QObject *parent = nullptr);
    MainModule(const QString &name, const QString &displayName, const QString &description, const QIcon &icon, QObject *parent = nullptr);
    MainModule(const QString &name, const QString &displayName, const QString &description, const QStringList &contentText, const QVariant &icon, QObject *parent = nullptr);
    ~MainModule() override;

    void appendChild(ModuleObject *const module) override;
    QWidget *page() override;
    ModuleObject *defultModule() override;
    inline DCC_MODULE_TYPE getClassID() const override { return MAINLAYOUT; }

private:
    QScopedPointer<MainModulePrivate> d_ptr;
    Q_DECLARE_PRIVATE(MainModule)
    Q_DISABLE_COPY(MainModule)
};

}

class DccPluginTestWidget : public DMainWindow
{
    Q_OBJECT

public:
    enum class UrlType {
        Name,
        DisplayName
    };
    explicit DccPluginTestWidget(QWidget *parent = nullptr);
    ~DccPluginTestWidget();

    /**
     * @brief 显示路径请求的页面，用于搜索或DBus接口
     * @param url 路径地址,从左至右搜索，如路径错误，只显示已搜索出的模块
     */
    void showPage(const QString &url, const UrlType &uType);

protected:
    void changeEvent(QEvent *event) override;

private:
    void openManual();
    void initUI();
    void initConfig();
    void loadModules();
    void toHome();
    void updateMainView();
    void clearPage(QWidget *const widget);
    void configLayout(QBoxLayout *const layout);
    int getScrollPos(const int index);
    void showPage(ModuleObject *module, const QString &url, const UrlType &uType);
    void showModule(ModuleObject *module, const int index = -1);
    QWidget *getPage(QWidget *const widget, const QString &title);
    QWidget *getExtraPage(QWidget *const widget);

private Q_SLOTS:
    void onAddModule(ModuleObject *module);
    void onRemoveModule(ModuleObject *module);
    void onTriggered();
    void onChildStateChanged(ModuleObject *child, uint32_t flag, bool state);

private:
    Dtk::Widget::DIconButton *m_backwardBtn; //回退按钮
    ModuleObject *m_rootModule;
    QList<ModuleObject *> m_currentModule;
    QAbstractItemView *m_mainView; //保存主菜单view, 方便改变背景
    QList<QWidget *> m_pages;      //保存终点的页面
};

#endif // DCCPLUGINTESTWIDGET_H
