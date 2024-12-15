#ifndef CUTEIPCINTERFACE_P_H
#define CUTEIPCINTERFACE_P_H

// Qt
#include <QObject>
#include <QMultiHash>
#include <QPointer>
#include <QHostAddress>
class QLocalSocket;

// Local
#include "CuteIPCInterface.h"
#include "CuteIPCMessage_p.h"
class CuteIPCInterfaceConnection;
class CuteIPCSignalHandler;
class CuteIPCInterfaceWorker;
class CuteIPCLoopVector;


class CuteIPCInterfacePrivate
{
  Q_DECLARE_PUBLIC(CuteIPCInterface)

  typedef QPair<QObject*,QString> MethodData;

  public:
    CuteIPCInterfacePrivate();
    virtual ~CuteIPCInterfacePrivate();

    bool checkConnectCorrection(const QString& signal, const QString& method);
    bool checkRemoteSlotExistance(const QString& slot);
    bool sendRemoteConnectRequest(const QString& signalSignature);
    bool sendRemoteDisconnectRequest(const QString& signalSignature);
    bool sendSynchronousRequest(const QByteArray& request, QGenericReturnArgument returnedObject = QGenericReturnArgument());

    void handleLocalSignalRequest(QObject* localObject, const QString& signalSignature, const QString& slotSignature);
    void registerConnection(const QString& signalSignature, QObject* reciever, const QString& methodSignature);

    void _q_sendAsynchronousRequest(const QByteArray& request);
    void _q_removeSignalHandlersOfObject(QObject*);
    void _q_setLastError(QString); //TODO: !!!!!!
    void _q_invokeRemoteSignal(const QString& signalSignature, const CuteIPCMessage::Arguments& arguments);
    void _q_removeRemoteConnectionsOfObject(QObject* destroyedObject);

    CuteIPCInterface* q_ptr;
    QMultiHash<MethodData, CuteIPCSignalHandler*> m_localSignalHandlers;
    QString m_lastError;

    QThread* m_workerThread;
    CuteIPCInterfaceWorker* m_worker;

    QString m_localServer;
    QPair<QHostAddress, quint16> m_tcpAddress;

    QMultiHash<QString, MethodData> m_connections;
};

#endif //CUTEIPCINTERFACE_P_H
