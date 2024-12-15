#ifndef CUTEIPCMARSHALLER_P_H
#define CUTEIPCMARSHALLER_P_H

// Qt
#include <QString>
#include <QByteArray>
#include <QPair>
#include <QGenericArgument>

// local
#include "CuteIPCMessage_p.h"

class CuteIPCMarshaller
{
  public:
    static QByteArray marshallMessage(const CuteIPCMessage& message);
    static CuteIPCMessage demarshallMessage(QByteArray& call);
    static CuteIPCMessage demarshallResponse(QByteArray& call, QGenericReturnArgument arg);

    static CuteIPCMessage::MessageType demarshallMessageType(QByteArray& message);

    static void freeArguments(const CuteIPCMessage::Arguments&);
    static void freeArgument(QGenericArgument);

  private:
    static bool marshallArgumentToStream(QGenericArgument value, QDataStream& stream);
    static QGenericArgument demarshallArgumentFromStream(bool& ok, QDataStream& stream);

    static bool marshallQImageToStream(QGenericArgument value, QDataStream& stream);

    template <template<class QImage> class Container>
    static bool marshallContainerOfQImagesToStream(QGenericArgument value, QDataStream& stream);

    static bool loadQImage(QDataStream& stream, void* data);

    template <template<class QImage> class Container>
    static bool loadContainerOfQImages(QDataStream& stream, void* data);
};

#endif // CUTEIPCMARSHALLER_P_H
