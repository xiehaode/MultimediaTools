QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#CONFIG += console
# 禁用 Qt 对标准输出的重定向
# DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT


SOURCES += \
    src/base/ipcmgrbase.cpp \
    src/gui/application.cpp \
    src/gui/basewindow.cpp \
    src/gui/mytitlebar.cpp \
    src/main.cpp \
    src/gui/page/videopage.cpp \
    src/gui/menu.cpp \
    src/gui/page/word.cpp \
    src/gui/page/effact.cpp \
    src/utils/encodinghelper.cpp \
    src/gui/page/picture.cpp \
    src/base/cmdexecutor.cpp \
    src/base/pageBase.cpp \
    src/base/timedcmdexecutor.cpp


HEADERS += \
    src/base/ipcmgrbase.h \
    src/gui/application.h \
    src/gui/basewindow.h \
    src/gui/mytitlebar.h \
    src/gui/page/videopage.h \
    src/gui/menu.h \
    src/gui/page/word.h \
    src/gui/page/effact.h \
    src/utils/encodinghelper.h \
    src/gui/page/picture.h \
    src/base/cmdexecutor.h \
    src/base/pageBase.h \
    src/base/timedcmdexecutor.h

FORMS += \
    src/gui/ui/videopage.ui\
    src/gui/ui/word.ui \
    src/gui/ui/effact.ui \
    src/gui/ui/picture.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32: LIBS += -L$$PWD/../bin/ -lOPENCVTOOLS


INCLUDEPATH += $$PWD/../OpenCVTools
DEPENDPATH += $$PWD/../OpenCVTools

RESOURCES += \
    icon.qrc

unix|win32: LIBS += -L$$PWD/../bin/ -lOPENCVTOOLS

DESTDIR = ../bin
