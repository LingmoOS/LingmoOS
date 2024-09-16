#ifndef CUTEIPCINTERFACECONNECTION_P_H
#define CUTEIPCINTERFACECONNECTION_P_H

// Qt
#include <QObject>
#include <QLocalSocket>
#include <QTcpSocket>

// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMessage_p.h"


class CuteIPCInterfaceConnection : public QObject
{
  Q_OBJECT

  public:
    CuteIPCInterfaceConnection(QLocalSocket* socket, QObject* parent = 0);
    CuteIPCInterfaceConnection(QTcpSocket* socket, QObject* parent = 0);

    void sendCallRequest(const QByteArray& request);
    void setReturnedObject(QGenericReturnArgument returnedObject);
    bool lastCallSuccessful() const;
    bool isConnected();

  signals:
    void callFinished();
    void socketDisconnected();
    void invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);
    void errorOccured(const QString&);

  public slots:
    void readyRead();

    void errorOccured(QLocalSocket::LocalSocketError); // local socket error
    void errorOccured(QAbstractSocket::SocketError);   // network socket error

  private:
    QIODevice* m_socket;

    quint32 m_nextBlockSize;
    QByteArray m_block;

    bool m_lastCallSuccessful;
    QGenericReturnArgument m_returnedObject;

    bool readMessageFromSocket();
};

#endif // CUTEIPCINTERFACECONNECTION_P_H
