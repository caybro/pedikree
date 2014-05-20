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
    dialogs/persondialog.cpp \
    dialogs/placedialog.cpp \
    dialogs/pddatedialog.cpp \
    dialogs/relationdialog.cpp \
    relationsmodel.cpp

HEADERS  += mainwindow.h \
    placesmodel.h \
    peoplemodel.h \
    dialogs/persondialog.h \
    dialogs/placedialog.h \
    dialogs/pddatedialog.h \
    dialogs/relationdialog.h \
    relationsmodel.h

FORMS    += mainwindow.ui \
    dialogs/persondialog.ui \
    dialogs/placedialog.ui \
    dialogs/pddatedialog.ui \
    dialogs/relationdialog.ui

RESOURCES += db_assets.qrc
