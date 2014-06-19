#-------------------------------------------------
#
# Project created by QtCreator 2014-05-08T14:50:57
#
#-------------------------------------------------

QT       += core gui widgets sql network xml

TARGET = pedikree
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    placesmodel.cpp \
    peoplemodel.cpp \
    dialogs/persondialog.cpp \
    dialogs/placedialog.cpp \
    dialogs/pddatedialog.cpp \
    dialogs/relationdialog.cpp \
    relationsmodel.cpp \
    osmgeocoder.cpp \
    mapwidget/mapwidget.cpp \
    mapwidget/slippymap.cpp \
    person.cpp

HEADERS  += mainwindow.h \
    placesmodel.h \
    peoplemodel.h \
    dialogs/persondialog.h \
    dialogs/placedialog.h \
    dialogs/pddatedialog.h \
    dialogs/relationdialog.h \
    relationsmodel.h \
    osmgeocoder.h \
    mapwidget/mapwidget.h \
    mapwidget/slippymap.h \
    person.h

FORMS    += mainwindow.ui \
    dialogs/persondialog.ui \
    dialogs/placedialog.ui \
    dialogs/pddatedialog.ui \
    dialogs/relationdialog.ui

RESOURCES += \
    assets.qrc

DEFINES += QT_USE_QSTRINGBUILDER

TRANSLATIONS = translations/pedikree_cs.ts
