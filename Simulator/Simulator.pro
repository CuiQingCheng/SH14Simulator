#-------------------------------------------------
#
# Project created by QtCreator 2019-07-03T11:44:53
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += xml
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Simulator
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    Signal/SignalValue.cpp \
    Telegram/Telegram.cpp \
    todvobccomm/Todvobctelegram.cpp \
    Gui/CheckBox.cpp \
    Gui/TextEdit.cpp \
    Gui/LineEdit.cpp \
    Core/Parser.cpp \
    Core/Factory/Factory.cpp \
    Core/Handler/AutoTestHandler.cpp \
    Core/Handler/WidgetHandler.cpp \
    Core/TodCommChannel/TodCommChannel.cpp \
    todtcmsccomm/Todtcmstelegram.cpp \
    CRC/mycrc.cpp

HEADERS += \
        mainwindow.h \
    Signal/SignalValue.h \
    Telegram/Telegram.h \
    todvobccomm/Todvobctelegram.h \
    Gui/CheckBox.h \
    Gui/TextEdit.h \
    Gui/LineEdit.h \
    Core/Parser.h \
    Core/Factory/Factory.h \
    Core/Handler/AutoTestHandler.h \
    Core/Handler/WidgetHandler.h \
    Core/TodCommChannel/TodCommChannel.h \
    todtcmsccomm/Todtcmstelegram.h \
    CRC/mycrc.h

FORMS += \
        mainwindow.ui

DISTFILES +=

RESOURCES += \
    resource.qrc
