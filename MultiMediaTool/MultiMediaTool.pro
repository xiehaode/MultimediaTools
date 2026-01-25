QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/gui/application.cpp \
    src/gui/basewindow.cpp \
    src/gui/mytitlebar.cpp \
    src/main.cpp \
    src/gui/page/videopage.cpp \
    src/gui/menu.cpp

HEADERS += \
    src/gui/application.h \
    src/gui/basewindow.h \
    src/gui/mytitlebar.h \
    src/gui/page/videopage.h \
    src/gui/menu.h

FORMS += \
    src/gui/ui/videopage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# OpenCVFFMpegTools 工程的 TargetName 固定为 OPENCVTOOLS（Debug/Release 都一样），
# 因此这里不要链接 OPENCVTOOLSd。
win32: LIBS += -L$$PWD/../bin/ -lOPENCVTOOLS


INCLUDEPATH += $$PWD/../OpenCVTools
DEPENDPATH += $$PWD/../OpenCVTools

RESOURCES += \
    icon.qrc
