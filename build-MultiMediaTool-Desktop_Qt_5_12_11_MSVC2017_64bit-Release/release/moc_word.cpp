/****************************************************************************
** Meta object code from reading C++ file 'word.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MultiMediaTool/src/gui/page/word.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'word.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_word_t {
    QByteArrayData data[11];
    char stringdata0[168];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_word_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_word_t qt_meta_stringdata_word = {
    {
QT_MOC_LITERAL(0, 0, 4), // "word"
QT_MOC_LITERAL(1, 5, 20), // "on_btnImport_clicked"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 21), // "on_pushButton_clicked"
QT_MOC_LITERAL(4, 49, 34), // "on_comboBoxSrc_currentIndexCh..."
QT_MOC_LITERAL(5, 84, 5), // "index"
QT_MOC_LITERAL(6, 90, 17), // "onProcessFinished"
QT_MOC_LITERAL(7, 108, 8), // "exitCode"
QT_MOC_LITERAL(8, 117, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(9, 138, 10), // "exitStatus"
QT_MOC_LITERAL(10, 149, 18) // "onProcessReadyRead"

    },
    "word\0on_btnImport_clicked\0\0"
    "on_pushButton_clicked\0"
    "on_comboBoxSrc_currentIndexChanged\0"
    "index\0onProcessFinished\0exitCode\0"
    "QProcess::ExitStatus\0exitStatus\0"
    "onProcessReadyRead"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_word[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    0,   40,    2, 0x08 /* Private */,
       4,    1,   41,    2, 0x08 /* Private */,
       6,    2,   44,    2, 0x08 /* Private */,
      10,    0,   49,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 8,    7,    9,
    QMetaType::Void,

       0        // eod
};

void word::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<word *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_btnImport_clicked(); break;
        case 1: _t->on_pushButton_clicked(); break;
        case 2: _t->on_comboBoxSrc_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->onProcessFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 4: _t->onProcessReadyRead(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject word::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_word.data,
    qt_meta_data_word,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *word::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *word::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_word.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int word::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
