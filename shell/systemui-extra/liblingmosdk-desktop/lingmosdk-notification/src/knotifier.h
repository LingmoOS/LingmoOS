#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <QObject>

namespace kdk {

class KNotifierPrivate;

/**
 * @defgroup NotificationModule
 */

/**
 * @brief 消息通知类，调用notify()发消息后会在屏幕右上角以弹窗形式展示消息内容和支持的操作
 * 支持设置应用名称，应用图标，消息标题，消息主体内容，显示时长，添加action等
 */

class KNotifier : public QObject
{
    Q_OBJECT

public:

    enum ShowTime
    {
        Default = -1, //系统默认显示消息时长
        AllTheTime = 0 //消息常驻
    };
    explicit KNotifier(QObject *parent = nullptr);
    ~KNotifier();

    /**
     * @brief 发送消息通知请求
     * @return 返回消息通知id
     */
    uint notify();

    /**
     * @brief 设置默认跳转应用，点击消息弹窗时进行跳转
     * @param 应用名称
     */
    void setDefaultAction(const QString& appName);

    /**
     * @brief 在消息弹窗中添加跳转按钮，最多可以添加三个
     * @param 按钮显示文本
     * @param 跳转应用名称
     */
    void addAction(const QString& appName, const QString& text);

    /**
     * @brief 设置消息弹窗的显示时长，Default为系统默认时长，AllTheTime为常驻消息
     * @param 毫秒数
     */
    void setShowTime(int milliseconds);

    /**
     * @brief 设置消息弹窗应用名称
     * @param 应用名称
     */
    void setAppName(const QString& appName);

    /**
     * @brief 设置消息弹窗主内容
     * @param 主内容
     */
    void setBodyText(const QString& bodyText);

    /**
     * @brief 设置消息弹窗标题
     * @param 标题
     */
    void setSummary(const QString& summary);

    /**
     * @brief 设置消息弹窗应用图标，仅支持系统图标
     * @param 图标名称
     */
    void setAppIcon(const QString& iconName);

    /**
     * @brief 设置替换消息弹窗的id替换通知即更新通知内容，在通知还未消失时，更新通知弹窗的主题、正文、跳转动作和按钮。
     * @param id
     */
    void setReplaceId(const uint id);

    /**
     * @brief 主动关闭消息弹窗
     * @param id
     */
    static void closeNotification(uint id);

private:
    Q_DECLARE_PRIVATE(KNotifier)
    KNotifierPrivate* const d_ptr;

};

}

#endif // NOTIFIER_H
