/****************************************************************************
** Meta object code from reading C++ file 'Entry.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../kcms/about-distro/src/Entry.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Entry.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSEntryENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSEntryENDCLASS = QtMocHelpers::stringData(
    "Entry",
    "localizedLabel",
    "",
    "Language",
    "language",
    "localizedValue",
    "isHidden",
    "System",
    "English",
    "Hidden",
    "No",
    "Yes"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSEntryENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       2,   53, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   44,    2, 0x142,    3 /* Public | isScriptable | MethodIsConst  */,
       1,    0,   47,    2, 0x162,    5 /* Public | MethodCloned | isScriptable | MethodIsConst  */,
       5,    1,   48,    2, 0x142,    6 /* Public | isScriptable | MethodIsConst  */,
       5,    0,   51,    2, 0x162,    8 /* Public | MethodCloned | isScriptable | MethodIsConst  */,
       6,    0,   52,    2, 0x102,    9 /* Public | MethodIsConst  */,

 // methods: parameters
    QMetaType::QString, 0x80000000 | 3,    4,
    QMetaType::QString,
    QMetaType::QString, 0x80000000 | 3,    4,
    QMetaType::QString,
    QMetaType::Bool,

 // enums: name, alias, flags, count, data
       3,    3, 0x2,    2,   63,
       9,    9, 0x2,    2,   67,

 // enum data: key, value
       7, uint(Entry::Language::System),
       8, uint(Entry::Language::English),
      10, uint(Entry::Hidden::No),
      11, uint(Entry::Hidden::Yes),

       0        // eod
};

Q_CONSTINIT const QMetaObject Entry::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSEntryENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSEntryENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSEntryENDCLASS_t,
        // enum 'Language'
        QtPrivate::TypeAndForceComplete<Entry::Language, std::true_type>,
        // enum 'Hidden'
        QtPrivate::TypeAndForceComplete<Entry::Hidden, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Entry, std::true_type>,
        // method 'localizedLabel'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<Language, std::false_type>,
        // method 'localizedLabel'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'localizedValue'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<Language, std::false_type>,
        // method 'localizedValue'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'isHidden'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void Entry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Entry *>(_o);
        (void)_t;
        switch (_id) {
        case 0: { QString _r = _t->localizedLabel((*reinterpret_cast< std::add_pointer_t<Language>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 1: { QString _r = _t->localizedLabel();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 2: { QString _r = _t->localizedValue((*reinterpret_cast< std::add_pointer_t<Language>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 3: { QString _r = _t->localizedValue();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 4: { bool _r = _t->isHidden();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *Entry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Entry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSEntryENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Entry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
    }
    return _id;
}
QT_WARNING_POP
