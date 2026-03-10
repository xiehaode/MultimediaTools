/****************************************************************************
** Meta object code from reading C++ file 'ffmpegcmd.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.11)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../MultiMediaTool/src/gui/page/ffmpegcmd.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ffmpegcmd.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.11. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ffmpegCmd_t {
    QByteArrayData data[20];
    char stringdata0[372];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ffmpegCmd_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ffmpegCmd_t qt_meta_stringdata_ffmpegCmd = {
    {
QT_MOC_LITERAL(0, 0, 9), // "ffmpegCmd"
QT_MOC_LITERAL(1, 10, 18), // "on_btn_Run_clicked"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 19), // "on_btn_Stop_clicked"
QT_MOC_LITERAL(4, 50, 27), // "on_btn_ClearHistory_clicked"
QT_MOC_LITERAL(5, 78, 20), // "onProcessOutputReady"
QT_MOC_LITERAL(6, 99, 19), // "onProcessErrorReady"
QT_MOC_LITERAL(7, 119, 17), // "onProcessFinished"
QT_MOC_LITERAL(8, 137, 8), // "exitCode"
QT_MOC_LITERAL(9, 146, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(10, 167, 10), // "exitStatus"
QT_MOC_LITERAL(11, 178, 26), // "onHistoryItemDoubleClicked"
QT_MOC_LITERAL(12, 205, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(13, 222, 4), // "item"
QT_MOC_LITERAL(14, 227, 21), // "on_btn_AISend_clicked"
QT_MOC_LITERAL(15, 249, 22), // "on_btn_AIClear_clicked"
QT_MOC_LITERAL(16, 272, 25), // "on_btn_AISettings_clicked"
QT_MOC_LITERAL(17, 298, 33), // "on_lineEdit_AIInput_returnPre..."
QT_MOC_LITERAL(18, 332, 31), // "on_groupBox_AIAssistant_toggled"
QT_MOC_LITERAL(19, 364, 7) // "enabled"

    },
    "ffmpegCmd\0on_btn_Run_clicked\0\0"
    "on_btn_Stop_clicked\0on_btn_ClearHistory_clicked\0"
    "onProcessOutputReady\0onProcessErrorReady\0"
    "onProcessFinished\0exitCode\0"
    "QProcess::ExitStatus\0exitStatus\0"
    "onHistoryItemDoubleClicked\0QListWidgetItem*\0"
    "item\0on_btn_AISend_clicked\0"
    "on_btn_AIClear_clicked\0on_btn_AISettings_clicked\0"
    "on_lineEdit_AIInput_returnPressed\0"
    "on_groupBox_AIAssistant_toggled\0enabled"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ffmpegCmd[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x08 /* Private */,
       3,    0,   75,    2, 0x08 /* Private */,
       4,    0,   76,    2, 0x08 /* Private */,
       5,    0,   77,    2, 0x08 /* Private */,
       6,    0,   78,    2, 0x08 /* Private */,
       7,    2,   79,    2, 0x08 /* Private */,
      11,    1,   84,    2, 0x08 /* Private */,
      14,    0,   87,    2, 0x08 /* Private */,
      15,    0,   88,    2, 0x08 /* Private */,
      16,    0,   89,    2, 0x08 /* Private */,
      17,    0,   90,    2, 0x08 /* Private */,
      18,    1,   91,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,    8,   10,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   19,

       0        // eod
};

void ffmpegCmd::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ffmpegCmd *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_btn_Run_clicked(); break;
        case 1: _t->on_btn_Stop_clicked(); break;
        case 2: _t->on_btn_ClearHistory_clicked(); break;
        case 3: _t->onProcessOutputReady(); break;
        case 4: _t->onProcessErrorReady(); break;
        case 5: _t->onProcessFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 6: _t->onHistoryItemDoubleClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 7: _t->on_btn_AISend_clicked(); break;
        case 8: _t->on_btn_AIClear_clicked(); break;
        case 9: _t->on_btn_AISettings_clicked(); break;
        case 10: _t->on_lineEdit_AIInput_returnPressed(); break;
        case 11: _t->on_groupBox_AIAssistant_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ffmpegCmd::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_ffmpegCmd.data,
    qt_meta_data_ffmpegCmd,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ffmpegCmd::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ffmpegCmd::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ffmpegCmd.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ffmpegCmd::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
