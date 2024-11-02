//#include <QCoreApplication>
//#include <QDebug>
//#include <QStringList>
//#include "/usr/include/x86_64-linux-gnu/qt5/QtDBus/QDBusConnection"
//#include "/usr/include/x86_64-linux-gnu/qt5/QtDBus/QDBusMessage"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCoreApplication>
#include <QDebug>
/**
 * @brief main
 * @param argc
 * @param argv
 * @return 此函数用来发送dbus信号 参数是日程id 一个日程id对应一个通知id 还有日程内容 通过char[]
 *
 */

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2) {
        qDebug() << "Usage: Dbus <message>";
        return 1;
    }
    QString summary = "您有日程";
    QString body = "您有日程";
    // 构造一个信号消息，注意这里不是创建信号，而是调用方法
    QDBusMessage methodCall = QDBusMessage::createMethodCall(
                "org.freedesktop.Notifications", // 服务名
                "/org/freedesktop/Notifications", // 对象路径
                "org.freedesktop.Notifications", // 接口名
                "Notify" // 方法名
                );
    // 按照Notify方法的参数要求，传递数据
    // 通常Notify方法需要以下参数：
    // uint32 appId, uint32 replacesId, string appIcon, string summary, string body, array{string} actions, array{uint32} hints, int32 timeout
    methodCall << "0" // 应用ID，通常设置为0
               << uint(0) // 替换ID，通常设置为0，除非你要替换一个之前的通知
               << "/home/lingmo/图片/GGbond.jpg" // 应用图标
               << summary // 摘要
               << body // 正文
               << QStringList() // 动作列表
               << QVariantMap() // 提示，比如设置通知的紧急程度等
               << -1; // 超时，设置为-1表示通知一直显示，直到用户关闭它

    // 发送消息
    if (!QDBusConnection::sessionBus().send(methodCall)) {
        qDebug() << "Failed to send DBus method call";
        return 1;
    }
//    QDBusConnection::sessionBus().send(methodCall);
    qDebug() << "DBus signal sent successfully";
    return 0;
    /*QCoreApplication a(argc, argv);

    // 输出所有命令行参数
    qDebug() << "Command line arguments:";
    for (int i = 0; i < argc; ++i) {
        qDebug() << argv[i];
    }



   /* return a.exec(); // 对于控制台应用，这里通常返回 0，但对于有事件循环的应用，使用 exec*/
}
