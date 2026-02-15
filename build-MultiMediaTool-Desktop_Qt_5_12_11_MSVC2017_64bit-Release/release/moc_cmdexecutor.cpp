/****************************************************************************
** Meta object code from reading C++ file 'cmdexecutor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MultiMediaTool/src/base/cmdexecutor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'cmdexecutor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CmdExecutor_t {
    QByteArrayData data[7];
    char stringdata0[73];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CmdExecutor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CmdExecutor_t qt_meta_stringdata_CmdExecutor = {
    {
QT_MOC_LITERAL(0, 0, 11), // "CmdExecutor"
QT_MOC_LITERAL(1, 12, 11), // "cmdFinished"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 8), // "exitCode"
QT_MOC_LITERAL(4, 34, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(5, 55, 10), // "exitStatus"
QT_MOC_LITERAL(6, 66, 6) // "output"

    },
    "CmdExecutor\0cmdFinished\0\0exitCode\0"
    "QProcess::ExitStatus\0exitStatus\0output"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CmdExecutor[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4, QMetaType::QString,    3,    5,    6,

       0        // eod
};

void CmdExecutor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CmdExecutor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->cmdFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CmdExecutor::*)(int , QProcess::ExitStatus , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CmdExecutor::cmdFinished)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CmdExecutor::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_CmdExecutor.data,
    qt_meta_data_CmdExecutor,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CmdExecutor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CmdExecutor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CmdExecutor.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CmdExecutor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void CmdExecutor::cmdFinished(int _t1, QProcess::ExitStatus _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
