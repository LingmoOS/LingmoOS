#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QIcon>
#include <QQmlComponent>
#include <QQuickStyle>

// 添加通知相关头文件
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QFile>

// 添加翻译支持
#include <QTranslator>
#include <QLocale>

class Notification : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyChanged)
    Q_PROPERTY(bool hasSound READ hasSound WRITE setHasSound NOTIFY hasSoundChanged)
    Q_PROPERTY(bool silent READ silent WRITE setSilent NOTIFY silentChanged)

public:
    explicit Notification(QObject *parent = nullptr) : QObject(parent) {}

    QString title() const { return m_title; }
    void setTitle(const QString &title) {
        if (m_title != title) {
            m_title = title;
            emit titleChanged();
        }
    }

    QString body() const { return m_body; }
    void setBody(const QString &body) {
        if (m_body != body) {
            m_body = body;
            emit bodyChanged();
        }
    }

    bool hasSound() const { return m_hasSound; }
    void setHasSound(bool hasSound) {
        if (m_hasSound != hasSound) {
            m_hasSound = hasSound;
            emit hasSoundChanged();
        }
    }

    bool silent() const { return m_silent; }
    void setSilent(bool silent) {
        if (m_silent != silent) {
            m_silent = silent;
            emit silentChanged();
        }
    }

    Q_INVOKABLE void show() {
        QDBusInterface iface("org.freedesktop.Notifications",
                           "/org/freedesktop/Notifications",
                           "org.freedesktop.Notifications",
                           QDBusConnection::sessionBus());
        if (iface.isValid()) {
            QVariantMap hints;
            hints["sound-name"] = m_hasSound ? "message" : "";
            hints["suppress-sound"] = m_silent;

            QList<QVariant> args;
            args << "lingmo-clock"                 // app_name
                 << (unsigned int) 0               // replaces_id
                 << "lingmo-clock"                 // app_icon
                 << m_title                        // summary
                 << m_body                         // body
                 << QStringList()                  // actions
                 << hints                          // hints
                 << (int) 3000;                    // timeout

            iface.callWithArgumentList(QDBus::NoBlock, "Notify", args);
        }
    }

signals:
    void titleChanged();
    void bodyChanged();
    void hasSoundChanged();
    void silentChanged();

private:
    QString m_title;
    QString m_body;
    bool m_hasSound = true;
    bool m_silent = false;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme("lingmo-clock"));

    // 设置应用程序属性
    app.setOrganizationName("lingmo");
    app.setOrganizationDomain("lingmo.org");
    app.setApplicationName("lingmo-clock");

    // 注册通知类型
    qmlRegisterType<Notification>("Lingmo.Notification", 1, 0, "Notification");

    // 添加翻译支持
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-clock/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(app.instance());
        if (translator->load(qmFilePath)) {
        app.installTranslator(translator);
        }
        else {
        translator->deleteLater();
        }
    }

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}

#include "main.moc" 