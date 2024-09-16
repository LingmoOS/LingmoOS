// Local
#include "CuteIPCSignalHandler_p.h"
#include "CuteIPCService.h"
#include "CuteIPCServiceConnection_p.h"
#include "CuteIPCMessage_p.h"
#include "CuteIPCMarshaller_p.h"

// Qt
#include <QObject>
#include <QMetaMethod>


CuteIPCSignalHandler::CuteIPCSignalHandler(const QString& signature, QObject* parent)
  : QObject(parent),
    m_signature(signature)
{
  QMetaObject::connect(this, this->metaObject()->indexOfSignal("destroyed(QString)"),
                       parent, parent->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("_q_removeSignalHandler(QString)")));
  m_signalParametersInfoWasSet = false;
}


void CuteIPCSignalHandler::setSignalParametersInfo(QObject* owner, const QString& signature)
{
  QMetaMethod method = owner->metaObject()->method(
                         owner->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(signature.toLatin1())));

  m_signalParametersInfo = method.parameterTypes();
  m_signalParametersInfoWasSet = true;
}


QString CuteIPCSignalHandler::signature() const
{
  return m_signature;
}


CuteIPCSignalHandler::~CuteIPCSignalHandler()
{}


void CuteIPCSignalHandler::relaySlot(void** args)
{
  if (!m_signalParametersInfoWasSet)
  {
    setSignalParametersInfo(parent(), m_signature);
    m_signalParametersInfoWasSet = true;
  }

  CuteIPCMessage::Arguments messageArguments;

  for (int i = 0; i < m_signalParametersInfo.size(); ++i)
  {
    const QByteArray& type = m_signalParametersInfo[i];

    //call arguments are started from index 1
    messageArguments.push_back(QGenericArgument(qstrdup(QString(type).toLatin1()), args[i+1]));
  }

  CuteIPCMessage message(CuteIPCMessage::MessageSignal, m_signature, messageArguments);
  QByteArray serializedMessage = CuteIPCMarshaller::marshallMessage(message);

  //cleanup memory
  foreach (const QGenericArgument& arg, messageArguments)
    delete[] arg.name();

  DEBUG << "Send remote signal" << message.method();

  emit signalCaptured(serializedMessage);
}


void CuteIPCSignalHandler::addListener(CuteIPCServiceConnection* listener)
{
  m_listeners.push_back(listener);

  QMetaObject::connect(listener, listener->metaObject()->indexOfSignal(
                         QMetaObject::normalizedSignature("destroyed(QObject*)")),
                       this, this->metaObject()->indexOfSlot("listenerDestroyed(QObject*)"));

  QMetaObject::connect(this, this->metaObject()->indexOfSignal(
                         QMetaObject::normalizedSignature("signalCaptured(QByteArray)")),
                       listener, listener->metaObject()->indexOfSlot(
                         QMetaObject::normalizedSignature("sendSignal(QByteArray)")));
}


void CuteIPCSignalHandler::removeListener(CuteIPCServiceConnection* listener)
{
  m_listeners.removeOne(listener);
  if (m_listeners.length() == 0)
  {
    emit destroyed(m_signature);
    this->deleteLater();
  }
}


void CuteIPCSignalHandler::listenerDestroyed(QObject* listener)
{
  for (int i = 0; i < m_listeners.size(); ++i)
  {
    if (qobject_cast<QObject*>(m_listeners[i])== listener)
    {
      m_listeners.removeAt(i);
      break;
    }
  }

  if (m_listeners.length() == 0)
  {
    emit destroyed(m_signature);
    this->deleteLater();
  }
}


// Our Meta Object
// modify carefully: this has been hand-edited!
// the relaySlot slot has local ID 1 (we use this when calling QMetaObject::connect)
// it also gets called with the void** array

#if QT_VERSION >= 0x050000
struct qt_meta_stringdata_CuteIPCSignalHandler_t {
    QByteArrayData data[12];
    char stringdata[152];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
  Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
  qptrdiff(offsetof(qt_meta_stringdata_CuteIPCSignalHandler_t, stringdata) + ofs \
  - idx * sizeof(QByteArrayData)) \
  )
static const qt_meta_stringdata_CuteIPCSignalHandler_t qt_meta_stringdata_CuteIPCSignalHandler = {
  {
    QT_MOC_LITERAL(0, 0, 20),
    QT_MOC_LITERAL(1, 21, 14),
    QT_MOC_LITERAL(2, 36, 0),
    QT_MOC_LITERAL(3, 37, 4),
    QT_MOC_LITERAL(4, 42, 9),
    QT_MOC_LITERAL(5, 52, 9),
    QT_MOC_LITERAL(6, 62, 9),
    QT_MOC_LITERAL(7, 72, 11),
    QT_MOC_LITERAL(8, 84, 25),
    QT_MOC_LITERAL(9, 110, 8),
    QT_MOC_LITERAL(10, 119, 14),
    QT_MOC_LITERAL(11, 134, 17)
  },
  "CuteIPCSignalHandler\0signalCaptured\0"
  "\0data\0destroyed\0signature\0relaySlot\0"
  "addListener\0CuteIPCServiceConnection*\0"
  "listener\0removeListener\0listenerDestroyed"
};

#undef QT_MOC_LITERAL


static const uint qt_meta_data_CuteIPCSignalHandler[] = {

  // content:
  7,       // revision
  0,       // classname
  0,    0, // classinfo
  6,   14, // methods
  0,    0, // properties
  0,    0, // enums/sets
  0,    0, // constructors
  0,       // flags
  2,       // signalCount

  // signals: name, argc, parameters, tag, flags
  1,    1,   44,    2, 0x06 /* Public */,
  4,    1,   47,    2, 0x06 /* Public */,

  // slots: name, argc, parameters, tag, flags
  6,    0,   50,    2, 0x0a /* Public */,
  7,    1,   51,    2, 0x0a /* Public */,
  10,    1,   54,    2, 0x0a /* Public */,
  11,    1,   57,    2, 0x0a /* Public */,

  // signals: parameters
  QMetaType::Void, QMetaType::QByteArray,    3,
  QMetaType::Void, QMetaType::QString,    5,

  // slots: parameters
  QMetaType::Void, //0x80000000 | 7,    2,
  QMetaType::Void, 0x80000000 | 8,   9,
  QMetaType::Void, 0x80000000 | 8,   9,
  QMetaType::Void, QMetaType::QObjectStar,   9,

  0        // eod
};


const QMetaObject CuteIPCSignalHandler::staticMetaObject = {
  { &QObject::staticMetaObject,
    qt_meta_stringdata_CuteIPCSignalHandler.data,
    qt_meta_data_CuteIPCSignalHandler,
    0,
    0,
    0 }
};
#else
static const uint qt_meta_data_CuteIPCSignalHandler[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   22,   21,   21, 0x05,
      64,   54,   21,   21, 0x05,

 // slots: signature, parameters, type, tag, flags
      83,   21,   21,   21, 0x0a,
     104,   95,   21,   21, 0x0a,
     143,   95,   21,   21, 0x0a,
     185,   95,   21,   21, 0x0a,

       0        // eod
};


static const char qt_meta_stringdata_CuteIPCSignalHandler[] = {
    "CuteIPCSignalHandler\0\0data\0"
    "signalCaptured(QByteArray)\0signature\0"
    "destroyed(QString)\0relaySlot()\0"
    "listener\0addListener(CuteIPCServiceConnection*)\0"
    "removeListener(CuteIPCServiceConnection*)\0"
    "listenerDestroyed(QObject*)\0"
};


const QMetaObject CuteIPCSignalHandler::staticMetaObject = {
  { &QObject::staticMetaObject,
    qt_meta_stringdata_CuteIPCSignalHandler,
    qt_meta_data_CuteIPCSignalHandler,
    0 }
};
#endif


#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CuteIPCSignalHandler::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION


const QMetaObject *CuteIPCSignalHandler::metaObject() const
{
#if QT_VERSION >= 0x050000
  return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
#else
  return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
#endif
}


void *CuteIPCSignalHandler::qt_metacast(const char *_clname)
{
  if (!_clname) return 0;

#if QT_VERSION >= 0x050000
  if (!strcmp(_clname, qt_meta_stringdata_CuteIPCSignalHandler.stringdata))
#else
  if (!strcmp(_clname, qt_meta_stringdata_CuteIPCSignalHandler))
#endif
    return static_cast<void*>(const_cast< CuteIPCSignalHandler*>(this));
  return QObject::qt_metacast(_clname);

}


int CuteIPCSignalHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
  _id = QObject::qt_metacall(_c, _id, _a);
  if (_id < 0)
    return _id;


  if (_c == QMetaObject::InvokeMetaMethod)
  {
    switch (_id)
    {
#if QT_VERSION >= 0x050000
      case 0: signalCaptured((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
      case 1: destroyed((*reinterpret_cast< QString(*)>(_a[1]))); break;
      case 2: relaySlot(_a); break;
      case 3: addListener((*reinterpret_cast< CuteIPCServiceConnection*(*)>(_a[1]))); break;
      case 4: removeListener((*reinterpret_cast< CuteIPCServiceConnection*(*)>(_a[1]))); break;
      case 5: listenerDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
      default: ;
    }
    _id -= 6;
  }
  else if (_c == QMetaObject::RegisterMethodArgumentMetaType)
  {
    if (_id < 6)
      *reinterpret_cast<int*>(_a[0]) = -1;
    _id -= 6;
  }
#else
      case 0: signalCaptured((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
      case 1: destroyed((*reinterpret_cast< QString(*)>(_a[1]))); break;
      case 2: relaySlot(_a); break;
      case 3: addListener((*reinterpret_cast< CuteIPCServiceConnection*(*)>(_a[1]))); break;
      case 4: removeListener((*reinterpret_cast< CuteIPCServiceConnection*(*)>(_a[1]))); break;
      case 5: listenerDestroyed((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
      default: ;
    }
    _id -= 5;
  }
#endif
  return _id;
}


// SIGNAL 0
void CuteIPCSignalHandler::signalCaptured(const QByteArray & _t1)
{
  void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
  QMetaObject::activate(this, &staticMetaObject, 0, _a);
}


// SIGNAL 1
void CuteIPCSignalHandler::destroyed(QString _t1)
{
  void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
  QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
