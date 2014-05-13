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
    peoplemodel.cpp \
    dialogs/persondialog.cpp

HEADERS  += mainwindow.h \
    placesmodel.h \
    peoplemodel.h \
    dialogs/persondialog.h

FORMS    += mainwindow.ui \
    dialogs/persondialog.ui

RESOURCES += db_assets.qrc
