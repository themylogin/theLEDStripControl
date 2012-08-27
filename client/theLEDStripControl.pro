#-------------------------------------------------
#
# Project created by QtCreator 2012-08-26T19:01:15
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theLEDStripControl
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

LIBS += -lboost_serialization

SOURCES += main.cpp\
    ColorWidget.cpp \
    Dialog.cpp

HEADERS  += \
    ColorWidget.h \
    ../shared/theLEDStripControlDescription.hpp \
    Dialog.h

FORMS    += \
    ColorWidget.ui \
    Dialog.ui
