/****************************************************************************
** Meta object code from reading C++ file 'pageBase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MultiMediaTool/src/base/pageBase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pageBase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_pageBase_t {
    QByteArrayData data[8];
    char stringdata0[95];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_pageBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_pageBase_t qt_meta_stringdata_pageBase = {
    {
QT_MOC_LITERAL(0, 0, 8), // "pageBase"
QT_MOC_LITERAL(1, 9, 17), // "onMessageReceived"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 3), // "msg"
QT_MOC_LITERAL(4, 32, 20), // "onChildProcessExited"
QT_MOC_LITERAL(5, 53, 8), // "exitCode"
QT_MOC_LITERAL(6, 62, 16), // "onConnectSuccess"
QT_MOC_LITERAL(7, 79, 15) // "onConnectFailed"

    },
    "pageBase\0onMessageReceived\0\0msg\0"
    "onChildProcessExited\0exitCode\0"
    "onConnectSuccess\0onConnectFailed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_pageBase[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x09 /* Protected */,
       4,    1,   37,    2, 0x09 /* Protected */,
       6,    0,   40,    2, 0x09 /* Protected */,
       7,    0,   41,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void pageBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<pageBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onMessageReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onChildProcessExited((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onConnectSuccess(); break;
        case 3: _t->onConnectFailed(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject pageBase::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_pageBase.data,
    qt_meta_data_pageBase,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *pageBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *pageBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_pageBase.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int pageBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
