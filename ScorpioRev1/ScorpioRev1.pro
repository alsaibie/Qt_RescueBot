#-------------------------------------------------
#
# Project created by QtCreator 2014-07-06T16:34:47
#
#-------------------------------------------------
include(../qextserialport/src/qextserialport.pri)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScorpioRev1
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    graphicsitemscorpio.cpp \
    RR_SDLJoystick.cpp \
    RR_QtTelemetry.cpp \
    RR_QtSerial.cpp


HEADERS  += mainwindow.h \
    graphicsitemscorpio.h \
    RR_SDLJoystick.h \
    RR_QtTelemetry.h \
    RR_QtSerial.h \
    gpsconversion.h

win32{
INCLUDEPATH += "$$(SDL_DIR_MINGW32)include"
LIBS += -L "$$(SDL_DIR_MINGW32)lib" \
            -lSDL2main \
            -lSDL2
DEFINES += _TTY_WIN_
}

unix{
INCLUDEPATH += /home/alsaibie/Codes/SDL/include/
LIBS += -L /home/alsaibie/Codes/SDL/build/ \
            -lSDL2main \
            -lSDL2
DEFINES += _TTY_POSIX_
}

FORMS    += mainwindow.ui
RESOURCES += ScorpioRev1.qrc
