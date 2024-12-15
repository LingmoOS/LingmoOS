#ifndef CUTEIPCMESSAGE_P_H
#define CUTEIPCMESSAGE_P_H

// Qt
#include <QString>
#include <QGenericArgument>
#include <QList>

#define DEBUG if (qgetenv("CUTEIPC_DEBUG") == "1") qDebug() << "CuteIPC:"

class CuteIPCMessage
{
  public:
    typedef QList<QGenericArgument> Arguments;

    enum MessageType
    {
      MessageCallWithReturn,
      MessageCallWithoutReturn,
      MessageResponse,
      MessageError,
      SignalConnectionRequest,
      SlotConnectionRequest,
      MessageSignal,
      AboutToCloseSocket,
      ConnectionInitialize
    };

    CuteIPCMessage(MessageType type,
                   const QString& method = QString(), QGenericArgument val0 = QGenericArgument(),
                   QGenericArgument val1 = QGenericArgument(), QGenericArgument val2 = QGenericArgument(),
                   QGenericArgument val3 = QGenericArgument(), QGenericArgument val4 = QGenericArgument(),
                   QGenericArgument val5 = QGenericArgument(), QGenericArgument val6 = QGenericArgument(),
                   QGenericArgument val7 = QGenericArgument(), QGenericArgument val8 = QGenericArgument(),
                   QGenericArgument val9 = QGenericArgument(), const QString& returnType = QString());

    CuteIPCMessage(MessageType type, const QString& method, const Arguments& arguments,
                   const QString& returnType = QString());

    const MessageType& messageType() const;
    const QString& method() const;
    const QString& returnType() const;
    const Arguments& arguments() const;

  private:
    QString m_method;
    Arguments m_arguments;
    MessageType m_messageType;
    QString m_returnType;
};

QDebug operator<<(QDebug dbg, const CuteIPCMessage& message);


#endif // CUTEIPCMESSAGE_P_H
