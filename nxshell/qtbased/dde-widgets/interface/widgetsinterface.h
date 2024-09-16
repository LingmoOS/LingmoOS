// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <widgetsglobal.h>
#include <QStringList>
#include <QVariant>
#include <QSize>
#include <QIcon>
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

WIDGETS_BEGIN_NAMESPACE
// dde-widget相关日志输出接口
Q_DECLARE_LOGGING_CATEGORY(dwLog)

class WidgetHandler;
class WidgetPluginSpec;
/**
 * @brief 组件类
 */
class Q_DECL_EXPORT IWidget
{
public:
    explicit IWidget();
    IWidget(const IWidget &w) = delete;
    IWidget(const IWidget &&w) = delete;
    IWidget &operator =(const IWidget &w) = delete;
    virtual ~IWidget();

    /**
     * @brief 组件尺寸类型，不同尺寸数据的数据类型保持兼容
     */
    enum Type {
        Invalid,
        Small,
        Middle,
        Large,
        Custom = 64
    };

    /**
     * @brief 组件的显示内容
     */
    virtual QWidget *view() = 0;

    /**
     * @brief 组件尺寸类型变化时被调用
     */
    virtual void typeChanged(const Type /*type*/) { }

    /**
     * @brief 组件获得信息的处理器
     */
    WidgetHandler *handler() const;

    /**
     * @brief 组件创建后被调用
     */
    virtual bool initialize(const QStringList &/*arguments*/) { return true;}

    /**
     * @brief 延迟初始化，不保证在主线程中调用
     */
    virtual void delayInitialize() {}

    /**
     * @brief 组件面板显示时被调用
     */
    virtual void showWidgets() {}

    /**
     * @brief 组建面板收起后被调用
     */
    virtual void hideWidgets() {}

    /**
     * @brief 组件移除时被调用
     */
    virtual void aboutToShutdown() {}

    /**
     * @brief 是否响应右键自定义菜单
     */
    virtual bool enableSettings() { return false;}

    /**
     * @brief 右键自定义配置菜单的响应事件
     */
    virtual void settings() {}

    /**
     * @brief 本地语言
     */
    static QString userInterfaceLanguage();
private:
    class Private;
    Private *d = nullptr;
    friend class WidgetPluginSpec;
};

/**
 * @brief 组件处理器，提供每个组件实例能够访问的接口
 */
class Q_DECL_EXPORT WidgetHandler
{
public:
    WidgetHandler() = default;
    virtual ~WidgetHandler() = default;

    /**
     * @brief 设置配置存储的值
     */
    virtual void setValue(const QString &key, const QVariant &value) = 0;

    /**
     * @brief 得到配置存储的值
     */
    virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const = 0;

    /**
     * @brief 重置配置存储的值
     */
    virtual void resetValue(const QString &key) = 0;

    /**
     * @brief 删除配置存储的值
     */
    virtual void removeValue(const QString &key) = 0;

    /**
     * @brief 判断是否含有配置存储值
     */
    virtual bool containsValue(const QString &key) = 0;

    /**
     * @brief 组件Id
     */
    virtual QString id() const = 0;

    /**
     * @brief 组件对应的插件Id
     */
    virtual QString pluginId() const = 0;

    /**
     * @brief 组件尺寸类型
     */
    virtual IWidget::Type type() const = 0;

    /**
     * @brief 组件应该设置的大小
     */
    virtual QSize size() const = 0;

    /**
     * @brief 组件应该设置的圆角半径
     */
    virtual int roundedCornerRadius() const = 0;
};

/**
 * @brief 插件类
 */
class Q_DECL_EXPORT IWidgetPlugin : public QObject
{
    Q_OBJECT
public:
    IWidgetPlugin() = default;
    virtual ~IWidgetPlugin() = default;

    /**
     * @brief 插件类型
     */
    enum Type {
        Normal, // 普通型，受用户编辑控制
        Resident, // 永驻型，不受用户编辑控制，内置插件类型，位置在Normal之上
        Alone, // 独立型，不受用户编辑控制，尺寸仅仅被控制最大值，位置在Resident之上，
    };

    /**
     * @brief 组件名称
     */
    virtual QString title() const = 0;

    /**
     * @brief 组件功能描述信息
     */
    virtual QString description() const { return QString();}

    /**
     * @brief 组件关于描述信息
     */
    virtual QString aboutDescription() const { return description();}

    /**
     * @brief 组件图片
     */
    virtual QIcon logo() const { return QIcon();}

    /**
     * @brief 组件贡献者
     */
    virtual QStringList contributors() const { return QStringList();}

    /**
     * @brief 创建组件实例，所属权归调用方所有
     */
    virtual IWidget *createWidget() = 0;

    /**
     * @brief 支持的尺寸类型组件实例
     */
    virtual QVector<IWidget::Type> supportTypes() const { return {IWidget::Small, IWidget::Middle, IWidget::Large};}

    /**
     * @brief 插件类型
     */
    virtual IWidgetPlugin::Type type() const { return Normal;}
};

WIDGETS_END_NAMESPACE

#define DdeWidgetsPlugin_iid "org.deepin.dde.widgets.PluginInterface"

Q_DECLARE_INTERFACE(WIDGETS_NAMESPACE::IWidgetPlugin, DdeWidgetsPlugin_iid)
