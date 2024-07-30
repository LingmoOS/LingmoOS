/****************************************************************************
** Meta object code from reading C++ file 'statisticsprovider.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../kcms/energy/statisticsprovider.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'statisticsprovider.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSStatisticsProviderENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSStatisticsProviderENDCLASS = QtMocHelpers::stringData(
    "StatisticsProvider",
    "deviceChanged",
    "",
    "typeChanged",
    "durationChanged",
    "dataChanged",
    "refresh",
    "device",
    "duration",
    "type",
    "HistoryType",
    "points",
    "count",
    "firstDataPointTime",
    "lastDataPointTime",
    "largestValue",
    "RateType",
    "ChargeType"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSStatisticsProviderENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       8,   49, // properties
       1,   89, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,   10 /* Public */,
       3,    0,   45,    2, 0x06,   11 /* Public */,
       4,    0,   46,    2, 0x06,   12 /* Public */,
       5,    0,   47,    2, 0x06,   13 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   48,    2, 0x0a,   14 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

 // properties: name, type, flags
       7, QMetaType::QString, 0x00015103, uint(0), 0,
       8, QMetaType::UInt, 0x00015103, uint(2), 0,
       9, 0x80000000 | 10, 0x0001510b, uint(1), 0,
      11, QMetaType::QVariantList, 0x00015001, uint(3), 0,
      12, QMetaType::Int, 0x00015001, uint(3), 0,
      13, QMetaType::Int, 0x00015001, uint(3), 0,
      14, QMetaType::Int, 0x00015001, uint(3), 0,
      15, QMetaType::Int, 0x00015001, uint(3), 0,

 // enums: name, alias, flags, count, data
      10,   10, 0x0,    2,   94,

 // enum data: key, value
      16, uint(StatisticsProvider::RateType),
      17, uint(StatisticsProvider::ChargeType),

       0        // eod
};

Q_CONSTINIT const QMetaObject StatisticsProvider::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSStatisticsProviderENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSStatisticsProviderENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSStatisticsProviderENDCLASS_t,
        // property 'device'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'duration'
        QtPrivate::TypeAndForceComplete<uint, std::true_type>,
        // property 'type'
        QtPrivate::TypeAndForceComplete<HistoryType, std::true_type>,
        // property 'points'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'count'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'firstDataPointTime'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'lastDataPointTime'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'largestValue'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // enum 'HistoryType'
        QtPrivate::TypeAndForceComplete<StatisticsProvider::HistoryType, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<StatisticsProvider, std::true_type>,
        // method 'deviceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'typeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'durationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refresh'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void StatisticsProvider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StatisticsProvider *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceChanged(); break;
        case 1: _t->typeChanged(); break;
        case 2: _t->durationChanged(); break;
        case 3: _t->dataChanged(); break;
        case 4: _t->refresh(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StatisticsProvider::*)();
            if (_t _q_method = &StatisticsProvider::deviceChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StatisticsProvider::*)();
            if (_t _q_method = &StatisticsProvider::typeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StatisticsProvider::*)();
            if (_t _q_method = &StatisticsProvider::durationChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StatisticsProvider::*)();
            if (_t _q_method = &StatisticsProvider::dataChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<StatisticsProvider *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->m_device; break;
        case 1: *reinterpret_cast< uint*>(_v) = _t->m_duration; break;
        case 2: *reinterpret_cast< HistoryType*>(_v) = _t->m_type; break;
        case 3: *reinterpret_cast< QVariantList*>(_v) = _t->asPoints(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->count(); break;
        case 5: *reinterpret_cast< int*>(_v) = _t->firstDataPointTime(); break;
        case 6: *reinterpret_cast< int*>(_v) = _t->lastDataPointTime(); break;
        case 7: *reinterpret_cast< int*>(_v) = _t->largestValue(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<StatisticsProvider *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setDevice(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setDuration(*reinterpret_cast< uint*>(_v)); break;
        case 2: _t->setType(*reinterpret_cast< HistoryType*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
    (void)_a;
}

const QMetaObject *StatisticsProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StatisticsProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSStatisticsProviderENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QQmlParserStatus"))
        return static_cast< QQmlParserStatus*>(this);
    if (!strcmp(_clname, "org.qt-project.Qt.QQmlParserStatus"))
        return static_cast< QQmlParserStatus*>(this);
    return QObject::qt_metacast(_clname);
}

int StatisticsProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void StatisticsProvider::deviceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void StatisticsProvider::typeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void StatisticsProvider::durationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void StatisticsProvider::dataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
