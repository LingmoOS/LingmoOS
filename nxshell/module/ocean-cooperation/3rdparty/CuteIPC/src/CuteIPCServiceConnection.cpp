// Local
#include "CuteIPCServiceConnection_p.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QLocalSocket>
#include <QTcpSocket>
#include <QDataStream>
#include <QTime>
#include <QMetaType>


CuteIPCServiceConnection::CuteIPCServiceConnection(QLocalSocket* socket, CuteIPCService* parent)
  : QObject(parent),
    m_socket(socket),
    m_nextBlockSize(0),
    m_subject(0)
{
  // Delete connection after the socket have been disconnected
  connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
  connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
  connect(this, SIGNAL(destroyed(QObject*)), parent, SLOT(_q_connectionDestroyed(QObject*)));
  connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(errorOccured(QLocalSocket::LocalSocketError)));
  connect(this, SIGNAL(signalRequest(QString,QString,QObject*)), parent, SLOT(_q_handleSignalRequest(QString,QString,QObject*)));
  connect(this, SIGNAL(signalDisconnectRequest(QString,QString,QObject*)),
          parent, SLOT(_q_handleSignalDisconnect(QString,QString,QObject*)));
  connect(this, SIGNAL(connectionInitializeRequest(QString,QObject*)), parent, SLOT(_q_initializeConnection(QString,QObject*)));

  connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
  if (socket->state() != QLocalSocket::ConnectedState || !socket->isReadable() || !socket->isWritable())
  {
    qWarning() << "CuteIPC:" << "Socket was not opened corectly. We tried to open again";
    socket->open(QIODevice::ReadWrite);
  }

  if (!socket->isOpen())
  {
    qWarning() << "CuteIPC:" << "Failed to open socket in ReadWrite mode:" << socket->errorString();
    deleteLater();
  }
}


CuteIPCServiceConnection::CuteIPCServiceConnection(QTcpSocket* socket, CuteIPCService* parent)
  : QObject(parent),
    m_socket(socket),
    m_nextBlockSize(0),
    m_subject(0)
{
  // Delete connection after the socket have been disconnected
  connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
  connect(socket, SIGNAL(disconnected()), SLOT(deleteLater()));
  connect(this, SIGNAL(destroyed(QObject*)), parent, SLOT(_q_connectionDestroyed(QObject*)));
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(errorOccured(QAbstractSocket::SocketError)));

  connect(this, SIGNAL(signalRequest(QString,QString,QObject*)), parent, SLOT(_q_handleSignalRequest(QString,QString,QObject*)));
  connect(this, SIGNAL(signalDisconnectRequest(QString,QString,QObject*)),
          parent, SLOT(_q_handleSignalDisconnect(QString,QString,QObject*)));
  connect(this, SIGNAL(connectionInitializeRequest(QString,QObject*)), parent, SLOT(_q_initializeConnection(QString,QObject*)));

  connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
  if (socket->state() != QAbstractSocket::ConnectedState || !socket->isReadable() || !socket->isWritable())
  {
    qWarning() << "CuteIPC:" << "Socket was not opened corectly. We tried to open again";
    socket->open(QIODevice::ReadWrite);
  }

  if (!socket->isOpen())
  {
    qWarning() << "CuteIPC:" << "Failed to open socket in ReadWrite mode:" << socket->errorString();
    deleteLater();
  }
}


CuteIPCServiceConnection::~CuteIPCServiceConnection()
{}


void CuteIPCServiceConnection::setSubject(QObject* subject)
{
  m_subject = subject;
}


void CuteIPCServiceConnection::readyRead()
{
  bool messageStreamFinished;

  do
  {
    messageStreamFinished = readMessageFromSocket();
  } while (!messageStreamFinished);
}


bool CuteIPCServiceConnection::readMessageFromSocket()
{
  QDataStream in(m_socket);

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
    processMessage();

    // Cleanup
    m_nextBlockSize = 0;
    m_block.clear();
  }

  if (m_socket->bytesAvailable())
    return false;
  return true;
}


void CuteIPCServiceConnection::processMessage()
{
  CuteIPCMessage call = CuteIPCMarshaller::demarshallMessage(m_block);
  CuteIPCMessage::MessageType messageType = call.messageType();
  DEBUG << call;

  QObject* subject = m_subject ? m_subject : parent();

  // Fill empty args
  CuteIPCMessage::Arguments args = call.arguments();
  while (args.size() < 10)
    args.append(QGenericArgument());

  if (messageType == CuteIPCMessage::MessageCallWithReturn && !call.returnType().isEmpty())
  {
    int retType = QMetaType::type(call.returnType().toLatin1());
    if (retType > 0)
    {
      // Read argument data from stream
#if QT_VERSION >= 0x050000
      void* retData = QMetaType::create(retType);
#else
      void* retData = QMetaType::construct(retType);
#endif

      DEBUG << "Invoke local method:" << call.method();
      bool successfulInvoke = QMetaObject::invokeMethod(subject, call.method().toLatin1(),
          QGenericReturnArgument(call.returnType().toLatin1(), retData),
          args.at(0), args.at(1), args.at(2), args.at(3), args.at(4), args.at(5),
          args.at(6), args.at(7), args.at(8), args.at(9));

      if (successfulInvoke)
      {
        sendResponseMessage(call.method(), QGenericArgument(call.returnType().toLatin1(), retData));
      }
      else
      {
        sendErrorMessage("Unsuccessful invoke");
      }

      QMetaType::destroy(retType, retData);
    }
    else
    {
      QString error = "Unsupported type of expected return value: " + call.returnType();
      qWarning() << "CuteIPC:" << error;
      sendErrorMessage(error);
    }
  }
  else if ((messageType == CuteIPCMessage::MessageCallWithReturn && call.returnType().isEmpty())
           || messageType == CuteIPCMessage::MessageCallWithoutReturn)
  {
    bool successfulInvoke = QMetaObject::invokeMethod(subject, call.method().toLatin1(),
        args.at(0), args.at(1), args.at(2), args.at(3), args.at(4), args.at(5),
        args.at(6), args.at(7), args.at(8), args.at(9));
    if (!successfulInvoke)
    {
      sendErrorMessage("Unsuccessful invoke");
    }
    else
    {
      if (messageType == CuteIPCMessage::MessageCallWithReturn)
        sendResponseMessage(call.method());
    }
  }
  else if (messageType == CuteIPCMessage::MessageSignal)
  {
    bool successfulInvoke = QMetaObject::invokeMethod(subject,
        call.method().left(call.method().indexOf("(")).toLatin1(),
        args.at(0), args.at(1), args.at(2), args.at(3), args.at(4), args.at(5),
        args.at(6), args.at(7), args.at(8), args.at(9));
    if (!successfulInvoke)
      sendErrorMessage("Unsuccessful invoke");
  }
  else if (messageType == CuteIPCMessage::SignalConnectionRequest)
  {
#if QT_VERSION >= 0x050000
    void* connectionId = QMetaType::create(QMetaType::QString, args.at(0).data());
#else

    void* connectionId = QMetaType::construct(QMetaType::QString, args.at(0).data());
#endif

    if (call.returnType() != QString("disconnect"))
      emit signalRequest(call.method(), *((QString*)(connectionId)), this);
    else
      emit signalDisconnectRequest(call.method(), *((QString*)(connectionId)), this);

    QMetaType::destroy(QMetaType::QString, connectionId);
  }
  else if (messageType == CuteIPCMessage::SlotConnectionRequest)
  {
    if (subject->metaObject()->indexOfSlot(QMetaObject::normalizedSignature(call.method().toLatin1())) == -1)
      sendErrorMessage("Remote slot doesn't exist:" + call.method());
    else
      sendResponseMessage(call.method());
  }
  else if (messageType == CuteIPCMessage::ConnectionInitialize)
  {
#if QT_VERSION >= 0x050000
    void* connectionId = QMetaType::create(QMetaType::QString, args.at(0).data());
#else
    void* connectionId = QMetaType::construct(QMetaType::QString, args.at(0).data());
#endif

    emit connectionInitializeRequest(*((QString*)(connectionId)), this);

    QMetaType::destroy(QMetaType::QString, connectionId);
  }


  // Cleanup
  CuteIPCMarshaller::freeArguments(call.arguments());
}


void CuteIPCServiceConnection::sendErrorMessage(const QString& error)
{
  CuteIPCMessage message(CuteIPCMessage::MessageError, error);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);
  sendResponse(request);
  qWarning() << "CuteIPC:" << "Error message was sent:" << error;
}


void CuteIPCServiceConnection::sendResponseMessage(const QString& method, QGenericArgument arg)
{
  CuteIPCMessage message(CuteIPCMessage::MessageResponse, method, arg);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  sendResponse(request);
  //  qDebug() << "Returned value was sent";
}


void CuteIPCServiceConnection::sendAboutToQuit()
{
  CuteIPCMessage message(CuteIPCMessage::AboutToCloseSocket);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  DEBUG << "Send aboutToClose notification";

  if (m_socket->isOpen()) {
    sendResponse(request);
  }
}


void CuteIPCServiceConnection::sendResponse(const QByteArray& response)
{
  QDataStream stream(m_socket);
  stream << (quint32)response.size();
  int written = stream.writeRawData(response.constData(), response.size());

  if (written != response.size())
    qWarning() << "CuteIPC:" << "Socket error: Written bytes and request size doesn't match";

  if (QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(m_socket))
    socket->flush();
  else if (QLocalSocket* socket = qobject_cast<QLocalSocket*>(m_socket))
    socket->flush();
}


void CuteIPCServiceConnection::errorOccured(QLocalSocket::LocalSocketError error)
{
  // Connection closed by peer is normal situation: it just notifies us that the remote client have been disconnected
  if (error != QLocalSocket::PeerClosedError)
    qWarning() << "CuteIPC:" << "Socket error: " << m_socket->errorString();

  deleteLater();
}


void CuteIPCServiceConnection::errorOccured(QAbstractSocket::SocketError error)
{
  // Connection closed by peer is normal situation: it just notifies us that the remote client have been disconnected
  if (error != QAbstractSocket::RemoteHostClosedError)
    qWarning() << "CuteIPC:" << "Socket error: " << m_socket->errorString();

  deleteLater();
}


void CuteIPCServiceConnection::sendSignal(const QByteArray& data)
{
  sendResponse(data);
}
