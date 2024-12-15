// Local
#include "CuteIPCInterface.h"
#include "CuteIPCInterface_p.h"
#include "CuteIPCMarshaller_p.h"
#include "CuteIPCInterfaceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCSignalHandler_p.h"
#include "CuteIPCInterfaceWorker.h"

// Qt
#include <QThread>
#include <QEventLoop>
#include <QHostAddress>


/*!
    \class CuteIPCInterface

    \brief The CuteIPCInterface class provides an IPC client
    used to send remote call requests and set connections between remote
    signals and slots.

    It is based on [QLocalSocket](http://doc.qt.io/qt-5/qlocalsocket.html)
    and [QTcpSocket](http://doc.qt.io/qt-5/qtcpsocket.html).
    To connect to the server, call connectToServer() method.

    Use call() and callNoReply() methods to send method invoke requests to the server
    (which are synchronous and asynchronous respectively).
    The signature of these methods concurs with QMetaObject::invokeMethod() method signature.
    Thus, you can invoke remote methods the same way as you did it locally through the QMetaObject.

    You can also use a remoteConnect() to connect the remote signal to the slot or signal
    of some local object.

    Contrarily, you can connect the local signal to the remote slot, by using
    remoteSlotConnect().

    \sa CuteIPCService
*/

CuteIPCInterfacePrivate::CuteIPCInterfacePrivate()
  : m_workerThread(new QThread),
    m_worker(new CuteIPCInterfaceWorker)
{
  m_worker->moveToThread(m_workerThread);
  m_workerThread->start();
}


CuteIPCInterfacePrivate::~CuteIPCInterfacePrivate()
{
  m_workerThread->quit();
  m_workerThread->wait();

  delete m_worker;
  delete m_workerThread;
}


bool CuteIPCInterfacePrivate::checkConnectCorrection(const QString& signal, const QString& method)
{
  if (signal[0] != '2' || (method[0] != '1' && method[0] != '2'))
    return false;

  QString signalSignature = signal.mid(1);
  QString methodSignature = method.mid(1);

  if (!QMetaObject::checkConnectArgs(signalSignature.toLatin1(), methodSignature.toLatin1()))
  {
    qWarning() << "CuteIPC:" << "Error: incompatible signatures" << signalSignature << methodSignature;
    m_lastError = "Incompatible signatures: " + signalSignature + "," + methodSignature;
    return false;
  }
  return true;
}


bool CuteIPCInterfacePrivate::checkRemoteSlotExistance(const QString& slot)
{
  DEBUG << "Check remote slot existence" << slot;
  CuteIPCMessage message(CuteIPCMessage::SlotConnectionRequest, slot);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);
  return sendSynchronousRequest(request);
}


bool CuteIPCInterfacePrivate::sendRemoteConnectRequest(const QString& signalSignature)
{
  QString connectionId = m_worker->connectionId();

  DEBUG << "Requesting connection to signal" << signalSignature << "Worker connection ID: " << connectionId;

  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, signalSignature, Q_ARG(QString, connectionId));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  return sendSynchronousRequest(request);
}


bool CuteIPCInterfacePrivate::sendRemoteDisconnectRequest(const QString& signalSignature)
{
  DEBUG << "Requesting remote signal disconnect" << signalSignature;

  QString connectionId = m_worker->connectionId();
  CuteIPCMessage::Arguments args;
  args.push_back(Q_ARG(QString, connectionId));
  CuteIPCMessage message(CuteIPCMessage::SignalConnectionRequest, signalSignature, args, "disconnect");
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  return sendSynchronousRequest(request);
}


bool CuteIPCInterfacePrivate::sendSynchronousRequest(const QByteArray& request, QGenericReturnArgument returnedObject)
{
  Q_Q(CuteIPCInterface);

  if (!m_localServer.isEmpty())
  {
    QLocalSocket socket;
    socket.connectToServer(m_localServer);
    bool connected = socket.waitForConnected(5000);
    if (!connected)
    {
      socket.disconnectFromServer();
      QString error("CuteIPC: Could not connect to the server when the synchronous method was called");
      qWarning() << error;
      _q_setLastError(error);
      return false;
    }

    CuteIPCInterfaceConnection connection(&socket);
    QObject::connect(&connection, SIGNAL(errorOccured(QString)), q, SLOT(_q_setLastError(QString)));
    connection.setReturnedObject(returnedObject);

    QEventLoop loop;
    QObject::connect(&connection, SIGNAL(callFinished()), &loop, SLOT(quit()));

    QObject::connect(&connection, SIGNAL(socketDisconnected()), q_ptr, SIGNAL(disconnected()));
    QObject::connect(&connection, SIGNAL(socketDisconnected()), &loop, SLOT(quit()));
    connection.sendCallRequest(request);
    loop.exec();

    return connection.lastCallSuccessful();
  }
  else if (!m_tcpAddress.first.isNull())
  {
    QTcpSocket socket;
    socket.connectToHost(m_tcpAddress.first, m_tcpAddress.second);
    bool connected = socket.waitForConnected(5000);
    if (!connected)
    {
      socket.disconnectFromHost();
      QString error("CuteIPC: Could not connect to the server when the synchronous method was called");
      qWarning() << error;
      _q_setLastError(error);
      return false;
    }

    CuteIPCInterfaceConnection connection(&socket);
    QObject::connect(&connection, SIGNAL(errorOccured(QString)), q, SLOT(_q_setLastError(QString)));
    connection.setReturnedObject(returnedObject);

    QEventLoop loop;
    QObject::connect(&connection, SIGNAL(callFinished()), &loop, SLOT(quit()));
    QObject::connect(&connection, SIGNAL(socketDisconnected()), &loop, SLOT(quit()));
    connection.sendCallRequest(request);
    loop.exec();

    return connection.lastCallSuccessful();
  }

  return false;
}


void CuteIPCInterfacePrivate::_q_setLastError(QString lastError)
{
  this->m_lastError = lastError;
}


void CuteIPCInterfacePrivate::_q_invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments)
{
  QList<MethodData> recieversData = m_connections.values(signalSignature);
  foreach (const MethodData& data, recieversData)
  {
    if (!data.first)
    {
      CuteIPCMarshaller::freeArguments(arguments);
      return;
    }

    DEBUG << "Invoke local method: " << data.second;

    QString methodName = data.second;
    methodName = methodName.left(methodName.indexOf("("));

    CuteIPCMessage::Arguments args = arguments;
    while (args.size() < 10)
      args.append(QGenericArgument());

    QMetaObject::invokeMethod(data.first, methodName.toLatin1(), Qt::QueuedConnection,
                              args.at(0), args.at(1), args.at(2), args.at(3), args.at(4), args.at(5), args.at(6),
                              args.at(7), args.at(8), args.at(9));
  }

  CuteIPCMarshaller::freeArguments(arguments);
}


void CuteIPCInterfacePrivate::handleLocalSignalRequest(QObject* localObject, const QString& signalSignature,
                                                       const QString& slotSignature)
{
  Q_Q(CuteIPCInterface);

  MethodData data(localObject, signalSignature);

  QList<CuteIPCSignalHandler*> handlers = m_localSignalHandlers.values(data);
  CuteIPCSignalHandler* handler = 0;
  foreach (CuteIPCSignalHandler* existingHandler, handlers)
  {
    if (existingHandler->signature() == slotSignature)
      handler = existingHandler;
  }

  if (!handler)
  {
    handler = new CuteIPCSignalHandler(slotSignature, q);
    handler->setSignalParametersInfo(localObject, signalSignature);

    m_localSignalHandlers.insert(data, handler);

    QMetaObject::connect(localObject,
        localObject->metaObject()->indexOfSignal("destroyed(QObject*)"),
        q, q->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("_q_removeSignalHandlersOfObject(QObject*)")));

    QMetaObject::connect(localObject,
        localObject->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(signalSignature.toLatin1())),
        handler, handler->metaObject()->indexOfSlot("relaySlot()"));

    QMetaObject::connect(
        handler, handler->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("signalCaptured(QByteArray)")),
        q, q->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("_q_sendAsynchronousRequest(QByteArray)")));
  }
}


void CuteIPCInterfacePrivate::_q_removeSignalHandlersOfObject(QObject* destroyedObject)
{
  auto it = m_localSignalHandlers.begin();
  while (it != m_localSignalHandlers.end()) {
    MethodData data = it.key();
    if (data.first == destroyedObject)
      it = m_localSignalHandlers.erase(it);
    else
      ++it;
  }
}


void CuteIPCInterfacePrivate::_q_sendAsynchronousRequest(const QByteArray& request)
{
  QMetaObject::invokeMethod(m_worker, "sendCallRequest", Q_ARG(QByteArray, request));
}


void CuteIPCInterfacePrivate::registerConnection(const QString& signalSignature, QObject* reciever, const QString& methodSignature)
{
  Q_Q(CuteIPCInterface);
  m_connections.insert(signalSignature, MethodData(reciever, methodSignature));
  QObject::connect(reciever, SIGNAL(destroyed(QObject*)), q, SLOT(_q_removeRemoteConnectionsOfObject(QObject*)));
}


void CuteIPCInterfacePrivate::_q_removeRemoteConnectionsOfObject(QObject* destroyedObject)
{
  QMutableHashIterator<QString, MethodData> i(m_connections);
  while (i.hasNext())
  {
    i.next();
    MethodData data = i.value();
    if (data.first == destroyedObject)
      i.remove();
  }
}


/*!
    Creates a new CuteIPCInterface object with the given \a parent.

    \sa connectToServer()
 */
CuteIPCInterface::CuteIPCInterface(QObject* parent)
  : QObject(parent),
    d_ptr(new CuteIPCInterfacePrivate())
{
  Q_D(CuteIPCInterface);
  d->q_ptr = this;

  connect(d->m_worker, SIGNAL(disconnected()), SIGNAL(disconnected()));
  connect(d->m_worker, SIGNAL(setLastError(QString)), SLOT(_q_setLastError(QString)));
  connect(d->m_worker, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)),
          SLOT(_q_invokeRemoteSignal(QString, CuteIPCMessage::Arguments)));

  qRegisterMetaType<QGenericReturnArgument>("QGenericReturnArgument");
  qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
  qRegisterMetaType<CuteIPCMessage::Arguments>("CuteIPCMessage::Arguments");
  qRegisterMetaType<QHostAddress>("QHostAddress");
}


CuteIPCInterface::CuteIPCInterface(CuteIPCInterfacePrivate& dd, QObject* parent)
  : QObject(parent),
    d_ptr(&dd)
{
  Q_D(CuteIPCInterface);
  d->q_ptr = this;

  connect(d->m_worker, SIGNAL(setLastError(QString)), SLOT(_q_setLastError(QString)));
  connect(d->m_worker, SIGNAL(invokeRemoteSignal(QString, CuteIPCMessage::Arguments)),
          SLOT(_q_invokeRemoteSignal(QString, CuteIPCMessage::Arguments)));

  qRegisterMetaType<QGenericReturnArgument>("QGenericReturnArgument");
  qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
  qRegisterMetaType<CuteIPCMessage::Arguments>("CuteIPCMessage::Arguments");
  qRegisterMetaType<QHostAddress>("QHostAddress");
}


bool CuteIPCInterface::isConnected() {
  Q_D(CuteIPCInterface);
  return d->m_worker->isConnected();
}

/*!
    Destroyes the object.
 */
CuteIPCInterface::~CuteIPCInterface()
{
  delete d_ptr;
}


/*!
  Attempts to make a connection to the local server with a given \a name.
  Returns true on success, otherwise false.
  \param name Local server name
  \sa CuteIPCInterface::connectToServer(const QHostAddress &host, quint16 port)
  \sa CuteIPCService::serverName()
 */
bool CuteIPCInterface::connectToServer(const QString& name)
{
  Q_D(CuteIPCInterface);
  bool isConnected;

  QEventLoop loop;
  connect(d->m_worker, SIGNAL(connectToServerFinished()), &loop, SLOT(quit()));
  QMetaObject::invokeMethod(d->m_worker, "connectToServer", Q_ARG(QString, name), Q_ARG(void*, &isConnected));
  loop.exec();

  d->m_localServer = name;
  if (isConnected)
      emit connected();

  return isConnected;
}

/*!
  Attempts to make a connection to the TCP server with a given \a host and \a port.
  Returns true on success, otherwise false.
  \param host Server host
  \param port Server tcp port
  \sa CuteIPCInterface::connectToServer(const QString& name)
  \sa CuteIPCService::serverName()
 */
bool CuteIPCInterface::connectToServer(const QHostAddress& host, quint16 port)
{
  Q_D(CuteIPCInterface);
  bool isConnected;

  QEventLoop loop;
  connect(d->m_worker, SIGNAL(connectToServerFinished()), &loop, SLOT(quit()));
  QMetaObject::invokeMethod(d->m_worker, "connectToTcpServer", Q_ARG(QHostAddress, host), Q_ARG(quint16, port), Q_ARG(void*, &isConnected));
  loop.exec();

  d->m_tcpAddress = qMakePair(host, port);
  if (isConnected)
      emit connected();

  return isConnected;
}


/*!
    Disconnects from server by closing the socket.
 */
void CuteIPCInterface::disconnectFromServer()
{
  Q_D(CuteIPCInterface);
  QEventLoop loop;
  connect(d->m_worker, SIGNAL(disconnectFromServerFinished()), &loop, SLOT(quit()));
  QMetaObject::invokeMethod(d->m_worker, "disconnectFromServer");
  loop.exec();
}


/*!
    The method is used to connect the remote signal (on the server-side) to the slot or signal
    of some local object.
    It returns true on success. False otherwise (the slot doesn't exist,
    or signatures are incompatible, or the server replies the error).

    After the connection being established, signals will be delivered asynchronously.

    \note It is recommended to use this method the same way as you call QObject::connect() method
    (by using SIGNAL() and SLOT() macros).
    \par
    For example, to connect the remote \a exampleSignal() signal to the \a exampleSlot() of some local \a object,
    you can type:
    \code remoteConnect(SIGNAL(exampleSignal()), object, SLOT(exampleSlot())); \endcode

    \note This method doesn't establish the connection to the server, you must use connectToServer() first.

    \sa remoteSlotConnect()
 */
bool CuteIPCInterface::remoteConnect(const char* signal, QObject* object, const char* method)
{
  Q_D(CuteIPCInterface);

  if (!object)
  {
    d->m_lastError = "Object doesn't exist";
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << object;
    return false;
  }

  QString signalSignature = QString::fromLatin1(signal);
  QString methodSignature = QString::fromLatin1(method);

  if (!d->checkConnectCorrection(signalSignature, methodSignature))
    return false;

  signalSignature = signalSignature.mid(1);
  methodSignature = methodSignature.mid(1);


  int methodIndex = -1;

  if (method[0] == '1')
    methodIndex = object->metaObject()->indexOfSlot(QMetaObject::normalizedSignature(methodSignature.toLatin1()));
  else if (method[0] == '2')
    methodIndex = object->metaObject()->indexOfSignal(QMetaObject::normalizedSignature(methodSignature.toLatin1()));

  if (methodIndex == -1)
  {
    d->m_lastError = "Method (slot or signal) doesn't exist:" + methodSignature;
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << object;
    return false;
  }

  bool ok = true;
  if (!(d->m_connections.contains(signalSignature)))
    ok = d->sendRemoteConnectRequest(signalSignature);

  if (ok)
    d->registerConnection(signalSignature, object, methodSignature);
  return ok;
}


/*!
    The method is used to connect the signal of some local object (on the client-side) to the remote signal or slot
    of the server.

    It returns true on success. False otherwise (the local signal doesn't exist, or signatures are incompatible).

    After the connection being established, all signals will be delivered asynchronously.

    \note It is recommended to use this method the same way as you call QObject::connect() method
    (by using SIGNAL() and SLOT() macros).
    \par
    For example, to connect the exampleSignal() signal of some local \a object to the remote \a exampleSlot() slot,
    you can type:
    \code remoteSlotConnect(object, SIGNAL(exampleSignal()), SLOT(exampleSlot())); \endcode

    \warning The method doesn't check the existence of the remote signal/slot on the server-side.

    \sa remoteConnect(), call()
 */
bool CuteIPCInterface::remoteConnect(QObject* localObject, const char* localSignal, const char* remoteMethod)
{
  Q_D(CuteIPCInterface);

  if (!localObject)
  {
    d->m_lastError = "Object doesn't exist";
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << localObject;
    return false;
  }

  QString signalSignature = QString::fromLatin1(localSignal);
  QString remoteMethodSignature = QString::fromLatin1(remoteMethod);

  if (!d->checkConnectCorrection(signalSignature, remoteMethodSignature))
    return false;

  signalSignature = signalSignature.mid(1);
  QChar type = remoteMethodSignature[0];
  remoteMethodSignature = remoteMethodSignature.mid(1);

  int signalIndex = localObject->metaObject()->indexOfSignal(
      QMetaObject::normalizedSignature(signalSignature.toLatin1()));

  if (signalIndex == -1)
  {
    d->m_lastError = "Signal doesn't exist:" + signalSignature;
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << localObject;
    return false;
  }


  if (type == '1') // slot
  {
    if (!d->checkRemoteSlotExistance(remoteMethodSignature))
    {
      d->m_lastError = "Remote slot doesn't exist:" + remoteMethodSignature;
      return false;
    }
  }
  else if (type == '2') // signal
  {
    if (!d->sendRemoteConnectRequest(remoteMethodSignature))
    {
      d->m_lastError = "Remote signal doesn't exist:" + remoteMethodSignature;
      return false;
    }
  }

  d->handleLocalSignalRequest(localObject, signalSignature, remoteMethodSignature);
  return true;
}


/*!
    Disconnects remote signal of server
    from local slot in object receiver.
    Returns true if the connection is successfully broken;
    otherwise returns false.

    \sa remoteConnect
 */
bool CuteIPCInterface::disconnectSignal(const char* signal, QObject* object, const char* method)
{
  Q_D(CuteIPCInterface);

  if (!object)
  {
    d->m_lastError = "Object doesn't exist";
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << object;
    return false;
  }

  if (signal[0] != '2' || (method[0] != '1' && method[0] != '2'))
    return false;

  QString signalSignature = QString::fromLatin1(signal).mid(1);
  QString methodSignature = QString::fromLatin1(method).mid(1);

  d->m_connections.remove(signalSignature, QPair<QObject*, QString>(object, methodSignature));
  if (!d->m_connections.contains(signalSignature))
    d->sendRemoteDisconnectRequest(signalSignature);
  return true;
}


/*!
    Disconnects local signal from remote slot of server.
    Returns true if the connection is successfully broken;
    otherwise returns false.

    \sa remoteConnect
 */
bool CuteIPCInterface::disconnectRemoteMethod(QObject* localObject, const char* signal, const char* remoteMethod)
{
  Q_D(CuteIPCInterface);

  if (!localObject)
  {
    d->m_lastError = "Object doesn't exist";
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << localObject;
    return false;
  }

  if (signal[0] != '2' || (remoteMethod[0] != '1' && remoteMethod[0] != '2'))
    return false;

  QString signalSignature = QString::fromLatin1(signal).mid(1);
  QString methodSignature = QString::fromLatin1(remoteMethod).mid(1);
  CuteIPCInterfacePrivate::MethodData data(localObject, signalSignature);

  QList<CuteIPCSignalHandler*> handlers = d->m_localSignalHandlers.values(data);
  foreach (CuteIPCSignalHandler* handler, handlers)
  {
    if (handler->signature() == methodSignature)
    {
      delete handler;
      d->m_localSignalHandlers.remove(data, handler);
    }
  }
  return true;
}


/*!
    The method is used to connect the signal of some local object (on the client-side) to the remote slot
    of the server.

    It returns true on success. False otherwise (the local signal doesn't exist, or signatures are incompatible).

    After the connection being established, all signals will be delivered asynchronously.

    \note It is recommended to use this method the same way as you call QObject::connect() method
    (by using SIGNAL() and SLOT() macros).
    \par
    For example, to connect the exampleSignal() signal of some local \a object to the remote \a exampleSlot() slot,
    you can type:
    \code remoteSlotConnect(object, SIGNAL(exampleSignal()), SLOT(exampleSlot())); \endcode

    \warning The method doesn't check the existence of the remote slot on the server-side.

    \sa remoteConnect(), call()
 */
bool CuteIPCInterface::remoteSlotConnect(QObject* localObject, const char* signal, const char* remoteSlot)
{
  Q_D(CuteIPCInterface);

  if (!localObject)
  {
    d->m_lastError = "Object doesn't exist";
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << localObject;
    return false;
  }

  QString signalSignature = QString::fromLatin1(signal);
  QString slotSignature = QString::fromLatin1(remoteSlot);

  if (!d->checkConnectCorrection(signalSignature, slotSignature))
    return false;

  signalSignature = signalSignature.mid(1);
  slotSignature = slotSignature.mid(1);

  int signalIndex = localObject->metaObject()->indexOfSignal(
      QMetaObject::normalizedSignature(signalSignature.toLatin1()));

  if (signalIndex == -1)
  {
    d->m_lastError = "Signal doesn't exist:" + signalSignature;
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << localObject;
    return false;
  }

  if (!d->checkRemoteSlotExistance(slotSignature))
  {
    d->m_lastError = "Remote slot doesn't exist:" + slotSignature;
    return false;
  }

  d->handleLocalSignalRequest(localObject, signalSignature, slotSignature);
  return true;
}


/*!
    Disconnects local signal from remote slot of server.
    Returns true if the connection is successfully broken;
    otherwise returns false.

    \sa remoteSlotConnect
 */
bool CuteIPCInterface::disconnectSlot(QObject* localObject, const char* signal, const char* remoteSlot)
{
  Q_D(CuteIPCInterface);

  if (!localObject)
  {
    d->m_lastError = "Object doesn't exist";
    qWarning() << "CuteIPC:" << "Error: " + d->m_lastError + "; object:" << localObject;
    return false;
  }

  if (signal[0] != '2' || remoteSlot[0] != '1')
    return false;

  QString signalSignature = QString::fromLatin1(signal).mid(1);
  QString slotSignature = QString::fromLatin1(remoteSlot).mid(1);
  CuteIPCInterfacePrivate::MethodData data(localObject, signalSignature);

  QList<CuteIPCSignalHandler*> handlers = d->m_localSignalHandlers.values(data);
  foreach (CuteIPCSignalHandler* handler, handlers)
  {
    if (handler->signature() == slotSignature)
    {
      delete handler;
      d->m_localSignalHandlers.remove(data, handler);
    }
  }
  return true;
}


/*!
    Invokes the remote \a method (of the server). Returns true if the invokation was successful, false otherwise.
    The invokation is synchronous (which means that client will be waiting for the response in the event loop).
    See callNoReply() method for asynchronous invokation.

    The signature of this method is completely concurs with QMetaObject::invokeMethod() Qt method signature.
    Thus, you can use it the same way as you did it locally, with invokeMethod().

    The return value of the member function call is placed in \a ret.
    You can pass up to ten arguments (val0, val1, val2, val3, val4, val5, val6, val7, val8, and val9)
    to the member function.

    \note To set arguments, you must enclose them using Q_ARG and Q_RETURN_ARG macros.
    \note This method doesn't establish the connection to the server, you must use connectToServer() first.
    \sa callNoReply()
 */
bool CuteIPCInterface::call(const QString& method, QGenericReturnArgument ret, QGenericArgument val0,
                            QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4,
                            QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8,
                            QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn, method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9, QString::fromLatin1(ret.name()));
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  DEBUG << "Remote call" << method;
  return d->sendSynchronousRequest(request, ret);
}


/*!
    This function overloads call() method.
    This overload can be used if the return value of the member is of no interest.

    \note To set arguments, you must enclose them using Q_ARG macro.
    \note This method doesn't establish the connection to the server, you must use connectToServer() first.
    \sa callNoReply()
 */
bool CuteIPCInterface::call(const QString& method, QGenericArgument val0, QGenericArgument val1, QGenericArgument val2,
                            QGenericArgument val3, QGenericArgument val4, QGenericArgument val5, QGenericArgument val6,
                            QGenericArgument val7, QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  CuteIPCMessage message(CuteIPCMessage::MessageCallWithReturn, method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  DEBUG << "Remote call" << method;
  return d->sendSynchronousRequest(request);
}



/*!
    Invokes the remote \a method (of the server). Returns true if the invokation was successful, false otherwise.
    Unlike the process of call() method, the invokation is asynchronous
    (which means that the client will not waiting for the response).

    The signature of this method is completely concurs with QMetaObject::invokeMethod() Qt method signature
    (without return value).
    Thus, you can use it the same way as you did it locally, with invokeMethod().

    You can pass up to ten arguments (val0, val1, val2, val3, val4, val5, val6, val7, val8, and val9)
    to the member function.

    \note To set arguments, you must enclose them using Q_ARG macro.
    \note This method doesn't establish the connection to the server, you must use connectToServer() first.
    \sa call(), connectToServer()
 */
void CuteIPCInterface::callNoReply(const QString& method, QGenericArgument val0, QGenericArgument val1,
                                        QGenericArgument val2, QGenericArgument val3, QGenericArgument val4,
                                        QGenericArgument val5, QGenericArgument val6, QGenericArgument val7,
                                        QGenericArgument val8, QGenericArgument val9)
{
  Q_D(CuteIPCInterface);
  CuteIPCMessage message(CuteIPCMessage::MessageCallWithoutReturn, method, val0, val1, val2, val3, val4,
                         val5, val6, val7, val8, val9);
  QByteArray request = CuteIPCMarshaller::marshallMessage(message);

  DEBUG << "Remote call (asynchronous)" << method;
  d->_q_sendAsynchronousRequest(request);
}


/*!
    Returns the last occured error.
 */
QString CuteIPCInterface::lastError() const
{
  Q_D(const CuteIPCInterface);
  return d->m_lastError;
}

#include "moc_CuteIPCInterface.cpp"
