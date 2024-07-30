/****************************************************************************
** Meta object code from reading C++ file 'CommandOutputContext.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/CommandOutputContext.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CommandOutputContext.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSCommandOutputContextENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSCommandOutputContextENDCLASS = QtMocHelpers::stringData(
    "CommandOutputContext",
    "filterChanged",
    "",
    "textChanged",
    "readyChanged",
    "errorChanged",
    "explanationChanged",
    "executable",
    "arguments",
    "text",
    "filter",
    "ready",
    "error",
    "explanation",
    "bugReportUrl"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCommandOutputContextENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       8,   49, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,    9 /* Public */,
       3,    0,   45,    2, 0x06,   10 /* Public */,
       4,    0,   46,    2, 0x06,   11 /* Public */,
       5,    0,   47,    2, 0x06,   12 /* Public */,
       6,    0,   48,    2, 0x06,   13 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       7, QMetaType::QString, 0x00015401, uint(-1), 0,
       8, QMetaType::QStringList, 0x00015401, uint(-1), 0,
       9, QMetaType::QString, 0x00015003, uint(1), 0,
      10, QMetaType::QString, 0x00015103, uint(0), 0,
      11, QMetaType::Bool, 0x00015003, uint(2), 0,
      12, QMetaType::QString, 0x00015003, uint(3), 0,
      13, QMetaType::QString, 0x00015003, uint(4), 0,
      14, QMetaType::QUrl, 0x00015401, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject CommandOutputContext::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSCommandOutputContextENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCommandOutputContextENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCommandOutputContextENDCLASS_t,
        // property 'executable'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'arguments'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // property 'text'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'filter'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'ready'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'error'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'explanation'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'bugReportUrl'
        QtPrivate::TypeAndForceComplete<QUrl, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CommandOutputContext, std::true_type>,
        // method 'filterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'textChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'readyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'errorChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'explanationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CommandOutputContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CommandOutputContext *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->filterChanged(); break;
        case 1: _t->textChanged(); break;
        case 2: _t->readyChanged(); break;
        case 3: _t->errorChanged(); break;
        case 4: _t->explanationChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CommandOutputContext::*)();
            if (_t _q_method = &CommandOutputContext::filterChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CommandOutputContext::*)();
            if (_t _q_method = &CommandOutputContext::textChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CommandOutputContext::*)();
            if (_t _q_method = &CommandOutputContext::readyChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CommandOutputContext::*)();
            if (_t _q_method = &CommandOutputContext::errorChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CommandOutputContext::*)();
            if (_t _q_method = &CommandOutputContext::explanationChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<CommandOutputContext *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->executableName(); break;
        case 1: *reinterpret_cast< QStringList*>(_v) = _t->arguments(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->m_text; break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->filter(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->m_ready; break;
        case 5: *reinterpret_cast< QString*>(_v) = _t->m_error; break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->m_explanation; break;
        case 7: *reinterpret_cast< QUrl*>(_v) = _t->m_bugReportUrl; break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<CommandOutputContext *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 2:
            if (_t->m_text != *reinterpret_cast< QString*>(_v)) {
                _t->m_text = *reinterpret_cast< QString*>(_v);
                Q_EMIT _t->textChanged();
            }
            break;
        case 3: _t->setFilter(*reinterpret_cast< QString*>(_v)); break;
        case 4:
            if (_t->m_ready != *reinterpret_cast< bool*>(_v)) {
                _t->m_ready = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->readyChanged();
            }
            break;
        case 5:
            if (_t->m_error != *reinterpret_cast< QString*>(_v)) {
                _t->m_error = *reinterpret_cast< QString*>(_v);
                Q_EMIT _t->errorChanged();
            }
            break;
        case 6:
            if (_t->m_explanation != *reinterpret_cast< QString*>(_v)) {
                _t->m_explanation = *reinterpret_cast< QString*>(_v);
                Q_EMIT _t->explanationChanged();
            }
            break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
    (void)_a;
}

const QMetaObject *CommandOutputContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CommandOutputContext::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCommandOutputContextENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CommandOutputContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void CommandOutputContext::filterChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CommandOutputContext::textChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CommandOutputContext::readyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CommandOutputContext::errorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void CommandOutputContext::explanationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
