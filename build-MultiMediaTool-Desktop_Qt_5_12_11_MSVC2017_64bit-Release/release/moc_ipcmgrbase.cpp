/****************************************************************************
** Meta object code from reading C++ file 'ipcmgrbase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MultiMediaTool/src/base/ipcmgrbase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ipcmgrbase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IPCMgrBase_t {
    QByteArrayData data[18];
    char stringdata0[264];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IPCMgrBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IPCMgrBase_t qt_meta_stringdata_IPCMgrBase = {
    {
QT_MOC_LITERAL(0, 0, 10), // "IPCMgrBase"
QT_MOC_LITERAL(1, 11, 15), // "messageReceived"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 3), // "msg"
QT_MOC_LITERAL(4, 32, 18), // "childProcessExited"
QT_MOC_LITERAL(5, 51, 8), // "exitCode"
QT_MOC_LITERAL(6, 60, 14), // "connectSuccess"
QT_MOC_LITERAL(7, 75, 13), // "connectFailed"
QT_MOC_LITERAL(8, 89, 17), // "onMessageReceived"
QT_MOC_LITERAL(9, 107, 20), // "onChildProcessExited"
QT_MOC_LITERAL(10, 128, 16), // "onConnectSuccess"
QT_MOC_LITERAL(11, 145, 15), // "onConnectFailed"
QT_MOC_LITERAL(12, 161, 20), // "onNewClientConnected"
QT_MOC_LITERAL(13, 182, 11), // "onReadyRead"
QT_MOC_LITERAL(14, 194, 14), // "onDisconnected"
QT_MOC_LITERAL(15, 209, 22), // "onChildProcessFinished"
QT_MOC_LITERAL(16, 232, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(17, 253, 10) // "exitStatus"

    },
    "IPCMgrBase\0messageReceived\0\0msg\0"
    "childProcessExited\0exitCode\0connectSuccess\0"
    "connectFailed\0onMessageReceived\0"
    "onChildProcessExited\0onConnectSuccess\0"
    "onConnectFailed\0onNewClientConnected\0"
    "onReadyRead\0onDisconnected\0"
    "onChildProcessFinished\0QProcess::ExitStatus\0"
    "exitStatus"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IPCMgrBase[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,
       4,    1,   77,    2, 0x06 /* Public */,
       6,    0,   80,    2, 0x06 /* Public */,
       7,    0,   81,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   82,    2, 0x09 /* Protected */,
       9,    1,   85,    2, 0x09 /* Protected */,
      10,    0,   88,    2, 0x09 /* Protected */,
      11,    0,   89,    2, 0x09 /* Protected */,
      12,    0,   90,    2, 0x08 /* Private */,
      13,    0,   91,    2, 0x08 /* Private */,
      14,    0,   92,    2, 0x08 /* Private */,
      15,    2,   93,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 16,    5,   17,

       0        // eod
};

void IPCMgrBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IPCMgrBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->messageReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->childProcessExited((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->connectSuccess(); break;
        case 3: _t->connectFailed(); break;
        case 4: _t->onMessageReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->onChildProcessExited((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onConnectSuccess(); break;
        case 7: _t->onConnectFailed(); break;
        case 8: _t->onNewClientConnected(); break;
        case 9: _t->onReadyRead(); break;
        case 10: _t->onDisconnected(); break;
        case 11: _t->onChildProcessFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (IPCMgrBase::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IPCMgrBase::messageReceived)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (IPCMgrBase::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IPCMgrBase::childProcessExited)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (IPCMgrBase::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IPCMgrBase::connectSuccess)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (IPCMgrBase::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IPCMgrBase::connectFailed)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IPCMgrBase::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_IPCMgrBase.data,
    qt_meta_data_IPCMgrBase,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IPCMgrBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IPCMgrBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IPCMgrBase.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int IPCMgrBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void IPCMgrBase::messageReceived(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IPCMgrBase::childProcessExited(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void IPCMgrBase::connectSuccess()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void IPCMgrBase::connectFailed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
