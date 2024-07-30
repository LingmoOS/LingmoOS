#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (!QDBusConnection::sessionBus().isConnected() || !QDBusConnection::sessionBus().interface()) {
        qCritical() << "Session bus not found";
        return 125;
    }

    qDebug() << "sending reparseConfiguration to object Konqueror in konqueror";
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/Konqueror"), //
                                                      QStringLiteral("org.kde.Konqueror"),
                                                      QStringLiteral("reparseConfiguration"));
    if (!QDBusConnection::sessionBus().send(message)) {
        qDebug() << "void expected, " << QDBusConnection::sessionBus().lastError().name() << " returned";
    }

    return 0;
}
