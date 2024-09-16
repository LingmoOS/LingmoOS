#ifndef CUTEIPCSERVICE_H
#define CUTEIPCSERVICE_H

#include "exportglobal.h"

// Qt
#include <QObject>
#include <QtNetwork/QHostAddress>
#include <QLocalServer>

// Local
class CuteIPCServicePrivate;

class EXPORT_API CuteIPCService : public QObject
{
  Q_OBJECT

  public:
    explicit CuteIPCService(QObject* parent = 0);
    ~CuteIPCService();

    bool listen(const QString& name = QString(), QObject* subject = 0, QLocalServer::SocketOptions options=QLocalServer::NoOptions);
    bool listen(QObject* subject, QLocalServer::SocketOptions options=QLocalServer::NoOptions);
    QString serverName() const;

    bool listenTcp(const QHostAddress& address = QHostAddress::Any, quint16 port = 0, QObject* subject = 0);
    bool listenTcp(QObject* subject);
    QHostAddress tcpAddress() const;
    quint16 tcpPort() const;

    void close();

  protected:
    CuteIPCServicePrivate* const d_ptr;
    CuteIPCService(CuteIPCServicePrivate& dd, QObject* parent);

  private:
    Q_DECLARE_PRIVATE(CuteIPCService)

    Q_PRIVATE_SLOT(d_func(),void _q_newLocalConnection())
    Q_PRIVATE_SLOT(d_func(),void _q_newTcpConnection())
    Q_PRIVATE_SLOT(d_func(),void _q_handleSignalRequest(QString,QString,QObject*))
    Q_PRIVATE_SLOT(d_func(),void _q_handleSignalDisconnect(QString,QString,QObject*))
    Q_PRIVATE_SLOT(d_func(),void _q_removeSignalHandler(QString))
    Q_PRIVATE_SLOT(d_func(),void _q_initializeConnection(QString,QObject*))
    Q_PRIVATE_SLOT(d_func(),void _q_connectionDestroyed(QObject*))
};

#endif // CUTEIPCSERVICE_H
