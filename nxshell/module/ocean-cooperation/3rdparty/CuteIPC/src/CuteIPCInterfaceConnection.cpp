// Local
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCMessage_p.h"

// Qt
#include <QLocalSocket>
#include <QDataStream>
#include <QTime>
#include <QMetaType>


CuteIPCInterfaceConnection::CuteIPCInterfaceConnection(QLocalSocket* socket, QObject* parent)
  : QObject(parent),
    m_socket(socket),
    m_nextBlockSize(0),
    m_lastCallSuccessful(false)
{
  connect(socket, SIGNAL(disconnected()), SIGNAL(socketDisconnected()));
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(errorOccured(QLocalSocket::LocalSocketError)));
  connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
}


CuteIPCInterfaceConnection::CuteIPCInterfaceConnection(QTcpSocket* socket, QObject* parent)
  : QObject(parent),
    m_socket(socket),
    m_nextBlockSize(0),
    m_lastCallSuccessful(false)
{
  connect(socket, SIGNAL(disconnected()), SIGNAL(socketDisconnected()));
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(errorOccured(QAbstractSocket::SocketError)));
  connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
}

bool CuteIPCInterfaceConnection::isConnected() {
  return m_socket && m_socket->isOpen();
}

void CuteIPCInterfaceConnection::sendCallRequest(const QByteArray& request)
{
  QDataStream stream(m_socket);
  stream << (quint32)request.size();
  int written = stream.writeRawData(request.constData(), request.size());

  if (written != request.size())
    qWarning() << "CuteIPC:" << "Warning:" << "Written bytes and request size doesn't match";

  if (QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(m_socket))
    socket->flush();
  else if (QLocalSocket* socket = qobject_cast<QLocalSocket*>(m_socket))
    socket->flush();

  m_lastCallSuccessful = true;
}


void CuteIPCInterfaceConnection::readyRead()
{
  bool messageStreamFinished;

  do
  {
    messageStreamFinished = readMessageFromSocket();
  } while (!messageStreamFinished);
}


bool CuteIPCInterfaceConnection::readMessageFromSocket()
{
  QDataStream in(m_socket);

  bool callWasFinished = false;

  // Fetch next block size
  if (m_nextBlockSize == 0)
  {
    if (m_socket->bytesAvailable() < (int)sizeof(quint32))
      return true;

    in >> m_nextBlockSize;
  }

  if (in.atEnd())
    return true;

  qint64 bytesToFetch = m_nextBlockSize - m_block.size();
  m_block.append(m_socket->read(bytesToFetch));

  if (m_block.size() == (int)m_nextBlockSize)
  {
    // Fetched enough, need to parse
    CuteIPCMessage::MessageType type = CuteIPCMarshaller::demarshallMessageType(m_block);

    switch (type)
    {
      case CuteIPCMessage::MessageResponse:
      {
        CuteIPCMessage message = CuteIPCMarshaller::demarshallResponse(m_block, m_returnedObject);
        callWasFinished = true;
        CuteIPCMarshaller::freeArguments(message.arguments());
        break;
      }
      case CuteIPCMessage::MessageError:
      {
        m_lastCallSuccessful = false;
        callWasFinished = true;
        CuteIPCMessage message = CuteIPCMarshaller::demarshallMessage(m_block);
        qWarning() << "CuteIPC:" << "Error:" << message.method();
        emit errorOccured(message.method());
        CuteIPCMarshaller::freeArguments(message.arguments());
        break;
      }
      case CuteIPCMessage::AboutToCloseSocket:
      {
        DEBUG << "The server reports that the connection is closed";
        CuteIPCMessage message = CuteIPCMarshaller::demarshallMessage(m_block);
        CuteIPCMarshaller::freeArguments(message.arguments());
        m_lastCallSuccessful = false;

        // Разрываем соединение с сервером. При этом отправляется сигнал socketDiconnected
        if (QTcpSocket* socket = qobject_cast<QTcpSocket*>(m_socket))
          socket->disconnectFromHost();
        else if (QLocalSocket* socket = qobject_cast<QLocalSocket*>(m_socket))
          socket->disconnectFromServer();

        break;
      }
      case CuteIPCMessage::MessageSignal:
      {
        CuteIPCMessage message = CuteIPCMarshaller::demarshallMessage(m_block);
        emit invokeRemoteSignal(message.method(), message.arguments());
        break;
      }
      default:
      {
        break;
        callWasFinished = true;
      }
    }

    m_nextBlockSize = 0;
    m_block.clear();

    if (callWasFinished)
      emit callFinished();

    if (m_socket->bytesAvailable())
      return false;
  }

  return true;
}


void CuteIPCInterfaceConnection::errorOccured(QLocalSocket::LocalSocketError)
{
  qWarning() << "CuteIPC" << "Socket error: " << m_socket->errorString();
  emit errorOccured(m_socket->errorString());
}


void CuteIPCInterfaceConnection::errorOccured(QAbstractSocket::SocketError)
{
  qWarning() << "CuteIPC" << "Socket error: " << m_socket->errorString();
  emit errorOccured(m_socket->errorString());
}


void CuteIPCInterfaceConnection::setReturnedObject(QGenericReturnArgument returnedObject)
{
  m_returnedObject = returnedObject;
}


bool CuteIPCInterfaceConnection::lastCallSuccessful() const
{
  return m_lastCallSuccessful;
}
