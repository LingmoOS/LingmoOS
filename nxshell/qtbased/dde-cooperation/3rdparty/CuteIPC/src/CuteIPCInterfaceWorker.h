#ifndef CUTEIPCINTERFACEWORKER_H
#define CUTEIPCINTERFACEWORKER_H

#include <QObject>
#include <QPointer>
class QIODevice;
class QHostAddress;

// Local
#include "CuteIPCMessage_p.h"

class CuteIPCInterfaceConnection;
class CuteIPCLoopVector;


class CuteIPCInterfaceWorker : public QObject
{
  Q_OBJECT

  public:
    explicit CuteIPCInterfaceWorker(QObject* parent = 0);
    ~CuteIPCInterfaceWorker();

    bool isConnected();
  signals:
    void setLastError(const QString& error);
    void disconnected();

    // slot finish signals
    void connectToServerFinished();
    void sendConnectionIdFinished();
    void disconnectFromServerFinished();
    void invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);

  public slots:
    void connectToServer(const QString& name, void* successful);
    void connectToTcpServer(const QHostAddress& host, const quint16 port, void* successful);

    void disconnectFromServer();

    void sendCallRequest(const QByteArray& request);
    QString connectionId() const;

  private:
    void sendRemoteConnectionRequest(const QString& signal);
    void sendSignalDisconnectRequest(const QString& signal);

    QPointer<CuteIPCInterfaceConnection> m_connection;
    QPointer<QIODevice> m_socket;
};

#endif // CUTEIPCINTERFACEWORKER_H
