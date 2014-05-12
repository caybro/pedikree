#-------------------------------------------------
#
# Project created by QtCreator 2014-05-08T14:50:57
#
#-------------------------------------------------

QT       += core gui widgets sql

TARGET = pedikree
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    placesmodel.cpp \
    peoplemodel.cpp

HEADERS  += mainwindow.h \
    placesmodel.h \
    peoplemodel.h

FORMS    += mainwindow.ui

RESOURCES += \
    db_assets.qrc
