/****************************************************************************
** Meta object code from reading C++ file 'ffmpegclientwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MultiMediaTool/src/gui/page/ffmpegclientwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ffmpegclientwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FfmpegClientWidget_t {
    QByteArrayData data[12];
    char stringdata0[185];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FfmpegClientWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FfmpegClientWidget_t qt_meta_stringdata_FfmpegClientWidget = {
    {
QT_MOC_LITERAL(0, 0, 18), // "FfmpegClientWidget"
QT_MOC_LITERAL(1, 19, 14), // "onLoginClicked"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 12), // "onLoginReply"
QT_MOC_LITERAL(4, 48, 20), // "onValidateTokenReply"
QT_MOC_LITERAL(5, 69, 17), // "onRegisterClicked"
QT_MOC_LITERAL(6, 87, 18), // "onShowRegisterPage"
QT_MOC_LITERAL(7, 106, 15), // "onShowLoginPage"
QT_MOC_LITERAL(8, 122, 15), // "onRegisterReply"
QT_MOC_LITERAL(9, 138, 15), // "onLogoutClicked"
QT_MOC_LITERAL(10, 154, 18), // "updateServerStatus"
QT_MOC_LITERAL(11, 173, 11) // "onTimerTick"

    },
    "FfmpegClientWidget\0onLoginClicked\0\0"
    "onLoginReply\0onValidateTokenReply\0"
    "onRegisterClicked\0onShowRegisterPage\0"
    "onShowLoginPage\0onRegisterReply\0"
    "onLogoutClicked\0updateServerStatus\0"
    "onTimerTick"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FfmpegClientWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x08 /* Private */,
       3,    0,   65,    2, 0x08 /* Private */,
       4,    0,   66,    2, 0x08 /* Private */,
       5,    0,   67,    2, 0x08 /* Private */,
       6,    0,   68,    2, 0x08 /* Private */,
       7,    0,   69,    2, 0x08 /* Private */,
       8,    0,   70,    2, 0x08 /* Private */,
       9,    0,   71,    2, 0x08 /* Private */,
      10,    0,   72,    2, 0x08 /* Private */,
      11,    0,   73,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void FfmpegClientWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FfmpegClientWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onLoginClicked(); break;
        case 1: _t->onLoginReply(); break;
        case 2: _t->onValidateTokenReply(); break;
        case 3: _t->onRegisterClicked(); break;
        case 4: _t->onShowRegisterPage(); break;
        case 5: _t->onShowLoginPage(); break;
        case 6: _t->onRegisterReply(); break;
        case 7: _t->onLogoutClicked(); break;
        case 8: _t->updateServerStatus(); break;
        case 9: _t->onTimerTick(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject FfmpegClientWidget::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_FfmpegClientWidget.data,
    qt_meta_data_FfmpegClientWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FfmpegClientWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FfmpegClientWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FfmpegClientWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int FfmpegClientWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
