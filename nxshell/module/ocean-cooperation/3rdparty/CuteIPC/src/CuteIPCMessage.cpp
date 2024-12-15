// Local
#include "CuteIPCMessage_p.h"

// Qt
#include <QDebug>

CuteIPCMessage::CuteIPCMessage(MessageType type, const QString& method,
                               QGenericArgument val0, QGenericArgument val1,
                               QGenericArgument val2, QGenericArgument val3,
                               QGenericArgument val4, QGenericArgument val5,
                               QGenericArgument val6, QGenericArgument val7,
                               QGenericArgument val8, QGenericArgument val9,
                               const QString& returnType)
{
  m_messageType = type;

  m_method = method;

  if (val0.data())
    m_arguments.append(val0);
  if (val1.data())
    m_arguments.append(val1);
  if (val2.data())
    m_arguments.append(val2);
  if (val3.data())
    m_arguments.append(val3);
  if (val4.data())
    m_arguments.append(val4);
  if (val5.data())
    m_arguments.append(val5);
  if (val6.data())
    m_arguments.append(val6);
  if (val7.data())
    m_arguments.append(val7);
  if (val8.data())
    m_arguments.append(val8);
  if (val9.data())
    m_arguments.append(val9);

  m_returnType = returnType;
}


CuteIPCMessage::CuteIPCMessage(MessageType type, const QString& method,
                               const Arguments& arguments, const QString& returnType)
{
  m_method = method;
  m_arguments = arguments;
  m_messageType = type;
  m_returnType = returnType;
}


const QString& CuteIPCMessage::method() const
{
  return m_method;
}


const CuteIPCMessage::Arguments& CuteIPCMessage::arguments() const
{
  return m_arguments;
}


const CuteIPCMessage::MessageType& CuteIPCMessage::messageType() const
{
  return m_messageType;
}


const QString& CuteIPCMessage::returnType() const
{
  return m_returnType;
}


QDebug operator<<(QDebug dbg, const CuteIPCMessage& message)
{
  QString type;
  switch (message.messageType())
  {
    case CuteIPCMessage::MessageCallWithReturn:
      type = "CallWithReturn";
      break;
    case CuteIPCMessage::MessageCallWithoutReturn:
      type = "CallWithoutReturn";
      break;
    case CuteIPCMessage::MessageResponse:
      type = "Response";
      break;
    case CuteIPCMessage::MessageError:
      type = "Error";
      break;
    case CuteIPCMessage::SignalConnectionRequest:
      type = "SignalConnectionRequest";
      break;
    case CuteIPCMessage::MessageSignal:
      type = "Signal";
      break;
    case CuteIPCMessage::SlotConnectionRequest:
      type = "SlotConnectionRequest";
      break;
    case CuteIPCMessage::AboutToCloseSocket:
      type = "AboutToCloseSocket";
      break;
    case CuteIPCMessage::ConnectionInitialize:
      type = "ConnectionInitialize";
      break;
    default: break;
  }

  dbg.nospace() << "MESSAGE of type: " << type << "  " << "Method: " << message.method();

  if (message.arguments().length())
  {
    dbg.nospace() << "  " << "Arguments of type: ";
    foreach (const QGenericArgument& arg, message.arguments())
    {
      dbg.space() << arg.name();
    }
  }

  if (!message.returnType().isEmpty())
    dbg.space() << " " << "Return type:" << message.returnType();

  return dbg.space();
}
