TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH = ../libs

HEADERS += \
    ../libs/ui_utils.h \
    src/arguments.h

SOURCES += \
    ../libs/ui_utils.cpp \
    src/arguments.cpp \
    src/main.cpp
