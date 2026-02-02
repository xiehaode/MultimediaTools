TARGET = mplayer
TEMPLATE = app

QT += core gui widgets opengl network


CONFIG += c++11

DESTDIR = ../bin

SOURCES += \
    src/ipcmgrbase.cpp \
    src/main.cpp \
    src/mdevice.cpp \
    src/mglwidget.cpp \
    src/mplayer.cpp \
    src/myipcmgr.cpp \
    src/player.cpp

HEADERS += \
    src/ffmpeg_util.h \
    src/ipcmgrbase.h \
    src/lan_util.h \
    src/mdevice.h \
    src/mframe.h \
    src/mglwidget.h \
    src/mplayer.h \
    src/myipcmgr.h \
    src/player.h


#-lstrmiids -ldshow -lole32 -loleaut32 -luuid     ------ldshow -lole32 -loleaut32 -luuid
win32 {
DEFINES += WIN32_LEAN_AND_MEAN
    # 补全所有DirectShow/COM必备库，-l+纯库名，无后缀、大小写匹配没有-ldshow
    LIBS += -lStrmiids  -lole32 -loleaut32 -luuid
    ## Windows API
    LIBS += -lkernel32    \
            -luser32      \
            -lgdi32       \
            \
            -lopengl32    \
            -lglu32       \
            \
            -lole32       \
            -loleaut32    \
            -lstrmiids    \
            \
            -lws2_32      \
            -lsecur32     \

}


win32: LIBS += -L$$PWD/3rd/lib/x64/ -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale

INCLUDEPATH += $$PWD/3rd/include
DEPENDPATH += $$PWD/3rd/include
