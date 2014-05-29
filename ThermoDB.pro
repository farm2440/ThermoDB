#-------------------------------------------------
#
# Project created by QtCreator 2014-05-13T08:00:34
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ThermoDB
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    channel.cpp \
    dialogperiodselect.cpp \
    dialogtimestamp.cpp

HEADERS  += mainwindow.h \
    channel.h \
    dialogperiodselect.h \
    dialogtimestamp.h

FORMS    += mainwindow.ui \
    dialogperiodselect.ui \
    dialogtimestamp.ui
