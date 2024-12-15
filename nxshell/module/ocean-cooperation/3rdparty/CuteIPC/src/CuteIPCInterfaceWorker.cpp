#include "CuteIPCInterfaceWorker.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QEventLoop>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QHostAddress>


CuteIPCInterfaceWorker::CuteIPCInterfaceWorker(QObject* parent)
  : QObject(parent)
{}


CuteIPCInterfaceWorker::~CuteIPCInterfaceWorker()
{
  if (m_socket)
    delete m_socket;
}


void CuteIPCInterfaceWorker::connectToServer(const QString& name, void* successful)
{
  // TODO: Add checking for existing connection

  QLocalSocket* socket = new QLocalSocket;
  socket->connectToServer(name);

  bool connected = socket->waitForConnected(5000);
  if (!connected)
  {
    socket->disconnectFromServer();
    delete socket;
  }
  else
  {
    m_socket = socket;
    m_connection = new CuteIPCInterfaceConnection(socket, this);
    connect(m_connection, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)),
            this, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)));
    connect(m_connection, SIGNAL(errorOccured(QString)), this, SIGNAL(setLastError(QString)));

    connect(m_connection, SIGNAL(socketDisconnected()), SIGNAL(disconnected()));
    connect(m_connection, SIGNAL(socketDisconnected()), m_connection, SLOT(deleteLater()));
    connect(m_connection, SIGNAL(socketDisconnected()), socket, SLOT(deleteLater()));

    DEBUG << "CuteIPC:" << "Connected:" << name << connected;

    // Register connection ID on the serverside
    QString id = connectionId();
    CuteIPCMessage message(CuteIPCMessage::ConnectionInitialize, "", Q_ARG(QString, id));
    QByteArray request = CuteIPCMarshaller::marshallMessage(message);

    DEBUG << "Send connection ID to the server:" << id;

    QEventLoop loop;
    QObject::connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
    QObject::connect(m_connection, SIGNAL(socketDisconnected()), &loop, SLOT(quit()));
    m_connection->sendCallRequest(request);
    loop.exec();

    bool ok = m_connection->lastCallSuccessful();
    if (!ok)
      qWarning() << "CuteIPC:" << "Error: send connection ID failed. Remote signal connections will be unsuccessful";
  }

  *reinterpret_cast<bool*>(successful) = connected;
  emit connectToServerFinished();
}


void CuteIPCInterfaceWorker::connectToTcpServer(const QHostAddress& host, const quint16 port, void *successful)
{
  QTcpSocket* socket = new QTcpSocket;
  socket->connectToHost(host, port);
  bool connected = socket->waitForConnected(5000);
  if (!connected)
  {
    socket->disconnectFromHost();
    delete socket;
  }
  else
  {
    m_socket = socket;
    m_connection = new CuteIPCInterfaceConnection(socket, this);
    connect(m_connection, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)),
            this, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)));
    connect(m_connection, SIGNAL(errorOccured(QString)), this, SIGNAL(setLastError(QString)));

    connect(m_connection, SIGNAL(socketDisconnected()), SIGNAL(disconnected()));
    connect(m_connection, SIGNAL(socketDisconnected()), m_connection, SLOT(deleteLater()));
    connect(m_connection, SIGNAL(socketDisconnected()), socket, SLOT(deleteLater()));

    DEBUG << "CuteIPC:" << "Connected over network:" << host << port << connected;

    // Register connection ID on the serverside
    QString id = connectionId();
    CuteIPCMessage message(CuteIPCMessage::ConnectionInitialize, "", Q_ARG(QString, id));
    QByteArray request = CuteIPCMarshaller::marshallMessage(message);

    DEBUG << "Send connection ID to the server:" << id;

    QEventLoop loop;
    QObject::connect(m_connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
    QObject::connect(m_connection, SIGNAL(socketDisconnected()), &loop, SLOT(quit()));
    m_connection->sendCallRequest(request);
    loop.exec();

    bool ok = m_connection->lastCallSuccessful();
    if (!ok)
      qWarning() << "CuteIPC:" << "Error: send connection ID failed. Remote signal connections will be unsuccessful";
  }

  *reinterpret_cast<bool*>(successful) = connected;
  emit connectToServerFinished();
}


void CuteIPCInterfaceWorker::disconnectFromServer()
{
  if (!m_socket)
    return;

  if (QTcpSocket* socket = qobject_cast<QTcpSocket*>(m_socket))
    socket->disconnectFromHost();
  else if (QLocalSocket* socket = qobject_cast<QLocalSocket*>(m_socket))
    socket->disconnectFromServer();

  delete m_socket;
  emit disconnectFromServerFinished();
}


bool CuteIPCInterfaceWorker::isConnected() {
  return m_connection->isConnected();
}

void CuteIPCInterfaceWorker::sendCallRequest(const QByteArray& request)
{
  if (!m_connection)
    return;

  m_connection->sendCallRequest(request);
}


QString CuteIPCInterfaceWorker::connectionId() const
{
  return QString::number(reinterpret_cast<quintptr>(m_connection.data()));
}
